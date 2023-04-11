cmake_minimum_required(VERSION 3.14)
project(VAD_Server)

add_executable(vad_server bin/vad_http_server.cc utils/wav.h infer/FsmnVad.h infer/FsmnVad.cc)
add_executable(vad_client bin/vad_http_client.cc )

# 将可执行文件输出到 build/bin 目录下
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
include_directories(
        ${PROJECT_SOURCE_DIR} /infer
        ${PROJECT_SOURCE_DIR} /utils
        ${PROJECT_SOURCE_DIR}/kaldi-native-fbank
        ${PROJECT_SOURCE_DIR}/onnxruntime/include
        ${gflags_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/glog
)

target_link_directories(vad_server PRIVATE ${PROJECT_SOURCE_DIR}/onnxruntime/lib)

set(GFLAGS_NAMESPACE "google" CACHE STRING "Gflags namespace")
option(BUILD_SHARED_LIBS "Build shared libraries (DLLs)." ON)
add_subdirectory(gflags)
add_subdirectory(poco)
add_subdirectory(kaldi-native-fbank/kaldi-native-fbank/csrc)
add_subdirectory(glog)

target_link_libraries(vad_server PUBLIC onnxruntime csrc gflags glog Poco::Net Poco::Util )
target_link_libraries(vad_client PUBLIC gflags glog Poco::Net )
