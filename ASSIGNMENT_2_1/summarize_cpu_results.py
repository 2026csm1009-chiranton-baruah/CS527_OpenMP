#!/usr/bin/env python3
"""Summarize repeated CPU benchmark rows using medians (standard library only)."""
import csv
import statistics
import sys
from collections import defaultdict

if len(sys.argv) != 2:
    raise SystemExit(f"Usage: {sys.argv[0]} results/cpu_benchmark_TIMESTAMP.csv")
path = sys.argv[1]
groups = defaultdict(list)
with open(path, newline="", encoding="utf-8") as f:
    for row in csv.DictReader(f):
        if row.get("status") == "PASS":
            groups[(row["workload"], int(row["size"]), int(row["threads"]))].append(row)
if not groups:
    raise SystemExit("No passing benchmark rows found.")

out = path.replace(".csv", "_summary.csv")
fields = ["workload", "size", "threads", "repetitions", "median_serial_seconds",
          "median_parallel_seconds", "median_speedup", "median_efficiency"]
with open(out, "w", newline="", encoding="utf-8") as f:
    writer = csv.DictWriter(f, fieldnames=fields)
    writer.writeheader()
    for (workload, size, threads), rows in sorted(groups.items()):
        serial = statistics.median(float(r["serial_seconds"]) for r in rows)
        parallel = statistics.median(float(r["parallel_seconds"]) for r in rows)
        speedup = serial / parallel if parallel else 0.0
        writer.writerow({
            "workload": workload, "size": size, "threads": threads,
            "repetitions": len(rows),
            "median_serial_seconds": f"{serial:.9f}",
            "median_parallel_seconds": f"{parallel:.9f}",
            "median_speedup": f"{speedup:.6f}",
            "median_efficiency": f"{speedup / threads:.6f}",
        })
print(f"Summary written to: {out}")
print("\nMedian results (speedup = serial / parallel; efficiency = speedup / threads):")
print(f"{'workload':<14} {'threads':>7} {'serial(s)':>12} {'parallel(s)':>12} {'speedup':>9} {'efficiency':>11}")
with open(out, newline="", encoding="utf-8") as f:
    for r in csv.DictReader(f):
        print(f"{r['workload']:<14} {r['threads']:>7} {r['median_serial_seconds']:>12} "
              f"{r['median_parallel_seconds']:>12} {r['median_speedup']:>9} {r['median_efficiency']:>11}")
