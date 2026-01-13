# ParticleSIM(D)

## Description
ParticleSIM(D) is a particle simulator developed as a laboratory
for testing data-oriented design and low-level optimizations
on limited hardware (Android mobile). The main goal is not
realistic physics or advanced graphics, but to explore code
decisions that directly impact performance.

## Project Goals
- Experiment with Data-Oriented Design (DOD) to improve memory access and reduce cache misses.
- Apply SIMD (NEON/AArch64) to process multiple particles simultaneously.
- Test implementation choices on Android, observing OS and hardware limitations.
- Measure and compare instruction, data layout, and memory access alternatives to understand what actually improves performance.

## Technical Details
- **Data layout:** identical attributes per particle (position, velocity) stored sequentially to allow bulk cache loading.
- **SIMD:** physics calculations process 4 particles at a time using 4-byte float vectors.
- **Practical decisions:**
  - FPS is rendered into the simulation buffer to work around Android debugging limitations.
- **Observations & Lessons Learned:** some theoretically promising optimizations were less efficient in practice, highlighting the importance of measuring and validating every decision in the real environment.