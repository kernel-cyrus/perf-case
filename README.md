# Introduction
Perf case is a simple perf event test framework with some interesting testcases to test the CPU micro-architecture and behavious for ARM.

You can simply run one case and checkout all the PMU events or write testcase of your own.

# Guide

Download

`git clone https://github.com/kernel-cyrus/perf-case.git`

Build

`make`

Show all testcase

`./perf_case -h`

Run one case

`./perf_case membw_rd_1`

Run one case and report all PMU events

`./perf_case membw_rd_1 -e armv8`

Show available options for a case

`./perf_case -h membw_rd_1`

Set param for a case (set buffer size and stride)

`./perf_case membw_rd_1 -b 4096 -s 64`

Run on specified CPU (CPU 1)

`./perf_case membw_rd_1 -c 1``

# Write Case

Follow a case in /cases/xxx.c

