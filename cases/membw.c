#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <getopt.h>

#include "../perf_stat.h"
#include "../perf_case.h"

#define BUF_SIZE (128 * 1024 * 1024)

struct membw_data {
	void *buf;
	void *src;
	void *buf_end;
	void *src_end;
	int buf_size;
	int stride;
	int iterations;
};

static int opt_buf_size = BUF_SIZE;
static int opt_stride = 1;
static int opt_iterations = 1;

static struct perf_option membw_opts[] = {
	{{"bufsize",    optional_argument, NULL, 'b' }, "b:", "Test buffer size. (bytes)"},
	{{"stride",     optional_argument, NULL, 's' }, "s:", "Test stride. (bytes)"},
	{{"iterations", optional_argument, NULL, 'i' }, "i:", "Iteration loops."},
};

static struct perf_event membw_events[] = {
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES, "cache-refs"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"),
};

static int membw_getopt(struct perf_case* p_case, int opt)
{
	switch (opt) {
	case 'b':
		opt_buf_size = atoi(optarg);
		break;
	case 's':
		opt_stride = atoi(optarg);
		break;
	case 'i':
		opt_iterations = atoi(optarg);
		break;
	default:
		return ERROR;
	}
	return SUCCESS;
}

static int membw_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	struct membw_data *p_data;
	
	p_case->data = malloc(sizeof(struct membw_data));
	if (!p_case->data)
		return ERROR;

	p_data = (struct membw_data*)p_case->data;

	p_data->buf_size   = opt_buf_size;
	p_data->stride     = opt_stride;
	p_data->iterations = opt_iterations;

	p_data->buf = malloc(p_data->buf_size);
	if (!p_data->buf)
		goto ERR_EXIT_1;

	p_data->src = malloc(p_data->buf_size);
	if (!p_data->src)
		goto ERR_EXIT_2;

	memset(p_data->buf, 0x1, p_data->buf_size);
	memset(p_data->src, 0x1, p_data->buf_size);

	p_data->buf_end = (char*)p_data->buf + p_data->buf_size;
	p_data->src_end = (char*)p_data->src + p_data->buf_size;

	return SUCCESS;

ERR_EXIT_2:
	free(p_data->buf);
ERR_EXIT_1:
	free(p_case->data);
	p_case->data = NULL;
	return ERROR;
}

static int membw_exit(struct perf_case *p_case, struct perf_stat *p_stat)
{
	struct membw_data *p_data = (struct membw_data*)p_case->data;
	free(p_data->src);
	free(p_data->buf);
	free(p_case->data);
	p_case->data = NULL;
	return SUCCESS;
}

static void print_bandwidth(int width, int stride, int buf_size, int iterations, struct perf_stat *p_stat, int nx)
{
	double size_mb = (double)buf_size / 1024 / 1024;
	double time_ms = ((double)p_stat->duration / iterations / 1000000000);
	printf("bufsize: %.6f MB\n", size_mb);
	printf("width: %d bytes (%dbit)", width / 8, width);
	if (nx > 1)
		printf(" * %d", nx);
	printf("\n");
	printf("stride: %d bytes\n", stride);
	printf("iterations: %d\n", iterations);
	printf("%.3f MB/s (%f ms)\n", size_mb / time_ms, time_ms);
}

#define MEMBW_RD_PREPARE(_type)							\
	struct membw_data *p_data = (struct membw_data*)p_case->data;		\
	volatile _type *p;							\
	_type *end = (_type*)p_data->buf_end;					\
	int step = p_data->stride / sizeof(_type);				\
	int sum;

#define MEMBW_RD_WORKLOAD(_type)						\
	step = step < 1 ? 1 : step;						\
	perf_stat_begin(p_stat);						\
	for (int i = 0; i < p_data->iterations; i++)				\
		for (p = (_type*)p_data->buf; p < end; p += step)		\
			sum += *p;						\
	perf_stat_end(p_stat);							\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 			\
		p_data->buf_size, p_data->iterations, p_stat, 1);

#define MEMBW_RD_WORKLOAD_4X(_type)						\
	step = step < 4 ? 4 : step;						\
	perf_stat_begin(p_stat);						\
	for (int i = 0; i < p_data->iterations; i++) {				\
		for (p = (_type*)p_data->buf; p < end; p += step) {		\
			sum += *p;						\
			sum += *(p + 1);					\
			sum += *(p + 2);					\
			sum += *(p + 3);					\
		}								\
	}									\
	perf_stat_end(p_stat);							\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 			\
		p_data->buf_size, p_data->iterations, p_stat, 4);

static void membw_rd_1(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint8_t);
	MEMBW_RD_WORKLOAD(uint8_t);
}

static void membw_rd_4(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint32_t);
	MEMBW_RD_WORKLOAD(uint32_t);
}

static void membw_rd_8(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint64_t);
	MEMBW_RD_WORKLOAD(uint64_t);
}

static void membw_rd_1_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint8_t);
	MEMBW_RD_WORKLOAD_4X(uint8_t);
}

static void membw_rd_4_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint32_t);
	MEMBW_RD_WORKLOAD_4X(uint32_t);
}

static void membw_rd_8_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint64_t);
	MEMBW_RD_WORKLOAD_4X(uint64_t);
}

#define MEMBW_WR_PREPARE(_type)							\
	struct membw_data *p_data = (struct membw_data*)p_case->data;		\
	volatile _type *p;							\
	_type *end = (_type*)p_data->buf_end;					\
	int step = p_data->stride / sizeof(_type);

#define MEMBW_WR_WORKLOAD(_type)						\
	step = step < 1 ? 1 : step;						\
	perf_stat_begin(p_stat);						\
	for (int i = 0; i < p_data->iterations; i++)				\
		for (p = (_type*)p_data->buf; p < end; p += step)		\
			*p = 1;							\
	perf_stat_end(p_stat);							\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 			\
		p_data->buf_size, p_data->iterations, p_stat, 1);

#define MEMBW_WR_WORKLOAD_4X(_type)						\
	step = step < 4 ? 4 : step;						\
	perf_stat_begin(p_stat);						\
	for (int i = 0; i < p_data->iterations; i++) {				\
		for (p = (_type*)p_data->buf; p < end; p += step) {		\
			*p = 1;							\
			*(p + 1) = 1;						\
			*(p + 2) = 1;						\
			*(p + 3) = 1;						\
		}								\
	}									\
	perf_stat_end(p_stat);							\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 			\
		p_data->buf_size, p_data->iterations, p_stat, 4);

static void membw_wr_1(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint8_t);
	MEMBW_WR_WORKLOAD(uint8_t);
}

static void membw_wr_4(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint32_t);
	MEMBW_WR_WORKLOAD(uint32_t);
}

static void membw_wr_8(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint64_t);
	MEMBW_WR_WORKLOAD(uint64_t);
}

static void membw_wr_1_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint8_t);
	MEMBW_WR_WORKLOAD_4X(uint8_t);
}

static void membw_wr_4_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint32_t);
	MEMBW_WR_WORKLOAD_4X(uint32_t);
}

static void membw_wr_8_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint64_t);
	MEMBW_WR_WORKLOAD_4X(uint64_t);
}

#define MEMBW_CP_PREPARE(_type)							\
	struct membw_data *p_data = (struct membw_data*)p_case->data;		\
	volatile _type *p, *s;							\
	_type *end = (_type*)p_data->buf_end;					\
	int step = p_data->stride / sizeof(_type);

#define MEMBW_CP_WORKLOAD(_type)						\
	step = step < 1 ? 1 : step;						\
	perf_stat_begin(p_stat);						\
	for (int i = 0; i < p_data->iterations; i++)				\
		for (p = (_type*)p_data->buf, s = (_type*)p_data->src;		\
		     p < end; p += step, s += step)				\
			*p = *s;						\
	perf_stat_end(p_stat);							\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 			\
		p_data->buf_size, p_data->iterations, p_stat, 1);

#define MEMBW_CP_WORKLOAD_4X(_type)						\
	step = step < 4 ? 4 : step;						\
	perf_stat_begin(p_stat);						\
	for (int i = 0; i < p_data->iterations; i++) {				\
		for (p = (_type*)p_data->buf, s = (_type*)p_data->src;		\
		     p < end; p += step, s += step) {				\
			*p = *s;						\
			*(p + 1) = *(s + 1);					\
			*(p + 2) = *(s + 2);					\
			*(p + 3) = *(s + 3);					\
		}								\
	}									\
	perf_stat_end(p_stat);							\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 			\
		p_data->buf_size, p_data->iterations, p_stat, 4);

static void membw_cp_1(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint8_t);
	MEMBW_CP_WORKLOAD(uint8_t);
}

static void membw_cp_4(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint32_t);
	MEMBW_CP_WORKLOAD(uint32_t);
}

static void membw_cp_8(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint64_t);
	MEMBW_CP_WORKLOAD(uint64_t);
}

static void membw_cp_1_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint8_t);
	MEMBW_CP_WORKLOAD_4X(uint8_t);
}

static void membw_cp_4_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint32_t);
	MEMBW_CP_WORKLOAD_4X(uint32_t);
}

static void membw_cp_8_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint64_t);
	MEMBW_CP_WORKLOAD_4X(uint64_t);
}

#define DEFINE_MEMBW_CASE(_name, _desc)						\
	PERF_CASE_DEFINE(_name) = {						\
		.name = #_name,							\
		.desc = _desc,							\
		.init = membw_init,						\
		.exit = membw_exit,						\
		.func = _name,							\
		.getopt = membw_getopt,					\
		.opts = membw_opts,						\
		.opts_num = sizeof(membw_opts) / sizeof(struct perf_option),	\
		.events = membw_events,						\
		.event_num = sizeof(membw_events) / sizeof(struct perf_event),	\
		.inner_stat = true						\
	};

DEFINE_MEMBW_CASE(membw_rd_1,    "read a memory buffer. (8bit)");
DEFINE_MEMBW_CASE(membw_rd_4,    "read a memory buffer. (32bit)");
DEFINE_MEMBW_CASE(membw_rd_8,    "read a memory buffer. (64bit)");
DEFINE_MEMBW_CASE(membw_rd_1_4x, "read a memory buffer. (8bit) * 4");
DEFINE_MEMBW_CASE(membw_rd_4_4x, "read a memory buffer. (32bit) * 4");
DEFINE_MEMBW_CASE(membw_rd_8_4x, "read a memory buffer. (64bit) * 4");
DEFINE_MEMBW_CASE(membw_wr_1,    "write a memory buffer. (8bit)");
DEFINE_MEMBW_CASE(membw_wr_4,    "write a memory buffer. (32bit)");
DEFINE_MEMBW_CASE(membw_wr_8,    "write a memory buffer. (64bit)");
DEFINE_MEMBW_CASE(membw_wr_1_4x, "write a memory buffer. (8bit) * 4");
DEFINE_MEMBW_CASE(membw_wr_4_4x, "write a memory buffer. (32bit) * 4");
DEFINE_MEMBW_CASE(membw_wr_8_4x, "write a memory buffer. (64bit) * 4");
DEFINE_MEMBW_CASE(membw_cp_1,    "copy a memory buffer. (8bit)");
DEFINE_MEMBW_CASE(membw_cp_4,    "copy a memory buffer. (32bit)");
DEFINE_MEMBW_CASE(membw_cp_8,    "copy a memory buffer. (64bit)");
DEFINE_MEMBW_CASE(membw_cp_1_4x, "copy a memory buffer. (8bit) * 4");
DEFINE_MEMBW_CASE(membw_cp_4_4x, "copy a memory buffer. (32bit) * 4");
DEFINE_MEMBW_CASE(membw_cp_8_4x, "copy a memory buffer. (64bit) * 4");