# 回执 · M08：结果输出与 CLI

> Task ID：`TS-M08-OUTPUT`
> 执行日期：2026-08-29 起
> 任务单：`会话交接/会话交接_M08_结果输出与CLI.md`
> 启动提交：`578f370 docs: accept M07 and dispatch M08 output task`
> 完成日期：2026-08-30
> 当前状态：第 0～4 档全部通过，全仓 152/152；已达到 M08 任务单 P1 完成定义，等待主会话复验和正式销项。

## 一、第 0 档：输出契约设计闸门

### 1. 基线核对

- 已完整阅读 M08 任务单、M07 回执和项目台账。
- 刷新远端后 `HEAD`、`main`、`origin/main` 均为 `578f370`，ahead/behind 为 `0/0`。
- `578f370` 是基于 `3c0df77` 创建 M08 任务单并更新验收/台账的纯文档提交，未改变生产代码。
- 工作区基线干净，`git diff --check` 通过。
- 重新构建后全仓测试 **124/124** 通过。

### 2. Circuit 元数据所有权

采用 `Circuit` 保存节点名和 V/L 支路名：

```cpp
std::vector<std::string> node_names{"0"};
std::vector<std::string> branch_names;
```

不采用“每个 `Device` 增加名称虚函数”，因为这会把输出职责侵入所有器件，并且仍需额外恢复 V/L 共享 branch 顺序。
不采用“CLI 重新解析网表”，因为它会复制 parser 逻辑、可能与真实编号漂移，也会让输出层依赖原始文件。

锁定不变量：

```text
node_names.size() == circuit.nodes
node_names[0] == "0"
branch_names.size() == circuit.num_branch_unknowns

x.size() == node_names.size() - 1 + branch_names.size()
x[i] <-> V(node_names[i + 1])
x[(nodes - 1) + branch_id] <-> I(branch_names[branch_id])
```

- parser 在首次创建节点编号时同步追加节点名；重复节点只复用编号。
- `0` 与 `gnd` 继续共享节点 0，规范显示名固定为 `0`。
- V/L 按网表出现顺序共享 `branch_names`；R/I/D/C 不进入该序列。
- 名称沿用 tokenizer 的小写规范；M08 不恢复原始大小写，也不新增器件名唯一性规则。
- 保留 `nodes` 和 `num_branch_unknowns` 以兼容现有消费者；writer 负责拒绝公开字段被手工改成的不一致状态。

### 3. ostream writer

采用独立模块：

```text
src/output/result_writer.h
src/output/result_writer.cpp
```

概念接口锁定为：

```cpp
void write_operating_point(
    std::ostream& out,
    const Circuit& circuit,
    const OperatingPointResult& result);

void write_transient_csv(
    std::ostream& out,
    const Circuit& circuit,
    const TransientAnalysisResult& result);

void write_simulation_result(
    std::ostream& out,
    const Circuit& circuit,
    const SimulationResult& result);
```

选择 `std::ostream&`，使同一实现可写终端、文件和测试用字符串流，并避免为完整瞬态轨迹额外构造一个大字符串。
不在 `main.cpp` 直接循环打印，避免格式逻辑复制；`write_simulation_result` 只负责 variant 分派，不调用 parser、controller 或 solver。

writer 在写出任何字符前完整校验元数据和结果维度：结构/尺寸错误抛 `std::invalid_argument`，流进入失败状态时抛
`std::runtime_error`。瞬态结果必须至少包含一个点，且每个点的 `x` 都必须满足同一维度契约。writer 不主动 flush
调用者拥有的流；CLI 写文件后负责 flush、close 并检查最终状态。

### 4. 标签、方向与文本格式

- 非地节点使用 `V(<node-name>)`。
- V/L 支路使用 `I(<device-name>)`。
- 支路电流沿既有 `node_pos -> node_neg` 方向；writer 不取反、不换单位。
- transient CSV 第一列固定为 `time`，后续列严格遵循 `x` 顺序。

`.op` 格式锁定为：

```text
analysis: .op
V(1) = 10
V(2) = 8
I(v1) = -0.002
newton_iterations = <整数>
```

每行和文件末尾均有换行，不添加额外空行。

`.tran` 语义示例：

```text
time,V(in),V(out),I(v1)
0,0,0,0
0.25,2,0.4,-0.8
0.5,2,0.72,-0.64
```

### 5. 数值与 locale

- 浮点采用 `std::defaultfloat` 和 `std::numeric_limits<double>::max_digits10`。
- 输出期间使用 `std::locale::classic()`，固定 `.` 小数点；返回前恢复调用者的 locale、flags 和 precision。
- CSV 固定使用英文逗号且逗号后不加空格。
- 不擅自把 `-0` 归一化为 `0`，不在数值后附加单位。
- M08 不实现复杂 CSV 转义；名称含逗号、双引号、CR 或 LF 时，`write_transient_csv` 明确拒绝输出。
- 表头、标签和固定文字逐字符检查；数值字段解析回 `double` 后与独立手算 oracle 比较，不因
  `0.4` 与 `0.40000000000000002` 的文本差异误判。

### 6. CLI 语法、分层与错误契约

应用调用链锁定为：

```text
main -> run_cli -> 打开网表 -> parse_circuit -> simulate -> writer -> stdout/文件
```

从薄 `main()` 中拆出可测试的 `run_cli(...)`，但它留在 `sandbox/cli/` 应用层，不进入 `SpiceLib` 的
solver/controller。函数级测试负责参数、stdout/stderr 和错误分支；真实进程测试负责 `main`、链接和 CMake 烟囱。

只接受：

```text
TinySpice <netlist-file>
TinySpice <netlist-file> -o <output-file>
```

固定 usage：

```text
usage: TinySpice <netlist-file> [-o <output-file>]
```

固定退出码：

```text
0 = 成功
1 = 文件、parser、solver 或 writer 运行失败
2 = CLI 参数语法错误
```

- 无 `-o` 时结果写 stdout；有 `-o` 时只写文件，不重复输出到 stdout。
- 参数错误写 stderr 并附 usage；输入/输出文件错误包含类别和实际路径。
- parser/solver/writer 的 `std::exception` 只在 CLI 顶层转换为 stderr 和退出码，库层继续传播异常。
- 参数校验、解析和仿真成功后才打开输出文件，避免早期失败提前截断已有文件。

### 7. 测试边界

后续测试仅覆盖 M08 授权范围：

1. parser metadata：默认地节点、首次出现顺序、重复节点、`0/gnd`、小写、L/V 混合顺序和 `.end` 截止。
2. writer：`.op`、RC/RL CSV、独立数值 oracle、locale、维度错误、空瞬态结果、非法 CSV 名称和失败流。
3. CLI runner：参数矩阵、文件失败、非法网表、奇异电路、stdout、`-o`、输出失败和退出码。
4. 真实进程：至少 `.op`、RC `.tran` 两条成功烟囱和一条失败路径；使用可移植 C++/CTest，不以 Bash 为唯一测试手段。
5. AI 编写的测试和机械 CMake 挂载必须同步登记 `docs/AI参与记录.md`。

不在 M08 扩展 `.dc`、多分析冲突、`.ic/UIC`、新波形源、变步长、稀疏化、GUI 或其他下游事项。

## 二、设计 Review 核销

- `D0-01`：名称所有权归 `Circuit`，编号与名称由 parser 同源产生——已核销。
- `D0-02`：地节点不进入 `x`，节点/branch 与向量布局公式明确——已核销。
- `D0-03`：选择可复用 ostream writer，职责与错误边界明确——已核销。
- `D0-04`：标签、方向、`.op` 文本和 `.tran` CSV 表头明确——已核销。
- `D0-05`：精度、locale、CSV 分隔符和数值测试策略明确——已核销。
- `D0-06`：CLI 最小语法、`run_cli` 分层和 `0/1/2` 退出码明确——已核销。
- `D0-07`：函数级测试、真实进程测试和范围边界明确——已核销。

设计选择和代价经逐项讲解后，用户于 2026-08-29 明确回复“同意”，第 0 档设计闸门通过。

## 三、代码与协作边界

- 第 0 档只完成只读核对、设计教学和回执记录，没有修改实现、测试或 CMake。
- 第 1 档 `Circuit`/parser 生产代码由用户亲手编写；AI 只写测试、参与登记和本回执。
- 用户因秋招在即，于 2026-08-30 特殊授权 AI 直接完成 M08 剩余非核心部分；第 2～4 档的 AI 实现范围均已如实登记。
- 本回执只能说明达到任务单完成定义，不能替代主会话复验或宣布主线正式销项。

## 四、第 1 档：Circuit 输出元数据

### 1. 实现结果

- `Circuit` 新增 `node_names{"0"}` 与空的 `branch_names`，保留既有 `nodes` 和
  `num_branch_unknowns` 数字字段。
- parser 首次创建非地节点时同步追加小写名称；重复节点复用原编号和名称。
- `0/gnd` 继续映射到节点 0，元数据中只保留规范名称 `0`。
- parser 成功创建 V/L 后，按共享 branch 编号顺序追加器件名；其他器件不进入 `branch_names`。
- `.end` 继续在进入后续行解析前终止，因此后续器件和节点不会污染元数据。

上述 `Circuit` 字段和 parser 实现均由用户亲手编写。

### 2. 编号 Review

首轮 review 发现并编号拦截 4 个阻塞项：

1. 名称字段误写成单个 `std::string`，且 branch 字段名未采用锁定的复数形式。
2. parser 开始时用空的 `line` 覆盖默认地名。
3. 新节点路径没有把名称追加到 `Circuit`。
4. V/L 路径没有同步追加 branch 名称。

用户第二轮逐项修正后，4 项全部核销，没有出现同一逻辑链“改 A 漏 B”。新节点路径对 `0/gnd` 的额外判断属于
不可达但无害的防御判断，不阻塞本档。

### 3. 测试与 AI 参与

AI 在 `tests/test_parser.cpp` 新增 4 个 `CircuitMetadataTest`，并登记到 `docs/AI参与记录.md`：

| 用例 | 防护目标 |
|------|----------|
| `DefaultCircuitUsesCanonicalGroundAndNoBranches` | 默认地名、空 branch 以及两个 size 不变量 |
| `NodeNamesFollowFirstOccurrenceWithoutDuplicates` | 首次出现顺序、小写、重复节点和 `0/gnd` 去重 |
| `VoltageAndInductorNamesShareNetlistBranchOrder` | L 先于 V 时的共享 branch 顺序，R/C 不污染名称 |
| `EndPreventsMetadataPollution` | `.end` 后器件、节点名和 branch 名均不进入 `Circuit` |

本档不需要新增测试目标或修改 CMake。

### 4. 验证结果

- `cmake --build ./build`：通过。
- `CircuitMetadataTest.*`：**4/4** 通过。
- 全仓：由 124 增至 **128/128**，原回归全部保留。
- `git diff --check`：通过。

截至本档提交时，第 1 档达到任务单绿灯；随后进入第 2 档 writer。

## 五、第 2 档：可复用结果 writer

### 1. 特殊授权与归属

用户因秋招在即，于 2026-08-30 明确授权 AI 直接完成 M08 剩余非核心部分。`result_writer.h` 和最初的
元数据校验骨架由用户亲手编写；AI 在该授权后完成 writer 生产实现、测试、CMake、登记和 Git 收口。

### 2. writer 实现

- 新增 `src/output/result_writer.h/.cpp`，公开 `.op`、`.tran` CSV 和统一 variant 分派三个入口。
- `.op` 输出 `analysis: .op`、全部非地 `V(name)`、全部 V/L `I(name)` 和 Newton 迭代数。
- `.tran` 输出 `time` 加完整解向量标签，每个轨迹点一行。
- 输出前完整校验 `Circuit` 元数据、结果维度和非空轨迹；任何后续点维度错误都在首字符写出前拒绝。
- CSV 名称含逗号、双引号、CR/LF 时明确拒绝，不在 M08 实现复杂转义。
- 使用经典 locale、`defaultfloat` 和 `max_digits10`；通过共享 streambuf 的局部格式流避免污染调用者状态。
- 失败 streambuf 转换为可观察的 `runtime_error`；writer 不主动 flush 调用者拥有的流。
- `write_simulation_result` 只做 variant 分派，不重新求解或解析。

### 3. 测试与验证

- AI 新增 `tests/test_result_writer.cpp` 的 8 个用例，并完成 SpiceLib/测试目标的机械 CMake 挂载。
- `.op` 独立 oracle：`V(1)=10V、V(2)=8V、I(v1)=-2mA`。
- RC CSV 独立 oracle：`t=0/0.25/0.5`，`V(out)=0/0.4/0.72`，`I(v1)=0/-0.8/-0.64`。
- writer 定向测试：**8/8** 通过。
- 全仓：由 128 增至 **136/136**，原回归全部保留。
- `git diff --check`：通过。

截至本档提交时，第 2 档达到任务单绿灯；随后进入第 3 档 CLI。

## 六、第 3 档：CLI 端到端（P0）

### 1. 应用分层

- 新增 `sandbox/cli/run_cli.h/.cpp`，薄 `main()` 只把 `argv[1..]` 转为参数容器并传入 stdout/stderr。
- `TinySpiceCli` 应用层库链接 `SpiceLib`，函数测试可直接调用 runner；solver/controller 不反向依赖 CLI。
- runner 严格调用 `parse_circuit -> simulate -> write_simulation_result`，没有复制任何下层算法。

### 2. 语法、文件与错误契约

仅接受：

```text
TinySpice <netlist-file>
TinySpice <netlist-file> -o <output-file>
```

- 成功返回 0；文件/parser/solver/writer 失败返回 1；参数错误返回 2 并输出固定 usage。
- 无 `-o` 写 stdout；有 `-o` 写文件且 stdout 不重复。
- 输出文件在 parser/simulator 成功后才打开；输入输出路径文字相同时提前拒绝，避免截断网表。
- 文件 writer 完成后显式检查 flush 和 close；所有顶层异常转换为 `error: ...` stderr。

### 3. 测试与验证

- AI 新增 9 个 `CliRunner` 函数级测试：`.op/.tran`、`-o`、参数矩阵、缺文件、parser/solver、同路径、
  输出打开失败和失败 stdout。
- AI 新增 `.op`、RC 测试网表以及 3 条真实进程 CTest：两条成功烟囱和一条缺文件失败烟囱。
- CLI 定向：**12/12** 通过。
- 全仓：由 136 增至 **148/148**，原回归全部保留。
- `git diff --check`：通过。

截至本档提交时，第 0～3 档全部绿灯、M08 P0 达成；随后进入第 4 档 P1 收口。

## 七、第 4 档：示例、Python 波形与收口（P1）

### 1. 输出文件安全

- `-o <output-file>` 已在第 3 档贯通：只写文件、不复制 stdout，并检查 open、flush 和 close。
- parser/simulator 成功后才创建或截断输出文件；早期失败不会破坏既有输出。
- 最终审计把输入/输出保护从“路径文字相同”加强为 `std::filesystem::equivalent`：硬链接、符号链接或
  `input.cir`/`./input.cir` 等别名指向同一文件时均拒绝，原网表保持不变。

### 2. 三份可运行示例

| 文件 | 分析 | 实际验收结果 |
|------|------|--------------|
| `examples/divider_op.cir` | `.op` 分压 | `V(1)=10`、`V(2)=8`、`I(v1)≈-2mA` |
| `examples/rc_transient.cir` | RC `.tran` | `time=0/0.25/0.5`，`V(out)=0/0.4/0.72` |
| `examples/rl_transient.cir` | RL `.tran` | branch 顺序 `[l1,v1]`，`I(l1)=0/0.5/0.75` |

三份文件均由真实 `TinySpice` 进程运行；RC/RL 使用 `-o` 实际生成 CSV 后逐行核对。

### 3. Python CSV/波形工具

- 新增 `scripts/plot_transient.py`，标准库 `csv` 负责读取，固定识别 `time` 为横轴。
- 未指定列时读取全部非 `time` 数值列；可在命令行指定一个或多个标签只画选中波形。
- 缺文件、缺 `time`、缺指定列、重复列、空数据以及非数值/非有限数值均返回 1 并给出具体错误。
- `--validate-only` 不导入 matplotlib，可完成无图形依赖的 CSV 验证；实际绘图支持交互窗口或 `-o` 图片文件。
- 本机 Python 3.9.6 可用但未安装 matplotlib。按任务单没有联网安装；纯解析与语法链路已通过，实际绘图路径已验证
  返回 1 并明确提示安装 matplotlib 或改用 `--validate-only`。

### 4. README 与自动测试

- README 已从单行占位扩充为从构建、`.op`、`.tran -o`、CSV 到 Python 波形的完整最小路径。
- README 明确当前器件/指令/数值后缀、退出码、输出单位、依赖和固定步长/零初值等限制。
- 新增三条 example 真实进程 CTest；新增一条 Python CTest，内部含 6 个 dependency-free 单元场景。
- 第 4 档 CTest：**4/4** 通过；全仓由 148 增至 **152/152**，原 124 个基线回归全部保留。

## 八、提交与测试增长

| 档位 | 提交 | 测试累计 |
|------|------|----------|
| 启动基线 | `578f370` | 124/124 |
| 第 0 档：设计契约 | `bf1bf3d docs: lock M08 output contract` | 124/124 |
| 第 1 档：Circuit 元数据 | `9ff7618 feat: preserve circuit output metadata` | 128/128 |
| 第 2 档：ostream writer | `235c432 feat: add simulation result writers` | 136/136 |
| 第 3 档：CLI/P0 | `9ce390f feat: add TinySpice command line workflow` | 148/148 |
| 第 4 档：examples/Python/README/P1 | 本回执所在提交 | 152/152 |

M08 共新增 28 个 CTest 项：4 个 metadata、8 个 writer、9 个 CLI runner、3 个 CLI 真实进程、3 个 example
真实进程和 1 个 Python 测试入口；Python 入口内部另含 6 个单元场景。

## 九、最终格式与使用契约

```text
Circuit metadata
  node_names[0] = "0"
  x[i] -> V(node_names[i + 1])
  x[(nodes - 1) + branch_id] -> I(branch_names[branch_id])

.op
  analysis: .op
  V(name) = value
  I(name) = value
  newton_iterations = integer

.tran CSV
  time,V(node)...,I(branch)...
```

- 浮点使用 classic locale、`defaultfloat`、`max_digits10`；writer 不改写调用者格式状态。
- 标签和列顺序由 parser 产生的 `Circuit` 元数据与 MNA 布局唯一决定，CLI/Python 不重新推导编号。
- CLI 语法为 `TinySpice <netlist-file> [-o <output-file>]`；退出码固定为 0/1/2。

最小复验命令：

```bash
cmake --build ./build
ctest --test-dir ./build --output-on-failure
./build/sandbox/TinySpice examples/divider_op.cir
./build/sandbox/TinySpice examples/rc_transient.cir -o build/rc.csv
./build/sandbox/TinySpice examples/rl_transient.cir -o build/rl.csv
python3 scripts/plot_transient.py build/rc.csv --validate-only
```

安装 matplotlib 后，可运行：

```bash
python3 scripts/plot_transient.py build/rc.csv 'V(out)' -o build/rc-output.png
```

## 十、AI 参与与“改 A 漏 B”复盘

- 第 1 档核心 `Circuit`/parser 由用户亲写；AI 的 4 条编号 review 被用户逐项修正并一次性核销，同一逻辑链
  “改 A 漏 B”计数为 0。
- 第 2 档起，用户基于秋招时间压力明确授权 AI 完成 M08 剩余非核心生产代码、工具、测试、CMake、文档和 Git；
  具体文件与边界已登记 `docs/AI参与记录.md` 第 20～22 节。
- CLI 首轮定向验证暴露两处测试预期问题：`max_digits10` 合法长尾不应按短文本比较，以及 CTest
  `WILL_FAIL`/正则组合不适合该失败烟囱；均只修正测试方法，没有掩盖产品失败。
- 最终审计主动发现路径别名仍可能指向输入文件，随后同步修改等价判断、扩充原测试和更新文档；未出现只改实现漏测试。
- 全程每档绿灯后立即提交并推送，没有攒批；没有执行 M08 以外的下游代码或预研。

## 十一、停车场（仅供主会话参考，不构成授权）

1. `.dc` 单参数/多参数扫描与 parser 语法。
2. `.op/.tran` 指令冲突和多分析任务列表。
3. `.ic/UIC`、DC operating-point initialization。
4. PULSE/SIN/PWL、变步长、梯形法、断点对齐。
5. ngspice 自动对拍、稀疏 CSR 和性能基准。
6. JSON/二进制输出、复杂 CSV 转义和大轨迹流式求解。
7. GUI、Web、Hazel/ImGui 和交互式波形浏览器。
8. `.dc` + worker 调度器的工业仿真农场演示。

以上内容没有在本会话中选择、检查、预研或启动。

## 十二、任务会话结论

- M08 第 0～4 档均达到各自绿灯，P0/P1 完成定义全部满足。
- 全仓 152/152，三份 example、stdout、`-o`、CSV 和 dependency-free Python 解析链路均已实跑。
- 因本机缺 matplotlib，图片渲染按任务单允许的依赖分支记录为未在本机执行；缺依赖错误已实测。
- 本结论是“达到 M08 任务单完成定义”，不代表主线正式销项；应由 `TinySpice｜MAIN｜主线管理` 复验。

## 十三、交回主会话的功能与文件清单

以下清单用于 `TinySpice｜MAIN｜主线管理` 复验 M08，覆盖从启动提交 `578f370` 到实现收口提交
`5d4ba32` 的全部功能文件，并明确生产代码归属。

| 功能模块 | 已完成功能 | 涉及文件 | 编写归属 |
|----------|------------|----------|----------|
| Circuit 输出元数据 | 保存规范地名、节点首次出现顺序、V/L 共享 branch 顺序，并与 MNA `x` 布局对齐 | `src/parser/Circuit.h`、`src/parser/Circuit.cpp` | 核心生产代码由用户亲写；AI 教学、编号 review 和测试 |
| ostream 结果 writer | `.op` 标签文本、`.tran` CSV、variant 分派、维度/locale/精度/失败流校验 | `src/output/result_writer.h`、`src/output/result_writer.cpp`、`src/CMakeLists.txt` | 用户写接口及最初校验骨架；AI 在特殊授权后完成其余实现与挂载 |
| CLI 应用层 | `TinySpice <file> [-o <file>]`、parser → controller → writer 编排、stdout/文件选择、0/1/2 退出码、stderr 错误、同文件保护 | `sandbox/cli/run_cli.h`、`sandbox/cli/run_cli.cpp`、`sandbox/cli/main.cpp`、`sandbox/CMakeLists.txt` | AI 在特殊授权下完成 |
| 可运行示例 | `.op` 分压、RC `.tran`、RL `.tran` | `examples/divider_op.cir`、`examples/rc_transient.cir`、`examples/rl_transient.cir` | AI 在特殊授权下完成 |
| Python 波形链路 | 读取 CSV、识别 `time`、选择一列/多列/全部列、保存或交互绘图、dependency-free 验证、明确错误 | `scripts/plot_transient.py` | AI 在特殊授权下完成 |
| 新用户文档 | 构建、运行、`-o` 导出、Python 画图、依赖、支持语法和限制 | `README.md` | AI 在特殊授权下完成 |

### 1. AI 完成的测试与工程文件

- metadata：`tests/test_parser.cpp` 中新增 4 个 `CircuitMetadataTest`。
- writer：`tests/test_result_writer.cpp`，共 8 个 CTest 项。
- CLI：`tests/test_cli.cpp`、`tests/fixtures/cli_divider.cir`、`tests/fixtures/cli_rc.cir`、
  `tests/check_cli_failure.cmake`，共 9 个 runner 用例和 3 个真实进程测试。
- examples/Python：`tests/test_plot_transient.py`，以及 `tests/CMakeLists.txt` 中 3 个 example 真实进程测试和
  1 个 Python 测试入口；Python 入口内部有 6 个单元场景。
- CMake：`src/CMakeLists.txt`、`sandbox/CMakeLists.txt`、`tests/CMakeLists.txt` 中仅完成相应目标和测试的机械挂载。

### 2. AI 完成的记录与 Git 工作

- `docs/AI参与记录.md`：第 19～22 节记录 metadata 测试、writer、CLI、examples/Python 的参与边界。
- `会话交接/回执_M08_结果输出与CLI.md`：设计决策、逐档验收、提交、测试增长、依赖状态和本清单。
- M08 的 status/diff/构建/测试、逐档 commit、push、main/origin 同步和最终干净工作区均由 AI 执行。

### 3. 主会话建议复验入口

```bash
git diff --name-status 578f370..5d4ba32
cmake --build ./build
ctest --test-dir ./build --output-on-failure
./build/sandbox/TinySpice examples/divider_op.cir
./build/sandbox/TinySpice examples/rc_transient.cir -o build/rc.csv
./build/sandbox/TinySpice examples/rl_transient.cir -o build/rl.csv
python3 scripts/plot_transient.py build/rc.csv --validate-only
```

实现范围之外没有隐藏改动；M08 停车场和下游任务均未启动。

## 十四、主会话正式验收（2026-08-30）

主会话以 `4d11881 docs: add M08 implementation handoff inventory` 为验收基线，独立复验并确认：

- `git fetch origin --prune` 成功；`main` 与最新 `origin/main` 均指向 `4d11881`，ahead/behind 为 `0/0`。
- 工作区干净，`git diff --check` 通过。
- `cmake --build ./build` 构建通过，全仓 **152/152** 全绿。
- `.op` 分压示例实跑得到 `V(1)=10`、`V(2)=8`、`I(v1)≈-2mA`。
- RC/RL 示例均通过真实 CLI 写出 CSV；时间、节点列和 L/V branch 顺序与回执一致。
- `python3 scripts/plot_transient.py <csv> --validate-only` 实跑成功。
- 当前机器可用 Python 3.9.6，但没有 matplotlib，也没有命令行可见的 MATLAB/Octave；图片渲染未实跑属于任务单已允许并如实记录的依赖分支。
- 从 `578f370` 到 `4d11881` 的功能、测试、文档和 AI/用户归属清单与仓库差异一致。

主会话结论：**`TS-M08-OUTPUT` 正式验收通过并销项。**CSV 是与绘图语言无关的稳定波形数据接口；最终以 Python、MATLAB 或双轨作为官方绘图消费者，留待用户完成选型后由主会话安排，不反向影响 M08 的 parser/controller/CLI/CSV 验收。
