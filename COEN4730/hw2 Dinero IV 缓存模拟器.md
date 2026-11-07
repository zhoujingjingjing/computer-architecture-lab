# COEN-4730 计算机架构

# HW 2

马凯特大学 电气与计算机工程系

*Cristinel Ababei*

## 目标

学习 **Dinero IV**——一个用于内存引用轨迹（memory reference trace）的缓存模拟器。通过使用 Dinero 模拟器，加深对以下内容的理解：替换策略、统一缓存与分离缓存（unified vs. split）、多级缓存。

## 1. 预备知识：在虚拟机中安装 Linux

如果你已经有一台 Linux 机器，或者之前已经做过这一步，则无需重复此步。

所谓虚拟化，就是可以在一个已有操作系统之上再运行另一个操作系统。例如，你可以在 Mac 上运行 Windows，或者在 Windows 机器上通过虚拟化软件安装 Linux。

**安装步骤：**

按照本教程中的步骤创建一个 Ubuntu 虚拟机：

（请下载并安装最新稳定版（LTS 长期支持版）的 Ubuntu。）

[https://henricasanova.github.io/files/vbox/VirtualBoxUbuntuHowTo.html](https://henricasanova.github.io/files/vbox/VirtualBoxUbuntuHowTo.html)

以我为例，我在以下位置安装了 VirtualBox（VB）：

```
M:\VirtualBox
```

此时，你应该已经在 VirtualBox 中安装好 Ubuntu 了。

以我为例，我把 Ubuntu 安装在：

```
M:\VirtualBox_VMs
```

> **关于安装"Guest Additions（增强功能）"的注意事项**——在做这些操作之前，先打开一个终端（Terminal），安装以下几样东西：
>
> ```
>> su root
>> nano /etc/sudoers
> ```
>
> 并注释掉下面这一行（在行首加 `#`）：
>
> ```
> #sudo   ALL=(ALL)   ALL
> ```
>
> 同时再添加一行（把 `"cristinel"` 换成你自己创建 Linux 用户账户时用的用户名）：
>
> ```
> cristinel  ALL=(ALL)  ALL
> ```
>
> 然后按 `CTRL-X` 并选择 Yes 保存更改。
>
> ```
>> sudo apt update
>> sudo apt install -y build-essential
> ```

此时在 Ubuntu 桌面左侧，你应该能看到一个光盘图标；把鼠标悬停上去，它会显示 "VBox_GAs_7.0.2"。点击它。

在弹出的新窗口中，在空白处右键，选择 "Open in Terminal"（在终端中打开）。

此时会打开一个新终端。在终端中输入以下命令以查看所有文件：

```
> ls
> ./autorun.sh
```

会弹出一个新窗口，输入你的密码。然后等待 VB Guest Additions 安装完成。

关闭（不是重启，不过你也可以先试试重启）这台 Ubuntu 机器。

再次启动这台机器并登录。这将完成 Guest Additions 的安装。之后选择菜单 **View → Auto-Resize Guest Display（自动调整客户机显示）**。此时你应该可以最大化你的 Linux Ubuntu 窗口了！

接下来，进行"创建共享文件夹（Shared Folder）"的操作。我自己的共享文件夹是这样创建的：

```
M:\VirtualBox_SharedFolder
```

当你的虚拟机实例重启并登录后，打开一个终端，执行：

```
> cd /media
> ls
```

你应该能看到共享文件夹 `sf_VirtualBox_SharedFolder`（注意前面有个 `sf_`）。

最后还需要做一件事：把你的用户名加入名为 `vboxsf` 的"组（group）"。这样做是为了避免每次操作 `sf_` 文件夹时都要 sudo。在终端中执行：

```
> sudo usermod -a -G vboxsf cristinel   （把 "Cristinel" 换成你自己的用户名）
```

最后再关闭/重启一次虚拟机，就配置好了。从现在起，你可以随时使用 `/media/sf_VirtualBox_SharedFolder` 目录，让文件同时存在于你自己的主机和 Ubuntu 虚拟机中。

此时，Ubuntu 应该已经安装完毕。

**一些基础操作：**

启动 Ubuntu 并打开一个新终端。

提醒一下：你应该始终在你的主目录（home directory）里工作——本作业以及今后的作业都应如此，而**不要**放在 Windows 与 Linux 之间的共享文件夹里！例如，做本作业时，我在主目录下新建了一个名字直观的文件夹 `hw2`，把所有相关文件都放在里面。要在主目录下新建这样一个文件夹，在终端中执行：

```
$ mkdir hw2
```

如果你之前在 Linux/Unix 机器上没有操作过终端——**请现在就花点时间学习一些基础操作**。网上有大量面向 Linux 初学者的入门教程。自己用 Google 搜一搜、学一学，熟悉 Linux 机器的操作。这非常重要，日后会帮你省去很多麻烦。

再补充一些基础命令：

要搜索你可能想安装的软件，可以用 `apt-cache search`，例如：

```
$ sudo apt-cache search emacs
```

从搜索结果中安装软件，可以用 `apt-get install`，例如：

```
$ sudo apt-get install build-essential
```

例如，要安装 svn、flex、bison 和 emacs，可以这样做：

```
$ sudo apt-get install subversion
$ sudo apt-get install flex
$ sudo apt-get install bison
$ sudo apt-get install emacs
```

## 2. DINERO IV 轨迹驱动的单处理器模拟器

> 注意：下面这段简短介绍改编自该模拟器的官方网页。

**Dinero IV 是一个用于内存引用轨迹的缓存模拟器。**

它有一些根深蒂固的局限：

- Dinero IV **不是**一个时序模拟器。它没有模拟时间或时钟周期的概念，只有引用（reference）。
- Dinero IV **不是**一个功能模拟器。数据和指令并不会真正在缓存中进出；实际上它们并不存在！**用 Dinero IV 模拟得到的主要结果是命中与失效（hit and miss）信息。**
- Dinero IV 不是多线程的。如果你有一个多处理器、且内存充足，可以同时运行多个相互独立的模拟。

基本思想是模拟一个由多级缓存组成的层次结构，这些缓存以一棵树或多棵树的形式连接，引用源（即各个处理器）位于叶子节点，内存位于每个根节点处。每级缓存的各项参数（体系结构、替换策略、统计信息等）都可以分别设置。在初始化阶段，会搭建好待模拟的缓存配置，每一级缓存都被视为一个特殊情形。初始化完成后，每一次引用都通过一次简单的函数调用被送入合适的顶层缓存。层次结构中更低的各层会被自动处理。

### 准备工作

下载并安装 Dinero IV。花点时间浏览它的文档，并快速浏览一下源代码（了解它大致是怎么写出来的）。安装步骤：

**第 1 步**

从下面的网站下载 Dinero IV（`d4-7.tar.gz`），并把文件放到你选定的目录中：

[http://pages.cs.wisc.edu/~markhill/DineroIV/](http://pages.cs.wisc.edu/~markhill/DineroIV/)

**第 2 步**

在终端窗口中运行以下命令，解压并解包下载的文件：

```
> gunzip d4-7.tar.gz
> tar -xvf d4-7.tar
```

**第 3 步**

在终端窗口中，进入第 2 步创建的 `d4-7` 文件夹，用以下命令编译 dinero：

```
> cd d4-7
> ./configure
> make
```

这会生成可执行文件 `dineroIV`。

**第 4 步（轨迹文件）**

你不需要自己生成轨迹文件；本作业的归档包中已经附带了三个程序的轨迹：`cc1`（C 编译器）、`spice`（电路模拟器）和 `tex`（文档格式化程序）。它们以压缩形式提供。要使用它们，请把它们复制到你为本项目创建的目录中并解压。例如，要解压 `cc1.din.Z` 文件，执行：

```
> gunzip cc1.din.Z
```

这样你就得到了 `cc1.din` 轨迹文件。三个轨迹文件的名字分别是 `cc1.din`、`spice.din` 和 `tex.din`。

**第 5 步（测试）**

在 `d4-7` 文件夹中有一个可执行文件 `dineroIV`，它就是 dinero 程序。运行方式如下：

```
> ./dineroIV (options) < trace_file_name
```

其中 `options` 用于指定缓存大小、缓存块大小和缓存布局选项；`trace_file_name` 是轨迹文件名。

要查看可用的选项，运行：

```
> ./dineroIV -help
```

你应该会看到类似下面的输出：

```
Usage: dineroIV [options]
Valid options:
  -help                Print this help message
  -copyright           Give details on copyright and lack of warranty
  -contact             Where to get the latest version or contact the authors
  -dineroIII           Explain replacements for Dinero III options
  -custom F            Generate and run custom simulator named F
  -lN-Tsize P          Size
  -lN-Tbsize P         Block size
  -lN-Tsbsize P        Sub-block size (default same as block size)
  -lN-Tassoc U         Associativity (default 1)
  -lN-Trepl C          Replacement policy
                       (l=LRU, f=FIFO, r=random) (default l)
  -lN-Tfetch C         Fetch policy
                       (d=demand, a=always, m=miss, t=tagged,
                        l=load forward, s=s subblock) (default d)
  -lN-Tpfdist U        Prefetch distance (in sub-blocks) (default 1)
  -lN-Tpfabort U       Prefetch abort percentage (0-100) (default 0)
  -lN-Twalloc C        Write allocate policy
                       (a=always, n=never, f=fetch) (default a)
  -lN-Twback C         Write back policy
                       (a=always, n=never, f=nofetch) (default a)
  -lN-Tccc            Compulsory/Capacity/Conflict miss statistics
  -skipcount U         Skip initial U references
  -flushcount U       Flush cache every U references
  -maxcount U         Stop simulation after U references
  -stat-interval U    Show statistics after every U references
  -informat C         Input trace format
                       (d=extended din, d=traditional din, p=pixie32, P=pixie64,
                        b=binary) (default d)
  -on-trigger A       Trigger address to start simulation
  -off-trigger A      Trigger address to stop simulation
  -stat-icdcomb ine   Combine I&D cache stats
Key:
  U unsigned decimal integer
  S like U but with optional [kKmMgG] scaling suffix
  P like S but must be a power of 2
  C single character
  A hexadecimal address
  F string

  N cache level (1 <= N <= 5)
  T cache type (u=unified, i=instruction, d=data)
```

一个 Dinero 命令示例如下：

```
> ./dineroIV -l1-isize 16384 -l1-iassoc 4 -l1-ibsize 32 -l1-irepl l -l1-dsize 32768 -l1-dassoc 2 -l1-dbsize 16 -l1-drepl f -l1-dwalloc a -l1-dwback a -informat d < cc1.din > sample.out
```

注意上面命令中用到的轨迹文件：

- `-informat d < cc1.din`：这是命令行的输入，即轨迹文件；该轨迹文件应与命令在同一目录下。
- `cc1.din > sample.out`：这是重定向输出，会被保存到名为 `sample.out` 的文件中。

用你喜欢的文本编辑器（例如 emacs）查看结果：

```
> emacs sample.out
```

你会看到类似这样的内容：

```
---Dinero IV trace simulator, version 7
---Written by Jan Edler and Mark D. Hill.
---Copyright (C) 1997 NEC Research Institute, Inc. and Mark D. Hill.
---All rights reserved.
---Copyright (C) 1985, 1989 Mark D. Hill.  All rights reserved.
---See -copyright option for details.
---Summary of options (-help option gives usage information).
-l1-isize 16384
-l1-dsize 32768
-l1-ibsize 32
-l1-dbsize 16
-l1-iassoc 4
-l1-dassoc 2
-l1-irepl l
-l1-drepl f
-l1-ifax d
-l1-dfax d
-l1-iwalloc a
-l1-dwalloc a
-l1-iwback a
-l1-dwback a
-skipcount 0
-flushcount 0
-maxcount 0
-stat-interval 0
-informat d
-on-trigger 0x0
-off-trigger 0x0
---Simulation begin.
---Simulation complete.

1i cache
  Metric           Total        Instr        Data        Read       Write       Misc
  -------          -----        -----        ----        ----       -----       ----
  Demand Fetches   757341       757341       0           0          0           0
    Fraction of total  1.00000    1.0000       0.0000      0.0000     0.0000      0.0000

  Demand Misses    13743        13743        0           0          0           0
    Demand miss rate  0.0181     0.0181       0.0000      0.0000     0.0000      0.0000

  Multi-block refs 0
  Bytes From Memory        439776
    ( / Demand Fetches)    0.5807
  Bytes to Memory         0
    ( / Demand Writes)    0.0000
  Total Bytes r/w Mem      439776
    ( / Demand Fetches)    0.5807

1d cache
  Metric           Total        Instr        Data        Read       Write       Misc
  -------          -----        -----        ----        ----       -----       ----
  Demand Fetches   242661       0            242661      159631     83030       0
    Fraction of total  1.00000    0.0000       1.0000      0.6578     0.3422      0.0000

  Demand Misses    4248         0            4248        2095       2153        0
    Demand miss rate  0.0175     0.0000       0.0175      0.0131     0.0259      0.0000

  Multi-block refs 0
  Bytes From Memory        67968
    ( / Demand Fetches)    0.2801
  Bytes to Memory          40724
    ( / Demand Writes)     0.4912
  Total Bytes r/w Mem      108752
    ( / Demand Fetches)    0.4482

---Execution complete.
```

## 3. 作业内容

**问题 1：替换策略（三个轨迹文件都要做）**

考察一个 8K 字节、2 路组相联（2-way associative）、块大小为 16 字节的缓存。LRU、FIFO 和随机替换策略下的平均失效率分别是多少？通过把缓存大小增加到 16K 字节来考察这些替换策略。**你能从这个实验中得出什么结论？**

**问题 2：统一缓存与分离缓存（仅使用 `cc1.din`）**

比较以下两种系统的缓存失效率：

- 一个使用 32K 字节统一缓存的系统；
- 一个使用 16K 字节仅指令缓存 + 16K 字节仅数据缓存的系统。

假设缓存为 4 路组相联，采用 LRU 替换策略，块大小为 32 字节。**你能从这个实验中得出什么结论？**

**问题 3：多级 L2 缓存（仅使用 LRU 替换策略）**

计算 2 级缓存的 AMAT（平均内存访问时间），采用 LRU 替换策略，条件如下：

- L1 缓存大小 16KB，L2 缓存大小 64KB；
- L1 缓存大小 32KB，L2 缓存大小 128KB。

假设 L1 为直接映射，L2 为 4 路组相联。L1 的命中时间为 1 个时钟周期，L2 的命中时间为 8 个时钟周期。L2 的失效代价为 50 个时钟周期。**你如何评价这两种缓存组织方式的性能？**

## 4. 交付物

写一份报告，说明你做了什么以及得到了什么结果。要简洁、清晰；但请使用你认为必要的表格和图。在报告末尾附上**仅一次** Dinero 模拟运行的输出（即这些实验中你随便选的一次运行）。

## 5. 参考文献与致谢

[1] 本作业改编自俄亥俄州立大学 Avinash Kodi 的教学材料。
