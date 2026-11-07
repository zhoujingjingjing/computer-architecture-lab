# CS/ECE 752（2025 年秋季学期）

## 作业 1 

*本作业应由你独立完成，但欢迎你当面或在 Piazza 上与同学讨论遇到的任何问题。适用标准迟交政策——最多可迟交 1 天而不扣分。此外，关于作业中 LLM（大语言模型）的使用政策，请见 Homeworks 网页。*

本作业中，你要通读你在 9 月 8 日（周一）被布置阅读的《Learning gem5》一书的前三章。这本书以及本作业的大部分内容，都是由威斯康星 former 博士生 Jason Lowe-Power 编写和设计的。具体而言，本作业你只需要用到《Learning gem5》的 Part I。虽然该教程仍在编写中、不断演进，但已经过我和他人相当充分的测试。尽管如此，如果你认为发现了 bug 或笔误，请在 Piazza 上发帖。如果你愿意，可以跳过 ARM 和默认配置脚本这两个子小节——不过它们对你的课程项目可能有用，所以做一下也值得。

**注意**：在 gem5 中，我们正在升级到一种新的工作负载运行格式（教程中在 "Setting up a configuration script for gem5 v24.1" 一节讨论了它），但我觉得它还没到可以正式使用的程度。因此本作业我们改用 "Setting up a configuration script for gem5 v21.0" 那版。另外，登录 CSL 机器时应 ssh 到 `best-linux.cs.wisc.edu`。不过由于这次改动，《Learning gem5》第二章有一处笔误。你需要在 Python 脚本顶部加入以下内容：

```python
# import the m5 (gem5) library created when gem5 is built
import m5

# import all of the SimObjects
from m5.objects import *
```

## 第 1 步：编译 gem5

通读《Learning gem5》书中的 Introduction 和 Getting Started 页面。在进入下一步之前，先确保 gem5 安装能跑通——如果它不行，后面的步骤也都不行。我强烈建议本作业使用 Linux（我在 CSL 计算机上测试过）。此外，强烈建议你使用 stable 分支，因为它是 gem5 最稳定的公开分支。**注意：你不需要像教程那样用 sudo 安装 git、scons 等组件——CSL 机器上应该都已经装好了。**

**注意：《Learning gem5》书中的编译命令有两个小问题。首先，X86 构建默认不会编译 *MinorCPU* 模型。请改用下面这条命令：**

```bash
python3 `which scons` build/X86/gem5.opt -jX CPU_MODELS=AtomicSimpleCPU,TimingSimpleCPU,O3CPU,MinorCPU
```

这里我建议把 X 设为你系统核数加 1——gem5 编译很耗时，所以应尽量多用线程加速编译！注意即便用若干线程，在 CSL 机器上编译也可能要 30 分钟左右。

## 第 2 步：《Learning gem5》书 Part I

本作业中，《Learning gem5》书最重要的部分是：

- 下载并构建 gem5；
- 创建一个简单的配置脚本；
- 如何运行 gem5；
- 给你的第一个脚本增加复杂度：加入两级缓存层次；
- 如何解析 gem5 输出并理解统计数据。

不过你也应该通读 Part I 的其余部分，因为它会提供关于 gem5 工作原理的宝贵背景信息，这些信息在学期后面可能有用（例如课程项目和后续作业）。你也可能觉得《Learning gem5》教程 Part I 的 YouTube 录像有用。此外，尽管教程在每节末尾都给出了最终脚本的链接，但为了你自己好，还是应该一步步走完教程、自己创建脚本。最后注意：你的 "hello world" 程序看到的 ticks 应与教程所示不同——例如，我用默认两级缓存文件运行 hello world 应用时得到 56435000 ticks。

## 第 3 步：写一个有趣的应用

写一个简单的 C++ 程序，实现埃拉托斯特尼筛法（Sieve of Eratosthenes），并在最后输出一个整数：不超过命令行传入输入参数的素数个数。如果你不熟悉该算法，见上面链接。把你的程序编译成静态二进制。例如，若输入为 1,000,000，输出应为：78498。

## 第 4 步：使用 gem5！

接下来，你要在 gem5 中用第 2 步做好的配置脚本运行你的应用。在这一步，你要在测试你的筛法程序时，改变 CPU 模型、CPU 频率和内存配置。此外，你还要描述不同配置在性能上的变化。

1. **在 gem5 中运行你的筛法程序**

   在 gem5 中运行你的筛法程序，而不是 'hello' 示例。你需要把配置文件中二进制的位置从 'hello' 二进制改成你的筛法二进制。**不要用 se.py 或任何其他已有配置脚本。用你在第 2 步做的那个带两级缓存的脚本。** 模拟器每次运行都会生成一个统计文件作为输出——保存每次运行生成的统计文件。**警告：默认情况下，gem5 每次都把输出文件写到同一个文件夹（m5out）。务必在每次后续运行之前把输出文件移走。**

   **选择合适的输入规模。** 你应选一个足够大、使应用有趣、但又不至于让 gem5 跑一次模拟超过 10 分钟的规模。我发现输入规模 1,000,000 大约需要 5 分钟，是个合理折中。此外，**把 L1 和 L2 缓存大小设成输入数据放不进任何一级缓存。**

   *注意：MinorCPU（下一步）比 TimingSimpleCPU 慢约 10 倍。*
2. **改变 CPU 模型**

   把 CPU 模型从 *TimingSimpleCPU* 改成 *MinorCPU*。

   提示：你可能想加一个命令行参数来控制 CPU 模型。
3. **改变 CPU 时钟频率**

   对两种 CPU 模型，把 CPU 时钟从 1 GHz 调到 2 GHz 再到 4 GHz。

   提示：同样，你可能想为频率加一个命令行参数。
4. **改变主存**

   你脚本中的默认内存配置是 DDR3_1600_8x8。改变内存配置，使你收集到 7 种不同主存配置下的结果：

   - *DDR3_1600_8x8*（默认），建模 DDR3 内存。
   - *DDR3_2133_8x8*，建模时钟更快的 DDR3。
   - *DDR4_2400_8x8*，建模 DDR4 内存。
   - *DDR5_4400_4x8*，建模 DDR5 内存。
   - *LPDDR2_S4_1066_1x32*，建模 LPDDR2，一种常见于移动设备的低功耗 DRAM。
   - *HBM_1000_4H_1x64*，建模高带宽内存（HBM），一种常用于 GPU 和网络设备的高性能内存。
   - *HBM_2000_4H_1x64*，建模第二代 HBM 内存。

   对每种内存配置，都用 *MinorCPU* 和 *TimingSimpleCPU* 各跑一次。这一步频率固定在 4 GHz。
5. **核对收集到的数据**

   上面步骤跑完后，你应有 18 个统计文件（每个文件按下表 Filename 列命名）：

| CPU 模型        | CPU 频率 (GHz) | 内存                | 文件名                          |
| --------------- | -------------- | ------------------- | ------------------------------- |
| TimingSimpleCPU | 1              | DDR3_1600_8x8       | stats.timing.ddr3-1600.1ghz.txt |
| TimingSimpleCPU | 2              | DDR3_1600_8x8       | stats.timing.ddr3-1600.2ghz.txt |
| TimingSimpleCPU | 4              | DDR3_1600_8x8       | stats.timing.ddr3-1600.4ghz.txt |
| MinorCPU        | 1              | DDR3_1600_8x8       | stats.minor.ddr3-1600.1ghz.txt  |
| MinorCPU        | 2              | DDR3_1600_8x8       | stats.minor.ddr3-1600.2ghz.txt  |
| MinorCPU        | 4              | DDR3_1600_8x8       | stats.minor.ddr3-1600.4ghz.txt  |
| TimingSimpleCPU | 4              | DDR3_2133_8x8       | stats.timing.ddr3-2133.4ghz.txt |
| TimingSimpleCPU | 4              | DDR4_2400_8x8       | stats.timing.ddr4-2400.4ghz.txt |
| TimingSimpleCPU | 4              | DDR5_4400_4x8       | stats.timing.ddr5-4400.4ghz.txt |
| TimingSimpleCPU | 4              | LPDDR2_S4_1066_1x32 | stats.timing.lpddr2.4ghz.txt    |
| TimingSimpleCPU | 4              | HBM_1000_4H_1x64    | stats.timing.hbm-1000.4ghz.txt  |
| TimingSimpleCPU | 4              | HBM_2000_4H_1x64    | stats.timing.hbm-2000.4ghz.txt  |
| MinorCPU        | 4              | DDR3_2133_8x8       | stats.minor.ddr3-2133.4ghz.txt  |
| MinorCPU        | 4              | DDR4_2400_8x8       | stats.minor.ddr4-2400.4ghz.txt  |
| MinorCPU        | 4              | DDR5_4400_4x8       | stats.minor.ddr5-4400.4ghz.txt  |
| MinorCPU        | 4              | LPDDR2_S4_1066_1x32 | stats.minor.lpddr2.4ghz.txt     |
| MinorCPU        | 4              | HBM_1000_4H_1x64    | stats.minor.hbm-1000.4ghz.txt   |
| MinorCPU        | 4              | HBM_2000_4H_1x64    | stats.minor.hbm-2000.4ghz.txt   |

## 第 5 步：分析模拟数据

收集完上一步所有数据后，分析你运行生成的统计数据，并写一份报告。报告应为 PDF，并回答以下问题：

1. 应使用什么指标来比较不同系统配置之间的性能？为什么这是合适的指标？
2. 哪种 CPU 模型对 CPU 频率变化更敏感？为什么？
3. 哪种 CPU 模型对内存技术变化更敏感？为什么？
4. 你的筛法应用对 CPU 模型、内存技术还是 CPU 频率更敏感？为什么？
5. 如果你换一个不同的应用，你认为结论会变吗？为什么？

## 需要提交什么

1. 把以下文件打成一个归档（.zip、.gz 或 .tgz）：
   1. 一个名为 `sieve.cpp` 的文件，内含你的埃拉托斯特尼筛法实现。
   2. 一个名为 `sieve-config.py` 的文件（以及任何其他必要文件），即用来运行 gem5 的配置。
   3. 你在 gem5 中运行筛法程序产生的全部 18 个统计文件（stats.txt），命名要能区分每个文件来自哪次运行。
2. 此外，**与上面的归档分开**，再创建一个名为 `report.pdf` 的文件，内含一份简短报告，写你从实验中得到的观察和结论，包括对上述问题的回答。
3. **把归档和报告提交到 Canvas 的 dropbox。**

## 评分细则

**总分：46 分**

1. **统计文件（共 18 分）**：每个统计文件提交了得 1 分，否则 0 分。
2. **sieve.cpp（4 分）**
3. **sieve-config.py（4 分）**
4. **报告（20 分）**：5 个问题每题 4 分。未完全答出问题的给部分分。
