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
};

static struct perf_event membw_events[] = {
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES, "cache-refs"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"),
};

static struct option membw_options[] = {
	{"bufsize",    optional_argument, NULL, 'b' },
	{"stride",     optional_argument, NULL, 's' },
	{"iterations", optional_argument, NULL, 'i' },
	{0, 0, 0, 0}
};

static void membw_help(struct perf_case *p_case)
{
	PERF_CASE_OPTION_HELP("-b", "--bufsize",    "Test buffer size.(bytes)");
	PERF_CASE_OPTION_HELP("-s", "--sride",      "Test stride.(bytes)");
	PERF_CASE_OPTION_HELP("-i", "--iterations", "Iteration loops.");
}

static int membw_init(struct perf_case *p_case, struct perf_stat *p_stat, int argc, char *argv[])
{
	struct membw_data *p_data;
	int opt, opt_idx;
	
	p_case->data = malloc(sizeof(struct membw_data));
	if (!p_case->data)
		return ERROR;

	p_data = (struct membw_data*)p_case->data;

	p_data->buf_size = BUF_SIZE;
	p_data->stride = 1;

	while ((opt = getopt_long(argc, argv, "b:s:i:", membw_options, &opt_idx)) != -1) {
		switch (opt) {
		case 'b':
			p_data->buf_size = atoi(optarg);
			break;
		case 's':
			p_data->stride = atoi(optarg);
			break;
		case 'i':
			break;
		default:
			membw_help(p_case);
			exit(0);
		}
	}

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
	free(p_data->buf);
	free(p_case->data);
	p_case->data = NULL;
	return SUCCESS;
}

static void print_bandwidth(int width, int stride, int buf_size, struct perf_stat *p_stat, int n)
{
	printf("width:  %2d bytes (%dbit)", width / 8, width);
	if (n > 1)
		printf(" * %d", n);
	printf("\n");
	printf("stride: %2d bytes\n", stride);
	printf("%.3f MB, %f ms, %.3f MB/s\n",
		(double)buf_size / 1024 / 1024, 
		(double)p_stat->duration / 1000000,
		((double)buf_size / 1024 / 1024) / ((double)p_stat->duration / 1000000000)
	);
}

#define MEMBW_RD_PREPARE(_type)						\
	struct membw_data *p_data = (struct membw_data*)p_case->data;	\
	volatile _type *p = (_type*)p_data->buf;			\
	_type *end = (_type*)p_data->buf_end;				\
	int step = p_data->stride / sizeof(_type);			\
	int sum;

#define MEMBW_RD_WORKLOAD()						\
	step = step < 1 ? 1 : step;					\
	perf_stat_begin(p_stat);					\
	for (; p < end; p += step)					\
		sum += *p;						\
	perf_stat_end(p_stat);						\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 		\
		p_data->buf_size, p_stat, 1);

#define MEMBW_RD_WORKLOAD_4X()						\
	step = step < 4 ? 4 : step;					\
	perf_stat_begin(p_stat);					\
	for (; p < end; p += step) {					\
		sum += *p;						\
		sum += *(p + 1);					\
		sum += *(p + 2);					\
		sum += *(p + 3);					\
	}								\
	perf_stat_end(p_stat);						\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 		\
		p_data->buf_size, p_stat, 4);

static void membw_rd_1(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint8_t);
	MEMBW_RD_WORKLOAD();
}

static void membw_rd_4(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint32_t);
	MEMBW_RD_WORKLOAD();
}

static void membw_rd_8(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint64_t);
	MEMBW_RD_WORKLOAD();
}

static void membw_rd_1_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint8_t);
	MEMBW_RD_WORKLOAD_4X();
}

static void membw_rd_4_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint32_t);
	MEMBW_RD_WORKLOAD_4X();
}

static void membw_rd_8_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_RD_PREPARE(uint64_t);
	MEMBW_RD_WORKLOAD_4X();
}

#define MEMBW_WR_PREPARE(_type)						\
	struct membw_data *p_data = (struct membw_data*)p_case->data;	\
	volatile _type *p = (_type*)p_data->buf;			\
	_type *end = (_type*)p_data->buf_end;				\
	int step = p_data->stride / sizeof(_type);

#define MEMBW_WR_WORKLOAD()						\
	step = step < 1 ? 1 : step;					\
	perf_stat_begin(p_stat);					\
	for (; p < end; p += step)					\
		*p = 1;							\
	perf_stat_end(p_stat);						\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 		\
		p_data->buf_size, p_stat, 1);

#define MEMBW_WR_WORKLOAD_4X()						\
	step = step < 4 ? 4 : step;					\
	perf_stat_begin(p_stat);					\
	for (; p < end; p += step) {					\
		*p = 1;							\
		*(p + 1) = 1;						\
		*(p + 2) = 1;						\
		*(p + 3) = 1;						\
	}								\
	perf_stat_end(p_stat);						\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 		\
		p_data->buf_size, p_stat, 4);

static void membw_wr_1(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint8_t);
	MEMBW_WR_WORKLOAD();
}

static void membw_wr_4(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint32_t);
	MEMBW_WR_WORKLOAD();
}

static void membw_wr_8(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint64_t);
	MEMBW_WR_WORKLOAD();
}

static void membw_wr_1_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint8_t);
	MEMBW_WR_WORKLOAD_4X();
}

static void membw_wr_4_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint32_t);
	MEMBW_WR_WORKLOAD_4X();
}

static void membw_wr_8_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_WR_PREPARE(uint64_t);
	MEMBW_WR_WORKLOAD_4X();
}

#define MEMBW_CP_PREPARE(_type)						\
	struct membw_data *p_data = (struct membw_data*)p_case->data;	\
	volatile _type *p = (_type*)p_data->buf;			\
	volatile _type *s = (_type*)p_data->src;			\
	_type *end = (_type*)p_data->buf_end;				\
	int step = p_data->stride / sizeof(_type);

#define MEMBW_CP_WORKLOAD()						\
	step = step < 1 ? 1 : step;					\
	perf_stat_begin(p_stat);					\
	for (; p < end; p += step, s += step)				\
		*p = *s;						\
	perf_stat_end(p_stat);						\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 		\
		p_data->buf_size, p_stat, 1);

#define MEMBW_CP_WORKLOAD_4X()						\
	step = step < 4 ? 4 : step;					\
	perf_stat_begin(p_stat);					\
	for (; p < end; p += step, s += step) {				\
		*p = *s;						\
		*(p + 1) = *(s + 1);					\
		*(p + 2) = *(s + 2);					\
		*(p + 3) = *(s + 3);					\
	}								\
	perf_stat_end(p_stat);						\
	print_bandwidth(sizeof(*p) * 8, step * sizeof(*p), 		\
		p_data->buf_size, p_stat, 4);

static void membw_cp_1(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint8_t);
	MEMBW_CP_WORKLOAD();
}

static void membw_cp_4(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint32_t);
	MEMBW_CP_WORKLOAD();
}

static void membw_cp_8(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint64_t);
	MEMBW_CP_WORKLOAD();
}

static void membw_cp_1_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint8_t);
	MEMBW_CP_WORKLOAD_4X();
}

static void membw_cp_4_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint32_t);
	MEMBW_CP_WORKLOAD_4X();
}

static void membw_cp_8_4x(struct perf_case *p_case, struct perf_stat *p_stat)
{
	MEMBW_CP_PREPARE(uint64_t);
	MEMBW_CP_WORKLOAD_4X();
}

#define DEFINE_MEMBW_CASE(_name, _desc)						\
	PERF_CASE_DEFINE(_name) = {						\
		.name = #_name,							\
		.desc = _desc,							\
		.init = membw_init,						\
		.exit = membw_exit,						\
		.help = membw_help,						\
		.func = _name,							\
		.events = membw_events,						\
		.event_num = sizeof(membw_events) / sizeof(struct perf_event),	\
		.inner_stat = true						\
	};

DEFINE_MEMBW_CASE(membw_rd_1,    "read a memory buffer.(8bit)");
DEFINE_MEMBW_CASE(membw_rd_4,    "read a memory buffer.(32bit)");
DEFINE_MEMBW_CASE(membw_rd_8,    "read a memory buffer.(64bit)");
DEFINE_MEMBW_CASE(membw_rd_1_4x, "read a memory buffer.(8bit) * 4");
DEFINE_MEMBW_CASE(membw_rd_4_4x, "read a memory buffer.(32bit) * 4");
DEFINE_MEMBW_CASE(membw_rd_8_4x, "read a memory buffer.(64bit) * 4");
DEFINE_MEMBW_CASE(membw_wr_1,    "write a memory buffer.(8bit)");
DEFINE_MEMBW_CASE(membw_wr_4,    "write a memory buffer.(32bit)");
DEFINE_MEMBW_CASE(membw_wr_8,    "write a memory buffer.(64bit)");
DEFINE_MEMBW_CASE(membw_wr_1_4x, "write a memory buffer.(8bit) * 4");
DEFINE_MEMBW_CASE(membw_wr_4_4x, "write a memory buffer.(32bit) * 4");
DEFINE_MEMBW_CASE(membw_wr_8_4x, "write a memory buffer.(64bit) * 4");
DEFINE_MEMBW_CASE(membw_cp_1,    "copy a memory buffer.(8bit)");
DEFINE_MEMBW_CASE(membw_cp_4,    "copy a memory buffer.(32bit)");
DEFINE_MEMBW_CASE(membw_cp_8,    "copy a memory buffer.(64bit)");
DEFINE_MEMBW_CASE(membw_cp_1_4x, "copy a memory buffer.(8bit) * 4");
DEFINE_MEMBW_CASE(membw_cp_4_4x, "copy a memory buffer.(32bit) * 4");
DEFINE_MEMBW_CASE(membw_cp_8_4x, "copy a memory buffer.(64bit) * 4");