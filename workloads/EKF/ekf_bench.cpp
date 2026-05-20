#ifdef RT_THREAD_PLATFORM
#include <rtthread.h>
#include <finsh.h>
#define EKF_HAVE_PTHREAD 0
#else
#define MSH_CMD_EXPORT(cmd, desc)
#define EKF_HAVE_PTHREAD 1
#endif
#include <cstdio>
#include <time.h>
#include <math.h>
#if EKF_HAVE_PTHREAD
#include <pthread.h>
#endif
#include "EKF/ekf.h"
#include "iris_gps.h"

static uint64_t get_time_ns(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

/* Defined in test_schedule.c — when nonzero, suppress printf output */
extern "C" { extern volatile int g_sched_suppress_output; }

/* Quiet-aware printf: skips output when running inside test-schedule */
#define EKF_PRINTF(...) do { if (!g_sched_suppress_output) printf(__VA_ARGS__); } while(0)

void print_quat(const Quatf& q, uint64_t time) {
    Eulerf euler(q);
    double t = (double)time / 1000000.0;
    float roll = math::degrees(euler(0));
    float pitch = math::degrees(euler(1));
    float yaw = math::degrees(euler(2));

    // 使用 %.2f 打印浮点数
    EKF_PRINTF("T: %.3fs | R: %.2f P: %.2f Y: %.2f\n", t, roll, pitch, yaw);
}

extern "C" int ekf_bench_run(void) {
    /* Output suppressed to avoid affecting performance measurements */
    // EKF_PRINTF("--- EKF Benchmark Test Start ---\n");

    Ekf _ekf;

    _ekf.init(iris_gps_imu[0].time_us);

    int mag_idx = 0;
    int baro_idx = 0;
    int gps_idx = 0;
    
    EKF_PRINTF("Processing...\n");
    
    int update_success_count = 0; // 记录成功更新的次数

    /* Start timing */
    uint64_t start_time = get_time_ns();

    for (int i = 0; i < iris_gps_imu_count; i++) {
        // 1. 填充 IMU
        imuSample imu_sample{};
        imu_sample.time_us = iris_gps_imu[i].time_us;
        // 计算 dt (除了第一帧)
        float dt = (i > 0) ? (iris_gps_imu[i].time_us - iris_gps_imu[i-1].time_us) * 1e-6f : 0.004f;
        if (dt <= 0) dt = 0.004f; 
        
        imu_sample.delta_ang_dt = dt;
        imu_sample.delta_vel_dt = dt;
        
        // 原始数据通常是速率(rad/s)和加速度(m/s^2)，需要乘 dt 得到 delta
        for(int k=0; k<3; k++) {
            imu_sample.delta_ang(k) = iris_gps_imu[i].gyro[k] * dt;
            imu_sample.delta_vel(k) = iris_gps_imu[i].accel[k] * dt;
        }
        _ekf.setIMUData(imu_sample);

        // 2. 检查并填充 Mag
        while (mag_idx < iris_gps_mag_count && iris_gps_mag[mag_idx].time_us <= imu_sample.time_us) {
            magSample mag{};
            mag.time_us = iris_gps_mag[mag_idx].time_us;
            mag.mag(0) = iris_gps_mag[mag_idx].mag[0];
            mag.mag(1) = iris_gps_mag[mag_idx].mag[1];
            mag.mag(2) = iris_gps_mag[mag_idx].mag[2];
            _ekf.setMagData(mag);
            mag_idx++;
        }

        // 3. 检查并填充 Baro
        while (baro_idx < iris_gps_baro_count && iris_gps_baro[baro_idx].time_us <= imu_sample.time_us) {
            _ekf.setBaroData(baroSample{iris_gps_baro[baro_idx].time_us, iris_gps_baro[baro_idx].hgt});
            baro_idx++;
        }
        
        // 4. 检查并填充 GPS
        while (gps_idx < iris_gps_gps_count && iris_gps_gps[gps_idx].time_us <= imu_sample.time_us) {
            gps_message gps{};
            gps.time_usec = iris_gps_gps[gps_idx].time_us;
            gps.lat = (int)(iris_gps_gps[gps_idx].lat * 1e7); // 注意单位转换
            gps.lon = (int)(iris_gps_gps[gps_idx].lon * 1e7);
            gps.alt = (int)(iris_gps_gps[gps_idx].alt * 1000); // m -> mm
            const float vn = iris_gps_gps[gps_idx].vel[0];
            const float ve = iris_gps_gps[gps_idx].vel[1];
            const float vd = iris_gps_gps[gps_idx].vel[2];
            gps.vel_ned(0) = vn;
            gps.vel_ned(1) = ve;
            gps.vel_ned(2) = vd;
            gps.vel_m_s = sqrtf(vn * vn + ve * ve + vd * vd);
            gps.vel_ned_valid = true;
            gps.fix_type = 3;
            gps.eph = 1.0f;
            gps.epv = 1.5f;
            gps.sacc = 0.5f;
            gps.nsats = 10;
            gps.pdop = 1.5f;
            gps.yaw = NAN;
            gps.yaw_offset = NAN;
            _ekf.setGpsData(gps);
            gps_idx++;
        }

        if (_ekf.update()) {
            update_success_count++;

            /* Output suppressed to avoid affecting performance measurements */
            // 每成功更新 50 次打印一次
            // if (update_success_count % 200 == 0) {
            //     if (update_success_count % 50 == 0) {
            //         Quatf q = _ekf.calculate_quaternion();
            //
            //         // 获取位置 (NED坐标系: North, East, Down)
            //         // getPosition() 返回的是 float[3]
            //         Vector3f pos = _ekf.getPosition();
            //
            //         // 获取速度
            //         Vector3f vel = _ekf.getVelocity();
            //
            //         print_quat(q, imu_sample.time_us);
            //         EKF_PRINTF("   Pos: N=%.2f E=%.2f D=%.2f | Vel: N=%.2f E=%.2f D=%.2f\n",
            //                (double)pos(0), (double)pos(1), (double)pos(2),
            //                (double)vel(0), (double)vel(1), (double)vel(2));
            //     }
            // }
        }
    }

    /* End timing */
    uint64_t end_time = get_time_ns();
    uint64_t total_ns = end_time - start_time;
    double avg_ns = (update_success_count > 0) ? (double)total_ns / update_success_count : 0.0;

    /* Output suppressed to avoid affecting performance measurements */
    // EKF_PRINTF("--- EKF Test Finished ---\n");
    // EKF_PRINTF("Total Successful Updates: %d\n", update_success_count);

    /* Print timing results (only in non-quiet mode) */
    EKF_PRINTF("[EKF] samples=%d total_time=%.3f ms avg_latency=%.3f us/update\n",
               update_success_count,
               (double)total_ns / 1000000.0,
               avg_ns / 1000.0);

    return 0;
}

static void* ekf_thread_entry(void *parameter) {
    (void)parameter;
    ekf_bench_run();
    return nullptr;
}


/* 导出命令到 MSH（仅在具备 pthread 的主机侧调试时使用） */
extern "C" int ekf_test(void) {
    pthread_t tid;
    pthread_attr_t attr;
    struct sched_param param;
    int ret;

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 32 * 1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 25;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&tid, &attr, ekf_thread_entry, NULL);
    if (ret == 0) {
        printf("EKF simulation thread created successfully (pthread).\n");
        pthread_join(tid, NULL);
    } else {
        printf("Failed to create EKF simulation thread! Error code: %d\n", ret);
    }
    pthread_attr_destroy(&attr);
    return 0;
}
MSH_CMD_EXPORT(ekf_test, Run EKF benchmark);
