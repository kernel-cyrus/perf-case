#ifndef __PERF_EVENT_H
#define __PERF_EVENT_H

#include <stdint.h>
#include <stdint.h>
#include <time.h>
#include <linux/perf_event.h>

#define EVENT_NAME_LEN		32
#define STAT_NAME_LEN		32
#define MAX_PERF_EVENTS		6

#define PERF_STAT_BEGIN(name, events, event_num) 						\
	do {									\
		struct perf_stat *__perf_stat;					\
		__perf_stat = perf_stat_begin(name, events, event_num);

#define PERF_STAT_END()								\
		perf_stat_end(__perf_stat);					\
	} while (0)

struct perf_event {
	uint32_t type;
	uint64_t event_id;
	char event_name[EVENT_NAME_LEN];
};

struct perf_stat {
	char name[STAT_NAME_LEN];
	struct perf_event *events;
	uint64_t event_counts[MAX_PERF_EVENTS];
	int event_fds[MAX_PERF_EVENTS];
	int event_num;
	struct timespec start;
	struct timespec end;
};

/* perf event interfaces */
int perf_event_open(uint32_t type, uint64_t event_id);
int perf_event_start(int fd);
int perf_event_stop(int fd);
uint64_t perf_event_read(int fd);
int perf_event_close(int fd);

/* perf stat interfaces */
struct perf_stat* perf_stat_begin(const char* name, struct perf_event *events, int event_num);
void perf_stat_end(struct perf_stat *stat);

#endif