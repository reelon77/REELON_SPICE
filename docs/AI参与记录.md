# AI 参与记录（复盘用）

> **用途**：项目的诚信红线是"每一行代码都能解释"。凡不是自己敲的部分，记在这里，面试前逐条复盘。
> **判定标准**：一段代码算"过关"，当且仅当你能**关掉屏幕、在白板上重写一遍，并说清每个设计取舍为什么这么选**。
> 复盘完一条，把状态改成 ✅，并写下你是怎么讲的。

---

## 总览

| 日期 | 内容 | 性质 | 复盘状态 |
|------|------|------|----------|
| 2026-07-31 | `lu_decomposition`（部分选主元 LU 分解） | **算法核心，最高优先级** | ⬜ 未复盘 |
| 2026-07-31 | `PivotLUTest` 测试组（10 个用例） | 测试设计 | ⬜ 未复盘 |
| 2026-07-31 | CMake `/utf-8` + `CMAKE_EXPORT_COMPILE_COMMANDS` | 工程配置，非算法 | ⬜ 未复盘 |
| 2026-07-31 | `scripts/build.ps1` 构建脚本 | 工具脚本，不进内核 | — 无需复盘 |
| 2026-07-31 | `项目梳理与开发计划.md` 的进度/日志/时间线更新 | 文档，非代码 | — 无需复盘 |
| 2026-08-01 | `SolveTest` 测试组（8 个用例 + 2 个辅助断言） | 测试设计 | ⬜ 未复盘 |
| 2026-08-01 | `lu_solve` 的 3 处工程性修改（**算法本体是你自己写的**） | 工程习惯，非算法 | ⬜ 未复盘 |
| 2026-08-03 | `MnaResistorTest` 测试组（4 个用例，`tests/test_mna.cpp` 全文） | 测试设计 | ⬜ 未复盘 |
| 2026-08-03 | `conductance_from` 校验辅助函数的套路（**MnaSystem/Resistor 本体是你自己写的**） | 工程习惯，非算法 | ⬜ 未复盘 |
| 2026-08-03 | `CurrentSourceTest` 测试组（3 个用例，`tests/test_sources.cpp` 全文 + `tests/CMakeLists.txt` 注册）（**CurrentSource 本体是你自己写的，含符号 bug 也是你按提示自己改对的**） | 测试设计 | ⬜ 未复盘 |
| 2026-08-03 | `VoltageSourceTest` 测试组（3 个用例，续写进 `tests/test_sources.cpp`）（**VoltageSource 本体是你自己写的**：raw 混合坐标的减一/跳地是你按伪代码提示自己完成的） | 测试设计 | ⬜ 未复盘 |
| 2026-08-03 | `EndToEndGateTest`（`tests/test_end_to_end.cpp` 全文 + CMake 注册）——**8V 闸门测试，简历物证，必须逐行能讲** | 测试设计 | ⬜ 未复盘 |
| 2026-08-13 | `SpiceValueTest`（8 个用例，`tests/test_parser.cpp` 数值解析部分） | 测试设计 | ⬜ 未复盘 |
| 2026-08-14 | `TokenizeTest`（6 个用例，续写进 `tests/test_parser.cpp`）+ `src/CMakeLists.txt` 挂载 | 测试设计 / 工程配置 | ⬜ 未复盘 |
| 2026-08-20 | `CircuitParserTest`（4 个用例）+ `src/CMakeLists.txt` 挂载 `Circuit.h/.cpp` | 测试设计 / 工程配置 | ⬜ 未复盘 |
| 2026-08-20 | `CircuitNodeMappingTest`（3 个用例，`tests/test_parser.cpp` 语义层第 2 档） | 测试设计 | ⬜ 未复盘 |
| 2026-08-21 | `CircuitDeviceParsingTest`（5 个用例，`tests/test_parser.cpp` 语义层第 3 档） | 测试设计 | ⬜ 未复盘 |
| 2026-08-21 | `ParserEndToEndTest`（2 个用例，`tests/test_end_to_end.cpp` 解析器第 4 档） | 测试设计 | ⬜ 未复盘 |
| 2026-08-22 | `CapacitorConstructionTest` / `CapacitorDcTest` / `CapacitorTransientTest` / `TransientContextDispatchTest`（8 个用例，`tests/test_capacitor.cpp`）+ `tests/CMakeLists.txt` 注册；2026-08-25 按用户明确授权机械挂载 `src/CMakeLists.txt` | 测试设计 / 工程配置 | ⬜ 未复盘 |
| 2026-08-25 | `NewtonInitialGuessTest`（2 个用例，`tests/test_newton.cpp` 瞬态第 2A 档） | 测试设计 | ⬜ 未复盘 |
| 2026-08-25 | `NewtonTransientContextTest` + 测试专用 `RecordingTransientDevice`（`tests/test_newton.cpp` 瞬态第 2B 档） | 测试设计 | ⬜ 未复盘 |
| 2026-08-25 | `NewtonTransientStepTest`（1 个用例，`tests/test_newton.cpp` 瞬态第 2C 档） | 测试设计 | ⬜ 未复盘 |
| 2026-08-25 | `TransientTrajectoryTest` / `TransientValidationTest` / `TransientHistoryTest` / `TransientRcTest` / `TransientFailureTest`（7 个用例，`tests/test_transient.cpp` 瞬态第 3 档）+ `tests/CMakeLists.txt` 注册 | 测试设计 / 工程配置 | ⬜ 未复盘 |
| 2026-08-26 | `InductorConstructionTest` / `InductorDcTest` / `InductorTransientTest` / `InductorTrajectoryTest`（7 个用例，`tests/test_inductor.cpp` 瞬态第 4 档）+ `tests/CMakeLists.txt` 注册 | 测试设计 / 工程配置 | ⬜ 未复盘 |
| 2026-08-26 | `CircuitTransientParsingTest`（5 个用例）+ `ParserTransientEndToEndTest`（2 个用例）；扩展 4 个旧 parser 测试覆盖 C/L 节点、类型、token/数值错误，并迁移旧 parser/DC 测试的通用支路计数字段 | 测试设计 | ⬜ 未复盘 |

> **`lu_solve` 的归属要分清**：重排 + 前代 + 回代的**算法逻辑全部是你自己写的**，包括那个致命 bug 也是你自己
> 按提示改对的 —— 这部分**不算 AI 代写，面试可以理直气壮说是自己实现的**。AI 只改了三处与算法无关的东西：
> 参数改 `const&`（避免整矩阵拷贝）、补维度校验、`int`/`size_t` 统一。见下方第四节。

**自己写的部分（不在本文档范围，无需复盘）**：`Matrix` 类、`lu_decomposition_naked`、`find_nonzero_row`、`exchange_rows`、`LUTest` 夹具与前 9 个测试、CMake 三层骨架。

---

## 一、`lu_decomposition` —— 部分选主元 LU 分解 ⭐ 重点

**位置**：`src/core/LU.cpp:70-148`（含上方注释块）
**日期**：2026-07-31
**背景**：原本是 Day1 下午自己要写的任务，卡了 4 天未动，第 2 周已开始，为止损让 AI 代写。

### 面试必答清单

复盘时逐题自问，答不上来就回去读代码，读完再合上重答。

**Q1. 为什么循环是列优先（k 在外层），而不是你 naked 版那种行优先？**

> 关键在**主元检查的时机**。naked 版在处理第 i 行时，先查 `A(i,i)` 是否为 0、再做消元；但主元是**消元之后**才可能变成 0 的。所以 `[[1,2],[2,4]]` 这种奇异矩阵：轮到第 1 行时 `A(1,1)` 还是原始的 4，检查放行，消元后才变 0，而此时已经没人再检查它了 —— 奇异矩阵静默通过，`U(1,1)=0` 被写进结果。
> 列优先每轮的顺序是「定第 k 列主元 → 立刻检查 → 再用它消下面所有行」，定稿与检查发生在同一时刻，结构上不可能漏。

**Q2. 为什么选绝对值最大的行，而不是第一个非零行？**

> 数值稳定性，不是正确性。乘数 `m = A(i,k)/A(k,k)`，选最大主元保证 **|m| ≤ 1**；消元式 `A(i,j) -= m·A(k,j)` 于是不会放大已有的舍入误差。若主元很小，`1/主元` 会把误差放大若干数量级。
> 第一个非零也能算完，但结果可能全是噪声。
> 代码里把这条写成了断言：`expect_multipliers_bounded` 检查所有 `|L(i,j)| ≤ 1`，选主元一旦失效测试立刻红。

**Q3. 为什么就地分解（乘数写回 `work(i,k)`），而不是直接往 L、U 两个矩阵里填？**

> 因为**换行时，前几轮已经算好的 L 乘数必须跟着一起换**。就地存储时它们和 U 的部分共处同一行，`exchange_rows` 整行一 swap 就自动正确；L/U 分开存两个矩阵，就必须记得同时 swap L 的左半边 —— 这是这个算法最经典的漏点（交接单也专门预警过）。
> 最后第 5 步才把 `work` 拆成独立的 L、U。

**Q4. `perm` 的语义是什么？为什么不构造真正的置换矩阵 P？**

> `perm[i] = j` 表示 `P·A` 的第 i 行取自 A 的第 j 行，满足 `L·U = P·A`。
> 不构造 P 矩阵是因为置换矩阵每行只有一个 1，用 n×n 存是 O(n²) 空间 + O(n³) 乘法开销；一个长度 n 的整数数组就够，后续 solve 时按 `perm` 重排 b 即可，O(n)。

**Q5. `kPivotEps = 1e-12` 这个阈值，物理含义是什么？**

> 对应电路里的**节点悬空 / 整个网络未接地**。未接地时 G 矩阵是带权图拉普拉斯，行和为 0 ⇒ 奇异 ⇒ 节点电压无唯一解（整体浮动一个常数）。必须在这里抛异常，而不是让 `1/0` 变成 inf 一路传到结果里。
> 用阈值而不是 `== 0`，是因为浮点消元后的主元几乎不会精确为 0，只会是 1e-18 这种量级的数值垃圾。
>
> ⚠️ **延伸思考（自己想清楚再答）**：绝对阈值 `1e-12` 对量纲敏感 —— 电导单位是西门子，1kΩ 电阻的电导是 1e-3；如果电路里全是 GΩ 级电阻（电导 1e-9），正常主元可能就逼近阈值而被误判奇异。更稳的做法是**相对阈值**（相对于该列或整个矩阵的范数）。这是一个能主动抛出、显示你想得比代码深的点。

**Q6. 复杂度？**

> 分解 O(n³)，选主元每轮 O(n−k) 扫描，总计 O(n²)，不改变量级。空间 O(n²)（一份工作副本）。
> 这也是第 3 周做稀疏 CSR 的动机：电路矩阵极度稀疏，稠密 O(n³) 在几千节点就撑不住了。

### 白板自测

不看代码，默写出下面这个循环骨架：

```
for k = 0 .. n-1:
    p = argmax_{i≥k} |A(i,k)|
    if |A(p,k)| < eps: throw 奇异
    swap row k, p;  swap perm[k], perm[p]
    for i = k+1 .. n-1:
        m = A(i,k) / A(k,k)
        A(i,k) = m
        for j = k+1 .. n-1:
            A(i,j) -= m * A(k,j)
```

默写通过后，手推 `A = [[0,2,1],[1,1,1],[2,0,1]]` 的完整分解过程（首列主元应选第 2 行，因为 |2| 最大），与程序输出对拍。

---

## 二、`PivotLUTest` 测试组 ⭐ 次重点

**位置**：`tests/test_lu.cpp:179-316`
**内容**：`PivotLUTest` 夹具（2 个新断言辅助）+ 10 个测试用例。

测试代码面试一般不逐行问，但**"你怎么验证它是对的"是必问题**。要能讲清这 10 个用例分别在防什么：

| 用例 | 防什么 |
|------|--------|
| `WellConditionedReconstructsPA` | 基本正确性：L·U = P·A |
| `PivotBecomesZeroMidway` | naked 版产生 inf/nan 的那个场景 |
| `NeedsRowSwapAtStart` | 首元为 0，且断言 `perm[0]==2`（选最大而非第一个非零） |
| `PicksLargestMagnitudeNotFirstNonzero` | **与 naked 版的分水岭**：`[[1,2],[100,3]]` 必须选第 1 行 |
| `SingularMatrixThrows` | 奇异必须抛异常，不是 NaN |
| `UngroundedLaplacianThrows` | 电路语义：未接地 ⇒ 奇异 |
| `GroundedGMatrixIsNonsingular` | 上一条的对照组，防止"什么都判奇异"的假阳性 |
| `NonSquareThrows` / `MismatchedLUSizeThrows` | 入参防御 |
| `OverwritesDirtyOutputMatrices` | 不依赖调用方传全零 L/U |

两个辅助断言值得单独理解：

- `expect_LU_equals_PA`：比 naked 版用的"行集合是重排"更严 —— **perm 必须精确对得上**，即 `(L·U)` 第 i 行 == `A` 第 `perm[i]` 行。naked 版的 perm 是假的（硬填恒等），这个断言就是为了防同类问题。
- `expect_multipliers_bounded`：把"选主元"这个性质本身变成可测断言（见 Q2）。

**顺带修正的一处知识错误**：交接单称 `[[0.001,−0.001],[−0.001,0.0015]]` 是"未接地拉普拉斯，奇异"，但其 det = 1.5e−6 − 1e−6 = **5e−7 ≠ 0**，非奇异。真正未接地的是 `[[0.001,−0.001],[−0.001,0.001]]`（行和为 0）。0.0015 = 1/1k + 1/2k 恰恰说明节点 2 **经 R2 接了地**。两者现在作为对照组同时入测。
→ 这是理解"行和为 0 ⇔ 未接地 ⇔ 奇异"的好素材，务必自己重新推一遍。

---

## 三、CMake 构建链修复（工程配置，非算法）

**位置**：`CMakeLists.txt:8-12`、`scripts/build.ps1`

| 改动 | 原因 |
|------|------|
| `add_compile_options(/utf-8)`（MSVC） | 源码是 UTF-8 无 BOM，MSVC 默认按本地代码页 936(GBK) 读，中文字符串字面量的字节被误解析、吃掉收尾引号 → `test_lu.cpp` 原本**编译不过**（error C2001 常量中有换行符）。这是真 bug，不是 IDE 误报。 |
| `set(CMAKE_EXPORT_COMPILE_COMMANDS ON)` | 生成 `compile_commands.json` 供 clangd 用。之前没有它，clangd 跑 fallback 模式，`tests/` 里的 include 全报红。 |
| 补 `lu_decomposition` 空函数体 | 非 void 函数不 return：clang 下 `-Wreturn-type` 报错、MSVC C4716。当时先填 `throw`，后被正式实现替换。 |

这部分是环境问题，面试不会问，了解即可，**不必投入复盘时间**。

### 附：本机构建（已封装成脚本，不必手敲）

```powershell
.\scripts\build.ps1              # 配置(首次) + 构建
.\scripts\build.ps1 -Test        # 构建后跑 ctest
.\scripts\build.ps1 -Fresh       # 删掉 build/ 重新配置
```

脚本做了三件手敲时容易漏的事：进 VS 开发者环境（PATH 里没有 cl/ninja）、把
`compile_commands.json` 拷到仓库根供 clangd 用、以及下面这个编码坑的规避。

### ⚠️ Windows 文本编码坑（同一个根因，已咬了三次）

**"这个文本文件到底按什么编码读"在 Windows 工具链上是反复出现的问题，不是一次性 bug**：

| 场次 | 现象 | 根因 | 解法 |
|------|------|------|------|
| 1. MSVC 编译 | `error C2001: 常量中有换行符`，`test_lu` 编译不过 | cl.exe 默认按代码页 936(GBK) 读 UTF-8 无 BOM 源码，中文字面量的字节被拆错、吃掉收尾引号 | 根 CMakeLists 加 `/utf-8` |
| 2. 终端乱码 | `'vswhere.exe' ���...` 一屏乱码 | `Enter-VsDevShell` 内部调 cmd.exe，而 vswhere 所在 Installer 目录不在 PATH；cmd 用 GBK 吐"不是内部或外部命令"，这段字节被按 UTF-8 读 | **把 Installer 目录加进 PATH，让报错根本不发生**（而不是去调 chcp——终端本来就已经是 65001 了） |
| 3. PowerShell 脚本 | `Unexpected token '}'`，脚本跑不起来 | Windows PowerShell 5.1 对**没有 BOM** 的 `.ps1` 一律按 ANSI(936) 解码，中文注释拆错字节 | `build.ps1` 必须存成 **UTF-8 with BOM** |

第 2 条的思路值得记住：**乱码的正解往往是消除产生乱码的那条消息，而不是折腾编码设置**。

---

## 四、`lu_solve` 相关（2026-08-01）

### 算法本体：你自己写的，不需要按"AI 代写"复盘

但**过程中的那个 bug 值得单独记住**，它是这类算法最经典的错误：

> 第一版里 `perm` 声明了、分解时也维护了，但 `lu_solve` **一次都没用**。
> 结果解出来的是 `A·x = P⁻¹b` 而不是 `A·x = b`。
> 症状很有辨识度：`A·x` 算出来正好等于 **b 的若干项按 perm 对调后的结果**。

**为什么单测差点没抓住它**：第一个想到的验证用例 `perm=[1,0,2]`（单次对调），而
**对调的逆等于它自身** —— 就算把 `y[i]=b[perm[i]]` 写成反方向的 `y[perm[i]]=b[i]`，
结果也完全一样。必须用**含长度 > 2 循环的置换**（如 `[2,0,1]`）才能验出方向。
`SolveTest.ThreeByThreeWithThreeCyclePermutation` 里的 `has_long_cycle` 断言就是守这个的。

更早还踩过一次同类陷阱：`b=[8,8,13]` 时 `b[0]==b[1]`，置换退化成空操作，**漏掉重排也照样通过**。
`PermutationDegeneratesWhenSwappedEntriesAreEqual` 特意保留了这个用例作反面教材。

👉 **通用教训：测试用例本身可能因为对称性/退化而丧失鉴别力。设计用例时要问一句
"如果代码写错了，这个用例真的会红吗？"**

### AI 实际改的三处（都与算法无关）

| 改动 | 原因 |
|------|------|
| `LUResult matrix` → `const LUResult& lu`；`vector<double> b` → `const vector<double>&` | 按值传会拷贝两个 n×n 矩阵。牛顿迭代每步、瞬态每个时间步都调 solve，拷贝会落在热路径上 |
| 补维度校验（`b.size()` / `perm.size()` 对不上就抛 `invalid_argument`） | 原来会越界读写；且 `lu_decomposition` 有三重校验，风格要统一 |
| `const int n = static_cast<int>(b.size())` 统一索引类型 | 消 4 个 C4267 警告；`int i = b.size()-1` 在 b 为空时是实现定义行为 |

另外把中间结果直接用 `y` 算到底，省掉原来 `b = b_` 那次多余的整向量拷贝。

### `SolveTest` 测试组

8 个用例，每个查**两条互相独立的判据**：与手推答案比对（验"答案对不对"）+ 残差 `|A·x − b|`（验"方程解没解对"）。

**为什么必须双判据**：本项目已经两次栽在**错误的手推答案**上（交接单的 3×3、以及验证过程中一次算错的期望值）。
只比对期望值时，红了你会以为是求解器坏了去瞎改；加上残差就能立刻区分——
**残差为 0 但期望值对不上 ⇒ 是手推答案错了，不是代码错了**。

自测题：
1. 为什么单次对调的置换测不出正/逆写反？举例说明。
2. `expect_solves` 里如果只留残差检查、去掉期望值比对，会漏掉哪一类 bug？（提示：想想 x 全为 0 的退化情形）

---

## 五、`MnaResistorTest` 测试组（2026-08-03）

**位置**：`tests/test_mna.cpp` 全文（AI 代写）；**MnaSystem、Device、Resistor 三个类全部是你自己写的，不在复盘范围**。
AI 另给过 `conductance_from` 的"文件局部辅助函数 + 初始化列表调用"套路（校验先于除法），逻辑是你的，位置安排是 AI 的。

4 个用例分别在防什么（面试"你怎么验证"必答）：

| 用例 | 防什么 |
|------|--------|
| `SingleResistorStampsFourEntries` | 4 个落点、±g 符号、G 对称性、b 不被电阻污染 |
| `SharedNodeConductancesAccumulate` | **`+=` 卫兵**：你 Day1 连错三次的已知弱点，共享对角元 = 电导之和而非覆盖 |
| `GroundedResistorTouchesOnlyOwnDiagonal` | 跳地逻辑：接地器件 4 次调用只有 1 次落盘 |
| `NonPositiveResistanceThrows` | 构造期校验：0 阻（无穷电导）与负阻拒收 |

自测题：如果 `add_to_A` 忘了写跳地的 `return`，哪个用例会以什么方式失败？（提示：下标 −1，越界写——想想为什么它可能不是干净地崩溃）

---

## 六、`CurrentSourceTest` 测试组（2026-08-03）

**位置**：`tests/test_sources.cpp` 全文（AI 代写）+ `tests/CMakeLists.txt` 的 `test_sources` 注册三行；
**CurrentSource.h/.cpp 本体是你自己写的**——初版符号反了（a 端写了 +I），是你按"a→b 意味着从 a 抽走 I"的提示自己改对的，这段推导面试必答。

3 个用例分别在防什么：

| 用例 | 防什么 |
|------|--------|
| `StampsTwoEntriesWithCorrectSigns` | 落点与符号：b(a) = −I、b(b) = +I，且 A 全零——电流源碰 A 就是 bug |
| `GroundedTerminalIsSkipped` | 跳地：任一端接地时该端写入静默消失，双向各测一次 |
| `CoexistsWithResistorWithoutPollutingA` | 器件正交性：电阻只写 A、电流源只写 b，同系统先后 stamp 互不污染 |

自测题：为什么电流源不进 A 矩阵、电压源却要升维？（提示：谁的电流是已知数？）

---

## 七、`VoltageSourceTest` 测试组（2026-08-03）

**位置**：`tests/test_sources.cpp` 的三个 `VoltageSourceTest` 用例（AI 代写）；
**VoltageSource.h/.cpp 本体是你自己写的**。过程中你走过两条弯路，都值得复盘：
① 5 次写入先用了 `add_to_A(…, k)`（矩阵下标混进节点接口，列错位一格）；
② 又试过 `k + 1` 抵消内部减一（能算对但依赖实现细节的 hack）；
③ 期间误删过 `MnaSystem::add_to_b` 的跳地卫兵（改公共模块迁就局部 bug，已还原）。
最终版：混合坐标走 raw、节点维自己做"跳地 + 减一"——这条演化路径面试可以直接当案例讲。

3 个用例分别在防什么：

| 用例 | 防什么 |
|------|--------|
| `DimensionIncludesBranchUnknown` | 升维：dim = 节点数−1+源数，支路下标排在节点块之后 |
| `StampsFiveEntriesInMixedCoordinates` | 5 个落点、KCL 行与约束行符号配对（转置对称）、G 子块/支路对角元/b 节点行不被污染 |
| `GroundedNegativeTerminalLeavesThreeEntries` | 接地端两次写入整体消失（5→3），下标不越界 |

自测题：把 KCL 行的 ±1 删掉（只留约束行），矩阵会发生什么？为什么物理上也必然如此？
（提示：第 k 列整列为零意味着什么？那 2mA 从哪来？）

---

## 八、`CircuitParserTest` 测试组（2026-08-20）

**位置**：`tests/test_parser.cpp` 的 4 个 `CircuitParserTest` 用例（AI 代写），
以及 `src/CMakeLists.txt` 中 `Circuit.h/.cpp` 的机械挂载；
**`Circuit` 与 `parse_circuit` 本体由用户亲手编写**。

| 用例 | 防什么 |
|------|--------|
| `ParsesOpWhileSkippingBlankAndCommentLines` | `getline → tokenize` 主循环、空行/注释跳过、`.op` 状态和默认空电路字段 |
| `EndStopsBeforeInvalidFollowingContent` | `.end` 必须立即停止；同时防止 `.end` 自己错误地设置 `Op` |
| `RejectsOpArgumentsAndReportsLineNumber` | `.op` 参数数量校验及从 1 开始的真实网表行号 |
| `RejectsEndArgumentsAndReportsLineNumber` | `.end` 必须先校验参数再退出，并报告真实行号 |

---

## 九、`CircuitNodeMappingTest` 测试组（2026-08-20）

**位置**：`tests/test_parser.cpp` 的 3 个 `CircuitNodeMappingTest` 用例（AI 代写）；
**节点映射及其与 `parse_circuit` 的集成由用户亲手编写**。

| 用例 | 防什么 |
|------|--------|
| `ZeroAndGndShareTheGroundNode` | `0` / `gnd` 两个名称必须共用编号 0，不能按 map 键数量多算节点 |
| `RepeatedNamesReuseExistingNodeNumbers` | `in/out/0/gnd/in` 的首次分配与重复复用，最终节点总数为 3 |
| `EverySupportedDevicePrefixContributesNodes` | R/V/I/D 四种合法首字母都能触发两个端点的节点映射 |

---

## 十、`CircuitDeviceParsingTest` 测试组（2026-08-21）

**位置**：`tests/test_parser.cpp` 的 5 个 `CircuitDeviceParsingTest` 用例（AI 代写）；
**R/V/I/D 的解析、节点映射、电压源编号和异常出口均由用户亲手编写**。

| 用例 | 防什么 |
|------|--------|
| `CreatesEverySupportedDeviceInNetlistOrder` | R/V/I/D 实际构造成正确的动态类型，保持网表顺序，并校验节点数、电压源数和 `.op` 状态 |
| `RejectsUnknownDeviceAndReportsLineAndToken` | 未知器件不能静默忽略，异常必须带真实行号与首 token |
| `RejectsUnsupportedDirectiveAndReportsLineAndToken` | `.dc` 等停车场指令不得被提前实现或静默接受 |
| `RejectsWrongTokenCountForEveryDeviceKind` | R/V/I 必须恰好 4 个 token，D 必须恰好 3 个；同时防止偷偷接受二极管模型名 |
| `RejectsInvalidNumericValueWithLineAndToken` | R/V/I 都必须经过统一数值解析和行号包装，不能将非法值带入器件构造 |

---

## 十一、`ParserEndToEndTest` 测试组（2026-08-21）

**位置**：`tests/test_end_to_end.cpp` 的 2 个 `ParserEndToEndTest` 用例（AI 代写）；
**parser、MNA、器件 stamp、LU 和牛顿求解器实现均由用户亲手编写**。

| 用例 | 防什么 |
|------|--------|
| `VoltageDividerNetlistSolvesTo8V` | 真实网表文本从切词、节点映射、R/V 构造一路进入 MNA/LU，并恢复 `v1=10V`、`v2=8V`、`iV1=-2mA` |
| `DiodeNetlistConvergesToExpectedOperatingPoint` | D 行确实使用默认 `Is/Vt`并参与牛顿迭代，工作点收敛到 `v2=0.574191503V` |

`borrow_devices` 只从 `Circuit` 借用 `Device*`，不转移所有权；两个测试执行期间 `Circuit` 始终存活。

---

## 十二、`Capacitor` 与 `TransientContext` 测试组（2026-08-22）

**位置**：`tests/test_capacitor.cpp` 的 8 个用例（AI 代写）、`tests/CMakeLists.txt` 的测试注册，
以及用户于 2026-08-25 明确授权后由 AI 完成的 `src/CMakeLists.txt` 机械挂载；
**`TransientContext`、Device 接口扩展与 `Capacitor` 本体必须由用户亲手编写**。

全部数值 oracle 独立来自后向欧拉：

```text
i_n = (C/dt)(v_n-v_prev)
G_eq = C/dt
I_hist = -G_eq*v_prev
```

其中主数值例固定取 `C=2F、dt=0.5s、v_prev=5V-2V=3V`，手算得到
`G_eq=4S、I_hist=-12A、A=[[4,-4],[-4,4]]、b=[12,-12]`，不使用待测实现生成期望值。

| 用例 | 防什么 |
|------|--------|
| `RejectsNonPositiveCapacitance` | `C<=0` 必须在构造期拒绝 |
| `StampIsOpenCircuitAndPreservesExistingSystem` | DC 电容严格开路，且不得清除其他器件已经累加的 A/b |
| `StampsFourPointConductanceAndHistorySourceFromPreviousSolution` | `G_eq/I_hist`、A 四点、b 两点、读取 `x_prev` 而非当前 Newton `x`，并守住历史只读 |
| `GroundedNegativeTerminalLeavesOneDiagonalAndOneRhsEntry` | 接地跳过后的单对角元/单 RHS 落点与符号 |
| `StampAccumulatesInsteadOfOverwriting` | stamp 必须使用 `+=`，重复盖章恰好翻倍 |
| `RejectsNonPositiveTimeStep` | `dt<=0` 不得产生除零、负等效电导或静默错误 |
| `LinearDeviceFallsBackToExistingStamp` | 新 context 虚接口默认转发，R/V/I 等线性器件旧路径不回归 |
| `DiodeFallsBackToCurrentNewtonIterate` | context 默认转发仍虚分派到 Diode 双参版本，Diode 读取当前 `x` 而非历史 `x_prev` |

---

## 十三、Newton 显式初值测试组（2026-08-25）

**位置**：`tests/test_newton.cpp` 的 2 个 `NewtonInitialGuessTest` 用例（AI 代写）；
**Newton 初值接口与实现必须由用户亲手编写**。

| 用例 | 独立 oracle 与防护目标 |
|------|------------------------|
| `ExactSolutionAsInitialGuessConvergesInOneIteration` | 分压电路手算真解为 `{10V, 8V, -2mA}`；线性方程每轮相同，真解作初值时第一轮 `diff=0`，因此应恰好 1 轮收敛。防止新重载仍偷偷从全零开始 |
| `RejectsInitialGuessWithWrongDimension` | `sys.dim()=3` 而初值只有 2 项，必须在 Newton 入口抛 `invalid_argument`，防止范数比较越界读取 |

旧 DC 重载的源码兼容和默认全零行为继续由已有 `NewtonSmokeTest`、`NewtonEndToEndTest` 与全仓回归守护。

第 2B 的 `NewtonTransientContextTest.ReusesFrozenContextAcrossEveryIteration` 使用测试专用线性器件
`A=1、b=x_prev+1`。取 `x_prev=7` 时，每轮独立方程均为 `x=8`：第一轮从初值 7 得到 8，
第二轮仍得到 8 并以 `diff=0` 收敛。该器件同时记录 context 地址、历史底层地址和值，验证 Newton
每轮传入同一份未变化的 context；其 DC stamp 故意抛异常，防止瞬态入口误走旧双参/单参路径。

---

## 十四、Newton 瞬态 RC 单步测试（2026-08-25）

**位置**：`tests/test_newton.cpp` 的
`NewtonTransientStepTest.BackwardEulerRcFirstStepMatchesHandCalculation`（AI 代写）；
**Newton、器件和瞬态 stamp 实现均由用户亲手编写**。

独立手算 oracle 取 `Vs=1V、R=1Ω、C=1F、dt=0.1s、vC(0)=0`。节点 2 的后向欧拉 KCL 为：

```text
(v2 - 1)/1 + (1/0.1)(v2 - 0) = 0
11 v2 = 1
```

因此第一步必须得到 `v2=1/11V`；同时核对电源节点 `v1=1V`、电压源支路电流
`iV1=-(1-v2)=-10/11A`，并确认调用结束后输入历史 `x_prev` 仍为全零。

---

## 十五、瞬态时间轨迹与 RC 验收测试（2026-08-25）

**位置**：`tests/test_transient.cpp` 的 7 个用例（AI 代写）及 `tests/CMakeLists.txt` 注册；
**`TransientPoint`、`transient_solve` 与时间步进实现必须由用户亲手编写**。

已锁定的接口语义：轨迹包含 `t=0` 初始解；`t_step>0`、`t_stop>=0`、初始解维度正确；
令 `q=t_stop/t_step`，仅当 `|q-round(q)|<=1e-12` 时接受固定时间网格，随后使用整数步号生成时间。

| 用例 | 独立 oracle 与防护目标 |
|------|------------------------|
| `ZeroStopReturnsOnlyInitialPoint` | `t_stop=0` 时只有 `{0, initial_x}`，且不得调用 Newton/stamp |
| `RejectsInvalidArguments` | 拒绝非正步长、负停止时间和错误初值维度；维度检查不能依赖进入 Newton |
| `AppliesOneEminus12RatioTolerance` | 商距整数 `5e-13` 时接受、`2e-12` 时拒绝，并拒绝明确非整数倍 `0.25/0.1` |
| `AdvancesHistoryOnlyAfterStepConverges` | 测试方程 `x_n=x_{n-1}+1` 每步两轮收敛；轨迹为 `[0,1,2]`，stamp 所见历史为 `[0,0,1,1]` |
| `TrajectoryMatchesBackwardEulerClosedForm` | `Vs=R=C=1、dt=0.1` 时 `v_n=1-(10/11)^n`；同时检查首步 `1/11`、单调上升且不超过 `1V` |
| `SmallerStepApproachesExactRcSolution` | 固定 `t=1`，分别对拍 `dt=0.2/0.1` 的后向欧拉闭式值，并验证细步长更接近 `1-e^-1` |
| `PropagatesNewtonFailureInsteadOfReturningPartialTrajectory` | 第一步收敛、第二步在 `max_iter=1` 下失败时必须向外抛异常，不得返回第一步的部分轨迹 |

本档不覆盖字符串时间、parser/`.tran`、末步缩短、变步长、梯形法、时变源、CSV/CLI、ngspice 或可视化。

---

## 十六、电感 MNA 支路未知量与 RL 轨迹测试（2026-08-26）

**位置**：`tests/test_inductor.cpp` 的 7 个用例（AI 代写）及 `tests/CMakeLists.txt` 注册；
**`Inductor`、MNA 支路计数语义与瞬态核心实现必须由用户亲手编写**。

已锁定电感电流正方向为 `p -> q`。后向欧拉的独立方程为：

```text
v_p - v_q - (L/dt)i_n = -(L/dt)i_prev
```

若 `k` 是电感电流的全局 MNA 下标，则瞬态 stamp 的五个矩阵落点和一个 RHS 落点为：
`A(p,k)+=1`、`A(q,k)-=1`、`A(k,p)+=1`、`A(k,q)-=1`、
`A(k,k)-=L/dt`、`b(k)-=(L/dt)i_prev`。DC 时只保留前四项，精确表达 `v_p-v_q=0`。

主数值例固定取 `L=2H、dt=0.5s、i_prev=3A`，手算得到 `L/dt=4ohm`、
`b(k)=-12V`；另用 `i_prev=-2A` 的接地例独立检查 RHS 正号，并用重复盖章检查所有贡献均为累加。

单位串联 RL 轨迹取 `Vs=R=L=1、dt=0.1、i_0=0`。独立后向欧拉闭式 oracle 为：

```text
i_n = 1 - (10/11)^n
v1_n = 1
v2_n = 1 - i_n
iV_n = -i_n
```

轨迹解向量固定为 `[v1,v2,iV,iL]`，因此还同时守护电压源与电感共享全局支路编号空间。
本档不接入 `Circuit`/parser/`.tran`，也不涉及梯形法、变步长、时变源、输出或可视化。

---

## 十七、`C/L/.tran` parser 与瞬态端到端测试（2026-08-26）

**位置**：`tests/test_parser.cpp` 新增 5 个 `CircuitTransientParsingTest`、
`tests/test_end_to_end.cpp` 新增 2 个 `ParserTransientEndToEndTest`；同时扩展既有的
`EverySupportedDevicePrefixContributesNodes`、`CreatesEverySupportedDeviceInNetlistOrder`、
`RejectsWrongTokenCountForEveryDeviceKind`、`RejectsInvalidNumericValueWithLineAndToken` 来覆盖 C/L，
并机械迁移旧 parser/DC 测试中的通用支路计数字段；
**`Circuit` 接口、C/L/.tran 解析和错误处理必须由用户亲手编写**。

parser 契约覆盖：C/L 各恰好 4 个 token、`.tran t_step t_stop` 恰好 3 个 token、
大小写不敏感、数值复用倍率后缀、非法数值保留行号和 token、非正 C/L 与非正时间参数保留行号。
V/L 按网表出现顺序共用 `num_branch_unknowns`，C 不增加支路未知量。RL 网表故意把 L 写在 V 前，
防止两类器件各自从 0 编号后碰撞。

RC 数值 oracle 独立取：

```text
Vs=2V, R=2ohm, C=0.5F, dt=0.25s
beta=RC/(RC+dt)=0.8
vC1=0.4V, vC2=0.72V
iV1=-0.8A, iV2=-0.64A
```

RL 数值 oracle 独立取：

```text
Vs=2V, R=2ohm, L=0.5H, dt=0.25s
beta=L/(L+R*dt)=0.5, Iinf=Vs/R=1A
iL1=0.5A, iL2=0.75A
vL1=1V, vL2=0.5V, iV=-iL
```

两条 parsed 轨迹还逐项对拍等参数的手工构造电路；手工构造结果只验证 parser 等价性，
上述独立手算值才是数值正确性的 oracle，统一容差为 `1e-12`。

本档不覆盖梯形法、变步长、时变源、`.ic/UIC`、分析指令冲突、CLI/CSV、ngspice、CSR 或可视化；
时间网格整数倍规则继续由 `transient_solve` 独立负责，不在 parser 重复实现。

---

## 复盘节奏建议

- **第一遍**（写完当天或次日）：读 `lu_decomposition` 全文 + 本文档 Q1–Q6，把答不上来的标出来。
- **第二遍**（一周内）：合上代码做「白板自测」，手推 3×3 对拍。
- **第三遍**（投简历/面试前）：只看 Q1–Q6 的问题、不看答案，口述一遍。
- 三遍都过了，把总览表的状态改成 ✅。
