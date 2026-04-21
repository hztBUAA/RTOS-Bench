#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#include <finsh.h>
#define MQTT_HAVE_PTHREAD 0
#define MQTT_HAVE_SOCKETS 0
#else
#define MSH_CMD_EXPORT(cmd, desc)
#define MQTT_HAVE_PTHREAD 1
#define MQTT_HAVE_SOCKETS 1
#endif
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Defined in test_schedule.c — when nonzero, suppress printf output */
extern volatile int g_sched_suppress_output;
static inline int _mqtt_printf(const char *fmt, ...) {
	if (g_sched_suppress_output) return 0;
	va_list ap; va_start(ap, fmt); int r = vprintf(fmt, ap); va_end(ap); return r;
}
#define printf(...) _mqtt_printf(__VA_ARGS__)
#if MQTT_HAVE_PTHREAD
#include <pthread.h>
#include <sched.h>
#endif

#include "mongoose.h"
#include "geolife.h"

// ================= 配置区域 =================
#define MQTT_URL "tcp://44.232.241.40:1883"
#define TOPIC_DATA "car/tracker/location"
#define PUB_INTERVAL_MS 2000

// 线程配置
#define THREAD_PRIORITY         20
#define THREAD_STACK_SIZE       (32 * 1024) 
#define THREAD_TIMESLICE        10

static uint64_t get_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static int g_cursor = 0;
static uint64_t g_total_pack_send_us = 0;
static uint64_t g_total_count = 0;
static int g_stop_flag = 0;
static int g_mqtt_ready = 0;
static int g_login_sent = 0;
static int g_tcp_connected = 0;
static uint64_t g_start_time = 0;
static int g_benchmark_started = 0;

static void fn(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_ERROR) {
        printf("[MQTT] Error: %s\n", (char *)ev_data);
    }
    else if (ev == MG_EV_OPEN) {
        printf("[MQTT] Socket Created\n");
        g_tcp_connected = 0;
    } 
    else if (ev == MG_EV_CONNECT) {
        printf("[MQTT] TCP Connected! (Handshake Complete)\n");
        c->is_connecting = 0; 
        g_tcp_connected = 1;
    }
    else if (ev == MG_EV_READ) {
        if (c->recv.len >= 4 && (unsigned char)c->recv.buf[0] == 0x20) {
            printf("[MQTT] >>> SUCCESS: Received CONNACK! <<<\n");
            g_mqtt_ready = 1;
            /* Start timing when connection is ready (actual data transmission begins) */
            if (!g_benchmark_started) {
                g_start_time = get_time_us();
                g_benchmark_started = 1;
            }
            mg_iobuf_del(&c->recv, 0, c->recv.len);
        }
    }
    else if (ev == MG_EV_POLL) {
        if (g_login_sent == 0) {
            if (c->id > 0 && g_tcp_connected == 1) {
                
                static const unsigned char raw_login[] = {
                    0x10, 0x16,                         // Fixed Header
                    0x00, 0x04, 'M', 'Q', 'T', 'T',     // Protocol Name
                    0x04, 0x02, 0x00, 0x3C,             // Level, Flags, KeepAlive
                    0x00, 0x0A,                         // Client ID Len
                    'r', 't', 'o', 's', '_', 'b', 'e', 'n', 'c', 'h' // Client ID
                };
                
                printf("[MQTT] Sending RAW HEX Login (%d bytes)...\n", sizeof(raw_login));
                
                int sent = send((int)c->fd, raw_login, sizeof(raw_login), 0);
                
                if (sent > 0) {
                    printf("[MQTT] Login Sent success\n");
                    g_login_sent = 1;
                } else {
                    printf("[MQTT] Login Sent failed\n");
                }
            }
            return;
        }

        if (!g_mqtt_ready) return;
        if (g_stop_flag) return;
        if (c->is_draining) return; 

        if (g_cursor >= GEOLIFE_COUNT) { g_stop_flag = 1; return; }
        
        GeoLifeRecord next_point = g_geolife_track[g_cursor % GEOLIFE_COUNT];
        g_cursor++;

        char json_payload[128];
        struct mg_mqtt_opts pub_opts;
        memset(&pub_opts, 0, sizeof(pub_opts));
        pub_opts.topic = mg_str(TOPIC_DATA);
        pub_opts.qos = 1;

        snprintf(json_payload, sizeof(json_payload), 
                "{\"lat\":%.6f,\"lon\":%.6f,\"alt\":%.1f,\"ts\":%u}",
                next_point.lat, next_point.lon, next_point.alt, next_point.ts);
        
        pub_opts.message = mg_str(json_payload);
        mg_mqtt_pub(c, &pub_opts); 

        g_total_count++;
        // if (g_total_count % 100 == 0) printf("[Bench] Sent: %lu\n", (unsigned long)g_total_count);
    }
}

static void* mqtt_thread_entry(void *parameter) {
    struct mg_mgr mgr;

    /* Reset global state for fresh benchmark run */
    g_cursor = 0;
    g_total_count = 0;
    g_stop_flag = 0;
    g_mqtt_ready = 0;
    g_login_sent = 0;
    g_tcp_connected = 0;
    g_start_time = 0;
    g_benchmark_started = 0;

    printf("[MQTT] Thread Started...\n");

    mg_mgr_init(&mgr);
    mg_log_set(0);

    printf("[MQTT] Connecting to %s (Raw TCP Mode)...\n", MQTT_URL);
    struct mg_connection *c = mg_connect(&mgr, MQTT_URL, fn, NULL);
    
    if (c == NULL) {
        printf("[MQTT] Conn failed\n");
        return NULL;
    }

    while (g_stop_flag == 0) {
        mg_mgr_poll(&mgr, 20); 
    }

    uint64_t end_time = get_time_us();

    
    /* Unified format timing output */
    if (g_total_count > 0 && g_benchmark_started) {
        uint64_t total_us = end_time - g_start_time;
        double avg_us = (double)total_us / g_total_count;
        printf("\n====== Benchmark Finished ======\n");
        printf("Total Sent: %lu\n", (unsigned long)g_total_count);
        printf("[MQTT] samples=%lu total_time=%.3f ms avg_latency=%.3f us/msg\n",
               (unsigned long)g_total_count, (double)total_us / 1000.0, avg_us);
    }

    mg_mgr_free(&mgr);
    return NULL;
}

int mqtt_test(void) {
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

    printf("Starting MQTT Benchmark...\n");
    ret = pthread_create(&tid, &attr, mqtt_thread_entry, NULL);
    pthread_attr_destroy(&attr);
    
    if (ret != 0) return -1;
    pthread_join(tid, NULL);
    return 0;
}

MSH_CMD_EXPORT(mqtt_test, run MQTT benchmark);

int mqtt_bench_run(void)
{
    /* Run synchronously without spawning a detached thread */
    return mqtt_thread_entry(NULL) == NULL ? 0 : 0;
}