#include <cstdio>
#include <pthread.h>
#include "EKF/ekf.h"
#include "iris_gps.h"

void print_quat(const Quatf& q, uint64_t time) {
    Eulerf euler(q);
    double t = (double)time / 1000000.0;
    float roll = math::degrees(euler(0));
    float pitch = math::degrees(euler(1));
    float yaw = math::degrees(euler(2));
    
    printf("T: %.3fs | R: %.2f P: %.2f Y: %.2f\n", t, roll, pitch, yaw);
}

static void* ekf_thread_entry(void *parameter) {
    printf("--- EKF Benchmark Test Start ---\n");

    Ekf _ekf;

    _ekf.init(iris_gps_imu[0].time_us);
    
    int mag_idx = 0;
    int baro_idx = 0;
    int gps_idx = 0;
    
    printf("Processing...\n");
    
    int update_success_count = 0;

    for (int i = 0; i < iris_gps_imu_count; i++) {
        imuSample imu_sample;
        imu_sample.time_us = iris_gps_imu[i].time_us;
        float dt = (i > 0) ? (iris_gps_imu[i].time_us - iris_gps_imu[i-1].time_us) * 1e-6f : 0.004f;
        if (dt <= 0) dt = 0.004f; 
        
        imu_sample.delta_ang_dt = dt;
        imu_sample.delta_vel_dt = dt;
        
        for(int k=0; k<3; k++) {
            imu_sample.delta_ang(k) = iris_gps_imu[i].gyro[k] * dt;
            imu_sample.delta_vel(k) = iris_gps_imu[i].accel[k] * dt;
        }
        _ekf.setIMUData(imu_sample);

        while (mag_idx < iris_gps_mag_count && iris_gps_mag[mag_idx].time_us <= imu_sample.time_us) {
            magSample mag;
            mag.time_us = iris_gps_mag[mag_idx].time_us;
            mag.mag(0) = iris_gps_mag[mag_idx].mag[0];
            mag.mag(1) = iris_gps_mag[mag_idx].mag[1];
            mag.mag(2) = iris_gps_mag[mag_idx].mag[2];
            _ekf.setMagData(mag);
            mag_idx++;
        }

        while (baro_idx < iris_gps_baro_count && iris_gps_baro[baro_idx].time_us <= imu_sample.time_us) {
            _ekf.setBaroData(baroSample{iris_gps_baro[baro_idx].time_us, iris_gps_baro[baro_idx].hgt});
            baro_idx++;
        }
        
        while (gps_idx < iris_gps_gps_count && iris_gps_gps[gps_idx].time_us <= imu_sample.time_us) {
            gps_message gps;
            gps.time_usec = iris_gps_gps[gps_idx].time_us;
            gps.lat = (int)(iris_gps_gps[gps_idx].lat * 1e7); 
            gps.lon = (int)(iris_gps_gps[gps_idx].lon * 1e7);
            gps.alt = (int)(iris_gps_gps[gps_idx].alt * 1000);
            gps.vel_ned(0) = iris_gps_gps[gps_idx].vel[0];
            gps.vel_ned(1) = iris_gps_gps[gps_idx].vel[1];
            gps.vel_ned(2) = iris_gps_gps[gps_idx].vel[2];
            _ekf.setGpsData(gps);
            gps_idx++;
        }

        if (_ekf.update()) {
            update_success_count++;
            
            if (update_success_count % 200 == 0) {
                if (update_success_count % 50 == 0) {
                Quatf q = _ekf.calculate_quaternion();
                
                Vector3f pos = _ekf.getPosition(); 
                
                Vector3f vel = _ekf.getVelocity();

                print_quat(q, imu_sample.time_us);
                printf("   Pos: N=%.2f E=%.2f D=%.2f | Vel: N=%.2f E=%.2f D=%.2f\n", 
                       (double)pos(0), (double)pos(1), (double)pos(2),
                       (double)vel(0), (double)vel(1), (double)vel(2));
            }
            }
        }
    }

    printf("--- EKF Test Finished ---\n");
    printf("Total Successful Updates: %d\n", update_success_count);
    
    return nullptr;
}

extern "C" int ekf_test(int argc, char **argv) {
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
    
    pthread_attr_destroy(&attr);

    if (ret != 0) {
        printf("Failed to create EKF thread! Error code: %d\n", ret);
        return -1;
    }
    
    printf("EKF simulation thread created successfully (pthread).\n");

    pthread_join(tid, NULL);
    
    return 0;
}