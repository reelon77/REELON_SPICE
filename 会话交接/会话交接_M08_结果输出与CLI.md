# 会话交接 · M08：结果输出与 CLI

> 下发日期：2026-08-29
> Task ID：`TS-M08-OUTPUT`
> 建议会话名称：`TinySpice｜M08-OUTPUT｜结果输出与CLI`
> 代码基线：`3c0df77 docs: enforce main-session task boundary`
> 上游回执：`会话交接/回执_M07_仿真控制器.md`
> 目标回执：`会话交接/回执_M08_结果输出与CLI.md`
> 当前基线：M07 已由主会话正式验收，工作区干净，controller 8/8、全仓 **124/124**。

## 零、授权前置条件与会话边界

本文件是 `TS-M08-OUTPUT` 的唯一授权。开始前必须同时确认：

1. `项目梳理与开发计划.md` 的“当前活动任务”为 `TS-M08-OUTPUT`；
2. 当前会话名称为 `TinySpice｜M08-OUTPUT｜结果输出与CLI`；
3. 已完整阅读本任务单和上游 M07 回执；
4. Git 基线与任务单记录一致，若不一致先停止并向主会话报告。

本任务会话只能执行 M08。即使本文停车场已写出下游方向，也**不得**：

- 自行选择、拆分、预研或启动 M09 或其他模块；
- 创建下一份 `会话交接_<任务>.md`；
- 把主台账“当前活动任务”改成下游任务；
- 在 M08 回执完成后继续只读检查下游接口；
- 把停车场建议解释为编码授权。

M08 完成后的唯一动作是：写好本任务回执，完成本任务 Git 收口，然后明确告诉用户“请把回执交回 `TinySpice｜MAIN｜主线管理`”，随即停止。正式销项、排期和下一任务下发只属于主会话。

## 一、任务目标

把当前仍停留在库调用层的链路：

```text
Circuit -> simulate() -> SimulationResult
```

变成用户可直接运行和消费的最小产品链路：

```text
网表文件
  -> CLI 打开文件
  -> parse_circuit()
  -> simulate()
  -> .op 人类可读结果 / .tran CSV
  -> 可选输出文件
  -> Python 读取 CSV 画波形
```

M08 不新增求解算法。它解决三个应用层问题：

1. 结果向量中每个分量叫什么；
2. 如何以稳定、可测试的格式写到 `std::ostream`；
3. CLI 如何把文件、parser、controller 和输出层串起来。

## 二、任务范围

### 必须完成

1. 为 `Circuit` 保存与内部编号严格对齐的节点名和 V/L 支路名元数据。
2. 新建可复用的结果输出模块；不把格式化逻辑塞进 solver 或 `main.cpp`。
3. `.op` 输出非地节点电压、V/L 支路电流和 Newton 迭代数。
4. `.tran` 输出带稳定表头的 CSV；第一列为时间，其余列顺序与解向量一致。
5. CLI 从文件读取网表，调用 parser 与 `simulate()`，把结果写到 stdout 或用户指定文件。
6. CLI 对参数错误、文件打开失败、parser/solver/output 异常给出 stderr 信息和非零退出码。
7. 至少提供 `.op`、RC `.tran`、RL `.tran` 示例网表。
8. 提供一个最小 Python 脚本读取 transient CSV 并绘制选定列或全部波形。
9. parser metadata、formatter、CLI 和端到端示例都有测试；原 124 个回归全部保留。

### 明确不做

- `.dc` 指令、扫描参数和批量仿真。
- `.op/.tran` 多指令冲突语义重构。
- `.ic/UIC`、自动 DC 初值、续跑。
- PULSE/SIN/PWL、梯形法、变步长和失败重试。
- 稀疏 CSR、并行扫描、ngspice 自动对拍。
- GUI、Hazel/ImGui、Web 前端。
- 通用日志框架、配置文件系统或第三方 CLI 参数库。
- 为输出方便而复制 parser、controller 或 solver 逻辑。

## 三、协作与提交纪律

1. **核心 `src/` 和生产逻辑由用户亲手编写。**包括 Circuit 元数据、输出 API/实现和 CLI 主流程。
2. AI 负责概念教学、接口比较、编号 review、测试、经任务授权的机械 CMake 挂载、验收与协调。
3. Python 绘图脚本和示例文件默认也由用户编写；若要 AI 代写，必须由用户在当次明确授权并登记。
4. AI 写入的测试/CMake 必须同步登记 `docs/AI参与记录.md`，逐项说明防护目标。
5. 所有 Git 操作由 AI 执行；每次先核对 `status`、`diff`、定向测试、全仓回归和提交范围。
6. 每档绿灯立即提交并推送，不攒批；未核销完编号 review 不进入下一档。
7. 每个数值输出用例必须有独立手算值；不得只与同一实现生成的字符串对拍。
8. 非阻塞问题只写入本任务停车场，不临时扩科。

## 四、当前接口与缺口

当前已有：

```text
Circuit
  devices
  nodes
  num_branch_unknowns
  analysis_type
  t_step / t_stop

simulate(const Circuit&)
  -> variant<OperatingPointResult, TransientAnalysisResult>
```

当前缺口：

- parser 的 `nodes_map` 是局部变量，解析结束后节点名称丢失；
- V/L 共享支路编号，但 `Circuit` 没保存对应器件名；
- `SimulationResult` 只有数值，不知道 `x[i]` 应显示成 `V(out)` 还是 `I(v1)`；
- `sandbox/cli/main.cpp` 仍只返回 0；
- 尚无 examples、CSV writer 或波形脚本。

因此 M08 的正确方向是补齐元数据和输出层，不是在 CLI 中重新解析原始网表来猜标签。

## 五、第 0 档：输出契约设计闸门（只读，不写实现）

### 1. 元数据所有权方案比较

必须比较：

1. `Circuit` 保存 `node_names` 与 `branch_names`；
2. 每个 `Device` 增加虚函数返回名称；
3. CLI 重新读取/解析网表以恢复名称。

当前推荐方案 1：编号由 parser 分配，名称映射也应由 parser 产出并成为 Circuit IR 的一部分。方案 2 会侵入所有器件，方案 3 会复制解析逻辑并可能与真实编号顺序漂移。

设计必须明确以下不变量：

```text
node_names.size() == circuit.nodes
node_names[0] == "0"                  // gnd 的规范显示名
branch_names.size() == num_branch_unknowns

x[0 .. nodes-2]              <-> node_names[1 .. nodes-1]
x[nodes-1 .. end]            <-> branch_names[0 .. end]
```

V/L 的 `branch_names` 必须按网表出现顺序共享同一序列，与 M06 的 branch 编号完全一致。名称大小写遵循当前 tokenizer 的小写规范，不在本任务恢复原始大小写。

### 2. 输出 API 方案比较

必须比较：

- formatter 直接返回一个大 `std::string`；
- formatter 接收 `std::ostream&`；
- 在 `main.cpp` 中直接循环打印。

当前推荐 `std::ostream&`：同一实现可写 `std::cout`、`std::ofstream` 和测试用 `std::ostringstream`，避免构造整份瞬态文本，也避免把格式逻辑复制进 CLI。

概念接口可分为：

```text
write_operating_point(out, circuit, op_result)
write_transient_csv(out, circuit, transient_result)
write_simulation_result(out, circuit, variant_result)
```

字段名可调整，但必须让 `.op` 与 `.tran` 的格式责任清晰、可独立测试。

### 3. 标签与方向约定

- 非地节点：`V(<node-name>)`；不输出独立的地节点变量，因为 x 中没有地。
- 支路未知量：`I(<v-or-l-device-name>)`。
- 电压源/电感电流正方向沿用对应器件已记录的 pos -> neg 约定；输出层不重新改符号。
- transient CSV 第一列固定为 `time`。

### 4. CLI 最小语法

设计闸门至少锁定：

```text
TinySpice <netlist-file>
TinySpice <netlist-file> -o <output-file>
```

- 无 `-o`：`.op` 人类可读文本写 stdout，`.tran` CSV 写 stdout。
- 有 `-o`：相同内容写指定文件；正常 stdout 不重复输出整份结果。
- usage、错误输出和退出码必须稳定，不能所有路径都返回 0。

是否把可测试的 `run_cli(...)` 从薄 `main()` 中拆出，由用户比较“进程级测试”与“函数级测试”后决定；不得为了测试把 argv 解析塞进 SpiceLib 的 solver/controller 层。

### 5. 数值格式

设计闸门必须决定并记录：

- 浮点精度；
- CSV 分隔符；
- locale 是否固定；
- 测试是按数值解析还是脆弱地逐字符比较。

推荐 CSV 使用逗号、`.` 小数点和足够稳定的有效数字；表头逐字符检查，数值列解析后与独立 oracle 比较。不要把 `0.4` 是否显示成 `0.40000000000000002` 当成数值正确性。

绿灯：用户能复述上述选择与代价；公开数据结构、writer 职责、CLI 语法和错误契约草图通过编号 review。在此之前不得修改实现。

## 六、第 1 档：Circuit 输出元数据

1. 为 `Circuit` 添加节点名和 branch 名序列，保持现有数字字段与消费者兼容。
2. parser 创建新节点时同步追加名称；重复节点只复用旧编号，不重复追加。
3. `0` 与 `gnd` 继续映射到同一地节点，规范显示名固定为 `0`。
4. 解析 V/L 时按它们共享的 branch 编号顺序记录器件名；R/I/D/C 不进入 branch 名序列。
5. `.end` 后内容不得污染名称元数据。

### 测试要求

- `node_names.size()==nodes`，下标和首次出现顺序一致；
- `0/gnd` 只有一个地条目；
- 重复节点名称不新增条目；
- L 先于 V 时 `branch_names == [l1, v1]`；
- 原 parser、controller 和 M06 全部回归保持绿色。

绿灯：metadata 定向测试全绿 + 全仓回归全绿 + 编号 review 清零；AI 完成 Git 提交并推送。

## 七、第 2 档：可复用结果 writer

1. 新增独立输出模块（建议 `src/output/`，最终命名由第 0 档锁定）。
2. writer 只消费 `Circuit` 元数据与 `SimulationResult`，不重新求解、不修改 Circuit。
3. `.op` 至少输出：分析类型、每个 `V(name)`、每个 `I(name)`、Newton iterations。
4. `.tran` CSV 表头为 `time` + 所有解分量标签；每个轨迹点一行。
5. 输出前校验元数据数量与结果维度，禁止越界、静默截断或多余列。
6. 写流失败必须保持可观察；不能返回“成功”但得到半份文件。

### 独立数值闸门

`.op` 分压：

```text
V(1) = 10
V(2) = 8
I(v1) = -0.002
```

RC `.tran` CSV 的标签和前两步必须对应：

```text
time,V(in),V(out),I(v1)
0,0,0,0
0.25,2,0.4,-0.8
0.5,2,0.72,-0.64
```

具体浮点文本服从第 0 档精度规则；测试应解析数值而不是机械复制上述字面量。

绿灯：writer 的 `.op/.tran`、维度错误和流错误测试全绿；全仓回归全绿；编号 review 清零；AI 提交并推送。

## 八、第 3 档：CLI 端到端（P0）

1. `main` 或可测试 CLI runner 解析最小参数。
2. 用 `std::ifstream` 打开网表；失败时 stderr 给出路径和原因，返回非零。
3. 依次调用 `parse_circuit`、`simulate` 和 writer，不复制任何一层逻辑。
4. 根据 `SimulationResult` 自动输出 `.op` 文本或 `.tran` CSV。
5. 捕获顶层 `std::exception` 只为转换成 CLI 错误信息/退出码；不得在库层吞异常。
6. 无参数、多余参数、`-o` 缺目标、网表解析失败和求解失败均有稳定行为。

### P0 端到端闸门

- 对 `.op` 分压示例运行可看到 `10V / 8V / -2mA` 对应标签。
- 对 RC 示例运行可得到含 `t=0,0.25,0.5` 的 CSV。
- 不存在文件、非法网表、奇异电路返回非零且错误写 stderr。
- CLI 成功路径返回 0。

优先使用可移植 C++/CTest；不要依赖 Bash 专有语法作为唯一测试手段，Windows 构建仍需可维护。

绿灯：CLI 定向测试、两个真实进程烟囱和全仓回归全绿；编号 review 清零；AI 提交并推送。此时 P0 完成，但 M08 尚未完整销项。

## 九、第 4 档：输出文件、示例、Python 波形与收口（P1）

1. 实现 `-o <output-file>`；文件创建/写入失败返回非零且不伪报成功。
2. 添加最小 examples：
   - `.op` 分压；
   - RC `.tran`；
   - RL `.tran`。
3. 添加最小 Python 绘图脚本：
   - 读取 CSV；
   - 识别 `time`；
   - 可绘制指定列或全部数值列；
   - 对缺文件、无 time 列、非数值列给出明确错误。
4. 脚本依赖和命令写入 README；若本机缺 matplotlib，不擅自联网安装，先报告并至少完成语法/纯解析层验证。
5. README 写清构建、运行 `.op`、导出 `.tran` CSV、画图、当前支持语法和明确限制。
6. 写回 M08 回执，报告每档提交、测试增长、格式契约、示例命令、AI 参与、“改 A 漏 B”与停车场。

### 最终验证

```bash
cmake --build ./build
ctest --test-dir ./build --output-on-failure
git diff --check
git status --short
```

另外实际运行三份 example，检查 `.op`、RC CSV、RL CSV；Python 脚本按本机依赖条件完成可执行验证或明确记录缺失依赖。

绿灯：P1 全部完成、全仓绿色、AI 参与登记完整、回执和本任务 Git 收口完成。随后任务会话必须停止并把回执交回主会话。

## 十、完成定义

### P0：命令行最小产品链路

- 第 0～3 档绿灯。
- `TinySpice <file>` 可执行 `.op/.tran`。
- 输出有稳定标签，不再暴露无语义的裸 `x[i]`。
- CLI 成功/失败退出码可测试。

### P1：M08 完整销项候选

- 第 4 档绿灯。
- stdout 与 `-o` 文件输出均可用。
- 示例网表、CSV 和 Python 绘图链路具备。
- README 足以让新用户从构建走到波形。
- 全仓测试全绿，回执完整。

注意：任务会话只能报告“达到 M08 任务单完成定义”，**不能自行宣布主线正式销项**。正式销项需主会话复验。

## 十一、停车场（不构成授权）

1. `.dc` 单参数/多参数扫描与 parser 语法。
2. `.op/.tran` 指令冲突和多分析任务列表。
3. `.ic/UIC`、DC operating-point initialization。
4. PULSE/SIN/PWL、变步长、梯形法、断点对齐。
5. ngspice 自动对拍、稀疏 CSR 和性能基准。
6. JSON/二进制输出、复杂 CSV 转义和大轨迹流式求解。
7. GUI、Web、Hazel/ImGui 和交互式波形浏览器。
8. `.dc` + worker 调度器的工业仿真农场演示。

停车场只允许写入 M08 回执供主会话参考；M08 会话不得检查或启动这些事项。

## 十二、主会话验收要求

1. 核对元数据与 MNA 向量布局严格一致，尤其是 L/V 混合 branch 顺序。
2. 核对 writer 不依赖 solver 内部实现，也没有重复 parser。
3. 核对 CLI 只做应用编排，顶层异常转换不掩盖失败。
4. 核对 CSV 表头、数值精度、locale 和维度错误契约。
5. 实际运行三份 example 和至少一条错误路径。
6. 核对 AI 只写了已授权测试/工程内容，生产代码归属如实登记。
7. 核对任务会话完成后立即停止，没有再发生下游越级预研。

## 十三、新会话开场提示词

```text
会话名称：TinySpice｜M08-OUTPUT｜结果输出与CLI

请先完整阅读并核对：
1. 会话交接/会话交接_M08_结果输出与CLI.md
2. 会话交接/回执_M07_仿真控制器.md
3. 项目梳理与开发计划.md

当前任务只能执行 TS-M08-OUTPUT。先核对 Git 基线、main/origin 同步状态、工作区和 124/124 基线。
严格遵守：核心 src/ 与生产逻辑由我亲手写；AI 负责教学、编号 review、测试、机械 CMake 挂载、验收和所有 Git 操作；每档绿灯立即提交并推送。

本轮只执行任务单第 0 档：比较并锁定 Circuit 元数据所有权、ostream writer、标签/数值格式、CLI 语法和测试边界。在我通过设计闸门前不要修改实现。

完成 M08 回执和本任务 Git 收口后必须停止，只提示我把回执交回 TinySpice｜MAIN｜主线管理；不得选择、预研或启动任何下游任务。
```
