# CS/ECE 752

## 作业 6

**截止时间：2025 年 10 月 26 日（周日）中部时间上午 11:00**

**姓名：____________________**

本作业应由你独立完成，但鼓励你通过电子方式或 Piazza 与同学讨论遇到的任何问题。适用标准迟交政策：最多迟交 1 天（24 小时）不扣分。关于作业中 LLM 使用的细节，请查阅 homework 页面。

## 需要提交什么

提交本作业时：

1. 把以下题目的答案打字整理好，在 Canvas 上提交**一个名为 `HW6-<netID>.pdf` 的 PDF 文件**。如果你更喜欢手写也可以，但请扫描后在 Canvas 上提交。**请务必把你的姓名放在该 PDF 第一页上方提示处。如果可能，请尽量保留所提供 PDF/Word 文档的版式。我们将用 Gradescope 批改本作业，它依赖答案出现在固定位置。**
2. 此外，你还需要创建并上交一个归档文件（`.zip`、`.gz` 或 `.tgz`），包含以下文件：

   - 对 6 种策略中的每一种：
     - 输出日志，包含命中、缺失和访问信息，并在文件名后追加相应的替换策略名：
       - `log-lfu.txt`
       - `log-lru.txt`
       - `log-plru.txt`
       - `log-sc.txt`
       - `log-srrip.txt`
       - `log-brrip.txt`
     - 每个替换策略对应的 stats 文件（通常叫 `stats.txt`），并在文件名后追加相应的替换策略名：
       - `stats-lfu.txt`
       - `stats-lru.txt`
       - `stats-plru.txt`
       - `stats-sc.txt`
       - `stats-srrip.txt`
       - `stats-brrip.txt`
     - 你为每个替换策略的访问序列创建的 trace 文件，并在文件名后追加相应的替换策略名：
       - `trace-lfu.py`
       - `trace-lru.py`
       - `trace-plru.py`
       - `trace-sc.py`
       - `trace-srrip.py`
       - `trace-brrip.py`
   - 运行这些 trace 所需的外层 Python 文件：
     - `run_replacement_policy.py`
     - `cache_hierarchies.py`

**总分：44 分**（文件 20 分：每个 1 分；报告 24 分：每个 policy 4 分）

本作业直接基于 HW5——我们在那里考察了不同缓存替换策略在相对小而简单的轨迹上的行为。不过这次我们要学习用 gem5 跑出和你上周手算**完全相同**的轨迹！

## 假设

假设与上周相同，不过你需要按下面讨论更新脚本来相应地对系统建模。

- 我们只有单级缓存，512B，64B 块，4 路组相联。
- 你可以假设该缓存是 PIPT，地址翻译已经替你完成。
  - 注意：在本作业截止前我们不会讨论虚拟地址 vs. 物理地址——基本上 PIPT 意味着你可以认为下面给出的地址就是缓存用于访问的地址。
  - 在 gem5 中，我们用什么方式指定地址对本作业是不可知的。
- 所有地址都是 64 位。
- 所有请求都是 load。
- 系统中没有其他核、缓存等，所以这些就是仅有的内存访问（即没有来自其他核的交错内存请求）。
- 我们使用标准十六进制/二进制记法。例如 `0xABCD = (1010 1011 1100 1101)₂`。
- 你 load 的数据值不重要。

## 访问序列

对每个缓存替换策略，你都要创建下面这串访问（你可以假设未显示的位全为 0）：

```
LD  0x0      //  Access #0
LD  0x81     //  Access #1
LD  0x100    //  Access #2
LD  0x188    //  Access #3
LD  0x4      //  Access #4
LD  0x200    //  Access #5
LD  0x18C    //  Access #6
LD  0x108    //  Access #7
LD  0x380    //  Access #8
LD  0x8      //  Access #9
```

## 在 gem5 中运行上面的访问序列

给定上面的信息和访问序列，创建一个 **trace**，把它传给 gem5 的 traffic generator（流量生成器），以在 gem5 中模拟 LFU、LRU、PseudoLRU（Tree-PLRU）、SecondChance、SRRIP 和 BRRIP 替换策略在上面访问序列下的行为。为此我们需要几样东西：

- Traffic Generator（流量生成器）
- Trace 到 Traffic Generator 的输入
- 配置缓存
- 如何在 gem5 中运行
- 如何更新 gem5 以拿到受害者信息
- 输出信息（日志）

下面几节解释如何使用这些部件。

### Traffic Generator

用一个含 CPU、缓存、主存、网络等的完整系统来建模上面那点微小缓存访问序列，是可行的，但会极其复杂且浪费。幸运的是，gem5 支持一种特殊配置：traffic generator。从高层看，traffic generator 的工作方式是：用一个 traffic generator、一个缓存和一个后备主存来建模一个系统。本质上，这意味着我们的系统极其简单，可以直接把访问从 traffic generator 送入缓存来研究它们的行为——这正是本作业所要的！关于 gem5 traffic generator 的更多基础信息，如果你想读，可以看[这里](此处为原文链接，原文以)。

### Trace 到 Traffic Generator 的输入

为了生成一串内存访问直接发给缓存，traffic generator 需要某种输入刺激。虽然设计 traffic generator 有多种方式，但本作业我们只关注一种特定格式来指定一串**线性**内存访问。这个输入 trace 用 Python 编写，由若干行组成，其中一些行指定要访问的地址、何时访问；trace 还必须指定一些额外信息，例如使用什么替换策略、trace 何时结束、用一行来同步所有访问、以及访问从哪里来。在 gem5 中，所有 trace 访问都应在一个 `python_generator` 函数内完成。

trace 中每一行缓存访问如下：

```python
yield generator.createLinear(duration, startAddr, endAddr, accessSize,
    minPeriod, maxPeriod, percentReads, dataLimit)
```

其中：

- `duration`：内存访问持续多长时间（以 ticks 计）。建议所有访问都取一个一致的数（如 60000）。
- `startAddr`：请求起始的基地址。
- `endAddr`：请求结束的地址。如果 end − start 跨越多条缓存行，这会产生多个请求。
- `accessSize`：访问大小（字节）。
- `minPeriod`：如果想让请求在一段时间内重复，这个参数指定该时段从何时开始。建议本作业把 `minPeriod = maxPeriod`，但取非 0 值。
- `maxPeriod`：如果想让请求在一段时间内重复，这个参数指定该时段何时结束。建议本作业把 `minPeriod = maxPeriod`，但取非 0 值。
- `percentReads`：一个 `[0,100]` 内的整数，0 表示全是 store，100 表示全是 read。
- `dataLimit`：数据限制（字节）。本作业基本上可以忽略这个（即设为 0）。

下面是一个（FIFO 替换策略的）例子：

```python
# tell gem5 what replacement policy this trace is using
from m5.objects.ReplacementPolicies import FIFORP as rp

# define the traffic generator
def python_generator(generator):
    # All generated linear memory accesses go here (each access
    # happens after the previous one)
    yield generator.createLinear(60000, 0, 63, 64, 30000, 30000, 100, 0)
    yield generator.createLinear(60000, 128, 191, 64, 30000, 30000, 100, 0)
    ...
    # After all memory accesses, synchronize all accesses with this access
    yield generator.createLinear(30000, 0, 0, 0, 30000, 30000, 100, 0)

    # Tell the traffic generator to exit
    yield generator.createExit(0)
```

### 配置缓存

现在我们已经写好了 trace，并访问了指定模式中的各内存位置，接下来需要搭建外围脚本，以我们希望的方式配置 gem5 来跑这些测试。

所有这些，我们都利用 gem5 对 traffic generator 和 replacement policy tests 的集成——这把我们需要写的样板代码量降到最少。要开始，在你 clone 的 gem5 中访问 `cache_hierarchies.py`（特别是这一行：[https://github.com/gem5/gem5/blob/stable/tests/gem5/replacement_policies/configs/cache_hierarchies.py#L41](https://github.com/gem5/gem5/blob/stable/tests/gem5/replacement_policies/configs/cache_hierarchies.py#L41)）。巧合的是，大小和相联度已经被设成你需要的值！但如果你想在一个缓存大小或相联度不同的系统上建模，这里就是你要改的地方。另外，注意 gem5 中的 Ruby 子系统（我们本作业用它）默认假设 64B 缓存行大小——这相对硬编码进了 Ruby。此外你可能注意到，我们用的是 "MIExample" 一致性协议——本作业中你可以忽略它（学期后面会讨论一致性）。不过将来参考时注意：本作业只在 Ruby 的 MI Example 一致性协议上测试过——尽管 Ruby 有许多其他一致性协议，没有一个用这个 traffic generator 作业测试过。

### 如何在 gem5 中运行 Traffic Generator

最后，我们需要在 gem5 中运行 trace。要做这件事，有几步：

1. 要跑测试，我们使用现有的 wrapper 脚本来跑测试：[https://github.com/gem5/gem5/blob/stable/tests/gem5/replacement_policies/run_replacement_policy.py](https://github.com/gem5/gem5/blob/stable/tests/gem5/replacement_policies/run_replacement_policy.py)。顾名思义，这个脚本负责用 traffic generator、缓存和主存运行我们的测试。它负责设置带 traffic generator 的系统。**你不需要改它。**
2. 接下来需要编译 gem5。具体地，我们需要为 MI_Example 协议编译 gem5（ALL 包含 MI_Example）。做法：

   a. `cd $GEM5_HEAD`
   b. `python \`which scons\` build/ALL/gem5.opt -j9`
3. 接下来需要构建并运行测试：`build/ALL/gem5.opt tests/gem5/replacement_policies/configs/run_replacement_policy.py traces/gem5/replacement_policies/traces/hw6_example.py`

   a. 注意这里 `run_replacement_policy.py` 的格式是 `[-h] config_name config_path`。
   b. 本作业我们只需要对每个替换策略跑一个 trace，但如果你在写不同的 trace（或想把每个替换策略的 trace 串在一条命令后跑），强烈建议你写一个 bash 脚本把它们一个接一个地调用。

### 如何更新 gem5 以拿到受害者信息（可选但非常有用）

在想弄清楚某个替换策略把哪些表项驱逐出去时，给 gem5 插桩、打印额外信息、看驱逐了什么，往往很有用。幸运的是，所有 gem5 替换策略用相同格式——它们的 `getVictim()` 函数负责决定选哪个 way 作为受害者。不幸的是，默认情况下这些策略什么都不打印。

因此下面我给出一个例子，说明如何给 FIFO 替换策略的 `getVictim` 函数打印信息：它正在考虑哪个候选（"Candidate ..."）以及它最终选了哪个受害者。这两处新增打印以蓝色高亮。虽然是可选的，但我鼓励你给 gem5 加类似打印，以便对其他替换策略拿到更多正在考虑/驱逐什么的信息。

```cpp
ReplaceableEntry*
FIFO::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // Visit all candidates to find victim
    ReplaceableEntry* victim = candidates[0];
    for (const auto& candidate : candidates) {
        // Update victim entry if necessary
        if (std::static_pointer_cast<FIFOReplData>(
                    candidate->replacementData)->tickInserted <
            std::static_pointer_cast<FIFOReplData>(
                    victim->replacementData)->tickInserted) {
            printf("Candidate TickInsert: %ld, Victim TickInserted: %ld\n",
                    std::static_pointer_cast<FIFOReplData>(candidate->replacementData)->
                    tickInserted,
                    std::static_pointer_cast<FIFOReplData>(victim->
                    replacementData)->tickInserted);
            victim = candidate;
        }
    }

    printf("replacement: %s\n", victim->print().c_str());
    return victim;
}
```

你将用到的所有替换策略都在这个文件夹：[https://github.com/gem5/gem5/tree/stable/src/mem/cache/replacement_policies](https://github.com/gem5/gem5/tree/stable/src/mem/cache/replacement_policies)。注意，相对于 FIFO 替换策略，你要做的修改可能会有所不同。

### 输出信息（日志）

如果你按上面的设置运行，会得到类似这样的输出（如 stdout）：

```
...
Global frequency set at 1000000000000 ticks per second

Beginning simulation!
    74000:      system.cache_hierarchy.ruby_system.controllers.sequencer:
Cache miss at [0x0, line 0x0]
   134000:      system.cache_hierarchy.ruby_system.controllers.sequencer:
Cache miss at [0x0, line 0x80]
...
```

你应把这些信息捕获到一个日志文件，如上面交付物所规定。注意这里的地址与上面 FIFO trace 示例中的地址是如何对应的。括号中地址的格式是 `[wordAddress, line lineAddress]`。另外，注意这里打印告诉你是命中还是缺失。最后，如果你检查最终的 `stats.txt` 文件，应该也会看到与上面 simout 中打印相对应的缓存命中和缺失。

默认情况下，上面的命令把输出写到 stdout，所有 stats 写到 m5out 文件夹。我强烈建议你用 `-d` 标志，把每个替换策略的输出重定向到单独文件夹。

## 你在 gem5 中需要做什么

给定上面这些，你要为每个替换策略（LFU、LRU、TreePLRU、SecondChance、SRRIP、BRRIP）把上面那串内存访问模式写成 trace，传入上面的系统，然后把 gem5 的结果（如命中数和缺失数）与 HW5 的结果比较。对每个策略，在报告中回答下面的问题：

- HW5 中该替换策略对这串访问得到多少次命中、多少次缺失？
- gem5 中该替换策略对这串访问得到多少次命中、多少次缺失？
- 如果不匹配，它们在哪里出现分歧？

> **提示：** 对 RRIP，gem5 用单一实现同时实现了 SRRIP 和 BRRIP。你需要改变传给该类的阈值，才能按 HW5 和 HW6 规范正确地对 BRRIP 建模。

### LFU

- HW5 中 LFU 对这串访问得到多少次命中、多少次缺失？
- gem5 中 LFU 对这串访问得到多少次命中、多少次缺失？
- 如果不匹配，它们在哪里出现分歧？

### LRU

- HW5 中 LRU 对这串访问得到多少次命中、多少次缺失？
- gem5 中 LRU 对这串访问得到多少次命中、多少次缺失？
- 如果不匹配，它们在哪里出现分歧？

### Tree PLRU

- HW5 中 PLRU 对这串访问得到多少次命中、多少次缺失？
- gem5 中 PLRU 对这串访问得到多少次命中、多少次缺失？
- 如果不匹配，它们在哪里出现分歧？

### Second Chance

- HW5 中 Second Chance 对这串访问得到多少次命中、多少次缺失？
- gem5 中 Second Chance 对这串访问得到多少次命中、多少次缺失？
- 如果不匹配，它们在哪里出现分歧？

### SRRIP

- HW5 中 SRRIP 对这串访问得到多少次命中、多少次缺失？
- gem5 中 SRRIP 对这串访问得到多少次命中、多少次缺失？
- 如果不匹配，它们在哪里出现分歧？

### BRRIP

- HW5 中 BRRIP 对这串访问得到多少次命中、多少次缺失？
- gem5 中 BRRIP 对这串访问得到多少次命中、多少次缺失？
- 如果不匹配，它们在哪里出现分歧？
