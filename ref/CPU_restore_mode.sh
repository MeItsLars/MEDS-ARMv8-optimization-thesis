#!/usr/bin/bash
echo 1 > /sys/devices/system/cpu/cpufreq/boost
echo schedutil > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu10/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu11/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu12/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu13/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu14/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu15/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu1/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu2/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu3/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu4/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu5/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu6/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu7/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu8/cpufreq/scaling_governor
echo schedutil > /sys/devices/system/cpu/cpu9/cpufreq/scaling_governor
