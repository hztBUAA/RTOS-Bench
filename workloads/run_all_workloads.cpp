#include <iostream>
#include <cstdio>
#include <cstring>

#include "../generator/workload_registry.h"

using namespace std;

extern "C" {
	int fast_test(void);
	int ekf_test(void);
	int epnp_test(void);
	int icp_test(void);
	int modbus_test(void);
	int mqtt_test(void);
	int pid_test(void);
	int cusum_bench_run(void);
	int ewma_bench_run(void);
	void rtosbench_register_rtos_workloads(void);
}
extern "C" long double rtbench_get_timestamp(void);
extern "C" int epnp_schedule_wcet_test(void) {
    static const int kEpnpWcetIterations = 50;
    const struct rtosbench_workload *wl = NULL;
    uint64_t max_duration_ns = 0;

    for (int i = 0; i < rtosbench_workload_count(); i++) {
        const struct rtosbench_workload *candidate = rtosbench_get_workload(i);
        if (candidate != NULL && candidate->name != NULL &&
            strcmp(candidate->name, "epnp") == 0) {
            wl = candidate;
            break;
        }
    }

    if (wl == NULL) {
        cout << "[EPNP-WCET] workload 'epnp' not found in registry." << endl;
        return -1;
    }

    if (wl->exec == NULL) {
        cout << "[EPNP-WCET] workload 'epnp' has no exec callback." << endl;
        return -1;
    }

    cout << "[EPNP-WCET] Starting schedule WCET style epnp repeat test ("
         << kEpnpWcetIterations << " iterations)..." << endl;

    if (wl->init != NULL) {
        wl->init(0, NULL);
    }

    for (int i = 0; i < kEpnpWcetIterations; i++) {
        long double start = rtbench_get_timestamp();
        wl->exec(0, NULL);
        long double end = rtbench_get_timestamp();
        uint64_t duration_ns =
            (uint64_t)((end - start) * 1000000000.0L);
        double duration_ms = (double)duration_ns / 1000000.0;

        if (duration_ns > max_duration_ns) {
            max_duration_ns = duration_ns;
        }

        printf("[EPNP-WCET] iter %d/%d: %.3f ms\n",
               i + 1, kEpnpWcetIterations, duration_ms);
    }

    if (wl->teardown != NULL) {
        wl->teardown(0, NULL);
    }

    printf("[EPNP-WCET] Finished: total_iters=%d max_time=%.3f ms\n",
           kEpnpWcetIterations, (double)max_duration_ns / 1000000.0);
    return 0;
}

extern "C" int run_all_workloads() {
    /* The -s suite path can enter here before platform main() registers workloads. */
    rtosbench_register_rtos_workloads();

    cout << "=== Epnp Test Start ===" << endl;

    // // 1. FAST Feature Detection
    // cout << "\n[1/9] Running FAST benchmark..." << endl;
    // fast_test();

    // // 2. EKF (Extended Kalman Filter)
    // cout << "\n[2/9] Running EKF benchmark..." << endl;
    // ekf_test();

    // // 3. ICP (Iterative Closest Point)
    // cout << "\n[3/9] Running ICP benchmark..." << endl;
    // icp_test();

    // // 4. PID Controller
    // cout << "\n[4/9] Running PID benchmark..." << endl;
    // pid_test();

    // // 5. CUSUM
    // cout << "\n[5/9] Running CUMSUM benchmark..." << endl;
    // cusum_bench_run();

    // // 6. EWMA
    // cout << "\n[6/9] Running EWMA benchmark..." << endl;
    // ewma_bench_run();

    // 7. EPNP (Efficient Perspective-n-Point)
    cout << "\n[7/9] Running EPNP benchmark..." << endl;
    epnp_schedule_wcet_test();

    // // 8. MODBUS Protocol
    // cout << "\n[8/9] Running MODBUS benchmark..." << endl;
    // modbus_test();

    // // 9. MQTT Protocol
    // cout << "\n[9/9] Running MQTT benchmark..." << endl;
    // mqtt_test();

    cout << "\n=== All Benchmarks Completed ===" << endl;
    return 0;
}
