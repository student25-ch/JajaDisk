# JajaDisk

开源磁盘管理与数据恢复工具（DiskGenius 风格）

**Copyright (C) 2026 杨子桐**

## 项目简介

JajaDisk 是一个面向 Windows 平台的磁盘管理工具，采用 Qt/C++ 编写 GUI，
核心磁盘操作封装在自研的 `JajaDiskCore.dll` 中。

**当前 v0.1 版本聚焦只读安全功能：**
- 枚举物理磁盘与分区
- 分区结构可视化（彩色色块图）
- 分区参数表格（起始 LBA、大小、类型、活动标志）
- 分区表备份到文件
- 原始扇区十六进制查看

## 许可证
本项目采用 **GNU Affero 通用公共许可证第 3 版或更新版本（AGPL-3.0-or-later）**。
详见 `LICENSE` 文件。

## 构建方式
1. 安装 Qt 6 (MinGW 64-bit) 或 Qt 5。
2. 使用 Qt Creator 打开根目录 `CMakeLists.txt`。
3. 选择 MinGW 构建套件，点击运行。
4. **注意：** 读取磁盘需要管理员权限，请以管理员身份运行生成的 `JajaDisk.exe`。
