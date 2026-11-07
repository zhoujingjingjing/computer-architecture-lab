# 附加实验 Lab 6：高级虚拟内存特性

> **计算机体系结构（227-2210-00L）｜2025 年秋季**
> **授课教师：** Prof. Onur Mutlu、Dr. Mohammad Sadrosadati
> **助教（TAs）：** Dr. Konstantina Koliogeorgi、Dr. Zhiheng Yue、Rahul Bera、Konstantinos Kanellopoulos、Nika Mansouri Ghiasi、Ataberk Olgun、Nisa Bostanci、Rakesh Nadig、Ismail Emir Yuksel、Haocong Luo、Mayank Kabra、Andreas Kosmas Kakolyris、Harshita Gupta、Konstantinos Marios Sgouras、Harsh Songara、Susana Rebolledo Ruiz、Spiros Galanopoulos
> **负责答疑的助教：** Konstantinos Kanellopoulos、Spiros Galanopoulos
> **布置日期：** 2025 年 12 月 9 日（星期二）
> **截止日期：** 2026 年 1 月 9 日（星期五）午夜

## 1. 引言

本实验将探索虚拟内存（Virtual Memory，VM）子系统的不同特性，以更深入理解其在系统软件和硬件层面的功能。我们将使用 Virtuoso [1]（集成 Sniper [2]）这一体系结构模拟器。它能够模拟操作系统的功能，以及为虚拟内存子系统提供支持的硬件组件。

你将研究两类横跨虚拟内存子系统不同层次的技术：

- **（硬件层面）页表项替换策略：** 页表是操作系统用来将虚拟地址映射到物理地址的数据结构。每个进程都有独立的页表，保存在物理内存中。CPU 访问内存时使用页表完成地址转换。为了降低转换延迟，页表数据会被缓存在 TLB（地址转换后备缓冲器）中；但 TLB 容量有限，满时必须驱逐条目。在此任务中，你将设计并实现 L2 缓存替换策略：根据运行时的需要，优先将页表项（PTE）而不是普通数据缓存行保留在 L2 缓存内。
- **（跨层）页表放置策略：** 操作系统负责管理页表，并决定页表在物理内存中的位置。页表放置方式会显著影响虚拟内存子系统的性能。此任务要求首先 (i) 分析传统页表放置策略对系统性能的影响，然后 (ii) 设计并实现新的页表放置策略，改善多个应用并发运行时的公平性。

## 2. 实验资源

### 2.1. 入门准备

运行模拟器需要基于 Ubuntu 的机器（当前模拟器尚不兼容采用 ARM 指令集的较新 macOS 版本）。如果希望在 macOS 上使用模拟器，可以创建一台安装 Ubuntu 的虚拟机。建议通过 [QEMU macOS 下载页面](https://www.qemu.org/download/#macos) 创建 Ubuntu 20.04 LTS 或更新版本的实例。

克隆 Virtuoso（集成了 Sniper [2]）仓库：

```bash
git clone https://gitlab.ethz.ch/kanellok/virtuoso_foca
git checkout comp_arch_lab6_2025
```

安装 Virtuoso 的依赖：

```bash
sh install_dependencies.sh
```

运行用于清理并构建模拟器的 `build_virtuoso.sh`。使用以下命令编译 Virtuoso 和 Sniper。首次完成构建后，可以用 `incremental_compile.sh` 仅编译发生变化的文件。如果想从头重新编译，请使用 `build_virtuoso.sh` 而不是增量脚本。

```bash
./build_virtuoso.sh
./incremental_compile.sh
```

测试 Virtuoso 是否成功编译：

```bash
sh ./run_example.sh
```

下面进一步说明如何调用模拟器及执行实验：

```bash
./run-sniper -c ./configuration_file.cfg -d ./example_output --genstats \
  -s stop-by-icount:1000000 -- <workload>
```

命令中各参数含义如下：

- **`-c ./configuration_file.cfg`：** 指定仿真的配置文件。配置文件包含控制模拟器行为的参数，例如核心数量、存储层次结构、TLB 配置等。
- **`-d ./example_output`：** 指定仿真结果输出目录。目录不存在时模拟器会自动创建。
- **`--genstats`：** 让模拟器在仿真结束时生成统计信息；统计文件 `sim.stats` 会保存在 `-d` 所指定的目录中。
- **`-s stop-by-icount:1000000`：** 执行 1,000,000 条指令后停止仿真。适用于调试、测试，或短时间验证某项功能，而不必运行完整工作负载。
- **`-- <workload>`：** 指定仿真期间运行的工作负载，可以是希望模拟的任意程序或基准测试。

### 2.2. 模拟的基线配置

本实验使用的模拟系统基线配置存储于：

```text
./config/virtuoso_configs/virtuoso_reserve_thp.cfg
```

*注意：该配置文件还会包含其他配置文件。*

- **内存容量：** 48 GB；
- **内核空间容量：** 32 GB（48 GB 总内存中有 32 GB 被内核空间占用，用户空间余下 16 GB）；
- **L1 TLB：** 64 个条目，4 路组相联；
- **L2 TLB：** 512 个条目，8 路组相联；
- **内存碎片化程度（Memory Fragmentation）：** 从 0.0%（完全碎片化）到 100%（无碎片化）。设置 0.0% 时无法分配任何 2 MB 页；设置 100% 时则允许分配全部 2 MB 页。
- **每个实验至少模拟 2000 万条指令**。
- 建议用 Bash 或 Python 脚本自动化：(i) 批量启动实验；(ii) 分析实验结果。

相关组件的详细介绍参见 [Virtuoso 文档](https://cmu-safari.github.io/Virtuoso/)；Sniper 的使用手册见 [Sniper Manual（PDF）](https://snipersim.org/documents/sniper-manual.pdf)。

你将主要接触三个子系统：

- **TLB 子系统：** 实现包括 L1、L2 TLB 在内的 TLB 层次结构。
- **内存管理单元（Memory Management Unit，MMU）：** 包括硬件页表遍历器（page table walker）、页表遍历缓存（page walk caches）及 TLB 子系统。
- **物理内存分配器（Physical Memory Allocator）：** 为大页实现一种类似文献 [3] 的基于预留区域的内存分配器。

## 3. 任务 1：感知页面大小的替换策略（45 分）

### 3.1. 目标

回顾虚拟内存课程内容，TLB（Translation Lookaside Buffer，地址转换后备缓冲器）是保存最近虚拟地址到物理地址转换结果的缓存。现代系统使用不止一种页面大小，以提升性能并降低 TLB 未命中率。例如，较新的 Linux 内核同时支持 4 KB 与 2 MB 页面，后者通常用于大型内存分配，以减少 TLB 压力。

TLB 条目的价值在很大程度上取决于其覆盖的页面大小。一个 2 MB 页的 TLB 条目所覆盖的地址空间，相当于 **512 个 4 KB 页的条目**。因此，从 TLB 驱逐一个 2 MB 条目，对 TLB 的**内存覆盖范围（Memory Reach）**造成的损失远大于驱逐 4 KB 条目。传统替换策略，例如最近最少使用（LRU），只基于最近访问时间平等对待所有条目，忽略转换结果的“权重”或覆盖范围。

此任务的目标是：(i) 理解不同页面大小对性能的影响；(ii) 设计能够体现大页价值的 TLB 替换策略；(iii) 协同设计末级缓存（Last Level Cache，LLC）的替换策略，降低 TLB 未命中的开销。

### 3.2. 任务 1.1：分析页面大小对地址转换性能的影响

本任务分析不同页面大小（如 4 KB 与 2 MB）在映射至对应大小物理页框时，对 L2 TLB 性能（命中率及内存覆盖范围）的影响。

完成以下步骤：

1. **设计微基准测试程序（microbenchmark）。** 通过访问大量虚拟地址对虚拟内存子系统施加压力。你可自行设计程序（提示：可参考稀疏矩阵—向量乘法或包含大量指针追踪的基准测试），但必须确保 **L2 TLB 每千条指令未命中数（MPKI）大于 10**，以保证程序确实给 L2 TLB 带来压力。建议至少模拟 2000 万条指令。可以从课程提供的 `microbenchmarks/` 模板开始，其中有记录微基准运行轨迹、并将其用于仿真的说明。**重要：微基准必须使用超过 8 GB 的内存，以确保 L2 TLB 承受足够压力。**
2. 使用 Virtuoso 以 **4 KB 页**运行微基准，收集 L2 TLB 命中率、总执行周期数，以及页表遍历耗费的周期数（即 TLB 未命中开销）。
3. 在同一个模拟系统中同时启用 **4 KB 和 2 MB 页**，再次运行微基准。除了前面的指标，还要收集 **L2 TLB 内存覆盖范围的时间序列**，即随时间变化、TLB 所覆盖的内存总量。原文此处提及跨越 10,000 条指令的 epoch。
4. 分析结果，理解不同页面大小如何影响系统性能。

> **备用方案：** 如果无法自行设计微基准，可在整个实验中使用课程提供的轨迹目录：`traces/randacc_8gb.sift`。

**实验报告必须包含：**

- 对微基准的描述，以及说明它为什么会对虚拟内存子系统形成压力；
- 对比两种运行条件（仅 4 KB 页、4 KB + 2 MB 页）的 L2 TLB 命中率、总执行周期与页表遍历周期的图；
- L2 TLB 内存覆盖范围的时间序列图，展示引入 2 MB 页后，每 **100,000 条指令**区间内的变化（此处区间大小沿用原文报告要求）。

### 3.3. 任务 1.2：设计感知页面大小的 TLB 替换策略

标准替换策略（如 LRU 或 SRRIP）可能为了接纳 4 KB 页条目而驱逐更有价值的 2 MB 页条目，从而大幅降低 TLB 的有效内存覆盖范围。本任务要求设计一种更倾向保留大页条目的替换策略。

**要求：**

- 修改 L2 TLB 逻辑，实现**优先保留 2 MB 页、而不是 4 KB 页**的替换策略。
- 调整物理内存分配器的碎片化程度，使分配的 4 KB 与 2 MB 页形成相对“均衡”的组合，例如各约占 50%。
- 可以基于模拟器已有的 LRU、SRRIP 或其他策略构建，例如让 2 MB 页更“黏着”（不易被驱逐），或者老化得更慢；也可以从头设计新策略。
- 示例：只有确实必要时（如整个组全是 2 MB 页，或者某个 2 MB 页很久未被使用）才允许驱逐 2 MB 页条目。

### 3.4. 任务 1.3：协同设计 LLC 替换策略

提高 L2 TLB 命中率十分重要，但未命中仍然不可避免。当 L2 TLB 未命中时，硬件页表遍历器必须从内存中取得页表项（PTE），如果页表遍历缓存中已有对应条目，也可能从中获取。如果这些 PTE 可以在末级缓存（LLC）中找到，页表遍历的代价就会显著降低。

你必须在 LLC 中设计一种机制，优先保留 TLB 当前缺失的地址转换所需要的 PTE。

**要求：**

- 修改 LLC 替换策略，区分普通数据请求和硬件页表遍历器发出的请求。
- 该策略应优先缓存页表遍历器访问的 PTE，而不是普通数据，因为它们对应 TLB 中不存在的地址转换。
- 必须保证这种优先级不会让普通数据访问完全得不到服务，否则会降低系统整体性能。
- **最难的部分**是判断 LLC 中的 PTE 是否对应当前 TLB 中缺失的页面。**提示：** 可维护一个小型结构（例如布隆过滤器或概率型硬件结构），记录从 TLB 驱逐的页面的物理／虚拟地址，以跟踪地址转换是否驻留在 TLB 中。当 LLC 接收到页表遍历请求时，就可以检查此结构，判断对应页面是否目前不在 TLB 中。

### 3.5. 任务 1.4：实现与评估

1. **TLB 实现：** 扩展 L2 TLB，加入感知页面大小的替换策略。L2 TLB 的实现在 `simulator/sniper/common/core/memory_subsystem/parametric_dram_directory_msi/translation_components/` 中。
2. **LLC 实现：** 扩展 LLC，加入优先保留 PTE 的替换策略。可以检查请求类型来识别来自页表遍历器的请求。
3. **评估：** 使用新设计的组合 TLB + LLC 策略，执行任务 1.1 的微基准（或提供的轨迹）。

**实验报告必须包含：**

- **TLB 替换算法的详细描述：** 如何选择驱逐对象？如何防止 4 KB 页长期得不到保留机会？
- **LLC 协同设计与替换策略的描述：** LLC 如何确定应该优先保留哪些 PTE？如何平衡 PTE 与普通数据？
- **量化比较：** 将新设计的 TLB 命中率、总执行周期，与基线（TLB 和 LLC 的默认替换策略）进行比较。

**重点文件**（可搜索 `CompArch` 或 `Lab 6`，定位相关代码）：

| 文件                             | 用途                                                     |
| -------------------------------- | -------------------------------------------------------- |
| `cache_set_tlb_template.cc`    | TLB 组的实现；在此修改替换逻辑                           |
| `cache_set_llc_template.cc`    | LLC 组的实现；在此修改替换逻辑                           |
| `cache_set_srrip.cc`           | SRRIP 替换策略实现；可以以此为基础构建 TLB／LLC 策略 [4] |
| `tlb.cc`                       | TLB 实现；在此修改替换逻辑                               |
| `virtuoso_pagetable_aware.cfg` | DRAM 中页表感知放置策略的配置                            |
| `virtuoso_tlb_replacement.cfg` | TLB 替换策略的配置（按需修改）                           |
| `virtuoso_llc_replacement.cfg` | 应为新的 LLC 替换策略创建配置文件并在此指定              |

## 4. 任务 2：感知 DRAM 的页表放置（40 分）

### 4.1. 目标

优化替换策略（任务 1）虽然能减少页表遍历次数，却不能将它们完全消除。当发生需要访问内存的 TLB 未命中时，硬件页表遍历器必须向 DRAM 发送请求，读取页表项（PTE）。

**DRAM 行缓冲区问题：** DRAM 按 Rank、Bank、行、列组织。读取数据时，必须先将对应行“激活”（打开）到行缓冲区（Row Buffer）中。

- **行缓冲区命中（Row Buffer Hit）：** 访问已打开行中的数据较快（约 **15—30 ns**）。
- **行缓冲区冲突／未命中（Row Buffer Conflict/Miss）：** 访问已关闭的行，或同一 Bank 中另一行时，需要先对 Bank 预充电，再激活新行，耗时较长（约 **40—60 ns**）。

在标准系统中，操作系统可能不加区分地从同一个空闲列表中给页表和应用程序数据分配物理页框。这样一来，页表项可能位于 DRAM Bank 0 的 Row 100，而它所指向的应用数据位于同一 Bank 的 Row 200。执行页表遍历时，内存控制器可能不得不关闭 Row 200（数据），打开 Row 100（PTE），随即关闭 Row 100，再打开 Row 200 来服务实际指令。这种行缓冲区的来回切换（*thrashing*）会显著增加有效内存访问延迟。

本任务的目标是设计一种**页表隔离策略（Page Table Isolation Policy）**：修改物理页框分配器，将页表放到 DRAM 中的专用区域（例如特定 Bank 或行），使其避免与普通应用数据争用行缓冲区。

### 4.2. 任务 2.1：分析地址映射方案

实现页表放置策略前，必须先理解模拟器如何将物理地址映射到 DRAM 坐标：Channel、Rank、Bank、Row、Column。

1. 检查模拟器代码中的内存控制器配置和地址映射函数，确定物理地址的哪些位用于 **Bank 选择器**，哪些位用于 **Row 选择器**。
2. 计算映射到特定 DRAM Bank 的物理地址范围。例如，如果系统有 8 个 Bank，确定哪些物理地址会落在 Bank 7。

### 4.3. 任务 2.2：实现感知 DRAM 的页表分配

你需要修改 Virtuoso 所模拟的操作系统物理页框分配器，以强制执行隔离。

**要求：**

- **保留区域（Reserved Region）：** 指定 DRAM 中的一部分区域（例如每个 Rank 的最后一个 Bank），专门用于页表页面。
- **拆分分配逻辑（Split Allocation Logic）：**
  - 操作系统需要分配保存 PTE 的页面（供页表遍历器使用）时，必须从**保留区域**分配物理页。
  - 需要为应用程序数据（栈、堆、代码）分配页面时，必须从**普通区域（Standard Region）**分配。
- **目标：** 让数据访问流和页表遍历访问流通常进入不同 Bank，能够利用**Bank 级并行性（Bank-Level Parallelism）**并行执行，避免行缓冲区冲突。

> **提示：** 最简单的方法是“Bank 隔离”。如果 DRAM 有 8 个 Bank（编号 0—7），可以将全部页表放在 Bank 7，全部普通数据放在 Bank 0—6。为此，需要在分配物理页时控制与 Bank 选择器对应的物理地址位。一种简单做法是建立一个仅包含 Bank 7 空闲页面的列表，每次需要页表页时从中分配。

### 4.4. 任务 2.3：评估

1. **实现：** 修改内存分配器逻辑。在仿真框架中找到负责分配物理页的函数（通常在 MMU／操作系统仿真文件中，名称可能类似 `allocate_physical_page`）。
2. **测试：** 运行任务 1.1 中的相同微基准。该基准具有很高的 TLB 压力，会在数据访问间穿插频繁的页表遍历。此实验中要调整物理内存分配器的碎片化程度，使系统**只使用 4 KB 页面**（即碎片化设为 `0.0`）。
3. **指标：** 分别收集以下请求的 **DRAM 行缓冲区命中率**：
   - 标记为 `PAGE_TABLE` 的请求（页表遍历）；
   - 标记为 `NON_PAGE_TABLE` 的请求（普通数据）；
   - 可在 `cache_block_info.h` 中查看请求类型。

**实验报告必须包含：**

- 仿真使用的 **物理地址 → DRAM 地址**映射图或文字说明（Bank／Row 对应哪些物理地址位）；
- 展示如何修改分配器，将页表与普通数据分离的代码片段；
- **量化比较：**
  1. **基线：** 混合分配时，页表遍历请求与数据请求各自的行缓冲区命中率；
  2. **你的设计：** 隔离分配时，页表遍历请求与数据请求各自的行缓冲区命中率。
- 应用程序的整体性能（IPC）是否提高？为什么？如果页表集中在一个 Bank 中形成“热点 Bank”，是否引入了新的瓶颈？

**重点文件：**

| 文件                               | 用途                                             |
| ---------------------------------- | ------------------------------------------------ |
| `dram_perf_model_detailed.cc.cc` | DRAM 逻辑（在此检查地址映射）                    |
| `ddr4_2400.cfg`                  | DRAM 参数配置（Bank、Rank 数量等）               |
| `page_table_aware.cfg`           | DRAM 中页表感知放置策略的配置                    |
| `pagetable_aware_allocator.cc`   | 物理页框分配器模板（本任务需要修改）             |
| `page_fault_handler.cc`          | 缺页处理逻辑（取决于分配器的实现，可能需要修改） |

## 5. 提示

- 如有需要，通过 [Moodle 在线问答论坛](https://moodle-app2.let.ethz.ch/mod/moodleoverflow/view.php?id=1315279) 向助教提问。
- 遇到技术问题时，先阅读错误信息；网络搜索通常能解决许多调试问题和报错。

### 5.1. 阅读统计信息

模拟器结束时会生成 `sim.stats` 文件，包含整个仿真过程中收集的统计数据。以下是一些特别值得关注的指标：

| 统计项（按原文名称保留）                   | 含义                                            |
| ------------------------------------------ | ----------------------------------------------- |
| `mmu.total_table_walk_latency`           | 仿真期间全部页表遍历的总延迟                    |
| `TLB_L2_1.miss`                          | L2 TLB 未命中次数（即进行的页表遍历次数）       |
| `TLB_L2_1.hit`                           | L2 TLB 命中次数                                 |
| `mmu.total_translation_latency`          | 仿真期间所有地址转换的总延迟                    |
| `performance_model.cycle_count`          | 仿真运行所经历的总周期数                        |
| `ddr.page-conflict-data-to-data`         | 两个数据请求之间的 DRAM 行冲突次数              |
| `ddr.page-conflict-data-to-metadata`     | 数据请求与元数据请求之间的 DRAM 行冲突次数      |
| `ddr.page-conflict-metadata-to-data`     | 元数据请求与数据请求之间的 DRAM 行冲突次数      |
| `ddr.page-conflict-metadata-to-metadata` | 两个元数据请求（页表项）之间的 DRAM 行冲突次数  |
| `ddr.page-hits`                          | DRAM 行命中次数（访问当前打开的行）             |
| `ddr.page-miss`                          | DRAM 行未命中事件数（访问未打开行并造成行冲突） |
| `ddr.page-empty`                         | DRAM 行为空事件数（访问当前未打开的行）         |
| `L1-D.loads-where-page-table-dram-local` | 加载请求导致从本地 DRAM 取得页表项的次数        |
| `L1-D.loads-where-page-table-L2`         | 加载请求导致从 L2 缓存取得页表项的次数          |
| `L1-D.loads-where-page-table-nuca-cache` | 加载请求导致从 NUCA 缓存取得页表项的次数        |
| `L1-D.loads-where-dram-local`            | 加载请求导致从本地 DRAM 取得数据的次数          |
| `L1-D.loads-where-L1`                    | 加载请求导致从 L1 缓存取得数据的次数            |
| `L1-D.loads-where-L2`                    | 加载请求导致从 L2 缓存取得数据的次数            |
| `L1-D.loads-where-nuca-cache`            | 加载请求导致从 NUCA 缓存取得数据的次数          |

### 5.2. 为模拟器添加新的统计项

首先，需要在所关注组件的定义文件中声明一个变量，用于存储希望收集的统计量。例如，若要给 L2 TLB 添加统计量，可在 `tlb.h` 的构造函数中定义：

```cpp
UInt64 variable;
```

之后，将变量注册为相应组件的统计指标。要使用正确的**组件名称**和**核心 ID**：组件名表示所属组件，核心 ID 表示注册指标的核心。例如，注册核心 0 的 L2 TLB 统计项：

```cpp
registerStatsMetric("l2tlb", 0, "variable", variable);
```

注册统计量的一般格式：

```cpp
registerStatsMetric("component name", core_id, "metric described by variable", variable);
```

### 5.3. 为模拟器添加新的配置

有两种主要方式：

- **新增配置文件：** 在 `simulator/sniper/config/virtuoso_configs/` 目录下创建配置，例如 `my_config.cfg`，复制 `simulator/sniper/config/virtuoso_configs/virtuoso_reserve_thp.cfg` 的内容，然后加入所需配置参数：

  ```ini
  # my_config.cfg
  [perf_model/my_component]
  number_of_entries = 64
  ```
- **通过 Sniper 命令行传参：** 运行 Sniper 时覆盖配置项：

  ```bash
  ./run-sniper -c ./config/virtuoso_configs/virtuoso_reservethp.cfg \
    -g --perf_model/my_component/number_of_entries=64 -- my_program
  ```

### 5.4. 提取仿真结果

每次完成新仿真时，应该至少看到以下输出文件：

- **`sim.cfg`：** 仿真使用的 CPU 参数。
- **`sim.info`：** 模拟器信息，以及运行仿真时使用的命令行参数。
- **`sim.out`：** 程序及处理器各组件的性能统计。
- **`sim.stats.sqlite3`：** 保存仿真期间全部数据及指标的数据库文件，由 Sniper 的各类分析工具读取和处理。
- **`sim.stats`：** 与 `sim.stats.sqlite3` 包含相同数据，但采用人类可读的文本格式，适合快速检查和调试。
- **`heartbeat.txt`：** Sniper 周期性更新该文件，以表明仿真仍在运行。

### 5.5. 从头重新编译

如果其他方法都失败了，可运行以下命令清理全部构建文件，然后从头编译（也可再次使用 `build_virtuoso.sh` 与 `incremental_compile.sh` 构建）：

```bash
make distclean
make clean
make -j
```

## 参考文献

[1] Konstantinos Kanellopoulos, Konstantinos Sgouras, F. Nisa Bostanci, Andreas Kosmas Kakolyris, Berkin Kerim Konar, Rahul Bera, Mohammad Sadrosadati, Rakesh Kumar, Nandita Vijaykumar, and Onur Mutlu. *Virtuoso: Enabling Fast and Accurate Virtual Memory Research via an Imitation-Based Operating System Simulation Methodology*. ASPLOS, 2025.

[2] Trevor E. Carlson, Wim Heirman, and Lieven Eeckhout. *Sniper: Exploring the Level of Abstraction for Scalable and Accurate Parallel Multi-Core Simulations*. SC, 2011.

[3] Juan Navarro, Sitaram Iyer, Peter Druschel, and Alan Cox. *Practical, Transparent Operating System Support for Superpages*. OSDI, 2002.

[4] Aamer Jaleel, Kevin B. Theobald, Simon C. Steely, and Joel Emer. *High Performance Cache Replacement Using Re-Reference Interval Prediction (RRIP)*. 2010.

---

> **翻译说明：** 本译文尽量保留原文的章节、编号、参数、关键术语、表格与超链接。部分含下划线的配置项、路径和文件名在 PDF 的文本提取中显示成空格；已在明显处恢复常见下划线写法，但实际操作请与下载的课程仓库核对。第 3.2 节原文对时间序列采样间隔分别提到 10,000 与 100,000 条指令，本译文如实保留这两处说明。
