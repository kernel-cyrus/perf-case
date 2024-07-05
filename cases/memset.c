#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../perf_stat.h"
#include "../perf_case.h"

#define BUF_SIZE (128 * 1024 * 1024)

static struct perf_event memset_events[] = {
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES, "cache-refs"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"},
	{PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, "page-faults"}
};

static int memset_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	p_case->pri = malloc(BUF_SIZE);
	if (!p_case->pri)
		return ERROR;
	return SUCCESS;
}

static int memset_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	free(p_case->pri);
	return SUCCESS;
}

static void memset_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	memset(p_case->pri, 0xFF, BUF_SIZE);
}

struct perf_case case_memset = {
	.name = "memset",
	.desc = "memset a malloc buffer.",
	.init = memset_init,
	.exit = memset_exit,
	.func = memset_func,
	.events = memset_events,
	.event_num = sizeof(memset_events) / sizeof(struct perf_event)
};

static void memset_again_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	memset(p_case->pri, 0xFF, BUF_SIZE);
	perf_stat_begin(p_stat);
	memset(p_case->pri, 0xFF, BUF_SIZE);
	perf_stat_end(p_stat);
}

struct perf_case case_memset_again = {
	.name = "memset_again",
	.desc = "memset a malloc buffer again.",
	.init = memset_init,
	.exit = memset_exit,
	.func = memset_again_func,
	.events = memset_events,
	.event_num = sizeof(memset_events) / sizeof(struct perf_event),
	.inner_stat = true
};