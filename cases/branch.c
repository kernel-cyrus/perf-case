#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "perf_stat.h"
#include "perf_case.h"

struct branch_data {
	int num;
	int pattern;
	int iterations;
};

typedef enum {
	BR_PAT_FIXED,
	BR_PAT_RANDOM
} branch_pattern;

static int opt_num = 1;
static int opt_pattern = BR_PAT_FIXED;
static int opt_iterations = 1000000;

static struct perf_option branch_next_opts[] = {
	{{"num", optional_argument, NULL, 'n' }, "n:", "Branches per loop. n = 2^[0~14]"},
	{{"iterations", optional_argument, NULL, 'i' }, "i:", "Iteration loops. (default: 1000K)"},
};

static struct perf_option branch_pred_opts[] = {
	{{"pattern", optional_argument, NULL, 'p' }, "p:", "Branch pattern: fixed | random"},
	{{"iterations", optional_argument, NULL, 'i' }, "i:", "Iteration loops. (default: 1000K)"},
};

static int branch_getopt(struct perf_case* p_case, int opt)
{
	switch (opt) {
	case 'n':
		opt_num = atoi(optarg);
		break;
	case 'p':
		if (!strcmp(optarg, "fixed")) {
			opt_pattern = BR_PAT_FIXED;
		} else if (!strcmp(optarg, "random")) {
			opt_pattern = BR_PAT_RANDOM;
		} else {
			printf("ERROR: Invalid pattern, only support \"fixed\" or \"random\".\n");
			exit(0);
		}
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

static int branch_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	struct branch_data *p_data;
	
	p_case->data = malloc(sizeof(struct branch_data));
	if (!p_case->data)
		return ERROR;

	p_data = (struct branch_data*)p_case->data;

	p_data->num = opt_num;
	p_data->pattern = opt_pattern;
	p_data->iterations = opt_iterations;

	return SUCCESS;
}

static int branch_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	free(p_case->data);
	return SUCCESS;
}

#define JUMP1(pri, maj, min, sub)				\
	"b NEXT_BRANCH_" #pri "_" #maj "_" #min "_" #sub "\n"	\
	"NEXT_BRANCH_" #pri "_"  #maj "_" #min "_" #sub ":\n"

#define JUMP16(pri, maj, min)					\
	JUMP1(pri, maj, min, 0x0) JUMP1(pri, maj, min, 0x1)	\
	JUMP1(pri, maj, min, 0x2) JUMP1(pri, maj, min, 0x3)	\
	JUMP1(pri, maj, min, 0x4) JUMP1(pri, maj, min, 0x5)	\
	JUMP1(pri, maj, min, 0x6) JUMP1(pri, maj, min, 0x7)	\
	JUMP1(pri, maj, min, 0x8) JUMP1(pri, maj, min, 0x9)	\
	JUMP1(pri, maj, min, 0xA) JUMP1(pri, maj, min, 0xB)	\
	JUMP1(pri, maj, min, 0xC) JUMP1(pri, maj, min, 0xD)	\
	JUMP1(pri, maj, min, 0xE) JUMP1(pri, maj, min, 0xF)

#define JUMP256(pri, maj)					\
	JUMP16(pri, maj, 0x0) JUMP16(pri, maj, 0x1)		\
	JUMP16(pri, maj, 0x2) JUMP16(pri, maj, 0x3)		\
	JUMP16(pri, maj, 0x4) JUMP16(pri, maj, 0x5)		\
	JUMP16(pri, maj, 0x6) JUMP16(pri, maj, 0x7)		\
	JUMP16(pri, maj, 0x8) JUMP16(pri, maj, 0x9)		\
	JUMP16(pri, maj, 0xA) JUMP16(pri, maj, 0xB)		\
	JUMP16(pri, maj, 0xC) JUMP16(pri, maj, 0xD)		\
	JUMP16(pri, maj, 0xE) JUMP16(pri, maj, 0xF)

#define JUMP4096(pri)						\
	JUMP256(pri, 0x0) JUMP256(pri, 0x1)			\
	JUMP256(pri, 0x2) JUMP256(pri, 0x3)			\
	JUMP256(pri, 0x4) JUMP256(pri, 0x5)			\
	JUMP256(pri, 0x6) JUMP256(pri, 0x7)			\
	JUMP256(pri, 0x8) JUMP256(pri, 0x9)			\
	JUMP256(pri, 0xA) JUMP256(pri, 0xB)			\
	JUMP256(pri, 0xC) JUMP256(pri, 0xD)			\
	JUMP256(pri, 0xE) JUMP256(pri, 0xF)

#pragma GCC push_options
#pragma GCC optimize ("O0")
static void branch_next_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct branch_data *p_data = (struct branch_data*)p_case->data;
	int num = p_data->num, err = 0;
	register int i = 0, loops = p_data->iterations;
	printf("branches per loop: %d\n", p_data->num);
	printf("iterations: %d\n", p_data->iterations);
	perf_stat_begin(p_stat);
	if (num == 1) {
		while (i < loops) {
			__asm__ volatile (
				JUMP1(0x0, 0x0, 0x0, 0x0)
			);
			i++;
		}
	} else if (num == 2) {
		while (i < loops) {
			__asm__ volatile (
				JUMP1(0x0, 0x0, 0x0, 0x1)
				JUMP1(0x0, 0x0, 0x0, 0x2)
			);
			i++;
		}
	} else if (num == 4) {
		while (i < loops) {
			__asm__ volatile (
				JUMP1(0x0, 0x0, 0x0, 0x3)
				JUMP1(0x0, 0x0, 0x0, 0x4)
				JUMP1(0x0, 0x0, 0x0, 0x5)
				JUMP1(0x0, 0x0, 0x0, 0x6)
			);
			i++;
		}
	} else if (num == 8) {
		while (i < loops) {
			__asm__ volatile (
				JUMP1(0x0, 0x0, 0x0, 0x7)
				JUMP1(0x0, 0x0, 0x0, 0x8)
				JUMP1(0x0, 0x0, 0x0, 0x9)
				JUMP1(0x0, 0x0, 0x0, 0xA)
				JUMP1(0x0, 0x0, 0x0, 0xB)
				JUMP1(0x0, 0x0, 0x0, 0xC)
				JUMP1(0x0, 0x0, 0x0, 0xD)
				JUMP1(0x0, 0x0, 0x0, 0xE)
			);
			i++;
		}
	} else if (num == 16) {
		while (i < loops) {
			__asm__ volatile (
				JUMP16(0x0, 0x0, 0x1)
			);
			i++;
		}
	} else if (num == 32) {
		while (i < loops) {
			__asm__ volatile (
				JUMP16(0x0, 0x0, 0x2)
				JUMP16(0x0, 0x0, 0x3)
			);
			i++;
		}
	} else if (num == 64) {
		while (i < loops) {
			__asm__ volatile (
				JUMP16(0x0, 0x0, 0x4)
				JUMP16(0x0, 0x0, 0x5)
				JUMP16(0x0, 0x0, 0x6)
				JUMP16(0x0, 0x0, 0x7)
			);
			i++;
		}
	} else if (num == 128) {
		while (i < loops) {
			__asm__ volatile (
				JUMP16(0x0, 0x0, 0x8)
				JUMP16(0x0, 0x0, 0x9)
				JUMP16(0x0, 0x0, 0xA)
				JUMP16(0x0, 0x0, 0xB)
				JUMP16(0x0, 0x0, 0xC)
				JUMP16(0x0, 0x0, 0xD)
				JUMP16(0x0, 0x0, 0xE)
				JUMP16(0x0, 0x0, 0xF)
			);
			i++;
		}
	} else if (num == 256) {
		while (i < loops) {
			__asm__ volatile (
				JUMP256(0x0, 0x1)
			);
			i++;
		}
	} else if (num == 512) {
		while (i < loops) {
			__asm__ volatile (
				JUMP256(0x0, 0x2)
				JUMP256(0x0, 0x3)
			);
			i++;
		}
	} else if (num == 1024) {
		while (i < loops) {
			__asm__ volatile (
				JUMP256(0x0, 0x4)
				JUMP256(0x0, 0x5)
				JUMP256(0x0, 0x6)
				JUMP256(0x0, 0x7)
			);
			i++;
		}
	} else if (num == 2048) {
		while (i < loops) {
			__asm__ volatile (
				JUMP256(0x0, 0x8)
				JUMP256(0x0, 0x9)
				JUMP256(0x0, 0xA)
				JUMP256(0x0, 0xB)
				JUMP256(0x0, 0xC)
				JUMP256(0x0, 0xD)
				JUMP256(0x0, 0xE)
				JUMP256(0x0, 0xF)
			);
			i++;
		}
	} else if (num == 4096) {
		while (i < loops) {
			__asm__ volatile (
				
				JUMP4096(0x1)
			);
			i++;
		}
	} else if (num == 8192) {
		while (i < loops) {
			__asm__ volatile (
				JUMP4096(0x2)
				JUMP4096(0x3)
			);
			i++;
		}
	} else if (num == 16384) {
		while (i < loops) {
			__asm__ volatile (
				JUMP4096(0x4)
				JUMP4096(0x5)
				JUMP4096(0x6)
				JUMP4096(0x7)
			);
			i++;
		}
	} else {
		err = 1;
	}
	perf_stat_end(p_stat);
	if (err) {
		printf("ERROR: Only support n = 2^[0~14]\n");
		exit(0);
	}
	/* Consume the data to avoid compiler optimizing. */
	use_it(i);
}
#pragma GCC pop_options

PERF_CASE_DEFINE(branch_next) = {
	.name = "branch_next",
	.desc = "jump to next instruction for n times.",
	.init = branch_init,
	.exit = branch_exit,
	.func = branch_next_func,
	.getopt = branch_getopt,
	.opts = branch_next_opts,
	.opts_num = sizeof(branch_next_opts) / sizeof(struct perf_option),
	.inner_stat = true
};

#pragma GCC push_options
#pragma GCC optimize ("O0")
static void branch_pred_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct branch_data *p_data = (struct branch_data*)p_case->data;
	int pattern = p_data->pattern; 
	register int i = 0, loops = p_data->iterations;
	register int sum = 0;
	register char* deciecions;
	deciecions = malloc(loops);
	if (!deciecions) {
		printf("ERROR: malloc failed.\n");
		exit(0);
	}
	if (pattern == BR_PAT_FIXED) {
		printf("br pattern: fixed\n");
		for (i = 0; i < loops; i++)
			deciecions[i] = i % 2;
	} else if (pattern == BR_PAT_RANDOM) {
		printf("br pattern: random\n");
		srand(1);
		for (i = 0; i < loops; i++)
			deciecions[i] = random() % 2;
	} else {
		printf("ERROR: Invalid pattern, only support \"fixed\" or \"random\".\n");
		free(deciecions);
		exit(0);
	}
	printf("iterations: %d\n", p_data->iterations);
	perf_stat_begin(p_stat);
	for (i = 0; i < loops; i++)
		if (deciecions[i])
			sum++;
	perf_stat_end(p_stat);
	printf("pred: %d, taken: %d\n", i, sum);
	free(deciecions);
}
#pragma GCC pop_options

PERF_CASE_DEFINE(branch_pred) = {
	.name = "branch_pred",
	.desc = "if loop with fixed/random condition pattern.",
	.init = branch_init,
	.exit = branch_exit,
	.func = branch_pred_func,
	.getopt = branch_getopt,
	.opts = branch_pred_opts,
	.opts_num = sizeof(branch_pred_opts) / sizeof(struct perf_option),
	.inner_stat = true
};
