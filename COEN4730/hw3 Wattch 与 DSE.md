# COEN-4730 计算机架构

# HW 3

马凯特大学 电气与计算机工程系

*Cristinel Ababei*

## 目标

学习 **SimpleScalar** 和 **Wattch** 工具。学习如何通过所谓的"设计空间探索（Design Space Exploration, DSE）"，为特定基准测试程序找到合适的超标量（superscalar）体系结构。本练习应能让你对超标量体系结构有所认识。

## 1. SimpleScalar 与 Wattch 模拟器

**SimpleScalar 工具集**是一个软件系统基础设施，用于模拟一系列处理器和系统。该工具集包含若干模拟器，从快速的功能模拟器到详细的、动态调度的处理器模型不等，后者支持非阻塞缓存（non-blocking caches）、推测执行（speculative execution）和最先进的分支预测。它在 2000 年代初非常流行，顶级计算机体系结构会议上发表的论文有超过三分之一使用 SimpleScalar 来评估其设计。

**Wattch** 是一个估算 CPU 功耗的体系结构模拟器。功耗估算基于一组可参数化的功耗模型，这些模型针对不同的硬件结构，并通过周期级模拟生成每周期的资源使用计数。这些功耗模型是集成在 SimpleScalar 体系结构模拟器之上的。

## 2. 准备工作

（在你上一次作业所用的 Linux 机器或虚拟机上）安装并熟悉 SimpleScalar 3.0 工具集和 Wattch 1.02 工具集。

### A. 安装 SimpleScalar 3.0

本作业提供的文件中包含 simplescalar 3.0 模拟器的安装文件。它就在提供的 .zip 文件中。把它下载到你的机器上。也可以从下面的网站下载，但网站上的版本可能不是最新的：

[http://www.simplescalar.com/](http://www.simplescalar.com/)

要编译 simplesim-3.0（即 SimpleScalar 处理器模拟器），在终端输入以下命令：

```
> cd $HOME/simplesim-3.0    （$HOME 应指向你解包 .tar 文件的位置；
                              例如我这里把 $HOME 设为 /home/cristinel/coen4730/，
                              因为这是我解包并安装该工具的目录）
> make config-pisa
> make
```

### B. 测试你的 SimpleScalar

输入以下命令：

```
> cd $HOME/simplesim-3.0
> ./sim-outorder ./tests/bin.little/test-math    （默认：乱序发射 out-of-order issue）
```

你应该会看到类似这样的输出：

```
sim: ** starting performance simulation **
         pow(12.0, 2.0) == 144.000000
         pow(10.0, 3.0) == 1000.000000
         ...
         -1e-17 != -1e-17  Worked!
```

从输出中你会发现，`sim_CPI`（每条指令的时钟周期数）约为 **1.0497**。

输入以下命令：

```
> cd $HOME/simplesim-3.0
> ./sim-outorder -issue:inorder ./tests/bin.little/test-math    （顺序发射 in-order issue）
```

从输出中你会发现，`sim_CPI` 约为 **1.51**。

为什么这两个 CPI 不同？

### C. 熟悉 SimpleScalar

花点时间阅读 SimpleScalar 的文档并熟悉它。本作业提供的归档包中包含若干 PDF 文件和其他资料。尝试不同的模拟器：`sim-safe`、`sim-fast`、`sim-eio`、`sim-profile`（**本作业其中一项会用到它，详见后文**）、`sim-cache`、`sim-bpred`、`sim-outorder`（预编译的测试程序在目录 `$HOME/simplesim-3.0/tests-pisa/bin.little/` 下），比较并理解这些模拟器的含义和用法。

运行 `sim-outorder` 模拟器，观察不同分支预测策略的影响（例如：始终不命中 always not taken、始终命中 always taken、bimodal 双峰、2 level 两级）。

### D. 安装 Wattch 1.02

从本作业提供的 .zip 文件下载 Wattch，即 `sim-watch-1.02e.tar.gz`。你也可以从这里下载：[http://www.eecs.harvard.edu/~dbrooks/wattch-form.html](http://www.eecs.harvard.edu/~dbrooks/wattch-form.html)

在终端输入以下命令编译 Wattch 模拟器：

```
> cd $HOME/sim-watch-1.02d    （同样取决于你把 .tar 文件解包到哪里）
> make config-pisa
> make
```

阅读本作业归档包中提供的 Wattch 论文。试用 Wattch 的 sim-outorder，看看新增了哪些特性。Wattch 集成了四种不同的功耗模型：`avg_total_power_cycle`、`avg_total_power_cycle_cc1`、`avg_total_power_cycle_cc2` 和 `avg_total_power_cycle_cc3`。它们有什么区别？

> **注意：**
> 仅仅为了运行模拟、收集数字以便完成本作业（见后文），下载并只使用 Wattch 工具即可——这是务实（PRACTICAL）的做法，因为它构建在 SimpleScalar 模拟器的一个版本之上。当你运行 Wattch 时，输出会和你运行 SimpleScalar 时类似，但它同时还会报告功率数字，你会需要这些数字来计算 EDP。不过，把两个工具都看一遍——原版和改进版——是一个很好的学习训练……

## 3. 设计空间探索（DSE）

到现在为止，你应该已经在 SimpleScalar 和 Wattch 工具上玩得差不多了。真正的作业在这里：任务是找出最适合运行 **compress** 基准测试程序的超标量体系结构。本作业主要使用 Wattch（sim-outorder），因为它能同时提供性能和功耗结果。不过，其他 SimpleScalar 工具（如 `sim-profile`、`sim-cache`、`sim-bpred`……）由于模拟速度更快、信息更详细，也可能会有帮助。本作业的目标是加深对超标量体系结构的理解。

### 总体描述

- 本作业使用的基准测试程序是 **compress**。
- 通过调整 SimpleScalar 体系结构模拟器的参数，我们寻找一个对运行 **compress** 应用"最合适"的处理器体系结构。
- 本作业中我们用两个指标来定义"最合适"：
  1. **性能（Performance）**，用 IPC（Instructions Per Cycle，每周期指令数）衡量；在 Wattch 模拟器中对应 `sim_IPC`。
  2. **能耗（Energy）**，用 EDP（Energy-Delay Product，能耗延迟积）衡量。这里使用近似公式 **(CPI × Energy/Cycle) × CPI**。其中 CPI（每周期指令数）对应 `sim_CPI`，Energy/Cycle 对应 Wattch 模拟器中的 `avg_total_power_cycle_cc3`。
- 我们主要关注超标量体系结构中的若干关键组件，例如分支预测器和存储系统。

### 系统默认配置

- 使用 `sim-outorder` 的 `-dumpconfig` 标志把默认体系结构配置导出到某个文件（例如 `my1.cfg`）：

```
./sim-outorder -dumpconfig > my1.cfg
```

- 查阅 SimpleScalar 相关文档和帮助（`-h`），理解每个参数的含义。
- 我们感兴趣的参数及其默认值，列在本作业归档包的 `file_1_parameters_file.pdf` 中。（本作业中，**只允许调整这些参数**）。底线是：你必须设置一个务实的（PRACTICAL）配置。例如，你不能随意选择"完美（Perfect）"分支预测器作为理想情形；同样，你不能修改缓存的命中率/缺失率并设为 "0"，因为这样做说不通。
- `config` 标志对于模拟你自己的配置非常有用。你可以先导出默认配置文件（如 `my1.cfg`），编辑它来调整参数，然后另存为新文件（如 `my2.cfg`），之后只需简单地调用 `./sim-outorder -config my2.cfg ...` 即可运行，而不必在命令行里一个个显式配置参数。

### compress 基准测试程序

- 本作业使用的基准测试程序是 **compress**，它是一个整数基准。预编译好的二进制文件（`compress.ss`）及其输入文件（`test.in`）都在本作业的归档包中。本作业使用 Wattch 的 sim-outorder 模拟器来获取 IPC（`sim_CPI`）、IPC（`sim_IPC`）以及功耗 `avg_total_power_cycle_cc3`。要使用默认设置模拟该应用（以 `compress.ss` 的输入文件为例，输入为 `test.in`），可以这样做：

```
> $HOME/sim-watch-1.02e/sim-outorder compress.ss < test.in
```

### 用脚本自动完成 DSE

- 脚本可以减轻在庞大设计/解空间上做设计空间探索（DSE）的痛苦。例如，假设你想探索仅由三个参数（如 `-bpred`、`-cache:dl1` 和 `-cache:il1`）定义的设计空间。搜索过程通常是这样的：取第 1 个参数的某个值、第 2 个参数的各个取值、第 3 个参数的各个取值。问题在于这是一个组合问题，随着参数个数和每个参数取值数目的增加，计算时间会呈指数增长。
- 本作业归档包中提供了一个简单的示例脚本。**你可以修改并在做本作业时使用它——让任务变轻松，但并不是必须这么做。** 不过，你也完全可以自己写脚本，使用任何脚本语言（如 Perl）来自动完成工作。你也可以用脚本从多个输出文件中挑出有用的数据，这能让分析更容易。
- **不要**写一个脚本去遍历 `file_1_parameters_file.pdf` 中所有参数、对所有参数做暴力搜索。那样会跑非常非常久，而且也不保证有意义的结果。

### 作业指导

1. **设置基线（baseline）**：用默认配置/设置运行 compress 应用。这次运行的性能（即 IPC）和能耗延迟积（即 `(CPI * Energy/Cycle) * CPI`）分别作为性能基线和性能-能耗基线。
2. **用 `sim_profile` 剖析基准测试程序**，识别其特征（例如各类指令所占百分比）。这项任务也是本作业的一部分。
3. 本作业中你可以调整的参数被预先分为四组：A）分支预测；B）存储系统；C）功能单元；D）其他数据通路（另见参数文件 `file_1_parameters_file.pdf`）。

   **A）分支预测**

   - 分支预测器类型（`-bpred`）
   - 双峰预测器配置（`-bpred:bimod`）
   - 2 级预测器配置（`-bpred:2lev`）。更多信息请参考 Yale Patt 的论文。
   - 返回地址栈大小（`-bpred:ras`）

   **B）存储系统**

   - L1 数据缓存（`-cache:dl1`）。你需要找出：1）不同缓存大小的影响；2）相同大小、不同相联度的影响；3）块大小的影响；4）替换策略的影响。
   - L1 指令缓存（`-cache:il1`）。你需要找出：1）不同缓存大小的影响；2）相同大小、不同相联度的影响；3）块大小的影响；4）替换策略的影响。
   - 与上面相同，但针对 L2 缓存。
   - 统一缓存还是分离缓存。

   **C）功能单元**

   - 整数功能单元的数量（`-res:ialu`、`-res:imult`）
   - 浮点功能单元的数量（`-res:fpalu`、`-res:fpmult`）

   **D）数据通路与其他**

   - 指令解码宽度的影响（`-decode:width`）
   - 顺序与乱序的影响（`-issue:inorder`）
   - 重排序缓冲（reorder buffer）大小的影响（`-ruu:size`）
4. **你必须从每组中选择一个参数。你将在由所选参数构成的空间中进行求解搜索。** 对于选定的参数，在可能的情况下，通过扫描至少三个不同取值来进行搜索。尽量不要和你的同学选的参数完全相同；如果确实相同，请讨论一下原因。
5. **执行设计空间探索（DSE）**

   - 单独取你选的每个参数，扫描其取值（把其他所有参数——包括另外三个所选参数——保持在默认值），通过运行模拟完成。
   - 绘制性能图和 EDP 图，并标注扫描的是哪个参数/设置。报告中要包含全部 8 张图。确认图的内容符合你的预期。每张图用一两句话解释结果。
   - 现在，通过同时改变全部四个参数，进行完整的解空间搜索。你必须找到让性能最好和 EDP 最好的四参数组合。注意这里你要跑 81 次模拟，因为 4 个参数各有 3 个取值：3×3×3×3 = 81。这一步尤其要写个脚本自动完成！

## 4. 交付物

一份报告，包含以下各节：

1. **标题、你的姓名、课程名称和日期**
2. **描述（Description）**：用一段话描述本作业是关于什么的。它的目标是什么。
3. **剖析（Profiling）**：列出用 `sim_profile` 对基准测试程序做剖析的结果。并附上一段简短的说明。
4. **单参数扫描（Sweeping of individual parameters）**：上面讨论的 8 张图，每张图配一段说明文字。讨论你的结果并得出结论。每张图都应有坐标轴标签。图必须包含图编号和标题，清楚地说明每张图代表什么。描述中请引用具体图号。
5. **四参数 DSE**：包含一个表格或两张图（一张性能、一张 EDP），给出全部 81 次模拟的结果。报告让性能最好和 EDP 最好的四参数组合。
6. **结论（Conclusion）**：总结你的发现。清楚地说明你从本作业中学到了什么。

## 7. 参考文献与致谢

本作业改编自 Henk Corporaal 教授及其助教 Zhenyu Ye 的教学材料。
