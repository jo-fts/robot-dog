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

# 快速入门
moss

# 目录结构
moss "review the code structure of this project"

# Piped stdin also works
echo "write me a ROS2 topic listener node" | moss
在TUI内，按Shift+Tab键切换交互模式：

模式	行为
plan	只读规划，什么都不执行
default	每次工具调用时的批准提示（建议开始）
accept-edits	自动运行文件编辑和命令（仅受拒绝列表和只读上限限制）
输入以内联附加文件，或输入完整命令引用。@/help

苔藓的三种使用方式
Three ways to use Moss

模式1 — 地方发展
直接在电脑上构建机器人应用;Moss负责读写代码、搜索文档并验证构建：

moss
> write a ROS2 node that subscribes to /camera/image_raw and runs edge detection with OpenCV
适合写ROS2节点、调试Python脚本、搜索官方RDK示例、代码审查。

模式2 — 连接到电路板
SSH 隧道将会话移动到板上;ROS2 和诊断自动解锁：

/connect root@192.168.1.10              # password login (entered interactively)
/connect root@192.168.1.10 --key ~/.ssh/id_rsa   # key-based login
/connect root@192.168.1.10 --hybrid     # keep local tools too
/disconnect                              # leave, back to local mode
连接后，Moss 检测主板型号、操作系统版本和 TROS 状态，然后解锁：

ROS2 工具：ros2_topic_list / ros2_topic_echo / ros2_topic_hz / ros2_node_list / ros2_service_list / ros2_service_call / ros2_launch / ros2_pkg_list
设备诊断：device_temperature / device_resources / device_processes / device_network / device_cameras
远程执行：device_exec / device_file_read / device_file_list
模式3 — 设备内运行
直接在 RDK 板上安装并运行 Moss，无需外接电脑：

# On the board
npm i -g @rdk-moss/agent@latest
moss
适合：边缘调试、机器人自主任务、嵌入RDK Studio作为代理模块。

内置RDK技能
Moss发布了开放设备知识包——20个自动加载的文件，无需设置：SKILL.md

技能	翻唱
rdk-llm-deployment	设备内LLM/VLM部署（hobot_llamacpp / InternVL / Qwen）
rdk-model-zoo	官方预建模型查找、下载与部署
rdk-ros	TROS / ROS2 节点目录、主题映射、感知
rdk-device	模型量化（hb_mapper / hb_compile）、BPU部署
rdk-peripheral-cookbook	GPIO / I2C / SPI 外设编程
rdk-multimedia	摄像机、编解码器、多媒体流水线
...共20个	主板知识、系统配置、文档查找器、Embodied/LeRobot、Jetson/Raspberry Pi 等
添加你自己的技能：在项目目录里放一个，Moss 启动时加载它：SKILL.md.moss/skills/

.moss/skills/my-robot-setup/SKILL.md
你也可以在配置中指向Moss的额外全局技能目录：

// .moss/config.json
{ "skills": { "extraRoots": ["~/.claude/skills", "/path/to/shared/skills"] } }
团队知识库：Moss学习的技能候选人通过晋升，然后通过团队技能目录（配置为）共享，确保成功的运行在团队中可重复使用。/skills promoteskills.extraRoots

交换机型号与供应商
moss setup          # interactive: choose provider, paste the API key
moss auth status    # show the resolved provider, model, and key source
分辨率优先级（高→低）：

CLI flags / -c key=value
  → project .moss/config.json
  → config saved by moss setup
  → built-in D-Robotics gateway (default, no setup)
注意：Moss 故意忽略环境变量，比如 ，所以导出给其他工具的密钥永远不会悄然更改你代理的提供者。OPENAI_API_KEY

长任务与简历
Moss每次游戏都会自动保存。被打断的任务不会丢失：

moss resume --last        # continue the most recent session
moss --continue           # continue the most recent session in place
moss resume <session-id>  # resume a specific session
moss sessions             # list all saved sessions
设定一个长远目标，莫斯会自主运行直到达成：

/goal deploy InternVL3 on the RDK X5 and pass the performance benchmark
达到回合限制的游戏是暂停的，不是失败——特工会告诉你如何继续。

命令引用
指挥	目的
moss	开始互动环节
moss "a task"	一次性：回答并退出
moss resume --last	继续最新一次会议
moss setup	配置模型和提供者
moss auth status	显示当前授权状态
moss doctor	健康检查（配置、连接、工具、MCP）
/connect <ip>	连接一块RDK板，进入板模模式
/disconnect	离开板子模式，恢复本地工具
/status	显示当前会话状态
/model	本次会议更换模型
/goal <condition>	设定目标，跑到达成为止
/diff · /review	节目变动·《虫子》评测
/compact	压缩历史，保存令牌
/skills	查看技能 · 一位博学的候选人promote
/help	完整指挥列表
建筑
A TypeScript / ESM / npm-workspaces monorepo （Node.js >= 22.16）.围绕明确的主机边界构建：

User (TUI / CLI / embedded SDK)
        ↕
Moss Agent Core          ← agent loop + goal runner
  ├─ Tool framework      ← ~30 core + ~17 device tools (on connect)
  ├─ Context & memory    ← compaction, persistence, retrieval
  ├─ Skill system        ← SKILL.md registry & learning
  └─ Safety & approval   ← layered confirmation
        ↕ Host Adapter contract
Model gateway / RDK board / device-knowledge
包装	NPM	职责
packages/moss	@rdk-moss/core	核心合同（KnowledgeModule、Host Adapter、提示符）。完全没有宿主依赖。
packages/moss-agent	@rdk-moss/agent	运行时 + CLI（代理循环、工具、内存、技能、安全）。moss
packages/create-moss-app	create-moss-app	项目支架（/模板）。minimalopenai
将苔藓嵌入您的产品中：

npx create-moss-app my-host
import {
  MOSS_HOST_ADAPTER_CONTRACT_VERSION,
  evaluateMossHostCompatibility,
  type MossHostRuntimeManifest,
} from '@rdk-moss/core/contracts/host-adapter';
主机会注册其提供者/工具/存储/审批门，发布一个，并在CI中运行，然后再采用发布版本。请参见docs/host-adapter-contract.md。MossHostRuntimeManifestevaluateMossHostCompatibility()

贡献
git clone https://github.com/D-Robotics/moss.git
cd moss && npm install
npm run verify   # boundaries + hygiene + build + typecheck + lint + test
莫斯的北极星是一个机器人级、宿主中立的运行时间。在提出功能提案前，请阅读范围规则——任何硬编码机器人家族或供应商工作流程的内容都应属于主机适配器、知识模块或平台扩展，而非核心。CLAUDE.md

更多内容：

CONTRIBUTING.md — 开发设置、命令、界限以及如何发送PR
docs/host-adapter-contract.md — 主机适配器合同与版本策略
CLAUDE.md—— 代理工作规则、架构审查纪律和漏洞修复检查表

🧠 边做边学莫斯在执行任务时观察着;一次会话后，它会将成功的流程提炼成一个技能候选——确认后保存为 SKILL.md。下次遇到同样问题时可以重复使用——团队知识自然积累，无需手动文档。/skills promote

⏸ 可重复的长任务每一步都会自动保存。被中断的任务（Ctrl-C、网络断开、关闭）不会丢失进度——会从中断处继续。远景目标能经得起重启。moss resume --last

🛡 设计成诚实将经过验证的事实与推断分开，报告不可用的功能，绝不伪造未核实的结果。每次工具调用都经过审批门，所以你能掌控局面。
🔌
