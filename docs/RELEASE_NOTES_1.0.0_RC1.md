# PVC-RotHash-1 1.0.0-rc1 Release Notes

This is the first frozen candidate intended for independent public
cryptanalysis. It does not introduce an algorithmic change over the analyzed
PVC-RotHash-1 construction and preserves the established anchor digests.

The release adds:

- a self-contained normative specification;
- a second, pure-Python conformance implementation;
- official digest and phase-state vectors;
- automated cross-implementation verification;
- a candidate freeze and change policy;
- consolidated known cryptanalysis and explicit non-claims;
- a public cryptanalysis challenge and reporting templates;
- reproducibility, citation, authorship, and AI-assistance disclosures.

The repository remains experimental and unsuitable for production security.
The purpose of this release is to make the target stable enough for outsiders
to attack without the algorithm moving underneath their analysis.
