# TinySpice

TinySpice 是一个学习用途的 C++ SPICE 子集：读取网表，执行直流工作点或固定步长瞬态分析，并把结果输出为带电路标签的文本或 CSV。

## 构建与测试

需要支持 C++23 的编译器和 CMake 3.16+。第一次配置会下载 GoogleTest，因此需要网络；之后可复用构建缓存。

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

下文以 Unix 风格的 `./build/sandbox/TinySpice` 为例。多配置生成器（例如 Visual Studio）通常把可执行文件放在 `build/sandbox/Debug/`。

## 命令行

```text
TinySpice <netlist-file>
TinySpice <netlist-file> -o <output-file>
```

不带 `-o` 时写 stdout；带 `-o` 时只写指定文件。成功退出码为 0，文件、解析、求解或输出失败为 1，命令行参数错误为 2。

运行 `.op` 分压示例：

```bash
./build/sandbox/TinySpice examples/divider_op.cir
```

输出包含节点电压、V/L 元件支路电流和 Newton 迭代数，例如 `V(2) = 8`。电压单位为 V，支路电流单位为 A。

导出 RC 或 RL 瞬态 CSV：

```bash
./build/sandbox/TinySpice examples/rc_transient.cir -o build/rc.csv
./build/sandbox/TinySpice examples/rl_transient.cir -o build/rl.csv
```

CSV 第一列固定为 `time`（秒），之后依次是 `V(node)` 和 `I(branch)`。标签顺序与 MNA 未知量顺序一致；数值使用 C locale 和可往返的 `double` 精度。

## 绘制瞬态波形

CSV 是稳定的数据交换格式。Python 工具作为自动校验和跨平台绘图入口；MATLAB 工具作为交互分析伴侣，两者读取同一份 CSV。

### Python

脚本需要 Python 3。实际绘图还需要 matplotlib：

```bash
python3 -m pip install matplotlib
```

绘制全部波形并保存图片：

```bash
python3 scripts/plot_transient.py build/rc.csv -o build/rc.png
```

只绘制指定列（列名含括号时应加引号）：

```bash
python3 scripts/plot_transient.py build/rc.csv 'V(out)' -o build/rc-output.png
```

不安装 matplotlib 也可验证 CSV 的表头和数值：

```bash
python3 scripts/plot_transient.py build/rc.csv --validate-only
```

省略 `-o` 会打开交互式窗口。脚本会明确报告文件不存在、缺少 `time`、列名不存在以及选中列含非数值等错误；未指定列时检查并绘制全部非 `time` 列。

### MATLAB

MATLAB R2020a 或更新版本可直接保留 `V(out)`、`I(v1)` 这类 CSV 表头并导出图片。在项目根目录执行：

```matlab
addpath("scripts");

% 交互显示全部波形
plot_transient("build/rc.csv");

% 只显示指定列
plot_transient("build/rc.csv", "V(out)");

% 选择多列并保存图片
plot_transient("build/rc.csv", ["V(out)", "I(v1)"], ...
    "build/rc-matlab.png");
```

若要保存全部波形，可把第二个参数写成 `[]`。MATLAB 工具同样会检查文件、`time` 列、波形列、空数据以及非数值或非有限值。上述全部列、指定列、多列图片导出和缺失列错误路径已在 MATLAB R2026a Update 5 实机验证。

## 当前网表语法

- 元件：`R`、独立 DC `V`、独立 DC `I`、`D`、`C`、`L`。
- `R/V/I/C/L` 行为 `name node+ node- value`；二极管行为 `name node+ node-`，当前使用内置模型参数。
- 分析指令：`.op` 或 `.tran <tstep> <tstop>`；`.end` 结束读取。
- 完整行注释以 `*` 开头；名称和指令不区分大小写，输出标签统一为小写。
- 数值支持普通/科学计数法及 `k`、`meg`、`g`、`t`、`m`、`u`、`n`、`p`、`f` 后缀。

示例位于 [`examples`](examples/) 目录。

## 明确限制

- 每份网表只执行一个分析；尚不支持 `.dc`、分析任务列表或 `.op/.tran` 冲突诊断。
- 瞬态分析采用固定步长后向欧拉，并从全零状态的 `t=0` 点开始；尚无 `.ic`/UIC、DC 工作点初始化、变步长或梯形法。
- `.tran` 的 `tstop/tstep` 必须在浮点容差内为整数，当前不会自动缩短最后一步。
- 独立源只有常量 DC 值；尚无 PULSE、SIN、PWL。
- 二极管模型参数固定；尚无模型卡、MOSFET、受控源。
- 当前为稠密矩阵求解器；尚无稀疏矩阵、大轨迹流式输出或 ngspice 自动对拍。
- CSV 标签不支持逗号、双引号或换行；输出不携带单位列。
