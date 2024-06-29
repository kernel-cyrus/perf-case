#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include "perf_test.h"
#include "perf_event.h"

#define BUF_SIZE (128 * 1024 * 1024)

static struct perf_event events[] = {
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES, "cpu-cycles"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "instructions"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND, "stall-frontend"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND, "stall-backend"},
	{PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, "page_faults"}
};

static struct perf_test tests[] = {
	{
		.name = "name",
		.desc = "desc",
		.func = NULL
	}
};

static char buf1[BUF_SIZE];

int main()
{
	char* buf2 = malloc(BUF_SIZE);
	char* buf3 = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	int x, y, z;

	PERF_STAT_BEGIN("memset-static", events);
	memset(buf1, 0xFF, BUF_SIZE);
	PERF_STAT_END();

	PERF_STAT_BEGIN("memset-malloc", events);
	memset(buf2, 0xFF, BUF_SIZE);
	PERF_STAT_END();

	PERF_STAT_BEGIN("memset-malloc-x2", events);
	memset(buf2, 0xFF, BUF_SIZE);
	PERF_STAT_END();

	PERF_STAT_BEGIN("memset-mmap", events);
	memset(buf3, 0xFF, BUF_SIZE);
	PERF_STAT_END();

	PERF_STAT_BEGIN("x++", events);
        for (int i = 0; i < 100000000; i++)
                x += i * 2;
	PERF_STAT_END();

	PERF_STAT_BEGIN("x,y,z++", events);
        for (int i = 0; i < 100000000; i++) {
                x += i * 2;
                y += i * 2;
                z += i * 2;
        }
	PERF_STAT_END();
}