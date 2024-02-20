#!/usr/bin/bash

cat /proc/cpuinfo | grep -q AMD && (
  rm CPU_restore_mode.sh

  echo "#!/usr/bin/bash" > CPU_restore_mode.sh

  chmod a+x CPU_restore_mode.sh

  tmp=$(cat /sys/devices/system/cpu/cpufreq/boost)
  echo "echo $tmp > /sys/devices/system/cpu/cpufreq/boost" >> CPU_restore_mode.sh

  for i in /sys/devices/system/cpu/cpu*/cpufreq
  do
    tmp=$(cat $i/scaling_governor)
    echo "echo $tmp > $i/scaling_governor" >> CPU_restore_mode.sh
  done

  echo 0 > /sys/devices/system/cpu/cpufreq/boost

#  for i in /sys/devices/system/cpu/cpu*/cpufreq
#  do
#    echo performance > $i/scaling_governor
#  done

  for i in /sys/devices/system/cpu/cpu*/cpufreq
  do
    echo userspace > $i/scaling_governor
    echo 1900000 > $i/scaling_setspeed
    echo 1900000 > $i/scaling_min_freq
    echo 1900000 > $i/scaling_max_freq
  done
)

