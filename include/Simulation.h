#pragma once
#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include "ContainmentField.h"
#include "ThreadManager.h"
#include "Particle.h"

class Simulation {
public:
    Simulation(const Config& config);
    ~Simulation();
    
    void start();
    void stop();
    void step();
    void addParticle(std::unique_ptr<Particle> particle);
    size_t getParticleCount() const;
    const std::vector<std::unique_ptr<Particle>>& getParticles() const;
    double getTotalEnergy() const;
    void setNumThreads(size_t newNumThreads);
    size_t getNumThreads() const;
    
private:
    void initializeParticles(const Config& config);
    void removeEscapedParticles();
    void applyForces(double dt);
    void updatePositions(double dt);
    void handleCollisions();
    void workerThread(size_t threadId);
    
    size_t numThreads;
    double fieldSize;
    double timeStep;
    
    std::atomic<bool> running{false};
    std::vector<std::unique_ptr<Particle>> particles;
    std::unique_ptr<ContainmentField> containmentField;
    std::unique_ptr<ThreadManager> threadManager;
    std::vector<std::thread> workerThreads;
};
