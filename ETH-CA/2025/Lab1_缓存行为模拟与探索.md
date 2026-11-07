# 实验 1：缓存行为的模拟与探索

**课程：** 计算机体系结构（Computer Architecture，227-2210-00L），2025 年秋季学期
**授课教师：** Prof. Onur Mutlu、Dr. Mohammad Sadrosadati
**助教：** Dr. Konstantina Koliogeorgi、Dr. Zhiheng Yue、Rahul Bera、Konstantinos Kanellopoulos、Nika Mansouri Ghiasi、Ataberk Olgun、Nisa Bostanci、Rakesh Nadig、İsmail Emir Yüksel、Haocong Luo、Mayank Kabra、Andreas Kosmas Kakolyris、Harshita Gupta、Konstantinos Sgouras、Harsh Songara、Susana Rebolledo Ruiz

**布置日期：** 2025 年 9 月 22 日（星期一）
**截止日期：** 2025 年 10 月 10 日（星期五）午夜

## 1. 引言

本实验中，你将扩展一个使用 C 语言编写的时序模拟器（timing simulator），以对指令缓存和数据缓存进行建模。它与你在“数字设计与计算机体系结构”课程中使用的 RTL 不同：时序模拟器并不是处理器的直接实现，也不能用于逻辑综合，而是一个更高层次的抽象模型，旨在支持快速的体系结构探索。在更高抽象层次上描述和模拟硬件，可以帮助设计人员迅速判断不同设计选择（例如缓存或分支预测器）对性能的影响。

我们将提供一个用于模拟简单 MIPS 处理器的基础模拟器，同时完整规定缓存的行为。你的任务是按照这些规格扩展模拟器，使其实现所要求的缓存。

## 2. 时序模拟器

对于基于 C 语言的时序模拟器，我们不受逻辑级实现的约束。主要目标是计算程序在被模拟处理器上执行所需的时钟周期数。因此，可以进行许多简化：

- 不必真正对处理器各模块中的控制逻辑与数据通路细节进行建模。
- 只需要为每个流水线阶段编写实现其较高层功能的代码（例如读取寄存器文件、访问内存等）。
- 通常来说，模拟器所用的算法和数据结构不必与真实处理器完全一致，只要结果相同即可。

虽然不再采用底层实现，因此无法确定关键路径（或其他关心的成本指标，例如芯片面积、运行功耗），但如果周期级模型足够准确，我们仍然能够知道程序需要执行多少个周期。

## 3. 任务 1/2：扩展基准时序模拟器

目标是根据下述规格实现时序模拟器，使它能够精确模拟配备指令缓存和数据缓存的 MIPS 机器。下面将完整规定待模拟 MIPS 机器的微体系结构。

### 3.1. 指令缓存（Instruction Cache）

取指阶段每个周期都会访问指令缓存。

**组织结构。** 指令缓存容量为 **8 KB**，采用 **4 路组相联**，缓存块大小为 **32 字节**（因此共有 **64 个组**）。访问缓存时，使用 `PC[10:5]` 作为组索引。

**缺失时序。** 当取指阶段发生指令缓存缺失时，必须从主存取回对应缓存块。主存访问需要 **50 个周期**。在第 50 个周期，将新块插入缓存。总之，一次指令缓存缺失会使流水线停顿 **50 个周期**。

**替换策略。** 从主存取回新块后，将其插入指令缓存中对应的组。如果该组有空的路（即无效路），则直接插入该无效路；如果所有路均被占用，则用新块替换该组中 **最近最少使用（LRU）**的块。两种情况下，新插入的块都成为**最近使用（MRU）** 的块。

### 3.2. 数据缓存（Data Cache）

每当加载（load）或存储（store）指令位于访存阶段时，便会访问数据缓存。

**组织结构。** 数据缓存容量为 **64 KB**，采用 **8 路组相联**，缓存块大小为 **32 字节**（因此共有 **256 个组**）。访问时，使用加载/存储数据地址的位 `[12:5]` 作为组索引。

**缺失时序与替换。** 数据缓存的缺失时序和替换行为与指令缓存完全相同。

**存储指令处理。** 加载缺失和存储缺失都会使流水线停顿 **50 个周期**。两种缺失都需要从主存取回一个新缓存块并插入缓存。

**脏块驱逐（Dirty Evictions）。** 当一个“脏”块被来自主存的新块替换时，必须将脏块写回主存。本实验假定脏块驱逐能够**瞬时完成**：即在新块插入缓存的同一个周期，脏块立即被写回主存。

### 3.3. 关于指令缓存和数据缓存的假设

- 假设两种缓存初始均为空（所有块均无效）。
- 两种缓存中的每个块都有一个独立的标签（tag），用于保存块的有关信息，例如地址、有效位、最近使用情况等。标签初始值均为 `0`。
- 假设处理器所运行的程序绝不会修改自身的代码（即**自修改代码**）；同一个块不可能同时驻留在这两种缓存中。

## 4. 任务 2/2：缓存设计探索

第二个任务是评估任务 1 所实现缓存的性能特征。需要研究以下设计因素：

1. **缓存容量、块大小、相联度：缓存参数扫描。** 编写一组使用大量内存的基准测试（例如按照流式或随机模式访问大型数组），并运行模拟器，测量不同缓存参数对应的 IPC。展示相联度、块大小和缓存容量变化对性能的影响。
2. **替换与插入策略：探索缓存替换和插入策略。** 缓存替换策略决定插入新块时，应该替换组中的哪个缓存块；缓存插入策略决定新块插入到块使用顺序列表的哪个位置。迄今为止，我们使用的替换策略会驱逐最近最少使用的块，插入策略会将新块置于最近使用的位置。不过，已有其他替换和插入策略被研究，并且其中部分策略对于特定访问模式可取得明显更好的性能（更少的缓存缺失）[1, 2]。请使用多种测试程序进行实验，优化缓存替换/插入策略。
3. **其他方向（额外加分）。** 也可以自行探索缓存的其他方面。例如，使用更复杂的哈希函数将缓存块映射到缓存组，或同时使用多个哈希函数 [3]；另一种选择是实现**牺牲缓存（victim cache）**[4]。由于此部分没有固定范围，授课教师保留决定具体额外加分的权利；根据优化的难度以及最终设计与实现质量，最多可获得 **25%** 的额外分数。

注意，性能评估的一项重要工作是与**合理的基准设计**比较。例如，如果基准模型没有模拟主存延迟，而你的设计模拟了主存延迟，那么比较时钟周期数就没有意义。对于每项被评估的设计，都需要自行讨论并选取合适的基准进行比较。

请撰写报告 `report_cache.pdf`，总结：

1. 对每项缓存参数所产生影响的观察。
2. 关于缓存替换/插入策略的研究发现。
3. 你实现的其他优化。

报告不必超过四页，但也可以使用更多页来展示示意图、数据和图表。此外，还需要提交实现性能最佳缓存设计的模拟器版本（`src/`）。该模拟器版本和报告应放在同一个提交归档包中的独立文件夹内（完整提交清单见第 7 节）。

## 5. 实验资源

### 5.1. 源代码

**除非下列清单明确允许，否则不要修改任何文件或文件夹。**

- `Makefile`
- `run.py`：运行模拟器并将结果与基准模拟器比较的脚本。为了调试，你可能需要修改此脚本；但最终提交必须能够与**未经修改**的版本配合工作。
- `src/`：源代码（**允许修改**，也可添加更多文件）
  - `pipe.c`：模拟器（允许修改）
  - `pipe.h`：模拟器（允许修改）
  - `mips.h`：MIPS 相关宏定义
  - `shell.c`：模拟器的交互式 shell
  - `shell.h`：模拟器的交互式 shell
- `inputs/`：模拟器的示例测试输入（**允许修改**，也可添加更多文件）

### 5.2. Makefile

我们提供了 `Makefile`，用于自动编译和验证模拟器。第一次使用它时，应先编译基准模拟器：

```bash
make basesim
```

这将生成 `basesim`，即与提供的源代码对应的基准模拟器。你可以用它来验证自己模拟器运行程序时的输出。注意：程序在你的模拟器中产生的输出，应始终与在基准模拟器中运行时得到的输出一致；但在修改缓存后，两种模拟器中的程序执行时间将不再相同。

编译你的模拟器：

```bash
make
```

使用一个或多个测试输入编译模拟器，并与基准模拟器比较：

```bash
make run INPUT=inputs/inst/addiu.x
make run INPUT=inputs/inst/*.x
make run
```

为调试代码，可以修改 `Makefile` 或用于比较 `sim` 与 `basesim` 输出的 Python 脚本 `run.py`。但是，最终提交版本必须包含**原始的** `Makefile` 和 `run.py`。

## 6. 入门指导与建议

### 6.1. 目标

我们提供用于模拟五级 MIPS 流水线的时序模拟器框架：`pipe.c` 和 `pipe.h`。目前该模拟器在体系结构功能上已经是正确的：只要程序只使用已经实现的指令，它就能正确执行任意 MIPS 程序。[^lab1-1] 检测到数据依赖时，模拟器会通过停顿和/或旁路（bypassing）正确处理；检测到控制依赖时，也会按需要停顿流水线。

执行下面的命令，可以看到对于 `inputs/` 中提供的所有测试输入，你的模拟器 `sim` 与基准模拟器 `basesim` 具有完全相同的体系结构输出（例如寄存器值）：

```bash
make run
```

你的工作是准确模拟缓存与主存对时序的影响。

### 6.2. 研究时序模拟器

请详细研究 `pipe.c` 和 `pipe.h`。

模拟器将每个流水线阶段建模为独立函数，例如 `pipe_stage_fetch()`。流水线状态由一组指向 `Pipe_Op` 结构的指针表示（该结构定义在 `pipe.h` 中）。每个 `Pipe_Op` 保存模拟器所需的、关于流水线中某条指令的全部信息。取指时会分配一个 `Pipe_Op`，随后它在流水线中向后传递，最终到达最后一级（写回阶段）；指令完成后，其结构被释放。更具体地说，每一级都从前一级接收 `Pipe_Op`，对其进行处理，再传给下一级。模拟器通过停止 `Pipe_Op` 结构的流动来模拟流水线停顿，通过释放不同流水线级中的 `Pipe_Op` 来模拟流水线冲刷（flush）。

### 6.3. 建议

- **请勿传播提供的程序文件。** 这些文件仅供计算机体系结构课程的每位学生个人使用；分发或分享将侵犯所提供软件的版权。
- 仔细阅读本实验说明。
- 如有需要，请通过 Moodle 中对应的[在线问答论坛](https://moodle-app2.let.ethz.ch/mod/moodleoverflow/view.php?id=1282940)向助教提问。
- 遇到技术问题时，请首先阅读错误消息。简单的网络搜索通常可以解决许多调试问题和报错。
- 使用调试工具。如果还不熟悉 GDB/LLDB，请学习它们。定位代码问题时，没有什么能代替逐步执行代码并检查中间值。
- 一种合理的实现路径是先写出通用的组相联缓存，再分别将其接入取指阶段（指令缓存）和访存阶段（数据缓存）。
- 编写更多测试用例。可以使用 MARS Simulator 编写并汇编 MIPS-32 测试程序。模拟器可从[此链接下载](https://safari.ethz.ch/digitaltechnik/spring2023/lib/exe/fetch.php?media=lab7_files.zip)。

### 6.4. 背景知识与参考资料

如果对本作业涉及的主题不太熟悉，可以阅读以下参考材料：

- [2025 年春季“数字设计与计算机体系结构”课程](https://safari.ethz.ch/ddca/spring2025/doku.php?id=schedule)。相关课程包括：
  - **L7**：冯·诺依曼模型与指令集体系结构（[讲义](https://safari.ethz.ch/ddca/spring2025/lib/exe/fetch.php?media=onur-ddca-2025-lecture7-vonneumann-isa-afterlecture.pdf)、[视频](http://www.youtube.com/watch?v=T0Ka9QG9t-o)）
  - **L8**：指令集体系结构 II（[讲义](https://safari.ethz.ch/ddca/spring2025/lib/exe/fetch.php?media=onur-ddca-2025-lecture8-isa-ii-afterlecture.pdf)、[视频](http://www.youtube.com/watch?v=o2YvlUCv9d4)）
  - **L9a**：ISA 与微体系结构（[讲义](https://safari.ethz.ch/ddca/spring2025/lib/exe/fetch.php?media=onur-ddca-2025-lecture9a-isa-microarchitecture-afterlecture.pdf)、[视频](http://www.youtube.com/watch?v=l8KpCtprJpc)）
  - **L9b**：微体系结构基础与设计（[讲义](https://safari.ethz.ch/ddca/spring2025/lib/exe/fetch.php?media=onur-ddca-2025-lecture9b-microarchitecture-fundamentals-design-afterlecture.pdf)、[视频](http://www.youtube.com/watch?v=l8KpCtprJpc)）
  - **L11**：多周期与流水线处理器设计（[讲义](https://safari.ethz.ch/ddca/spring2025/lib/exe/fetch.php?media=onur-ddca-2025-lecture11-multicycle-pipelined-design-afterlecture.pdf)、[视频](http://www.youtube.com/watch?v=7EiYH010rf4)）
  - **L12**：流水线处理器设计 II（[讲义](https://safari.ethz.ch/ddca/spring2025/lib/exe/fetch.php?media=onur-ddca-2025-lecture12-pipelined-design-ii-afterlecture.pdf)、[视频](http://www.youtube.com/watch?v=EkLmDhThlQc)）
  - **L21**：内存组织、技术、层次结构与缓存（[讲义](https://safari.ethz.ch/ddca/spring2025/lib/exe/fetch.php?media=onur-ddca-2025-lecture21-memory-organization-hierarchy-caches-afterlecture.pdf)、[视频](http://www.youtube.com/watch?v=wTz-d6XiB_Y)）
  - **L22**：缓存（[讲义](https://safari.ethz.ch/ddca/spring2025/lib/exe/fetch.php?media=onur-ddca-2025-lecture22-caches-afterlecture.pdf)、[视频](http://www.youtube.com/watch?v=EjjbMfHPkbg)）
  - **L23a**：缓存 II（[讲义](https://safari.ethz.ch/ddca/spring2025/lib/exe/fetch.php?media=rahul-ddca-2025-lecture23a-caches-ii-afterlecture.pdf)、[视频](http://www.youtube.com/watch?v=eQiN4tetvHA)）
  - **L23b**：多核缓存问题（[讲义](https://safari.ethz.ch/ddca/spring2025/lib/exe/fetch.php?media=onur-ddca-2025-lecture23b-mc-issues-in-caching-afterlecture.pdf)、[视频](http://www.youtube.com/watch?v=ohdoVff7vvY)）
- **Harris 与 Harris 教材** [5]
  - 第 7 章：微体系结构
  - 第 8.3 节：缓存

## 7. 提交要求

通过 Moodle 中的[对应作业入口](https://moodle-app2.let.ethz.ch/mod/assign/view.php?id=1282939)提交。将编译和模拟代码所需的全部文件打包为**一个 tar 归档文件**，名称为 `lab1_YourSurname_YourName.tar.gz`。请在模拟器代码中添加注释，说明自己的实现。

提交归档包的目录结构应如下：

```text
lab1/
├── README                          # 可选：需要告知助教的特殊说明
├── task_1/                         # 缓存实现的源代码
│   ├── Makefile                    # 未修改
│   ├── run.py                      # 未修改
│   ├── src/
│   └── inputs/
└── task_2/                         # 缓存探索的源代码和报告
    ├── report_cache.pdf
    └── most_performant_design/
        ├── Makefile                # 未修改
        ├── run.py                  # 未修改
        ├── src/
        └── inputs/                 # 还应包含自行编写的内存性能基准测试
```

欢迎附上一份简短的 `README`，说明构建/运行项目时任何可能影响评分的细节。如果想展示报告中讨论的某项设计内容，也可以在 `task_2/` 中的 `most_performant_design/` 目录旁提交额外的源代码。

## 参考文献

[1] M. K. Qureshi et al. *Adaptive Insertion Policies for High Performance Caching*. ISCA, 2007.

[2] V. Seshadri et al. *The Evicted-Address Filter: A Unified Mechanism to Address Both Cache Pollution and Thrashing*. PACT, 2012.

[3] A. Seznec. *A Case for Two-Way Skewed-Associative Cache*. ISCA, 1993.

[4] N. P. Jouppi. *Improving Direct-Mapped Cache Performance by the Addition of a Small Fully-Associative Cache and Prefetch Buffers*. ISCA, 1990.

[5] David Harris and Sarah Harris. *Digital Design and Computer Architecture (Second Edition)*. Morgan Kaufmann, 2013.

[^lab1-1]: 严格来说并非完全如此，因为对系统调用、异常等仍有通常的限制。
