//
// Created by zyf on 4/10/23.
//

#include <iostream>
#include <fstream>
#include <vector>

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "glog/logging.h"
#include "gflags/gflags.h"
#include "glog/logging.h"


DEFINE_string(server, "http://127.0.0.1:8080", "Vad http server");
DEFINE_string(wav_file, "16k.wav", "Wave file");
//TODO
//DEFINE_int32(concurrency, 1, "For performance test only,concurrency threads");
//DEFINE_int32(round, 1, "For performance test only, repeat test times");

using namespace Poco;
using namespace Poco::Net;

// 定义发送请求的线程函数
void sendRequest(const URI &uri, const std::vector<char> &content) {
    HTTPClientSession session(uri.getHost(), uri.getPort());
    HTTPRequest request(HTTPRequest::HTTP_POST, "/");
    request.setContentType("audio/wave");
    request.setContentLength(content.size());

    try {
        request.set("filename",FLAGS_wav_file);
        // 发送请求
        std::ostream &requestBody = session.sendRequest(request);
        requestBody.write(&content[0], content.size());

        // 获取响应
        HTTPResponse response;
        std::istream &responseBody = session.receiveResponse(response);

        // 打印响应信息
        LOG(INFO) << "Status: " << response.getStatus() << " "
                << response.getReason();
        LOG(INFO) << responseBody.rdbuf();
    } catch (Exception &ex) {
        LOG(ERROR) << ex.displayText();
    }
}

int main(int argc, char **argv) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = true;

    // 解析 URI
    URI uri(FLAGS_server);

    // 读取二进制音频文件内容
    std::vector<char> content;
    File file(FLAGS_wav_file);
    if (file.exists() && file.isFile()) {
        std::ifstream ifs(file.path(), std::ios::binary);
        content.resize(file.getSize());
        ifs.read(&content[0], content.size());
    } else {
        LOG(ERROR) << "File not found or is not a regular file." << std::endl;
        return EXIT_FAILURE;
    }

    try {
        // 使用多线程发送请求
        Thread thread;
        thread.startFunc([&uri, &content]() {
            sendRequest(uri, content);
        });
        thread.join();
    } catch (Exception &ex) {
        LOG(ERROR) << ex.displayText();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}