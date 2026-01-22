#pragma once

#include <span>
#include <vector>
#include <memory>
#include "core/jobsystem.hpp"

#define NUM_PARTICLES 3400

struct SimulationJobData {
    float* x;
    float* y;
    
    int length;
    float* vx;
    float* vy;
    float dt;
};

class Simulation {
private:
    int bodyCount;
    
    // Particles position
    std::vector<float> x;
    std::vector<float> y;
    
    std::vector<float> vx;
    std::vector<float> vy;
    
    JobSystem* jobSystem;
    SimulationJobData* data;
    
public:
    Simulation(int bodyCount, int width, int height);
    ~Simulation();
    void update(float dt);
    std::span<const float> getX();
    std::span<const float> getY();
};
