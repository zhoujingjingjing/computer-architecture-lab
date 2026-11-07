# COEN-4730 计算机架构

# HW 4

马凯特大学 电气与计算机工程系

*Cristinel Ababei*

## 目标：

学习 **gem5 全系统模拟器** 和 **McPAT 功耗计算器**。我们将在 HW#5 中使用它们，对片上多处理器（CMP）进行设计空间探索，其中核与核之间通过传统总线（Bus）通信。

## 1. GEM5 模拟器

GEM5（[https://www.gem5.org/](https://www.gem5.org/)）是一个计算机系统模拟平台。与处理器体系结构模拟器 SimpleScalar 不同，GEM5 可以对完整的多核平台（包括处理器、存储系统、操作系统本身）进行模拟。对处理器而言，GEM5 能够模拟多种指令集体系结构（ISA），包括 Alpha、ARM、MIPS 和 x86。

GEM5 模拟一个系统有两种模式：

- **系统调用模拟（Syscall Emulation, SE）**：这种模式下没有操作系统（OS）。应用程序中的所有系统调用都由 GEM5 模拟。
- **全系统模拟（Full System, FS）**：这种模式下模拟一个完整的系统，包括 OS 和所有外设。这就是我们最常使用的模式。

在 GEM5 的 SE 模式下，对多线程应用的支持有限。因此，本作业和下一个作业我们都将使用 FS 模式。此外，我们将使用 Alpha 和 ARM 两种 ISA 作为目标体系结构。

```
========================================================================
1.1  安装 Ubuntu 18.04
========================================================================
```

由于 gem5 的变化，以及可能还有 Ubuntu 各版本的变化，在 VirtualBox 上工作时，安装最新版 gem5、并为 ARM 和 Alpha ISA 编译内存占用较大的构建会遇到空间问题：当虚拟 HDD 为 20GB、内存为 10GB 时，编译最新版 gem5 会因空间不足而在安装阶段崩溃。

因此，本作业我们使用稍旧版本的 Ubuntu（即 18.04）以及稍旧版本的 gem5。首先，在你的 VirtualBox 中删除之前版本的 Ubuntu，按照 hw2 中给出的步骤重新安装——这次是 18.04。从下面下载桌面版 .iso 文件：

[https://releases.ubuntu.com/18.04/](https://releases.ubuntu.com/18.04/)

> 请确保至少分配 20GB 磁盘空间，并在你电脑能承受的情况下分配尽量多的内存；我自己分配了 10GB 内存。另外，当被询问是否升级到更新版本的 Ubuntu 时，不要同意！

```
========================================================================
1.2  为 ALPHA 体系结构构建并运行全系统（FS）模式
========================================================================
```

### 安装前置软件：

在安装 GEM5 之前，你需要先在 Linux Ubuntu 机器上安装几样东西。在终端中执行：

```
> sudo apt-get update
> sudo apt-get install emacs
> sudo apt-get install build-essential
> sudo apt-get install mercurial
> sudo apt-get install python-dev
> sudo apt-get install scons
> sudo apt-get install swig
> sudo apt-get install zlib1g-dev
> sudo apt-get install m4
> sudo apt-get install protobuf{,-c,-l,-compiler}
> sudo apt-get install libgoogle-perftools-dev
> sudo apt-get install git
```

### 下载 GEM5：

本作业的文件中附带了一个较旧但稳定版本的 gem5（也放在 D2L 上）。

归档文件名是：`gem5-stable_2015_09_03.tar.gz`

把它放到某个特定文件夹中并解压（用 gunzip，然后 tar xvf）。以我为例，我在 `/user/cristinel/hw4/` 中完成这些操作。此时你应该已经创建了一个 `gem5-stable_2015_09_03/` 文件夹，里面是 gem5 的所有文件。为方便操作，我们把它重命名为 `gem5`：

```
> mkdir hw4
> cd hw4
（把 gem5-stable_2015_09_03.tar.gz 复制到 hw4 文件夹中）
> gunzip gem5-stable_2015_09_03.tar.gz
> tar xvf gem5-stable_2015_09_03.tar
> mv gem5-stable_2015_09_03 gem5
```

还有一件事要做：用文本编辑器（之前装的 emacs）打开 `gem5/src/SConscript` 并编辑：

```
> cd gem5
> emacs src/SConscript
```

在文件中搜索 `"Werror=True"`，把它替换为 `Werror=False`。保存更改。

```
========================================================================
1.2  为 ALPHA 体系结构构建并运行全系统（FS）模式
========================================================================
```

### 下载预编译的 Linux 内核：

在 FS 模式下，**Linux 本身是在 GEM5 模拟过程中、在被模拟的体系结构上启动的**。也就是说，操作系统本身也被模拟了。为此，我们需要先下载并安装全系统二进制和磁盘镜像文件、Linux 内核、磁盘镜像文件，以及用于 Alpha、ARM 和 x86 的引导加载程序。这些都可以在 gem5 的 Download 页面找到。不过，在本课程使用的 gem5 版本中，**本作业提供的归档文件中已包含这些文件**。

文件名是：`m5_system_2.0b3.tar.bz2`

我们下载预编译的 Linux 内核、PALcode/Console 代码，以及 Alpha 体系结构的文件系统，并把它们保存到一个新文件夹中（我在 `gem5/` 下命名为 `full_system_images_ALPHA`）。解压刚下载的归档文件：

```
> cd gem5
> mkdir full_system_images_ALPHA
> cd full_system_images_ALPHA
（把 m5_system_2.0b3.tar.bz2 放入 full_system_images_ALPHA 文件夹）
> tar vxfj m5_system_2.0b3.tar.bz2
> rm m5_system_2.0b3.tar.bz2
> mv m5_system_2.0b3/* .
> rm -rf m5_system_2.0b3/
> chmod -R 777 *
```

此时你应该已经在 `full_system_images_ALPHA/` 文件夹中有了 **binaries（即 Linux 内核）和 disk/ 文件夹**。

这些文件的路径在 `configs/common/SysPaths.py` 中指定/确定。GEM5 会在这里查找这些文件。该脚本中硬编码了几个默认路径。因此，要使用新下载的内核和磁盘文件，可以采取以下任一做法：

1. 把系统文件放到这些硬编码路径中的某一个。我没有这么做，因为我自己创建了一个名字直观的 `full_system_images_ALPHA`。
2. 把系统文件放到某个新文件夹，然后设置 `M5_PATH` 环境变量来覆盖脚本中硬编码的路径。**这就是我采用的做法。**
   要永久设置 `M5_PATH` 变量，在你的 `.bashrc` 文件中添加以下一行：

   ```
   export M5_PATH=/home/cristinel/hw4/gem5/full_system_images_ALPHA
   ```
3. 把系统文件放到某个新文件夹，然后编辑 `SysPaths.py` 来修改预定义路径、指定新路径。**我就是这么做的，以防万一。你也应该这么做。**
   要修改 `configs/common/SysPaths.py`，我把原来的：

   ```
   path = [ '/dist/m5/system', '/home/guoqi/simulators/gem5/dist/m5/system' ]
   ```

   改成了：

   ```
   path = [ '/dist/m5/system', '/home/cristinel/hw4/gem5/full_system_images_ALPHA' ]
   ```

**注意：**

1. 如果这一步做得不对，你会遇到这种错误：
   `"ImportError: Can't find a path to system files."`
   这发生在你第一次尝试以全系统模式运行 GEM5 模拟器时。
2. 如果你采用上面第 1 种做法，需要 root（sudo）权限，因为你要新建一个文件夹 `/dist/m5/system`，系统文件需要复制到那里。这种做法的好处是文件放在一个可被多个用户共享的位置。
   要这么做，可以参考下面这个例子，把系统文件放到默认位置：

   ```
   > sudo mkdir -p /dist/m5/system
   > cd /dist/m5/system
   ```

### 构建（Building）：

**例 1：** 现在，我们构建 GEM5 模拟器的 ALPHA 目标，以便用它运行 FS 模拟：

```
> scons FULL_SYSTEM=1 build/ALPHA/gem5.opt -j4
```

上面会生成一个可以运行 FS 模拟的二进制。如果你运行时处理器体系结构包含多个核，核间通信将是基于总线的传统方式。

不过，如果你希望核间通信通过片上网络（Network-on-Chip, NoC）完成，就需要在构建时额外加上 `RUBY=true` 选项来构建目标 GEM5。但我们不这么做。我们只关注基于总线的多核处理器。

**注意：**

1. 我们可以运行 `scons build/ALPHA/gem5.opt --help` 来查看 ALPHA 可用的全局构建变量。
2. 构建时必须指定缓存一致性协议（否则你可能拿不到 `l2_cntrl...`）。
3. 如果你想为某个 GEM5 构建目标和配置目录修改 `"build"` 的值（如果该目录已创建），或者想在创建时就覆盖它，可以在命令行指定新值。语法和在 shell 命令行设置环境变量类似，但要放在 scons 命令之后。
   例如，要在现有 ALPHA 构建上构建 MOESI_hammer 协议，可以用下面的命令：

   ```
   > scons PROTOCOL=MOESI_hammer build/ALPHA/gem5.opt
   ```
4. 构建 FS 目标时，我们必须把配置变量 `FULL_SYSTEM=1`：

   [http://www.m5sim.org/Build_System_Per_Configuration](http://www.m5sim.org/Build_System_Per_Configuration)

### 运行（Running）：

**例 1：** 现在，我们运行 gem5/configs/examples 目录下的 `fs.py` 配置文件，并使用之前下载的内核和磁盘：

```
> build/ALPHA/gem5.opt configs/example/fs.py \
    --kernel=/home/cristinel/hw4/gem5/full_system_images_ALPHA/binaries/vmlinux \
    --disk-image=/home/cristinel/hw4/gem5/full_system_images_ALPHA/disks/linux-latest.img
```

默认情况下，`fs.py` 脚本会启动 Linux 并在 gem5 控制台上启动一个 shell。为了把模拟控制台与模拟器的输入输出分开，**这个被模拟的控制台通过一个 TCP 端口进行关联**。要与控制台交互，你必须用 telnet 这类程序连接到该端口运行一个程序。例如，在上面的模拟启动后，在另一个不同的终端中输入：

```
> telnet localhost 3456
```

在终端提示符下连上 telnet 后，进入 root 文件系统并输入：

```
> ls
```

**你会看到什么？** 你应该会看到打印出一堆内容，显示 Linux 正在启动的过程。

由于 telnet 的回显行为与 GEM5 配合得不好，所以如果你经常使用模拟控制台，可能更想用 **m5term** 而不是 telnet；至少 GEM5 的人是这么说的。第二种与模拟控制台交互的方式是使用 m5term 程序而不是 telnet。m5term 是 gem5 提供的全系统控制台接口。要做到这一点，进入 `gem5/util/term` 目录并构建 m5term：

```
> cd gem5/util/term
> make
> sudo make install
```

现在 m5term 可以用来与模拟器交互了，不过用户通常需要设置各种终端选项才能正常工作。下面是一个 m5term 实际使用的小例子：

```
> m5term localhost 3456
```

然后像之前那样输入 `ls`：

```
> ls
```

**你会看到什么？** 你应该会看到打印出一堆内容，显示 Linux 正在启动的过程。

### 用 .rcS 脚本自动运行基准测试：

在 FS 模拟中，除了加载 Linux 内核外，gem5 还会为其文件系统加载一个或多个磁盘镜像。至少必须把一个磁盘镜像挂载为根文件系统。**任何你想在 GEM5 内部运行的应用/基准测试可执行文件，都必须存在于这些磁盘镜像上！**

基本上，你要运行的任何基准测试都必须被交叉编译，并被放到某个磁盘镜像中！

为了在运行基准测试时不需要交互式 shell（如上面所述）——那种方式很繁琐——gem5 可以加载 `.rcS` 脚本文件，这些脚本会替换正常的 Linux 引导脚本，在引导完 OS 后直接执行。这些 `.rcS` 文件可用于配置以太网接口、执行特殊的 gem5 指令，或开始执行位于磁盘镜像上的二进制。

下面是一个 .rcS 文件的例子/片段：

```
echo -n "setting up network..."
/sbin/ifconfig eth0 192.168.0.10 txqueuelen 1000
/sbin/ifconfig lo 127.0.0.1
echo -n "running surge client..."
/bin/bash -c "/benchmarks/surge && ./Surge 2 100 1 192.168.0.1 5."
echo -n "halting machine"
m5 exit
```

你可以从 `gem5/configs/boot/*.rcS` 阅读示例脚本，学习如何编写你想运行的脚本。

例如，要在模拟器中运行一个 Linux 引导后立即执行的 rcS 脚本，可以这样运行：

```
> build/ALPHA/gem5.opt configs/example/fs.py --script=configs/boot/rcS
```

> **注意：** 有时我们能找到已经预编译好、可公开访问的基准测试，用于某些特定体系结构。这些基准测试是为在 GEM5 模拟中使用而"准备好"的，由非常好的人完成。这种情况下，你可以简单地下载并在我们自己的 GEM5 模拟中把它们当作预编译基准测试使用。不过，如果你想对这些基准测试做一些改动（例如，你修改了基准测试中某个 C 源文件的一部分），就需要交叉编译器来重新编译该基准测试，并放到磁盘镜像上，供你在模拟中考虑你为基准测试本身所做的改动。**下一节将介绍一个可用基准测试的例子。**

```
========================================================================
1.3  预编译的 PARSEC 2.1 基准测试：为 ALPHA 体系结构提供 FS 模式
========================================================================
```

**注意：**

1. 本节根据以下网页中的信息编写：[http://www.cs.utexas.edu/~parsec_m5/](http://www.cs.utexas.edu/~parsec_m5/)
2. **在本作业中，这一节是最重要的。**

1) 从本作业提供的文件或 D2L 下载文件 `vmlinux_2.6.27-gcc_4.3.4`

仅供参考，我是从下面下载的 [http://www.cs.utexas.edu/~parsec_m5/vmlinux_2.6.27-gcc_4.3.4](http://www.cs.utexas.edu/~parsec_m5/vmlinux_2.6.27-gcc_4.3.4)，它包含预编译的 Linux 2.6.27 内核。

现在把它放到 `gem5/full_system_images_ALPHA/binaries` 中。

```
> cd /home/cristinel/hw4/gem5/full_system_images_ALPHA/binaries
```

还要把"磁盘镜像"放到 `gem5/full_system_images_ALPHA/disks` 中。

直接从 D2L 下载文件 `linux-parsec-2-1-m5-with-test-inputs.img.bz2`：

```
> cd /home/cristinel/hw4/gem5/full_system_images_ALPHA/disks
```

这里，该磁盘镜像（包含本基准测试）是从这里下载的：

[http://www.cs.utexas.edu/~parsec_m5/linux-parsec-2-1-m5-with-test-inputs.img.bz2](http://www.cs.utexas.edu/~parsec_m5/linux-parsec-2-1-m5-with-test-inputs.img.bz2)

解压归档文件：

```
> bunzip2 linux-parsec-2-1-m5-with-test-inputs.img.bz2
```

此时你应该有了磁盘镜像：`linux-parsec-2-1-m5-with-test-inputs.img`。

2) 要让 GEM5 知道去哪里找磁盘镜像和二进制/内核，请在你的 `.bashrc` 中把 `M5_PATH` 变量更新为：

```
export M5_PATH=/home/cristinel/hw4/gem5/full_system_images_ALPHA
```

**注意：** 如前面章节所述，你也可以通过修改 `configs/common/SysPaths.py` 来实现。

然后，修改文件 `configs/common/Benchmarks.py` 中的磁盘镜像名字：

```
> cd gem5
> emacs configs/common/Benchmarks.py
```

通过把下面这一行：

```
return env.get('LINUX_IMAGE', disk('linux-latest.img'))
```

替换为这一行：

```
return env.get('LINUX_IMAGE', disk('linux-parsec-2-1-m5-with-test-input.img'))
```

来指定你磁盘镜像的路径。

> **注意：** 如果你不想改 `Benchmarks.py`，那每次运行 GEM5 模拟器时都必须在命令行加上参数：
> `--disk-image=/home/cristinel/multicore2/gem5/full_system_images_ALPHA/disks/linux-parsec-2-1-m5-with-test-inputs.img`

3) 用同一站点提供的 Perl 脚本生成 .rcS 运行脚本：

[http://www.cs.utexas.edu/~parsec_m5](http://www.cs.utexas.edu/~parsec_m5)

技术报告见：[http://www.cs.utexas.edu/~parsec_m5/TR-09-32.pdf](http://www.cs.utexas.edu/~parsec_m5/TR-09-32.pdf)（请花时间读一下）。

我们为你想运行的 PARSEC 基准测试中的每一个，用上面的 Perl 脚本创建两个新文件夹，里面是要处理的文件：

```
> cd /home/cristinel/hw4/gem5
> mkdir benchmarks
> cd benchmarks
> mkdir parsec_files
> cd parsec_files
```

把 Perl 脚本、输入集和 Checkpoint 脚本保存到这里。实际上，你可以直接从本作业提供的文件或 D2L 下载：

- [http://www.cs.utexas.edu/~parsec_m5/writescripts.pl](http://www.cs.utexas.edu/~parsec_m5/writescripts.pl)（**你应该读一下这个！**）
- [http://www.cs.utexas.edu/~parsec_m5/inputsets.txt](http://www.cs.utexas.edu/~parsec_m5/inputsets.txt)
- [http://www.cs.utexas.edu/~parsec_m5/m5hack_back_ckpt.pl](http://www.cs.utexas.edu/~parsec_m5/m5hack_back_ckpt.pl)

下面这个 Perl 脚本必须按如下方式运行：

```
> perl writescripts.pl <benchmark> <nthreads>
```

举个例子，我们要在一个四核体系结构上为 `blackscholes` 基准测试跑 GEM5 模拟——所以我们运行这个脚本时通过 `<nthreads>` 选项指定 4。要生成我在 GEM5 中运行 blackscholes 时使用的 .rcS 文件，运行这个 Perl 脚本（举例）：

```
> perl writescripts.pl blackscholes 4 <--- FOUR threads; I want this many threads for this benchmark!!
```

这会生成几个运行脚本，我把它们复制到一个新文件夹，比如 `run_scripts/`：

```
> cd /home/cristinel/hw4/gem5/
> mkdir run_scripts
> cd run_scripts
> cp /home/cristinel/hw4/gem5/benchmarks/parsec_files/blackscholes* .
```

我把它们复制到这个新文件夹 `run_scripts`，同时把 `parsec_files` 只用于运行生成基准测试 .rcS 文件的 Perl 脚本。我会把所有 .rcS 文件保留在 `run_scripts/` 文件夹中。

4) 现在，构建一个我们想模拟的 GEM5 目标（如果你在前面的章节还没做的话）。例如：

**例 1：**

我们构建一个 GEM5 目标，用它模拟采用**经典总线（Bus）**作为核间通信方式的多核处理器体系结构：

```
> cd gem5
> scons FULL_SYSTEM=1 build/ALPHA/gem5.opt  <--- 如果你之前已经构建过就不用再做！如果没有才构建！
```

我们在一个 4 核处理器上用 4 个线程运行 blackscholes 基准测试。

**注意：**

1. 下一次 GEM5 调用中我使用了 `--cpu-type=timing`，因为把 gem5 转换为 mcpat 的脚本会输出 atomcpu。现在先不用操心这个。
2. 在运行 .rcS 脚本生成上面的内容之前，你应该先注释掉 `blackscholes_4c_smallsim.rcS` 脚本中的 `"/sbin/m5 switchcpu"` 这一行，否则 GEM5 会过早退出。这些问题都是使用 GEM5 的人在不同时间、以不同方式学到的。下面是一个 GEM5 源代码例子，其哲学是不同人在不同时间写下的……

```
> build/ALPHA/gem5.opt configs/example/fs.py --cpu-type=timing \
    --script=run_scripts/blackscholes_4c_smallsim.rcS \
    --num-cpus=4 \
    --caches --l2cache --l2_size=512kB \
    --l1d_size=32kB --l1i_size=32kB --l1d_assoc=2 --l1i_assoc=2 \
    --kernel=/home/cristinel/hw4/gem5/full_system_images_ALPHA/binaries/vmlinux_2.6.27-gcc_4.3.4 \
    --disk-image=/home/cristinel/hw4/gem5/full_system_images_ALPHA/disks/linux-parsec-2-1-m5-with-test-inputs.img
```

如果上面成功完成，你会创建一个 `m5out/` 文件夹，里面是模拟结果。我们把这次 GEM5 模拟的结果保存到一个名为 `runs_results` 的新文件夹中（以免以后运行 GEM5 时被覆盖）：

```
> mkdir runs_results
> mv m5out runs_results/m5out_blackscholes_4c_bus
> cd runs_results/m5out_blackscholes_4c_bus
```

打开 m5out 文件夹中创建的每个文件，研究一下。

我们再用这个基准测试做一次 GEM5 运行，这次是单核处理器体系结构（这是默认设置，至少在写本文时是这样）：

```
> build/ALPHA/gem5.opt configs/example/fs.py \
    --script=run_scripts/blackscholes_4c_smallsim.rcS \
    --kernel=/home/cristinel/hw4/gem5/full_system_images_ALPHA/binaries/vmlinux_2.6.27-gcc_4.3.4 \
    --disk-image=/home/cristinel/hw4/gem5/full_system_images_ALPHA/disks/linux-parsec-2-1-m5-with-test-inputs.img
```

到现在为止，你应该状态良好，对 GEM5 模拟有很好的理解了！再次强调，本课程我们主要使用 FS 模拟。

**注意：**

如实际技术报告（[http://www.cs.utexas.edu/~parsec_m5/TR-09-32.pdf](http://www.cs.utexas.edu/~parsec_m5/TR-09-32.pdf)）中所述，PARSEC 基准测试使用多种并行化方法，包括 pthreads、Intel TBB 和 OpenMP。本节讨论的预编译 PARSEC 基准测试使用所有基准测试的 pthreads 版本，除了 freqmine——它没有 pthreads 实现，所以用的是 OpenMP 版本。

这些应用被分为三个阶段：

- 初始串行阶段；
- 并行阶段；
- 最终串行阶段。

并行阶段被称为**感兴趣区域（region of interest, ROI）**，在应用源代码中用对 PARSEC hooks 库的调用来标记。

因此，gem5 模拟会在基准测试 ROI 区域的开始和结束处转储统计信息。例如，对于上面从 `blackscholes_4c_smallsim.rcS` 脚本运行的模拟，其中有一行 `"/sbin/m5 switchcpu" 已被注释掉`，你应该在模拟结束时看到 4 组统计信息写入 `m5out/stats.txt`。它们对应：1）模拟开始到 `dumpstats` 的时刻；2）基准测试开始到 ROI 开始；3）ROI 开始到模拟在 `"/sbin/m5 exit"` 处退出。**我们关心的是第三组模拟统计信息，即 ROI 部分。**

## 2. McPAT 建模框架

McPAT（Multicore Power, Area, and Timing，多核功耗、面积与时序，[http://www.hpl.hp.com/research/mcpat/](http://www.hpl.hp.com/research/mcpat/)）是一个集成化的功耗、面积和时序建模框架，面向多线程、多核和众核体系结构。

基本上，McPAT 读取（micro-architecture）参数和事件统计，估算系统中每个组件的面积、时序和功耗数字。它有从 90nm 到 22nm 不同技术节点的模型。精度取决于你提供的细节级别。

> **注意：** 直到 2016 年，较旧的 McPAT 0.8 版本才是"在 terms of 'accepting' the power.xml 方面更易用"的版本，它通过把 GEM5 输出转换为 McPAT 输入文件的 Python 脚本创建。不过，从 2017 年起，由于 GEM5 的变化，出现了一些问题：GEM5 生成的内容与旧 McPAT 0.8 预期的不符。现在，请使用最新版本的 GEM5（稳定版）和 McPAT 1.0 或 1.3，以及那个能开箱即用的 Python 脚本。

直接从本作业包含的文件或 D2L 下载 `McPAT.tar.gz`。把 McPAT 安装到 `gem5` 文件夹内。为简单起见，把 McPAT 文件夹重命名为 `mcpat`：

```
> cd SGEM5/mcpat
> sudo apt-get install libc6-dev-i386
> sudo apt-get install gcc-multilib g++-multilib
> make
```

此时你应该已经创建了可执行文件 `mcpat`。要了解如何使用 McPAT，运行：

```
> ./mcpat -help
```

**注意：**

从下面下载并阅读文档、技术报告等，了解 McPAT 是什么：

[http://www.hpl.hp.com/research/mcpat/McPATAlpha_TechRep.pdf](http://www.hpl.hp.com/research/mcpat/McPATAlpha_TechRep.pdf)

现在我们已经准备好用 McPAT 估算我们体系结构的功耗了。为此，我们需要用 GEM5 运行的结果作为输入来运行 McPAT。不过，`m5out/` 中的文件并不是 McPAT 能理解的格式。所以，要能够用 GEM5 运行时生成的输出来喂给 McPAT，我们需要把 GEM5 的输出文件转换成 McPAT 能理解的格式。一种方法是使用下面这个脚本（Richard Strong 的 Python 解析器）：

[https://bitbucket.org/rickshon/m5-mcpat-parser](https://bitbucket.org/rickshon/m5-mcpat-parser)

（我把它做了一些修改后放到 D2L 上，让它能工作。）

所以，从 D2L 下载脚本 `m5-mcpat-parse-se-ALPHA-ARM-Bus.py`，把它放到在 `gem5/gem5_2_mcpat/` 创建的文件夹内。

这个脚本用于生成 McPAT 模拟所需的输入文件。它读取 GEM5 模拟生成的输出文件作为输入，这些文件默认保存在 `m5out/` 下。

我们新建一个文件夹，在里面使用这个脚本：

```
> cd SGEM5
> mkdir gem5_2_mcpat
> cd gem5_2_mcpat
```

把刚做的 GEM5 运行的结果复制给我们（4 核、基于总线通信）：

```
> cp -r ../runs_results/m5out_blackscholes_4c_bus m5out
```

把解析器的一个副本放到 `gem5_2_mcpat/`。你也可以在 D2L 上找到它。

> **注意：** 打开这个脚本并读一下！试着理解它是干什么的！

运行脚本（它知道要在 `m5out/` 中找 config.ini 和 stats.txt）：

```
> python m5-mcpat-parse-se-ALPHA-ARM-Bus.py
```

如果一切顺利，你应该会看到一个新文件被创建：`power.xml`。

现在，运行 McPAT，把脚本生成的 power.xml 作为输入：

```
> /home/cristinel/hw4/gem5/mcpat/mcpat -infile power.xml
```

如果一切顺利，你应该会在终端看到打印出各种数据：面积、功耗等！！！

> **注意：** 你可能会看到类似 `"Warning: icache array structure cannot satisfy latency constraint."` 的警告。试着回答这是什么、为什么会这样。

## 3. 交付物：

一份单文件 PDF 报告，包含：

- 标题、姓名。
- 报告摘要，列出你做的主要工作。
- 你在安装和运行这些工具时遇到的问题，以及你是如何解决的。
- 在两个附录中给出运行 gem5 和 McPAT 工具的示例。

## 4. 参考文献

[1] 自己学习 GEM5 网站，了解如何为全系统（FS）模拟构建 GEM5，并让它使用 Garnet mesh NoC。有很多内容要读；要有选择、快速地读。下面是一些 GEM5 网页，便于快速学习：

- [http://www.m5sim.org/Ruby_Memory_System](http://www.m5sim.org/Ruby_Memory_System)
- [http://www.m5sim.org/Ruby](http://www.m5sim.org/Ruby)
- [http://www.m5sim.org/Interconnection_Network](http://www.m5sim.org/Interconnection_Network)
- [http://gem5.org/Ruby_Network_Test](http://gem5.org/Ruby_Network_Test)

[2] 阅读下面网页，了解关于模拟脚本（`config.py`）的更多信息。例如，要修改缓存配置，可以修改 `config.py` 中的设置。

[http://www.m5sim.org/Simulation_Scripts_Explained](http://www.m5sim.org/Simulation_Scripts_Explained)

[3] 我整理了一个简短的 PPT 演示，里面是 GEM5 的"鸟瞰图"。它包含框图和附加参考资料，并指向相关阅读材料。你可以在这里找到它：

[http://dejazzar.com/coen4730/doc/lecture09_supplemental_gem5.pdf](http://dejazzar.com/coen4730/doc/lecture09_supplemental_gem5.pdf)
