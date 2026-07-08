# 项目概述与设计理念
本项目是一款基于**具身智能理念**、面向老年人、视障人士及慢病患者等约2.0–2.5亿特需人群的多场景陪护机器狗。项目以“让智能科技成为连接‘室内安全’与‘户外自由’的情感纽带”为核心理念，融合机器人、AI与数字化感知技术，将居家养老与户外出行辅助整合于同一平台，旨在破解特需人群“室内无人照料、户外不敢出行”的双重困境，重塑有温度的智慧生活空间。面对深度老龄化社会，项目确立“主动关怀、无缝守护”的核心宣言，将具身智能贯穿始终——把智能体视为通过**“感知—决策—执行”**实时闭环与环境深度耦合的主动实体。机械狗不仅会“思考”，更通过四足步态、云台扫描、避障绕行等物理行动与环境持续交互，使AI真正“具身化”。产品摒弃功能堆砌，探索“未来智居环境”范式，将居家与出行场景有机融合，打造主动感知、主动关怀、主动守护的智慧生活新生态。
居家场景：自然语言对话、药品识别、记忆云图、主动预警、轻物搬运，构建全天候安全照护。
户外场景：盲道识别、障碍物检测、DeepSORT多目标跟踪、自主循迹，为视障用户提供动态导引。
城市治理：自动上报道路破损、盲道被占信息至管理平台，形成“众包式”道路巡检。



# 核心能力
🧠 智能语音交互	基于云端大模型，支持多轮对话、方言识别、语速调节，可控制机器人动作、查询天气、播放新闻等

🛡 语义控制功能	自然语音指令（如“往前走”“停下”）直接控制机器狗动作，以及功能的转换，实现“听懂即做到”

🔌 药品/物体识别	搭载高清摄像头与YOLOv11，实时识别药品并语音讲解名称、用法、注意事项，支持“看见即讲解”

⏸  盲道自主循迹	YOLOv11实例分割识别盲道，PID算法控制四足步态沿盲道巡航，摄像头四档位全方位扫描找回盲道

💬 道路破损上报	自动识别破损路面、盲道被占等问题，通过GPS/北斗获取定位并上报城市管理平台，记忆云图	记录老人关键事件、人际关系、重要日期，形成个性化记忆图谱，主动提醒或回答遗忘问题。

🔗 障碍物检测与多目标跟踪	部署DeepSORT算法，持续追踪视野内目标，预测运动轨迹，输出具象化预警（如“左侧有行人快速接近”）

🤖 状态预警与主动守护	智能打卡机制，超时未打卡自动语音询问，连续无响应则向子女或社区平台发送预警

# 芯片型号与平台
1. RDK X5（地平线——边缘AI计算核心）：视觉识别与推理：部署轻量化 YOLOv11 模型，实时运行目标检测与实例分割。算力加速策略：将训练好的 .pt 模型转化为 .mnn 轻量化格式，借助芯片内置的 BPU（脑神经网络处理单元） 进行硬件加速，实现 29.6帧/秒 的实时推理，后处理耗时控制在10ms内。系统运行环境：搭载 Ubuntu 系统，负责图像采集、数据预处理（“先筛选后计算”优化策略），并通过 UART 串口 与下位机 STM32 通信，下发运动控制触发信号。
软件环境： 操作系统：Ubuntu 20.04（RDK X5板端系统）
          Python版本：3.8+
          主要依赖库：
          opencv-python>=4.5.0
          numpy>=1.19.0
          scipy>=1.6.0
          hobot-dnn>=1.0.0
          pyserial>=3.5

  
3. STM32F103C8T6（意法半导体——底层运动执行核心）：四足步态解算：运行 几何运动学逆解算法，将空间足端坐标（x,y,z）实时转换为12个关节舵机的目标角度，驱动机械狗实现全向移动、零半径转向及越障。
软件环境： STM32CubeIDE for VS Code

5. ESP32-S3（乐鑫——物联网网关与语音交互核心）：集成 语音识别（ASR） → DeepSeek 大模型（LLM） → 语音合成（TTS） 全链路。解析大模型返回的 JSON 控制流（区分“交流字符串”与“控制字符串”）：交流内容直接合成语音播报；控制指令通过 UART 串口 转发给以上芯片执行。使用MCP（模型上下文协议）封装功能为标准化 JSON-RPC 2.0 工具，供云端大模型动态调用，实现“意图→工具选择→物理执行”的闭环。


📦 通用依赖（跨平台）

依赖项	版本/说明

Git	任意版本，用于代码管理

Python 3.8+	模型训练、数据处理、边缘推理脚本

CMake 3.16+	ESP32 与 STM32 构建系统

串口驱动	CP210x / CH340（USB 转串口芯片，视具体开发板而定）

硬件清单	详见 docs/hardware_bom.md

📌 版本信息汇总
RDK X5	Ubuntu 22.04 + 地平线 AI 工具链	最新稳定版

ESP32-S3	VS Code + ESP-IDF Extension	ESP-IDF v5.3.3

STM32F103C8T6	VS Code + STM32Cube for VS Code	最新稳定版

🔗 其他环境配置参考

MCP Server 配置：参见 config/mcp_server.json

大模型 API 密钥：在 config/secret_template.json 中填写后重命名为 secret.json（已加入 .gitignore）

Blinker 物联网平台配置：参见 docs/blinker_setup.md

# 快速入门
本项目基于 “端-边-云”三层异构架构，需为以下三款核心芯片分别配置开发环境。

1. RDK X5（地平线）——边缘AI计算核心

RDK X5 具备 10 Tops 端侧推理算力与 8核 ARM A55 处理器，负责 YOLOv11 模型推理与 DeepSORT 多目标跟踪。

资源链接

官方文档中心	https://developer.d-robotics.cc/rdk_doc_center/

系统镜像构建工具 (x5-rdk-gen)	https://github.com/D-Robotics/x5-rdk-gen

系统烧录指南	https://d-robotics.github.io/rdk_doc/Quick_start/install_os/rdk_x5/

RDK 套件使用文档	https://developer.d-robotics.cc/rdk_doc

主机编译环境要求：

操作系统：Ubuntu 22.04（推荐与 RDK X5 保持同版本）

必备软件包：

bash

sudo apt update

sudo apt install build-essential make cmake python3-numpy git repo

交叉编译工具链：http://archive.d-robotics.cc/toolchain/

Python 依赖：

bash

pip install ultralytics opencv-python numpy pyserial

部署流程：

将训练好的 .pt 模型转化为 .onnx，再转化为 .mnn 轻量化格式

通过地平线 AI 工具链量化并部署至 BPU

在 RDK X5 上运行推理服务（Python / C++）

2. ESP32-S3（乐鑫）——语音交互与 MCP 网关核心

ESP32-S3 搭载 Xtensa® 32 位 LX7 双核处理器，负责语音采集、MCP Server 运行及云端大模型调度。

官方文档：

资源链接

ESP-IDF 快速入门（通用）	https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/get-started/

ESP32-S3 编程指南（v5.3.3）	https://documentation.espressif.com/esp-idf/zh_CN/v5.3.3/esp32s3/

VS Code ESP-IDF 扩展	https://docs.espressif.com/projects/vscode-esp-idf-extension/

推荐开发方式：VS Code + ESP-IDF Extension

安装步骤：

安装 VS Code 后，搜索并安装 ESP-IDF Extension

通过扩展的配置向导，选择 ESP-IDF 版本 v5.3.3，自动下载工具链（GCC、CMake、Ninja）

若使用 Windows，需提前安装 CP210x USB 驱动（串口调试必备）

bash
# 手动安装 ESP-IDF 备用方案
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
source export.sh

3. STM32F103C8T6（意法半导体）——底层运动执行核心

STM32F103C8T6 基于 ARM Cortex-M3 内核，负责舵机控制、陀螺仪读取、超声波避障等实时运动任务。

官方文档：

资源链接

STM32Cube for VS Code 产品页	https://www.st.com/stm32cubevscode

STM32Cube for VS Code 安装指南	https://dev.st.com/stm32cube-docs/stm32cubeide-vscode/1.0.1/en/docs/markup/getting_started/installation.html

官方文档入口	https://dev.st.com/stm32cube-docs/stm32cubeide-vscode/latest/en/

官方支持论坛	https://community.st.com/s/topic/0TO3P000001PSCrWAO/stm32-vs-code-extension

推荐开发方式：VS Code + STM32Cube for VS Code 扩展包（官方一站式方案）

安装步骤：

安装 VS Code 后，在扩展市场搜索 STM32，安装由 STMicroelectronics 官方发布的 STM32Cube for VS Code 扩展包（会自动包含 CMake、CubeMX 等工具）

根据使用的调试器提前安装驱动：

ST-Link：可通过扩展自动安装驱动

J-Link：需从 SEGGER 官网手动下载安装

创建工程：点击 VS Code 左侧 STM32Cube 图标 → 新建项目 → 选择芯片 STM32F103C8T6 → 在 Project Manager 中将 Toolchain/IDE 设置为 CMake → 生成代码后自动打开

编译/烧录/调试：点击 VS Code 底部状态栏的 “编译” 按钮，或侧边栏的 “运行和调试” → 选择 STM32Cube 配置




# 目录结构
moss "review the code structure of this project"

# Piped stdin also works
echo "write me a ROS2 topic listener node" | moss
在TUI内，按Shift+Tab键切换交互模式：
