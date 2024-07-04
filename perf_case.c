#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>
#include "perf_case.h"
#include "perf_stat.h"

static struct perf_event g_events[] = {
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES, "cpu-cycles"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "instructions"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES, "cache-refs"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"},
	//{PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND, "stall-frontend"},
	//{PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND, "stall-backend"},
	//{PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, "page_faults"}
};

static struct perf_case g_cases[] = {
	PERF_CASE("memset", perf_case_memset, "case memset for different memory types"),
	PERF_CASE("intcal", perf_case_intcal, "case various int calculations"),
	PERF_CASE("bw_mem", perf_case_bw_mem, "lmbench bw_mem case")
};

int perf_case_memset(struct perf_event *events, int event_num)
{
#define BUF_SIZE (128 * 1024 * 1024)
	static char buf1[BUF_SIZE];
	static char buf2[BUF_SIZE] = {0};
	char* buf3 = malloc(BUF_SIZE);
	char* buf4 = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	PERF_STAT_BEGIN("memset", events, event_num);
	memset(buf1, 0xFF, BUF_SIZE);
	PERF_STAT_END();

	PERF_STAT_BEGIN("memset-static", events, event_num);
	memset(buf2, 0xFF, BUF_SIZE);
	PERF_STAT_END();

	PERF_STAT_BEGIN("memset-malloc", events, event_num);
	memset(buf3, 0xFF, BUF_SIZE);
	PERF_STAT_END();

	PERF_STAT_BEGIN("memset-malloc-x2", events, event_num);
	memset(buf3, 0xFF, BUF_SIZE);
	PERF_STAT_END();

	PERF_STAT_BEGIN("memset-mmap", events, event_num);
	memset(buf4, 0xFF, BUF_SIZE);
	PERF_STAT_END();
#undef BUF_SIZE

	return 0;
}

int perf_case_intcal(struct perf_event *events, int event_num)
{
	int x, y, z;

	PERF_STAT_BEGIN("x++", events, event_num);
	for (int i = 0; i < 100000000; i++)
		x += i * 2;
	PERF_STAT_END();

	PERF_STAT_BEGIN("x,y,z++", events, event_num);
	for (int i = 0; i < 100000000; i++) {
		x += i * 2;
		y += i * 2;
		z += i * 2;
	}
	PERF_STAT_END();

	return 0;
}

int perf_case_bw_mem(struct perf_event *events, int event_num)
{
	int buf_size = 128 * 1024 * 1024;
	char *buf = malloc(buf_size);
	char *src = malloc(buf_size);
	volatile uint8_t *p_8, *s_8;
	volatile uint32_t *p_32, *s_32;
	volatile uint64_t *p_64, *s_64;
	int stride_list[] = {1, 4, 8, 16, 64, 128, 256, 512};
	int stride;
	char *end = buf + buf_size;
	char title[32];
	int sum;

	// Warmup
	PERF_STAT_BEGIN("memset", events, event_num);
	memset(buf, 0x1, buf_size);
	memset(src, 0x1, buf_size);
	PERF_STAT_END();

	// RD
	for (int i = 0; i < sizeof(stride_list) / sizeof(int); i++) {
		stride = stride_list[i];
		sprintf(title, "rd-1/%d", stride);
		PERF_STAT_BEGIN(title, events, event_num);
		for (p_8 = (uint8_t*)buf; p_8 < (uint8_t*)end; p_8 += stride)
			sum += *p_8;
		PERF_STAT_END();
		
		if ((int)(stride / 4)) {
			sprintf(title, "rd-4/%d", stride);
			PERF_STAT_BEGIN(title, events, event_num);
			for (p_32 = (uint32_t*)buf; p_32 < (uint32_t*)end; p_32 += stride / 4)
				sum += *p_32;
			PERF_STAT_END();
		}
	
		if ((int)(stride / 8)) {
			sprintf(title, "rd-8/%d", stride);
			PERF_STAT_BEGIN(title, events, event_num);
			for (p_64 = (uint64_t*)buf; p_64 < (uint64_t*)end; p_64 += stride / 8)
				sum += *p_64;
			PERF_STAT_END();
		}
	}

	// WR
	for (int i = 0; i < sizeof(stride_list) / sizeof(int); i++) {
		stride = stride_list[i];
		sprintf(title, "wr-1/%d", stride);
		PERF_STAT_BEGIN(title, events, event_num);
		for (p_8 = (uint8_t*)buf; p_8 < (uint8_t*)end; p_8 += stride)
			*p_8 = 1;
		PERF_STAT_END();
		
		if ((int)(stride / 4)) {
			sprintf(title, "wr-4/%d", stride);
			PERF_STAT_BEGIN(title, events, event_num);
			for (p_32 = (uint32_t*)buf; p_32 < (uint32_t*)end; p_32 += stride / 4)
				*p_32 = 1;
			PERF_STAT_END();
		}
	
		if ((int)(stride / 8)) {
			sprintf(title, "wr-8/%d", stride);
			PERF_STAT_BEGIN(title, events, event_num);
			for (p_64 = (uint64_t*)buf; p_64 < (uint64_t*)end; p_64 += stride / 2) {
				*p_64 = 1;
				*(p_64 + 1) = 1;
				*(p_64 + 2) = 1;
				*(p_64 + 3) = 1;
			}
			PERF_STAT_END();
		}
	}

	// CP
	for (int i = 0; i < sizeof(stride_list) / sizeof(int); i++) {
		stride = stride_list[i];
		sprintf(title, "cp-1/%d", stride);
		PERF_STAT_BEGIN(title, events, event_num);
		for (p_8 = (uint8_t*)buf, s_8 = (uint8_t*)src; p_8 < (uint8_t*)end; p_8 += stride, s_8 += stride)
			*p_8 = *s_8;
		PERF_STAT_END();
		
		if ((int)(stride / 4)) {
			sprintf(title, "cp-4/%d", stride);
			PERF_STAT_BEGIN(title, events, event_num);
			for (p_32 = (uint32_t*)buf, s_32 = (uint32_t*)src; p_32 < (uint32_t*)end; p_32 += stride, s_32 += stride)
				*p_32 = *s_32;
			PERF_STAT_END();
		}
	
		if ((int)(stride / 8)) {
			sprintf(title, "cp-8/%d", stride);
			PERF_STAT_BEGIN(title, events, event_num);
			for (p_64 = (uint64_t*)buf, s_64 = (uint64_t*)src; p_64 < (uint64_t*)end; p_64 += stride, s_64 += stride)
				*p_64 = *s_64;
			PERF_STAT_END();
		}
	}

	free(src);
	free(buf);

	return 0;
}

void print_help()
{
	int i;
	int case_num = sizeof(g_cases) / sizeof(struct perf_case);

	printf("./perf_case <case>\n");
	printf("==========================\n");
	for (i = 0; i < case_num; i++)
		printf("%s - %s\n", g_cases[i].name, g_cases[i].desc);
}

int run_case(char *case_name)
{
	int i, ret;
	struct perf_case *p_case;
	int case_num = sizeof(g_cases) / sizeof(struct perf_case);
	int event_num = sizeof(g_events) / sizeof(struct perf_event);
	bool found = false;

	for (i = 0; i < case_num; i++) {
		p_case = &g_cases[i];
		if (!strcmp(case_name, p_case->name) || !strcmp(case_name, "all")) {
			found = true;
			printf("%s\n", p_case->name);
			printf("=======================\n");
			printf("%s\n\n", p_case->desc);
			ret = p_case->func(g_events, event_num);
			if (ret < 0)
				return ret;
		}
	}

	if (!found) {
		printf("ERROR: No case named \"%s\"\n", case_name);
		exit(0);
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		print_help();
		return 0;
	}

	run_case(argv[1]);

	return 0;
}
