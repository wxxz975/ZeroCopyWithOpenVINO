
# OpenVINO + 零拷贝 + ResNet50 Demo

## 概述

本文档提供了在Intel xboard开发板上使用OpenVINO工具包运行ResNet50演示的概述和说明。此演示的主要重点是展示**零拷贝技术**的实现，优化主机与集成GPU之间的数据传输。

## 先决条件

在运行演示之前，请确保满足以下先决条件：

- OpenVINO工具包：在开发板上安装OpenVINO工具包。您可以在[官方OpenVINO网站](https://software.intel.com/en-us/openvino-toolkit)上找到安装说明。

- Intel开发板：确保Intel xboard 正确配置c++开发环境。

- ResNet50模型：从[OpenVINO Model Zoo](https://github.com/openvinotoolkit/open_model_zoo)下载ResNet50模型，或使用其他适当的预训练ResNet50模型, demo中的[resnet50v2](models/resnet50v2.onnx) 由ImageNet数据集训练。

## 运行

1. **克隆存储库：**
   ```bash
   git clone https://github.com/wxxz975/ZeroCopyWithOpenVINO.git
   cd ZeroCopyWithOpenVINO
   ```
2. **构建演示**
    ```bash
    mkdir build && cd build
    cmake ..
    make -j4
    ```
3. **运行案例**
    ```
    ./ZeroCopyResnet50 <your_model_path> <your_video_path>
    ```
