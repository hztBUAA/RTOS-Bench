#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#include <finsh.h>
#else
#define MSH_CMD_EXPORT(cmd, desc)
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

#include "mongoose.h"
#include "geolife.h"

// ================= 配置区域 =================
#define MQTT_URL "mqtt://broker.emqx.io:1883"
#define TOPIC_DATA "car/tracker/location"
#define PUB_INTERVAL_MS 2000

// 线程配置
#define THREAD_PRIORITY         20
#define THREAD_STACK_SIZE       (8 * 1024) 
#define THREAD_TIMESLICE        10

static uint64_t get_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static int g_cursor = 0;
static uint64_t g_total_pack_send_us = 0; // 累积耗时
static uint64_t g_total_count = 0;        // 发送计数
static int g_stop_flag = 0;

static void fn(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_OPEN) {
        printf("[MQTT] Network Connected\n");
    } 
    else if (ev == MG_EV_ERROR) {
        printf("[MQTT] Connection Error: %s\n", (char *) ev_data);
    }
    else if (ev == MG_EV_MQTT_OPEN) {
        printf("[MQTT] Session Started (Broker Connected)\n");
    }
    else if (ev == MG_EV_POLL) {  
        if (c->is_draining) return;

        if (g_cursor >= GEOLIFE_COUNT) {
            g_stop_flag = 1; // 通知主循环退出
            return;
        }

        // 获取下一条数据
        GeoLifeRecord next_point = g_geolife_track[g_cursor % GEOLIFE_COUNT];
        g_cursor++;

        char json_payload[128];
        
        struct mg_mqtt_opts pub_opts;
        memset(&pub_opts, 0, sizeof(pub_opts));
        pub_opts.topic = mg_str(TOPIC_DATA);
        pub_opts.qos = 1;

        uint64_t t_start = get_time_us();

        // 序列化
        snprintf(json_payload, sizeof(json_payload), 
                "{\"lat\":%.6f,\"lon\":%.6f,\"alt\":%.1f,\"ts\":%u}",
                next_point.lat, 
                next_point.lon, 
                next_point.alt,
                next_point.ts); // 发送 Unix 时间戳
        pub_opts.message = mg_str(json_payload);
        // 发送
        mg_mqtt_pub(c, &pub_opts);
        uint64_t t_end = get_time_us();
        g_total_pack_send_us += (t_end - t_start);
        g_total_count++;
        // 每发送 100 条，输出一次平均性能
        if (g_total_count % 100 == 0) {
            uint64_t avg_us = g_total_pack_send_us / g_total_count;
            printf("[Bench] Sent: %lu | Avg Pack+Send Time: %lu us\n", 
                    (unsigned long)g_total_count, 
                    (unsigned long)avg_us);
        }
    }
}

static void* mqtt_thread_entry(void *parameter) {
    struct mg_mgr mgr;
    
    printf("[MQTT] Thread Started...\n");

    // 初始化 Mongoose
    mg_mgr_init(&mgr);
    mg_log_set(0);
    // 连接 MQTT Broker 
    struct mg_connection *c = mg_mqtt_connect(&mgr, MQTT_URL, NULL, fn, NULL);
    
    if (c == NULL) {
        printf("[MQTT] Create connection failed\n");
        mg_mgr_free(&mgr);
        return NULL;
    }

    uint64_t t_bench_start = get_time_us();

    while (g_stop_flag == 0) {
        mg_mgr_poll(&mgr, 1);
    }

    uint64_t t_bench_end = get_time_us();

    printf("\n\n====== Benchmark Finished ======\n");
    printf("Total Records:   %ld\n", GEOLIFE_COUNT);
    printf("Total Sent:      %lu\n", (unsigned long)g_total_count);
    
    double total_time_ms = (t_bench_end - t_bench_start) / 1000.0;
    printf("Total Duration:  %.2f ms\n", total_time_ms);

    if (g_total_count > 0) {
        uint64_t avg_us = g_total_pack_send_us / g_total_count;
        printf("Avg Processing:  %lu us/msg (Pack + Enqueue)\n", (unsigned long)avg_us);
    }
    printf("================================\n");

    mg_mgr_free(&mgr);
    return NULL;
}

int mqtt_test(int argc, char** argv) {
    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;
    int ret;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = THREAD_PRIORITY;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    printf("Starting MQTT Benchmark using pthread...\n");

    // 5. 创建线程
    ret = pthread_create(&tid, &attr, mqtt_thread_entry, NULL);

    pthread_attr_destroy(&attr);
    if (ret == 0) {
        pthread_detach(tid); 
        printf("MQTT thread created successfully.\n");
    } else {
        printf("Failed to create MQTT thread! Error code: %d\n", ret);
    }
    
    return 0;
}

MSH_CMD_EXPORT(mqtt_test, run MQTT benchmark);

int mqtt_bench_run(void)
{
    /* Run synchronously without spawning a detached thread */
    return mqtt_thread_entry(NULL) == NULL ? 0 : 0;
}
