# CS/ECE 752（2025 年秋季学期）

## 作业 7 

*本作业应由你独立完成，但欢迎你当面或在 Piazza 上与同学讨论遇到的任何问题。适用标准迟交政策——最多可迟交 1 天而不扣分。*

本作业的目的是让你获得在 [CHTC](https://chtc.cs.wisc.edu/) 中运行 gem5 的实践经验。CHTC 是校园内一个使用 HTCondor 批处理提交服务的大规模计算系统。在作业 2 中，你跑的输入规模和程序是故意设小的，以便能在短时间内完成模拟。然而，对于要投会议论文所需的测试，这些输入规模并不现实。不幸的是，当每次模拟都要花好几个小时时，像你作业 2 那样在单个 CPU 上跑所有模拟并不现实。这就是为什么架构师常常借助批处理提交软件（如 HTCondor，以及像 CHTC 这样的大规模计算系统）来并行跑大量作业！本作业由 Kyle Roarty 和 Matt Sinclair 创建。虽然我们已测试并相信它没有 bug，但若发现任何异常请联系我们。

因此，本作业的目标是：拿一个与作业 2 类似的设置，在 CHTC 上测量它在大得多的输入规模、更多应用（与作业 3 相同的应用）、以及多种 CPU 配置下的表现。通过在 gem5 + CHTC 中完成本作业，你将能够 (a) 研究每个应用的性能如何随输入规模和 CPU 配置变化；(b) 演示如何让你的模拟器作业在 CHTC 提交软件中执行。和以前一样，建议使用 gem5 的 *stable* 分支——注意教程页面上我们说明了如何用一个为你做好的 docker 自动完成大部分工作。你完全可以在 docker 之外做这一切，但需要自己解决一些额外的设置问题。

*注意，虽然技术上不借助 CHTC 也能完成本作业的大部分（例如在一个 CPU 上顺序跑完所有），但我特意把输入和配置的规模设成那样：那种方式会非常非常慢，不切实际。*

**在 CHTC 中运行 gem5 的教程**：我们写了一个[教程](https://chtc.cs.wisc.edu/uw-research-computing/gem5/)，讲如何在 CHTC 中运行 gem5。在跑更多实验前，强烈建议先走完这个教程。

如果你不熟悉 CHTC，强烈建议看一些 CHTC 工作人员做的教程视频（**见 Canvas**）。

**系统设置**：所有实验中，假设你的系统有单 CPU，并使用你在作业 1 中创建的两级缓存层次。L1 缓存应为 8 路组相联、16 KB（I\$ 和 D\$ 都如此）；L2 缓存应为统一、16 路组相联、128 KB。主存使用 HBM_2000_4H_1x64（如作业 1）。

**应用**：我们用来测试模拟的程序与作业 3 相同的 6 个应用。但注意下面这段代码有一个重要不同之处（除非你作业 3 时自己改过）：现在输入规模（*N*）基于传入的参数值。变体为：

- DAX：双精度 $aX$
- IAXPY：整数精度 $aX+Y$。对 iaxpy 和 iax，用 *alpha* = 2，X 和 Y 改用 `rand()`。
- IAX：整数精度 $aX$
- SAXPY：单精度（float）$aX+Y$
- SAX：单精度（float）$aX$

与作业 2、3 类似，下面的代码用 C++20 实现 DAXPY（可能与前几次作业略有不同）：

```cpp
#include <cstdio>
#include <stdlib.h>
#include <random>

int main(int argc, char * argv[])
{
    const int N = atoi(argv[1]);
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

1. 你的第一个任务是编译每个应用，并用 gem5 的 MinorCPU 在 CHTC 中模拟它。每个应用用输入规模 65536。这还要求你学习如何用 HTCondor 编译你的应用（见[教程](https://chtc.cs.wisc.edu/uw-research-computing/gem5/)）。**在报告中**，**仅针对感兴趣区域**，报告每个应用不同操作类别的指令分布，并对这 6 个应用的分布做简要分析。为此，像以前一样，在 stats.txt 文件中 grep MinorCPU 对应的统计类。**注意：教程故意没有讲如何让 m5ops 工作——你应该花点时间思考如何把教程和作业 2 中学到的东西用上，让 m5ops 工作**（你的提交必须用 HTCondor 编译带 m5ops 的版本，而不是在提交节点上编译）。编译 m5ops 时如果遇到 TERM 未设置的错误，在你的 HTCondor 提交文件（.sub）中加下面一行：

   ```
   environment = "TERM=xterm-256color"
   ```
2. 接下来我们要把 MinorCPU 与 O3CPU 比较（编译见教程）。O3CPU 是一个更复杂的乱序处理器，相对现代处理器更真实（你们已经知道了）。因此这一步，你要在 CHTC 中用 gem5 的 O3CPU 模拟每个应用。注意这要求你（在 CHTC 中）编译带 O3CPU 的 gem5（如果你还没做的话）。**在报告中**，再次**仅针对感兴趣区域**，比较 MinorCPU 和 O3CPU 模拟这 6 个应用各需多少周期。你认为为什么一个 CPU 比另一个快？请提供基于模拟结果的统计证据。注意这与作业 3 类似，只是输入规模大得多，且不含 TimingSimple CPU。
3. **在报告中**，针对第 2 步得到的 O3CPU 运行结果，比较这 6 个应用在*感兴趣区域*上的性能。哪些应用表现最好？你认为它们为什么表现最好？同样，请提供模拟结果的统计证据。

## 需要提交什么

1. 把以下文件打成一个归档（.zip、.gz 或 .tgz）：
   1. **cpp 文件**：你用于测试的 6 个应用各一个文件。每个文件都应包含第 1–3 问所需的伪指令（`m5_dump_reset_stats()`）。文件名：
      - daxpy.cpp
      - dax.cpp
      - iaxpy.cpp
      - iax.cpp
      - saxpy.cpp
      - sax.cpp
   2. 你用来运行模拟的任何 Python 文件（如 hw7.py）。
   3. 所有模拟的 stats.txt 和 config.ini 文件，命名要能区分每个文件来自哪次运行：
      - stats-daxpy-minor.txt, config-daxpy-minor.ini, stats-daxpy-o3.txt, config-daxpy-o3.ini
      - stats-dax-minor.txt, config-dax-minor.ini, stats-dax-o3.txt, config-dax-o3.ini
      - stats-iaxpy-minor.txt, config-iaxpy-minor.ini, stats-iaxpy-o3.txt, config-iaxpy-o3.ini
      - stats-iax-minor.txt, config-iax-minor.ini, stats-iax-o3.txt, config-iax-o3.ini
      - stats-saxpy-minor.txt, config-saxpy-minor.ini, stats-saxpy-o3.txt, config-saxpy-o3.ini
      - stats-sax-minor.txt, config-sax-minor.ini, stats-sax-o3.txt, config-sax-o3.ini
   4. 你用于所有模拟和编译的 CHTC 提交脚本和 shell 脚本，命名要能区分你用它们跑了哪些实验：
      - compile-gem5.sub, compile-gem5.sh
      - compile-gem5-m5ops.sub, compile-gem5-m5ops.sh
      - run-daxpy.sub, run-daxpy.sh, build-daxpy.sub, build-daxpy.sh
      - run-dax.sub, run-dax.sh, build-dax.sub, build-dax.sh
      - run-iaxpy.sub, run-iaxpy.sh, build-iaxpy.sub, build-iaxpy.sh
      - run-iax.sub, run-iax.sh, build-iax.sub, build-iax.sh
      - run-saxpy.sub, run-saxpy.sh, build-saxpy.sub, build-saxpy.sh
      - run-sax.sub, run-sax.sh, build-sax.sub, build-sax.sh
2. 此外，**与上面的归档分开**，再创建一个名为 `report.pdf` 的文件，内含一份 3 页报告（单倍行距、10 磅字体），回答上述问题。即使某页没写满，也请让每个问题的答案另起一页（即第 1 页答问题 1，第 2 页答问题 2，依此类推）。
3. **把归档和报告提交到 Canvas。**

## 评分细则

**总分：76 分**

1. **编译每个应用的 CHTC 提交（.sub）和 shell（.sh）脚本（共 6 分）**：每个用 m5ops 编译应用的 sub/sh 文件提交了得 1/2 分，否则 0 分。**注意：** 可以写一对 .sub/.sh 文件来启动编译所有应用的命令。教程为简单起见没讲这个，但如果你想这么做，只要你的脚本能完成启动所有作业编译命令所需的工作，我照样给满分。
2. **每个应用运行对应的 CHTC 提交（.sub）和 shell（.sh）脚本（共 6 分）**：每个启动在 gem5 中运行应用作业的 sub/sh 文件提交了得 1/2 分，否则 0 分。**注意：** 可以写一对 .sub/.sh 文件来启动所有 gem5 作业。教程为简单起见没讲这个，但如果你想这么做，只要你的脚本能启动所有作业，我照样给满分。做法见[这里](https://chtc.cs.wisc.edu/uw-research-computing/file-arguments/)。
3. **编译 m5ops 的 CHTC 提交（.sub）和 shell（.sh）脚本（共 1 分）**：编译 m5ops 的 sub/sh 文件对提交了得 1 分，否则 0 分。
4. C++ 文件（共 12 分）：
   - daxpy.cpp（2 分）
   - dax.cpp（2 分）
   - iaxpy.cpp（2 分）
   - iax.cpp（2 分）
   - saxpy.cpp（2 分）
   - sax.cpp（2 分）
5. **编译 gem5 的 CHTC 提交（.sub）和 shell（.sh）脚本（2 分）**：这些脚本每个提交了得 1 分，否则 0 分。
6. **统计与配置文件（共 24 分）**：每个统计文件提交了得 1 分，否则 0 分（共 24 个文件，12 个 MinorCPU、12 个 O3CPU）。
7. **运行 gem5 的 Python 脚本（4 分）**：如可能，主脚本请命名为 hw7.py。
8. **报告（21 分）**：3 个问题每题 7 分。未完全答出问题的给部分分。

## 其他资源：

- [CHTC 用户指南](https://chtc.cs.wisc.edu/)
- Douglas Thain, Todd Tannenbaum, and Miron Livny. 1995. [Distributed Computing in Practice: The Condor Experience](https://pages.cs.wisc.edu/~sinclair/courses/cs752/fall2025/)。
