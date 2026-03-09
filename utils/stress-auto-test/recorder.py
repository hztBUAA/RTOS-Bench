# recorder.py
import threading
import time

class PowerRecorder:
    def __init__(self, power_supply):
        self.power = power_supply
        self.is_recording = False
        self.records = []
        self.thread = None

    def start(self):
        print("[Recorder] 开始录制功耗数据...")
        self.is_recording = True
        self.records = []
        self.thread = threading.Thread(target=self._record_loop)
        self.thread.start()

    def _record_loop(self):
        start_time = time.time()
        while self.is_recording:
            v, i, p = self.power.measure_all()
            current_time = time.time() - start_time
            self.records.append((current_time, v, i, p))
            time.sleep(0.01)

    def stop(self):
        print("[Recorder] 结束录制功耗数据...")
        self.is_recording = False
        if self.thread:
            self.thread.join()
        return self.records


def calculate_energy(records):
    """微元法积分计算总功耗: E = Σ P * Δt"""
    if len(records) < 2:
        return 0.0

    total_joules = 0.0
    for idx in range(1, len(records)):
        t1, v1, i1, _ = records[idx - 1]
        t2, v2, i2, _ = records[idx]
        delta_t = t2 - t1
        avg_power = (v1 * i1 + v2 * i2) / 2.0
        total_joules += avg_power * delta_t

    return total_joules