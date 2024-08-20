#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../perf_stat.h"
#include "../perf_case.h"

struct cpuint_data {
	int num;
	int iterations;
};

static int opt_num = 1;
static int opt_iterations = 1000000;

static struct perf_option cpuint_opts[] = {
	{{"num", optional_argument, NULL, 'n' }, "n:", "Operations per loop. (n: 1-6)"},
	{{"iterations", optional_argument, NULL, 'i' }, "i:", "Iteration loops. (default: 1000K)"},
};

static int cpuint_getopt(struct perf_case* p_case, int opt)
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

static void use_it(int a)
{
	static int use = 0;
	use += a;
}

static int cpuint_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	struct cpuint_data *p_data;
	
	p_case->data = malloc(sizeof(struct cpuint_data));
	if (!p_case->data)
		return ERROR;

	p_data = (struct cpuint_data*)p_case->data;

	p_data->num = opt_num;
	p_data->iterations = opt_iterations;

	printf("num per loop: %d\n", p_data->num);
	printf("iterations: %d\n", p_data->iterations);

	return SUCCESS;
}

static int cpuint_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	free(p_case->data);
	return SUCCESS;
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
/* NOTE: Each loop contains two extra CMP,BNE instructions. */
static void cpuint_add_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct cpuint_data *p_data = (struct cpuint_data*)p_case->data;
	int num = p_data->num;
	register int loops = p_data->iterations;
	register int a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
	if (num < 1 || num > 6) {
		printf("ERROR: Only support n from 1 to 6.\n");
		exit(0);
	}
	perf_stat_begin(p_stat);
	if (num == 1) {
		while(a < loops) {
			a++;
		}
	} else if (num == 2) {
		while(a < loops) {
			a++; b++;
		}
	} else if (num == 3) {
		while(a < loops) {
			a++; b++; c++;
		}
	} else if (num == 4) {
		while(a < loops) {
			a++; b++; c++; d++;
		}
	} else if (num == 5) {
		while(a < loops) {
			a++; b++; c++; d++; e++;
		}
	} else if (num == 6) {
		while(a < loops) {
			a++; b++; c++; d++; e++; f++;
		}
	}
	perf_stat_end(p_stat);
	use_it(a);use_it(b);use_it(c);use_it(d);use_it(e);use_it(f);
}
#pragma GCC pop_options

PERF_CASE_DEFINE(cpuint_add) = {
	.name = "cpuint_add",
	.desc = "simple int add loop.",
	.init = cpuint_init,
	.exit = cpuint_exit,
	.func = cpuint_add_func,
	.getopt = cpuint_getopt,
	.opts = cpuint_opts,
	.opts_num = sizeof(cpuint_opts) / sizeof(struct perf_option),
	.inner_stat = true
};


#pragma GCC push_options
#pragma GCC optimize ("O0")
/* NOTE: Each loop contains three extra ADD,CMP,BNE instructions. */
static void cpuint_mul_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct cpuint_data *p_data = (struct cpuint_data*)p_case->data;
	int num = p_data->num;
	register int loops = p_data->iterations;
	register int i = 0, a = 1, b = 1, c = 1, d = 1, e = 1, f = 1;
	if (num < 1 || num > 6) {
		printf("ERROR: Only support n from 1 to 6.\n");
		exit(0);
	}
	perf_stat_begin(p_stat);
	if (num == 1) {
		while(i < loops) {
			i++; a *= i;
		}
	} else if (num == 2) {
		while(i < loops) {
			i++; a *= i; b *= i;
		}
	} else if (num == 3) {
		while(i < loops) {
			i++; a *= i; b *= i; c *= i;
		}
	} else if (num == 4) {
		while(i < loops) {
			i++; a *= i; b *= i; c *= i; d *= i;
		}
	} else if (num == 5) {
		while(i < loops) {
			i++; a *= i; b *= i; c *= i; d *= i; e *= i;
		}
	} else if (num == 6) {
		while(i < loops) {
			i++; a *= i; b *= i; c *= i; d *= i; e *= i; f *= i;
		}
	}
	perf_stat_end(p_stat);
	use_it(a);use_it(b);use_it(c);use_it(d);use_it(e);use_it(f);
}
#pragma GCC pop_options

PERF_CASE_DEFINE(cpuint_mul) = {
	.name = "cpuint_mul",
	.desc = "simple int mul loop.",
	.init = cpuint_init,
	.exit = cpuint_exit,
	.func = cpuint_mul_func,
	.getopt = cpuint_getopt,
	.opts = cpuint_opts,
	.opts_num = sizeof(cpuint_opts) / sizeof(struct perf_option),
	.inner_stat = true
};
