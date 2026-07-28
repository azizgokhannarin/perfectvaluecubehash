# Contributing

Contributions should prioritize falsification, reproducibility, and precise
claims.

## Rules

1. Do not add an existing cryptographic primitive to the candidate algorithm.
2. Analysis tools may use general programming and statistical techniques.
3. Every algorithm change must update `docs/DESIGN.md`.
4. Every discovered weakness must be recorded.
5. Avoid claims such as "secure", "quantum-safe", or "collision-resistant"
   without evidence and independent review.

## Build before submitting

```bash
cmake -S . -B build -DPVC_WARNINGS_AS_ERRORS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```


## Cryptanalysis contributions

- Keep the canonical `RotHash1` parameters unchanged unless a separate design
  revision is explicitly proposed.
- Use `pvc/research.hpp` for reduced-round experiments.
- Distinguish forward-state convergence, after-foldback state collision, final-
  state collision, truncated digest collision, and full digest collision.
- Compact indexing fingerprints are permitted only when reported collisions are
  verified against the complete operational state.
- Include the exact command, preset, input domain, and colliding messages.
