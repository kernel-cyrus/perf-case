#ifndef __PERF_CASE_H
#define __PERF_CASE_H

#include "perf_stat.h"

#define PERF_CASE_DEFINE(_name)	\
	struct perf_case __perf_case_##_name

#define PERF_CASE_DECLARE(_name) \
	extern struct perf_case __perf_case_##_name

#define PERF_CASE(_name) \
	&__perf_case_##_name

struct perf_case {
	const char *name;
	const char *desc;
	int  (*init)(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[]);
	int  (*exit)(struct perf_case *p_case, struct perf_stat *p_stat);
	void (*func)(struct perf_case *p_case, struct perf_stat *p_stat);
	void (*help)(struct perf_case *p_case);
	void* data;
	struct perf_event *events;
	int event_num;
	int inner_stat;
};

struct perf_run {
	struct perf_case *p_case;
	struct perf_stat *stats;
	int stat_num;
	long max_dur;
	long min_dur;
	long avg_dur;
};

PERF_CASE_DECLARE(memset_malloc);
PERF_CASE_DECLARE(memset_malloc_x2);
PERF_CASE_DECLARE(memset_mmap);
PERF_CASE_DECLARE(memset_static_bss);
PERF_CASE_DECLARE(memset_static_data);
PERF_CASE_DECLARE(membw_rd_1);
PERF_CASE_DECLARE(membw_rd_4);
PERF_CASE_DECLARE(membw_rd_8);
PERF_CASE_DECLARE(membw_rd_1_4x);
PERF_CASE_DECLARE(membw_rd_4_4x);
PERF_CASE_DECLARE(membw_rd_8_4x);
PERF_CASE_DECLARE(membw_wr_1);
PERF_CASE_DECLARE(membw_wr_4);
PERF_CASE_DECLARE(membw_wr_8);
PERF_CASE_DECLARE(membw_wr_1_4x);
PERF_CASE_DECLARE(membw_wr_4_4x);
PERF_CASE_DECLARE(membw_wr_8_4x);
PERF_CASE_DECLARE(membw_cp_1);
PERF_CASE_DECLARE(membw_cp_4);
PERF_CASE_DECLARE(membw_cp_8);
PERF_CASE_DECLARE(membw_cp_1_4x);
PERF_CASE_DECLARE(membw_cp_4_4x);
PERF_CASE_DECLARE(membw_cp_8_4x);

#endif