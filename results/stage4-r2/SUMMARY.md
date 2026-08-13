# Stage 4 deep campaigns — R5-rothash2

- date: 2026-08-13T21:30:11Z
- mode: standard
- build: /home/agn/Projects/perfectvaluecubehash/build
- host: nuc nproc=12
- note: experimental; not production; no security claim


### collision-1byte
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-collision-probe --rothash2 1
EXIT=0 elapsed=0s
No collision among all 256 one-byte inputs (RotHash-2).
```

### collision-r1-pair
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-collision-probe --rothash2 r1-pair
EXIT=0 elapsed=0s
Known RotHash-1 forward pair digests differ under RotHash-2.
```

### collision-2byte
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-collision-probe --rothash2 2
EXIT=0 elapsed=6s
No collision among all 65,536 two-byte inputs (RotHash-2).
```

### transition-d1
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-transition-collision --preset R5-rothash2 --depth 1
EXIT=0 elapsed=0s
Transition collision search
preset=R5-rothash2 depth=1 prefix=
No exact state collision among 256 sequences from this start state.
```

### transition-d2
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-transition-collision --preset R5-rothash2 --depth 2
EXIT=0 elapsed=0s
Transition collision search
preset=R5-rothash2 depth=2 prefix=
No exact state collision among 65536 sequences from this start state.
```

### phase-1byte
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-phase-collision --preset R5-rothash2 --message-bytes 1
EXIT=0 elapsed=0s
Phase collision enumeration
preset=R5-rothash2 message_bytes=1 domain=256
phase,collisions,distinct_fingerprints
digest_collisions=0 distinct_digests=256
```

### phase-2byte
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-phase-collision --preset R5-rothash2 --message-bytes 2
EXIT=0 elapsed=7s
Phase collision enumeration
preset=R5-rothash2 message_bytes=2 domain=65536
phase,collisions,distinct_fingerprints
digest_collisions=0 distinct_digests=65536
```

### return-alias-surface
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-return-alias-surface --preset R5-rothash2 --messages 512 --message-bytes 8
EXIT=0 elapsed=0s
Reachable foldback-controller alias surface
preset=R5-rothash2 messages=512 message_bytes=8
reverse_depth=0 contexts=512 contexts_with_alias=0 alias_pairs=0 max_aliases_in_context=0
reverse_depth=1 contexts=512 contexts_with_alias=0 alias_pairs=0 max_aliases_in_context=0
reverse_depth=2 contexts=512 contexts_with_alias=0 alias_pairs=0 max_aliases_in_context=0
reverse_depth=3 contexts=512 contexts_with_alias=0 alias_pairs=0 max_aliases_in_context=0
reverse_depth=4 contexts=512 contexts_with_alias=0 alias_pairs=0 max_aliases_in_context=0
reverse_depth=5 contexts=512 contexts_with_alias=0 alias_pairs=0 max_aliases_in_context=0
reverse_depth=6 contexts=512 contexts_with_alias=0 alias_pairs=0 max_aliases_in_context=0
reverse_depth=7 contexts=512 contexts_with_alias=0 alias_pairs=0 max_aliases_in_context=0
reverse_depth=8 contexts=512 contexts_with_alias=0 alias_pairs=0 max_aliases_in_context=0
total_contexts=4608
contexts_with_alias=0
context_alias_probability=0.00000000
total_alias_pairs=0
mean_alias_pairs_per_context=0.00000000
maximum_alias_pairs_in_context=0
alias_count,contexts
```

### related-input
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-related-input-probe --preset R5-rothash2 --samples 256
EXIT=0 elapsed=4s
preset=R5-rothash2 samples=256 message_bytes=16
relation=reverse mean_digest_bits=127.4570 digest_matches=0 state_matches=0
relation=complement mean_digest_bits=128.7266 digest_matches=0 state_matches=0
relation=rotate-left-one mean_digest_bits=128.4141 digest_matches=0 state_matches=0
permutation_domain=ABCDEFGH unique_messages=40320 digest_collisions=0 state_collisions=0
permutation_domain=AABBCCDD unique_messages=2520 digest_collisions=0 state_collisions=0
permutation_domain=00001111 unique_messages=70 digest_collisions=0 state_collisions=0
```

### alignment
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-alignment-probe --preset R5-rothash2
EXIT=0 elapsed=0s
preset=R5-rothash2 delta=42
tested_pairs=54784
pairs_with_at_least_one_equal_move=19200
```

### length-framing
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-length-framing-probe --preset R5-rothash2
EXIT=0 elapsed=0s
preset=R5-rothash2 maximum_length=64
unique_forward_symbol_indices=65
unique_foldback_symbol_indices=65
unique_final_symbol_indices=65
note=This blocks classic identical-operational-state expandable messages; it does not prove unequal-length digest collision resistance.
```

### differential
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-differential-search --preset R5-rothash2 --samples 16
EXIT=0 elapsed=1s
preset=R5-rothash2 samples=16 message_bytes=16 mode=single
phase,index,mean_cube_bits,min,max,mean_cube_bytes,min,max,exact_states
digest comparisons=2048 mean_bits=128.4619 min=103 max=154 exact_digest_matches=0
```

### multicollision
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-multicollision-probe --preset R5-rothash2 --prefix-count 4096 --max-levels 4 --suffix-bytes 0 --suffix-limit 1
EXIT=0 elapsed=0s
Forward multicollision-chain probe
preset=R5-rothash2 prefix_count=4096 max_levels=4 suffix_bytes=0 suffix_domain=1
three_byte_seed_pairs=0
seeds_with_next_symbol_alias=0
next_symbol_alias_pairs=0
maximum_collision_levels=1
extension_cases_with_foldback_collision=0
extension_cases_with_digest_collision=0
next_alias_delta,hits
```

### foldback-aware
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-foldback-aware-alias --preset R5-rothash2 --prefix-count 4096 --suffix-bytes 0 --suffix-limit 1
EXIT=0 elapsed=0s
Foldback-aware controller alias search
preset=R5-rothash2 prefix_count=4096 suffix_bytes=0 suffix_domain=1
forward_collision_pairs=0
local_alias_pairs=0
inherited_pairs=0
```

### dual-return
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-dual-return-alias --preset R5-rothash2 --prefix-count 4096 --suffix-bytes 0 --suffix-limit 1 --no-family-surface
EXIT=0 elapsed=0s
Dual return-alias search (Phase 1-b)
preset=R5-rothash2 prefix_count=4096 suffix_bytes=0 suffix_domain=1 family_surface=no
forward_collision_pairs=0
minimum_direct_return_state_bits=18446744073709551615
minimum_after_foldback_state_bits=18446744073709551615
```

### bridged
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-bridged-multicollision --preset R5-rothash2 --prefix-count 4096 --levels 8 --materialize-levels 0
EXIT=1 (treated as negative evidence) elapsed=0s
Bridged forward multicollision construction
preset=R5-rothash2 requested_levels=8 theoretical_messages=2^8 seed_pairs=0
path_found=no
```

### three-byte-forward
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-three-byte-collision --preset R5-rothash2 --phase forward --first-byte-count 32
EXIT=0 elapsed=0s
Three-byte exact state collision scan
preset=R5-rothash2 first_byte_count=32 domain=2097152 threads=12
exact_state_pairs,0
inherited_pairs,0
new_convergence_pairs,0
delta42_pairs,0
local_third_symbol_pairs,0
multi_position_pairs,0
after_foldback_pairs,0
```

### three-byte-foldback
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-three-byte-collision --preset R5-rothash2 --phase foldback --first-byte-count 32
EXIT=0 elapsed=1s
Three-byte exact state collision scan
preset=R5-rothash2 first_byte_count=32 domain=2097152 threads=12
exact_state_pairs,0
inherited_pairs,0
new_convergence_pairs,0
delta42_pairs,0
local_third_symbol_pairs,0
multi_position_pairs,0
after_foldback_pairs,0
```

### truncated-campaign
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-truncated-campaign --preset R5-rothash2 --bits 24,32,40 --trials 4 --limit 200000
EXIT=0 elapsed=109s
Truncated collision multi-width campaign
preset=R5-rothash2 trials=4 limit_per_trial=200000 message_bytes=16
bits,found,censored,mean_messages,expected,mean_ratio,min,max
```

### foldback-separation
```
cmd: /home/agn/Projects/perfectvaluecubehash/build/pvc-foldback-separation-profile --preset R5-rothash2 --prefix-count 1024
EXIT=0 elapsed=0s
preset=R5-rothash2 prefix_count=1024 forward_collision_pairs=0
direct_return_transition_aliases=0
gate_cube_bit_distance_mean=0.0000
gate_cube_byte_distance_mean=0.0000
final_cube_bit_distance_mean=0.0000
final_cube_byte_distance_mean=0.0000
```

## Campaign completion

- tool failures (non-zero exit): 0
- logs directory: `/home/agn/Projects/perfectvaluecubehash/results/stage4-r2`

Interpret zero structural hits as **budget-limited negative evidence**, not
a security claim. Production use remains prohibited.

### three-byte-forward-full
```
Three-byte exact state collision scan
phase=forward
preset=R5-rothash2 first_byte_count=256 domain=16777216 threads=12
moves_per_symbol=6 diagonal_closure=64 orbit_closure=128 squeeze_bytes=32 squeeze_symbols_per_byte=4 foldback=on

metric,value
phase,forward
domain,16777216
fingerprint_buckets,0
fingerprint_candidate_messages,0
exact_state_groups,0
duplicate_messages,0
exact_state_pairs,0
inherited_pairs,0
new_convergence_pairs,0
delta42_pairs,0
local_third_symbol_pairs,0
local_identical_move_paths,0
local_distinct_move_paths,0
multi_position_pairs,0
after_foldback_pairs,0
local_third_absolute_delta,hits
generation_seconds,0.890
radix_sort_seconds,0.774
verification_seconds,0.047
elapsed=1,758
```

### three-byte-foldback-full
```
Three-byte exact state collision scan
phase=foldback
preset=R5-rothash2 first_byte_count=256 domain=16777216 threads=12
moves_per_symbol=6 diagonal_closure=64 orbit_closure=128 squeeze_bytes=32 squeeze_symbols_per_byte=4 foldback=on

metric,value
phase,foldback
domain,16777216
fingerprint_buckets,0
fingerprint_candidate_messages,0
exact_state_groups,0
duplicate_messages,0
exact_state_pairs,0
inherited_pairs,0
new_convergence_pairs,0
delta42_pairs,0
local_third_symbol_pairs,0
local_identical_move_paths,0
local_distinct_move_paths,0
multi_position_pairs,0
after_foldback_pairs,0
local_third_absolute_delta,hits
generation_seconds,3.951
radix_sort_seconds,0.757
verification_seconds,0.047
elapsed=4,802
```
