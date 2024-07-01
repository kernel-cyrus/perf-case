#ifndef __PERF_TEST_H
#define __PERF_TEST_H

#include "perf_event.h"

typedef int (*perf_test_func)(struct perf_event* events, int event_num);

struct perf_test {
	const char *name;
	const char *desc;
	perf_test_func func;
};

#define PERF_TEST(_name, _func, _desc) \
	{.name = _name, .func = _func, .desc = _desc}

int perf_test_memset(struct perf_event *events, int event_num);
int perf_test_intcal(struct perf_event *events, int event_num);
int perf_test_bw_mem(struct perf_event *events, int event_num);

#endif