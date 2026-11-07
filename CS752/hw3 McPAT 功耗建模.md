# CS/ECE 752（2025 年秋季学期）

## 作业 3 

*本作业应由你独立完成，但欢迎你当面或在 Piazza 上与同学讨论遇到的任何问题。适用标准迟交政策——最多可迟交 1 天而不扣分。关于作业中 LLM 使用的细节，见 Homeworks 网页。*

本作业的目的是让你获得现代 CPU 功耗建模的实践经验。具体地，我们将把广泛使用、流行的 [McPAT](https://github.com/HewlettPackard/mcpat) 功耗建模工具与 gem5 结合使用，理解各种 CPU 应用如何消耗功耗。对于所有这些应用，你都应使用作业 2 中的 m5ops 设置，以便更好地把感兴趣区域（ROI）的行为与设置和清理阶段隔离开。本作业由 Alex Smith 和 Matt Sinclair 创建。虽然我们已测试并相信它没有 bug，但若发现任何异常请联系我们。

**系统设置**：所有实验中，假设你的系统有单 CPU，并使用你在作业 1 中创建的两级缓存层次。L1 缓存应为 8 路组相联、16 KB（I\$ 和 D\$ 都如此）；L2 缓存应为统一、16 路组相联、128 KB。主存使用 DDR3_1600_8x8（如作业 1；旁注：McPAT 只支持到 DDR3，所以如果你试图改用更新的 DDR 版本和 HBM，它们不会工作）。你还应使用作业 2 的 m5ops（但现在应用到更多应用上），以获取 ROI 而非整个应用的结果。最后，对于 gem5 中有分支预测器的 CPU，为保持一致，请使用 gem5 的 Tournament Branch Predictor（TournamentBP）及其默认配置（即你无需改动）。

**应用**：我们用来测试模拟的程序都基于你在作业 2 跑过的 DAXPY（双精度 $aX+Y$）程序。不过，除 DAXPY 外，你还要把 DAXPY 代码改成跑另外几个变体：

- DAX：双精度 $aX$
- IAXPY：整数精度 $aX+Y$。对 iaxpy 和 iax，用 *alpha* = 2，X 和 Y 改用 `rand()`。
- IAX：整数精度 $aX$
- SAXPY：单精度（float）$aX+Y$
- SAX：单精度（float）$aX$

所有应用使用相同输入规模（4096）——注意这与你作业 2 用的相同。与作业 2 一样，用 `-O3 -static` 编译程序。

**1.** 你的第一个任务是用与作业 2 类似的编译方式编译每个应用。然后，在 gem5 中分别用 TimingSimple CPU、MinorCPU 和 O3CPU 运行这 6 个应用（因此这些实验做完后你会有 18 个模拟结果）。与作业 1、2 不同，这次我们用 gem5 的 ARM CPU 支持。要为 ARM 编译你的工作负载，除非你的机器本身是 ARM CPU，否则你需要（在本地机器上）安装或（在 best-linux.cs.wisc.edu 上）使用 ARM 交叉编译器支持。无论如何，基于 Ubuntu 的系统上 `g++-14-aarch64-linux-gnu` 包应足够（CSL 机器上已装好这个包）。注意，使用 ARM CPU 还需要对配置脚本做小改动，指定你想跑 ARM CPU 而不是 X86。最后，为保证 gem5 中结果最准确，我们**强烈建议你拉取[这个](https://www.gem5.org/documentation/general_docs/building/)提交补丁，叠加到你自己的 gem5 分支上**。**在报告中**，为每个基准、每个配置，作图并报告执行时间（即 simTicks）**仅 ROI 部分**的差异。此外，简要分析各应用在不同配置下的相对性能。

**2.** 接下来我们要把每个应用/CPU 组合的 gem5 结果传给 McPAT。如前所述，McPAT 常被认为是当今系统中 CPU 功耗建模的最先进工具。McPAT 用基于第一性原理的层次化模型建模 CPU，根据 CPU 体系结构和组件（如分支预测器、ALU）估算功耗。McPAT 假设顺序核有 4 个流水级（Fetch、Execute、LSU、Memory），乱序核有 5 个流水级（与顺序相同，但多了 Renaming）。本作业你不需要深入理解 LSU 和 Renaming，我们学期后面会讲。

要把 gem5 结果传给 McPAT，按以下步骤：

1. **搭建 McPAT**

   1. 用下面的命令 clone McPAT 仓库，下载 McPAT [源代码](https://github.com/HewlettPackard/mcpat)：

      ```bash
      git clone https://github.com/HewlettPackard/mcpat
      ```
   2. 运行 `make` 构建 McPAT。
   3. 建议浏览 README 了解更多；可执行文件（默认名为 `mcpat`）接收两个参数：描述你系统的输入文件（核、缓存、内存等），以及你想打印的详细程度。如果要跑 McPAT，仓库的 *ProcessorDescriptionFiles* 文件夹里有几个处理器设计示例。例如，要估算一个跑过真实工作负载的 Alpha21364 处理器（它类似于 gem5 的 O3CPU 所基于的 Alpha21264 处理器）的功耗，可以在仓库根目录下这样跑其中一个示例：

      ```bash
      ./mcpat -infile ProcessorDescriptionFiles/Alpha21364.xml -print_level 3
      ```
   4. 建议始终把 print_level 参数留在 5，因为这样能给出各组件功耗（和面积）相当细的分解。
2. **配置你的 XML 文件**

   1. 为 McPAT 配置系统需要一个 XML 文件，组件按层次嵌套。XML 中主要要定义两件事：

      1. 处理器（和系统）的属性，如 ISA、核数、时钟频率等；以及
      2. 从模拟器跑完工作负载后得到的统计（即处理器确实做了**一些**工作，这里就是我们在 gem5 中模拟的工作负载）。
   2. 本作业中我们将说明如何为上面每一点配置 XML 文件。由于手工修改很繁琐，建议你写一个解析器，读取 gem5 输出的 *stats.txt*，并与 McPAT 中的统计匹配（可选：你的脚本可以解析 gem5 的 config.ini 或 config.json 文件以匹配下面 2c 的配置步骤，但主要工作量在匹配统计）。
   3. **系统和处理器配置**

      1. 你可以从 *ProcessorDescriptorFiles* 目录复制任一模板，但建议从复制 *ARM_A9_2GHz.xml* 文件开始（**Canvas 上提供**）。在 *system* 组件下，编辑以下参数：
         - <u>number_of_cores</u>：设为 1（因为我们跑单核模拟）。
         - <u>number_of_L2s</u>、<u>Private_L2</u>：分别设为 1 和 0，因为我们建模的是统一 L2 缓存。
         - <u>target_core_clockrate</u>：应为你 CPU 核的时钟频率（MHz），所以如果频率是 2GHz，该参数填 2000。
         - <u>total_cycles</u>：应为 `system.cpu.numCycles` 统计。
         - <u>idle_cycles</u>：应为 `system.cpu.idleCycles` 统计。
         - <u>busy_cycles</u>：取 total_cycles 与 idle_cycles 之差作为此统计。
      2. 接下来在 *system.core0* 组件下，改以下参数：
         - <u>clock_rate</u>：与 target_core_clockrate 相同。
         - <u>x86</u>：设为 0，因为我们用的是 ARM 核。
         - <u>machine_type</u>：如果为 O3CPU 跑 McPAT 则设为 0，否则设为 1。
         - <u>number_hardware_threads</u>：设为 1。
         - <u>ALU_per_core</u>：确保设为 1。
         - <u>rename_scheme</u>：确保设为 1。
      3. 严格说你还要更新 McPAT 的指令缓存、L1 数据缓存和 L2 缓存。不过本作业我们用默认值，所以你不用担心。
   4. **统计配置**
      本节说明哪些 gem5 统计对应 McPAT 中的哪些统计。我们**强烈建议**你使用提供的 *gem5-mcpat-parser.py*（Canvas 上提供）来自动完成这个转换。使用示例：

      ```bash
      python3 gem5-mcpat-parser.py --cpu_type in-order --m5_stats m5out/stats.txt --output_xml iax-minor.xml --template_xml ../mcpat/ARM_A9_2GHz_gem5.xml
      ```

      参数含义：

      ```bash
      python3 gem5-mcpat-parser.py --verbose --cpu_type {in-order, out-of-order} --m5_stats {M5_STATS_FILE} --output_xml {OUTPUT_FILENAME} --template_xml {TEMPLATE_XML_FILE}
      ```

      上面的例子中，我们把 XML 输出到文件 iax-minor.xml，M5 统计文件在 `m5out/stats.txt`，CPU 是顺序 CPU（即 Minor/Timing）。`--verbose` 选项给出详细输出，便于你检查某些统计是否正确输出。

      **关键：解析脚本假设你只传入一组统计。因此你应只传入 ROI 的统计（或修改脚本解析第 N 组统计）。**

      不过如果你想自己搭建，需要用下面的表把 gem5 的统计格式转换成 McPAT 需要的 XML 格式。要这样做，参考我们创建的[教程](https://www.gem5.org/documentation/general_docs/mcpat/)。
3. **运行 McPAT**

   XML 配置正确后，你应能这样运行 McPAT：

   ```bash
   ./mcpat -infile {Application XML} -print_level 5
   ```

   McPAT 功耗建模的输出会显示在标准输出——你将用它绘制各配置下各组件的功耗。

   **在报告中**，为 6 个应用、3 种 CPU 分别作图展示功耗。然后回答关于这 6 个应用在 3 种 CPU 下功耗的问题：(a) 按 McPAT，三种 CPU 模型中哪一个通常功耗最高？这个结果符合你的预期吗？(b) 对 O3CPU，讨论 IAXPY、SAXPY 和 DAXPY 的相对功耗——这个顺序合理吗？(c) 对 O3CPU，评论 IAX 与 IAXPY 的相对功耗——这个顺序合理吗？三个问题都要提供统计证据并做定性论证。

## 需要提交什么

1. 把以下文件打成一个归档（.zip、.gz 或 .tgz）：
   1. **cpp 文件**：你用于测试的 6 个应用各一个文件。每个文件都应包含第 1–3 问所需的伪指令（`m5_dump_reset_stats()`）。文件名：
      - daxpy.cpp
      - dax.cpp
      - iaxpy.cpp
      - iax.cpp
      - saxpy.cpp
      - sax.cpp
2. 你用来编译基准程序的 Makefile。
3. 你用来运行模拟的任何 Python 文件（如 hw3.py）。
4. 你用来生成 McPAT 数值的任何文件（即 XML 文件），按下面格式命名：
   - daxpy-simple.xml, daxpy-minor.xml, daxpy-o3.xml
   - dax-simple.xml, dax-minor.xml, dax-o3.xml
   - iaxpy-simple.xml, iaxpy-minor.xml, iaxpy-o3.xml
   - iax-simple.xml, iax-minor.xml, iax-o3.xml
   - saxpy-simple.xml, saxpy-minor.xml, saxpy-o3.xml
   - sax-simple.xml, sax-minor.xml, sax-o3.xml
5. 功耗输出文件（即把 McPAT 的 stdout 输出存成文件）、stats.txt、以及所有模拟的 config.ini/config.json 文件，命名要能区分每个文件来自哪次运行：
   - power-daxpy-simple.txt, stats-daxpy-simple.txt, config-daxpy-simple.[ini,json], power-daxpy-minor.txt, stats-daxpy-minor.txt, config-daxpy-minor.[ini,json], power-daxpy-o3.txt, stats-daxpy-o3.txt, config-daxpy-o3.[ini,json]
   - power-dax-simple.txt, stats-dax-simple.txt, config-dax-simple.[ini,json], power-dax-minor.txt, stats-dax-minor.txt, config-dax-minor.[ini,json], power-dax-o3.txt, stats-dax-o3.txt, config-dax-o3.[ini,json]
   - power-iaxpy-simple.txt, stats-iaxpy-simple.txt, config-iaxpy-simple.[ini,json], power-iaxpy-minor.txt, stats-iaxpy-minor.txt, config-iaxpy-minor.[ini,json], power-iaxpy-o3.txt, stats-iaxpy-o3.txt, config-iaxpy-o3.[ini,json]
   - power-iax-simple.txt, stats-iax-simple.txt, config-iax-simple.[ini,json], power-iax-minor.txt, stats-iax-minor.txt, config-iax-minor.[ini,json], power-iax-o3.txt, stats-iax-o3.txt, config-iax-o3.[ini,json]
   - power-saxpy-simple.txt, stats-saxpy-simple.txt, config-saxpy-simple.[ini,json], power-saxpy-minor.txt, stats-saxpy-minor.txt, config-saxpy-minor.[ini,json], power-saxpy-o3.txt, stats-saxpy-o3.txt, config-saxpy-o3.[ini,json]
   - power-sax-simple.txt, stats-sax-simple.txt, config-sax-simple.[ini,json], power-sax-minor.txt, stats-sax-minor.txt, config-sax-minor.[ini,json], power-sax-o3.txt, stats-sax-o3.txt, config-sax-o3.[ini,json]
6. 此外，**与上面的归档分开**，再创建一个名为 `report.pdf` 的文件，内含一份简短报告（400–800 字），给出上述问题的答案（和图）。**而且请让报告中每个答案另起一页——这样批改会快很多。此外，请把你的姓名放在第一页右上角。**
7. **把归档和报告提交到 Canvas。**

## 评分细则

**总分：104 分**

1. **功耗输出、统计与配置文件（共 54 分）**：每个功耗输出、统计（stats.txt）和配置文件（config.ini/config.json）提交了得 1 分，否则 0 分（命名应遵循上面方案）。
2. **功耗输入（XML）文件（共 18 分）**：每个功耗输入（XML）文件提交了得 1 分，否则 0 分（命名应遵循上面方案）。
3. C++ 文件（共 6 分）：
   - daxpy.cpp（1 分）
   - dax.cpp（1 分）
   - iaxpy.cpp（1 分）
   - iax.cpp（1 分）
   - saxpy.cpp（1 分）
   - sax.cpp（1 分）
4. **Makefile（2 分）**
5. **hw3.py（4 分）**（或等效脚本）
6. **报告（20 分）**：4 个问题每题 5 分。未完全答出问题的给部分分。
