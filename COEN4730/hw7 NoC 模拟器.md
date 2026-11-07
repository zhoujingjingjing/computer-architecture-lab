# COEN-4730 计算机架构

# HW 7

马凯特大学 电气与计算机工程系

*Cristinel Ababei*

## 1. 目标

使用一个片上网络（Network-on-Chip, NoC）模拟器，来研究 NoC 的性能。

## 2. 片上网络（NoC）模拟器

虽然我们在课上讨论过 NoC，但你可以阅读论文 [1] 来获得额外的快速介绍。要研究 NoC 的性能，我们将使用 NoC 模拟器；它们非常有用，因为它们能在有任何物理实现之前，就对不同 NoC 拓扑、路由策略等给出性能估算。

本作业中，你将使用一个 NoC 模拟器（如 D2L 上附加文档所指定的那一个）。它是我们大家都会看的一个。不过，还有很多其他 NoC 模拟器。要了解一个相对全面的列表，请见 [2]。

**a) Booksim**

最流行的 NoC 模拟器之一是 BookSim，你可以从 [3] 下载。下载 BookSim 2.0 模拟器的源代码，安装在你的 Linux 机器上。同时从上面的网页下载并阅读它的手册（手册里描述了如何安装和运行模拟器）。

**b) Noxim**

另一个流行的 NoC 模拟器是 Noxim。你可以从 [4] 下载它的源代码。注意你还需要下载 SystemC，才能编译 Noxim。此时，如何让 Noxim 模拟器顺利工作，就要靠你自己去摸索了。

**c) VNOC 2.0**

我们要用的第三个模拟器是 VNOC 2.0。可以从这里下载 [5]。它提供下载的源代码里有一个 readme 文件，描述了如何安装（它需要一些先决条件的安装）以及如何运行它。

## 3. 作业内容

你的作业是运行模拟器，收集足够的数据点（每张图 10 个数据点），以便能够为两种不同的 NoC **创建延迟 vs. 提供负载（又称注入速率）的四张图**。

两张图应针对 **4×4 规则网格（mesh）拓扑**，另外两张图应针对 **8×8 规则网格拓扑**。

每个 NoC 的这两张图，应针对**两种不同类型的流量：uniform（均匀）和 transpose（转置）**。你应该用 XY 路由策略建立 NoC 拓扑，输入缓冲区大小为 128，输出缓冲区大小为 16，包大小为 5 个 flit，2 个虚拟通道。其他所有参数都保持默认值。

基本上，你的图应该长得和论文中图 6 差不多：

[1] Nan Jiang, Daniel U. Becker, George Michelogiannakis, James Balfour, Brian Towles, John Kim 和 William J. Dally, *A Detailed and Flexible Cycle-Accurate Network-on-Chip Simulator*, IEEE International Symposium on Performance Analysis of Systems and Software, 2013。从这里下载：[http://crd.lbl.gov/assets/booksimspass.pdf](http://crd.lbl.gov/assets/booksimspass.pdf)

## 4. 交付物

写一份报告，描述你在做本作业时遇到的问题，以及你是如何解决的。四张图（每张收集 10 个延迟 vs. 注入速率数据点）。简要讨论你的图：它们的含义以及相互比较。什么时候 NoC 性能更好？为什么？

## 参考文献

[1] Dally 和 Towles, *Route packets, not wires: on-chip interconnection networks*, 2001。
[http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=935594&amp;tag=1](http://ieeexplore.ieee.org/xpls/abs_all.jsp?arnumber=935594&tag=1)

[2] NoC 博客：[http://networkonchip.wordpress.com/2011/02/22/simulators/](http://networkonchip.wordpress.com/2011/02/22/simulators/)

[3] BookSim：[https://github.com/booksim/booksim2](https://github.com/booksim/booksim2)

[4] Noxim：[https://github.com/davidepatti/noxim](https://github.com/davidepatti/noxim)

[5] VNOC 2.0：[https://github.com/eigenpi/vnoc20](https://github.com/eigenpi/vnoc20)
