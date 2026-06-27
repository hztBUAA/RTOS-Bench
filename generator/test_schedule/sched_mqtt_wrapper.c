/**
 * @file sched_mqtt_wrapper.c
 * @brief MQTT bounded offline wrapper for test-schedule.
 * @details Encodes real MQTT CONNECT/PUBLISH packets from GeoLife records using
 *          Mongoose, but does not connect to an external broker.
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../../workloads/MQTT/geolife.h"
#include "../../workloads/MQTT/mongoose.h"

#include "sched_workloads.h"

#define SCHED_MQTT_MAX_MESSAGES 512
#define SCHED_MQTT_TOPIC        "car/tracker/location"

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

static uint64_t sched_mqtt_get_time_us(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

int sched_mqtt_init(void)
{
	return 0;
}

int sched_mqtt_quick_exec(void)
{
	struct mg_connection c;
	struct mg_mqtt_opts opts;
	uint64_t start_time;
	uint64_t end_time;
	uint32_t checksum = 0;

	memset(&c, 0, sizeof(c));
	memset(&opts, 0, sizeof(opts));
	mg_log_set(0);
	if (!mg_iobuf_init(&c.send, 0, 64)) {
		return -1;
	}

	c.id = 1;
	c.is_client = 1;
	opts.client_id = mg_str("rtos_bench_sched");
	opts.keepalive = 60;
	opts.clean = true;
	mg_mqtt_login(&c, &opts);

	start_time = sched_mqtt_get_time_us();
	for (int i = 0; i < SCHED_MQTT_MAX_MESSAGES; i++) {
		GeoLifeRecord next_point = g_geolife_track[i % GEOLIFE_COUNT];
		char json_payload[128];
		size_t before;
		uint16_t packet_id;

		memset(&opts, 0, sizeof(opts));
		opts.topic = mg_str(SCHED_MQTT_TOPIC);
		opts.qos = 1;
		opts.retransmit_id = (uint16_t)(i + 1);
		snprintf(json_payload, sizeof(json_payload),
			 "{\"lat\":%.6f,\"lon\":%.6f,\"alt\":%.1f,\"ts\":%u}",
			 next_point.lat, next_point.lon, next_point.alt, next_point.ts);
		opts.message = mg_str(json_payload);

		before = c.send.len;
		packet_id = mg_mqtt_pub(&c, &opts);
		if (packet_id == 0 || c.send.len <= before) {
			mg_iobuf_free(&c.send);
			return -1;
		}
		for (size_t j = before; j < c.send.len; j++) {
			checksum = (checksum * 33u) ^ c.send.buf[j];
		}
	}
	end_time = sched_mqtt_get_time_us();

	sched_mqtt_printf("[SCHED-MQTT] offline packets=%d bytes=%lu checksum=%lu total_time=%.3f ms\n",
			  SCHED_MQTT_MAX_MESSAGES,
			  (unsigned long)c.send.len,
			  (unsigned long)checksum,
			  (double)(end_time - start_time) / 1000.0);
	mg_iobuf_free(&c.send);
	return checksum != 0 ? 0 : -1;
}

void sched_mqtt_teardown(void)
{
}
