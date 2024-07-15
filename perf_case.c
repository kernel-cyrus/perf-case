#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <sched.h>
#include <sys/mman.h>
#include "perf_case.h"
#include "perf_stat.h"

static int opt_cpu = 0;

static struct perf_option default_options[] = {
	{{"help",   optional_argument, NULL, 'h' }, "h",  "Help."},
	{{"cpu",    optional_argument, NULL, 'c' }, "c:", "Choose a CPU to run."},
	{{"events", optional_argument, NULL, 'e' }, "e:", "Run case with events. (case|default|armv8)."},
};

static struct perf_eventset *g_eventset = NULL;

static struct perf_event default_events[] = {
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES, "cpu-cycles"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "instructions"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES, "cache-refs"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND, "stall-frontend"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND, "stall-backend"),
	PERF_EVENT(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, "page-faults")
};

static struct perf_event armv8_events[] = {
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES, "cpu-cycles"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, "instructions"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES, "cache-refs"),
	PERF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES, "cache-misses"),
	PERF_RAW_EVENT("armv8_pmuv3/stall", 			NULL),
	PERF_RAW_EVENT("armv8_pmuv3/stall_backend", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/stall_backend_mem", 	NULL),
	PERF_RAW_EVENT("armv8_pmuv3/stall_frontend", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/stall_slot", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/stall_slot_backend", 	NULL),
	PERF_RAW_EVENT("armv8_pmuv3/stall_slot_frontend", 	NULL),
	PERF_RAW_EVENT("armv8_pmuv3/mem_access", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/dtlb_walk", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/l1d_tlb", 			NULL),
	PERF_RAW_EVENT("armv8_pmuv3/l1d_tlb_refill", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/l2d_tlb", 			NULL),
	PERF_RAW_EVENT("armv8_pmuv3/l2d_tlb_refill", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/l1d_cache", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/l1d_cache_refill", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/l2d_cache", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/l2d_cache_refill", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/l3d_cache", 		NULL),
	PERF_RAW_EVENT("armv8_pmuv3/l3d_cache_refill", 		NULL),
	PERF_RAW_EVENT("scf_pmu/scf_cache", 			NULL),
	PERF_RAW_EVENT("scf_pmu/scf_cache_refill", 		NULL),
};

static struct perf_eventset perf_event_sets[] = {
	PERF_EVENT_SET("default", default_events),
	PERF_EVENT_SET("armv8", armv8_events),
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
	PERF_CASE(memlat_random),
};

struct perf_case* perf_case_find(char* name)
{
	int case_num = sizeof(perf_cases) / sizeof(struct perf_case*);

	for (int i = 0; i < case_num; i++)
		if (!strcmp(name, perf_cases[i]->name))
			return perf_cases[i];

	return NULL;
}

struct perf_eventset* perf_eventset_find(char* name)
{
	int num = sizeof(perf_event_sets) / sizeof(struct perf_eventset);

	for (int i = 0; i < num; i++)
		if (!strcmp(name, perf_event_sets[i].name))
			return &perf_event_sets[i];

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

	if (g_eventset) {
		events = g_eventset->events;
		event_num = g_eventset->event_num;
	} else if (p_case->events) {
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
			printf("    %-20s: %16ld\n",				\
				p_run->stats[i].events[j].event_name,	\
				p_run->stats[i].event_counts[j]		\
			);
	printf("-----------------------\n");
	if (p_run->stat_num > 1) {
		printf("finished with %d runs:\n", p_run->stat_num);
		printf("    min time: %f ms\n", (double)p_run->min_dur / 1000000);
		printf("    max time: %f ms\n", (double)p_run->max_dur / 1000000);
		printf("    avg time: %f ms\n", (double)p_run->avg_dur / 1000000);
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

static void print_default_opts()
{
	int def_num = sizeof(default_options) / sizeof(struct perf_option);

	for (int i = 0; i < def_num; i++) {
		printf("    -%c, --%-14s %s\n",			\
			(char)default_options[i].opt.val,	\
			default_options[i].opt.name,		\
			default_options[i].desc			\
		);
	}
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
	printf("Options:\n");
	print_default_opts();
	printf("\n");
	printf("Cases Available:\n");
	for (int i = 0; i < case_num; i++) {
		p_case = perf_cases[i];
		printf("    %-20s - %s\n", p_case->name, p_case->desc);
	}
	printf("\n");
}

static void print_case_opts(struct perf_case *p_case)
{
	for (int i = 0; i < p_case->opts_num; i++) {
		printf("    -%c, --%-14s %s\n",			\
			(char)p_case->opts[i].opt.val,		\
			p_case->opts[i].opt.name,		\
			p_case->opts[i].desc			\
		);
	}
}

static void print_case_help(struct perf_case *p_case)
{
	printf("%s\n", p_case->name);
	printf("==========================\n");
	printf("%s\n\n", p_case->desc);
	printf("Usage:\n");
	printf("    ./perf_case %s [options]\n", p_case->name);
	printf("Options:\n");
	print_default_opts();

	if (p_case->opts) {
		print_case_opts(p_case);
	} else {
		printf("    [no options]\n");
	}

	if (p_case->help) {
		p_case->help(p_case);
	}

	printf("\n");
}

static int set_cpu(int cpu)
{
	pid_t pid = getpid();
	cpu_set_t mask;

	CPU_ZERO(&mask);
	CPU_SET(cpu, &mask);

	return sched_setaffinity(pid, sizeof(mask), &mask);
}

static void init_opts(struct perf_case *p_case, int argc, char **argv)
{
	struct option *opts;
	char ostr[64];
	int opt, opt_idx;
	int opt_num, def_num;
	int i, j;
	int err;

	def_num = sizeof(default_options) / sizeof(struct perf_option);
	opt_num = def_num + p_case->opts_num;

	opts = malloc(sizeof(struct option) * (opt_num + 1));

	for (i = 0; i < def_num; i++) {
		memcpy(&opts[i], &default_options[i].opt, sizeof(struct option));
		strcat(ostr, default_options[i].ostr);
	}

	for (j = 0; j < p_case->opts_num; j++, i++) {
		memcpy(&opts[i], &p_case->opts[j].opt, sizeof(struct option));
		strcat(ostr, p_case->opts[j].ostr);
	}

	memset(&opts[i], 0, sizeof(struct option));

	while ((opt = getopt_long(argc, argv, ostr, opts, &opt_idx)) != -1) {
		switch (opt) {
		case 'h':
			print_case_help(p_case);
			exit(0);
		case 'c':
			opt_cpu = atoi(optarg);
			err = set_cpu(opt_cpu);
			if (err) {
				printf("ERROR: Set cpu affinity failed.\n");
				exit(0);
			}
			printf("Run on CPU: %d\n", opt_cpu);
			break;
		case 'e':
			if (!strcmp(optarg, "case"))
				break;
			g_eventset = perf_eventset_find(optarg);
			if (!g_eventset) {
				printf("ERROR: No event set named \"%s\"\n", optarg);
				exit(0);
			}
			printf("Enable events: %s\n", g_eventset->name);
			break;
		default:
			if (!p_case->getopt(p_case, opt))
				break;
			printf("ERROR: Invalid parameter, please run \"./perf_case -h\" for help.\n");
			exit(0);
		}
	}

	free(opts);
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

	init_opts(p_case, argc, argv);

	run_case(p_case, argc, argv);

	return 0;
}
