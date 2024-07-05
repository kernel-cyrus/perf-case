#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/mman.h>

#include "../perf_stat.h"
#include "../perf_case.h"

#define STATIC_BUF_SIZE		(4 * 4096)
#define DYNAMIC_BUF_SIZE	(128 * 1024 * 1024)

static struct perf_event memset_events[] = {
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES, "cache-refs"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"),
	PERF_EVENT(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, "page-faults")
};

static int memset_malloc_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	p_case->pri = malloc(DYNAMIC_BUF_SIZE);
	if (!p_case->pri)
		return ERROR;
	return SUCCESS;
}

static int memset_malloc_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	free(p_case->pri);
	return SUCCESS;
}

static void memset_malloc_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	memset(p_case->pri, 0xFF, DYNAMIC_BUF_SIZE);
}

PERF_CASE_DEFINE(memset_malloc) = {
	.name = "memset_malloc",
	.desc = "memset a malloc buffer.        (128MB)",
	.init = memset_malloc_init,
	.exit = memset_malloc_exit,
	.func = memset_malloc_func,
	.events = memset_events,
	.event_num = sizeof(memset_events) / sizeof(struct perf_event)
};

static void memset_malloc_func_x2(struct perf_case *p_case, struct perf_stat *p_stat)
{
	memset(p_case->pri, 0xFF, DYNAMIC_BUF_SIZE);
	perf_stat_begin(p_stat);
	memset(p_case->pri, 0xFF, DYNAMIC_BUF_SIZE);
	perf_stat_end(p_stat);
}

PERF_CASE_DEFINE(memset_malloc_x2) = {
	.name = "memset_malloc_x2",
	.desc = "memset a malloc buffer again.  (128MB)",
	.init = memset_malloc_init,
	.exit = memset_malloc_exit,
	.func = memset_malloc_func_x2,
	.events = memset_events,
	.event_num = sizeof(memset_events) / sizeof(struct perf_event),
	.inner_stat = true
};

static int memset_mmap_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	p_case->pri = mmap(NULL, DYNAMIC_BUF_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (!p_case->pri)
		return ERROR;
	return SUCCESS;
}

static int memset_mmap_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	munmap(p_case->pri, DYNAMIC_BUF_SIZE);
	return SUCCESS;
}

static void memset_mmap_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	memset(p_case->pri, 0xFF, DYNAMIC_BUF_SIZE);
}

PERF_CASE_DEFINE(memset_mmap) = {
	.name = "memset_mmap",
	.desc = "memset a mmap buffer.          (128MB)",
	.init = memset_mmap_init,
	.exit = memset_mmap_exit,
	.func = memset_mmap_func,
	.events = memset_events,
	.event_num = sizeof(memset_events) / sizeof(struct perf_event)
};

static char static_buf_bss[STATIC_BUF_SIZE];

static void memset_static_bss_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	memset(static_buf_bss, 0xFF, STATIC_BUF_SIZE);
}

PERF_CASE_DEFINE(memset_static_bss) = {
	.name = "memset_static_bss",
	.desc = "memset a static bss buffer.    (4KB)",
	.func = memset_static_bss_func,
	.events = memset_events,
	.event_num = sizeof(memset_events) / sizeof(struct perf_event)
};

static char static_buf_data[STATIC_BUF_SIZE] = {0x1};

static void memset_static_data_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	memset(static_buf_data, 0xFF, STATIC_BUF_SIZE);
}

PERF_CASE_DEFINE(memset_static_data) = {
	.name = "memset_static_data",
	.desc = "memset a static data buffer.   (4KB)",
	.func = memset_static_data_func,
	.events = memset_events,
	.event_num = sizeof(memset_events) / sizeof(struct perf_event)
};