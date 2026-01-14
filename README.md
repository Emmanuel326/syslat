# syslat — Syscall & Page Cache Latency Exploration

This repository explores **`read()` syscall latency under different Linux page
cache states** on a fixed machine and kernel.

The goal is **understanding and observability**, not absolute performance claims.

All results should be interpreted as **relative comparisons** within this
environment.

---

## Test Environment

This document describes the hardware and software environment used for all
measurements in this repository.

---

## Hardware

### Machine
- Model: Lenovo ThinkPad T470s (20JTS07L00)

### CPU
- Model: Intel(R) Core(TM) i5-6300U
- Microarchitecture: Skylake
- Cores: 2
- Threads: 4
- Max Turbo Frequency: 3.00 GHz

### Memory
- Installed RAM: 8 GiB
- Swap: 8 GiB (not actively used during tests)

### Storage
- Device: Samsung MZVLW256HEHP-000L7
- Interface: NVMe
- Capacity: 238.5 GiB
- Rotational: No (SSD)

### GPU
- Intel HD Graphics 520 (Integrated)

---

## Operating System

- Distribution: Arch Linux x86_64
- Kernel Version: Linux 6.17.9-arch1-1
- Init System: systemd
- Shell: bash 5.3.8

---

## CPU Frequency & Power Management

- CPU Frequency Driver: `intel_pstate`
- Available Governors: `performance`, `powersave`
- Default Governor: `powersave`
- Hardware Frequency Range: 400 MHz – 3.00 GHz
- Turbo Boost: Enabled
- Energy Performance Preference: `balance_performance`

No manual CPU pinning, frequency locking, or turbo disabling is applied unless
explicitly noted.

---

## Filesystem & Mounts

- Root Filesystem: ext4
- Block Device: `/dev/nvme0n1p2`
- Storage Interface: NVMe
- Page cache behavior is provided by the Linux kernel VFS layer.

---

## Desktop Environment

- Window Manager: i3 (X11)
- Terminal Emulator: Alacritty 0.16.1
- Display: 1920x1080 @ 60 Hz

---

## Lab Environment Overrides

Some experiments are repeated under a **lab mode** to reduce measurement noise
and improve repeatability.

These settings are applied via `perf-lab-on.sh` and are **not representative of
default system behavior**.

### CPU
- Governor forced to `performance`
- Turbo enabled

### Kernel
- `kernel.perf_event_paranoid = -1`
- `kernel.kptr_restrict = 0`
- `kernel.sched_autogroup_enabled = 0`
- ASLR disabled

### VM
- `vm.swappiness = 1`
- `vm.dirty_ratio = 10`
- `vm.dirty_background_ratio = 5`

---

## Locked Invariants

The following invariants **must not change** for results to be comparable:

- Same physical machine
- Same CPU and kernel version
- Same filesystem (ext4 on NVMe)
- Single process, single thread
- Blocking I/O only
- Same file path and file size
- Same clock source for timing
- Same execution mode (baseline or lab)

If any of these change, measurements are considered a new experiment.

---

## Experimental Constraints

These are **deliberate design limits**:

- Only the `read()` syscall is measured
- No writes, `fsync`, `mmap`, async I/O, or `io_uring`
- Latency is measured per syscall, not throughput
- Fixed read sizes (e.g. 4 KiB, 64 KiB)
- Fixed iteration counts per run
- Results reported as latency distributions (p50, p99, max)

---

## Cache State Control

Page cache state is treated as **experimental input**, not environment
configuration.

Cold-cache runs are performed by explicitly dropping caches immediately before
measurement:

```bash
sync
echo 3 | sudo tee /proc/sys/vm/drop_caches

