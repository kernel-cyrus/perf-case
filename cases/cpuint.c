#include <stdio.h>
#include <stdlib.h>

#include "../perf_stat.h"
#include "../perf_case.h"

struct cpuint_data {
	int iterations;
};

static int opt_iterations = 10000;

static struct perf_option cpuint_opts[] = {
	{{"iterations", optional_argument, NULL, 'i' }, "i:", "Iteration loops."},
};

static int cpuint_getopt(struct perf_case* p_case, int opt)
{
	switch (opt) {
	case 'i':
		opt_iterations = atoi(optarg);
		break;
	default:
		return ERROR;
	}
	return SUCCESS;
}

static int cpuint_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	struct cpuint_data *p_data;
	
	p_case->data = malloc(sizeof(struct cpuint_data));
	if (!p_case->data)
		return ERROR;

	p_data = (struct cpuint_data*)p_case->data;

	p_data->iterations = opt_iterations;

	printf("iterations: %d\n", p_data->iterations);

	return SUCCESS;
}

static int cpuint_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	free(p_case->data);
	return SUCCESS;
}

static void cpuint_add_1x_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct cpuint_data *p_data = (struct cpuint_data*)p_case->data;

	register int i;
	register int loops = p_data->iterations;
	register int a = 0;

	perf_stat_begin(p_stat);
	for (i = 0; i < loops; i++) {
		a += 1;
	}
	perf_stat_end(p_stat);
}

PERF_CASE_DEFINE(cpuint_add_1x) = {
	.name = "cpuint_add_1x",
	.desc = "loop add operation 10K times.",
	.init = cpuint_init,
	.exit = cpuint_exit,
	.func = cpuint_add_1x_func,
	.getopt = cpuint_getopt,
	.opts = cpuint_opts,
	.opts_num = sizeof(cpuint_opts) / sizeof(struct perf_option),
};