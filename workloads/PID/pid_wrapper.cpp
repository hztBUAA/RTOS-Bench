// pid_wrapper.cpp
#include "Arduino.h"
#include "PID_v1.h" // 原仓库的头文件

// 定义伪造时间
unsigned long mock_millis_counter = 0;

// PID 变量
double Setpoint, Input, Output;

// 初始化 PID: P=2, I=5, D=1
PID myPID(&Input, &Output, &Setpoint, 2, 5, 1, DIRECT);

extern "C" {
    // 初始化函数
    void PID_Init_Wrapper(double target_val, double kp, double ki, double kd) {
        Input = 0.0;          // 初始环境值
        Setpoint = target_val; // 设定目标
        
        myPID.SetTunings(kp, ki, kd);

        myPID.SetMode(AUTOMATIC);
        myPID.SetOutputLimits(0, 255); // 模拟 PWM 输出范围 (0-255)
        myPID.SetSampleTime(100);      // 设置采样周期 100ms
    }

    // 更新 PID 的反馈值 (Input)
    void PID_Set_Input(double val) {
        Input = val;
    }

    // 获取 PID 计算出的输出值 (Output)
    double PID_Get_Output(void) {
        return Output;
    }

    // 核心计算步骤
    int PID_Compute_Step(void) {
        // [关键技巧] 手动增加时间，欺骗 PID 库时间已经到了，强制它计算
        mock_millis_counter += 100; 
        
        // 调用原库计算
        return myPID.Compute(); 
    }
}