#include <iostream>
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
}

extern "C" int run_all_workloads() {
    cout << "=== Benchmark Suite Start ===" << endl;

    // 1. FAST Feature Detection
    cout << "\n[1/9] Running FAST benchmark..." << endl;
    fast_test();

    // 2. EKF (Extended Kalman Filter)
    cout << "\n[2/9] Running EKF benchmark..." << endl;
    ekf_test();

    // 3. EPNP (Efficient Perspective-n-Point)
    cout << "\n[3/9] Running EPNP benchmark..." << endl;
    epnp_test();

    // 4. ICP (Iterative Closest Point)
    cout << "\n[4/9] Running ICP benchmark..." << endl;
    icp_test();

    // 5. MODBUS Protocol
    cout << "\n[5/9] Running MODBUS benchmark..." << endl;
    modbus_test();

    // 6. MQTT Protocol
    cout << "\n[6/9] Running MQTT benchmark..." << endl;
    mqtt_test();

    // 7. PID Controller
    cout << "\n[7/9] Running PID benchmark..." << endl;
    pid_test();

    // 8. CUSUM
    cout << "\n[8/9] Running CUMSUM benchmark..." << endl;
    cusum_bench_run();

    // 9. EWMA
    cout << "\n[9/9] Running EWMA benchmark..." << endl;
    ewma_bench_run();

    cout << "\n=== All Benchmarks Completed ===" << endl;
    return 0;
}
