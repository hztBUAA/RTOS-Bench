/**
 * @file sched_mqtt_wrapper.c
 * @brief MQTT quick-execution wrapper for test-schedule
 *
 * test-schedule needs deterministic, bounded workload execution inside
 * periodic worker threads. The standalone MQTT workload still exercises the
 * real broker/network path; this schedule wrapper keeps the MQTT data-shaping
 * work (GeoLife sample -> JSON payload -> MQTT publish frame) but avoids
 * external broker state and platform-specific poll() behavior.
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../../workloads/MQTT/geolife.h"
#include "sched_workloads.h"

#define SCHED_MQTT_MAX_MESSAGES 2000
#define SCHED_MQTT_TOPIC "car/tracker/location"

extern volatile int g_sched_suppress_output;

static volatile uint32_t s_mqtt_checksum;

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

static uint32_t sched_mqtt_checksum(const unsigned char *buf, size_t len)
{
	uint32_t hash = 2166136261u;
	for (size_t i = 0; i < len; i++) {
		hash ^= buf[i];
		hash *= 16777619u;
	}
	return hash;
}

static size_t sched_mqtt_encode_remaining_length(unsigned char *out, size_t value)
{
	size_t n = 0;
	do {
		unsigned char byte = (unsigned char)(value % 128);
		value /= 128;
		if (value > 0) {
			byte |= 0x80;
		}
		out[n++] = byte;
	} while (value > 0 && n < 4);
	return n;
}

static size_t sched_mqtt_append_char(char *buf, size_t off, size_t cap, char ch)
{
	if (off + 1 < cap) {
		buf[off] = ch;
		return off + 1;
	}
	return off;
}

static size_t sched_mqtt_append_str(char *buf, size_t off, size_t cap,
				    const char *str)
{
	while (*str) {
		off = sched_mqtt_append_char(buf, off, cap, *str++);
	}
	return off;
}

static size_t sched_mqtt_append_u64(char *buf, size_t off, size_t cap,
				    uint64_t value)
{
	char tmp[20];
	size_t n = 0;

	do {
		tmp[n++] = (char)('0' + (value % 10));
		value /= 10;
	} while (value > 0 && n < sizeof(tmp));

	while (n > 0) {
		off = sched_mqtt_append_char(buf, off, cap, tmp[--n]);
	}
	return off;
}

static size_t sched_mqtt_append_fixed(char *buf, size_t off, size_t cap,
				      double value, uint32_t scale,
				      unsigned decimals)
{
	int negative = value < 0.0;
	double scaled_d = value * (double)scale;
	uint64_t scaled;

	if (negative) {
		scaled_d = -scaled_d;
		off = sched_mqtt_append_char(buf, off, cap, '-');
	}

	scaled = (uint64_t)(scaled_d + 0.5);
	off = sched_mqtt_append_u64(buf, off, cap, scaled / scale);

	if (decimals > 0) {
		uint64_t frac = scaled % scale;
		uint64_t divisor = scale / 10;
		off = sched_mqtt_append_char(buf, off, cap, '.');
		while (decimals-- > 0 && divisor > 0) {
			off = sched_mqtt_append_char(buf, off, cap,
						(char)('0' + (frac / divisor) % 10));
			divisor /= 10;
		}
	}

	return off;
}

static size_t sched_mqtt_build_payload(const GeoLifeRecord *point,
				       char *payload, size_t payload_len)
{
	size_t off = 0;

	off = sched_mqtt_append_str(payload, off, payload_len, "{\"lat\":");
	off = sched_mqtt_append_fixed(payload, off, payload_len, point->lat,
				       1000000u, 6);
	off = sched_mqtt_append_str(payload, off, payload_len, ",\"lon\":");
	off = sched_mqtt_append_fixed(payload, off, payload_len, point->lon,
				       1000000u, 6);
	off = sched_mqtt_append_str(payload, off, payload_len, ",\"alt\":");
	off = sched_mqtt_append_fixed(payload, off, payload_len,
				       (double)point->alt, 10u, 1);
	off = sched_mqtt_append_str(payload, off, payload_len, ",\"ts\":");
	off = sched_mqtt_append_u64(payload, off, payload_len, point->ts);
	off = sched_mqtt_append_char(payload, off, payload_len, '}');

	if (off >= payload_len) {
		payload[payload_len - 1] = '\0';
		return 0;
	}
	payload[off] = '\0';
	return off;
}

static size_t sched_mqtt_build_publish_frame(const GeoLifeRecord *point,
					     unsigned char *frame,
					     size_t frame_len)
{
	char payload[128];
	const size_t topic_len = strlen(SCHED_MQTT_TOPIC);
	size_t payload_len = sched_mqtt_build_payload(point, payload,
						      sizeof(payload));
	if (payload_len == 0) return 0;

	size_t remain_len = 2 + topic_len + payload_len;
	size_t off = 0;
	if (frame_len < 1 + 4 + remain_len) {
		return 0;
	}

	frame[off++] = 0x30; /* MQTT PUBLISH, QoS 0 */
	off += sched_mqtt_encode_remaining_length(frame + off, remain_len);
	frame[off++] = (unsigned char)((topic_len >> 8) & 0xff);
	frame[off++] = (unsigned char)(topic_len & 0xff);
	memcpy(frame + off, SCHED_MQTT_TOPIC, topic_len);
	off += topic_len;
	memcpy(frame + off, payload, payload_len);
	off += payload_len;
	return off;
}

int sched_mqtt_init(void)
{
	s_mqtt_checksum = 0;
	return 0;
}

int sched_mqtt_quick_exec(void)
{
	unsigned char frame[256];
	uint64_t start = sched_mqtt_get_time_us();
	uint32_t checksum = 0;
	int samples = 0;

	for (int i = 0; i < SCHED_MQTT_MAX_MESSAGES; i++) {
		const GeoLifeRecord *point = &g_geolife_track[i % GEOLIFE_COUNT];
		size_t len = sched_mqtt_build_publish_frame(point, frame, sizeof(frame));
		if (len == 0) {
			continue;
		}
		checksum ^= sched_mqtt_checksum(frame, len) + (uint32_t)i;
		samples++;
	}

	s_mqtt_checksum ^= checksum;
	uint64_t total_us = sched_mqtt_get_time_us() - start;
	if (samples > 0) {
		sched_mqtt_printf("[SCHED-MQTT] samples=%d total_time=%.3f ms avg=%.3f us/msg checksum=%lu\n",
				  samples, (double)total_us / 1000.0,
				  (double)total_us / (double)samples,
				  (unsigned long)s_mqtt_checksum);
	}
	return 0;
}

void sched_mqtt_teardown(void)
{
}
