#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#include <finsh.h>
#else
#define MSH_CMD_EXPORT(cmd, desc)
#define MSH_CMD_EXPORT_ALIAS(cmd, alias, desc)
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* POSIX */
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>

#include "fast.h" 
#include "include_imgs/dataset_registry.h"

static double diff_timespec_us(const struct timespec *start, const struct timespec *end) {
    double start_us = (double)start->tv_sec * 1000000.0 + (double)start->tv_nsec / 1000.0;
    double end_us   = (double)end->tv_sec   * 1000000.0 + (double)end->tv_nsec   / 1000.0;
    return end_us - start_us;
}

int fast_bench_run_once(int loops) {
    printf("[POSIX] Starting FAST Benchmark ...\n");
    printf("Total Images: %d\n", benchmark_suite_len);

    // 1. 准备内存
    // 为了模拟真实的图像处理流程，我们需要一块 RAM 区域作为"显存/帧缓冲区"
    // 我们先遍历一遍数据集，找到最大的分辨率，分配一次内存即可
    int max_buffer_size = 0;
    for (int i = 0; i < benchmark_suite_len; i++) {
        int sz = benchmark_suite[i].w * benchmark_suite[i].h;
        if (sz > max_buffer_size) max_buffer_size = sz;
    }

    printf("Allocating RAM buffer: %d bytes (KB: %d)\n", max_buffer_size, max_buffer_size/1024);

    unsigned char* img_buffer = (unsigned char*)malloc(max_buffer_size);
    if (!img_buffer) {
        printf("Error: Failed to allocate RAM buffer (OOM).\n");
        return -1;
    }

    // 打印表头
    printf("\n");
    printf("| %-15s | %-9s | %-8s | %-9s | %-7s |\n", "Image", "Size", "Corners", "Time(us)", "FPS");
    printf("|-----------------|-----------|----------|-----------|---------|\n");

    if (loops <= 0) {
        loops = 1000; // 默认循环次数
    }
    double bench_total_us = 0.0;

    for (int i = 0; i < benchmark_suite_len; i++) {
        const BenchmarkImage* img = &benchmark_suite[i];
        
        // A. 模拟采集：从 Flash (RO段) 拷贝到 RAM
        // 这一步模拟了 DMA 从摄像头搬运数据到内存的过程
        memcpy(img_buffer, img->data, img->w * img->h);
        
        int num_corners = 0;
        xy* corners = NULL;

        // C. 正式 Benchmark
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        for (int j = 0; j < loops; j++) {
            // 注意：fast9_detect 内部 malloc 了返回的 corners 数组
            // 必须 free，否则 1000 次循环会耗尽堆内存
            if (corners) free(corners); 
            
            // 核心算法调用
            corners = fast9_detect(img_buffer, img->w, img->h, img->w, 30, &num_corners);
        }

        clock_gettime(CLOCK_MONOTONIC, &end_time);

        // D. 清理最后一次的结果
        if (corners) free(corners);

        // E. 计算统计数据
        double total_us = diff_timespec_us(&start_time, &end_time);

        bench_total_us += total_us;

        double avg_us = total_us / loops;
        double fps = 1000000.0 / avg_us;

        // F. 输出表格行
        printf("| %-15s | %-4dx%-4d | %-8d | %9.1f | %7.1f |\n", 
               img->name, img->w, img->h, num_corners, avg_us, fps);
    }

    printf("|-----------------|-----------|----------|-----------|---------|\n");
    
    printf("\n[Result] Total Time: %.3f s\n", bench_total_us / 1000000.0);
    
    // 3. 释放全局缓冲区
    free(img_buffer);
    printf("[POSIX] Benchmark Finished.\n");
    return 0;
}

// 这是实际的测试线程入口函数
void fast_bench_entry(void* parameter) {
    int loops = parameter ? *((int*)parameter) : 0;
    fast_bench_run_once(loops);
}

void* fast_bench_pthread_entry(void* parameter) {
    fast_bench_entry(parameter); // 调用原来的逻辑
    return NULL;
}

// MSH 导出命令函数
void fast_test(void) {
    pthread_t tid;
    pthread_attr_t attr;
    int ret;
    
    /* 初始化线程属性 */
    pthread_attr_init(&attr);
    
    pthread_attr_setstacksize(&attr, 64 * 1024); 
    
    /* 设置优先级 (可选) */
    struct sched_param param;
    param.sched_priority = 20; // 对应 RT-Thread 的优先级
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    printf("Creating POSIX thread for FAST benchmark...\n");

    ret = pthread_create(&tid, &attr, fast_bench_pthread_entry, NULL);
    
    pthread_attr_destroy(&attr);

    if (ret != 0) {
        printf("Failed to create pthread. Error: %d\n", ret);
    } else {
        pthread_detach(tid); 
    }
}
// 导出到 MSH 控制台
MSH_CMD_EXPORT(fast_test, Run FAST benchmark);
