#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../perf_stat.h"
#include "../perf_case.h"

struct cpufp_data {
	int num;
	int iterations;
};

static int opt_num = 1;
static int opt_iterations = 1000000;

static struct perf_option cpufp_opts[] = {
	{{"num", optional_argument, NULL, 'n' }, "n:", "Operations per loop. (n: 1-6)"},
	{{"iterations", optional_argument, NULL, 'i' }, "i:", "Iteration loops. (default: 1000K)"},
};

static int cpufp_getopt(struct perf_case* p_case, int opt)
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

static int cpufp_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	struct cpufp_data *p_data;
	
	p_case->data = malloc(sizeof(struct cpufp_data));
	if (!p_case->data)
		return ERROR;

	p_data = (struct cpufp_data*)p_case->data;

	p_data->num = opt_num;
	p_data->iterations = opt_iterations;

	printf("num per loop: %d\n", p_data->num);
	printf("iterations: %d\n", p_data->iterations);

	return SUCCESS;
}

static int cpufp_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	free(p_case->data);
	return SUCCESS;
}

#pragma GCC push_options
#pragma GCC optimize ("O0")
/* NOTE: INT & FP Instructions can run parallel, each loop contains ADD,CMP,FMOV,FADD,B */
static void cpufp_add_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct cpufp_data *p_data = (struct cpufp_data*)p_case->data;
	int num = p_data->num;
	register int i = 0, loops = p_data->iterations;
	register double a = 0, b = 0, c = 0, d = 0, e = 0, f = 0;
	if (num < 1 || num > 6) {
		printf("ERROR: Only support n from 1 to 6.\n");
		exit(0);
	}
	perf_stat_begin(p_stat);
	if (num == 1) {
		while(i < loops) {
			i++; a++;
		}
	} else if (num == 2) {
		while(i < loops) {
			i++; a++; b++;
		}
	} else if (num == 3) {
		while(i < loops) {
			i++; a++; b++; c++;
		}
	} else if (num == 4) {
		while(i < loops) {
			i++; a++; b++; c++; d++;
		}
	} else if (num == 5) {
		while(i < loops) {
			i++; a++; b++; c++; d++; e++;
		}
	} else if (num == 6) {
		while(i < loops) {
			i++; a++; b++; c++; d++; e++; f++;
		}
	}
	perf_stat_end(p_stat);
	use_it(a);use_it(b);use_it(c);use_it(d);use_it(e);use_it(f);
}
#pragma GCC pop_options

PERF_CASE_DEFINE(cpufp_add) = {
	.name = "cpufp_add",
	.desc = "simple fp add loop.",
	.init = cpufp_init,
	.exit = cpufp_exit,
	.func = cpufp_add_func,
	.getopt = cpufp_getopt,
	.opts = cpufp_opts,
	.opts_num = sizeof(cpufp_opts) / sizeof(struct perf_option),
	.inner_stat = true
};


#pragma GCC push_options
#pragma GCC optimize ("O0")
/* NOTE: INT & FP Instructions can run parallel, each loop contains ADD,CMP,FMOV,FMUL,B */
static void cpufp_mul_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct cpufp_data *p_data = (struct cpufp_data*)p_case->data;
	int num = p_data->num;
	register int i = 0, loops = p_data->iterations;
	register double a = 1, b = 1, c = 1, d = 1, e = 1, f = 1;
	if (num < 1 || num > 6) {
		printf("ERROR: Only support n from 1 to 6.\n");
		exit(0);
	}
	perf_stat_begin(p_stat);
	if (num == 1) {
		while(i < loops) {
			i++; a *= 3;
		}
	} else if (num == 2) {
		while(i < loops) {
			i++; a *= 3; b *= 3;
		}
	} else if (num == 3) {
		while(i < loops) {
			i++; a *= 3; b *= 3; c *= 3;
		}
	} else if (num == 4) {
		while(i < loops) {
			i++; a *= 3; b *= 3; c *= 3; d *= 3;
		}
	} else if (num == 5) {
		while(i < loops) {
			i++; a *= 3; b *= 3; c *= 3; d *= 3; e *= 3;
		}
	} else if (num == 6) {
		while(i < loops) {
			i++; a *= 3; b *= 3; c *= 3; d *= 3; e *= 3; f *= 3;
		}
	}
	perf_stat_end(p_stat);
	use_it(a);use_it(b);use_it(c);use_it(d);use_it(e);use_it(f);
}
#pragma GCC pop_options

PERF_CASE_DEFINE(cpufp_mul) = {
	.name = "cpufp_mul",
	.desc = "simple fp mul loop.",
	.init = cpufp_init,
	.exit = cpufp_exit,
	.func = cpufp_mul_func,
	.getopt = cpufp_getopt,
	.opts = cpufp_opts,
	.opts_num = sizeof(cpufp_opts) / sizeof(struct perf_option),
	.inner_stat = true
};
