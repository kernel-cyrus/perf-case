#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../perf_stat.h"
#include "../perf_case.h"

struct cpusimd_data {
	int num;
	int iterations;
};

static int opt_num = 1;
static int opt_iterations = 1000000;

static struct perf_option cpusimd_opts[] = {
	{{"num", optional_argument, NULL, 'n' }, "n:", "Operations per loop. (n: 1-6)"},
	{{"iterations", optional_argument, NULL, 'i' }, "i:", "Iteration loops. (default: 1000K)"},
};

static int cpusimd_getopt(struct perf_case* p_case, int opt)
{
	switch (opt) {
	case 'n':
		opt_num = atoi(optarg);
		break;
	case 'i':
		opt_iterations = atoi(optarg);
		break;
	default:
		return ERROR;
	}
	return SUCCESS;
}

static void use_it(unsigned char* a)
{
	static unsigned char* use;
	use = a;
}

static int cpusimd_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	struct cpusimd_data *p_data;
	
	p_case->data = malloc(sizeof(struct cpusimd_data));
	if (!p_case->data)
		return ERROR;

	p_data = (struct cpusimd_data*)p_case->data;

	p_data->num = opt_num;
	p_data->iterations = opt_iterations;

	printf("iterations: %d\n", p_data->iterations);
	printf("num per loop: %d\n", p_data->num);

	return SUCCESS;
}

static int cpusimd_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	free(p_case->data);
	return SUCCESS;
}

unsigned char a[16];
unsigned char b[16];
unsigned char c[16];

#pragma GCC push_options
#pragma GCC optimize ("O0")
/* NOTE: Each loop contains tw extra CMP,BNE instructions. */
static void cpusimd_add_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct cpusimd_data *p_data = (struct cpusimd_data*)p_case->data;
	int num = p_data->num;
	register int loops = p_data->iterations;

	memset(a, 1, sizeof(a));
	memset(b, 1, sizeof(b));
	memset(c, 0, sizeof(c));

	perf_stat_begin(p_stat);
	for (int i = 0; i < loops; i++)
		for (int j = 0; j < 16; j++)
			c[j] = a[j] + b[j];
	perf_stat_end(p_stat);

	use_it(c);
}
#pragma GCC pop_options

PERF_CASE_DEFINE(cpusimd_add) = {
	.name = "cpusimd_add",
	.desc = "simple add loop.",
	.init = cpusimd_init,
	.exit = cpusimd_exit,
	.func = cpusimd_add_func,
	.getopt = cpusimd_getopt,
	.opts = cpusimd_opts,
	.opts_num = sizeof(cpusimd_opts) / sizeof(struct perf_option),
	.inner_stat = true
};


#pragma GCC push_options
#pragma GCC optimize ("O0")
/* NOTE: Each loop contains three extra ADD,CMP,BNE instructions. */
static void cpusimd_mul_func(struct perf_case *p_case, struct perf_stat *p_stat)
{

}
#pragma GCC pop_options

PERF_CASE_DEFINE(cpusimd_mul) = {
	.name = "cpusimd_mul",
	.desc = "simple mul loop.",
	.init = cpusimd_init,
	.exit = cpusimd_exit,
	.func = cpusimd_mul_func,
	.getopt = cpusimd_getopt,
	.opts = cpusimd_opts,
	.opts_num = sizeof(cpusimd_opts) / sizeof(struct perf_option),
	.inner_stat = true
};
