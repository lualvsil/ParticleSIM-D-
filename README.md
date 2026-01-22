# ParticleSIM(D)

## Description
ParticleSIM(D) is a particle simulation project developed as a laboratory
for experimenting with data-oriented design and low-level performance
optimizations on constrained hardware, specifically Android mobile devices.

The focus is not on realistic physics or advanced rendering, but on
exploring how data layout, instruction selection, and memory access
patterns directly impact CPU performance in real-world environments.

---

## Project Goals
- Experiment with Data-Oriented Design (DOD) to improve cache efficiency and memory access patterns.
- Apply SIMD (ARM NEON / AArch64) to process multiple particles in parallel.
- Evaluate performance trade-offs on Android, considering OS behavior and mobile CPU constraints.
- Measure and compare different instruction choices, data layouts, and algorithm variants to validate which optimizations actually improve performance.
- Explore job-based multithreading to scale physics updates across CPU cores.

---

## Technical Details
- **Target platform:** Android ARM64 (AArch64).  
  32-bit ARM and non-ARM platforms are intentionally not supported.
- **Data layout:** particle attributes (position, velocity) are stored in contiguous arrays to maximize cache locality.
- **SIMD:** physics calculations operate on vectors of 4 single-precision floats using ARM NEON intrinsics.
- **Integration model:** particle interactions are computed using a CPU-bound N-body–style loop, optimized for instruction-level and data-level parallelism.
- **Practical constraints:**
  - FPS and debug information are written directly into the simulation buffer to avoid Android logging and debugging overhead.
- **Multithreading:** physics updates are distributed across a fixed worker thread pool using a job-based execution model.

---

## Design Decisions

- **No GPU acceleration:** The simulation intentionally runs entirely on the CPU.
  The goal is to study data-oriented design, SIMD usage, cache behavior, and threading strategies on ARM CPUs.
  Using the GPU would shift the performance bottleneck and invalidate the purpose of the experiment.
- **CPU parallelism:** a custom job system is used instead of external frameworks to retain control over
  scheduling and synchronization costs.

---

## Observations & Lessons Learned
Some optimizations that appear beneficial in theory proved ineffective or counterproductive in practice.
This project emphasizes the importance of profiling, benchmarking, and validating every optimization
on the actual target hardware rather than relying solely on assumptions.