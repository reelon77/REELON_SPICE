# TinySpice AI 后续实现台账

> 建立日期：2026-08-30
> 起算基线：`4d11881 docs: add M08 implementation handoff inventory`
> 状态：启用；波形绘图采用 Python 自动化 + MATLAB 交互分析双路线

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

验证：M08 主会话复验全仓 152/152；三份 example、CLI CSV 和 Python `--validate-only` 通过。matplotlib/MATLAB 图片渲染尚未在当前机器实跑。

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
- AI 测试/CMake：未新增或修改；当前环境没有 MATLAB/Octave，不虚构运行测试，也不把不可运行的 MATLAB 用例挂入 CTest。
- 明确未修改：全部 `src/`、CLI、CSV writer、Python 工具及既有测试。
- 验证：重新构建成功；全仓 152/152；真实 RC CSV 经 Python `--validate-only` 验证成功（3 行，`time/V(in)/V(out)/I(v1)`）；MATLAB 文件完成静态审阅。当前环境没有 MATLAB/Octave，MATLAB 实机运行状态为“未验证”。
- 提交：`19c3751 feat: add MATLAB transient waveform companion`；本行由随后纯文档提交回填。
- 用户复核点：在学校 MATLAB 中依次执行 README 的全列、选列和保存图片示例；若 MATLAB 报版本兼容问题，再根据实际版本做最小兼容调整。

## 六、已确认的后续工具路线

2026-08-30 用户确认采用双路线：

- `scripts/plot_transient.py` 是仓库的自动校验、CI 友好和跨平台绘图入口；
- `scripts/plot_transient.m` 是 MATLAB 环境中的交互分析伴侣；
- 二者只消费同一份 TinySpice CSV，不改变仿真内核、CLI 或 CSV 契约。
