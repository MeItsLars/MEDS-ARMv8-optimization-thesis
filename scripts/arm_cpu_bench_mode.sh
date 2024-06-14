echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
echo 1500000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed

echo "Current scaling freq:"
cat /sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq
echo "ARM CPU freq measurement:"
vcgencmd measure_clock arm