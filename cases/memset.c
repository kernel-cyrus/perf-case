#include <stdlib.h>
#include <string.h>

#include "../perf_stat.h"
#include "../perf_case.h"

#define BUF_SIZE (128 * 1024 * 1024)

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

struct perf_case __perf_case_memset = {
	.name = "memset",
	.desc = "Test memset on various memory types.",
	.init = memset_init,
	.exit = memset_exit,
	.func = memset_func
};