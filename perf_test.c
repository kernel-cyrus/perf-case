#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>
#include "perf_test.h"
#include "perf_event.h"

static struct perf_event g_events[] = {
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES, "cpu-cycles"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "instructions"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND, "stall-frontend"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND, "stall-backend"},
	{PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, "page_faults"}
};

static struct perf_test g_tests[] = {
	PERF_TEST("memset", perf_test_memset, "test memset for different memory types"),
	PERF_TEST("intcal", perf_test_intcal, "test various int calculations")
};

int perf_test_memset(struct perf_event *events, int event_num)
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

int perf_test_intcal(struct perf_event *events, int event_num)
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

void print_help()
{
	int i;
	int test_num = sizeof(g_tests) / sizeof(struct perf_test);

	printf("./perf_test <test>\n");
	printf("==========================\n");
	for (i = 0; i < test_num; i++)
		printf("%s - %s\n", g_tests[i].name, g_tests[i].desc);
}

int run_test(char *test_name)
{
	int i, ret;
	struct perf_test *p_test;
	int test_num = sizeof(g_tests) / sizeof(struct perf_test);
	int event_num = sizeof(g_events) / sizeof(struct perf_event);
	bool found = false;

	for (i = 0; i < test_num; i++) {
		p_test = &g_tests[i];
		if (!strcmp(test_name, p_test->name) || !strcmp(test_name, "all")) {
			found = true;
			printf("%s\n", p_test->name);
			printf("=======================\n");
			printf("%s\n\n", p_test->desc);
			ret = p_test->func(g_events, event_num);
			if (ret < 0)
				return ret;
		}
	}

	if (!found) {
		printf("ERROR: No test named \"%s\"\n", test_name);
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

	run_test(argv[1]);

	return 0;
}