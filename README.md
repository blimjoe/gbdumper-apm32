# GBDumper for APM32

这是一个基于 APM32F103 系列 MCU 的 Game Boy / Game Boy Color / Game Boy Advance 卡带转储（ROM Dumper）项目。

它通过硬件方式连接到 Nintendo 游戏卡带，并利用 MCU 读取卡带中的 ROM 数据，再通过 USB 把数据传送到宿主机（PC）。宿主机端程序会把这些数据保存为 ROM 文件，用于备份、研究或存档恢复等用途。

## 1. 使用的芯片

本项目基于 APM32F103 系列 MCU，属于 ARM Cortex-M3 内核的 32 位微控制器。

项目中包含了以下板卡支持：

- APM32F103 MINI
- APM32F103VCS MINI

这些板卡用于控制卡带总线、读取 ROM 数据，并通过 USB CDC 与 PC 通信。

## 2. 这个项目做什么

这个项目的核心功能是：

- 检测插入的卡带类型
  - Game Boy（GB）
  - Game Boy Color（GBC）
  - Game Boy Advance（GBA）
- 读取卡带头信息
- 判断卡带大小及类型
- 从卡带中读取 ROM 数据
- 通过 USB 发送到宿主机
- 在 PC 上保存为 `.GB`、`.GBC` 或 `.GBA` 文件

简单来说，这个项目就是一个“游戏卡带转储器”，用于把实体卡带中的程序内容导出成文件。

## 3. 系统结构

这个仓库主要分为两部分：

### 3.1 MCU 固件

MCU 端程序运行在 APM32F103 上，负责：

- 控制 GPIO 和总线信号
- 读取卡带地址和数据
- 处理 GB/GBC/GBA 卡带的不同读写方式
- 通过 USB 把数据发送给 PC

### 3.2 宿主机程序

宿主机程序在 Linux 环境中编译运行，负责：

- 打开 USB 串口 / CDC 设备
- 接收 MCU 发送的数据
- 判断当前卡带类型
- 保存转储文件到本地磁盘

例如，主程序 `host/dumpAll.c` 会根据卡带类型调用不同的读取流程，并把 ROM 写入文件。

## 4. 项目目录说明

- `Application/`：应用程序源码
- `Boards/`：开发板和外设配置代码
- `Libraries/`：MCU 相关库文件
- `RTT/`：SEGGER RTT 相关支持
- `host/`：宿主机端转储工具源代码
- `project.uvprojx`：Keil 工程文件
- `README.md`：项目说明

## 5. 编译 MCU 固件

打开 Keil 5 工程文件：

```bash
project.uvprojx
```

本项目在以下环境中验证过：

> IDE-Version:  
> μVision V5.38.0.0  
> Copyright (C) 2022 ARM Ltd and ARM Germany GmbH. All rights reserved.
>
> Toolchain: MDK-ARM Plus Version: 5.38.0.0  
> Toolchain Path: C:\Keil_v5\ARM\ARMCLANG\Bin  
> C Compiler: ArmClang.exe V6.19  
> Assembler: Armasm.exe V6.19  
> Linker/Locator: ArmLink.exe V6.19  
> Library Manager: ArmAr.exe V6.19  
> Hex Converter: FromElf.exe V6.19

## 6. 烧录到 MCU

编译完成后，使用 J-Flash 或其他支持的下载工具把固件烧录到 APM32F103 开发板中。

## 7. 编译宿主机程序

当前宿主机软件是 Linux x86_64 环境下编译的：

```bash
gcc -o host/dumpAll host/dumpAll.c
```

## 8. 使用方法

1. 用 USB Type-C 转 USB Type-A 线连接开发板到 PC
2. 将游戏卡带插入转储设备
3. 运行宿主机程序：

```bash
cd host
./dumpAll
```

程序会自动检测卡带类型并开始读取卡带内容，然后把数据保存到当前目录下的文件中。

## 9. 适用场景

该项目适合用于：

- 游戏卡带备份
- ROM 提取
- 硬件调试
- 嵌入式系统学习
- Nintendo 经典游戏机接口研究

## 10. 说明

本项目是一个硬件开发和嵌入式接口实验项目，重点在于利用 MCU 读取游戏机卡带数据，并通过 USB 把内容传送到 PC。它不是通用型游戏机模拟器或商用 ROM 发行工具，而是面向硬件读取和数据提取的工程实现。

## 11. 结论

这套方案使用 APM32F103 系列 MCU 来实现经典任天堂卡带的 ROM 转储功能，最终通过 PC 端脚本保存卡带内容，属于一类“硬件级 ROM 读取/备份”工具。

---

如果你需要，我也可以继续把这份 README 再整理成：

- 更正式的 GitHub 风格英文版
- 更简洁的中文极简版
- 带图片/流程图的版本
- 对外发布用的 README 版本
