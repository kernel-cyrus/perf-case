#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>
#include "perf_case.h"
#include "perf_stat.h"

static struct perf_event default_events[] = {
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES, "cpu-cycles"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "instructions"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES, "cache-refs"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND, "stall-frontend"},
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND, "stall-backend"},
};

extern struct perf_case __perf_case_memset;

static struct perf_case *perf_cases[] = {
	&__perf_case_memset,
};

struct perf_case* perf_case_find(char* name)
{
	int case_num = sizeof(perf_cases) / sizeof(struct perf_case*);

	for (int i = 0; i < case_num; i++)
		if (!strcmp(name, perf_cases[i]->name))
			return perf_cases[i];

	return NULL;
}

void print_help()
{
	struct perf_case *p_case;

	int case_num = sizeof(perf_cases) / sizeof(struct perf_case*);

	printf("./perf_case <case>\n");
	printf("==========================\n");

	for (int i = 0; i < case_num; i++) {
		p_case = perf_cases[i];
		if (p_case->help) {
			p_case->help(p_case);
		} else {
			printf("%s - %s\n", p_case->name, p_case->desc);
		}
	}
}

int run_case(char *name, int argc, char **argv)
{
	int err;
	struct perf_case *p_case;

	p_case = perf_case_find(name);
	if (!p_case) {
		printf("ERROR: No case named \"%s\"\n", name);
		exit(0);
	}

	printf("%s\n", p_case->name);
	printf("=======================\n");
	printf("%s\n\n", p_case->desc);

	err = p_case->init(p_case, argc, argv);
	if (err) {
		printf("ERROR: Case \"%s\" init failed.\n", name);
		exit(0);
	}

	PERF_STAT_BEGIN(name, default_events, sizeof(default_events) / sizeof(struct perf_event));
	p_case->func(p_case);
	PERF_STAT_END();

	err = p_case->exit(p_case);
	if (err) {
		printf("ERROR: Case \"%s\" exit failed.\n", name);
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

	run_case(argv[1], argc, argv);

	return 0;
}
