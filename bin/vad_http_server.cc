////
//// Created by zyf on 4/8/23.
////
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/ThreadPool.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/NetException.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Stringifier.h"

#include "infer/FsmnVad.h"
#include "glog/logging.h"
#include <gflags/gflags.h>
#include "utils/wav.h"

using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;

DEFINE_string(vad_model, "../model/model.onnx", "Vad model file");
DEFINE_int32(vad_sample_rate, 16000, "Vad model sample rate");
DEFINE_int32(vad_silence_duration, 800, "Vad silence duration");
DEFINE_int32(vad_max_single_segment_time, 15000, "Vad max single segment time");
DEFINE_double(vad_speech_noise_thres, 0.9, "Vad speech noise threshold");
DEFINE_int32(port, 8080, "HTTP server port");

FsmnVad fsmn_vad = FsmnVad();

class MyRequestHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
        try {
            // 读取请求头信息
            std::string contentType = request.getContentType();
            std::string contentLength = request.get("Content-Length");
            std::string filename = request.get("filename", "");
            if (contentType == "audio/wave" && !contentLength.empty()) {
                // 读取请求体二进制数据
                int length = std::stoi(contentLength);
                std::vector<char> buffer(length);
                request.stream().read(&buffer[0], buffer.size());

                std::vector<float> pcm_data = get_pcm_data(buffer);
                std::vector<std::vector<int>> vad_segments = fsmn_vad.infer(pcm_data);

                Poco::JSON::Object res;
                res.set("filename", filename);
                Poco::JSON::Array sentences;
                for (auto vad_segment: vad_segments) {
                    Poco::JSON::Object segment;
                    segment.set("bg", vad_segment[0]);
                    segment.set("ed", vad_segment[1]);
                    sentences.add(segment);
                }
                res.set("vad_segments", sentences);

                Poco::JSON::Stringifier::stringify(res, response.send());

                // 设置响应头
                response.setContentType("application/json");
                response.setStatus(HTTPResponse::HTTP_OK);

            } else {
                throw HTTPException("Invalid content type or content length.");
            }
        }
        catch (const Exception &ex) {
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, ex.displayText());
            response.send();
        }
        catch (...) {
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, "Unknown error.");
            response.send();
        }
    }
};

class MyHTTPServer : public ServerApplication {

public:
    int main(const std::vector<std::string> &args) {

        ServerSocket socket(FLAGS_port);
        ThreadPool threadPool(sysconf(_SC_NPROCESSORS_ONLN));
        HTTPServer server(new MyRequestHandlerFactory, threadPool, socket, new HTTPServerParams);
        server.start();
        LOG(INFO) << "Vad server started at " << FLAGS_port;
        waitForTerminationRequest();
        server.stop();
        return Application::EXIT_OK;
    }

private:
    class MyRequestHandlerFactory : public HTTPRequestHandlerFactory {
    public:
        MyRequestHandler *createRequestHandler(const HTTPServerRequest &) override {
            return new MyRequestHandler;
        }
    };
};

int main(int argc, char **argv) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true;

    fsmn_vad.init_vad(FLAGS_vad_model, FLAGS_vad_sample_rate, FLAGS_vad_silence_duration,
                      FLAGS_vad_max_single_segment_time, FLAGS_vad_speech_noise_thres);
    MyHTTPServer app;
    return app.run(argc, argv);
}


