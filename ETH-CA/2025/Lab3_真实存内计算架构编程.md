# 实验 3：真实存内计算架构编程

**课程：** 计算机体系结构（Computer Architecture，227-2210-00L），2025 年秋季学期
**授课教师：** Prof. Onur Mutlu、Dr. Mohammad Sadrosadati
**助教：** Dr. Konstantina Koliogeorgi、Dr. Zhiheng Yue、Rahul Bera、Konstantinos Kanellopoulos、Nika Mansouri Ghiasi、Ataberk Olgun、Nisa Bostanci、Rakesh Nadig、Ismail Emir Yuksel、Haocong Luo、Mayank Kabra、Andreas Kosmas Kakolyris、Harshita Gupta、Konstantinos Marios Sgouras、Harsh Songara、Susana Rebolledo Ruiz
**负责答疑的助教：** Mayank Kabra、Harshita Gupta

**布置日期：** 2025 年 10 月 24 日（星期五）
**截止日期：** 2025 年 11 月 7 日（星期五）午夜

## 1. 引言

本实验中，你将动手使用一套真实的 **存内计算（Processing-in-Memory，PIM）** 架构。你需要在多个工作负载中对 **UPMEM PIM** 架构 [1, 2, 3, 4] 进行编程和实验。主要目标是：(1) 熟悉 UPMEM PIM 系统的组织结构（作为真实世界以存储器为中心的计算系统的例子）；(2) 理解 UPMEM 编程模型并编写自己的代码；(3) 理解 UPMEM PIM 核心——**DRAM 处理单元（DRAM Processing Unit，DPU）**——的微体系结构及指令集体系结构（ISA）。

正如课堂上介绍的，UPMEM PIM 架构包含多个 DPU（最多 **2,560 个**）。每个 DPU 都能访问自己的 DRAM bank（称为 **Main RAM，MRAM**）和自己的暂存存储器（scratchpad，称为 **Working RAM，WRAM**）。有关 UPMEM PIM 系统的完整描述，参见 [3, 4]。

## 2. 任务 0/4：安装 UPMEM SDK

目标是在自己的机器上安装和配置 UPMEM SDK，以编译并运行后续任务的代码。如果使用支持的 Linux 版本，可以直接从 UPMEM 网站 [1, 2] 原生安装 UPMEM SDK。如果安装遇到问题，或无法使用受支持的 Linux 系统，可以使用我们提供的 `Dockerfile`，并结合适用于 Windows 或 Unix 系列宿主系统的 shell 脚本。

### 2.1. 使用 Dockerfile

使用 `Dockerfile` 前，需要先安装 [Docker](https://www.docker.com) [5] 或 [Podman](https://podman.io) [6] 等容器工具。安装 Docker（或 Podman）后，在 Linux 上可以运行 shell 脚本 `container/start_docker.sh`（使用 Podman 时为 `container/start_podman.sh`）；在 Windows 上可以运行 `container\start_docker.bat`（Podman 版本为 `container\start_podman.bat`）。

```bash
./container/start_docker.sh
```

脚本会自动构建容器镜像（首次构建需要几分钟），随后在容器中启动交互式 shell。启动容器时所在的宿主机工作目录会被挂载到容器内部（可以在容器中执行 `ls` 试试看）。之后即可通过这个交互式 shell 编译、运行本实验的代码。

## 3. 任务 1/4：在主存与支持 PIM 的存储器之间传输数据

目标是熟悉宿主机主存与 PIM 存储器之间不同类型的数据传输。本任务提供了模板，详情见第 9 节。

**任务内容：**

1. 编写宿主机程序，测试宿主机主存与一个或多个 MRAM bank 之间所有类型的数据传输。具体而言，有三种传输类型 [2]：(1) 串行（serial）、(2) 并行（parallel）以及 (3) 广播（broadcast）。串行和并行传输都可以将数据从主存传至 MRAM bank，或反方向传输；广播只能由**主存向 MRAM bank** 进行。
2. 分别使用每个 DPU **1 MB、24 MB、48 MB** 的数据量评估所有传输类型。使用 **1 到 64** 范围内不同数量的 DPU。

**根据分析结果，在实验报告中回答：**

1. 绘制图表，展示不同**传输大小、类型、方向和 DPU 数量**所有组合对应的实测数据传输带宽。
2. 单个 DPU 的最大带宽是多少？64 个 DPU 的最大带宽是多少？这些数据是否符合预期？请解释。

## 4. 任务 2/4：AXPY

目标是熟悉 DPU 内核的启动与执行，并研究使用不同数量 PIM 线程时的性能扩展特征（UPMEM 架构将线程称为 **tasklet**）。本任务提供了模板，详情见第 9 节。

**任务内容：**

1. 编写 DPU 内核，对向量的每个元素执行 **AXPY** 运算（`y = y + alpha × x`）[7]。需要：(1) 将两个输入向量 `Y` 和 `X` 传入一个或多个 MRAM bank；(2) 使用数量可变的 tasklet 执行 AXPY；(3) 将结果写入输出向量 `Y`；(4) 将输出向量传回宿主机主存。
2. 按需要分配 WRAM，并使用 `mram_read` 和 `mram_write` 在 MRAM 与 WRAM 之间传输数据。
3. 无论数据类型（例如 `char`、`short`、`int`、`long long int`、`float`、`double`）或向量大小如何，代码都应产生正确结果。**提示：** 确保所有数据传输都按 **8 字节对齐** [2]。
4. 使用 **1 至 24** 之间所有可能的 tasklet 数量运行内核，并确保在所有情况下（即任意 tasklet 和 DPU 数量）结果都正确。每个 DPU 使用**至少 8 MB** 的输入向量。

**根据分析结果，在实验报告中回答：**

1. 针对不同 tasklet 数量，使用性能计数器统计**每个 tasklet 执行的指令数**（参见第 9 节）并绘图。分析结果，说明观察到的现象。

## 5. 任务 3/4：操作与数据类型

目标是分析 DPU 在不同数据类型及运算操作下的性能。

**任务内容：**

1. 修改 AXPY DPU 内核，使其执行向量加法（`y = y + x`），并且支持加法之外的其他操作，包括**减法、乘法和除法**。
2. 评估新内核在不同操作（加、减、乘、除）与不同数据类型（`char`、`short`、`int`、`long long int`、`float`、`double`）下的性能。

**根据分析结果，在实验报告中回答：**

1. 使用性能计数器（参见第 9 节），测量每种数据类型及操作对应的指令数。可以针对每种操作与数据类型计算**平均指令数**。
2. 汇报评估过程中的观察结果。
3. 建议使用 LLVM 对象文件反汇编工具（参见第 9 节）查看程序的汇编代码，结合这些信息解释观察结果。

## 6. 任务 4/4：向量归约（Vector Reduction）

目标是熟悉 DPU 内部通信（即 tasklet 之间通信）的同步原语。为此，你将编写一个对输入向量执行并行归约的 DPU 内核。

**任务内容：**

1. 向量归约内核应实现四个不同版本：(1) **单 tasklet 完成最终归约**；(2) **使用屏障（barrier）的树形最终归约**；(3) **使用握手（handshake）的树形最终归约**；(4) **使用互斥锁（mutex）的最终归约**。
2. 使用不同 tasklet 数量运行每个版本。确保在任意 tasklet 与 DPU 数量下，所有版本都能产生正确结果。统计执行的指令数。使用 **16 KB、1 MB、16 MB** 三种向量大小。

**根据分析结果，在实验报告中回答：**

1. 汇报评估观察结果。对于不同向量大小，哪个版本的性能最好？

## 7. 附加任务 1/2：在真实 UPMEM PIM 系统上评估

本任务要求在**真实的 UPMEM PIM 系统**上评估任务 1–4 的实现。之前已经使用 UPMEM 模拟器验证程序功能正确性；现在需要在真实硬件上运行代码，并分析其性能。

**任务内容：**

1. **访问真实的 UPMEM PIM 系统：**
   - 在 UPMEM 网站注册，以获得真实 UPMEM PIM 系统的访问权限。注册链接由授课教师或助教提供。
   - 每次登录最长 **2 小时**，每天最多 **4 小时**。由于有许多学生使用该系统，请查看预约日历并预订时段。
   - 请合理利用访问时长。**强烈建议**先在 UPMEM 模拟器中验证程序核心功能，再到真实系统运行，以免浪费使用时间。
2. **运行你的实现：**
   - 在真实 UPMEM PIM 系统上运行任务 1–4 的实现。
   - 确保程序在真实系统上也能够输出正确结果，与模拟器上一致。
3. **性能分析：**
   - 对内存传输进行性能分析，并记录应用程序各函数执行的持续时间，分析真实硬件与模拟器结果之间的差异。
   - 使用 UPMEM SDK 提供的 `dpu-profiling` 工具进行分析。详细工具文档见[此处](https://sdk.upmem.com/2023.2.0/261_Application_profiling.html)。

**根据分析结果，在实验报告中回答：**

1. 你的实现运行在真实 UPMEM PIM 系统上的性能与模拟器相比如何？请对每个任务作出详细观察与分析。
2. 汇报评估观察结果。对于不同数量的 DPU 和 tasklet，哪个版本的性能最好？其性能相较于模拟器如何？

如果在真实 UPMEM PIM 系统上完成所有任务并提交详细报告，本实验最多可获得 **15% 的额外分数**。

## 8. 附加任务 2/2：在真实 UPMEM PIM 系统上实现 RGB 转灰度

本任务要求在数量可变的 DPU 上，将 RGB 图像转换为灰度图像 [8]。图像的红、绿、蓝三个通道均为 **8 位**。对每个像素，使用其 `R`、`G`、`B` 值计算灰度值，并比较以下三种方法：

1. **明度法（Lightness）：**

   \[
   \mathrm{grayscale}=\frac{\min(R,G,B)+\max(R,G,B)}{2}
   \]
2. **平均值法（Average）：**

   \[
   \mathrm{grayscale}=\frac{R+G+B}{3}
   \]
3. **亮度加权法（Luminosity）：**

   \[
   \mathrm{grayscale}=0.3R+0.59G+0.11B
   \]

**根据分析结果，在实验报告中回答：**

1. 使用不同数量 tasklet 时，各种方法的可扩展性如何？
2. 哪一种方法最快？请解释。
3. 哪一种方法最慢？请解释。

请在 UPMEM 模拟器上实现并验证这些代码，然后在真实 UPMEM PIM 系统上评估性能（参见附加任务 1/2）。思考可能的代码优化，完成优化并重新评估。一个通用建议是尽量**减少执行的指令数**。

如果实现正确版本，并探索其他优化且取得成功结果，最多可获得 **15% 的额外分数**。

## 9. 实验资源

### 9.1. 源代码

提供的源代码包含任务 1（第 3 节）和任务 2（第 4 节）的模板；其余任务可以使用与任务 2 相同的模板。模板位于 `template` 文件夹。搜索 `//@@` 可以找到需要补充代码的位置。

**除非下列清单明确允许，否则不要修改其他文件或文件夹。**

```text
task1/
├── Makefile
├── host/
│   └── app.c               # 宿主 CPU 代码（允许修改）
├── dpu/
│   └── task.c              # DPU 内核代码；任务 1 无需使用，故模板中为空
└── support/
    ├── common.h            # 通用定义；此任务中 T 为 int64_t
    ├── params.h            # 从命令行读取输入参数的函数
    └── timer.h             # 计时函数

task2/
├── Makefile
├── host/
│   └── app.c               # 宿主 CPU 代码（允许修改）
├── dpu/
│   └── task.c              # DPU 内核代码（允许修改）
└── support/
    ├── common.h            # 通用定义；包括各种数据类型及 MRAM 与 WRAM
    │                       # 之间数据传输大小的定义
    ├── params.h            # 从命令行读取输入参数的函数
    ├── timer.h             # 计时函数
    └── cyclecount.h        # 性能计数器函数（周期与指令数）
```

有关性能计数器的更多信息，参阅 SDK 用户手册 [2] 中的 **“Measuring performances”** 小节。

### 9.2. 编译

**任务 1。** 任务 1 的 `Makefile` 包含以下输入参数与默认值：

- `NR_DPUS ?= 1`：程序使用的 DPU 数量。
- `NR_TASKLETS ?= 16`：每个 DPU 使用的 tasklet 数量。
- `TRANSFER ?= PARALLEL`：数据传输类型（`SERIAL`、`PARALLEL`、`BROADCAST`）。
- `PRINT ?= 0`：是否打印 DPU 内核的日志。

对于任务 1，只需要使用 `TRANSFER` 和 `NR_DPUS`。使用默认参数编译：

```bash
make
```

例如，使用串行传输和 64 个 DPU 编译：

```bash
NR_DPUS=64 TRANSFER=SERIAL make
```

编译生成的二进制文件位于 `bin` 文件夹。用默认输入运行程序：

```bash
./bin/host_code
```

使用 `-h` 查看可用的输入参数：

```bash
./bin/host_code -h
```

例如，下面的命令将在宿主机主存和 MRAM bank 之间双向传输 **2 MB** 数据（即 **262144 个 64 位元素**）；先预热 2 次，再重复实验 10 次：

```bash
./bin/host_code -w 2 -e 10 -i 262144
```

**任务 2 及其余任务。** 任务 2 的 `Makefile` 包含以下输入参数与默认值：

- `NR_DPUS ?= 1`：程序使用的 DPU 数量。
- `NR_TASKLETS ?= 16`：每个 DPU 使用的 tasklet 数量。
- `BLOCK ?= 10`：MRAM 与 WRAM 之间搬运的数据块（chunk）大小；实际字节数为 \(2^{\mathrm{BLOCK}}\)。
- `TYPE ?= INT32`：输入数组的数据类型（`CHAR`、`SHORT`、`INT32`、`INT64`、`FLOAT`、`DOUBLE`）。
- `TRANSFER ?= PARALLEL`：传输类型（`SERIAL`、`PARALLEL`）。
- `PRINT ?= 0`：是否打印 DPU 内核日志。
- `PERF ?= NO`：是否使用性能计数器统计周期或指令数（`CYCLES`、`INSTRUCTIONS`）。

使用默认参数编译：

```bash
make
```

例如，以并行传输方式使用 **64 个 DPU**、每个 DPU **12 个 tasklet**、**32 位浮点数**，每次在 MRAM 与 WRAM 之间双向传输 **512 字节**，并统计 tasklet 执行的指令数：

```bash
NR_DPUS=64 NR_TASKLETS=12 BLOCK=9 TYPE=FLOAT TRANSFER=PARALLEL PERF=INSTRUCTIONS make
```

与任务 1 一样，编译后的二进制文件位于 `bin` 文件夹。例如，运行 AXPY 程序，对 **4 MB** 的数组（即 **1048576 个 32 位浮点元素**）执行运算，取 `alpha=20`，预热 2 次后重复实验 10 次：

```bash
./bin/host_code -w 2 -e 10 -i 1048576 -a 20
```

### 9.3. LLVM 对象文件反汇编工具

可以使用此工具显示对象文件内容 [9]。`-S` 选项能够将源代码与反汇编代码交错显示：

```bash
llvm-objdump -S ./bin/dpu_code
```

另一个用途类似的工具是 [Compiler Explorer](https://dpu.dev) [10]。

## 10. 建议

- **请勿传播提供的程序文件。** 这些文件仅供计算机体系结构课程学生个人使用；分发和共享将侵犯所提供软件的版权。
- 仔细阅读实验说明。
- 阅读 UPMEM 编程指南 [2]。
- 如有需要，通过 Moodle 在线问答论坛向助教提问。
- 遇到技术问题（例如编译错误）时，请首先阅读错误消息。

## 11. 提交要求

通过 Moodle 的[对应作业入口](https://moodle-app2.let.ethz.ch/)提交。需要提交：

- 编译代码所需的**全部文件**（包括没有修改过的文件）。
- **每项任务一个文件夹**，与提供的模板具有相同的组织结构。
- **一份 PDF 格式的报告**，包含本实验说明要求回答的问题。
- **不要提交编译产物**（例如二进制文件）。

将上述内容打包为**一个 tar 归档文件**，命名为 `lab3_YourSurname_YourName.tar.gz`。

## 参考文献

[1] UPMEM. *UPMEM Software Development Kit (SDK)*. [https://sdk.upmem.com](https://sdk.upmem.com), 2021.

[2] UPMEM. *UPMEM User Manual*. [https://sdk.upmem.com/2024.1.0/](https://sdk.upmem.com/2024.1.0/), 2024.

[3] Juan Gómez-Luna, Izzat El Hajj, Ivan Fernández, Christina Giannoula, Geraldo F. Oliveira, and Onur Mutlu. *Benchmarking a New Paradigm: An Experimental Analysis of a Real Processing-in-Memory Architecture*. arXiv:2105.03814 [cs.AR], 2021.

[4] Juan Gómez-Luna, Izzat El Hajj, Ivan Fernandez, Christina Giannoula, Geraldo F. Oliveira, and Onur Mutlu. *Benchmarking a New Paradigm: Experimental Analysis and Characterization of a Real Processing-in-Memory System*. IEEE Access, 2022.

[5] Docker Inc. *Docker*. [https://www.docker.com](https://www.docker.com), 2023.

[6] Podman. *Podman*. [https://podman.io](https://podman.io), 2023.

[7] Wikipedia. *Basic Linear Algebra Subprograms: Level 1*. [条目链接](https://en.wikipedia.org/wiki/Basic_Linear_Algebra_Subprograms#Level_1), 2022.

[8] Wikipedia. *Grayscale*. [条目链接](https://en.wikipedia.org/wiki/Grayscale), 2022.

[9] LLVM. *llvm-objdump — LLVM's Object File Dumper*. [文档链接](https://llvm.org/docs/CommandGuide/llvm-objdump.html), 2022.

[10] Compiler Explorer. *Compiler Explorer for DPU*. [https://dpu.dev](https://dpu.dev), 2023.
