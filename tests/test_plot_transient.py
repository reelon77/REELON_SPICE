#!/usr/bin/env python3
"""Dependency-free tests for the TinySpice transient CSV reader."""

import importlib.util
import tempfile
import unittest
from pathlib import Path


SCRIPT_PATH = Path(__file__).resolve().parents[1] / "scripts" / "plot_transient.py"
SPEC = importlib.util.spec_from_file_location("plot_transient", SCRIPT_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"cannot load {SCRIPT_PATH}")
PLOT_TRANSIENT = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PLOT_TRANSIENT)


class TransientCsvReaderTest(unittest.TestCase):
    def setUp(self):
        self.workspace = tempfile.TemporaryDirectory(prefix="tinyspice_plot_")
        self.root = Path(self.workspace.name)

    def tearDown(self):
        self.workspace.cleanup()

    def write_csv(self, name, content):
        path = self.root / name
        path.write_text(content, encoding="utf-8")
        return path

    def test_loads_all_waveforms_as_numbers(self):
        path = self.write_csv(
            "valid.csv",
            "time,V(out),I(v1)\n0,0,0\n0.25,0.4,-0.8\n",
        )

        columns, series = PLOT_TRANSIENT.load_transient_csv(path)

        self.assertEqual(columns, ["V(out)", "I(v1)"])
        self.assertEqual(series["time"], [0.0, 0.25])
        self.assertEqual(series["V(out)"], [0.0, 0.4])

    def test_selected_columns_ignore_unselected_text(self):
        path = self.write_csv(
            "selected.csv",
            "time,V(out),note\n0,0,start\n0.25,0.4,rising\n",
        )

        columns, series = PLOT_TRANSIENT.load_transient_csv(path, ["V(out)"])

        self.assertEqual(columns, ["V(out)"])
        self.assertNotIn("note", series)

    def test_reports_missing_file(self):
        with self.assertRaisesRegex(FileNotFoundError, "does not exist"):
            PLOT_TRANSIENT.load_transient_csv(self.root / "missing.csv")

    def test_reports_missing_time_column(self):
        path = self.write_csv("no-time.csv", "V(out)\n0\n")

        with self.assertRaisesRegex(ValueError, "required 'time'"):
            PLOT_TRANSIENT.load_transient_csv(path)

    def test_reports_non_numeric_selected_column(self):
        path = self.write_csv("bad.csv", "time,V(out)\n0,zero\n")

        with self.assertRaisesRegex(ValueError, "non-numeric.*row 2"):
            PLOT_TRANSIENT.load_transient_csv(path)

    def test_validate_only_does_not_require_matplotlib(self):
        path = self.write_csv("valid.csv", "time,V(out)\n0,0\n")

        self.assertEqual(
            PLOT_TRANSIENT.main([str(path), "V(out)", "--validate-only"]),
            0,
        )


if __name__ == "__main__":
    unittest.main()
