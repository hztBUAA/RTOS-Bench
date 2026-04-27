/**
 * @file sched_modbus_wrapper.c
 * @brief MODBUS quick-execution wrapper for test-schedule
 * @details Implements a fast version of MODBUS benchmark with reduced timeouts
 *          for use in schedulability testing. Uses the same nanomodbus library
 *          but with shorter wait times to keep WCET under 2 seconds.
 *
 * Key differences from original modbus_bench.c:
 * - Client wait: 100ms instead of 1s
 * - Server accept timeout: 1s instead of 3s
 * - Error retry delay: 100ms instead of 1s
 * - Self-contained implementation that doesn't modify original code
 */

#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#define SCHED_MDB_HAVE_PTHREAD 1
#define SCHED_MDB_HAVE_SOCKETS 1
#else
typedef unsigned int rt_uint32_t;
#define SCHED_MDB_HAVE_PTHREAD 1
#define SCHED_MDB_HAVE_SOCKETS 1
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if SCHED_MDB_HAVE_PTHREAD
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sched.h>
#endif

#if SCHED_MDB_HAVE_SOCKETS
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

/* Include nanomodbus and sim_plc from original workload */
/* Note: NANOMODBUS_IMPLEMENTATION is defined in the original modbus_bench.c,
 * so we only include the header here for type definitions */
#include "../../workloads/MODBUS/nanomodbus.h"
#include "../../workloads/MODBUS/sim_plc.h"

#include "sched_workloads.h"

/* Suppress output during scheduler runs */
extern volatile int g_sched_suppress_output;

#define SCHED_MDB_PRINTF(...) do { if (!g_sched_suppress_output) printf(__VA_ARGS__); } while (0)

/* Quick mode configuration - reduced timeouts */
#define SCHED_MDB_PORT              5021  /* Different port to avoid conflict with original */
#define SCHED_MDB_SERVER_IP         "127.0.0.1"
#define SCHED_MDB_TEST_ROUNDS       1
#define SCHED_MDB_CLIENT_WAIT_US    100000   /* 100ms instead of 1s */
#define SCHED_MDB_SERVER_TIMEOUT_S  1        /* 1s instead of 3s */
#define SCHED_MDB_ERROR_RETRY_US    100000   /* 100ms instead of 1s */
#define SCHED_MDB_THREAD_STACK_SIZE (16*1024)

static volatile int s_server_stop = 0;

static double sched_mdb_diff_timespec_us(const struct timespec *start,
					 const struct timespec *end)
{
	double start_us = (double)start->tv_sec * 1000000.0 + (double)start->tv_nsec / 1000.0;
	double end_us = (double)end->tv_sec * 1000000.0 + (double)end->tv_nsec / 1000.0;
	return end_us - start_us;
}

static int32_t sched_mdb_transport_read(uint8_t *buf, uint16_t count,
					int32_t timeout_ms, void *arg)
{
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

static int32_t sched_mdb_transport_write(const uint8_t *buf, uint16_t count,
					 int32_t timeout_ms, void *arg)
{
	int sockfd = (int)(intptr_t)arg;
	ssize_t n = send(sockfd, buf, count, 0);

	if (n < 0) {
		return -1;
	}

	return (int32_t)n;
}

static void *sched_mdb_server_thread(void *parameter)
{
	int server_fd, client_fd;
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);
	int opt = 1;

	plc_init();

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(SCHED_MDB_PORT);
	bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	listen(server_fd, 3);

	/* Quick mode: shorter accept timeout */
	struct timeval server_tv = {SCHED_MDB_SERVER_TIMEOUT_S, 0};
	setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &server_tv, sizeof(server_tv));

	while (!s_server_stop) {
		client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
		if (client_fd < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			}
			/* Quick mode: shorter retry delay */
			usleep(SCHED_MDB_ERROR_RETRY_US);
			continue;
		}

		int flag = 1;
		setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
		struct timeval tv = {5, 0};
		setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

		nmbs_t nmbs;
		nmbs_platform_conf conf;
		nmbs_platform_conf_create(&conf);
		conf.transport = NMBS_TRANSPORT_TCP;
		conf.read = sched_mdb_transport_read;
		conf.write = sched_mdb_transport_write;
		conf.arg = (void *)(intptr_t)client_fd;

		nmbs_callbacks callbacks;
		nmbs_callbacks_create(&callbacks);
		callbacks.read_coils = cb_read_coils;
		callbacks.write_single_coil = cb_write_single_coil;
		callbacks.write_multiple_coils = cb_write_mult_coils;
		callbacks.read_holding_registers = cb_read_holding;
		callbacks.write_single_register = cb_write_single_reg;
		callbacks.write_multiple_registers = cb_write_mult_regs;

		nmbs_server_create(&nmbs, 1, &conf, &callbacks);

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
				continue;
			}
			plc_tick();
		}

		close(client_fd);
	}

	close(server_fd);
	return NULL;
}

static void *sched_mdb_client_thread(void *parameter)
{
	int sock;
	struct sockaddr_in serv_addr;

	SCHED_MDB_PRINTF("[SCHED-MODBUS] Client waiting...\n");
	/* Quick mode: shorter wait for server */
	usleep(SCHED_MDB_CLIENT_WAIT_US);

	sock = socket(AF_INET, SOCK_STREAM, 0);
	int flag = 1;
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
	struct timeval tv = {2, 0};
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SCHED_MDB_PORT);
	serv_addr.sin_addr.s_addr = inet_addr(SCHED_MDB_SERVER_IP);

	SCHED_MDB_PRINTF("[SCHED-MODBUS] Connecting to %s:%d...\n",
			 SCHED_MDB_SERVER_IP, SCHED_MDB_PORT);

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		SCHED_MDB_PRINTF("[SCHED-MODBUS] Connect failed: %d\n", errno);
		close(sock);
		return NULL;
	}

	SCHED_MDB_PRINTF("[SCHED-MODBUS] Connected!\n");

	nmbs_t nmbs;
	nmbs_platform_conf conf;
	nmbs_platform_conf_create(&conf);
	conf.transport = NMBS_TRANSPORT_TCP;
	conf.read = sched_mdb_transport_read;
	conf.write = sched_mdb_transport_write;
	conf.arg = (void *)(intptr_t)sock;
	nmbs_client_create(&nmbs, &conf);
	nmbs_set_read_timeout(&nmbs, 100);

	uint16_t r_regs[10];
	uint16_t w_regs[10];
	nmbs_bitfield r_coils;

	int errors = 0;
	struct timespec start_time = {0, 0};
	struct timespec end_time = {0, 0};

	clock_gettime(CLOCK_MONOTONIC, &start_time);

	for (int i = 0; i < SCHED_MDB_TEST_ROUNDS; i++) {
		for (int k = 0; k < 10; k++) {
			w_regs[k] = (uint16_t)(i + k);
		}

		if (nmbs_write_multiple_registers(&nmbs, 50, 10, w_regs) != NMBS_ERROR_NONE) {
			errors++;
		}

		if (nmbs_read_holding_registers(&nmbs, 50, 10, r_regs) != NMBS_ERROR_NONE) {
			errors++;
		}

		if (r_regs[0] != i || r_regs[9] != i + 9) {
			errors++;
		}

		bool coil_val = (i % 2 == 0);
		if (nmbs_write_single_coil(&nmbs, 10, coil_val) != NMBS_ERROR_NONE) {
			errors++;
		}

		if (nmbs_read_coils(&nmbs, 10, 1, r_coils) != NMBS_ERROR_NONE) {
			errors++;
		}

		bool read_val = (r_coils[0] & 0x01) ? true : false;
		if (read_val != coil_val) {
			errors++;
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &end_time);
	double time_us = sched_mdb_diff_timespec_us(&start_time, &end_time);
	double total_reqs = SCHED_MDB_TEST_ROUNDS * 4.0;

	SCHED_MDB_PRINTF("[SCHED-MODBUS] samples=%.0f total_time=%.3f ms errors=%d\n",
			 total_reqs, time_us / 1000.0, errors);

	close(sock);
	return NULL;
}

/* ============================================================================
 * Public API for sched_workloads
 * ============================================================================ */

int sched_modbus_init(void)
{
	/* No persistent state to initialize */
	return 0;
}

int sched_modbus_quick_exec(void)
{
	pthread_t s_tid, c_tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, SCHED_MDB_THREAD_STACK_SIZE);

	s_server_stop = 0;

	/* Create server thread */
	if (pthread_create(&s_tid, &attr, sched_mdb_server_thread, NULL) != 0) {
		SCHED_MDB_PRINTF("[SCHED-MODBUS] Server thread create failed\n");
		pthread_attr_destroy(&attr);
		return -1;
	}

	/* Create client thread */
	if (pthread_create(&c_tid, &attr, sched_mdb_client_thread, NULL) != 0) {
		SCHED_MDB_PRINTF("[SCHED-MODBUS] Client thread create failed\n");
		s_server_stop = 1;
		pthread_join(s_tid, NULL);
		pthread_attr_destroy(&attr);
		return -1;
	}

	/* Wait for client to complete */
	pthread_join(c_tid, NULL);

	/* Signal server to stop and wait */
	s_server_stop = 1;
	pthread_join(s_tid, NULL);

	pthread_attr_destroy(&attr);
	return 0;
}

void sched_modbus_teardown(void)
{
	/* All cleanup happens in quick_exec */
}
