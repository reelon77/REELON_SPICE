# 回执 · M08：结果输出与 CLI（进行中）

> Task ID：`TS-M08-OUTPUT`
> 执行日期：2026-08-29 起
> 任务单：`会话交接/会话交接_M08_结果输出与CLI.md`
> 启动提交：`578f370 docs: accept M07 and dispatch M08 output task`
> 当前状态：第 0 档设计闸门已通过；第 1～4 档尚未开始，M08 尚未达到完成定义。

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

- 本档只完成只读核对、设计教学和回执记录。
- 未修改 `src/`、CLI 实现、测试、CMake、示例或脚本。
- 第 1～4 档尚未开始；不得把本阶段回执解释为 M08 完成或主线正式销项。
