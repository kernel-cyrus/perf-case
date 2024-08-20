# Introduction
Perf case is a simple perf event test framework with some interesting testcases to test the CPU micro-architecture and behavious for ARM.

You can simply run one case and collect all the PMU events or write testcase of your own.

![image](https://github.com/user-attachments/assets/5deb3ccc-82a9-4186-a23a-74fe656d1a92)

# Run a simple case

```
$ sudo ./perf_case membw_rd_1 -e armv8
```

All the PMU events are collected:

```
membw_rd_1
-----------------------
bufsize: 128.000000 MB
width: 1 bytes (8bit)
stride: 1 bytes
iterations: 1
2027.895 MB/s (0.063120 ms)
-----------------------
    cpu-cycles              :        136623561
    instructions            :        537019216
    cache-refs              :        134271675
    cache-misses            :             3442
    page-faults             :                0
    context-switch          :                0
    mem_access              :        134287865
    mem_access_rd           :        134263481
    mem_access_wr           :            24336
    br_pred                 :        134264456
    br_miss_pred            :              933
    br_retired              :        134255659
    br_miss_pred_retired    :              740
    stall                   :          2916860
    stall_frontend          :          2820420
    stall_backend           :            98750
    stall_slot              :        420835130
    stall_slot_backend      :           659117
    stall_slot_frontend     :        428222445
    l1i_tlb                 :           348508
    l1i_tlb_refill          :               65
    l2i_tlb                 :                0
    l2i_tlb_refill          :                0
    l1d_tlb                 :        134284378
    l1d_tlb_rd              :        134256733
    l1d_tlb_wr              :            20577
    l1d_tlb_refill          :            33070
    l1d_tlb_refill_rd       :            33040
    l1d_tlb_refill_wr       :               30
    l2d_tlb                 :            33213
    l2d_tlb_rd              :            33126
    l2d_tlb_wr              :               35
    l2d_tlb_refill          :            32946
    l2d_tlb_refill_rd       :            32917
    l2d_tlb_refill_wr       :               29
    itlb_walk               :                0
    dtlb_walk               :            32965
    l1i_cache               :        134384636
    l1i_cache_refill        :             3425
    l1i_cache_lmiss         :             3415
    l2i_cache               :                0
    l2i_cache_refill        :                0
    l2i_cache_lmiss         :                0
    l1d_cache               :        134289693
    l1d_cache_rd            :        134264643
    l1d_cache_wr            :            25037
    l1d_cache_refill        :             4303
    l1d_cache_refill_rd     :             3533
    l1d_cache_refill_wr     :              725
    l1d_cache_refill_inner  :             3697
    l1d_cache_refill_outer  :              188
    l1d_cache_allocate      :                0
    l1d_cache_inval         :              815
    l1d_cache_wb            :             2078
    l1d_cache_wb_clean      :              148
    l1d_cache_wb_victim     :             1743
    l1d_cache_lmiss_rd      :             2809
    l2d_cache               :          4235694
    l2d_cache_rd            :          2135312
    l2d_cache_wr            :          2100398
    l2d_cache_refill        :            11166
    l2d_cache_refill_rd     :            11182
    l2d_cache_refill_wr     :                0
    l2d_cache_allocate      :                0
    l2d_cache_inval         :              314
    l2d_cache_wb            :           147104
    l2d_cache_wb_clean      :              286
    l2d_cache_wb_victim     :           141930
    l2d_cache_lmiss_rd      :             8411
    l3d_cache               :          2247404
    l3d_cache_rd            :          2105494
    l3d_cache_wr            :                0
    l3d_cache_refill        :          2102154
    l3d_cache_refill_rd     :                0
    l3d_cache_refill_wr     :                0
    l3d_cache_allocate      :           146092
    l3d_cache_inval         :                0
    l3d_cache_wb            :                0
    l3d_cache_wb_clean      :                0
    l3d_cache_wb_victim     :                0
    l3d_cache_lmiss_rd      :          2102339
    llc_cache               :                0
    llc_cache_rd            :          2106613
    llc_cache_miss          :                0
    llc_cache_miss_rd       :          2102147
    bus_cycles              :        137057523
    bus_access              :          9003180
    bus_access_rd           :          8425292
    bus_access_wr           :           577092
    bus_access_normal       :                0
    bus_access_periph       :                0
    bus_access_shared       :                0
    bus_access_not_shared   :                0
    inst_spec               :        537069467
    inst_retired            :        537016200
    op_spec                 :        402902614
    op_retired              :        402937453
    ld_spec                 :        134259393
    st_spec                 :            18426
    ldrex_spec              :                0
    strex_spec              :                0
    dp_spec                 :        268534990
    vfp_spec                :                0
    ase_spec                :                0
-----------------------
finished with 18 runs:
    min time: 61.833936 ms
    max time: 63.223458 ms
    avg time: 62.645802 ms
```

# Guide

**Download**

```
git clone https://github.com/kernel-cyrus/perf-case.git
```

**Build**

```
make
```

**Show all testcase**

```
./perf_case -h
```

**Run one case**

```
./perf_case membw_rd_1
```

The case shows memory read bandwith on 1 byte sequential read pattern with basic PMU events reported.

**Run one case and report all PMU events**

```
./perf_case membw_rd_1 -e armv8
```

The case will run several times to collect all the PMU events and report to console.

**Show available options for a case**

```
./perf_case -h membw_rd_1
```
Each case may have its own options can be set.

**Set param for a case (set buffer size and stride)**

```
./perf_case membw_rd_1 -b 4096 -s 64
```
This test operate a 1 byte read for each 64 bytes, total read size is 4KB.

**Run case on specified CPU (CPU 1)**

```
./perf_case membw_rd_1 -c 1
```

The test thread is bind to CPU 0 by default, you can choose to bind to another core.

# Write Case

Follow a case in /cases/xxx.c

