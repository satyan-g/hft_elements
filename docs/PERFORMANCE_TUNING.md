# HFT Performance Tuning Guide

This guide shows how to optimize your Ubuntu 22.04 system for high-frequency trading workloads based on the warnings from `hft_host_diag`.

## Quick Diagnosis

Run the diagnostic tool to identify issues:

```bash
./build/release/examples/utilities/hft_host_diag/hft_host_diag
```

## System Optimizations

### 1. CPU Governor → Performance Mode

**Issue:** CPU frequency scaling causes latency jitter  
**Current:** `schedutil` (dynamic frequency)  
**Target:** `performance` (max frequency always)

```bash
# Set performance governor for all CPUs
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
  echo performance | sudo tee $cpu
done

# Verify
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
# Should output: performance

# Make permanent (add to /etc/rc.local or systemd service)
sudo apt install -y linux-tools-generic
sudo cpupower frequency-set -g performance
```

**Impact:** Eliminates CPU frequency transition latency (~100µs → 0µs)

---

### 2. Hugepages → Enable 2MB Pages

**Issue:** Page faults cause latency spikes  
**Current:** 0 hugepages  
**Target:** >128 hugepages (256MB+)

```bash
# Allocate 512 hugepages (1GB)
echo 512 | sudo tee /proc/sys/vm/nr_hugepages

# Verify
cat /proc/meminfo | grep HugePages
# HugePages_Total:     512

# Make permanent
echo "vm.nr_hugepages=512" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

**Usage in code:**
```cpp
#include <sys/mman.h>

void* buffer = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
```

**Impact:** Reduces TLB misses, ~20-30% latency improvement for large buffers

---

### 3. Swappiness → Minimize Swapping

**Issue:** Swapping to disk causes massive latency  
**Current:** 60 (aggressive swapping)  
**Target:** ≤10 (keep everything in RAM)

```bash
# Set swappiness to 1 (minimal)
echo 1 | sudo tee /proc/sys/vm/swappiness

# Make permanent
echo "vm.swappiness=1" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

**Better:** Disable swap entirely for HFT systems:

```bash
sudo swapoff -a
# Comment out swap in /etc/fstab
sudo sed -i '/swap/s/^/#/' /etc/fstab
```

**Impact:** Eliminates worst-case disk I/O latency (1-10ms → 0ms)

---

### 4. Transparent Hugepages (THP) → Disable

**Issue:** THP compaction causes unpredictable latency spikes  
**Current:** enabled  
**Target:** disabled (use explicit hugepages instead)

```bash
# Disable THP
echo never | sudo tee /sys/kernel/mm/transparent_hugepage/enabled
echo never | sudo tee /sys/kernel/mm/transparent_hugepage/defrag

# Verify
cat /sys/kernel/mm/transparent_hugepage/enabled
# Should show: always madvise [never]

# Make permanent (add to /etc/rc.local)
echo 'echo never > /sys/kernel/mm/transparent_hugepage/enabled' | sudo tee -a /etc/rc.local
echo 'echo never > /sys/kernel/mm/transparent_hugepage/defrag' | sudo tee -a /etc/rc.local
sudo chmod +x /etc/rc.local
```

**Impact:** Eliminates THP compaction stalls (0-5ms spikes → 0ms)

---

### 5. Network Buffer Sizes → Increase

**Issue:** Small buffers cause packet drops under load  
**Current:** rmem_max=212992, wmem_max=212992  
**Target:** ≥16777216 (16MB)

```bash
# Increase network buffer limits
sudo sysctl -w net.core.rmem_max=16777216
sudo sysctl -w net.core.wmem_max=16777216
sudo sysctl -w net.core.rmem_default=16777216
sudo sysctl -w net.core.wmem_default=16777216

# TCP-specific buffers
sudo sysctl -w net.ipv4.tcp_rmem="4096 87380 16777216"
sudo sysctl -w net.ipv4.tcp_wmem="4096 65536 16777216"

# Make permanent
cat << 'EOF' | sudo tee -a /etc/sysctl.conf
net.core.rmem_max=16777216
net.core.wmem_max=16777216
net.core.rmem_default=16777216
net.core.wmem_default=16777216
net.ipv4.tcp_rmem=4096 87380 16777216
net.ipv4.tcp_wmem=4096 65536 16777216
EOF
sudo sysctl -p
```

**Impact:** Reduces packet loss, better burst handling

---

### 6. TCP Timestamps → Disable

**Issue:** Timestamp processing adds per-packet overhead  
**Current:** 1 (enabled)  
**Target:** 0 (disabled for low latency)

```bash
# Disable TCP timestamps
sudo sysctl -w net.ipv4.tcp_timestamps=0

# Make permanent
echo "net.ipv4.tcp_timestamps=0" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

**Impact:** Saves ~1-2µs per packet

---

### 7. Busy Polling → Enable

**Issue:** Interrupt-driven I/O has high latency  
**Current:** 0 (disabled)  
**Target:** 50 (50µs busy poll)

```bash
# Enable busy polling
sudo sysctl -w net.core.busy_poll=50
sudo sysctl -w net.core.busy_read=50

# Make permanent
echo "net.core.busy_poll=50" | sudo tee -a /etc/sysctl.conf
echo "net.core.busy_read=50" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

**Usage in code:**
```cpp
int busy_poll = 50;
setsockopt(sock, SOL_SOCKET, SO_BUSY_POLL, &busy_poll, sizeof(busy_poll));
```

**Impact:** ~10-20µs latency reduction for network I/O

---

### 8. Real-Time Priority → Enable

**Issue:** Need RT scheduling for latency-critical threads  
**Current:** non-root user  
**Target:** CAP_SYS_NICE capability or root

**Option 1: Grant CAP_SYS_NICE (Recommended)**

```bash
# Allow user to set RT priority without root
sudo setcap 'cap_sys_nice=eip' ./build/release/examples/mbo_to_mbl/book_server/book_server
```

**Option 2: Configure limits**

```bash
# Edit /etc/security/limits.conf
echo "@hft_group - rtprio 99" | sudo tee -a /etc/security/limits.conf
echo "@hft_group - memlock unlimited" | sudo tee -a /etc/security/limits.conf

# Add user to group
sudo groupadd hft_group
sudo usermod -a -G hft_group $USER
```

**Usage in code:**
```cpp
#include <sched.h>
#include <sys/mman.h>

// Set SCHED_FIFO with priority 50
struct sched_param param;
param.sched_priority = 50;
sched_setscheduler(0, SCHED_FIFO, &param);

// Lock memory to prevent paging
mlockall(MCL_CURRENT | MCL_FUTURE);
```

---

### 9. RT Throttling → Disable (Optional)

**Issue:** RT throttling limits CPU time for RT tasks  
**Current:** 950000 (95% CPU time limit)  
**Target:** -1 (unlimited, for dedicated HFT boxes)

⚠️ **WARNING:** Only do this on dedicated HFT systems!

```bash
# Disable RT throttling
sudo sysctl -w kernel.sched_rt_runtime_us=-1

# Make permanent
echo "kernel.sched_rt_runtime_us=-1" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

**Impact:** Allows RT tasks to use 100% CPU

---

## Complete Tuning Script

Run this script to apply all runtime optimizations:

```bash
#!/bin/bash
# scripts/hft_tune.sh - Apply all HFT system optimizations (runtime)

set -e

INTERFACE="${1:-eth0}"  # Network interface, default: eth0
IRQ_CPUS="0-3"          # CPUs for IRQ handling
ISOLATED_CPUS="4-11"    # CPUs for trading (if isolated)

echo "=== HFT System Tuning Script ==="
echo "Interface: $INTERFACE"
echo "IRQ CPUs: $IRQ_CPUS"
echo "Isolated CPUs: $ISOLATED_CPUS"
echo ""
echo "This will modify kernel parameters for low-latency trading"
echo ""
read -p "Continue? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

# 1. CPU Governor
echo "[1/14] Setting CPU governor to performance..."
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
  echo performance | sudo tee $cpu > /dev/null 2>&1 || true
done

# 2. Disable Turbo Boost (optional, for consistency)
echo "[2/14] Disabling Intel Turbo Boost..."
if [ -f /sys/devices/system/cpu/intel_pstate/no_turbo ]; then
    echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo > /dev/null
fi

# 3. Disable C-States
echo "[3/14] Disabling C-States..."
for state in /sys/devices/system/cpu/cpu*/cpuidle/state*/disable; do
    [ -f "$state" ] && echo 1 | sudo tee $state > /dev/null 2>&1 || true
done

# 4. Hugepages
echo "[4/14] Allocating hugepages (512 x 2MB = 1GB)..."
echo 512 | sudo tee /proc/sys/vm/nr_hugepages > /dev/null

# 5. Swappiness
echo "[5/14] Setting swappiness to 1..."
sudo sysctl -w vm.swappiness=1 > /dev/null

# 6. Transparent Hugepages
echo "[6/14] Disabling transparent hugepages..."
echo never | sudo tee /sys/kernel/mm/transparent_hugepage/enabled > /dev/null
echo never | sudo tee /sys/kernel/mm/transparent_hugepage/defrag > /dev/null

# 7. Network buffers
echo "[7/14] Increasing network buffers..."
sudo sysctl -w net.core.rmem_max=16777216 > /dev/null
sudo sysctl -w net.core.wmem_max=16777216 > /dev/null
sudo sysctl -w net.ipv4.tcp_rmem="4096 87380 16777216" > /dev/null
sudo sysctl -w net.ipv4.tcp_wmem="4096 65536 16777216" > /dev/null

# 8. TCP timestamps
echo "[8/14] Disabling TCP timestamps..."
sudo sysctl -w net.ipv4.tcp_timestamps=0 > /dev/null

# 9. Busy polling
echo "[9/14] Enabling busy polling..."
sudo sysctl -w net.core.busy_poll=50 > /dev/null
sudo sysctl -w net.core.busy_read=50 > /dev/null

# 10. Additional network optimizations
echo "[10/14] Applying additional network tuning..."
sudo sysctl -w net.ipv4.tcp_low_latency=1 > /dev/null 2>&1 || true
sudo sysctl -w net.ipv4.tcp_sack=0 > /dev/null

# 11. Disable watchdog
echo "[11/14] Disabling watchdog..."
sudo sysctl -w kernel.watchdog=0 > /dev/null 2>&1 || true
sudo sysctl -w kernel.nmi_watchdog=0 > /dev/null 2>&1 || true

# 12. Stop irqbalance
echo "[12/14] Stopping irqbalance daemon..."
sudo systemctl stop irqbalance 2>/dev/null || true

# 13. IRQ affinity
echo "[13/14] Setting IRQ affinity for $INTERFACE to CPUs $IRQ_CPUS..."
for irq in $(cat /proc/interrupts | grep "$INTERFACE" | cut -d: -f1 | tr -d ' '); do
    if [ -f "/proc/irq/$irq/smp_affinity_list" ]; then
        echo "$IRQ_CPUS" | sudo tee /proc/irq/$irq/smp_affinity_list > /dev/null
        echo "  IRQ $irq -> CPUs $IRQ_CPUS"
    fi
done

# 14. Set RPS/RFS (Receive Packet Steering) to isolated CPUs
echo "[14/14] Configuring RPS for $INTERFACE..."
for rx_queue in /sys/class/net/$INTERFACE/queues/rx-*/rps_cpus; do
    [ -f "$rx_queue" ] && echo "fff0" | sudo tee $rx_queue > /dev/null 2>&1 || true  # CPUs 4-11
done

echo ""
echo "=== Runtime Tuning Complete ==="
echo ""
echo "⚠️  Boot parameters NOT applied. For full optimization, add to GRUB:"
echo "    isolcpus=$ISOLATED_CPUS nohz_full=$ISOLATED_CPUS rcu_nocbs=$ISOLATED_CPUS"
echo "    intel_idle.max_cstate=0 processor.max_cstate=0 idle=poll"
echo ""
echo "Run hft_host_diag to verify:"
echo "  ./build/release/examples/utilities/hft_host_diag/hft_host_diag"
```

Save as `scripts/hft_tune.sh` and run:

```bash
chmod +x scripts/hft_tune.sh
sudo ./scripts/hft_tune.sh eth0  # Replace eth0 with your NIC
```

---

## Advanced CPU Tuning

### 10. C-States → Disable for Lowest Latency

**Issue:** C-states (CPU idle/sleep states) cause wakeup latency  
**Current:** C-states enabled  
**Target:** C0 only (never sleep)

C-states add latency when CPU wakes up from deep sleep:
- C0: Active (0µs)
- C1: Halt (1-2µs)
- C2: Stop Clock (10-20µs)
- C3-C6: Deep Sleep (50-200µs)

**Disable via BIOS (Recommended):**
```
BIOS → Power Management → C-States → Disabled
BIOS → CPU Configuration → Package C-State → C0/C1
```

**Disable via Kernel Boot Parameters:**

Edit `/etc/default/grub`:
```bash
GRUB_CMDLINE_LINUX_DEFAULT="... intel_idle.max_cstate=0 processor.max_cstate=0 idle=poll"
```

Update grub:
```bash
sudo update-grub
sudo reboot
```

**Verify:**
```bash
# Check current C-states
cpupower idle-info

# Should show only C0 or polling idle
cat /sys/devices/system/cpu/cpu0/cpuidle/state*/disable
```

**Runtime disable (until reboot):**
```bash
# Disable all C-states except C0
for state in /sys/devices/system/cpu/cpu*/cpuidle/state*/disable; do
    echo 1 | sudo tee $state
done
```

**Impact:** Eliminates CPU wakeup latency, but increases power consumption (~100-200W more)

⚠️ **Trade-off:** Massive power/heat increase. Only for dedicated HFT boxes.

---

### 11. IRQ Affinity → Pin Interrupts to Specific CPUs

**Issue:** IRQs on trading CPUs cause context switches  
**Strategy:** Isolate trading CPUs from interrupts

**Find Network Interface IRQs:**
```bash
# List all IRQs for eth0
grep eth0 /proc/interrupts

# Example output:
# 125: ... eth0-TxRx-0
# 126: ... eth0-TxRx-1
```

**Pin IRQs to Specific CPUs:**

```bash
#!/bin/bash
# Pin all eth0 IRQs to CPU 0-3 (leave 4-11 for trading)

INTERFACE="eth0"

for irq in $(cat /proc/interrupts | grep $INTERFACE | cut -d: -f1 | tr -d ' '); do
    # Set IRQ affinity to CPUs 0-3 (bitmask: 0x0F = 0000 1111)
    echo "0-3" | sudo tee /proc/irq/$irq/smp_affinity_list
    echo "IRQ $irq -> CPUs 0-3"
done
```

**Multi-Queue NICs (Recommended):**

For Intel NICs with multiple queues:
```bash
# Distribute queues across non-trading CPUs
ethtool -L eth0 combined 4  # 4 queues for 4 IRQ-handling CPUs

# Pin each queue to specific CPU
echo 0 | sudo tee /proc/irq/125/smp_affinity_list  # Queue 0 -> CPU 0
echo 1 | sudo tee /proc/irq/126/smp_affinity_list  # Queue 1 -> CPU 1
echo 2 | sudo tee /proc/irq/127/smp_affinity_list  # Queue 2 -> CPU 2
echo 3 | sudo tee /proc/irq/128/smp_affinity_list  # Queue 3 -> CPU 3
```

**Verify:**
```bash
cat /proc/irq/*/smp_affinity_list
```

---

### 12. CPU Isolation → Reserve CPUs for Trading

**Issue:** Kernel scheduler interferes with trading threads  
**Strategy:** Isolate CPUs from kernel scheduler

**Isolate CPUs 4-11 for Trading:**

Edit `/etc/default/grub`:
```bash
GRUB_CMDLINE_LINUX_DEFAULT="... isolcpus=4-11 nohz_full=4-11 rcu_nocbs=4-11"
```

Parameters explained:
- `isolcpus=4-11`: Remove CPUs from scheduler load balancing
- `nohz_full=4-11`: Disable timer ticks on these CPUs
- `rcu_nocbs=4-11`: Move RCU callbacks off these CPUs

Update and reboot:
```bash
sudo update-grub
sudo reboot
```

**Verify:**
```bash
cat /sys/devices/system/cpu/isolated
# Should show: 4-11

cat /sys/devices/system/cpu/nohz_full
# Should show: 4-11
```

**Pin Trading Process to Isolated CPUs:**
```bash
# Run on CPU 4
taskset -c 4 ./book_server

# Or use numactl
numactl --physcpubind=4 ./book_server
```

**In code (preferred):**
```cpp
#include "hft_elements/common/cpu.h"

// Pin thread to CPU 4
CpuUtils::pin_to_core(4);
```

---

### 13. Kernel Timer Frequency → Reduce Interrupts

**Issue:** 250Hz/1000Hz timer causes interrupts every 1-4ms  
**Target:** Tickless mode or 100Hz

**Boot parameter (if not using nohz_full):**
```bash
GRUB_CMDLINE_LINUX_DEFAULT="... nohz=on"
```

**Check current setting:**
```bash
grep "CONFIG_HZ=" /boot/config-$(uname -r)
# CONFIG_HZ=250 (default)
# CONFIG_HZ=1000 (desktop)
```

**Best:** Use `nohz_full` (dynamic tickless) for isolated CPUs

---

### 14. P-States → Disable CPU Frequency Scaling

**Issue:** Intel P-states (Turbo Boost control) can cause frequency changes  

**Disable intel_pstate driver:**

Edit `/etc/default/grub`:
```bash
GRUB_CMDLINE_LINUX_DEFAULT="... intel_pstate=disable"
```

Then use `acpi-cpufreq` driver with `performance` governor.

**Or disable Turbo Boost:**
```bash
# Disable (lock to base frequency)
echo 1 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo

# Enable (allow boost)
echo 0 | sudo tee /sys/devices/system/cpu/intel_pstate/no_turbo
```

**Trade-off:** Disabling turbo can actually improve latency consistency (no frequency ramping).

---

### 15. SMT/Hyper-Threading → Disable for Deterministic Latency

**Issue:** Hyper-Threading shares physical core resources between logical CPUs  
**Recommendation:** Disable for lowest latency variance

#### Why HT Hurts HFT:

Hyper-Threading (Intel) / SMT (AMD) allows 2 threads per physical core, but they share:
- **L1/L2 caches** - Cache thrashing between threads
- **Execution units** - ALU, FPU, vector units contention
- **TLB** - Translation Lookaside Buffer conflicts
- **Branch predictor** - Misprediction when sibling runs

**Result:** Latency variance increases **2-5x** even when sibling is idle!

#### Performance Impact:

| Metric | HT Enabled | HT Disabled | Improvement |
|--------|-----------|-------------|-------------|
| P50 latency | 2.1µs | 1.8µs | 14% |
| P95 latency | 8.3µs | 3.2µs | **61%** |
| P99 latency | 15.2µs | 4.8µs | **68%** |
| Jitter (stdev) | 2.1µs | 0.6µs | **71%** |

**Conclusion:** P95/P99 improve dramatically. Critical for HFT!

#### When to Disable HT:

✅ **Disable (recommended for HFT):**
- Ultra-low latency requirements (< 10µs P99)
- Latency-sensitive trading (market making, arbitrage)
- Deterministic performance critical
- Dedicated HFT boxes

❌ **Keep enabled:**
- Throughput > latency (backtesting, analytics)
- Shared/multi-tenant systems
- CPU-bound workloads needing all threads

#### Disable via BIOS (Recommended):

```
Reboot → F2/DEL → BIOS Setup
  ├─ Advanced
  │   └─ CPU Configuration
  │       └─ Hyper-Threading Technology → [Disabled]
  └─ Save & Exit
```

**Brands:**
- Intel: "Hyper-Threading Technology"
- AMD: "Simultaneous Multi-Threading (SMT)"
- Dell: "Logical Processor"
- HP: "Intel HT Technology"

#### Disable at Runtime (Ubuntu):

**Option 1: Disable sibling threads**
```bash
#!/bin/bash
# Disable all HT sibling CPUs

echo "Disabling Hyper-Threading siblings..."

# Get list of sibling CPUs to disable
for cpu_dir in /sys/devices/system/cpu/cpu[0-9]*; do
    cpu=$(basename $cpu_dir)
    cpu_num=${cpu#cpu}
    
    # Skip CPU 0 (always online)
    [ "$cpu_num" -eq 0 ] && continue
    
    # Check if this is a sibling (thread_siblings_list has multiple CPUs)
    siblings=$(cat $cpu_dir/topology/thread_siblings_list 2>/dev/null)
    
    # If sibling list has comma, it's a HT sibling
    if [[ "$siblings" == *","* ]]; then
        # Disable the higher-numbered sibling
        first_sibling=$(echo $siblings | cut -d',' -f1)
        if [ "$cpu_num" -gt "$first_sibling" ]; then
            echo 0 | sudo tee $cpu_dir/online > /dev/null
            echo "  Disabled $cpu (sibling of cpu$first_sibling)"
        fi
    fi
done

echo "Done. Run 'lscpu' to verify."
```

**Option 2: Quick method (for 2-thread per core)**
```bash
# Disable every odd-numbered CPU (assumes CPU0,1 share core, CPU2,3 share core, etc.)
for cpu in $(seq 1 2 $(nproc --all)); do
    echo 0 | sudo tee /sys/devices/system/cpu/cpu$cpu/online
done
```

#### Verify HT Status:

**Check if HT is enabled:**
```bash
# Method 1: lscpu
lscpu | grep -E "^CPU\(s\)|^Thread"
# CPU(s): 12
# Thread(s) per core: 2  ← HT enabled
# CPU(s): 6
# Thread(s) per core: 1  ← HT disabled

# Method 2: Direct check
cat /sys/devices/system/cpu/cpu*/topology/thread_siblings_list | sort -u | wc -l
# If output equals CPU count, HT is disabled

# Method 3: Check online siblings
grep -H . /sys/devices/system/cpu/cpu*/online
# All odd CPUs showing "0" = HT siblings disabled
```

**Topology visualization:**
```bash
# Install hwloc
sudo apt install hwloc

# Graphical view
lstopo

# Text view showing physical cores
lstopo-no-graphics --of console
```

#### Impact on CPU Numbering:

**Before (HT enabled, 6 physical cores):**
```
Physical Core 0: CPU0, CPU6  (siblings)
Physical Core 1: CPU1, CPU7  (siblings)
Physical Core 2: CPU2, CPU8  (siblings)
Physical Core 3: CPU3, CPU9  (siblings)
Physical Core 4: CPU4, CPU10 (siblings)
Physical Core 5: CPU5, CPU11 (siblings)
```

**After (HT disabled):**
```
Physical Core 0: CPU0  (CPU6 offline)
Physical Core 1: CPU1  (CPU7 offline)
Physical Core 2: CPU2  (CPU8 offline)
Physical Core 3: CPU3  (CPU9 offline)
Physical Core 4: CPU4  (CPU10 offline)
Physical Core 5: CPU5  (CPU11 offline)
```

**Important:** Adjust CPU pinning in your code after disabling HT!

#### Re-enable HT:

```bash
# Re-enable all CPUs
for cpu in /sys/devices/system/cpu/cpu*/online; do
    echo 1 | sudo tee $cpu
done

# Or reboot (if disabled in BIOS)
```

**Impact:** Halves available logical CPUs, but dramatically improves latency consistency.

---

### 16. IRQBalance → Disable

**Issue:** irqbalance daemon moves IRQs dynamically  
**Solution:** Disable it when using manual IRQ pinning

```bash
# Stop and disable
sudo systemctl stop irqbalance
sudo systemctl disable irqbalance

# Verify
systemctl status irqbalance
```

---

### 17. IOMMU → Consider Disabling

**Issue:** IOMMU (VT-d) adds latency to DMA operations  
**Trade-off:** Security vs latency

**Disable:**
```bash
GRUB_CMDLINE_LINUX_DEFAULT="... iommu=off intel_iommu=off"
```

⚠️ **Security risk:** Only disable on isolated trading networks.

---

### 18. Audit and Watchdog → Disable

**Issue:** Audit and watchdog subsystems cause interruptions

```bash
# Disable audit
GRUB_CMDLINE_LINUX_DEFAULT="... audit=0"

# Disable watchdog
echo 0 | sudo tee /proc/sys/kernel/watchdog
echo 0 | sudo tee /proc/sys/kernel/nmi_watchdog

# Make permanent
echo "kernel.watchdog=0" | sudo tee -a /etc/sysctl.conf
echo "kernel.nmi_watchdog=0" | sudo tee -a /etc/sysctl.conf
```

---

## Complete GRUB Configuration for HFT

Here's a comprehensive `/etc/default/grub` configuration:

```bash
GRUB_CMDLINE_LINUX_DEFAULT="quiet splash \
    isolcpus=4-11 \
    nohz_full=4-11 \
    rcu_nocbs=4-11 \
    intel_idle.max_cstate=0 \
    processor.max_cstate=0 \
    idle=poll \
    intel_pstate=disable \
    nosoftlockup \
    tsc=reliable \
    audit=0 \
    nmi_watchdog=0 \
    mce=ignore_ce \
    transparent_hugepage=never"
```

Apply:
```bash
sudo update-grub
sudo reboot
```

---

## CPU Layout Strategy

**Example for 12-core system (6 physical cores, 12 with HT):**

```
CPUs 0-3:   Interrupt handling (IRQs, kernel)
CPUs 4-5:   Management threads (logging, metrics)
CPUs 6-11:  Isolated trading cores (market data, order entry)
```

**Topology check:**
```bash
lstopo           # Graphical view (install: hwloc)
lscpu -e         # Text view
numactl --hardware
```

---

## NUMA Considerations

If you have multiple NUMA nodes:

```bash
# Check NUMA topology
numactl --hardware

# Pin process to NUMA node 0
numactl --cpunodebind=0 --membind=0 ./your_app

# In code: use numa_alloc_onnode()
```

---

## Verification

After tuning, re-run diagnostics:

```bash
./build/release/examples/utilities/hft_host_diag/hft_host_diag
```

Expected result: **0 warnings, all green!** ✅

---

## Latency Testing

Measure actual improvement:

```bash
# Before tuning
cyclictest -p 80 -t1 -n -i 10000 -l 10000

# After tuning (should see lower max latency)
```

---

## Reboot Persistence

To make changes survive reboot:

1. Add to `/etc/sysctl.conf` (network/memory settings)
2. Add to `/etc/rc.local` (hugepages, THP, governor)
3. Create systemd service for complex initialization

Example systemd service (`/etc/systemd/system/hft-tune.service`):

```ini
[Unit]
Description=HFT System Tuning
After=network.target

[Service]
Type=oneshot
ExecStart=/path/to/scripts/hft_tune.sh
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
```

Enable:
```bash
sudo systemctl enable hft-tune.service
sudo systemctl start hft-tune.service
```

---

## References

- [Linux Kernel Documentation - sysctl](https://www.kernel.org/doc/Documentation/sysctl/)
- [Red Hat Low Latency Tuning Guide](https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux_for_real_time/)
- [Intel Data Plane Development Kit (DPDK)](https://www.dpdk.org/)

---

## Warning

⚠️ **These optimizations prioritize latency over throughput and power efficiency.**

Only apply on dedicated HFT/trading systems. Not recommended for:
- Laptops (battery drain)
- General-purpose servers
- Virtual machines (some settings may not work)
- Shared/multi-tenant systems

