# 项目描述

- DAMO/FSMN VAD C++ onnx 推理实现
- 通过RestFul接口获取音频文件vad信息
- 支持16k 16bit mono PCM格式wav推理

## 编译安装
```
mkdir build && cd build
cmake .. && cmake --build .
```
## 启动方法

server启动

```
./vad_server \
     --vad_model ../model/model.onnx \
     --vad_sample_rate 16000 \
     --vad_silence_duration 800 \
     --vad_max_single_segment_time 15000 \
     --vad_speech_noise_thres 0.9 \
     --port 8080
```

client测试

```
./vad_client \
     --server  "http://127.0.0.1:8080" \
     --wav_file xx.wav
```


## 接口名称

返回音频的VAD信息

## 请求URL

```
POST /
```

## 请求头

| 参数名   | 类型   | 必选 | 描述             |
| -------- | ------ | ---- | ---------------- |
| `filename` | string | 是   | 发送的wave音频名称 |

## 请求体

请求体需要发送一个二进制的wave音频文件。

## 响应体

| 参数名       | 类型   | 描述                                   |
| ------------ | ------ | -------------------------------------- |
| `filename`     | string | 音频文件名                             |
| `vad_segments` | array  | 音频文件的VAD信息，每个元素由`bg`和`ed`两个int类型字段表示开始和结束时间（毫秒） |

## 返回示例

```json
{
  "filename": "test.wav",
  "vad_segments": [
    {
      "bg": 0,
      "ed": 2500
    },
    {
      "bg": 3000,
      "ed": 5700
    }
  ]
}
```

## 错误码

| 错误码 | 错误描述               |
| ------ | ---------------------- |
| `500`    | 服务器内部错误         |
