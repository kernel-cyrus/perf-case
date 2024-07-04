#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>

#include "perf_stat.h"

int __perf_event_open(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags)
{
	return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

int perf_event_open(uint32_t type, uint64_t event_id)
{
	struct perf_event_attr attr;

	memset(&attr, 0, sizeof(struct perf_event_attr));

	attr.size = sizeof(struct perf_event_attr);
	attr.type = type;
	attr.config = event_id;
	attr.disabled = 1;

	return __perf_event_open(&attr, 0, -1, -1, 0);
}

int perf_event_start(int fd)
{
	return ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
}

int perf_event_stop(int fd)
{
	return ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
}

uint64_t perf_event_read(int fd)
{
	int ret;
	uint64_t count;

	ret = read(fd, &count, sizeof(count));
	if (ret < 0)
		return 0;

	return count;
}

int perf_event_close(int fd)
{
	return close(fd);
}

void perf_simple_stat()
{
	int fd;
	uint64_t count;

	fd = perf_event_open(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_CLOCK);
	if (fd < 0) {
		printf("ERROR: Event not supported.");
		exit(-1);
	}

	perf_event_start(fd);
	sleep(1);
	perf_event_stop(fd);

	count = perf_event_read(fd);
	printf("count: %ld\n", count);

	perf_event_close(fd);
}

int perf_stat_init(struct perf_stat *stat, const char* name, struct perf_event *events, int event_num)
{
	if (!stat || !name || !events)
		return ERROR;

	if (event_num > MAX_PERF_EVENTS)
		return ERROR;

	memset(stat, 0, sizeof(struct perf_stat));
	stat->events = events;
	stat->event_num = event_num;
	strncpy(stat->name, name, sizeof(stat->name) - 1);

	return SUCCESS;
}

void perf_stat_begin(struct perf_stat *stat)
{
	for (int i = 0; i < stat->event_num; i++)
		stat->event_fds[i] = perf_event_open(stat->events[i].type, stat->events[i].event_id);

	clock_gettime(CLOCK_MONOTONIC, &stat->start);

	for (int i = 0; i < stat->event_num; i++)
		if (stat->event_fds[i] > 0)
			perf_event_start(stat->event_fds[i]);
}

void perf_stat_end(struct perf_stat *stat)
{
	long secs, nano;

	for (int i = 0; i < stat->event_num; i++)
		if (stat->event_fds[i] > 0)
			perf_event_stop(stat->event_fds[i]);

	clock_gettime(CLOCK_MONOTONIC, &stat->end);
	secs = stat->end.tv_sec - stat->start.tv_sec;
	nano = stat->end.tv_nsec - stat->start.tv_nsec;
	stat->duration = secs * 1000000000L + nano;

	for (int i = 0; i < stat->event_num; i++) {
		if (stat->event_fds[i] > 0) {
			stat->event_counts[i] = perf_event_read(stat->event_fds[i]);
			perf_event_close(stat->event_fds[i]);
		}
	}
}

void perf_stat_report(struct perf_stat *stat)
{
	printf("TEST: %s\n", stat->name);
	printf("-----------------------\n");
	for (int i = 0; i < stat->event_num; i++)
		printf("%*s: %*ld\n", 16, stat->events[i].event_name, 16, stat->event_counts[i]);
	printf("-----------------------\n");
	printf("Time spent: %f ms\n\n", (double)stat->duration / 1000000);
}