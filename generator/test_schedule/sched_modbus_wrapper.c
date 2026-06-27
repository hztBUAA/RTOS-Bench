/**
 * @file sched_modbus_wrapper.c
 * @brief MODBUS bounded offline wrapper for test-schedule.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../../workloads/MODBUS/nanomodbus.h"
#include "../../workloads/MODBUS/sim_plc.h"

#include "sched_workloads.h"

extern volatile int g_sched_suppress_output;

#define SCHED_MDB_PRINTF(...) do { if (!g_sched_suppress_output) printf(__VA_ARGS__); } while (0)
#define SCHED_MDB_TEST_ROUNDS 2048

static double sched_mdb_diff_timespec_us(const struct timespec *start,
					 const struct timespec *end)
{
	double start_us = (double)start->tv_sec * 1000000.0 + (double)start->tv_nsec / 1000.0;
	double end_us = (double)end->tv_sec * 1000000.0 + (double)end->tv_nsec / 1000.0;
	return end_us - start_us;
}

static void sched_mdb_write_u16(uint8_t *buf, uint16_t v)
{
	buf[0] = (uint8_t)(v >> 8);
	buf[1] = (uint8_t)(v & 0xff);
}

static uint16_t sched_mdb_read_u16(const uint8_t *buf)
{
	return (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
}

static size_t sched_mdb_make_mbap(uint8_t *frame, uint16_t txid,
				  uint16_t pdu_len)
{
	sched_mdb_write_u16(&frame[0], txid);
	sched_mdb_write_u16(&frame[2], 0);
	sched_mdb_write_u16(&frame[4], (uint16_t)(pdu_len + 1));
	frame[6] = 1;
	return 7;
}

static int sched_mdb_roundtrip_once(uint16_t txid, uint16_t base)
{
	uint8_t request[64];
	uint8_t response[64];
	uint16_t w_regs[10];
	uint16_t r_regs[10];
	uint8_t coils[1] = {0};
	size_t off;

	for (int k = 0; k < 10; k++) {
		w_regs[k] = (uint16_t)(base + k);
	}

	off = sched_mdb_make_mbap(request, txid, 6 + 10 * 2);
	request[off++] = 0x10;
	sched_mdb_write_u16(&request[off], 50); off += 2;
	sched_mdb_write_u16(&request[off], 10); off += 2;
	request[off++] = 20;
	for (int k = 0; k < 10; k++) {
		sched_mdb_write_u16(&request[off], w_regs[k]);
		off += 2;
	}
	if (request[7] != 0x10 ||
	    cb_write_mult_regs(sched_mdb_read_u16(&request[8]),
			       sched_mdb_read_u16(&request[10]),
			       w_regs, request[6], NULL) != NMBS_ERROR_NONE) {
		return -1;
	}

	off = sched_mdb_make_mbap(response, txid, 5);
	response[off++] = 0x10;
	memcpy(&response[off], &request[8], 4);
	off += 4;

	off = sched_mdb_make_mbap(request, (uint16_t)(txid + 1), 5);
	request[off++] = 0x03;
	sched_mdb_write_u16(&request[off], 50); off += 2;
	sched_mdb_write_u16(&request[off], 10); off += 2;
	if (request[7] != 0x03 ||
	    cb_read_holding(sched_mdb_read_u16(&request[8]),
			    sched_mdb_read_u16(&request[10]),
			    r_regs, request[6], NULL) != NMBS_ERROR_NONE) {
		return -1;
	}
	off = sched_mdb_make_mbap(response, (uint16_t)(txid + 1), 2 + 10 * 2);
	response[off++] = 0x03;
	response[off++] = 20;
	for (int k = 0; k < 10; k++) {
		sched_mdb_write_u16(&response[off], r_regs[k]);
		off += 2;
	}
	if (sched_mdb_read_u16(&response[9]) != base ||
	    sched_mdb_read_u16(&response[27]) != (uint16_t)(base + 9)) {
		return -1;
	}

	off = sched_mdb_make_mbap(request, (uint16_t)(txid + 2), 5);
	request[off++] = 0x05;
	sched_mdb_write_u16(&request[off], 10); off += 2;
	sched_mdb_write_u16(&request[off], (base & 1) ? 0x0000 : 0xff00);
	if (request[7] != 0x05 ||
	    cb_write_single_coil(sched_mdb_read_u16(&request[8]),
				 sched_mdb_read_u16(&request[10]) == 0xff00,
				 request[6], NULL) != NMBS_ERROR_NONE) {
		return -1;
	}
	if (cb_read_coils(10, 1, coils, 1, NULL) != NMBS_ERROR_NONE) {
		return -1;
	}
	return ((coils[0] & 0x01) != 0) == ((base & 1) == 0) ? 0 : -1;
}

int sched_modbus_init(void)
{
	return 0;
}

int sched_modbus_quick_exec(void)
{
	struct timespec start_time = {0, 0};
	struct timespec end_time = {0, 0};
	int errors = 0;

	plc_init();
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	for (int i = 0; i < SCHED_MDB_TEST_ROUNDS; i++) {
		if (sched_mdb_roundtrip_once((uint16_t)(100 + i * 4),
					     (uint16_t)i) != 0) {
			errors++;
		}
		plc_tick();
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);

	double time_us = sched_mdb_diff_timespec_us(&start_time, &end_time);
	SCHED_MDB_PRINTF("[SCHED-MODBUS] offline frames=%d total_time=%.3f ms errors=%d\n",
			 SCHED_MDB_TEST_ROUNDS * 3, time_us / 1000.0, errors);
	return errors == 0 ? 0 : -1;
}

void sched_modbus_teardown(void)
{
}
