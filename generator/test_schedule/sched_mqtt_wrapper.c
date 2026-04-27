/**
 * @file sched_mqtt_wrapper.c
 * @brief MQTT quick-execution wrapper for test-schedule
 * @details Implements a fast version of MQTT benchmark with limited message count
 *          for use in schedulability testing. Uses the same mongoose library but
 *          restricts the number of messages to keep WCET under 2 seconds.
 *
 * Key differences from original mqtt_bench.c:
 * - SCHED_MQTT_MAX_MESSAGES = 50 (vs original ~617 GEOLIFE_COUNT)
 * - Self-contained implementation that doesn't modify original code
 */

#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#define SCHED_MQTT_HAVE_SOCKETS 0
#else
#define SCHED_MQTT_HAVE_SOCKETS 1
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#if SCHED_MQTT_HAVE_SOCKETS
#include <pthread.h>
#include <sched.h>
#endif

/* Include mongoose and geolife from original workload */
#include "../../workloads/MQTT/mongoose.h"
#include "../../workloads/MQTT/geolife.h"

#include "sched_workloads.h"

/* Configuration for quick schedule mode */
#define SCHED_MQTT_MAX_MESSAGES   50   /* Much smaller than GEOLIFE_COUNT (~617) */
#define SCHED_MQTT_URL            "tcp://44.232.241.40:1883"
#define SCHED_MQTT_TOPIC          "car/tracker/location"

/* Suppress output during scheduler runs */
extern volatile int g_sched_suppress_output;

static inline int sched_mqtt_printf(const char *fmt, ...)
{
	if (g_sched_suppress_output) return 0;
	va_list ap;
	va_start(ap, fmt);
	int r = vprintf(fmt, ap);
	va_end(ap);
	return r;
}

/* Local state for this wrapper instance */
static int s_cursor = 0;
static uint64_t s_total_count = 0;
static int s_stop_flag = 0;
static int s_mqtt_ready = 0;
static int s_login_sent = 0;
static int s_tcp_connected = 0;
static uint64_t s_start_time = 0;
static int s_benchmark_started = 0;

static uint64_t sched_mqtt_get_time_us(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

static void sched_mqtt_callback(struct mg_connection *c, int ev, void *ev_data)
{
	if (ev == MG_EV_ERROR) {
		sched_mqtt_printf("[SCHED-MQTT] Error: %s\n", (char *)ev_data);
	}
	else if (ev == MG_EV_OPEN) {
		sched_mqtt_printf("[SCHED-MQTT] Socket Created\n");
		s_tcp_connected = 0;
	}
	else if (ev == MG_EV_CONNECT) {
		sched_mqtt_printf("[SCHED-MQTT] TCP Connected!\n");
		c->is_connecting = 0;
		s_tcp_connected = 1;
	}
	else if (ev == MG_EV_READ) {
		if (c->recv.len >= 4 && (unsigned char)c->recv.buf[0] == 0x20) {
			sched_mqtt_printf("[SCHED-MQTT] Received CONNACK!\n");
			s_mqtt_ready = 1;
			if (!s_benchmark_started) {
				s_start_time = sched_mqtt_get_time_us();
				s_benchmark_started = 1;
			}
			mg_iobuf_del(&c->recv, 0, c->recv.len);
		}
	}
	else if (ev == MG_EV_POLL) {
		if (c->id > 0) c->is_readable = 1;

		if (s_login_sent == 0) {
			if (c->id > 0 && s_tcp_connected == 1) {
				static const unsigned char raw_login[] = {
					0x10, 0x16,
					0x00, 0x04, 'M', 'Q', 'T', 'T',
					0x04, 0x02, 0x00, 0x3C,
					0x00, 0x0A,
					'r', 't', 'o', 's', '_', 'b', 'e', 'n', 'c', 'h'
				};

				sched_mqtt_printf("[SCHED-MQTT] Sending login (%d bytes)...\n",
						  (int)sizeof(raw_login));

				int sent = send((int)c->fd, raw_login, sizeof(raw_login), 0);
				if (sent > 0) {
					sched_mqtt_printf("[SCHED-MQTT] Login sent\n");
					s_login_sent = 1;
				}
			}
			return;
		}

		if (!s_mqtt_ready) return;
		if (s_stop_flag) return;
		if (c->is_draining) return;

		/* Key difference: use SCHED_MQTT_MAX_MESSAGES instead of GEOLIFE_COUNT */
		if (s_cursor >= SCHED_MQTT_MAX_MESSAGES) {
			s_stop_flag = 1;
			return;
		}

		GeoLifeRecord next_point = g_geolife_track[s_cursor % GEOLIFE_COUNT];
		s_cursor++;

		char json_payload[128];
		struct mg_mqtt_opts pub_opts;
		memset(&pub_opts, 0, sizeof(pub_opts));
		pub_opts.topic = mg_str(SCHED_MQTT_TOPIC);
		pub_opts.qos = 1;

		snprintf(json_payload, sizeof(json_payload),
			 "{\"lat\":%.6f,\"lon\":%.6f,\"alt\":%.1f,\"ts\":%u}",
			 next_point.lat, next_point.lon, next_point.alt, next_point.ts);

		pub_opts.message = mg_str(json_payload);
		mg_mqtt_pub(c, &pub_opts);

		if (c->send.len > 0) {
			c->is_writable = 1;
		}

		s_total_count++;
	}
}

static void *sched_mqtt_thread_entry(void *parameter)
{
	struct mg_mgr mgr;

	/* Reset all state for fresh run */
	s_cursor = 0;
	s_total_count = 0;
	s_stop_flag = 0;
	s_mqtt_ready = 0;
	s_login_sent = 0;
	s_tcp_connected = 0;
	s_start_time = 0;
	s_benchmark_started = 0;

	sched_mqtt_printf("[SCHED-MQTT] Quick mode started (max %d msgs)...\n",
			  SCHED_MQTT_MAX_MESSAGES);

	mg_mgr_init(&mgr);
	mg_log_set(0);

	sched_mqtt_printf("[SCHED-MQTT] Connecting to %s...\n", SCHED_MQTT_URL);
	struct mg_connection *c = mg_connect(&mgr, SCHED_MQTT_URL,
					     sched_mqtt_callback, NULL);

	if (c == NULL) {
		sched_mqtt_printf("[SCHED-MQTT] Connection failed\n");
		return NULL;
	}

	while (s_stop_flag == 0) {
		mg_mgr_poll(&mgr, 20);
	}

	uint64_t end_time = sched_mqtt_get_time_us();

	if (s_total_count > 0 && s_benchmark_started) {
		uint64_t total_us = end_time - s_start_time;
		double avg_us = (double)total_us / s_total_count;
		sched_mqtt_printf("[SCHED-MQTT] samples=%lu total_time=%.3f ms avg=%.3f us/msg\n",
				  (unsigned long)s_total_count,
				  (double)total_us / 1000.0, avg_us);
	}

	mg_mgr_free(&mgr);
	return NULL;
}

/* ============================================================================
 * Public API for sched_workloads
 * ============================================================================ */

int sched_mqtt_init(void)
{
	/* No persistent state to initialize */
	return 0;
}

int sched_mqtt_quick_exec(void)
{
	/* Run synchronously without spawning a thread */
	sched_mqtt_thread_entry(NULL);
	return 0;
}

void sched_mqtt_teardown(void)
{
	/* Mongoose cleans up in thread_entry, nothing persistent */
}
