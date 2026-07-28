# Suggested Public-Review Announcement

PVC-RotHash-1 1.0.0-rc1 is now frozen and available for independent
cryptanalysis.

The candidate is a 256-bit experimental hash construction based on the Perfect
Value Cube, state-dependent intersecting line rotations, a reverse foldback
pass, full-cube closure, and a four-diagonal squeeze. It deliberately embeds no
existing cryptographic primitive.

The repository includes a normative specification, C++ and independent
pure-Python implementations, official digest and full phase-state vectors,
reduced-round interfaces, historical attack tools, known weaknesses, and
explicit public challenge targets.

Important: this is not a production hash and no collision, preimage, or
post-quantum security claim is made. The purpose of the frozen release is to
make the algorithm stable enough for external researchers to attack.

Repository:
https://github.com/azizgokhannarin/perfectvaluecubehash

Suggested tag:
`v1.0.0-rc1`
