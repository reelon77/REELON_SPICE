# 回执 · Day 4:牛顿迭代 + 二极管 companion model

执行日期:2026-08-04。**08-05 里程碑(非线性 DC)提前一天达成。**

## 一、完成情况

**热身(两项都完成)**
- LU 接口收敛:`LUResult lu_decomposition(const Matrix&)`,出参与尺寸死检查删除,
  16 处调用点同步,连带删除 2 个失去被测对象的用例 → `deefb93`
- 极性命名:VoltageSource `node_pos_/node_neg_`、CurrentSource `node_from_/node_to_`
  (电流源用 from/to 比 pos/neg 更准确),方向约定入头文件注释 → `f6350b1`

**正餐**
- 接口方案:**杂交虚函数**(基类加 `stamp(mna, x)` 默认转发 → 线性器件零改动,
  Diode 只 override 双参版;单参版 override 成抛 `logic_error` 响亮拒绝误用)。
  方案由用户在三候选 + variant/concepts 替代方案的完整权衡后选定,能说理由。
- 二极管电路收敛:**v2 = 0.574191503 V(与真解逐位一致),初值全 0 实测 177 次迭代**
  (交接单预告 178,浮点路径差异,量级精确命中)。
- 单测:第 1~5 档全绿。全仓 53 用例 51 绿(2 红为 naked LU 已知缺陷,本日未动)。
- commit:`deefb93` `f6350b1` `2902f5e`(Diode)`5ba92b1`(clear)`ca43473`(solver,里程碑)

## 二、提交纪律执行情况

**5 个阶段 5 次提交,零攒批**——Day 3 的 0 提交对比鲜明。但如实说:git 全部由 AI 代敲
(用户授权的分工:"机械活不占我时间"),用户本人尚未亲手执行过阶段提交。习惯是否内化待观察。

## 三、代码质量评价

- 接口改造:杂交方案落地干净,分层试金石通过(solver 不 include Diode)。
- 清零重组装:`Matrix::clear` + `MnaSystem::clear` 下沉到位,烟囱测试守住。
- 收敛判据:∞ 范数,`transform_reduce` 一行流(review 放行不点赞:能跑、略难读)。
- **遗留小尾巴(未修,下次热身清)**:
  1. newton.h 里 `<cmath>/<numeric>` 是实现细节该挪进 .cpp;newton.cpp 缺自己的
     `<stdexcept>/<numeric>`(现靠传递包含,链条脆)
  2. `newton_solve` 缺 tol/max_iter 默认值(说定 1e-9/300)
  3. `NewtonResult::res` 命名弱,建议改 `x`(和数学记号对齐)
  4. `Matrix::clear` 与 STL clear 语义冲突(建议 `set_zero`)
  5. Diode 两端同时接地走 `x[-1]`(病态网表,已备案)

## 四、用户掌握度评价

- **companion model:真懂,但过程值得记录。** g_eq 推导三轮才对:第一轮在 0 处展开
  (锚点错),第二轮形状错(g_eq 含自由变量、丢 f(k)),第三轮 g_eq 对但 I_eq 仍丢
  f(k) 项,第四轮全对。暴露的问题不是微积分能力,是**不跑对答数验证**——三次错误
  全部能被"代 0.5 验一下"当场拦截,但他嫌麻烦跳过,直到被指出"对答数 = 机器算数、
  人对数"才接受。此后写代码阶段这个习惯有好转。
- **多维牛顿**:经"三维严格降到一维"的桥接讲解(讲义 artifact)后概念通。中途有一次
  典型误解("先解线性再单独迭代非线性"),用"消元与牛顿可交换 + 消元是谁做的"讲透。
- **stamp 符号**:忘了又重建,"矩阵元 = 灵敏度 / 自己涨水往外漏"的物理画面反馈极好。
- **"改A漏B"复现 2 次**:①Diode 加双参接口没动基类;②新增 3 文件没挂 CMakeLists
  (预告过的头号高发点)。较 Day 3 的 4 次有收敛,`override` 卫兵的价值已亲历。
- **新行为模式(要盯)**:review 意见不逐条落实就喊下一轮,当天发生 3 次(std::exp
  提了 4 轮才改);Diode 取电压的地结点处理连续 3 版绕开"写小函数"的建议(最终用
  三分支 if 落地,可接受但啰嗦)。已明说:"review 意见是清单,不是散文"。

## 五、遗留问题

- 两个 naked LU 红测试(技能课压轴被跳过,转对照用例的活继续挂着)
- 三之"小尾巴"5 项 + launch.json 入库决策 + `docs/构建工具链与调试.md` 用户短笔记
- 面试素材待用户自己归档:177 步爬坡机理(每步 ≈Vt)、limiting 的动机、二次收敛的
  位数翻倍证据(讲义 artifact 里有实算表)、虚函数 vs variant+concepts 权衡

## 六、AI 代写清单(供登记核对)

1. test_lu.cpp / test_end_to_end.cpp:16 处调用点机械改 + 删 2 用例(deefb93)
2. LU.cpp 一行注释更新(deefb93)
3. VoltageSource / CurrentSource 四文件改名 + 头文件注释(f6350b1)
4. tests/test_diode.cpp 全文 9 用例 + tests/CMakeLists.txt 挂载(2902f5e)
5. tests/test_newton.cpp 全文 3 用例 + tests/CMakeLists.txt 挂载(ca43473)
6. 两个讲义 artifact(牛顿数学案例 + 求解器施工图)、Day4 题面 .md
7. git 提交全部代敲(5 次,均带 Co-Authored-By)
8. **src/ 核心代码(Device.h 双参虚函数、Diode、clear、newton_solve)全部用户亲手写**,
   AI 仅 review
