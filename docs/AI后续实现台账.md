# TinySpice AI 后续实现台账

> 建立日期：2026-08-30
> 起算基线：`4d11881 docs: add M08 implementation handoff inventory`
> 状态：启用；项目已按用户要求暂停，当前无活动任务，M10 尚未下发

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
> 状态：第 0～4 档已完成；主会话已正式验收销项

- 目标与范围：在不修改 `const Circuit` 内器件的前提下，完成类型化 `.op/.tran/.dc` 请求、全局唯一器件身份、独立 V/I 源局部克隆覆盖、单/双源扫描、结构化结果及 CSV/CLI。
- 第 0 档 AI 设计：选择具名 struct + `std::variant` 作为唯一分析 IR；`device_names` 与 `devices` 一一对应；`.dc` 前向引用在解析结束后绑定到 `IndependentSource`；每点仅克隆被扫源并替换非拥有视图；整数步号生成扫描序列；双源为 source2 外层。
- 第 0 档 AI 文档：新建 `会话交接/回执_M09_直流扫描与源抽象.md`，记录接口草图、被拒方案、不变量、错误和结果契约；本档不修改生产代码、测试或 CMake。
- 明确保留的用户历史归属：现有 parser、V/I 源 stamp、Newton、transient、MNA/LU 与 M07 controller 原始实现归属不变；M09 后续对其修改将精确登记函数/区域。
- 第 0 档验证：`main == origin/main == fb6bba8`，工作区干净；先构建再测试，全仓 152/152。
- 第 0 档提交：`731f607 docs: lock M09 dc sweep design`。
- 第 1 档 AI 生产代码：新建 `src/devices/IndependentSource.h`；修改 `src/parser/Circuit.h` 的分析 IR/`device_names`，`src/parser/Circuit.cpp::parse_circuit` 的单一分析赋值和同步器件登记，`VoltageSource`/`CurrentSource` 的 `dc_value/clone_with_dc_value`，以及 `src/sim/simulate.cpp::simulate` 的 variant 分派和身份防御检查。未修改 V/I stamp 公式、Newton、transient、MNA 或 LU。
- 第 1 档 AI 测试/工程：修改 `tests/test_parser.cpp`、`tests/test_end_to_end.cpp`、`tests/test_simulate.cpp`、`tests/test_sources.cpp`；修改 `src/CMakeLists.txt` 挂载新头文件。覆盖分析冲突、名称映射/重名、V/I 克隆不污染原对象、旧 `.op/.tran` 结果。
- 第 1 档验证：parser 38/38、source 8/8、controller 9/9、end-to-end 5/5；先构建再全仓测试 158/158。
- 第 1 档提交：`f5eb3ba refactor: add typed analyses and independent sources`。
- 第 2 档 AI 生产代码：修改 `src/parser/Circuit.h` 公开唯一扫描序列函数；修改 `src/parser/Circuit.cpp::generate_dc_sweep_values/parse_circuit`，实现 5/9 token `.dc`、整数步号序列、前向引用、V/I 类型绑定及带行号错误。未修改任何器件、solver、writer 或 CLI 生产实现。
- 第 2 档 AI 测试：修改 `tests/test_parser.cpp`，新增 6 个测试覆盖升/降/不整除/单点序列、异常序列、前向引用、双源绑定、错误分类和分析冲突。
- 第 2 档验证：parser 44/44；先构建再全仓测试 164/164。
- 第 2 档提交：`cd02976 feat: parse dc sweep analyses`。
- 第 3 档 AI 生产代码：修改 `src/sim/simulate.h` 新增 DC point/result 并扩展 result variant；修改 `src/sim/simulate.cpp`，新增源绑定防御、单/双层扫描、局部 clone 覆盖、延续初值、点数溢出检查和带点/源值的失败传播；`src/output/result_writer.cpp` 暂时增加 DC 穷尽分支以避免 variant 误分派，完整 writer 留第 4 档。未修改 Newton、器件 stamp、MNA、LU 或 transient。
- 第 3 档 AI 测试：修改 `tests/test_simulate.cpp`，新增 7 个测试，使用分压/KCL/双源手算 oracle、Newton 初值 probe、二极管 KCL 残差+单点参考、重复调用和失败上下文验证契约。
- 第 3 档验证：controller 16/16；先构建再全仓测试 171/171。
- 第 3 档提交：`f2307ca feat: execute dc sweep analyses`。
- 第 4 档 AI 生产代码：修改 `src/output/result_writer.h/.cpp`，新增 DC CSV 的写前完整形状/名称/维度校验、`sweep(source)` 列和 `.op/.tran/.dc` 穷尽分派；CLI runner 不增加分析逻辑，直接消费结构化 result。
- 第 4 档 AI 测试/工程：修改 `tests/test_result_writer.cpp`、`tests/test_cli.cpp`、`tests/CMakeLists.txt`；新建 `tests/fixtures/cli_dc.cir`；覆盖单/双源 writer、八类写前失败、三类 variant、CLI DC 成功/失败、DC 真实进程和三份 example。
- 第 4 档 AI examples/文档：新建 `examples/dc_divider.cir`、`examples/dc_double.cir`、`examples/dc_current_descending.cir`；修改 `README.md` 写明语法、运行、CSV、Python/MATLAB 消费和限制；完成 M09 回执与本台账归属清单。
- 第 4 档 AI 修正：首次 CLI 测试使用短小数 `-0.0005` 期待，与 writer 的可往返 `double` 精度契约不符；修正测试期待为实际稳定文本，未降低生产输出精度。
- 第 4 档独立只读复核与修正：复核发现第 2 档扫描容差以 `1.0` 为尺度下限，会破坏极小区间和大偏置小步长。AI 修改 `src/parser/Circuit.cpp::generate_dc_sweep_values`，改用 stop ULP/机器 epsilon 量级且由 `|step|/4` 封顶的边界容差，命中 stop 后结束并保证严格单调无重复；修改 `src/sim/simulate.cpp::validate_device_identity/simulate_dc_sweep`，拒绝手工 Circuit 的 null、空名、大写、重名及双轴同源。
- 第 4 档复核测试：修改 `tests/test_parser.cpp`，加入显式 `.dc v1 0 1p 0.1p` 和 `1e12` 大偏置小区间的 11 点严格递增回归；修改 `tests/test_simulate.cpp`，加入 null/重名/未规范化身份及手工双轴同名拒绝。该问题在第 4 档提交前被捕获和修复，未进入已推送的最终实现提交。
- 第 4 档验证：parser 45/45，controller 17/17，writer 11/11，CLI 函数级 11/11，DC 真实进程/examples 4/4，先构建再全仓 182/182；三份 DC examples 手工实跑；单源 CSV 通过 Python 标准库和 MATLAB R2026a `readtable` 实机回读断言。
- 第 4 档提交：`ae05de8 feat: add dc sweep output and examples`；本行由随后纯文档提交回填。
- 任务会话收口：`8280d40 docs: finalize M09 dc sweep receipt`，`HEAD == main == origin/main` 且工作区干净。
- 主会话独立验收：重新构建成功；M09 定向 23/23、全仓 182/182；三份 DC examples 手工实跑并核对点数、嵌套顺序和数值；`git diff --check fb6bba8..8280d40` 通过，未发现新的阻塞问题。
- 主会话结论：`TS-M09-DC` 正式销项；当前活动任务置空，未创建或启动 M10。

### `AI-0005`：TinySpice CLI 自动调用 MATLAB 批处理绘图

> 日期：2026-08-30
> Task ID：`TS-M08-MATLAB-BATCH`
> 类型：M08 波形工具后续工程化

- 目标与范围：用户选择“C++ 生成 CSV 后通过 `matlab -batch` 自动绘图”；只扩展 CLI 可选后处理，不修改 `SpiceLib`、仿真算法、CSV 契约或 MATLAB 绘图函数。
- AI 设计：新增 `--matlab-plot <image-file>`，要求同时提供 `-o <csv-file>` 且分析必须为 `.tran`；先完整提交 CSV，再启动 MATLAB，因此绘图失败不会丢失仿真结果。输入、CSV 和图片路径必须互不指向同一文件。
- AI 生产代码：新建 `sandbox/cli/matlab_plot.h/.cpp`，使用无 shell 的子进程参数调用 `matlab -batch`；支持环境变量显式路径、macOS 应用目录自动发现和 `PATH` 回退。修改 `sandbox/cli/run_cli.h/.cpp` 增加参数解析、分析类型闸门、错误传播和可注入测试边界；修改 `sandbox/CMakeLists.txt` 挂载实现并记录脚本目录。
- AI 测试：修改 `tests/test_cli.cpp`，新增 CSV 先写后调用、参数顺序、MATLAB 失败保留 CSV、非瞬态写前拒绝、路径冲突五项测试，并扩充非法参数矩阵。
- AI 文档：修改 `README.md`，记录自动运行方式、查找顺序、错误语义和分布式 Worker 边界。
- 明确未修改：全部 `src/`、`scripts/plot_transient.m`、Python 工具、求解器、器件和结果 writer。
- 实机验证：本机自动发现 `/Users/reelon/Applications/MATLAB/MATLAB_R2026a.app`，真实 RC 仿真经单条 CLI 命令成功生成 CSV 和 1274×820 PNG，目视确认三条波形、坐标和图例正确；用不存在的 MATLAB 路径验证退出码 1、CSV 保留且无图片。
- 自动测试：CLI 定向 20/20、全仓 187/187；`git diff --check` 通过。
- 提交：`d94c808 feat: automate MATLAB transient plotting`；本行由随后纯文档提交回填。

### `AI-0006`：项目暂停与跨项目交接收口

> 日期：2026-08-30
> Task ID：`TS-PROJECT-PAUSE`
> 类型：只读复验、状态冻结与文档交接

- 用户决定暂时截止 TinySpice 独立开发，要求确认 Go 联动形态、登记 MATLAB 系统路径并形成可恢复/可跨项目阅读的状态文档。
- AI 核对 CMake：当前为 `SpiceLib STATIC`、`TinySpiceCli STATIC` 和 `TinySpice` executable，不存在 DLL、`.dylib`、`.so` 或稳定 C ABI；Go 第一版应使用 CLI 子进程。
- AI 新建 `会话交接/项目暂停交接_2026-08-30.md`，集中记录功能、架构、命令、验证、归属、限制、M10～M17、Go 联动方式、恢复清单和跨项目开场提示词。
- AI 更新本台账和 `项目梳理与开发计划.md`，将项目标为暂停、当前活动任务保持为空；未创建或启动 M10。
- 本地环境登记：MATLAB R2026a `bin` 已加入 `~/.zprofile` 与 `~/.zshrc`；登录和非登录交互 zsh 均能直接解析 `matlab`，batch 版本调用返回 R2026a Update 5。用户目录 shell 配置不属于 Git 仓库提交。
- 明确未修改：全部生产代码、测试、CMake、README、MATLAB/Python 脚本和 187 项既有测试。
- 验证：重新构建成功，全仓 187/187，`git diff --check` 通过。
- 暂停交接提交：`6d1553c docs: pause TinySpice and add project handoff`；本行由随后纯文档提交回填。

## 六、已确认的后续工具路线

2026-08-30 用户确认采用双路线：

- `scripts/plot_transient.py` 是仓库的自动校验、CI 友好和跨平台绘图入口；
- `scripts/plot_transient.m` 是 MATLAB 环境中的交互分析伴侣；
- 二者只消费同一份 TinySpice CSV，不改变仿真内核、CLI 或 CSV 契约。
- CLI 的 `--matlab-plot` 是可选自动后处理入口，仍通过 CSV 调用 MATLAB，不把 MATLAB 依赖引入 `SpiceLib` 或分布式 Worker。
