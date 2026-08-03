# ⛔ 先读这两行

1. **你是教练，不是代办**。这是一节技能课：所有操作（装扩展、写配置、打断点、按 F5）由用户**亲手做**，你讲原理、给步骤方向、他卡住时指路。配置文件也尽量让他自己敲——launch.json 就十几行，抄一遍才记得住。若最终有你代写的配置，提醒登记进 `docs/AI参与记录.md`（注明"配置文件，非算法代码"）。
2. 概念讲解遵循用户风格：中文、直接、翻译成他熟悉的语言（数学/系统抽象）；公式和示意用 Unicode，不用 LaTeX。**问题停车场机制**照旧：非阻塞的"为什么"攒到断点再答。

---

# 会话交接 · 技能课：VSCode 单测调试 + CMake/Ninja/编译器工具链

> 执行日期：2026-08-04 建议排在 Day 4 牛顿任务**开工之前**（调试器马上就能用来跟牛顿迭代循环）。
> **时间盒：1.5~2 小时，硬上限**。08-05 里程碑（非线性 DC）没有缓冲，本课不许挤占它——
> 若时间告急，只做第三节实操，概念课删减进停车场。**完成后写回执**（见文末）。

## 一、背景

- 用户：数学本硕 + C++。**没系统用过调试器**，此前排错靠打印和读代码。跨两台机器工作：
  Windows（MSVC，`scripts/build.ps1`）+ macOS（当前机器）。
- 项目状态：第 1 周闸门已过（端到端 8V），43 用例 41 绿，挂的 2 个是教学版 naked LU 的已知缺陷——本课压轴练习正好拿它开刀。

**当前机器工具链事实（主会话 2026-08-03 实查，可直接引用）**：

| 项 | 值 |
|----|-----|
| 编译器 | AppleClang 21.0.0（`CMAKE_CXX_COMPILER=/usr/bin/c++`） |
| 生成器 | Ninja（`CMAKE_MAKE_PROGRAM=/opt/homebrew/bin/ninja`） |
| 构建类型 | `CMAKE_BUILD_TYPE=Debug`（调试符号已就位，不用改） |
| VSCode 配置 | 无 `.vscode/` 目录，且 `.gitignore` 整目录忽略 |

## 二、概念课：cmake / ninja / 编译器是什么关系（约 30 分钟）

**一句话总纲**：CMake 是元构建系统（生成构建脚本），Ninja 是构建执行器（按依赖图调度），clang++/MSVC 才是真正把 .cpp 变成 .o/.exe 的编译器。三层各干各的：

```
CMakeLists.txt ──(cmake 配置阶段)──▶ build.ninja ──(ninja 调度)──▶ clang++ 一次次编译/链接
     "描述"                             "任务图"                        "干活"
```

讲解时抓这几个点，全部让用户**亲手查证**而不是听结论：

1. `cmake -S . -B build` 与 `cmake --build build` 是两个完全不同的阶段——后者其实只是转手调 ninja（可以让他跑一遍加 `--verbose` 看）。
2. `build/CMakeCache.txt` 里 grep `CMAKE_CXX_COMPILER` / `CMAKE_GENERATOR`，自己找出上表的答案。
3. `build/build.ninja` 打开看几行：ninja 拿到的是展开后的具体命令和依赖边，对它来说没有"CMake"这回事。数学翻译：**ninja 做的就是 DAG 上的拓扑排序 + 增量重算**（哪个输入的 mtime 变了，只重算下游）——"ninja: no work to do" = 图上无脏节点。
4. 生成器可换：同一份 CMakeLists 可生成 Makefile、VS 解决方案、Ninja——这就是"元"的含义。
5. **活素材**：根 `CMakeLists.txt` 第 10 行 `if(MSVC) add_compile_options(/utf-8)`。让用户自己回答两个问题：①为什么这行在 Mac 上不生效也不报错（配置阶段的条件分支，clang 根本没见过这个 flag）；②当初为什么需要它（07-31 的 936 代码页事故，他亲历过）。能答上来，说明"配置阶段/编译阶段"分层真懂了。
6. 顺带收尾：`compile_commands.json` 是干嘛的（clangd 靠它知道每个文件的编译命令——第四个消费者视角）。

## 三、实操：VSCode 调试 GTest 单测（约 40 分钟，本课主菜）

路线：**先手写 launch.json 懂原理，再装 TestMate 享受便利**——顺序别反，反了就只会点按钮。

1. 扩展：macOS 上用 **CodeLLDB**（或 MS C/C++ 扩展的 lldb 模式，二选一，CodeLLDB 配置更省心）。
2. 手写 `.vscode/launch.json`：`program` 指向 `build/tests/test_lu`，`args` 用
   `["--gtest_filter=LUTest.PivotBecomesZeroMidway"]`——**顺便学会 gtest_filter：调试永远只跑一个用例**。
3. 基本功五件套，在真代码上练：断点、单步（step over/into/out）、变量面板、watch 表达式、调用栈。
   练习场就用 `lu_decomposition`：断在循环里，watch 住主元 `A(k,k)`，单步看消元一轮轮进行。
4. 条件断点进阶：右键断点 → 条件 `k == 1`——大循环里等目标时刻，不用按几十次 F10。
5. 装 **C++ TestMate** 扩展：测试列表里每个用例旁一个调试按钮，日常用它；launch.json 留作理解底座和 TestMate 失灵时的后备。
6. **配置入库决策**（引导用户做）：`.gitignore` 目前整目录忽略 `.vscode/`。建议改成负排除（`!.vscode/launch.json`）让配置跨机器同步，launch.json 里同时放两套 configuration（mac/lldb + windows/cppvsdbg，`program` 后缀差个 .exe）。他跨两台机器干活，这个决策有实际收益；但如果他想保持忽略也尊重。

## 四、压轴练习：亲眼看着主元变零（约 30 分钟，可选但强烈推荐）

台账"遗留教训 2"的现场版，也是拖了一周的红测试的了结机会：

1. 用刚学的条件断点跟 `LUTest.PivotBecomesZeroMidway`：在 naked 版 `lu_decomposition_naked` 里 watch `A(k,k)`，**亲眼看到**"检查发生在消元前、主元在消元后才变零、检查扑空"的完整时序——之前只在纸面上推过，这次用调试器验证。
2. 看完顺手把两个红测试按台账方案 A 改成对照用例（`EXPECT_NO_THROW` + 注释写清 naked 版为什么漏检）——测试改动可由你协助（登记）。
3. **改完全仓 43/43 全绿**——红了一周的测试套件清零，单独提交一次。

## 五、交付物清单（回执逐项核）

- [ ] 用户能**独立**（不看笔记）对任意单个 GTest 用例：改 filter → 打断点 → F5 → 单步 + watch
- [ ] `.vscode/launch.json`（入库与否按第三节第 6 条的决策执行）
- [ ] 概念短笔记 `docs/构建工具链与调试.md`，**用户自己写**（100~300 字 + 一张三层关系图即可），内容至少含：三层关系、本机工具链答案、`if(MSVC)` 两问的自答
- [ ] 选做：两个红测试转对照用例，套件 43/43 全绿
- [ ] **每档绿灯/交付物完成即提交**（硬规则，Day 3 立的）

## 六、回执要求（写入 `会话交接/回执_技能课_VSCode调试与构建工具链.md`，简短版即可）

1. 交付物清单逐项勾选；红测试处理了没
2. 掌握度：调试五件套是否独立复现；概念课两道自答题（`if(MSVC)` ①②）答得怎么样
3. 用时 vs 时间盒；有没有挤占 Day 4
4. AI 代写清单（配置/测试若有代写，逐条列）
