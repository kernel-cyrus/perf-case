#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>

#include "perf_stat.h"
#include "perf_case.h"

#define BUF_SIZE (128 * 1024 * 1024)

struct memlat_data {
	char **buf;
	int buf_size;
	int iterations;
};

static int opt_buf_size = BUF_SIZE;
static int opt_iterations = 1;

static struct perf_option memlat_opts[] = {
	{{"bufsize",    optional_argument, NULL, 'b' }, "b:", "Test buffer size. (bytes)"},
	{{"iterations", optional_argument, NULL, 'i' }, "i:", "Iteration loops."},
};

static struct perf_event memlat_events[] = {
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES, "cache-refs"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"),
};

static int memlat_getopt(struct perf_case* p_case, int opt)
{
	switch (opt) {
	case 'b':
		opt_buf_size = atoi(optarg);
		break;
	case 'i':
		opt_iterations = atoi(optarg);
		break;
	default:
		return ERROR;
	}
	return SUCCESS;
}

static void init_random_buf(char **buf, int buf_size)
{
	int num = buf_size / sizeof(char*);
	int i, j;
	char *tmp;

	for (i = 0; i < num; i++) {
		buf[i] = (char*)&buf[i];
	}

	while (--i > 0) {
		j = i - 1 == 0 ? 0 : rand() % (i - 1);
		tmp = buf[i];
		buf[i] = buf[j];
		buf[j] = tmp;
	}
}

static int memlat_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	struct memlat_data *p_data;

	p_case->data = malloc(sizeof(struct memlat_data));
	if (!p_case->data)
		return ERROR;

	p_data = (struct memlat_data*)p_case->data;

	p_data->buf_size   = opt_buf_size;
	p_data->iterations = opt_iterations;

	p_data->buf = malloc(p_data->buf_size);
	if (!p_data->buf)
		goto ERR_EXIT_1;

	init_random_buf(p_data->buf, p_data->buf_size);

	return SUCCESS;

ERR_EXIT_1:
	free(p_case->data);
	p_case->data = NULL;
	return ERROR;
}

static int memlat_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct memlat_data *p_data = (struct memlat_data*)p_case->data;
	free(p_data->buf);
	free(p_case->data);
	p_case->data = NULL;
	return SUCCESS;
}

static void print_latency(int buf_size, int count, int iterations, struct perf_stat *p_stat)
{
	double size_mb = (double)buf_size / 1024 / 1024;
	double latency_ns = (double)p_stat->duration / count;
	printf("bufsize: %.6f MB\n", size_mb);
	printf("iterations: %d\n", iterations);
	printf("total ops: %d\n", count);
	printf("latency: %.3f ns\n", latency_ns);
}

#define	DO_1	p = (char **)*p;
#define	DO_8	DO_1  DO_1  DO_1  DO_1  DO_1  DO_1  DO_1  DO_1
#define	DO_64	DO_8  DO_8  DO_8  DO_8  DO_8  DO_8  DO_8  DO_8
#define	DO_128	DO_64 DO_64

void use_pointer(void *pointer)
{
	static long long use = 0;
	use += (long long)pointer;
}

static void memlat_func(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct memlat_data *p_data = (struct memlat_data*)p_case->data;
	register char **p = (char**)p_data->buf[0];
	register int iterations = p_data->iterations;
	register int i;
	register int round = p_data->buf_size / (sizeof(char*) * 128);

	perf_stat_begin(p_stat);
	while (iterations-- > 0) {
		for (i = 0; i < round; i++) {
			DO_128; // 1KB
		}
	}
	perf_stat_end(p_stat);

	use_pointer(p); // to avoid compiler optimization
	print_latency(p_data->buf_size, 128 * round * p_data->iterations, p_data->iterations, p_stat);
}

PERF_CASE_DEFINE(memlat_random) = {
	.name = "memlat_random",
	.desc = "memory random access latnecy.",
	.init = memlat_init,
	.exit = memlat_exit,
	.func = memlat_func,
	.getopt = memlat_getopt,
	.opts = memlat_opts,
	.opts_num = sizeof(memlat_opts) / sizeof(struct perf_option),
	.events = memlat_events,
	.event_num = sizeof(memlat_events) / sizeof(struct perf_event),
	.inner_stat = true
};
