#!/usr/bin/env python3
"""Read a TinySpice transient CSV and plot selected waveform columns."""

from __future__ import annotations

import argparse
import csv
import math
import sys
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Tuple


def _parse_number(raw_value: Optional[str], column: str, row_number: int) -> float:
    if raw_value is None:
        raise ValueError(
            f"row {row_number} has no value for column '{column}'"
        )
    try:
        value = float(raw_value)
    except ValueError as error:
        raise ValueError(
            f"column '{column}' contains a non-numeric value "
            f"at row {row_number}: {raw_value!r}"
        ) from error
    if not math.isfinite(value):
        raise ValueError(
            f"column '{column}' contains a non-finite value "
            f"at row {row_number}: {raw_value!r}"
        )
    return value


def load_transient_csv(
    csv_path: Path,
    requested_columns: Optional[Sequence[str]] = None,
) -> Tuple[List[str], Dict[str, List[float]]]:
    """Return plotted column names and numeric series, including ``time``."""
    if not csv_path.is_file():
        raise FileNotFoundError(f"CSV file does not exist: {csv_path}")

    with csv_path.open("r", encoding="utf-8", newline="") as input_file:
        reader = csv.DictReader(input_file)
        fieldnames = reader.fieldnames
        if not fieldnames:
            raise ValueError("CSV has no header")
        if len(set(fieldnames)) != len(fieldnames):
            raise ValueError("CSV header contains duplicate column names")
        if "time" not in fieldnames:
            raise ValueError("CSV is missing required 'time' column")

        if requested_columns:
            columns = list(requested_columns)
            if len(set(columns)) != len(columns):
                raise ValueError("requested columns contain duplicates")
            if "time" in columns:
                raise ValueError("'time' is the x-axis and cannot be a waveform column")
            missing = [column for column in columns if column not in fieldnames]
            if missing:
                raise ValueError(
                    "CSV does not contain requested column(s): " + ", ".join(missing)
                )
        else:
            columns = [column for column in fieldnames if column != "time"]

        if not columns:
            raise ValueError("CSV has no waveform columns to plot")

        series: Dict[str, List[float]] = {
            column: [] for column in ["time", *columns]
        }
        for row_number, row in enumerate(reader, start=2):
            for column in series:
                series[column].append(
                    _parse_number(row.get(column), column, row_number)
                )

    if not series["time"]:
        raise ValueError("CSV has no data rows")
    return columns, series


def render_plot(
    columns: Sequence[str],
    series: Dict[str, List[float]],
    output_path: Optional[Path],
) -> None:
    try:
        import matplotlib.pyplot as plt
    except ModuleNotFoundError as error:
        raise RuntimeError(
            "matplotlib is required for plotting; install it with "
            "'python3 -m pip install matplotlib' or use --validate-only"
        ) from error

    figure, axes = plt.subplots()
    for column in columns:
        axes.plot(series["time"], series[column], label=column)
    axes.set_xlabel("time (s)")
    axes.set_ylabel("value")
    axes.grid(True)
    axes.legend()
    figure.tight_layout()

    if output_path is None:
        plt.show()
    else:
        figure.savefig(output_path)
        print(f"wrote plot: {output_path}")
    plt.close(figure)


def build_argument_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Plot waveform columns from a TinySpice transient CSV."
    )
    parser.add_argument("csv_file", type=Path, help="TinySpice transient CSV")
    parser.add_argument(
        "columns",
        nargs="*",
        metavar="COLUMN",
        help="waveform columns; omit to plot every non-time column",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        help="save the plot instead of opening an interactive window",
    )
    parser.add_argument(
        "--validate-only",
        action="store_true",
        help="validate and summarize the CSV without importing matplotlib",
    )
    return parser


def main(arguments: Optional[Sequence[str]] = None) -> int:
    parser = build_argument_parser()
    args = parser.parse_args(arguments)
    try:
        columns, series = load_transient_csv(args.csv_file, args.columns)
        if args.validate_only:
            names = ", ".join(["time", *columns])
            print(f"validated {len(series['time'])} row(s): {names}")
            return 0
        render_plot(columns, series, args.output)
        return 0
    except (OSError, RuntimeError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
