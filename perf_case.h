#ifndef __PERF_CASE_H
#define __PERF_CASE_H

#include "perf_stat.h"

typedef int (*perf_case_func)(struct perf_event* events, int event_num);

struct perf_case {
	const char *name;
	const char *desc;
	struct perf_event* events;
	int event_num;
	perf_case_func func;
};

#define PERF_CASE(_name, _func, _desc) \
	{.name = _name, .func = _func, .desc = _desc}

int perf_case_memset(struct perf_event *events, int event_num);
int perf_case_intcal(struct perf_event *events, int event_num);
int perf_case_bw_mem(struct perf_event *events, int event_num);

#endif