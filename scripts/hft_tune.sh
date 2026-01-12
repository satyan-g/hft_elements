#!/bin/bash
# scripts/hft_tune.sh - Apply all HFT system optimizations (runtime)
# See docs/PERFORMANCE_TUNING.md for details

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
echo "Run hft_host_diag to verify improvements:"
echo "  ./build/release/examples/utilities/hft_host_diag/hft_host_diag"

