#ifndef SIM_PLC_H
#define SIM_PLC_H

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "nanomodbus.h" // 引用 nanomodbus 类型定义

// ============================================================================
// PLC 内存模型
// ============================================================================
typedef struct {
    // 0x: Coils (可读写位) - 128 bits
    uint8_t coils[16]; 

    // 4x: Holding Registers (可读写字) - 100 Words
    // 0-49:   线性数据区
    // 50-99:  读写测试区
    uint16_t regs[100];

} PLCMemoryMap;

static PLCMemoryMap g_plc;
static pthread_mutex_t plc_mutex = PTHREAD_MUTEX_INITIALIZER;

// ============================================================================
// PLC 内部逻辑 (Tick)
// ============================================================================
static void plc_init() {
    memset(&g_plc, 0, sizeof(g_plc));
    // 初始化一些固定数据
    for (int i = 0; i < 50; i++) g_plc.regs[i] = i;
}

// 模拟 PLC 内部运算：例如 Reg[0] 自增，模拟传感器变化
static void plc_tick() {
    pthread_mutex_lock(&plc_mutex);
    g_plc.regs[0]++; // 模拟心跳计数器
    pthread_mutex_unlock(&plc_mutex);
}

// ============================================================================
// Modbus 回调函数 (Server Side Implementation)
// ============================================================================

// --- 0x01 Read Coils ---
static nmbs_error cb_read_coils(uint16_t address, uint16_t quantity, uint8_t* coils_out, uint8_t unit_id, void* arg) {
    if (address + quantity > 128) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    pthread_mutex_lock(&plc_mutex);
    // 简单实现：逐位打包 (Bit Packing)
    // 实际生产中可用位运算优化，这里为了逻辑清晰使用循环
    memset(coils_out, 0, (quantity + 7) / 8);
    for (int i = 0; i < quantity; i++) {
        int bit_index = address + i;
        int byte_idx = bit_index / 8;
        int bit_pos  = bit_index % 8;
        
        if (g_plc.coils[byte_idx] & (1 << bit_pos)) {
            coils_out[i / 8] |= (1 << (i % 8));
        }
    }
    pthread_mutex_unlock(&plc_mutex);
    return NMBS_ERROR_NONE;
}

// --- 0x05 Write Single Coil ---
static nmbs_error cb_write_single_coil(uint16_t address, bool value, uint8_t unit_id, void* arg) {
    if (address >= 128) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    pthread_mutex_lock(&plc_mutex);
    int byte_idx = address / 8;
    int bit_pos  = address % 8;
    
    if (value) g_plc.coils[byte_idx] |= (1 << bit_pos);
    else       g_plc.coils[byte_idx] &= ~(1 << bit_pos);
    
    pthread_mutex_unlock(&plc_mutex);
    return NMBS_ERROR_NONE;
}

// --- 0x0F Write Multiple Coils ---
static nmbs_error cb_write_mult_coils(uint16_t address, uint16_t quantity, const uint8_t* coils_in, uint8_t unit_id, void* arg) {
    if (address + quantity > 128) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    pthread_mutex_lock(&plc_mutex);
    for (int i = 0; i < quantity; i++) {
        // 读取输入数据中的第 i 位
        bool val = (coils_in[i / 8] & (1 << (i % 8))) ? true : false;
        
        // 写入 PLC 内存中的 (address + i) 位
        int target_idx = address + i;
        int byte_idx = target_idx / 8;
        int bit_pos  = target_idx % 8;

        if (val) g_plc.coils[byte_idx] |= (1 << bit_pos);
        else     g_plc.coils[byte_idx] &= ~(1 << bit_pos);
    }
    pthread_mutex_unlock(&plc_mutex);
    return NMBS_ERROR_NONE;
}

// --- 0x03 Read Holding Registers ---
static nmbs_error cb_read_holding(uint16_t address, uint16_t quantity, uint16_t* registers_out, uint8_t unit_id, void* arg) {
    if (address + quantity > 100) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    pthread_mutex_lock(&plc_mutex);
    memcpy(registers_out, &g_plc.regs[address], quantity * 2);
    pthread_mutex_unlock(&plc_mutex);
    return NMBS_ERROR_NONE;
}

// --- 0x06 Write Single Register ---
static nmbs_error cb_write_single_reg(uint16_t address, uint16_t value, uint8_t unit_id, void* arg) {
    if (address >= 100) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    pthread_mutex_lock(&plc_mutex);
    g_plc.regs[address] = value;
    pthread_mutex_unlock(&plc_mutex);
    return NMBS_ERROR_NONE;
}

// --- 0x10 Write Multiple Registers ---
static nmbs_error cb_write_mult_regs(uint16_t address, uint16_t quantity, const uint16_t* registers, uint8_t unit_id, void* arg) {
    if (address + quantity > 100) return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    pthread_mutex_lock(&plc_mutex);
    memcpy(&g_plc.regs[address], registers, quantity * 2);
    pthread_mutex_unlock(&plc_mutex);
    return NMBS_ERROR_NONE;
}

#endif