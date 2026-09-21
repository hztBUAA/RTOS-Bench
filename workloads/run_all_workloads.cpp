#include <iostream>
using namespace std;

extern "C" {
	int fast_test(void);
	int icp_test(void);
	int modbus_test(void);
	int mqtt_test(void);
	int pid_test(void);
	int cusum_bench_run(void);
	int ewma_bench_run(void);
}

extern "C" int run_all_workloads() {
    cout << "=== Benchmark Suite Start ===" << endl;

    // Category 1: Real-Time Control
    cout << "\n============================================================" << endl;
    cout << "[Category 1/4] Real-Time Control" << endl;
    cout << "Benchmarks: PID" << endl;
    cout << "============================================================" << endl;

    cout << "\n--- [Benchmark 1/7] PID benchmark start ---\n" << endl;
    pid_test();
    cout << "\n--- [Benchmark 1/7] PID benchmark finished ---" << endl;

    // Category 2: Perception Computing
    cout << "\n============================================================" << endl;
    cout << "[Category 2/4] Perception Computing" << endl;
    cout << "Benchmarks: FAST, ICP" << endl;
    cout << "============================================================" << endl;

    cout << "\n--- [Benchmark 2/7] FAST benchmark start ---\n" << endl;
    fast_test();
    cout << "\n--- [Benchmark 2/7] FAST benchmark finished ---" << endl;

    cout << "\n--- [Benchmark 3/7] ICP benchmark start ---\n" << endl;
    icp_test();
    cout << "\n--- [Benchmark 3/7] ICP benchmark finished ---" << endl;

    // Category 3: Communication Protocol
    cout << "\n============================================================" << endl;
    cout << "[Category 3/4] Communication Protocol" << endl;
    cout << "Benchmarks: MODBUS, MQTT" << endl;
    cout << "============================================================" << endl;

    cout << "\n--- [Benchmark 4/7] MODBUS benchmark start ---\n" << endl;
    modbus_test();
    cout << "\n--- [Benchmark 4/7] MODBUS benchmark finished ---" << endl;

    cout << "\n--- [Benchmark 5/7] MQTT benchmark start ---\n" << endl;
    mqtt_test();
    cout << "\n--- [Benchmark 5/7] MQTT benchmark finished ---" << endl;

    // Category 4: Operations Monitoring
    cout << "\n============================================================" << endl;
    cout << "[Category 4/4] Operations Monitoring" << endl;
    cout << "Benchmarks: EWMA, CUMSUM" << endl;
    cout << "============================================================" << endl;

    cout << "\n--- [Benchmark 6/7] EWMA benchmark start ---\n" << endl;
    ewma_bench_run();
    cout << "\n--- [Benchmark 6/7] EWMA benchmark finished ---" << endl;

    cout << "\n--- [Benchmark 7/7] CUMSUM benchmark start ---\n" << endl;
    cusum_bench_run();
    cout << "\n--- [Benchmark 7/7] CUMSUM benchmark finished ---" << endl;

    cout << "\n=== All Benchmarks Completed ===" << endl;
    return 0;
}
