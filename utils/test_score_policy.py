"""Regression checks for the seven-workload saturation scoring policy."""
import copy
import json
import logging
from pathlib import Path
import tempfile
import unittest

import score_caculate as scorer
import score_seven_workload as adapter
import ssot_to_score_json as schema

logging.disable(logging.CRITICAL)


def sample(board='LS2K1000LA', os_name='SylixOS', value=10):
    return {'env': {'board': board, 'os_name': os_name, 'cpu_type': 'loongarch64'},
            'modules': {
                'test-realtime': {},
                'test-schedule': {'summary': {'average_miss_rate': 0.1}},
                'typical-workload': {'workloads': [
                    {'name': name, 'avg_time_ms': value}
                    for name in scorer.RAW_WORKLOAD_CATEGORY_MAP]},
                'test-stress': {'power': [{'name': name, 'power_data': value}
                                          for name in scorer.POWER_TASK_MAP]}}}


def add(conn, data):
    scorer._parse_json_data(copy.deepcopy(data), conn, 'raw.json')
    conn.commit()


def export(conn, board='LS2K1000LA', os_name='SylixOS'):
    return scorer.export_results(board, os_name, 'loongarch64', conn)


class SevenWorkloadPolicy(unittest.TestCase):
    def test_single_baseline_system_scores_eighty(self):
        conn = scorer.init_db(':memory:')
        self.addCleanup(conn.close)
        add(conn, sample())
        scorer.recompute_all_scores(conn)
        result = export(conn)
        self.assertAlmostEqual(result['scores']['workload']['score'], 80)
        self.assertFalse(result['scores']['workload']['incomplete'])
        items = {name for cat in result['scores']['workload']['details'].values() for name in cat['items']}
        self.assertEqual(items, set(scorer.WORKLOAD_CATEGORY_MAP))
        self.assertEqual(conn.execute("SELECT COUNT(*) FROM raw_records WHERE category='workload'").fetchone()[0], 9)
        self.assertEqual(conn.execute("SELECT COUNT(item_score) FROM raw_records WHERE item_key IN ('ekf','epnp')").fetchone()[0], 0)

    def test_single_target_with_reference_is_independent_of_peers(self):
        conn = scorer.init_db(':memory:')
        self.addCleanup(conn.close)
        add(conn, sample())
        add(conn, sample('Target', 'Solo', 20))
        scorer.recompute_all_scores(conn)
        first = export(conn, 'Target', 'Solo')['scores']['workload']['score']
        self.assertAlmostEqual(first, 100 * 0.5 / 0.75)
        add(conn, sample('Target', 'Peer', 1))
        scorer.recompute_all_scores(conn)
        self.assertEqual(first, export(conn, 'Target', 'Solo')['scores']['workload']['score'])

    def test_legacy_cache_migration_preserves_measurements(self):
        with tempfile.TemporaryDirectory() as folder:
            path = str(Path(folder) / 'results.sqlite')
            conn = scorer.init_db(path)
            add(conn, sample())
            conn.execute("UPDATE raw_records SET last_updated='2020-01-01T00:00:00+00:00'")
            conn.commit()
            columns = 'board_model,os_name,arch,category,sub_category,item_key,raw_value,raw_unit,metric_type,source_file,last_updated'
            before = [tuple(row) for row in conn.execute(f'SELECT {columns} FROM raw_records ORDER BY item_key')]
            conn.execute('UPDATE raw_records SET item_score=17')
            scorer.upsert_score(conn, 'LS2K1000LA', 'SylixOS', 'loongarch64', 'total', 'legacy_total', 17)
            conn.execute("UPDATE scoring_metadata SET value='legacy-minmax' WHERE key='rule_version'")
            conn.commit()
            conn.close()
            conn = scorer.init_db(path)
            try:
                after = [tuple(row) for row in conn.execute(f'SELECT {columns} FROM raw_records ORDER BY item_key')]
                self.assertEqual(before, after)
                self.assertEqual(conn.execute("SELECT COUNT(*) FROM score_cache WHERE score_key='legacy_total'").fetchone()[0], 0)
                self.assertEqual(conn.execute("SELECT COUNT(item_score) FROM raw_records WHERE item_key IN ('ekf','epnp')").fetchone()[0], 0)
                self.assertAlmostEqual(export(conn)['scores']['workload']['score'], 80)
                self.assertAlmostEqual(export(conn)['scores']['realtime']['details']['schedulability']['score'], 90)
            finally:
                conn.close()

    def test_missing_member_marks_partial(self):
        conn = scorer.init_db(':memory:')
        self.addCleanup(conn.close)
        data = sample()
        data['modules']['typical-workload']['workloads'] = [w for w in data['modules']['typical-workload']['workloads'] if w['name'] != 'fast']
        add(conn, data)
        scorer.recompute_all_scores(conn)
        result = export(conn)['scores']['workload']
        self.assertTrue(result['incomplete'])
        self.assertIn('fast', result['missing_items'])

    def test_no_baseline_is_missing_not_a_fabricated_score(self):
        conn = scorer.init_db(':memory:')
        self.addCleanup(conn.close)
        add(conn, sample('Target', 'Solo'))
        scorer.recompute_all_scores(conn)
        self.assertIsNone(export(conn, 'Target', 'Solo')['scores']['workload']['score'])

    def test_adapter_uses_same_scorer_and_rejects_nine(self):
        records = [{'name': 'reference', 'vendor': 'vendor', 'family': 'reference', 'score_input': sample()}]
        result = adapter.compute(records)[0]
        self.assertEqual(result['T'], 80)
        self.assertEqual(result['P'], 80)
        self.assertEqual(result['sched'], 90)
        with self.assertRaises(ValueError):
            adapter.compute(records, 9)

    def test_converter_drops_old_scores_and_excluded_tasks(self):
        data = sample()
        data['scores'] = {'T': 17}
        result = schema.convert(data)
        self.assertNotIn('scores', result)
        self.assertEqual(result['env']['os_name'], 'SylixOS')
        self.assertEqual(len(result['modules']['typical-workload']['workloads']), 7)

    def test_baseline_change_refreshes_existing_ratios(self):
        conn = scorer.init_db(':memory:')
        self.addCleanup(conn.close)
        add(conn, sample())
        add(conn, sample('Target', 'Solo', 20))
        scorer.recompute_all_scores(conn)
        add(conn, sample(value=20))
        scorer.recompute_all_scores(conn)
        self.assertAlmostEqual(export(conn, 'Target', 'Solo')['scores']['workload']['score'], 80)

    def test_adapter_single_target_with_explicit_reference(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / 'baseline.json'
            path.write_text(json.dumps(sample()))
            records = [{'name': 'solo', 'vendor': 'vendor', 'family': 'target',
                        'score_input': sample('Target', 'Solo')}]
            self.assertEqual(adapter.compute(records, baseline_path=path)[0]['T'], 80)

    def test_exec_time_fallback_and_untagged_migration(self):
        with tempfile.TemporaryDirectory() as folder:
            path = str(Path(folder) / 'legacy.sqlite')
            conn = scorer.init_db(path)
            data = sample()
            for entry in data['modules']['typical-workload']['workloads']:
                entry['exec_time_ms'] = entry.pop('avg_time_ms')
            add(conn, data)
            conn.execute('DELETE FROM scoring_metadata')
            conn.execute('UPDATE raw_records SET item_score=3')
            conn.commit()
            conn.close()
            conn = scorer.init_db(path)
            try:
                self.assertEqual(export(conn)['scores']['workload']['score'], 80)
            finally:
                conn.close()

    def test_missing_schedule_is_not_perfect_score(self):
        import openpyxl
        import excel_to_result_json
        workbook = openpyxl.Workbook()
        result = excel_to_result_json._parse_schedule(workbook.active, 15, 7)
        self.assertIsNone(result['summary']['average_miss_rate'])
        self.assertIsNone(result['summary']['final_score'])

    def test_invalid_latency_never_scores(self):
        for value in (0, -1, float('inf'), float('nan')):
            self.assertIsNone(scorer.calc_R(value, 10, 'latency'))


if __name__ == '__main__':
    unittest.main()
