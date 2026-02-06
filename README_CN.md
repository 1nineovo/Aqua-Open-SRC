# 🌊 Aqua Client

![Version](https://img.shields.io/badge/Minecraft_Bedrock-1.21.9x-green?style=flat-square&logo=minecraft)
![License](https://img.shields.io/badge/License-MIT-blue?style=flat-square)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?style=flat-square&logo=windows)

<!-- Jump Link -->
[🇺🇸 **English Documentation**](./README.md)

---

## 📖 简介 (Introduction)

**Aqua** 是一款针对 **Minecraft 基岩版 (Bedrock Edition) 1.21.9x** 开发的高性能开源内部 Hook 客户端。

本项目拥有完全基于 **ImGui** 构建的渲染系统以及较为完善的 SDK 支持。代码结构经过精心设计，逻辑清晰，非常适合作为学习游戏逆向分析或二次开发的参考范例。

🔗 **项目仓库**: [https://github.com/1nineovo/Aqua-Open-SRC](https://github.com/1nineovo/Aqua-Open-SRC)

## ✨ 核心特性 (Features)

*   🎨 **完全 ImGui 渲染**
    *   摒弃传统渲染方式，全套 UI 采用 **Dear ImGui** 绘制，带来丝滑的视觉体验和极高的自定义自由度。
*   📚 **完善的 SDK 支持**
    *   提供了较为完全的基岩版引擎 SDK 封装，极大地简化了功能开发和内存交互流程。
*   🏗️ **清晰的项目结构**
    *   代码架构分层明确，逻辑清晰，易于阅读、维护和功能扩展。
*   ⚓ **内部 Hook 实现**
    *   采用内存 Hook 技术直接介入游戏循环，性能损耗极低，响应速度快。

## 🛠️ 构建环境 (Environment)

本项目不提供具体的构建步骤教程。若要编译源代码，请确保您的开发设备具备以下环境：

*   **IDE**: Visual Studio 2022 (推荐最新版本)
*   **SDK**: Windows 10/11 SDK
*   **语言标准**: C++ 20 (推荐)

## 📝 致谢与资源 (Credits)

本项目使用了以下优秀的第三方资源：

*   **Dear ImGui**
    *   用于构建核心图形用户界面的 C++ GUI 库。
    *   [https://github.com/ocornut/imgui](https://github.com/ocornut/imgui)
*   **MiSans**
    *   本客户端 UI 默认搭载了小米 **MiSans** 字体，以提供清晰舒适的阅读体验。
    *   [https://hyperos.mi.com/font/download](https://hyperos.mi.com/font/download)
*   **StarPwn**
    *   [https://github.com/StarPwn](https://github.com/StarPwn)

## ⚖️ 开源协议 (License)

本项目采用 **MIT 协议** 开源。详细条款请参阅仓库根目录下的 `LICENSE` 文件。

## ⚠️ 免责声明 (Disclaimer)

本软件仅供 **教育、学习和技术研究** 使用。Aqua Client 与 Mojang Studios 或 Microsoft 无任何关联。请勿用于破坏游戏平衡，开发者不对因使用本软件产生的后果负责。
