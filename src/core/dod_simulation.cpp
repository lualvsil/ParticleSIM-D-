#include "simulation.hpp"
#include "core/random.hpp"

#include <chrono>
#include <cmath>
#include <arm_neon.h>

// Execute on thread
static void _update_simd(void* data, int begin, int end) {
    auto sim = static_cast<SimulationJobData*>(data);
    auto dt = sim->dt;
    
    constexpr float G = 10.0f;
    constexpr float particleMass = 50.0f;
    constexpr float minNonZero = 0.0001f;
    
    // F = [G * 20] / r2
    float32x4_t massTimesGV = vdupq_n_f32(particleMass*G);
    
    for (int i = begin; i < end; i++) {
        float xi = sim->x[i];
        float yi = sim->y[i];
        
        float32x4_t forceAccumulatorX = vdupq_n_f32(0.0f);
        float32x4_t forceAccumulatorY = vdupq_n_f32(0.0f);
        
        float32x4_t xiV = vdupq_n_f32(xi);
        float32x4_t yiV = vdupq_n_f32(yi);
        
        for (int j = 0; j < sim->length; j+=4) {
            // Load the next 4 particles into vector register
            float32x4_t xV = vld1q_f32(&sim->x[j]);
            float32x4_t yV = vld1q_f32(&sim->y[j]);
            
            // d = xj - xi
            float32x4_t dxV = vsubq_f32(xV, xiV);
            float32x4_t dyV = vsubq_f32(yV, yiV);
            
            // d2 = d*d
            float32x4_t dx2V = vmulq_f32(dxV, dxV);
            float32x4_t dy2V = vmulq_f32(dyV, dyV);
            
            // r2 -> total distance
            float32x4_t r2V0 = vaddq_f32(dx2V, dy2V);
            float32x4_t r2V = vaddq_f32(r2V0, vdupq_n_f32(minNonZero));
            
            // Approximate invR = 1 / sqrt(r2)
            float32x4_t invrV_ = vrsqrteq_f32(r2V);
            // Newthon-Rapshon algorithm
            float32x4_t step = vrsqrtsq_f32(r2V, vmulq_f32(invrV_, invrV_));
            float32x4_t invrV = vmulq_f32(invrV_, step);
            
            // F = [(G*mass) / r2]
            float32x4_t forceV = vdivq_f32(massTimesGV, r2V);
            
            // finv = F * invR
            float32x4_t finvV_ = vmulq_f32(forceV, invrV);
            float32x4_t finvV = vmulq_n_f32(finvV_, dt); // multiply by delta time
            
            // F = F * d
            float32x4_t fxV = vmulq_f32(finvV, dxV);
            float32x4_t fyV = vmulq_f32(finvV, dyV);
            
            float32x4_t vxV = vld1q_f32(&sim->vx[j]);
            float32x4_t vyV = vld1q_f32(&sim->vy[j]);
            
            // Accumulate force in i for each interaction
            forceAccumulatorX = vaddq_f32(fxV, forceAccumulatorX);
            forceAccumulatorY = vaddq_f32(fyV, forceAccumulatorY);
        }
        
        float scalarForceX = vaddvq_f32(forceAccumulatorX);
        float scalarForceY = vaddvq_f32(forceAccumulatorY);
        
        sim->vx[i] += scalarForceX;
        sim->vy[i] += scalarForceY;
    }
}

Simulation::Simulation(int bodyCount, int width, int height)
    : x(bodyCount), y(bodyCount), bodyCount(bodyCount),
      vx(bodyCount), vy(bodyCount) {
    
    init_random();
    
    for (int i=0; i<bodyCount; i++) {
        float rx = genRandom01();
        float ry = genRandom01();
        
        x[i] = rx * width;
        y[i] = ry * height;
    }
    
    data = new SimulationJobData {
        .x=&x[0],
        .y=&y[0],
        .length=bodyCount,
        .vx=&vx[0],
        .vy=&vy[0],
    };
    
    jobSystem = new JobSystem(8);
    jobSystem->set_executor(_update_simd);
    
    int particlesPerJob = bodyCount / 256;
    jobSystem->set_data(data, bodyCount, particlesPerJob);
}

Simulation::~Simulation() {
    delete jobSystem;
    delete data;
}

void Simulation::update(float dt) {
    data->dt = dt;
    
    jobSystem->dispatch();
    jobSystem->wait();
    
    for (int i = 0; i < bodyCount; i++) {
        x[i] += vx[i] * dt;
        y[i] += vy[i] * dt;
    }
}

std::span<const float> Simulation::getX() { return this->x; }
std::span<const float> Simulation::getY() { return this->y; }