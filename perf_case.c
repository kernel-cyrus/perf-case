#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <sys/mman.h>
#include "perf_case.h"
#include "perf_stat.h"

static struct perf_event default_events[] = {
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES, "cpu-cycles"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "instructions"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES, "cache-refs"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND, "stall-frontend"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND, "stall-backend"),
	PERF_EVENT(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, "page-faults")
};

static struct perf_case *perf_cases[] = {
	PERF_CASE(memset_malloc),
	PERF_CASE(memset_malloc_x2),
	PERF_CASE(memset_mmap),
	PERF_CASE(memset_static_bss),
	PERF_CASE(memset_static_data),
	PERF_CASE(membw_rd_1),
	PERF_CASE(membw_rd_4),
	PERF_CASE(membw_rd_8),
	PERF_CASE(membw_rd_1_4x),
	PERF_CASE(membw_rd_4_4x),
	PERF_CASE(membw_rd_8_4x),
	PERF_CASE(membw_wr_1),
	PERF_CASE(membw_wr_4),
	PERF_CASE(membw_wr_8),
	PERF_CASE(membw_wr_1_4x),
	PERF_CASE(membw_wr_4_4x),
	PERF_CASE(membw_wr_8_4x),
	PERF_CASE(membw_cp_1),
	PERF_CASE(membw_cp_4),
	PERF_CASE(membw_cp_8),
	PERF_CASE(membw_cp_1_4x),
	PERF_CASE(membw_cp_4_4x),
	PERF_CASE(membw_cp_8_4x),
};

struct perf_case* perf_case_find(char* name)
{
	int case_num = sizeof(perf_cases) / sizeof(struct perf_case*);

	for (int i = 0; i < case_num; i++)
		if (!strcmp(name, perf_cases[i]->name))
			return perf_cases[i];

	return NULL;
}

struct perf_run* perf_case_create_run(struct perf_case *p_case)
{
	struct perf_run *p_run;
	struct perf_event *events;
	int event_num, stat_num, err;

	if (!p_case)
		return NULL;

	p_run = malloc(sizeof(struct perf_run));
	memset(p_run, 0, sizeof(struct perf_run));

	if (p_case->events) {
		events = p_case->events;
		event_num = p_case->event_num;
	} else {
		events = default_events;
		event_num = sizeof(default_events) / sizeof(struct perf_event);
	}

	stat_num = ceill((float)event_num / (float)MAX_PERF_EVENTS);
	p_run->stats = malloc(sizeof(struct perf_stat) * stat_num);
	p_run->stat_num = stat_num;
	p_run->p_case = p_case;

	for (int i = 0; i < stat_num; i++) {
		err = perf_stat_init(								\
			&p_run->stats[i], p_case->name,						\
			events + MAX_PERF_EVENTS * i, 						\
			(i == stat_num - 1) ? event_num % MAX_PERF_EVENTS : MAX_PERF_EVENTS	\
		);
		if (err)
			goto ERR_EXIT;
	}

	return p_run;

ERR_EXIT:
	free(p_run->stats);
	free(p_run);
	return NULL;
}

void perf_case_destroy_run(struct perf_run *p_run)
{
	free(p_run->stats);
	free(p_run);
}

int perf_case_run(struct perf_run* p_run, int argc, char **argv)
{
	struct perf_case *p_case;
	struct perf_stat *p_stat;
	long total_dur = 0;
	int err;

	p_case = p_run->p_case;

	for (int i = 0; i < p_run->stat_num; i++) {

		p_stat = &p_run->stats[i];

		if (p_case->init) {
			err = p_case->init(p_case, p_stat, argc, argv);
			if (err)
				return ERROR;
		}

		if (!p_case->inner_stat)
			perf_stat_begin(&p_run->stats[i]);

		p_case->func(p_case, p_stat);

		if (!p_case->inner_stat)
			perf_stat_end(&p_run->stats[i]);

		if (p_case->exit) {
			err = p_case->exit(p_case, p_stat);
			if (err)
				return ERROR;
		}

		if (!p_run->min_dur || p_stat->duration < p_run->min_dur)
			p_run->min_dur = p_stat->duration;
		if (!p_run->max_dur || p_stat->duration > p_run->max_dur)
			p_run->max_dur = p_stat->duration;
		total_dur += p_stat->duration;
	}

	p_run->avg_dur = total_dur / p_run->stat_num;

	return SUCCESS;
}

void perf_case_report_run(struct perf_run *p_run)
{
	printf("-----------------------\n");
	for (int i = 0; i < p_run->stat_num; i++)
		for (int j = 0; j < p_run->stats[i].event_num; j++)
			printf("%16s: %16ld\n",				\
				p_run->stats[i].events[j].event_name,	\
				p_run->stats[i].event_counts[j]		\
			);
	printf("-----------------------\n");
	if (p_run->stat_num > 1) {
		printf("finished with %d runs:\n", p_run->stat_num);
		printf("  min time: %f ms\n", (double)p_run->min_dur / 1000000);
		printf("  max time: %f ms\n", (double)p_run->max_dur / 1000000);
		printf("  avg time: %f ms\n", (double)p_run->avg_dur / 1000000);
	} else {
		printf("time: %f ms\n", (double)p_run->avg_dur / 1000000);
	}
}

void run_case(struct perf_case *p_case, int argc, char **argv)
{
	int err;
	struct perf_run *p_run;

	if (!p_case)
		return;

	p_run = perf_case_create_run(p_case);
	if (!p_run) {
		printf("ERROR: Failed to create a run.\n");
		exit(0);
	}

	printf("%s\n", p_case->name);
	printf("-----------------------\n");

	err = perf_case_run(p_run, argc, argv);
	if (err) {
		printf("ERROR: Case run failed.\n");
		exit(0);
	}

	perf_case_report_run(p_run);

	perf_case_destroy_run(p_run);
}

static void print_help()
{
	struct perf_case *p_case;
	int case_num = sizeof(perf_cases) / sizeof(struct perf_case*);

	printf("\n");
	printf("Perf-Case\n");
	printf("==========================\n");
	printf("A simple perf event test framework and some interesting testcases.\n\n");
	printf("Usage:\n");
	printf("    ./perf_case [case] [options]      // run a case\n");
	printf("    ./perf_case -h                    // help\n");
	printf("    ./perf_case -h [case]             // help for each case\n\n");
	printf("Cases Available:\n");
	for (int i = 0; i < case_num; i++) {
		p_case = perf_cases[i];
		printf("    %-20s - %s\n", p_case->name, p_case->desc);
	}
	printf("\n");
}

static void print_case_help(struct perf_case *p_case)
{
	printf("%s\n", p_case->name);
	printf("==========================\n");
	printf("%s\n\n", p_case->desc);
	printf("Usage:\n");
	printf("    ./perf_case %s [options]\n", p_case->name);
	printf("Options:\n");
	if (p_case->help) {
		p_case->help(p_case);
	} else {
		printf("    [no options]\n");
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	char *case_name;
	struct perf_case *p_case;

	if (argc < 2) {
		print_help();
		return 0;
	}

	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		if (argc == 2 || argv[2][0] == '-') {
			print_help();
			return 0;
		}
		case_name = argv[2];
		p_case = perf_case_find(case_name);
		if (!p_case) {
			printf("ERROR: No case named \"%s\"\n", case_name);
			return 0;
		}
		print_case_help(p_case);
		return 0;
	}

	case_name = argv[1];

	p_case = perf_case_find(case_name);
	if (!p_case) {
		printf("ERROR: No case named \"%s\"\n", case_name);
		return 0;
	}

	if (argc > 2 && (!strcmp(argv[2], "-h") || !strcmp(argv[2], "--help"))) {
		print_case_help(p_case);
		return 0;
	}

	run_case(p_case, argc, argv);

	return 0;
}
