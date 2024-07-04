#ifndef __PERF_CASE_H
#define __PERF_CASE_H

#include "perf_stat.h"

struct perf_case {
	const char *name;
	const char *desc;
	int (*init)(struct perf_case *p_case, int argc, char *argv[]);
	int (*exit)(struct perf_case *p_case);
	void (*func)(struct perf_case *p_case);
	void (*help)(struct perf_case *p_case);
	void* pri;
	struct perf_event *events;
	int event_num;
};

#endif