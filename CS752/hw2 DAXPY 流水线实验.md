# CS/ECE 752（2025 年秋季学期）

## 作业 2

*本作业应由你独立完成，但欢迎你当面或在 Piazza 上与同学讨论遇到的任何问题。适用标准迟交政策——最多可迟交 1 天而不扣分。关于作业中 LLM 使用的细节，见 Homeworks 网页。*

本作业的目的是让你获得流水线 CPU 的实践经验。你将用 TimingSimple CPU 模拟一个给定程序，理解该程序的指令混合（instruction mix）。然后，你将用一个流水线化的顺序（in-order）CPU 模拟同一程序，理解流水线不同部分的延迟和带宽如何影响性能。你还会接触到用于执行底层实验所需功能的伪指令。本作业基于 CA:AQA 第 3 版（本课程原教材）的练习 3.6，部分由 Jason Lowe-Power、Nilay Vaish 和 David Wood 开发，后由我（Matt Sinclair）和 Jason Lowe-Power 更新为现代版本。

**1.** DAXPY 循环（双精度 $aX+Y$）是处理矩阵和向量的程序中常用的运算。下面的代码用 C++20 实现 DAXPY（请按 c++20 编译你的程序）：

```cpp
#include <cstdio>
#include <random>

int main()
{
    const int N = 4096;
    double X[N], Y[N], alpha = 0.5;
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1, 2);
    for (int i = 0; i < N; ++i)
    {
        X[i] = dis(gen);
        Y[i] = dis(gen);
    }

    // Start of daxpy loop
    for (int i = 0; i < N; ++i)
    {
        Y[i] = alpha * X[i] + Y[i];
    }
    // End of daxpy loop

    double sum = 0;
    for (int i = 0; i < N; ++i)
    {
        sum += Y[i];
    }
    printf("%lf\n", sum);
    return 0;
}
```

你的第一个任务是静态编译这段代码，并用 gem5 的 TimingSimple CPU 模拟它。**在报告中**，报告不同操作类别（op class）的指令分布，并对该分布做简要分析。为此，在 stats.txt 文件中 grep `committedInstType` 和/或 `statIssuedInstType`。**注意**：TimingSimple 和 Minor CPU 都只使用 committedInstType，但其他 CPU（如 O3CPU）同时使用 committedInstType 和 statIssuedInstType。本作业你不需要 O3CPU，但我提一下，免得你学期后面用到时困惑。此外，committedInstType 统计正上方还有一些汇总统计，你可能觉得有用。你还应使用与作业 1 相同的两级缓存配置。

**2.** 用 g++ 编译时加 `-S` 和 `-O3` 选项，为上面的 DAXPY 程序生成汇编代码。从汇编代码可以看到，并非直接服务于程序实际任务（计算 $aX+Y$）的指令也会被模拟。这包括生成向量 X 和 Y、对 Y 中元素求和、以及打印和的指令。我用 -S 编译时，O2 下约得到 320 行汇编、O3 下约 500 行汇编，而真正的 DAXPY 循环只有约 15–20 行。可选：本步你可能觉得在 Makefile 里加 `-fverbose-asm` 有用。

在做设计评估实验时，通常希望只看代码中**最重要**那部分的统计。这部分代码也称为**感兴趣区域**（region of interest, ROI）。要只看 ROI，通常会给程序加注解，使模拟器在到达注解代码段开头时执行诸如创建检查点、输出、重置统计变量等操作。这样能保证我们的统计反映的是我们关心的那段代码，而不是把这些统计与我们不关注的部分（例如 DAXPY 中生成向量的部分）混在一起。

要学习如何在 gem5 中重置统计，你要修改第一部分的 C++ 代码，在 DAXPY 循环开始之前和结束之后输出并重置统计。为此，在程序中包含头文件 [m5op.h](https://pages.cs.wisc.edu/~sinclair/courses/cs752/fall2025/handouts/)（你会在 gem5 仓库的 `include/gem5/` 目录中找到这个文件）。在程序中使用该文件里的 `m5_dump_reset_stats()` 函数。这个函数会输出统计变量然后重置它们。delay 和 period 两个参数都传 0。如果你想了解更多 m5ops，[这里](https://www.gem5.org/documentation/general_docs/m5ops/)是个好的起点。特别地，你需要像文档那样更新你的 CFLAGS 和 LDFLAGS。感兴趣者见[详情](https://www.gem5.org/documentation/general_docs/m5ops/)。

注意：我们正在更新 gem5，改用 "hypercalls" 而不是直接用 m5ops。感兴趣可以[阅读相关内容](https://www.gem5.org/documentation/learning_gem5/part1/using_m5ops/)，但由于 hypercalls 还没进 gem5 的 stable 分支，本作业我们不用它们。

在 `$GEM5_ROOT/util/m5/` 目录下执行 **`scons build/x86/out/m5`**。这会在 `$GEM5_ROOT/util/m5/build/x86/out` 下创建一个名为 `libm5.a` 的库（以及一个名为 `m5` 的二进制）。把这个库与 DAXPY 程序链接（用 g++ 编译，CFLAGS/LDFLAGS 更新见上）。然后再次用 TimingSimple CPU 模拟该程序。这一次你应在 stats.txt 文件中看到三组统计。**在报告中**，报告程序三部分中不同操作类别的指令分布。还要附上生成的汇编代码片段：从调用 `m5_dump_reset_stats()` 开始，到下一次调用 `m5_dump_reset_stats()` 结束，中间夹着主 DAXPY 循环。**在本作业后续题目中，你将使用这个集成了 m5ops 的 DAXPY 版本。**

**3.** 如作业 1 的教程所述，gem5 支持几种不同类型的 CPU：atomic、TimingSimple、乱序（out-of-order）、顺序（in-order）和 KVM。我们这里谈时序型和顺序型 CPU。TimingSimple CPU 在单周期内执行每条算术指令，但内存访问需要多个周期。而且它不是流水线化的，任何时刻只处理一条指令。顺序 CPU（也称 MinorCPU）以流水线方式执行指令，流水级为：Fetch1、Fetch2、Decode、Execute。记住，如作业 1 所讨论的，你必须在命令行加上 MinorCPU 才能让它编译出来。

特别是如果你在作业 1 时还没看过，请打开 [MinorCPU.py](https://pages.cs.wisc.edu/~sinclair/courses/cs752/fall2025/handouts/) 和 [BaseMinorCPU.py](https://pages.cs.wisc.edu/~sinclair/courses/cs752/fall2025/handouts/) 文件。在功能单元类 **MinorFU** 的定义中，我们定义了两个量 **opLat** 和 **issueLat**。根据文件中的注释，理解这两个参数应如何使用。还要注意类 **MinorDefaultFUPool** 中实例化的各种功能单元。

假设 FloatSimdFU 的 issueLat 和 opLat 可在 1 到 6 个周期之间变化，且二者之和恒为 7 个周期（例如 issueLat=1、opLat=6，1+6=7）。opLat 每减少 1，就要以 issueLat 增加 1 为代价（例如 issueLat=2、opLat=5）。**在报告中**回答：你更倾向于哪种 FloatSimd 功能单元设计？请提供通过对注解代码段模拟得到的统计证据。

你可以在[这里](http://pages.cs.wisc.edu/~sinclair/courses/cs752/fall2025/handouts/hw/hw2/cpu.py)找到一个扩展 minor CPU 的骨架文件。**注意：CDIS IT 正在阻止从课程网站访问 Python 文件，所以目前请从 Canvas 获取这个文件。** 或者，如果你登录了 CSL 机器，也可以从 `/u/s/i/sinclair/public/html-s/courses/cs752/fall2025/handouts/hw/hw2/cpu.py` 获取。如果你用这个文件，就要修改你的配置脚本以配合它。而且你还要修改这个文件以支持下一部分。

**4.** Minor CPU 在 MinorCPU.py 文件中默认定义了两个整数功能单元（忽略乘法和除法单元）。假设我们最初的 Minor CPU 设计中，整数功能需要 2 个周期（操作延迟/opLat），浮点功能需要 4 个周期（操作延迟/opLat）。假设你在一家公司工作，规划新版 Minor CPU，你可以选择把这两个延迟之一减半。但由于时间有限，你只能选其中一个。**在报告中**回答：我们（贵公司）应选哪个/更倾向哪个？请提供通过模拟得到的统计证据。

## 需要提交什么

1. 把以下文件打成一个归档（.zip、.gz 或 .tgz）：
   1. 一个名为 `daxpy.cpp` 的文件，用于测试。该文件还应包含第 2 问要求的伪指令（`m5_dump_reset_stats()`）。
   2. 再提供一个 `daxpy.s` 文件，含第 2 问要求的那段生成汇编。
   3. 你用来运行模拟的任何 Python 文件。
   4. 所有模拟的 stats.txt 和 config.ini 文件，按下面的文件名命名：

| 题目 | 配置                  | 配置文件名           | 统计文件名          |
| ---- | --------------------- | -------------------- | ------------------- |
| 1    | ---                   | config-opClasses.ini | stats-opClasses.txt |
| 2    | ---                   | config-roi.ini       | stats-roi.txt       |
| 3    | issueLat=6, opLat=1   | config-iL6-oL1.ini   | stats-iL6-oL1.txt   |
| 3    | issueLat=5, opLat=2   | config-iL5-oL2.ini   | stats-iL5-oL2.txt   |
| 3    | issueLat=4, opLat=3   | config-iL4-oL3.ini   | stats-iL4-oL3.txt   |
| 3    | issueLat=3, opLat=4   | config-iL3-oL4.ini   | stats-iL3-oL4.txt   |
| 3    | issueLat=2, opLat=5   | config-iL2-oL5.ini   | stats-iL2-oL5.txt   |
| 3    | issueLat=1, opLat=6   | config-iL1-oL6.ini   | stats-iL1-oL6.txt   |
| 4    | intOpLat=1, fpOpLat=4 | config-ioL1-foL4.ini | stats-ioL1-foL4.txt |
| 4    | intOpLat=2, fpOpLat=2 | config-ioL2-foL2.ini | stats-ioL2-foL2.txt |

5. 你用来编译基准程序的 Makefile。
6. 此外，**与上面的归档分开**，再创建一个名为 `report.pdf` 的文件，内含一份简短报告（400–800 字），回答上述问题。**而且请让报告中每个答案另起一页——这样批改会快很多。此外，请把你的姓名放在第一页右上角。**
7. **把归档和报告提交到 Canvas。**

## 评分细则

**总分：48 分**

1. **统计与配置文件（共 20 分）**：每个统计（stats.txt）/配置文件（config.ini）提交了得 1 分，否则 0 分。命名规范见上表。
2. **daxpy.cpp（2 分）**
3. **cpu.py（4 分）**（如果你选择不改这个文件，则为等效脚本）
4. **Makefile（2 分）**
5. **报告（20 分）**：4 个问题每题 5 分。未完全答出问题的给部分分。
