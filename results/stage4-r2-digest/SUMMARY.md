# Stage 4 digest-surface — R5-rothash2

- date: 2026-08-13T21:38:06Z
- mode: standard
- note: experimental; not production; no security claim


### digest-beam-same-forward
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-digest-beam-search --preset R5-rothash2 --levels 8 --beam 64 --prefix-count 4096 --print-limit 0
EXIT=1 (negative / no path) elapsed=0s
path_found=no
```

### divergent-beam-000000-000001
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-divergent-digest-beam --preset R5-rothash2 --left 000000 --right 000001 --depth 8 --beam 128 --branch 16 --print-limit 0
EXIT=0 elapsed=3s
Digest-guided beam over deliberately different forward states
preset=R5-rothash2 seed_left=000000 seed_right=000001 depth=8 beam_width=128 branch=16
depth=0 best_digest_bits=116 forward_bits=261
depth=1 expanded=256 generic_level_min_digest_bits=107 generic_cumulative_min_digest_bits=107 best_digest_bits=107 best_forward_bits=572
depth=2 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=96 best_digest_bits=96 best_forward_bits=829
depth=3 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=95 best_digest_bits=90 best_forward_bits=1019
depth=4 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=94 best_digest_bits=93 best_forward_bits=1154
depth=5 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=94 best_digest_bits=99 best_forward_bits=1333
depth=6 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=93 best_digest_bits=97 best_forward_bits=1572
depth=7 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=93 best_digest_bits=96 best_forward_bits=1603
depth=8 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=93 best_digest_bits=97 best_forward_bits=1607
cumulative_pairs_evaluated=229633
generic_cumulative_min_digest_bits=93
global_best_depth=3
global_best_digest_bits=90
global_best_forward_bits=1019
global_best_left=000000f8cdd2
global_best_right=0000015cf116
exact_digest_collision=no
phase=after-forward state_bits=1019 state_bytes=250
```

### divergent-beam-a5c301-5a3cfe
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-divergent-digest-beam --preset R5-rothash2 --left a5c301 --right 5a3cfe --depth 8 --beam 128 --branch 16 --print-limit 0
EXIT=0 elapsed=4s
Digest-guided beam over deliberately different forward states
preset=R5-rothash2 seed_left=a5c301 seed_right=5a3cfe depth=8 beam_width=128 branch=16
depth=0 best_digest_bits=118 forward_bits=928
depth=1 expanded=256 generic_level_min_digest_bits=107 generic_cumulative_min_digest_bits=107 best_digest_bits=106 best_forward_bits=1194
depth=2 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=96 best_digest_bits=97 best_forward_bits=1347
depth=3 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=95 best_digest_bits=91 best_forward_bits=1311
depth=4 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=94 best_digest_bits=97 best_forward_bits=1543
depth=5 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=94 best_digest_bits=98 best_forward_bits=1600
depth=6 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=93 best_digest_bits=96 best_forward_bits=1786
depth=7 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=93 best_digest_bits=96 best_forward_bits=1849
depth=8 expanded=32768 generic_level_min_digest_bits=96 generic_cumulative_min_digest_bits=93 best_digest_bits=94 best_forward_bits=1887
cumulative_pairs_evaluated=229633
generic_cumulative_min_digest_bits=93
global_best_depth=3
global_best_digest_bits=91
global_best_forward_bits=1311
global_best_left=a5c301488de2
global_best_right=5a3cfeacf186
exact_digest_collision=no
phase=after-forward state_bits=1311 state_bytes=321
```

### lsh-000000-000001
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-digest-lsh-search --preset R5-rothash2 --left 000000 --right 000001 --suffix-bytes 2 --suffix-limit 4096 --projections 32 --projection-bytes 1
EXIT=0 elapsed=1s
Digest-LSH search across different forward-state families
preset=R5-rothash2 left_prefix=000000 right_prefix=000001 suffix_bytes=2 suffixes_per_side=4096 logical_cross_pairs=16777216 projections=32 projection_bytes=1
projection=0 cumulative_candidates=65132 best_digest_bits=90
projection=1 cumulative_candidates=130479 best_digest_bits=83
projection=2 cumulative_candidates=196172 best_digest_bits=83
projection=3 cumulative_candidates=262039 best_digest_bits=83
projection=4 cumulative_candidates=327219 best_digest_bits=83
projection=5 cumulative_candidates=393084 best_digest_bits=83
projection=6 cumulative_candidates=459149 best_digest_bits=83
projection=7 cumulative_candidates=524251 best_digest_bits=83
projection=8 cumulative_candidates=589869 best_digest_bits=83
projection=9 cumulative_candidates=655807 best_digest_bits=83
projection=10 cumulative_candidates=721286 best_digest_bits=83
projection=11 cumulative_candidates=787033 best_digest_bits=83
projection=12 cumulative_candidates=852348 best_digest_bits=83
projection=13 cumulative_candidates=917452 best_digest_bits=83
projection=14 cumulative_candidates=983206 best_digest_bits=83
projection=15 cumulative_candidates=1048577 best_digest_bits=83
projection=16 cumulative_candidates=1114102 best_digest_bits=83
projection=17 cumulative_candidates=1179626 best_digest_bits=83
projection=18 cumulative_candidates=1244941 best_digest_bits=83
projection=19 cumulative_candidates=1310506 best_digest_bits=83
projection=20 cumulative_candidates=1375770 best_digest_bits=83
projection=21 cumulative_candidates=1441605 best_digest_bits=83
projection=22 cumulative_candidates=1507066 best_digest_bits=83
projection=23 cumulative_candidates=1572254 best_digest_bits=83
projection=24 cumulative_candidates=1637642 best_digest_bits=83
projection=25 cumulative_candidates=1703604 best_digest_bits=83
projection=26 cumulative_candidates=1769380 best_digest_bits=83
projection=27 cumulative_candidates=1834949 best_digest_bits=83
projection=28 cumulative_candidates=1900571 best_digest_bits=83
projection=29 cumulative_candidates=1966337 best_digest_bits=83
projection=30 cumulative_candidates=2032361 best_digest_bits=83
projection=31 cumulative_candidates=2097593 best_digest_bits=83
candidate_pairs_evaluated=2097593
forward_equal_skipped=0
generic_cross_min_digest_bits=86
best_digest_bits=83
exact_digest_collision=no
phase=after-forward state_bits=732 state_bytes=191
```

### lsh-102030-102031
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-digest-lsh-search --preset R5-rothash2 --left 102030 --right 102031 --suffix-bytes 2 --suffix-limit 4096 --projections 32 --projection-bytes 1
EXIT=0 elapsed=1s
Digest-LSH search across different forward-state families
preset=R5-rothash2 left_prefix=102030 right_prefix=102031 suffix_bytes=2 suffixes_per_side=4096 logical_cross_pairs=16777216 projections=32 projection_bytes=1
projection=0 cumulative_candidates=65715 best_digest_bits=91
projection=1 cumulative_candidates=131588 best_digest_bits=91
projection=2 cumulative_candidates=196996 best_digest_bits=91
projection=3 cumulative_candidates=262432 best_digest_bits=91
projection=4 cumulative_candidates=328110 best_digest_bits=90
projection=5 cumulative_candidates=394095 best_digest_bits=90
projection=6 cumulative_candidates=459408 best_digest_bits=90
projection=7 cumulative_candidates=524944 best_digest_bits=90
projection=8 cumulative_candidates=590715 best_digest_bits=90
projection=9 cumulative_candidates=656253 best_digest_bits=90
projection=10 cumulative_candidates=722062 best_digest_bits=90
projection=11 cumulative_candidates=787314 best_digest_bits=90
projection=12 cumulative_candidates=852734 best_digest_bits=90
projection=13 cumulative_candidates=918073 best_digest_bits=88
projection=14 cumulative_candidates=983545 best_digest_bits=88
projection=15 cumulative_candidates=1048586 best_digest_bits=88
projection=16 cumulative_candidates=1113663 best_digest_bits=88
projection=17 cumulative_candidates=1178817 best_digest_bits=88
projection=18 cumulative_candidates=1244616 best_digest_bits=88
projection=19 cumulative_candidates=1309817 best_digest_bits=88
projection=20 cumulative_candidates=1375001 best_digest_bits=88
projection=21 cumulative_candidates=1441087 best_digest_bits=88
projection=22 cumulative_candidates=1506604 best_digest_bits=88
projection=23 cumulative_candidates=1572468 best_digest_bits=88
projection=24 cumulative_candidates=1638401 best_digest_bits=88
projection=25 cumulative_candidates=1703726 best_digest_bits=88
projection=26 cumulative_candidates=1769193 best_digest_bits=88
projection=27 cumulative_candidates=1834900 best_digest_bits=88
projection=28 cumulative_candidates=1901264 best_digest_bits=88
projection=29 cumulative_candidates=1966513 best_digest_bits=88
projection=30 cumulative_candidates=2032281 best_digest_bits=88
projection=31 cumulative_candidates=2098045 best_digest_bits=88
candidate_pairs_evaluated=2098045
forward_equal_skipped=0
generic_cross_min_digest_bits=86
best_digest_bits=88
exact_digest_collision=no
phase=after-forward state_bits=780 state_bytes=192
```

### lsh-abcdef-abcdee
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-digest-lsh-search --preset R5-rothash2 --left abcdef --right abcdee --suffix-bytes 2 --suffix-limit 4096 --projections 32 --projection-bytes 1
EXIT=0 elapsed=1s
Digest-LSH search across different forward-state families
preset=R5-rothash2 left_prefix=abcdef right_prefix=abcdee suffix_bytes=2 suffixes_per_side=4096 logical_cross_pairs=16777216 projections=32 projection_bytes=1
projection=0 cumulative_candidates=65676 best_digest_bits=92
projection=1 cumulative_candidates=131208 best_digest_bits=89
projection=2 cumulative_candidates=196619 best_digest_bits=87
projection=3 cumulative_candidates=262015 best_digest_bits=87
projection=4 cumulative_candidates=327238 best_digest_bits=87
projection=5 cumulative_candidates=393114 best_digest_bits=87
projection=6 cumulative_candidates=458795 best_digest_bits=87
projection=7 cumulative_candidates=524061 best_digest_bits=87
projection=8 cumulative_candidates=589813 best_digest_bits=87
projection=9 cumulative_candidates=655399 best_digest_bits=87
projection=10 cumulative_candidates=721076 best_digest_bits=87
projection=11 cumulative_candidates=786425 best_digest_bits=87
projection=12 cumulative_candidates=852568 best_digest_bits=87
projection=13 cumulative_candidates=918340 best_digest_bits=87
projection=14 cumulative_candidates=984052 best_digest_bits=87
projection=15 cumulative_candidates=1049301 best_digest_bits=87
projection=16 cumulative_candidates=1114831 best_digest_bits=87
projection=17 cumulative_candidates=1180410 best_digest_bits=87
projection=18 cumulative_candidates=1246239 best_digest_bits=86
projection=19 cumulative_candidates=1311523 best_digest_bits=86
projection=20 cumulative_candidates=1376967 best_digest_bits=86
projection=21 cumulative_candidates=1442107 best_digest_bits=86
projection=22 cumulative_candidates=1507715 best_digest_bits=86
projection=23 cumulative_candidates=1573373 best_digest_bits=86
projection=24 cumulative_candidates=1639086 best_digest_bits=86
projection=25 cumulative_candidates=1705045 best_digest_bits=86
projection=26 cumulative_candidates=1770821 best_digest_bits=86
projection=27 cumulative_candidates=1836268 best_digest_bits=86
projection=28 cumulative_candidates=1901914 best_digest_bits=86
projection=29 cumulative_candidates=1967348 best_digest_bits=86
projection=30 cumulative_candidates=2032805 best_digest_bits=86
projection=31 cumulative_candidates=2098118 best_digest_bits=86
candidate_pairs_evaluated=2098118
forward_equal_skipped=0
generic_cross_min_digest_bits=86
best_digest_bits=86
exact_digest_collision=no
phase=after-forward state_bits=858 state_bytes=203
```

### lsh-5a00c3-5a00c4
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-digest-lsh-search --preset R5-rothash2 --left 5a00c3 --right 5a00c4 --suffix-bytes 2 --suffix-limit 4096 --projections 32 --projection-bytes 1
EXIT=0 elapsed=1s
Digest-LSH search across different forward-state families
preset=R5-rothash2 left_prefix=5a00c3 right_prefix=5a00c4 suffix_bytes=2 suffixes_per_side=4096 logical_cross_pairs=16777216 projections=32 projection_bytes=1
projection=0 cumulative_candidates=65284 best_digest_bits=88
projection=1 cumulative_candidates=130426 best_digest_bits=88
projection=2 cumulative_candidates=196303 best_digest_bits=88
projection=3 cumulative_candidates=261048 best_digest_bits=88
projection=4 cumulative_candidates=326179 best_digest_bits=88
projection=5 cumulative_candidates=391280 best_digest_bits=88
projection=6 cumulative_candidates=456806 best_digest_bits=88
projection=7 cumulative_candidates=522382 best_digest_bits=88
projection=8 cumulative_candidates=587499 best_digest_bits=88
projection=9 cumulative_candidates=653006 best_digest_bits=86
projection=10 cumulative_candidates=718651 best_digest_bits=86
projection=11 cumulative_candidates=784118 best_digest_bits=86
projection=12 cumulative_candidates=849125 best_digest_bits=86
projection=13 cumulative_candidates=914068 best_digest_bits=86
projection=14 cumulative_candidates=979933 best_digest_bits=86
projection=15 cumulative_candidates=1045666 best_digest_bits=86
projection=16 cumulative_candidates=1111060 best_digest_bits=86
projection=17 cumulative_candidates=1177135 best_digest_bits=86
projection=18 cumulative_candidates=1242529 best_digest_bits=86
projection=19 cumulative_candidates=1307780 best_digest_bits=86
projection=20 cumulative_candidates=1373764 best_digest_bits=86
projection=21 cumulative_candidates=1439037 best_digest_bits=86
projection=22 cumulative_candidates=1504482 best_digest_bits=86
projection=23 cumulative_candidates=1569729 best_digest_bits=86
projection=24 cumulative_candidates=1635467 best_digest_bits=86
projection=25 cumulative_candidates=1701287 best_digest_bits=86
projection=26 cumulative_candidates=1766725 best_digest_bits=86
projection=27 cumulative_candidates=1831962 best_digest_bits=86
projection=28 cumulative_candidates=1897006 best_digest_bits=85
projection=29 cumulative_candidates=1962305 best_digest_bits=85
projection=30 cumulative_candidates=2028346 best_digest_bits=85
projection=31 cumulative_candidates=2094012 best_digest_bits=85
candidate_pairs_evaluated=2094012
forward_equal_skipped=0
generic_cross_min_digest_bits=86
best_digest_bits=85
exact_digest_collision=no
phase=after-forward state_bits=895 state_bytes=202
```

### barrier-independent
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-barrier-correlation --preset R5-rothash2 --left 000000 --right 000001 --samples 5000 --suffix-bytes 2 --independent-suffix
EXIT=0 elapsed=1s
Correlation of internal phase distance with final digest distance
preset=R5-rothash2 left_prefix=000000 right_prefix=000001 samples=5000 suffix_bytes=2 suffix_mode=independent
digest_mean_bits=128.035400
exact_digest_collisions=0
forward_equal_samples=0
phase=after-forward mean_state_bits=806.316000 min_state_bits=552 max_state_bits=993 digest_correlation=-0.020367
phase=after-foldback mean_state_bits=1553.384800 min_state_bits=1242 max_state_bits=1830 digest_correlation=-0.007755
phase=after-diagonal-closure mean_state_bits=2054.689400 min_state_bits=1913 max_state_bits=2170 digest_correlation=-0.001356
phase=after-orbit-closure mean_state_bits=2053.113400 min_state_bits=1945 max_state_bits=2163 digest_correlation=0.000869
phase=final mean_state_bits=2054.197800 min_state_bits=1919 max_state_bits=2164 digest_correlation=0.005590
```

### barrier-common
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-barrier-correlation --preset R5-rothash2 --left 000000 --right 000001 --samples 5000 --suffix-bytes 2 --common-suffix
EXIT=0 elapsed=1s
Correlation of internal phase distance with final digest distance
preset=R5-rothash2 left_prefix=000000 right_prefix=000001 samples=5000 suffix_bytes=2 suffix_mode=common
digest_mean_bits=128.022000
exact_digest_collisions=0
forward_equal_samples=0
phase=after-forward mean_state_bits=793.331400 min_state_bits=556 max_state_bits=960 digest_correlation=0.003370
phase=after-foldback mean_state_bits=1549.651400 min_state_bits=1166 max_state_bits=1822 digest_correlation=0.007351
phase=after-diagonal-closure mean_state_bits=2052.953800 min_state_bits=1934 max_state_bits=2154 digest_correlation=-0.001594
phase=after-orbit-closure mean_state_bits=2053.177800 min_state_bits=1902 max_state_bits=2162 digest_correlation=0.009397
phase=final mean_state_bits=2052.615000 min_state_bits=1940 max_state_bits=2175 digest_correlation=0.011063
```

### truncated-long
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-truncated-campaign --preset R5-rothash2 --bits 24,32,40,48 --trials 4 --limit 400000
EXIT=0 elapsed=328s
Truncated collision multi-width campaign
preset=R5-rothash2 trials=4 limit_per_trial=400000 message_bytes=16
bits,found,censored,mean_messages,expected,mean_ratio,min,max
```

## Campaign completion

- tool failures (true errors): 0
- logs: `/home/agn/Projects/perfectvaluecubehash/results/stage4-r2-digest`

Budget-limited negative evidence only. Production prohibited.

### lsh-000000-000001-8192 (ad-hoc R1-parity)
```
preset=R5-rothash2 left_prefix=000000 right_prefix=000001 suffix_bytes=2 suffixes_per_side=8192 logical_cross_pairs=67108864 projections=32 projection_bytes=1
candidate_pairs_evaluated=8389677
forward_equal_skipped=0
generic_cross_min_digest_bits=84
best_digest_bits=83
exact_digest_collision=no
```

### truncated-40-2m (ad-hoc)
```
Truncated collision multi-width campaign
preset=R5-rothash2 trials=4 limit_per_trial=2000000 message_bytes=16
moves_per_symbol=6 diagonal_closure=64 orbit_closure=128 squeeze_bytes=32 squeeze_symbols_per_byte=4 foldback=on
bits,found,censored,mean_messages,expected,mean_ratio,min,max
40,4,0,1176177.5000,1314195.1248,0.8950,795593,1356285
```
