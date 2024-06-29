#ifndef __PERF_TEST_H
#define __PERF_TEST_H

typedef void *perf_test_func();

struct perf_test {
	const char *name;
	const char *desc;
	perf_test_func *func;
};

#endif