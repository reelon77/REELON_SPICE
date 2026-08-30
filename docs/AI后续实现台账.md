# TinySpice AI 后续实现台账

> 建立日期：2026-08-30
> 起算基线：`4d11881 docs: add M08 implementation handoff inventory`
> 状态：启用；TinySpice 1.0 AI 实现阶段启动，当前任务 `TS-M09-DC`

## 一、用途

用户因秋招临近，于 2026-08-30 明确授权：TinySpice 核心算法完成后的剩余内容可以由 AI 直接设计和实现。

本文件从该授权起持续记录：

1. 哪些模块由 AI 设计；
2. AI 作出的关键设计决策及理由；
3. AI 新建或修改了哪些生产文件；
4. AI 编写了哪些测试、工程配置和文档；
5. 每项工作的验证结果、提交和用户复核状态。

本台账不改变历史归属。`Matrix/LU/MNA`、器件模型、Newton、瞬态求解、parser 主体和 M07 controller 等此前由用户亲手完成的核心代码，仍归用户实现；不得因后续授权改写为 AI 作品。

## 二、与原 AI 记录的关系

- `docs/AI参与记录.md`：保留从项目早期开始的逐测试、逐阶段历史记录，是完整历史账本。
- `docs/AI后续实现台账.md`：从 2026-08-30 起记录 AI 获得生产实现授权后的模块级设计与文件归属，是后续工作的主索引。
- 两者出现交叉时，以更细粒度的原始阶段记录和 Git diff 为证据，不重复夸大 AI 或用户贡献。

## 三、强制登记规则

每个 AI 实现阶段必须在同一次阶段提交中更新本文件，至少登记：

```text
记录 ID / 日期 / Task ID
目标与范围
AI 设计决策
AI 生产代码文件
AI 测试、CMake、脚本和文档文件
明确未修改的用户核心模块
验证命令与结果
提交 hash
用户需要理解或复核的内容
```

规则：

1. 文件同时含用户和 AI 修改时，必须说明到函数、区域或阶段，不能只按整文件粗略归属。
2. AI 修复自己引入的问题必须记录，不能只登记最终正确版本。
3. 机械挂载、格式化、测试、文档和生产逻辑分开列出。
4. 未经验证的功能标为“未验证”，不能写“完成”。
5. 新模块的回执必须引用本台账对应记录 ID。
6. 所有 Git 操作继续由 AI 执行；每档提交前核对 status、diff、定向测试、全仓测试和范围。

## 四、起算点之前但与当前交付直接相关的 AI 实现

### `AI-BASE-M08`：M08 输出、CLI 与波形工具

> Task ID：`TS-M08-OUTPUT`
> 实现提交：`235c432`、`9ce390f`、`5d4ba32`
> 归属依据：`docs/AI参与记录.md` 第 20～22 节、`会话交接/回执_M08_结果输出与CLI.md`

AI 设计/实现范围：

- 完成 `src/output/result_writer.cpp` 的大部分 writer 生产实现；用户此前完成公开接口和最初校验骨架。
- 完成 CLI runner、薄 main、stdout/文件输出、退出码和同文件保护。
- 完成三份 example、Python CSV/波形工具和 README。
- 完成相关测试、CMake 挂载、验证、Git 和回执记录。

涉及生产/工具文件：

- `src/output/result_writer.h`
- `src/output/result_writer.cpp`
- `sandbox/cli/run_cli.h`
- `sandbox/cli/run_cli.cpp`
- `sandbox/cli/main.cpp`
- `examples/divider_op.cir`
- `examples/rc_transient.cir`
- `examples/rl_transient.cir`
- `scripts/plot_transient.py`
- `README.md`

涉及测试/工程文件：

- `tests/test_result_writer.cpp`
- `tests/test_cli.cpp`
- `tests/test_plot_transient.py`
- `tests/check_cli_failure.cmake`
- `tests/fixtures/cli_divider.cir`
- `tests/fixtures/cli_rc.cir`
- `src/CMakeLists.txt`
- `sandbox/CMakeLists.txt`
- `tests/CMakeLists.txt`

明确保留的用户归属：`Circuit` 元数据与 parser 同步实现由用户亲写；M07 及其之前的核心算法归属不变。

验证：M08 主会话复验全仓 152/152；三份 example、CLI CSV 和 Python `--validate-only` 通过。MATLAB 在后续 `AI-0002` 中完成 R2026a 实机验收。

## 五、授权后记录

### `AI-0001`：建立后续 AI 生产实现的归属与审计规则

> 日期：2026-08-30
> 类型：主会话管理与文档

- 新建本台账，锁定后续 AI 设计、文件、测试、提交和复核的登记格式。
- 在 M08 回执写入主会话独立验收结果。
- 更新项目台账：M08 正式销项，下一任务等待波形绘图路线选择。
- 未修改任何 `src/`、CLI、脚本或测试实现。

主要内容提交：`3e15882 docs: accept M08 and establish AI implementation ledger`；本行由随后纯文档提交回填。

### `AI-0002`：MATLAB 瞬态波形伴侣

> 日期：2026-08-30
> Task ID：`TS-M08-WAVEFORM-COMPANION`
> 类型：M08 后续工具增强

- 目标与范围：保持 M08 CSV 契约及 Python 工具不变，新增 MATLAB 交互绘图入口；不扩展 `.dc`、求解器或 `src/`。
- AI 设计决策：CSV 继续作为唯一稳定交换格式；Python 负责自动验证、CI 友好和跨平台入口，MATLAB 负责学校环境下的交互分析。两个消费者互不依赖。
- AI 生产/工具代码：新建 `scripts/plot_transient.m`，支持全列/选列、交互显示/文件导出，并校验文件、`time`、列名、空数据、实数数值和有限值。
- AI 文档：修改 `README.md`、本台账及 `项目梳理与开发计划.md`，锁定双路线职责与调用方式。
- AI 测试/CMake：未新增或修改；MATLAB 是可选外部消费者，不强制挂入 CTest。
- 明确未修改：全部 `src/`、CLI、CSV writer、Python 工具及既有测试。
- 验证：重新构建成功；全仓 152/152；真实 RC CSV 经 Python `--validate-only` 验证成功（3 行，`time/V(in)/V(out)/I(v1)`）。随后在 MATLAB R2026a Update 5 实跑全部列、指定列、多列 PNG 导出和缺失列错误路径，均通过；导出图片为 1274×820，目视确认曲线、坐标轴与图例正常。
- 提交：`19c3751 feat: add MATLAB transient waveform companion`；本行由随后纯文档提交回填。
- 用户复核点：可按 README 示例自行调整列选择和图片样式；功能与错误路径已完成实机验收。

### `AI-0003`：冻结 TinySpice 1.0 范围并下发 M09

> 日期：2026-08-30
> Task ID：`TS-1.0-ROADMAP` / `TS-M09-DC`
> 类型：主会话架构规划与任务下发

- 用户确认采用“TinySpice 1.0 实用 SPICE 子集”，不采用 ngspice 后端冒充自主实现。
- AI 新建 `docs/TinySpice_1.0_规格与路线图.md`，明确分析、器件、网表、数值能力、非目标、9 个任务依赖和 `v1.0.0` 发布闸门。
- AI 新建 `会话交接/会话交接_M09_直流扫描与源抽象.md`，把类型化分析请求、器件身份、非破坏式源覆盖、单/双源 `.dc`、writer/CLI 和测试拆为第 0～4 档。
- AI 更新 `项目梳理与开发计划.md` 和本台账，将当前活动任务切换为 `TS-M09-DC`。
- 本记录阶段不修改任何 `src/`、测试、CMake、CLI 或 example；生产实现只能在正式 M09 任务会话开始。
- 验证：文档范围与会话边界检查通过；重新构建成功，全仓 152/152；变更仅含规格、任务单和项目管理文档。
- 提交：`ecbe794 docs: define TinySpice 1.0 and dispatch M09`；本行由随后纯文档提交回填。

### `AI-0004`：M09 直流扫描与源抽象

> 日期：2026-08-30
> Task ID：`TS-M09-DC`
> 状态：第 0～1 档已完成；第 2～4 档待执行

- 目标与范围：在不修改 `const Circuit` 内器件的前提下，完成类型化 `.op/.tran/.dc` 请求、全局唯一器件身份、独立 V/I 源局部克隆覆盖、单/双源扫描、结构化结果及 CSV/CLI。
- 第 0 档 AI 设计：选择具名 struct + `std::variant` 作为唯一分析 IR；`device_names` 与 `devices` 一一对应；`.dc` 前向引用在解析结束后绑定到 `IndependentSource`；每点仅克隆被扫源并替换非拥有视图；整数步号生成扫描序列；双源为 source2 外层。
- 第 0 档 AI 文档：新建 `会话交接/回执_M09_直流扫描与源抽象.md`，记录接口草图、被拒方案、不变量、错误和结果契约；本档不修改生产代码、测试或 CMake。
- 明确保留的用户历史归属：现有 parser、V/I 源 stamp、Newton、transient、MNA/LU 与 M07 controller 原始实现归属不变；M09 后续对其修改将精确登记函数/区域。
- 第 0 档验证：`main == origin/main == fb6bba8`，工作区干净；先构建再测试，全仓 152/152。
- 第 0 档提交：`731f607 docs: lock M09 dc sweep design`。
- 第 1 档 AI 生产代码：新建 `src/devices/IndependentSource.h`；修改 `src/parser/Circuit.h` 的分析 IR/`device_names`，`src/parser/Circuit.cpp::parse_circuit` 的单一分析赋值和同步器件登记，`VoltageSource`/`CurrentSource` 的 `dc_value/clone_with_dc_value`，以及 `src/sim/simulate.cpp::simulate` 的 variant 分派和身份防御检查。未修改 V/I stamp 公式、Newton、transient、MNA 或 LU。
- 第 1 档 AI 测试/工程：修改 `tests/test_parser.cpp`、`tests/test_end_to_end.cpp`、`tests/test_simulate.cpp`、`tests/test_sources.cpp`；修改 `src/CMakeLists.txt` 挂载新头文件。覆盖分析冲突、名称映射/重名、V/I 克隆不污染原对象、旧 `.op/.tran` 结果。
- 第 1 档验证：parser 38/38、source 8/8、controller 9/9、end-to-end 5/5；先构建再全仓测试 158/158。
- 第 1 档提交：待本档提交后在 M09 最终收口回填精确 hash。

## 六、已确认的后续工具路线

2026-08-30 用户确认采用双路线：

- `scripts/plot_transient.py` 是仓库的自动校验、CI 友好和跨平台绘图入口；
- `scripts/plot_transient.m` 是 MATLAB 环境中的交互分析伴侣；
- 二者只消费同一份 TinySpice CSV，不改变仿真内核、CLI 或 CSV 契约。
