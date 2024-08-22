#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arm_neon.h>

#include "perf_stat.h"
#include "perf_case.h"

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

static int cpusimd_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	struct cpusimd_data *p_data;
	
	p_case->data = malloc(sizeof(struct cpusimd_data));
	if (!p_case->data)
		return ERROR;

	p_data = (struct cpusimd_data*)p_case->data;

	p_data->num = opt_num;
	p_data->iterations = opt_iterations;

	printf("num per loop: %d\n", p_data->num);
	printf("iterations: %d\n", p_data->iterations);

	return SUCCESS;
}

static int cpusimd_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	free(p_case->data);
	return SUCCESS;
}

static float32x4_t a = {1.0, 1.0, 1.0, 1.0};
static float32x4_t b = {2.0, 2.0, 2.0, 2.0};
static float32x4_t c = {3.0, 3.0, 3.0, 3.0};
static float32x4_t d = {4.0, 4.0, 4.0, 4.0};
static float32x4_t e = {5.0, 5.0, 5.0, 5.0};
static float32x4_t f = {6.0, 6.0, 6.0, 6.0};
static float32x4_t x = {1.1, 1.1, 1.1, 1.1};

/* NOTE: Each loop contains n FADD.VF32 and ADD,CMP,BNE. */
static void cpusimd_add_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct cpusimd_data *p_data = (struct cpusimd_data*)p_case->data;
	int num = p_data->num;
	register int i = 0, loops = p_data->iterations;
    	float output[4];
	if (num < 1 || num > 6) {
		printf("ERROR: Only support n from 1 to 6.\n");
		exit(0);
	}
	perf_stat_begin(p_stat);
	if (num == 1) {
		while(i < loops) {
			i++;
			a = vaddq_f32(a, x);
		}
	} else if (num == 2) {
		while(i < loops) {
			i++;
			a = vaddq_f32(a, x);
			b = vaddq_f32(b, x);
		}
	} else if (num == 3) {
		while(i < loops) {
			i++;
			a = vaddq_f32(a, x);
			b = vaddq_f32(b, x);
			c = vaddq_f32(c, x);
		}
	} else if (num == 4) {
		while(i < loops) {
			i++;
			a = vaddq_f32(a, x);
			b = vaddq_f32(b, x);
			c = vaddq_f32(c, x);
			d = vaddq_f32(d, x);
		}
	} else if (num == 5) {
		while(i < loops) {
			i++;
			a = vaddq_f32(a, x);
			b = vaddq_f32(b, x);
			c = vaddq_f32(c, x);
			d = vaddq_f32(d, x);
			e = vaddq_f32(e, x);
		}
	} else if (num == 6) {
		while(i < loops) {
			i++;
			a = vaddq_f32(a, x);
			b = vaddq_f32(b, x);
			c = vaddq_f32(c, x);
			d = vaddq_f32(d, x);
			e = vaddq_f32(e, x);
			f = vaddq_f32(f, x);
		}
	}
	perf_stat_end(p_stat);
	/* Consume the data to avoid compiler optimizing. */
	vst1q_f32(output, a);
	vst1q_f32(output, b);
	vst1q_f32(output, c);
	vst1q_f32(output, d);
	vst1q_f32(output, e);
	vst1q_f32(output, f);
}

PERF_CASE_DEFINE(cpusimd_add) = {
	.name = "cpusimd_add",
	.desc = "simple simd add loop.",
	.init = cpusimd_init,
	.exit = cpusimd_exit,
	.func = cpusimd_add_func,
	.getopt = cpusimd_getopt,
	.opts = cpusimd_opts,
	.opts_num = sizeof(cpusimd_opts) / sizeof(struct perf_option),
	.inner_stat = true
};


/* NOTE: Each loop contains n FMUL.VF32 and ADD,CMP,BNE. */
static void cpusimd_mul_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct cpusimd_data *p_data = (struct cpusimd_data*)p_case->data;
	int num = p_data->num;
	register int i = 0, loops = p_data->iterations;
    	float output[4];
	if (num < 1 || num > 6) {
		printf("ERROR: Only support n from 1 to 6.\n");
		exit(0);
	}
	perf_stat_begin(p_stat);
	if (num == 1) {
		while(i < loops) {
			i++;
			a = vmulq_f32(a, x);
		}
	} else if (num == 2) {
		while(i < loops) {
			i++;
			a = vmulq_f32(a, x);
			b = vmulq_f32(b, x);
		}
	} else if (num == 3) {
		while(i < loops) {
			i++;
			a = vmulq_f32(a, x);
			b = vmulq_f32(b, x);
			c = vmulq_f32(c, x);
		}
	} else if (num == 4) {
		while(i < loops) {
			i++;
			a = vmulq_f32(a, x);
			b = vmulq_f32(b, x);
			c = vmulq_f32(c, x);
			d = vmulq_f32(d, x);
		}
	} else if (num == 5) {
		while(i < loops) {
			i++;
			a = vmulq_f32(a, x);
			b = vmulq_f32(b, x);
			c = vmulq_f32(c, x);
			d = vmulq_f32(d, x);
			e = vmulq_f32(e, x);
		}
	} else if (num == 6) {
		while(i < loops) {
			i++;
			a = vmulq_f32(a, x);
			b = vmulq_f32(b, x);
			c = vmulq_f32(c, x);
			d = vmulq_f32(d, x);
			e = vmulq_f32(e, x);
			f = vmulq_f32(f, x);
		}
	}
	perf_stat_end(p_stat);
	/* Consume the data to avoid compiler optimizing. */
	vst1q_f32(output, a);
	vst1q_f32(output, b);
	vst1q_f32(output, c);
	vst1q_f32(output, d);
	vst1q_f32(output, e);
	vst1q_f32(output, f);
}

PERF_CASE_DEFINE(cpusimd_mul) = {
	.name = "cpusimd_mul",
	.desc = "simple simd mul loop.",
	.init = cpusimd_init,
	.exit = cpusimd_exit,
	.func = cpusimd_mul_func,
	.getopt = cpusimd_getopt,
	.opts = cpusimd_opts,
	.opts_num = sizeof(cpusimd_opts) / sizeof(struct perf_option),
	.inner_stat = true
};
