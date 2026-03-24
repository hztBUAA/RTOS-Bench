import os
import pandas as pd
from datetime import datetime

def export_test_result(board_name, os_name, job_type, data_records, summary_info):
    os.makedirs("data", exist_ok=True)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"data/{os_name}_{board_name}_{job_type.upper()}_{timestamp}.xlsx"

    df_data = pd.DataFrame(data_records, columns=["Time(s)", "Voltage(V)", "Current(A)", "Power(W)"])
    df_summary = pd.DataFrame([summary_info])

    try:
        with pd.ExcelWriter(filename, engine='openpyxl') as writer:
            df_summary.to_excel(writer, sheet_name="测试汇总", index=False)
            df_data.to_excel(writer, sheet_name="详细采样数据", index=False)
        print(f"\n[Excel Exporter] 结果已保存至: {filename}")
    except Exception as e:
        print(f"\n[Excel Exporter] 保存Excel失败: {e}")
