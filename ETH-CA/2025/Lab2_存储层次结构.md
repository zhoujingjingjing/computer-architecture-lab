# 实验 2：存储层次结构

**课程：** 计算机体系结构（Computer Architecture，227-2210-00L），2025 年秋季学期
**授课教师：** Prof. Onur Mutlu、Dr. Mohammad Sadrosadati
**助教：** Dr. Konstantina Koliogeorgi、Dr. Zhiheng Yue、Rahul Bera、Konstantinos Kanellopoulos、Nika Mansouri Ghiasi、Ataberk Olgun、Nisa Bostanci、Rakesh Nadig、Ismail Emir Yuksel、Haocong Luo、Mayank Kabra、Andreas Kosmas Kakolyris、Harshita Gupta、Konstantinos Marios Sgouras、Harsh Songara、Susana Rebolledo Ruiz
**负责答疑的助教：** Andreas Kosmas Kakolyris、Rakesh Nadig

**布置日期：** 2025 年 10 月 10 日（星期五）
**截止日期：** 2025 年 10 月 24 日（星期五）午夜

## 1. 引言

本实验将在实验 1 的基础上继续扩展，实现包含 **L2 缓存**和基于 **DRAM 的主存**的存储层次结构。与实验 1 一样，我们将完整规定 L2 缓存和 DRAM 的逐周期行为。

L1 缓存与处理器流水线紧密集成，而 L2 缓存与流水线之间的耦合相对较弱。本实验将保留实验 1 中 L1 缓存的行为语义，但把固定的 **50 周期 L1 缓存缺失延迟**替换为可变延迟：至少访问 L2 缓存，也可能进一步访问 DRAM。在实验 1 中，每当处理器原本要开始一次持续 50 周期的 L1 缓存缺失停顿时，现在则向存储层次结构发送请求。当数据从 L2 或主存变得可用时（具体行为见下文），存储层次结构向 L1 缓存发送**缓存填充通知**。如果 L1 缓存仍因等待缺失数据而停顿（即该请求未被取消），就将数据插入缓存，并在下一个周期结束停顿。下面进一步明确存储层次结构的组成与请求处理方式。

## 2. 任务：扩展模拟器，加入 L2 缓存和 DRAM

### 2.1. 统一 L2 缓存

无论 L1 指令缓存还是 L1 数据缓存发生缺失，都要访问统一 L2 缓存。

**组织结构。** L2 缓存容量 **256 KB**，采用 **16 路组相联**，缓存块大小 **32 字节**（因此共有 **512 个组**，需要 **9 个组索引位**）。访问 L2 缓存时，组索引为地址的位 `[13:5]`。

**MSHR。** L2 缓存包含 **16 个 MSHR**（Miss-Status Holding Register，缺失状态保持寄存器），对于本实验简单的顺序执行流水线而言绰绰有余。每次 L2 缓存缺失，L2 都分配一个 MSHR 来跟踪该缺失。每个 MSHR 包含三个字段：(i) 有效位（valid），(ii) 引发缺失的缓存块地址，以及 (iii) 完成位（done）。可以假设所有位的初始值都是 `0`。当主存完成某次 L2 缓存缺失请求时，会向 L2 缓存发送填充通知，将对应 MSHR 的 `done` 位置位（即当 `done=1` 时，该 MSHR 条目失效，`valid=0`）。有效位可以用来表示条目中存有一个有效的内存地址（而非例如 MSHR 中的初始地址）。

**访问 L2 缓存。** 假设 L2 缓存拥有大量端口，因而其访问绝不会被串行化。任何一次内存访问发生 L1 缓存缺失时，**在同一周期**立即探测（probe）[^lab2-1] L2 缓存。根据该访问在 L2 是命中还是缺失，分别处理如下：

- **L2 缓存命中。** **15 个周期后**，L2 缓存向相应的 L1 缓存（指令缓存或数据缓存）发送填充通知。例如，一次访问在第 0 周期发生 L1 缺失但 L2 命中，则 L1 在第 **15** 周期收到填充通知。
- **L2 缓存缺失。** L2 立即为该缺失分配 MSHR。**5 个周期后**，L2 向主存发送内存请求（用于模拟 L2 与内存控制器之间的延迟）。主存完成请求后，L2 收到填充通知。再过 **5 个周期**，将取回的缓存块插入 L2，并释放对应 MSHR（用于模拟内存控制器到 L2 的延迟）。在**同一周期**，如果流水线正因该缓存块而停顿，L2 还会向 L1 发送填充通知，并同时将该缓存块插入 L1。**下一周期**，流水线解除停顿。

**管理策略。** 新缓存块插入 L2 时置于**最近使用位置（MRU）**。L2 命中时，被访问块提升到 MRU 位置。同一周期最多可能发生两次 L2 命中：一次来自取指阶段，一次来自访存阶段。如果命中的两个不同缓存块属于同一个组，则需要确定哪个块最终处于 MRU 位置。这里假定访存阶段先于取指阶段访问 L2：因此取指阶段请求的块提升到 **MRU**，访存阶段请求的块置于 **MRU-1**。L2 使用真正的 **LRU 替换策略**。

**其他规定。**

- 假设 L1 和 L2 缓存初始都为空。
- 假设程序不会修改自身代码（自修改代码）：一个给定缓存块不会同时驻留于两个 L1 缓存（指令缓存和数据缓存）。
- 与 L1 缓存访问不同，**L2 缓存访问一旦开始，就无法取消**。
- 和实验 1 类似，我们不模拟 L1 数据缓存向 L2、以及 L2 向主存驱逐脏块时发生的实际写回过程。当插入新缓存块而驱逐一个脏块时，假定该脏块被**瞬时**写入紧邻的下一层存储。此操作没有任何副作用：不会影响对应缓存组内的 LRU 顺序；在 DRAM 中不会占用总线或存储体（bank），也不会改变行缓冲区状态。
- 当 L2 缓存没有空闲 MSHR 时（在本实验简单的顺序流水线中不应发生），L1 缓存缺失**甚至不能探测** L2。换言之，访问 L2 的前提是存在空闲 MSHR，**无论此次访问最终命中还是缺失**。一个释放的 MSHR 可以在**同一周期**分配给另一次 L2 缺失。
- 一旦在 MEM 阶段发生缓存缺失（数据或指令），允许你停顿流水线，直到 L2（无论命中或缺失）或主存完成请求。不过，如果同时存在一次指令缓存缺失，以及一条已经处于 MEM 阶段的较老指令引起的数据缓存缺失，则可以分别为两次缺失分配一个 MSHR 来处理它们。
- 在理想设计中，当某条指令因 L1 缓存缺失而停顿时，如果 (1) 其他指令与被停顿指令不存在数据依赖，并且 (2) 有可分配的 MSHR，流水线应该仍能处理其他指令。**但是，实现这种设计并不是获得满分的必要条件**（即可以按照上一项所述停顿流水线）。

### 2.2. 主存（DRAM）

**组织结构。** DRAM 系统具有一个通道（channel）和一个 rank，通道包含命令总线、地址总线和数据总线。该 rank 包含 **8 个 bank**；每个 bank 包含 **64K 行**，每行大小 **8 KB**。访问主存时，bank 索引取地址的 `[7:5]` 位，行索引取 `[31:16]` 位。例如，地址 `0x00000000` 对应的 32 字节缓存块存储在第 0 个 bank 的第 0 行；地址 `0x00000020` 对应的 32 字节缓存块则存储在第 1 个 bank 的第 0 行。

**内存控制器。** 内存控制器使用一个请求队列保存从 L2 收到的内存请求。该队列可以容纳无限数量的请求。每个周期，控制器扫描请求队列（**包括当周期新到达的请求**），寻找“可调度”（schedulable，下文定义）的请求。如果只有一个可调度请求，就对该请求发起 DRAM 访问。如果存在多个，则按以下顺序确定优先级，类似 **FR-FCFS** 策略：

1. 优先选择**行缓冲区命中**的请求。
2. 优先选择**更早到达**的请求。
3. 优先选择来自**访存阶段**的请求。

**DRAM 命令与时序。** 一次 DRAM 访问由发送给某个 bank 的一系列命令组成。有四种 DRAM 命令：`ACTIVATE`（携带 bank/行地址）、`READ` / `WRITE`（携带列地址）和 `PRECHARGE`（携带 bank 地址）。每条命令同时占用命令总线与地址总线 **4 个周期**。一个 DRAM bank 收到命令后会进入 **100 个周期**的“忙碌”状态，在此期间不能接收其他命令。收到 `READ` 或 `WRITE` 命令后 **100 个周期**，bank 才能通过数据总线发送或接收一段 **32 字节**数据；数据传输会占用数据总线 **50 个周期**。

**行缓冲区状态。** 每个 DRAM bank 都有一个行缓冲区。处理针对该 bank 的请求时，所需命令序列由行缓冲区状态决定。例如，假设一个请求需要访问第 0 个 bank 的第 0 行，则可能出现三种情况：

- **行缓冲区命中（Row-Buffer Hit）：** 第 0 行已经位于行缓冲区。
- **行缓冲区缺失（Row-Buffer Miss）：** 对应 bank 的行缓冲区中没有装载任何行。
- **行缓冲区冲突（Row-Buffer Conflict）：** 行缓冲区中装载的是与第 0 行不同的另一行。

下表总结三种情况各自需要执行的命令序列：

| 情况         | 命令序列                                          |
| ------------ | ------------------------------------------------- |
| 行缓冲区命中 | `READ` / `WRITE`                              |
| 行缓冲区缺失 | `ACTIVATE`、`READ` / `WRITE`                |
| 行缓冲区冲突 | `PRECHARGE`、`ACTIVATE`、`READ` / `WRITE` |

**“可调度”请求。** 如果某个内存请求的所有命令都能在各自应发出命令、传输数据的周期执行，且与命令/地址/数据总线或 bank 的其他占用**不发生冲突**，则该请求被定义为“可调度”。例如，在第 **0** 周期，针对行缓冲区已关闭的 bank 的请求，**当且仅当**以下条件全部满足时才可调度：

1. 命令总线和地址总线在周期 `0、1、2、3、100、101、102、103` 空闲。
2. 数据总线在周期 `200–249` 空闲。
3. 该 bank 在周期 `0–99` 和 `100–199` 空闲。

**其他假设与策略。**

- 假设所有 bank 的行缓冲区初始均处于关闭状态，即其中都没有装载任何行。
- 内存控制器采用**开放行策略（open row policy）**：一行打开后不会自动关闭，只有另一个已调度请求需要访问同一 bank 的其他行时才会关闭它。

## 3. 额外加分

如果考虑缓存写回和内存控制器刷新，设计并实现更加真实的系统，本实验最多可获得 **25% 的额外分数**。获得额外加分的前提是**实验 2 的基本实现完全正确**。

可以在模拟器中加入写回与刷新支持。支持写回的模拟器应模拟从 L1 缓存向 L2 缓存写回脏块，以及从 L2 缓存向主存写回脏块。必须保证这些请求像其他请求一样占用资源并消耗时间。

支持刷新的模拟器应模拟对每一条 DRAM 行进行周期性刷新。刷新由内存控制器发起，刷新频率应可配置，例如每隔 `N` 个周期刷新某一行。必须准确模拟刷新与其他请求之间的资源竞争，并确保刷新请求占用内存控制器请求队列中的条目、消耗相应时间。

如希望探索更先进的机制，可参考文献 [1, 2]。

请撰写 `report_writeback_refresh.pdf` 报告，简要总结：(i) 实现的写回策略和刷新机制；(ii) 与不考虑真实写回、刷新过程的模拟器相比，观察到的性能及缓存命中/缺失率变化；(iii) 实现的其他优化。报告不必超过四页，但欢迎使用更多页面展示示意图、数据和图表。

## 4. 实验资源

### 4.1. 源代码

本实验提供的源代码与实验 1 完全相同。可以选择基于原始框架从头开始，也可以沿用自己在实验 1 中已加入指令缓存/数据缓存的模拟器。如果继续使用上一次的模拟器，可以跳过本节剩余部分；如果从头开始，则需要按照实验 1 的说明实现 L1 指令缓存和数据缓存。

**除非下面的清单明确允许，否则不要修改文件或文件夹。**

- `Makefile`
- `run.py`：运行模拟器，并与基准模拟器比较的脚本。调试时可能需要修改，但最终提交必须能够与**原始版本**配合使用。
- `src/`：源代码（允许修改，也可增加文件）
  - `pipe.c`：模拟器（允许修改）
  - `pipe.h`：模拟器（允许修改）
  - `mips.h`：MIPS 相关宏定义
  - `shell.c`：模拟器交互式 shell
  - `shell.h`：模拟器交互式 shell
- `inputs/`：示例测试输入（允许修改，也可增加文件）

### 4.2. Makefile

我们提供自动完成编译和验证的 `Makefile`。首次使用时，应先编译基准模拟器：

```bash
make basesim
```

该命令生成与提供代码对应的 `basesim`。可以用它验证自己模拟器执行程序产生的输出。**程序输出必须始终与基准模拟器一致**，但在加入 L2 缓存和 DRAM 后，两种模拟器的程序执行时间将有所不同。

编译自己的模拟器：

```bash
make
```

使用一个或多个测试输入编译、运行模拟器，并与基准模拟器比较：

```bash
make run INPUT=inputs/inst/addiu.x
make run INPUT=inputs/inst/*.x
make run
```

## 5. 入门指导与建议

### 5.1. 推荐学习的课程资料

为了开始本实验，我们强烈建议学习[“数字设计与计算机体系结构”（2023 年春季）课程](https://safari.ethz.ch/digitaltechnik/spring2023/doku.php?id=schedule)中的相关内容：

- **L21：** 存储器组织与存储器技术（PDF、PPT、视频；见课程网站）
- **L22：** 存储层次结构与缓存（[PDF](https://safari.ethz.ch/digitaltechnik/spring2023/lib/exe/fetch.php?media=onur-ddca-2023-lecture22-memory-hierarchy-and-caches-afterlecture.pdf)、[PPT](https://safari.ethz.ch/digitaltechnik/spring2023/lib/exe/fetch.php?media=onur-ddca-2023-lecture22-memory-hierarchy-and-caches-afterlecture.pptx)、[视频](http://www.youtube.com/watch?v=1zXDoDDkICQ)）
- **L23：** 缓存设计与管理（[PDF](https://safari.ethz.ch/digitaltechnik/spring2023/lib/exe/fetch.php?media=onur-ddca-2023-lecture23-cache-design-and-management-afterlecture.pdf)、[PPT](https://safari.ethz.ch/digitaltechnik/spring2023/lib/exe/fetch.php?media=onur-ddca-2023-lecture23-cache-design-and-management-afterlecture.pptx)、[视频](http://www.youtube.com/watch?v=b7y3rN-NX5E)）
- **L24a：** 高级缓存技术（[PDF](https://safari.ethz.ch/digitaltechnik/spring2023/lib/exe/fetch.php?media=onur-ddca-2023-lecture24a-advanced-caching-afterlecture.pdf)、[PPT](https://safari.ethz.ch/digitaltechnik/spring2023/lib/exe/fetch.php?media=onur-ddca-2023-lecture24a-advanced-caching-afterlecture.pptx)、[视频](http://www.youtube.com/watch?v=cLTCEOrB428)）

### 5.2. 目标

如果已经熟悉实验 1 提供的基准模拟器，可以跳过本节。

我们提供了模拟五级 MIPS 流水线的时序模拟器框架：`pipe.c` 和 `pipe.h`。目前它已经能够在体系结构功能层面正确执行所有只使用已实现指令的 MIPS 程序。[^lab2-2] 检测到数据依赖时，会通过停顿和/或旁路正确处理；检测到控制依赖时，会按需要停顿流水线。

执行以下命令，便可以看到对于 `inputs/` 中所有测试输入，模拟器 `sim` 和基准模拟器 `basesim` 的体系结构输出（例如寄存器值）完全一致：

```bash
make run
```

本实验的任务是在时序模拟器中**准确模拟 L2 缓存和 DRAM 的时序影响**。

### 5.3. 研究时序模拟器

请详细阅读 `pipe.c` 和 `pipe.h`。

模拟器使用单独的函数来表示每个流水线阶段，例如 `pipe_stage_fetch()`。流水线状态由若干指向 `Pipe_Op` 结构的指针构成（结构定义见 `pipe.h`）。每个 `Pipe_Op` 保存关于流水线中某条指令的所有必要信息。取指时分配一个 `Pipe_Op`，随后它沿流水线流动，最终到达最后一级（写回阶段），指令完成后被释放。换句话说，每一级从前一级接收 `Pipe_Op`，处理后将其传递给下一级。模拟器通过停止 `Pipe_Op` 结构的传递来表示流水线停顿，通过释放位于不同阶段的 `Pipe_Op` 来表示流水线冲刷（flush）。

### 5.4. 建议

- **请勿传播提供的程序文件。** 这些文件仅供计算机体系结构课程学生个人使用；分发或共享会侵犯软件版权。
- 仔细阅读本实验说明。
- 如有需要，请通过 Moodle 中的[在线问答论坛](https://moodle-app2.let.ethz.ch/mod/moodleoverflow/view.php?id=1290568)向助教提问。
- 遇到技术问题时，应首先仔细阅读错误消息；网络搜索通常能够解决许多调试问题和报错。

## 6. 提交要求

请通过 Moodle 的[对应作业页面](https://moodle-app2.let.ethz.ch/mod/assign/view.php?id=1290564)提交。将编译和模拟代码所需的全部文件放入**单个 tar 归档包**，命名为 `lab2_YourSurname_YourName.tar.gz`。请在模拟器代码中添加注释，解释所完成的实现。

提交归档包应具有以下结构：

```text
lab2/
├── README                        # 可选：给助教的特别说明
├── task_1/                       # L2 缓存与 DRAM 实现的源代码
│   ├── Makefile
│   ├── run.py
│   ├── src/
│   └── inputs/
└── extra_credit/                 # 可选：写回与刷新实现
    ├── report_writeback_refresh.pdf
    ├── Makefile
    ├── run.py
    ├── src/
    └── inputs/
```

欢迎附上一份简短的 `README`，说明构建与运行项目时，评分人员需要了解的任何细节。

## 参考文献

[1] Chang Joo Lee, Veynu Narasiman, Eiman Ebrahimi, Onur Mutlu, and Yale N. Patt. *DRAM-Aware Last-Level Cache Writeback: Reducing Write-Caused Interference in Memory Systems*. Technical report, UT Austin, 2010.

[2] Kevin Kai-Wei Chang, Donghyuk Lee, Zeshan Chishti, Alaa R. Alameldeen, Chris Wilkerson, Yoongu Kim, and Onur Mutlu. *Improving DRAM Performance by Parallelizing Refreshes with Accesses*. HPCA, 2014.

[^lab2-1]: “探测”指搜索缓存标签，以确定一次访问是命中还是缺失。
    
[^lab2-2]: 严格来说并非完全如此，因为如实验 1 所述，对于系统调用、异常等仍有通常的限制。
