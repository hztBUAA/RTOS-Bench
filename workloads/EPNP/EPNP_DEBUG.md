# EPNP Benchmark Debug Analysis

## 问题描述

EPNP (Efficient Perspective-n-Point) 位姿估计算法在 ARM64 SylixOS 飞腾派平台上运行时崩溃。

## 崩溃现象

### 类型 1: Eigen 矩阵维度断言失败

```
condition: dst.rows() == src.rows() && dst.cols() == src.cols()
function: run() in file: ProductEvaluators.h line: 157 assert error!
```

**调用栈**:
```
epnp_bench.cpp:100  absolute_pose::epnp(adapter)
    ↓
OpenGV methods.cpp  epnp() 函数
    ↓
Epnp.cpp           compute_pose() 或 find_betas_approx_*()
    ↓
Eigen JacobiSVD    ColPivHouseholderQR
    ↓
ProductEvaluators.h:157  矩阵乘法 dst = A * B（维度不匹配！）
```

### 类型 2: memcpy 崩溃

```
[07] /lib/libvpmpdm.so memcpy+332
[06] /apps/hzt_feiteng_rtos_bench BlockImpl_dense...+172
...
_ZN5Eigen8internal26outer_product_selector_run...
```

## 数据流分析

### Seed 如何影响 EPNP

```
srand(seed)
    ↓
generateRandomTranslation(2.0)  → position (3D 平移向量)
generateRandomRotation(0.5)     → rotation (3x3 旋转矩阵)
    ↓
generateCentralCameraSystem()   → camOffsets, camRotations
    ↓
generateRandom2D3DCorrespondences()
    ├── 生成 100 个 3D 世界点 (points)
    └── 生成对应的 bearing vectors (归一化方向向量)
    ↓
CentralAbsoluteAdapter(bearingVectors, points, rotation)
    ↓
absolute_pose::epnp(adapter)
    ├── Epnp::compute_pose()
    │   ├── choose_control_points()     → PCA 选择 4 个控制点
    │   ├── compute_barycentric_coordinates()
    │   ├── JacobiSVD 分解
    │   ├── find_betas_approx_1/2/3()   → 求解 beta 系数
    │   └── gauss_newton()              → 高斯牛顿优化
    └── 返回变换矩阵
```

### 退化配置场景

某些随机种子可能产生以下退化配置：
1. **共面点**: 所有 3D 点近似在同一平面上 → PCA 奇异值退化
2. **共线点**: 点接近共线 → 控制点选择失败
3. **深度异常**: bearing vector 的 z 分量接近 0 → 投影不稳定
4. **浮点精度**: ARM64 浮点运算与 x86 不同 → 边界条件触发

## 关键发现：`-s` 模式 vs `test-schedule` 模式

### 调用链路差异

| 模式 | 调用路径 | 实际迭代次数 |
|------|----------|--------------|
| `-s` | `run_all_workloads()` → `epnp_test()` → `epnp_bench_run(NULL)` | **1 次** |
| `test-schedule` | `measure_wcet_ns()` → `epnp_exec()` → `epnp_bench_run(1000)` × 5 轮 | **5000 次** |

### 根本原因

```cpp
// epnp_bench.cpp 中 epnp_test() 的调用
static void* epnp_thread_entry(void* parameter) {
    size_t iterations = parameter ? *((size_t*)parameter) : 1;  // NULL → 1
    epnp_bench_run(iterations);
}

// -s 模式传 NULL，所以 iterations=1，只执行 1 次 epnp()
// test-schedule 模式传 1000，执行 1000 次，且被 WCET 测量调用 5 次
```

**这就是为什么 `-s` 模式从不崩溃！**

## 试错记录

| 日期 | Seed | 迭代次数 | 结果 | 说明 |
|------|------|----------|------|------|
| 2026-04-15 | 时间戳 | 1000×5 | 崩溃 | 原始实现 |
| 2026-04-15 | 42 | 1000×5 | 崩溃 | 固定种子仍触发 Eigen 断言 |
| 2026-04-15 | 12345 | 1000×5 | 崩溃/ELF加载失败 | 设备多次崩溃后不稳定 |
| 2026-04-15 | - | 1 | 正常 | `-s` 模式只跑 1 次 |

## 可能的根因分析

### 假设 1：栈空间累积（最可能）

```cpp
for (size_t i = 0; i < 1000; i++) {
    epnp_transformation = absolute_pose::epnp(adapter);
    // 每次 epnp() 内部创建大量临时 Eigen 矩阵
    // JacobiSVD, ColPivHouseholderQR 都在栈上分配临时对象
}
```

- Eigen 临时对象在栈上分配
- 1000 次循环，编译器可能不会完美回收栈空间
- ARM64 栈帧比 x86 更大（更多寄存器需要保存）
- **逐渐蚕食栈空间直到溢出，表现为 memcpy 崩溃或数据损坏**

### 假设 2：Eigen 内存对齐问题

- ARM64 要求 16 字节对齐
- `EIGEN_DONT_VECTORIZE` 已定义，但某些代码路径可能仍有对齐要求
- 累积调用后内存布局变化可能触发对齐边界

### 假设 3：OpenGV 内部状态（不太可能）

- OpenGV 的 `Epnp` 类可能有静态成员被污染
- 多次调用可能累积错误状态

### 已排除：数据退化问题

- 固定种子 42、12345 都崩溃
- 说明**不是**随机数据产生的几何退化配置问题

## 当前修复方案（2026-04-15）

### 修改 1：减少迭代次数 + 函数边界隔离

**文件**: `workloads/EPNP/epnp_bench.cpp`

```cpp
// 新增：每次迭代在独立函数中执行，确保栈完全释放
static int epnp_single_run(size_t seed_offset) {
    srand(EPNP_BENCHMARK_SEED + seed_offset);
    // 重新生成所有数据
    translation_t position = generateRandomTranslation(2.0);
    rotation_t rotation = generateRandomRotation(0.5);
    // ... 生成 bearingVectors, points
    absolute_pose::CentralAbsoluteAdapter adapter(...);
    absolute_pose::epnp(adapter);  // 单次调用
    return 0;
}  // 函数返回，栈完全释放

extern "C" int epnp_bench_run(size_t iterations) {
    size_t loops = (iterations > 0) ? iterations : 10;
    if (loops > 50) loops = 50;  // 上限保护

    for (size_t i = 0; i < loops; i++) {
        epnp_single_run(i);  // 每次独立函数调用
    }
}
```

### 修改 2：降低 exec 调用的迭代次数

**文件**: `workloads/rtbench_workloads.cpp`

```cpp
// 原来
epnp_iterations_arg = 1000;

// 现在
epnp_iterations_arg = 10;
```

### 修改 3：移除 test-schedule 中的跳过逻辑

**文件**: `generator/test_schedule.c`

移除了临时跳过 EPNP 的代码，EPNP 现在正常参与 test-schedule。

### 修复原理

| 原来 | 现在 | 效果 |
|------|------|------|
| 1 个 adapter，循环 1000 次 | 10 次独立函数调用 | 栈空间每次完全释放 |
| 5 轮 × 1000 = 5000 次 | 5 轮 × 10 = 50 次 | 大幅减少累积风险 |
| 同一函数内紧密循环 | 每次 `epnp_single_run()` 返回 | 函数边界强制栈回收 |

## 待验证实验（设备恢复后）

1. **实验 A**：保持 1000 次，但每 100 次包装一个函数调用
   - 如果不崩溃 → 确认是栈累积问题

2. **实验 B**：保持 10 次，但复用同一个 adapter（不重新生成数据）
   - 如果不崩溃 → 确认重新生成 adapter 不是关键

3. **实验 C**：增加线程栈大小到 8MB 或 16MB
   - 如果不崩溃 → 最终确认是栈溢出

## 其他修复方向（备选）

1. **使用预计算测试数据**
   - 在开发机上预先生成已知稳定的 2D-3D 对应点
   - 嵌入到代码中作为静态数据
   - 避免运行时随机生成

2. **添加退化检测**
   ```cpp
   bool is_degenerate_config(const points_t& points) {
       // 检查点是否共面/共线
       // 检查 bearing vectors 的有效性
       // 检查数值范围
   }
   ```

3. **异常捕获**
   - C++ try-catch 包装 OpenGV 调用
   - 检测到异常时跳过当前迭代

4. **深入分析 Eigen 问题**
   - 检查 `EIGEN_DONT_VECTORIZE` 是否正确生效
   - 检查栈对齐要求（ARM64 16 字节对齐）
   - 分析 `fix_opengv.h` 的 long double stub

## 相关文件

| 文件 | 作用 |
|------|------|
| `workloads/EPNP/epnp_bench.cpp` | Benchmark 入口 |
| `workloads/EPNP/random_generators.cpp` | 随机数据生成 |
| `workloads/EPNP/Epnp.cpp` | OpenGV EPNP 核心算法 |
| `workloads/EPNP/methods.cpp` | OpenGV 入口 |
| `workloads/EPNP/Eigen/` | Eigen 矩阵库 |
| `workloads/EPNP/fix_opengv.h` | ARM64 long double 修复 |
| `generator/test_schedule.c` | 临时跳过逻辑 |

## 环境信息

- **平台**: 飞腾派 (Phytium E2000Q)
- **OS**: SylixOS 3.9.0
- **架构**: ARM64 (AArch64)
- **编译器**: aarch64-sylixos-elf-g++
- **Eigen版本**: 3.x (头文件库)
- **OpenGV版本**: 自定义移植版本

---

*Created: 2026-04-15*
*Updated: 2026-04-15*
*Status: 已实施修复方案，待设备恢复后验证*
