#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#include <finsh.h>
#define MDB_HAVE_PTHREAD 1
#define MDB_HAVE_SOCKETS 1
#else
#define MSH_CMD_EXPORT(cmd, desc)
typedef unsigned int rt_uint32_t;
#define MDB_HAVE_PTHREAD 1
#define MDB_HAVE_SOCKETS 1
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* Standard printf for workload output */

#if MDB_HAVE_PTHREAD
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>     
#include <time.h>          
#include <sched.h>
#endif

#if MDB_HAVE_SOCKETS
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#define NANOMODBUS_IMPLEMENTATION
#include "nanomodbus.h"
#include "sim_plc.h"

// 本地测试服务器地址和端口
#define PORT 5020
#define SERVER_IP "127.0.0.1"
#define TEST_ROUNDS 5

#define THREAD_STACK_SIZE (16*1024)

#define SERVER_PRIORITY   19 
#define CLIENT_PRIORITY   20

static double diff_timespec_us(const struct timespec *start, const struct timespec *end) {
    double start_us = (double)start->tv_sec * 1000000.0 + (double)start->tv_nsec / 1000.0;
    double end_us   = (double)end->tv_sec   * 1000000.0 + (double)end->tv_nsec   / 1000.0;
    return end_us - start_us;
}

static void print_nmbs_error(nmbs_error err) {
    switch (err) {
        case NMBS_ERROR_NONE: printf("NONE"); break;
        case NMBS_ERROR_INVALID_ARGUMENT: printf("INVALID_ARGUMENT"); break;
        case NMBS_ERROR_TIMEOUT: printf("TIMEOUT"); break;
        case NMBS_ERROR_INVALID_TCP_MBAP: printf("INVALID_TCP_MBAP"); break;
        case NMBS_ERROR_INVALID_RESPONSE: printf("INVALID_RESPONSE"); break;
        case NMBS_ERROR_TRANSPORT: printf("TRANSPORT"); break;
        default: printf("UNKNOWN(%d)", err); break;
    }
}

int32_t transport_read(uint8_t* buf, uint16_t count, int32_t timeout_ms, void* arg) {
    int sockfd = (int)(intptr_t)arg;
    
    ssize_t n = recv(sockfd, buf, count, 0);
    
    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }
        return -1;
    }
    
    if (n == 0) {
        return -1;
    }
    
    return (int32_t)n;
}

int32_t transport_write(const uint8_t* buf, uint16_t count, int32_t timeout_ms, void* arg) {
    int sockfd = (int)(intptr_t)arg;
    ssize_t n = send(sockfd, buf, count, 0);
    
    if (n < 0) {
        return -1;
    }
    
    return (int32_t)n;
}

static volatile int g_server_stop = 0;

static void* server_thread_entry(void* parameter) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int opt = 1;

    plc_init();

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    
    struct timeval server_tv = {3, 0};
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &server_tv, sizeof(server_tv));

    /* Output suppressed to avoid affecting performance measurements */
    // printf("[Server] Listening on %d...\n", PORT);

    while (!g_server_stop) {
        client_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            sleep(1);
            continue;
        }

        /* Output suppressed to avoid affecting performance measurements */
        // printf("[Server] Client connected\n");

        int flag = 1;
        setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
        struct timeval tv = {5, 0};
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        nmbs_t nmbs;
        nmbs_platform_conf conf;
        nmbs_platform_conf_create(&conf);
        conf.transport = NMBS_TRANSPORT_TCP;
        conf.read = transport_read;
        conf.write = transport_write;
        conf.arg = (void*)(intptr_t)client_fd;

        nmbs_callbacks callbacks;
        nmbs_callbacks_create(&callbacks);
        callbacks.read_coils = cb_read_coils;
        callbacks.write_single_coil = cb_write_single_coil;
        callbacks.write_multiple_coils = cb_write_mult_coils;
        callbacks.read_holding_registers = cb_read_holding;
        callbacks.write_single_register = cb_write_single_reg;
        callbacks.write_multiple_registers = cb_write_mult_regs;

        nmbs_server_create(&nmbs, 1, &conf, &callbacks);
        /* Output suppressed to avoid affecting performance measurements */
        // printf("[Server] Modbus server created\n");

        int request_count = 0;

        while (1) {
            nmbs_error err = nmbs_server_poll(&nmbs);
            if (err != NMBS_ERROR_NONE) {
                if (err == NMBS_ERROR_TIMEOUT) {
                    sched_yield();
                    continue;
                }

                if (err == NMBS_ERROR_TRANSPORT) {
                    break;
                }
                
                /* Output suppressed to avoid affecting performance measurements */
                // printf("[Server] Poll error: ");
                // print_nmbs_error(err);
                // printf("\n");
                continue;
            }
            request_count++;
            plc_tick();
            // usleep(1000); // Optional: yield CPU
        }
        
        /* Output suppressed to avoid affecting performance measurements */
        // printf("[Server] Client disconnected (processed %d requests)\n", request_count);
        close(client_fd);
    }
    
    close(server_fd);
    return NULL;
}

static void* client_thread_entry(void* parameter) {
    int sock;
    struct sockaddr_in serv_addr;
    
    printf("[Client] Waiting for server...\n");
    sleep(1);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    int flag = 1;
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
    struct timeval tv = {2, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    printf("[Client] Connecting to %s:%d...\n", SERVER_IP, PORT);
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("[Client] Connect fail: %d\n", errno);
        close(sock);
        return NULL;
    }

    printf("[Client] Connected!\n");

    nmbs_t nmbs;
    nmbs_platform_conf conf;
    
    nmbs_platform_conf_create(&conf);
    conf.transport = NMBS_TRANSPORT_TCP;
    conf.read = transport_read;
    conf.write = transport_write;
    conf.arg = (void*)(intptr_t)sock;
    nmbs_client_create(&nmbs, &conf);
    nmbs_set_read_timeout(&nmbs, 100);
    printf("[Client] Modbus client created\n");

    uint16_t r_regs[10];
    uint16_t w_regs[10];
    nmbs_bitfield r_coils;

    int errors = 0;
    struct timespec start_time = {0, 0}; 
    struct timespec end_time = {0, 0};

    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    for (int i = 0; i < TEST_ROUNDS; i++) {
        for(int k = 0; k < 10; k++) {
            w_regs[k] = (uint16_t)(i + k);
        }

        if (nmbs_write_multiple_registers(&nmbs, 50, 10, w_regs) != NMBS_ERROR_NONE) {
            errors++; printf("E1");
        }

        if (nmbs_read_holding_registers(&nmbs, 50, 10, r_regs) != NMBS_ERROR_NONE) {
            errors++; printf("E2");
        }
        
        if (r_regs[0] != i || r_regs[9] != i + 9) {
            printf("[Err] Data Verify Fail! Exp: %d, Got: %d\n", i, r_regs[0]);
            errors++;
        }

        bool coil_val = (i % 2 == 0);
        if (nmbs_write_single_coil(&nmbs, 10, coil_val) != NMBS_ERROR_NONE) {
            errors++; printf("E3");
        }

        if (nmbs_read_coils(&nmbs, 10, 1, r_coils) != NMBS_ERROR_NONE) {
            errors++; printf("E4");
        }

        bool read_val = (r_coils[0] & 0x01) ? true : false;
        if (read_val != coil_val) {
            printf("[Err] Coil Verify Fail!\n");
            errors++;
        }
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double time_us = diff_timespec_us(&start_time, &end_time);
    double time_s = time_us / 1000000.0;

    double total_reqs = TEST_ROUNDS * 4.0;

    printf("\n=== R/W Benchmark Result ===\n");
    printf("Time:       %.3f s\n", time_s);
    printf("Requests:   %.0f (Write+Read)\n", total_reqs);
    printf("Errors:     %d\n", errors);
    printf("TPS:        %.2f\n", total_reqs / time_s);

    double avg_us = time_us / total_reqs;
    printf("[modbus] samples=%.0f total_time=%.3f ms avg_latency=%.3f us/request\n",
           total_reqs, time_us / 1000.0, avg_us);

    close(sock);

    printf("[Client] Finished\n");
    return NULL;
}

int modbus_test(int argc, char** argv) {
    pthread_t s_tid, c_tid;
    pthread_attr_t attr;
    int ret;
    
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);

    struct sched_param param;
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = SERVER_PRIORITY; 
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    g_server_stop = 0;

    /* Output suppressed to avoid affecting performance measurements */
    // printf("Creating Server thread...\n");
    ret = pthread_create(&s_tid, &attr, server_thread_entry, NULL);
    if (ret != 0) {
        printf("Error creating server thread: %d\n", ret);
        return -1;
    }
    // pthread_detach(s_tid);

    ret = pthread_create(&c_tid, &attr, client_thread_entry, NULL);
    if (ret != 0) {
        printf("Error creating client thread: %d\n", ret);
        return -1;
    }
    pthread_join(c_tid, NULL);
    g_server_stop = 1;
    pthread_join(s_tid, NULL);
    /* Output suppressed to avoid affecting performance measurements */
    // printf("[MODBUS] Server thread joined. Test complete.\n");

    pthread_attr_destroy(&attr);
    return 0;
}

MSH_CMD_EXPORT(modbus_test, Modbus TCP Benchmark);

int modbus_bench_run(void)
{
    /* Run the benchmark inline (no shell arguments) */
    return modbus_test(0, NULL);
}
