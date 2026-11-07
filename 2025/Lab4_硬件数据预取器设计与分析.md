# 实验 4：硬件数据预取器的设计与分析

**课程：** 计算机体系结构（Computer Architecture，227-2210-00L），2025 年秋季学期
**授课教师：** Prof. Onur Mutlu、Dr. Mohammad Sadrosadati
**助教：** Dr. Konstantina Koliogeorgi、Dr. Zhiheng Yue、Rahul Bera、Konstantinos Kanellopoulos、Nika Mansouri Ghiasi、Ataberk Olgun、Nisa Bostanci、Rakesh Nadig、Ismail Emir Yuksel、Haocong Luo、Mayank Kabra、Andreas Kosmas Kakolyris、Harshita Gupta、Konstantinos Marios Sgouras、Harsh Songara、Susana Rebolledo Ruiz
**负责答疑的助教：** Rahul Bera、Konstantinos Kanellopoulos

**布置日期：** 2025 年 11 月 20 日（星期四）
**截止日期：** 2025 年 12 月 3 日（星期三）午夜

## 1. 引言

在本实验中，你将在处理器微体系结构模拟器 **ChampSim** 中设计并评估多种硬件数据预取技术。ChampSim 完全由 C++ 编写，能够较详细地模拟现代高性能处理器核心，包括激进的**乱序执行流水线**、**三级缓存层次结构**和**基于 DRAM 的主存**。模拟器读取指令跟踪文件（instruction trace，即记录真实程序指令的文件），逐条模拟执行这些指令，从而再现处理器的行为。这种模拟方法的目标是：在不必实际制造经修改的处理器的情况下，忠实评估新想法及其对程序执行的影响（例如处理器需要执行多少周期、会访问多少次 DRAM 等）。

本实验仅关注处理器设计的一个方面：**硬件数据预取器**。硬件数据预取器会预测未来内存请求的地址，并在运行中的程序实际需要数据之前，将相关数据提前取入片上缓存 [1, 2, 3]。实验目标是设计多种数据预取机制，并理解：(1) 它们对处理器性能的影响；(2) 它们产生的额外开销所带来的影响。

为简化预取器的实现过程，ChampSim 提供了一组[接口函数](https://champsim.github.io/ChampSim/master/Legacy-modules.html#memory-prefetchers)，让预取器与模拟器的其他部分交互。你的任务是：(1) 按照各项任务的要求定义接口函数；(2) 评估得到的预取器；(3) 撰写全面的报告，总结观察结果与实验经验。

> **竞赛信息：** 本实验使用的 ChampSim 也将用于第四届[数据预取锦标赛（DPC4）](https://sites.google.com/view/dpc4-2026)。该竞赛将于 2026 年 2 月在澳大利亚悉尼举行的计算机体系结构领域旗舰会议 [HPCA](https://hpca-conf.org/2026/) 期间举办。投稿截止时间为 **12 月 14 日 23:59（AoE）**。如果感兴趣，可以将你在任务 4 中设计的预取器投稿到该竞赛，并有机会向领域专家展示你的工作。

## 2. 实验资源

### 2.1. 如何开始

以下指令均已在基于 Ubuntu 的机器上测试过，也应能直接在其他 Linux 系统中运行。如果使用 Windows 或 macOS，请尝试使用 Linux 机器，或者自行找出操作系统中对应的命令。如需帮助，请发送邮件联系我们。

首先克隆 [ChampSim 仓库](https://github.com/CMU-SAFARI/ChampSim)：

```bash
git clone https://github.com/CMU-SAFARI/ChampSim.git
```

配置构建系统：

```bash
git submodule update --init
vcpkg/bootstrap-vcpkg.sh
vcpkg/vcpkg install
```

要构建模拟器，先指定处理器配置，再执行编译：

```bash
./config.sh dpc4/1C.fullBW.nopref.json
make -j
```

其中 `dpc4/1C.fullBW.nopref.json` 是基准处理器的配置文件：处理器只有**一个核心**，包含三级缓存层次结构，但**没有任何数据预取器**。请阅读这个 JSON 文件，基本了解处理器参数是如何定义的，尤其是各级缓存中的预取器配置（`L1D`、`L2` 和 `LLC` 对象中的 `prefetcher` 属性，当前均为 `"no"`，表示未启用预取器）。编译成功后，应在 `bin` 目录中看到可执行文件 `1C.fullBW.nopref`。

运行 ChampSim 需要程序指令跟踪文件。请在 ChampSim 目录内执行以下命令。**注意：** 下载跟踪文件需要 Python 3 和约 **14 GB** 可用空间。

**✎ 提示：** 从实验说明复制 Google Drive 链接时请格外注意：下划线 `_` 字符可能无法正确复制。

```bash
pip install gdown
mkdir traces
cd traces
gdown --folder https://drive.google.com/drive/u/1/folders/1pj9Tq-lz3TEbztHUdzUoT33wzk1Yqb_r
gdown --folder https://drive.google.com/drive/u/1/folders/1ynHx_dVL4s_ipBvJ8bgE_9Fs45Qj3AuQ
```

**☞ 注意：** Google 有时会限制 Google Drive 的下载带宽。如果下载跟踪文件时出现错误，请改从[此云存储桶](https://console.cloud.google.com/storage/browser/dpc4-public-traces)下载。**只需要下载** `Graph/GAP` 和 `gtraces_v2/charlie` 目录。

上述命令应下载两个名为 `GAP` 和 `charlie` 的目录，分别包含 **9** 个和 **5** 个指令跟踪文件。`GAP` 中的跟踪来自图分析基准测试，`charlie` 中的跟踪来自 Google 数据中心运行的真实工作负载。

现在，从 ChampSim 目录执行以下命令，使用指定的指令跟踪文件模拟处理器：

```bash
./bin/1C.fullBW.nopref --warmup-instructions=10000000 --simulation-instructions=50000000 <TRACE-PATH>
```

除非另外说明，**始终先用 1,000 万条指令预热模拟器，然后再模拟接下来的 5,000 万条指令**。

**✎ 提示：** 根据指令跟踪文件、模拟处理器配置和运行机器的性能，每次模拟可能耗时 **10 分钟至 1 小时**。模拟器默认每执行 100 万条指令输出一次进度日志。可以通过 `heartbeat` 参数自定义日志频率（例如 `--heartbeat=100000`）。如果个人电脑性能不够，也可以考虑使用 ETH 的 **Euler 集群** [4] 来运行实验。

### 2.2. 重要的模拟指标

ChampSim 在模拟结束后会报告多项指标。评估预取器时值得关注的指标包括：

- `cumulative IPC`（累计 IPC）
- `cpu0 L2C LOAD ACCESSES` 和 `MISSES`（L2 加载访问次数与缺失次数）
- `cpu0 L2C PREFETCH REQUESTED`、`ISSUED`、`USEFUL` 和 `USELESS`（请求、已发出、有用、无用的预取数量）

本实验主要关注 **cumulative IPC**，该指标反映处理器性能：通常，性能更好的预取器应该带来更高的累计 IPC。

### 2.3. 如何实现预取器？

实现一个预取器大致分为两步：(1) 定义所有预取器接口函数；(2) 配置处理器，使其使用新实现的预取器。下面详细介绍这两个步骤。

#### 2.3.1. 定义预取器接口函数

假设要实现名为 `my_prefetcher` 的新预取器，请按以下步骤操作：

1. 在 `prefetcher/` 内创建目录 `my_prefetcher/`。
2. 在 `my_prefetcher/` 目录中创建 `my_prefetcher.h` 和 `my_prefetcher.cc` 两个文件。
3. 参照 `no/no.h` 中的框架，在 `my_prefetcher.h` 内定义一个空的 `my_prefetcher` 预取器类。注意，该框架声明了 **6 个接口函数**，其中只有 **2 个未被注释掉**。
4. 根据本实验具体任务的要求，在 `my_prefetcher.cc` 内适当地定义各接口函数。

各接口函数的作用如下；更详细的文档见[这里](https://champsim.github.io/ChampSim/master/Legacy-modules.html#memory-prefetchers)。

- **`prefetcher_initialize()`：** 在模拟开始、预取器初始化时调用。可用于初始化预取器类中定义的动态数据结构。
- **`prefetcher_cache_operate()`：** 每当发生一次缓存查询时调用。它提供当前查询缓存的内存请求的许多信息，例如：(1) 请求的内存地址；(2) 产生请求的加载指令的地址（即 instruction pointer，IP）；(3) 请求是否命中缓存等。这些信息在实现预取器的访问模式学习逻辑时非常有用。
- **`prefetcher_cache_fill()`：** 发生缓存填充时调用。如果希望在预取缓存行被填入缓存时执行某些操作，可使用此函数。
- **`prefetcher_cycle_operate()`：** 每个时钟周期调用。
- **`prefetcher_final_stats()`：** 模拟结束时调用。可用于输出预取器收集的统计数据。

**☞ 注意：**尽管 `no/no.h` 中展示了 6 个预取器接口函数，本实验**不需要**定义 `prefetcher_branch_operate()`。如有需要，也可以将某些接口函数实现为空操作。无论如何实现，**请勿修改接口函数原型**，否则除非同时修改模拟器其他部分，否则会引起编译错误。

**✎ 提示：** 我们提供了多种预取器的实现供参考。建议查看两种简单的预取器：(1) **下一行预取器（next-line prefetcher）**；(2) **基于表的 IP-stride 预取器**。通过阅读它们，可以全面理解接口函数如何实现，以及如何将生成的预取请求注入缓存层次结构（注意 `prefetch_line()` 函数）。

#### 2.3.2. 配置处理器以使用新预取器

实现 `my_prefetcher` 后，需要将其连接到处理器，更准确地说，是连接到处理器的某一级缓存。步骤如下：

1. 复制 `dpc4/1C.fullBW.nopref.json`，新建一个处理器配置文件。
2. 将其中 `L2C` 对象的 `prefetcher` 属性从 `"no"` 改成 `"my_prefetcher"`。
3. 将 `"executable_name"` 属性从原来的基准可执行文件名称修改为自选名称（例如 `"1C.fullBW.mypref"`）。
4. 使用前面介绍的命令重新构建模拟器（先运行 `config.sh`，再运行 `make`）。编译后，`bin` 目录中将出现新的模拟器可执行文件，该处理器配置此时已经启用新预取器。

**☞ 注意：** 从技术上来说，通过修改对应级别缓存的 `prefetcher` 属性，你可以把预取器接入任意缓存层次（例如 L1D、L2C 或 LLC）。**但本实验要求设计的预取器只接入 L2C**。

## 3. 实验任务

本节规定各项任务，请仔细阅读。每个任务都有**最低交付要求**：为获得该任务的分数，报告中必须包含这些内容。同时，也鼓励在报告中加入其他有趣的观察结果。

### 3.1. 预热任务：运行不带预取器的基准配置

首先使用基准处理器配置 `1C.fullBW.nopref.json`，运行**全部 14 个指令跟踪文件**，并报告各自的累计 IPC 指标。

**最低交付要求。** 绘制一张柱状图：横轴是各工作负载，纵轴是 IPC。在每根柱子的顶部标明 IPC 数值，并指出 IPC **最高**和**最低**的工作负载名称。

### 3.2. 任务 1/4：实现基于 GHB 的步幅预取器

下一个任务是实现**基于全局历史缓冲区（GHB）的步幅预取器**。请阅读 GHB 预取器论文 [5]。该预取器包含两种主要结构：(1) **索引表（Index Table，IT）**；(2) **全局历史缓冲区（Global History Buffer，GHB）**。你需要实现论文第 3.2 节介绍的 **PC/CS** 方案。

**预取算法。** 对每次 L2 缓存访问（包括命中和缺失），算法使用此次访问的 PC 查询 IT，并将缓存行地址（设为 \(A\)）插入 GHB。利用 PC 和 GHB 条目中的链接指针，算法检索**同一 PC 最近 3 次访问 L2 时的地址序列**。通过连续两个地址的差值计算步幅。如果两个步幅相等（设为 \(d\)），预取器便向以下缓存行发出预取请求：

\[
A + ld,\quad A+(l+1)d,\quad A+(l+2)d,\quad \ldots,\quad A+(l+n)d
\]

其中 \(l\) 表示**预取距离（prefetch distance）**，\(n\) 表示**预取度（prefetch degree）**。本设计中，应静态设定 **\(l=4\)、\(n=6\)**；IT 和 GHB 分别设为 **256 个条目**。

**最低交付要求。** 绘制柱状图，横轴为工作负载，纵轴为各工作负载的**加速比（speedup）**。加速比定义为：

\[
\mathrm{Speedup}=\frac{\mathrm{IPC}_{\text{步幅预取器}}}{\mathrm{IPC}_{\text{基准配置}}}
\]

加速比**大于 1** 表示预取器提高了处理器性能，**小于 1** 表示导致性能下降。新增一根标为 `GEOMEAN` 的柱子，显示 14 个工作负载加速比的**几何平均值**。在每根柱顶标注数值，并指出加速比最高和最低的工作负载。

### 3.3. 任务 2/4：主存带宽受限情况下的预取

此任务要求在主存带宽受限的处理器配置中评估任务 1 实现的 GHB 预取器。需要对**全部 14 个工作负载**分别执行两项实验。

**第一项实验：**在主存带宽受限的情况下运行基准处理器。使用 `dpc4/1C.limitBW.nopref.json` 重新配置模拟器。该配置在其他所有方面都与 `1C.fullBW.nopref.json` 相同，唯一的区别是其**主存带宽只有后者的 1/6**。编译模拟器，运行全部跟踪文件，记录各自的 IPC。

**第二项实验：** 在受限主存带宽下运行任务 1 所实现的 GHB 预取器。复制 `dpc4/1C.limitBW.nopref.json`，将 `L2C` 的 `prefetcher` 属性值替换成你的 GHB 预取器名称。利用新配置重新配置并编译模拟器，运行所有跟踪文件，计算各工作负载的 IPC 加速比。此处加速比定义为：**受限带宽下启用 GHB 预取器的 IPC / 受限带宽下基准处理器的 IPC**（即第一项实验记录的 IPC）。

**最低交付要求。** 绘制柱状图，横轴为工作负载，纵轴为各自的加速比。加入一根标为 `GEOMEAN` 的柱，表示 14 个工作负载加速比的几何平均值。柱顶标注相应数值。此外回答：

1. 此任务的几何平均 IPC 性能提升与任务 1 相比有何变化？
2. 此任务中加速比最高与最低的是哪些工作负载？
3. 从任务 1 到任务 2，哪些工作负载的加速比**变化最大**和**变化最小**？
4. 是否存在加速比发生**反转**的工作负载？例如任务 1 中大于 1，但任务 2 中小于 1；或者反过来。

### 3.4. 任务 3/4：具有系统感知能力的预取器设计

在任务 2 中，你很可能会观察到：当主存带宽受限时，GHB 预取器带来的加速比下降。这是因为预取器产生的内存请求会消耗稀缺的主存带宽；特别是当预取不准确时（预取的数据之后并未被处理器实际需要），这些请求会占用本可用于处理器需求请求的带宽。

本任务要求扩展 GHB 预取器，使它具备**系统感知能力（system awareness）**。核心思想是：根据**预取准确率**（已发出的预取请求中有多少后来被处理器真正用到）和**主存带宽使用率**，动态调整任务 1 实现的 GHB 预取器的**预取度**。为此需要访问三项系统级信息：(1) 预取器发出的预取请求数量；(2) 后来被处理器使用的预取请求数量；(3) 处理器的内存带宽使用情况。

前两项信息可通过缓存维护的两个统计计数器 `pf_issued` 和 `pf_useful` 读取。[^lab4-1] 主存带宽使用率可以通过调用 `get_dram_bw()` 获取；该函数返回使用的主存带宽占峰值带宽的比例。[^lab4-2]

请按下列方法实现系统感知逻辑：

1. 以 **1,000 个时钟周期**为一个时间窗口（epoch），跟踪每个 epoch 内发出的预取请求总数和有用预取请求数。
2. 每个 epoch 结束时（即 1,000 个周期后）：(1) 计算该 epoch 的预取准确率（**有用预取数 / 已发出预取数**）；(2) 测量当前主存带宽使用率。
3. 按照**表 1**，依据计算出的准确率及主存带宽使用率，调整**下一 epoch** 的预取度。**预取度绝不能高于 6，也不能低于 1。**

**表 1：根据预取准确率和主存带宽使用率（峰值带宽的比例）选择预取度的规则**

| 主存带宽使用率（BW）        | 预取准确率               | 预取度调整 |
| --------------------------- | ------------------------ | ---------- |
| \(\mathrm{BW}\ge75\%\)      | \(\ge90\%\)              | 不变       |
| \(\mathrm{BW}\ge75\%\)      | \(\ge50\%\) 且 \(<90\%\) | 减少 1     |
| \(\mathrm{BW}\ge75\%\)      | \(<50\%\)                | 减少 2     |
| \(25\%\le\mathrm{BW}<75\%\) | \(\ge90\%\)              | 增加 1     |
| \(25\%\le\mathrm{BW}<75\%\) | \(\ge50\%\) 且 \(<90\%\) | 不变       |
| \(25\%\le\mathrm{BW}<75\%\) | \(<50\%\)                | 减少 1     |
| \(\mathrm{BW}<25\%\)        | \(\ge90\%\)              | 增加 2     |
| \(\mathrm{BW}<25\%\)        | \(\ge50\%\) 且 \(<90\%\) | 增加 1     |
| \(\mathrm{BW}<25\%\)        | \(<50\%\)                | 不变       |

**最低交付要求。** 在以下两种系统配置中，与**未启用预取器的基准处理器**比较你的预取器性能：(1) **完整带宽**（预热任务中运行的基准实验）；(2) **受限带宽**（任务 2 第一项实验的结果）。分别为完整带宽和受限带宽绘制两张柱状图。每张图的横轴为工作负载，纵轴为相应带宽配置下的加速比。

例如，完整带宽下工作负载 A 的加速比为：\(\mathrm{IPC}_{\text{A, 有预取, 完整带宽}}/\mathrm{IPC}_{\text{A, 无预取, 完整带宽}}\)。受限带宽下的计算方法类似：\(\mathrm{IPC}_{\text{A, 有预取, 受限带宽}}/\mathrm{IPC}_{\text{A, 无预取, 受限带宽}}\)。两张图都需要有单独的 `GEOMEAN` 柱。

**系统感知机制是否有助于改善预取器性能？**

> **💡 延伸思考：**表 1 中的阈值基本上都是凭经验设定的，并在设计阶段固定下来。虽然可以通过大量实验调节这些阈值，选择一组能在若干工作负载上取得最佳性能的数值，但调节完成后就不能再改变。这种僵化限制了预取器设计：同一组阈值未必适用于各种工作负载和系统配置。如果预取器可以在运行期间观察阈值变化对性能的影响，从而**动态调整阈值**，是不是更好？可以利用元启发式搜索方法，例如**随机爬山算法** [6]、**模拟退火** [7]、**随机游走** [8]，甚至**强化学习** [9]，来实现这一想法。你可以在下一任务中选择设计这样的预取器！

### 3.5. 任务 4/4：设计自己的预取器

本实验的第四项也是最后一项任务，是**自行设计一种数据预取器**。可以从文献 [1, 10, 11, 12, 13, 14, 15, 16, 17, 3, 18] 中已有的数据预取机制获得启发。此任务**不要求限制硬件开销或设计复杂度**。

**最低交付要求。**

1. 简要描述你设计的预取算法。
2. **扩展上一任务中的两张柱状图。** 在第一张（完整带宽）中，添加你的预取器相对于完整带宽基准处理器在各个工作负载以及总体平均（几何平均）上的加速比。在第二张（受限带宽）中，加入相对于受限带宽基准处理器的加速比。

### 3.6. 附加任务（+20%）：与先进预取器比较

附加任务的目标是将之前设计的全部预取器与一种先进预取器 **Pythia** [18] 进行定量比较。Pythia 是一种利用**强化学习**自适应生成预取请求的硬件数据预取器。为方便完成任务，我们已经在仓库中提供了 Pythia 的源代码。完成本任务可获得本实验总分的**额外 20%**。

**最低交付要求。** 绘制两张柱状图，均在之前任务的图表上扩展：

1. **完整带宽：** 展示 Pythia 相对完整主存带宽的无预取基准处理器，在各个工作负载及总体（几何平均）上的加速比。请在任务 3 的完整带宽柱状图中加入 Pythia 的数据。
2. **受限带宽：** 同样展示 Pythia 相对于受限主存带宽的无预取基准处理器的加速比。请在任务 3 的受限带宽柱状图中加入 Pythia 的数据。

## 4. 提交与评估

### 4.1. 提交要求

通过 Moodle 的[对应作业页面](https://moodle-app2.let.ethz.ch/mod/assign/view.php?id=1306626)提交。提交内容至少包括：(1) 正确组织在 `prefetcher/` 目录中的全部预取器代码；(2) **PDF 格式的实验报告**。**请注意：如果没有报告，你可能无法获得本实验的任何分数。**

请按以下步骤准备提交：

- 在 ChampSim 根目录执行 `make clean`，删除编译器生成的文件。
- 将预取器代码放入 `prefetcher/` 目录。
- 将所有配置文件也放入 `prefetcher/` 目录。
- 将报告放在 ChampSim 根目录，并命名为 **`report.pdf`**。
- **确保 ChampSim 目录内没有下载的指令跟踪文件**，否则提交包体积会被不必要地增大。
- 将 ChampSim 仓库打包为**一个 tar 归档文件**，排除 `traces/` 目录，并命名为 `lab4_YourSurname_YourName.tar.gz`。

### 4.2. 评估方式

提交成果将根据以下方面评估：报告质量（非常鼓励在最低要求之外增加其他观察和见解）、预取器代码质量（逻辑正确性与可读性）、任务 4 中自行设计的预取器，以及可选附加任务。对于任务 4 中具有**创新性**的预取器，将给予特别加分。

## 5. 建议

- 如有需要，请在 Moodle 的[在线问答论坛](https://moodle-app2.let.ethz.ch/mod/moodleoverflow/view.php?id=1306628)向助教提问。
- 遇到技术问题时，请首先阅读错误消息；网络搜索通常可以解决许多调试问题和报错。
- 调试自己设计的预取器时，一个简单办法是使用**内存地址访问模式规律**的工作负载。可以从[此链接下载示例指令跟踪文件 `462.libquantum-714B.champsimtrace.xz`](https://dpc3.compas.cs.stonybrook.edu/champsim-traces/speccpu/462.libquantum-714B.champsimtrace.xz)，并按照第 2.1 节的方法用 ChampSim 进行模拟。该跟踪来自基于 C 语言的量子计算模拟库 **libquantum** [19]。该库提供了表示量子寄存器及一些基本量子门的数据结构；其程序会遍历一个 **32 MB 的整数数组**。这种访问模式在每个物理页内产生步幅恒为 **+1** 的缓存行地址序列，因此适合测试预取器。

## 参考文献

[1] John W. C. Fu, Janak H. Patel, and Bob L. Janssens. *Stride Directed Prefetching in Scalar Processors*. MICRO, 1992.

[2] Tien-Fu Chen and Jean-Loup Baer. *Effective Hardware-Based Data Prefetching for High-Performance Processors*. IEEE TC, 1995.

[3] Rahul Bera, Anant V. Nori, Onur Mutlu, and Sreenivas Subramoney. *DSPatch: Dual Spatial Pattern Prefetcher*. MICRO, 2019.

[4] ETH Zurich Euler Cluster. [集群入门指南](https://scicomp.ethz.ch/wiki/Getting_started_with_clusters).

[5] Kyle J. Nesbit and James E. Smith. *Data Cache Prefetching Using a Global History Buffer*. HPCA, 2004.

[6] Ari Juels and Martin Wattenberg. *Stochastic Hillclimbing as a Baseline Method for Evaluating Genetic Algorithms*. NIPS, 1995.

[7] Peter J. M. van Laarhoven and Emile H. L. Aarts. *Simulated Annealing*. 1987.

[8] Samuel Karlin and James McGregor. *Random Walks*. Illinois Journal of Mathematics, 1959.

[9] Richard S. Sutton, Andrew G. Barto, et al. *Reinforcement Learning: An Introduction*. MIT Press, Cambridge, 1998.

[10] Stephen Somogyi, Thomas F. Wenisch, Anastassia Ailamaki, Babak Falsafi, and Andreas Moshovos. *Spatial Memory Streaming*. ISCA, 2006.

[11] Mohammad Bakhshalipour, Mehran Shakerinava, Pejman Lotfi-Kamran, and Hamid Sarbazi-Azad. *Bingo Spatial Data Prefetcher*. HPCA, 2019.

[12] Jinchun Kim, Seth H. Pugsley, Paul V. Gratz, A. L. Reddy, Chris Wilkerson, and Zeshan Chishti. *Path Confidence Based Lookahead Prefetching*. MICRO, 2016.

[13] Manjunath Shevgoor, Sahil Koladiya, Rajeev Balasubramonian, Chris Wilkerson, Seth H. Pugsley, and Zeshan Chishti. *Efficiently Prefetching Complex Address Patterns*. MICRO, 2015.

[14] Yasuo Ishii, Mary Inaba, and Kei Hiraki. *Access Map Pattern Matching for Data Cache Prefetch*. ISC, 2009.

[15] Pierre Michaud. *Best-Offset Hardware Prefetching*. HPCA, 2016.

[16] *Disclosure of Hardware Prefetcher Control on Some Intel® Processors*. [Intel 相关文档](https://software.intel.com/en-us/articles/disclosure-of-hw-prefetcher-control-on-some-intel-processors).

[17] Santhosh Srinath, Onur Mutlu, Hyesoon Kim, and Yale N. Patt. *Feedback Directed Prefetching: Improving the Performance and Bandwidth-Efficiency of Hardware Prefetchers*. HPCA, 2007.

[18] Rahul Bera, Konstantinos Kanellopoulos, Anant V. Nori, Taha Shahroodi, Sreenivas Subramoney, and Onur Mutlu. *Pythia: A Customizable Hardware Prefetching Framework Using Online Reinforcement Learning*. MICRO, 2021.

[19] *libquantum — The C Library for Quantum Computing and Quantum Simulation*. [http://www.libquantum.de](http://www.libquantum.de).

[^lab4-1]: 提示：可以使用 `intern_` 指针访问预取器所连接的缓存对象。请查看 `ip_stride` 预取器的实现。
    
[^lab4-2]: 关于该函数的更多信息，请参阅 `inc/dpc_api.h`。
