#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "perf_stat.h"
#include "perf_case.h"

struct ustress_data {
	int iterations;
};

static int opt_iterations = 0;

static struct perf_option ustress_opts[] = {
	{{"iterations", optional_argument, NULL, 'i' }, "i:", "Iteration loops."},
};

static int ustress_getopt(struct perf_case* p_case, int opt)
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

static int ustress_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	struct ustress_data *p_data;
	
	p_case->data = malloc(sizeof(struct ustress_data));
	if (!p_case->data)
		return ERROR;

	p_data = (struct ustress_data*)p_case->data;
	p_data->iterations = opt_iterations;

	return SUCCESS;
}

static int ustress_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	free(p_case->data);
	return SUCCESS;
}

#define DEFINE_USTRESS_CASE(_name, _runs)						\
											\
extern void ustress_##_name(long runs);							\
											\
static void ustress_##_name##_func(struct perf_case *p_case, struct perf_stat *p_stat)	\
{											\
	struct ustress_data *p_data = (struct ustress_data*)p_case->data;		\
	int iterations = p_data->iterations > 0 ? p_data->iterations : _runs;		\
	printf("iterations: %d\n", iterations);						\
	perf_stat_begin(p_stat);							\
	ustress_##_name(iterations);							\
	perf_stat_end(p_stat);								\
}											\
											\
PERF_CASE_DEFINE(ustress_##_name) = {							\
	.name = "ustress_" # _name,							\
	.desc = "ARM ustress/" # _name "_workload",					\
	.init = ustress_init,								\
	.exit = ustress_exit,								\
	.func = ustress_##_name##_func,							\
	.getopt = ustress_getopt,							\
	.opts = ustress_opts,								\
	.opts_num = sizeof(ustress_opts) / sizeof(struct perf_option),			\
	.inner_stat = true								\
};

DEFINE_USTRESS_CASE(branch_direct, 20000000);
DEFINE_USTRESS_CASE(branch_indirect, 3000000);
DEFINE_USTRESS_CASE(call_return, 15000);
DEFINE_USTRESS_CASE(div32, 200000000);
DEFINE_USTRESS_CASE(div64, 200000000);
DEFINE_USTRESS_CASE(double2int, 1500000000);
DEFINE_USTRESS_CASE(fpdiv, 120000000);
DEFINE_USTRESS_CASE(fpmac, 200000000);
DEFINE_USTRESS_CASE(fpmul, 260000000);
DEFINE_USTRESS_CASE(fpsqrt, 120000000);
DEFINE_USTRESS_CASE(int2double, 1500000000);
DEFINE_USTRESS_CASE(isb, 2800);
DEFINE_USTRESS_CASE(l1d_cache, 440000);
DEFINE_USTRESS_CASE(l1d_tlb, 1300000);
DEFINE_USTRESS_CASE(l1i_cache, 8000000);
DEFINE_USTRESS_CASE(l2d_cache, 4000);
DEFINE_USTRESS_CASE(load_after_store, 2300000);
DEFINE_USTRESS_CASE(mac32, 400000000);
DEFINE_USTRESS_CASE(mac64, 330000000);
DEFINE_USTRESS_CASE(memcpy, 2200000);
DEFINE_USTRESS_CASE(mul32, 400000000);
DEFINE_USTRESS_CASE(mul64, 330000000);
DEFINE_USTRESS_CASE(store_buffer_full, 30000000);