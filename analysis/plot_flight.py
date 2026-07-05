#!/usr/bin/env python3
"""
plot_flight.py  —  Quick-look analysis for flight_computer CSV logs
Usage:
    python plot_flight.py FLT_0000.CSV

Produces a 3-panel plot:
    1. Altitude AGL (m) vs time (s)
    2. Acceleration magnitude |a| (m/s²) vs time
    3. Gyro rates gx/gy/gz (°/s) vs time

Derived stats printed to console:
    - Apogee altitude and time
    - Peak acceleration
    - Estimated burn duration (accel threshold)
"""

import sys
import pathlib
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# ── Constants ──────────────────────────────────────────────────────────────
G = 9.80665          # m/s²
BURN_ACCEL_THRESH = 2 * G   # net accel above 2 g considered "motor burning"

# ── Load data ──────────────────────────────────────────────────────────────
def load(path: str) -> pd.DataFrame:
    df = pd.read_csv(path)
    df.columns = df.columns.str.strip()
    df["time_s"] = (df["timestamp_ms"] - df["timestamp_ms"].iloc[0]) / 1000.0
    df["accel_mag"] = np.sqrt(df["ax_mps2"]**2 + df["ay_mps2"]**2 + df["az_mps2"]**2)
    return df

# ── Stats ──────────────────────────────────────────────────────────────────
def print_stats(df: pd.DataFrame) -> None:
    apogee_idx = df["altitude_m"].idxmax()
    apogee_alt = df["altitude_m"].iloc[apogee_idx]
    apogee_t   = df["time_s"].iloc[apogee_idx]

    peak_accel_idx = df["accel_mag"].idxmax()
    peak_accel     = df["accel_mag"].iloc[peak_accel_idx]
    peak_accel_t   = df["time_s"].iloc[peak_accel_idx]

    # Burn duration: first and last sample where net accel > threshold
    burn_mask = df["accel_mag"] > BURN_ACCEL_THRESH
    if burn_mask.any():
        burn_start = df.loc[burn_mask, "time_s"].iloc[0]
        burn_end   = df.loc[burn_mask, "time_s"].iloc[-1]
        burn_dur   = burn_end - burn_start
    else:
        burn_start = burn_end = burn_dur = float("nan")

    print(f"\n{'─'*44}")
    print(f"  Flight summary")
    print(f"{'─'*44}")
    print(f"  Duration          : {df['time_s'].iloc[-1]:.1f} s")
    print(f"  Apogee altitude   : {apogee_alt:.1f} m  ({apogee_alt*3.281:.0f} ft)  at t={apogee_t:.2f} s")
    print(f"  Peak acceleration : {peak_accel/G:.1f} g  at t={peak_accel_t:.2f} s")
    print(f"  Burn duration     : {burn_dur:.2f} s  (>{BURN_ACCEL_THRESH/G:.0f} g threshold)")
    print(f"{'─'*44}\n")

# ── Plot ───────────────────────────────────────────────────────────────────
def plot(df: pd.DataFrame, source_path: str) -> None:
    fig, axes = plt.subplots(3, 1, figsize=(10, 8), sharex=True)
    fig.suptitle(f"Flight data — {pathlib.Path(source_path).name}", fontsize=13)

    t = df["time_s"]

    # Panel 1: Altitude
    ax1 = axes[0]
    ax1.plot(t, df["altitude_m"], color="#1f77b4", linewidth=1.2)
    ax1.set_ylabel("Altitude AGL (m)")
    ax1.yaxis.set_minor_locator(ticker.AutoMinorLocator())
    apogee_idx = df["altitude_m"].idxmax()
    ax1.axvline(df["time_s"].iloc[apogee_idx], color="gray", linestyle="--",
                linewidth=0.8, label=f"Apogee {df['altitude_m'].iloc[apogee_idx]:.0f} m")
    ax1.legend(fontsize=9, loc="upper right")
    ax1.grid(True, which="major", linestyle=":", alpha=0.5)

    # Panel 2: Acceleration magnitude
    ax2 = axes[1]
    ax2.plot(t, df["accel_mag"] / G, color="#d62728", linewidth=1.0)
    ax2.axhline(1.0, color="gray", linestyle=":", linewidth=0.8, label="1 g")
    ax2.set_ylabel("Accel magnitude (g)")
    ax2.yaxis.set_minor_locator(ticker.AutoMinorLocator())
    ax2.legend(fontsize=9, loc="upper right")
    ax2.grid(True, which="major", linestyle=":", alpha=0.5)

    # Panel 3: Gyro rates
    ax3 = axes[2]
    ax3.plot(t, df["gx_dps"], linewidth=0.8, label="gx")
    ax3.plot(t, df["gy_dps"], linewidth=0.8, label="gy")
    ax3.plot(t, df["gz_dps"], linewidth=0.8, label="gz")
    ax3.set_ylabel("Angular rate (°/s)")
    ax3.set_xlabel("Time (s)")
    ax3.legend(fontsize=9, loc="upper right")
    ax3.grid(True, which="major", linestyle=":", alpha=0.5)

    plt.tight_layout()
    out = pathlib.Path(source_path).with_suffix(".png")
    plt.savefig(out, dpi=150)
    print(f"Plot saved → {out}")
    plt.show()

# ── Entry point ────────────────────────────────────────────────────────────
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python plot_flight.py <FLT_NNNN.CSV>")
        sys.exit(1)

    csv_path = sys.argv[1]
    df = load(csv_path)
    print_stats(df)
    plot(df, csv_path)
