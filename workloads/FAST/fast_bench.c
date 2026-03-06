#include <string.h>
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

void fast_bench_entry(void* parameter) {
    printf("[POSIX] Starting FAST Benchmark ...\n");
    printf("Total Images: %d\n", benchmark_suite_len);

    int max_buffer_size = 0;
    for (int i = 0; i < benchmark_suite_len; i++) {
        int sz = benchmark_suite[i].w * benchmark_suite[i].h;
        if (sz > max_buffer_size) max_buffer_size = sz;
    }

    printf("Allocating RAM buffer: %d bytes (KB: %d)\n", max_buffer_size, max_buffer_size/1024);

    unsigned char* img_buffer = (unsigned char*)malloc(max_buffer_size);
    if (!img_buffer) {
        printf("Error: Failed to allocate RAM buffer (OOM).\n");
        return;
    }

    printf("\n");
    printf("| %-15s | %-9s | %-8s | %-9s | %-7s |\n", "Image", "Size", "Corners", "Time(us)", "FPS");
    printf("|-----------------|-----------|----------|-----------|---------|\n");

    int loops = 1000;
    double bench_total_us = 0.0;

    for (int i = 0; i < benchmark_suite_len; i++) {
        const BenchmarkImage* img = &benchmark_suite[i];
        
        memcpy(img_buffer, img->data, img->w * img->h);
        
        int num_corners = 0;
        xy* corners = NULL;

        struct timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);

        for (int j = 0; j < loops; j++) {
            if (corners) free(corners); 
            
            corners = fast9_detect(img_buffer, img->w, img->h, img->w, 30, &num_corners);
        }

        clock_gettime(CLOCK_MONOTONIC, &end_time);

        if (corners) free(corners);

        double total_us = diff_timespec_us(&start_time, &end_time);

        bench_total_us += total_us;

        double avg_us = total_us / loops;
        double fps = 1000000.0 / avg_us;

        printf("| %-15s | %-4dx%-4d | %-8d | %9.1f | %7.1f |\n", 
               img->name, img->w, img->h, num_corners, avg_us, fps);
    }

    printf("|-----------------|-----------|----------|-----------|---------|\n");
    
    printf("\n[Result] Total Time: %.3f s\n", bench_total_us / 1000000.0);
    
    free(img_buffer);
    printf("[POSIX] Benchmark Finished.\n");
}

void* fast_bench_pthread_entry(void* parameter) {
    fast_bench_entry(parameter);
    return NULL;
}

int fast_test(int argc, char** argv) {
    pthread_t tid;
    pthread_attr_t attr;
    int ret;
    
    pthread_attr_init(&attr);
    
    pthread_attr_setstacksize(&attr, 64 * 1024); 
    
    struct sched_param param;
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 20;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    printf("Creating POSIX thread for FAST benchmark...\n");

    ret = pthread_create(&tid, &attr, fast_bench_pthread_entry, NULL);
    
    pthread_attr_destroy(&attr);

    if (ret != 0) {
        printf("Failed to create pthread. Error: %d\n", ret);
        return -1;
    }
    
    // Wait for thread to complete
    pthread_join(tid, NULL);
    return 0;
}
