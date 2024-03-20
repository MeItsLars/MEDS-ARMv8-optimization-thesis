#include <asm/unistd.h>
#include <linux/perf_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "cyclecounter.h"

static int perf_fd = 0;
void enable_cyclecounter()
{
    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(struct perf_event_attr));
    pe.type = PERF_TYPE_HARDWARE;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = PERF_COUNT_HW_CPU_CYCLES;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;

    perf_fd = syscall(__NR_perf_event_open, &pe, 0, -1, -1, 0);

    ioctl(perf_fd, PERF_EVENT_IOC_RESET, 0);
    ioctl(perf_fd, PERF_EVENT_IOC_ENABLE, 0);
}

void disable_cyclecounter()
{
    ioctl(perf_fd, PERF_EVENT_IOC_DISABLE, 0);
    close(perf_fd);
}

uint64_t get_cyclecounter()
{
    long long cpu_cycles;
    ioctl(perf_fd, PERF_EVENT_IOC_DISABLE, 0);
    ssize_t read_count = read(perf_fd, &cpu_cycles, sizeof(cpu_cycles));
    if (read_count < 0)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }
    else if (read_count == 0)
    {
        /* Should not happen */
        printf("perf counter empty\n");
        exit(EXIT_FAILURE);
    }
    ioctl(perf_fd, PERF_EVENT_IOC_ENABLE, 0);
    return cpu_cycles;
}