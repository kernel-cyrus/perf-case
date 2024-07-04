#ifndef __PERF_CASE_H
#define __PERF_CASE_H

#include "perf_stat.h"

struct perf_case {
	const char *name;
	const char *desc;
	int  (*init)(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[]);
	int  (*exit)(struct perf_case *p_case, struct perf_stat *p_stat);
	void (*func)(struct perf_case *p_case, struct perf_stat *p_stat);
	void (*help)(struct perf_case *p_case);
	void* pri;
	struct perf_event *events;
	int event_num;
};

struct perf_run {
	struct perf_case *p_case;
	struct perf_stat *stats;
	int stat_num;
	long max_dur;
	long min_dur;
	long avg_dur;
};

#endif