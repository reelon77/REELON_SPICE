# TinySpice 1.0 实用 SPICE 子集规格与路线图

> 规格冻结日期：2026-08-30
> 决策人：用户
> 实现责任：AI 负责新增生产代码、测试、工程挂载、文档和 Git；历史归属保持不变
> 基线：`4d102cc docs: record MATLAB runtime verification`

## 一、产品定位与命名边界

TinySpice 1.0 是一个自主实现、可解释、可测试的 **SPICE3 实用子集**。它的目标是覆盖离散模拟电路学习、器件曲线、基础放大器和 RC/RL/RLC 瞬态所需的主干能力，并通过 ngspice 对拍证明数值与语义可信。

“1.0 完成”不等于“兼容任意 ngspice/PSPICE/HSPICE 网表”，也不等于支持现代 PDK 的全部工业模型。README、简历和面试中必须使用“实用 SPICE 子集”或“TinySpice 1.0”，不得写成完整替代 ngspice。

官方 ngspice 的处理链还包括参数/子电路预处理、复杂器件模型、稀疏矩阵、时间步控制、XSPICE 和外部模型接口等长期演进能力。本项目只实现本文件明确列出的 1.0 契约：

- <https://ngspice.sourceforge.io/docs.html>
- <https://ngspice.sourceforge.io/docs/ngspice-manual.pdf>
- <https://ngspice.sourceforge.io/modelparams.html>

## 二、TinySpice 1.0 完成定义

### 1. 分析类型

| 分析 | 1.0 契约 |
|------|----------|
| `.op` | 线性与支持范围内的非线性直流工作点 |
| `.dc` | 一个或两个独立 V/I 源的线性扫描；双源时第二源为外层 |
| `.ac` | `lin/dec/oct` 小信号频率扫描，复数节点电压和支路电流 |
| `.tran` | DC 工作点初始化、`.ic/UIC`、后向欧拉/梯形法、受控变步长 |

### 2. 器件与源

| 类别 | 1.0 契约 |
|------|----------|
| 无源器件 | R、C、L |
| 独立源 | V、I；DC、PULSE、SIN、PWL、AC 幅值/相位 |
| 线性受控源 | E（VCVS）、G（VCCS）、F（CCCS）、H（CCVS） |
| 半导体 | D 模型卡、BJT、JFET、MOSFET Level 1 |

### 3. 网表组合能力

- `.model`：支持 1.0 器件所需的明确参数集合和默认值。
- `.param`：标量参数和基础算术表达式；禁止静默忽略未知参数。
- `.include`：相对当前文件解析；循环包含和缺失文件明确报错。
- `.subckt/.ends`：层次实例化、节点绑定和实例内名称隔离。
- `.ic`、`.end`，以及 `.op/.dc/.ac/.tran` 的唯一分析指令约束。
- 名称大小写不敏感；错误必须尽量保留文件和行号。

### 4. 数值与工程能力

- 保留稠密手写 LU 作为教学/小电路后端；新增稀疏矩阵后端并通过同一组数值测试。
- 非线性求解加入阻尼、器件 limiting、延续初值和可观察的收敛诊断。
- 瞬态失败时缩步重试；历史状态只在成功步后提交。
- CLI、结构化结果、CSV、Python 和 MATLAB 消费链保持分层，不把输出逻辑塞回 solver。
- 每一分析至少包含解析/手算 oracle；1.0 发布前再加入 ngspice 差分回归。

## 三、明确不属于 1.0

- BSIM3/4、HiSIM、PSP、SOI、HiCUM 等工业紧凑模型及完整 PDK 兼容。
- Verilog-A、OSDI/OpenVAF、XSPICE code model、数字/混合信号内核。
- B 行为源、传输线、开关、互感、噪声、失真、极零、灵敏度、温度扫描和蒙特卡洛。
- 任意厂商加密模型、任意 ngspice 控制脚本或 Nutmeg 命令兼容。
- GUI、原理图编辑器、分布式仿真农场。

这些能力只有经用户重新扩展规格后才能进入主线；不能因为官方 SPICE 存在就默认纳入。

## 四、架构演进约束

1. `Circuit` 是解析后的拥有型 IR；分析请求必须是类型安全数据，不能继续无限堆叠互相矛盾的裸字段。
2. `simulate` 只做分析编排；parser、stamp、Newton、时间步、输出不得相互复制。
3. 扫描和时间变化不能通过修改 `const Circuit` 内的器件制造跨调用污染；参数覆盖必须是调用局部状态。
4. 器件名称、节点名称、branch 名称和模型引用必须有唯一、稳定、可验证的映射。
5. 稠密与稀疏后端必须共享器件 stamp 语义和数值 oracle，禁止维护两套物理公式。
6. 任何新结果类型先定义结构化 C++ 契约，再接 writer/CLI；核心层不直接打印。
7. 每个阶段先构建再测试：`cmake --build ./build` 后执行 `ctest --test-dir build --output-on-failure`。

## 五、模块路线与依赖

| 顺序 | Task ID | 建议会话名称 | 交付物 | 直接依赖 |
|------|---------|--------------|--------|----------|
| 1 | `TS-M09-DC` | `TinySpice｜M09-DC｜直流扫描与源抽象` | 类型化分析请求、源身份、单/双源 `.dc`、CSV/CLI | M08 |
| 2 | `TS-M10-CTRL` | `TinySpice｜M10-CTRL｜线性受控源` | E/G/F/H 与控制支路引用 | M09 |
| 3 | `TS-M11-SOURCE` | `TinySpice｜M11-SOURCE｜时变与交流源` | DC/PULSE/SIN/PWL/AC 源表达 | M09、M10 |
| 4 | `TS-M12-AC` | `TinySpice｜M12-AC｜小信号交流分析` | 复数系统、线性化、lin/dec/oct、输出 | M10、M11 |
| 5 | `TS-M13-MODEL` | `TinySpice｜M13-MODEL｜模型卡与半导体器件` | D model、BJT、JFET、MOS Level 1 | M09、M12 |
| 6 | `TS-M14-NETLIST` | `TinySpice｜M14-NETLIST｜参数子电路与包含` | `.param/.include/.subckt` 展开和诊断 | M13 |
| 7 | `TS-M15-TRAN` | `TinySpice｜M15-TRAN｜进阶瞬态与收敛` | DC-init、`.ic/UIC`、梯形法、变步长、阻尼/limiting | M11、M13 |
| 8 | `TS-M16-SPARSE` | `TinySpice｜M16-SPARSE｜稀疏求解后端` | 稀疏存储/求解接口、后端一致性和规模测试 | M12、M15 |
| 9 | `TS-M17-VERIFY` | `TinySpice｜M17-VERIFY｜ngspice对拍与1.0发布` | 差分测试、示例、文档、性能基线、`v1.0.0` | M09～M16 |

主会话只能下发当前活动任务。任务会话完成本任务回执和 Git 收口后必须停止，不得自行预研或启动表中的下一行。

## 六、统一绿灯与发布闸门

每个模块必须同时满足：

1. 设计闸门记录关键选择、被拒方案和不变量。
2. 生产代码、测试、CMake 和文档范围与任务单一致。
3. 新增数值测试使用独立手算/解析值；旧实现输出不能成为唯一 oracle。
4. 错误路径有测试，不靠崩溃或空结果表达失败。
5. 定向测试和全仓测试全绿，`git diff --check` 通过。
6. `docs/AI后续实现台账.md` 登记 AI 设计、函数/文件、测试、修复和提交。
7. 每档绿灯立即提交并推送，不跨档攒批。

TinySpice 1.0 最终发布还必须满足：

- 四种分析和全部 1.0 器件至少各有一个 CLI example；
- ngspice 差分测试覆盖线性、非线性、DC、AC 和 transient 主路径；
- README 明确支持矩阵与限制，不使用“完整兼容 SPICE”的误导表述；
- clean build 与全仓测试通过，`main == origin/main`，工作区干净；
- 主会话正式验收并创建 `v1.0.0` 标签。
