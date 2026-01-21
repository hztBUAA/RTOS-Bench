#ifndef PID_DATA_H
#define PID_DATA_H

#include <stdint.h>

// 数据长度
#define SYNTH_DATA_COUNT 100

typedef struct {
    double target;   // 目标值 (Setpoint)
    double measured; // 当前测量值 (Input/Process Value)
    const char* desc;// 场景描述（用于调试）
} PidSimData;

/* * 构造策略：
 * 1. P项压力：巨大的误差跳变 (Step Change)
 * 2. D项压力：目标值不变，但测量值高频剧烈抖动 (Noise)
 * 3. I项压力：长时间保持一个微小的恒定误差 (Steady State Error)
 */
static const PidSimData g_synth_data[SYNTH_DATA_COUNT] = {
    // Phase 1: 静止状态 (Warm up)
    {0.0, 0.0, "Idle"}, {0.0, 0.0, "Idle"}, {0.0, 0.0, "Idle"}, {0.0, 0.0, "Idle"},
    
    // Phase 2: 阶跃响应 (Step Response) - 测试 P 项爆发计算
    // 误差瞬间从 0 变到 1000
    {1000.0, 0.0, "Step_Start"}, 
    {1000.0, 200.0, "Step_Rise"},
    {1000.0, 500.0, "Step_Rise"},
    {1000.0, 800.0, "Step_Rise"},
    {1000.0, 950.0, "Step_Rise"},
    
    // Phase 3: 超调与震荡 (Overshoot) - 测试符号翻转处理
    {1000.0, 1050.0, "Overshoot"},
    {1000.0, 1020.0, "Correction"},
    {1000.0, 980.0, "Undershoot"},
    {1000.0, 1010.0, "Correction"},

    // Phase 4: 高频噪声注入 (High Freq Noise) - 测试 D 项微分计算压力
    // 目标值不变，测量值剧烈跳动，模拟传感器故障或恶劣工况
    {1000.0, 1000.0 + 50.5, "Noise_High"},
    {1000.0, 1000.0 - 45.2, "Noise_Low"},
    {1000.0, 1000.0 + 60.1, "Noise_High"},
    {1000.0, 1000.0 - 55.8, "Noise_Low"},
    {1000.0, 1000.0 + 40.2, "Noise_High"},
    {1000.0, 1000.0 - 30.5, "Noise_Low"},
    {1000.0, 1000.0 + 20.1, "Noise_High"},
    {1000.0, 1000.0 - 10.5, "Noise_Low"},

    // Phase 5: 积分饱和测试 (Integral Windup)
    // 保持恒定误差，迫使 I 项累加到可能的溢出边缘
    {2000.0, 1000.0, "Saturation_Test"}, // Error = 1000
    {2000.0, 1000.0, "Saturation_Test"},
    {2000.0, 1000.0, "Saturation_Test"},
    {2000.0, 1000.0, "Saturation_Test"},
    {2000.0, 1000.0, "Saturation_Test"},
    
    // Phase 6: 反向全速突变 (Reverse) - 测试大动态范围
    {-1000.0, 1000.0, "Reverse_Shock"}, // Error = -2000
    {-1000.0, 500.0,  "Reverse_Fall"},
    {-1000.0, 0.0,    "Reverse_Zero"},
    {-1000.0, -500.0, "Reverse_Neg"},
    {-1000.0, -900.0, "Reverse_Near"},
    
    // ... 填充剩余数据以维持 Loop
    {0.0, 0.0, "End"} 
};

// 简单的辅助函数，防止数组越界
static void get_synth_point(int index, double *tgt, double *meas) {
    int safe_idx = index % (sizeof(g_synth_data)/sizeof(g_synth_data[0]));
    // 如果碰到了 "End" 标记，回到开头 (模拟 continuous loop)
    if (safe_idx >= 27) safe_idx = safe_idx % 27; 
    
    *tgt = g_synth_data[safe_idx].target;
    *meas = g_synth_data[safe_idx].measured;
}

#endif