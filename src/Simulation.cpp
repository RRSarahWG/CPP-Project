#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>
#include "ThreadManager.h"

const int WIDTH = 80;
const int HEIGHT = 24;
const int NUM_PARTICLES = 10;
const float TIME_STEP = 0.1f;
const float RADIUS = 1.0f;

std::mutex draw_mutex;

struct Particle {
    float x, y;
    float vx, vy;

    void update() {
        x += vx * TIME_STEP;
        y += vy * TIME_STEP;

        if (x < 0 || x >= WIDTH) vx *= -1;
        if (y < 0 || y >= HEIGHT) vy *= -1;

        x = std::clamp(x, 0.0f, static_cast<float>(WIDTH - 1));
        y = std::clamp(y, 0.0f, static_cast<float>(HEIGHT - 1));
    }
};

std::vector<Particle> particles;

void resolve_collision(Particle &a, Particle &b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    if (distance < 2 * RADIUS && distance > 0.0f) {
        std::swap(a.vx, b.vx);
        std::swap(a.vy, b.vy);
    }
}

void update_particles(int start, int end) {
    for (int i = start; i < end; ++i) {
        particles[i].update();
        for (int j = 0; j < particles.size(); ++j) {
            if (i != j) {
                resolve_collision(particles[i], particles[j]);
            }
        }
    }
}

void draw() {
    std::vector<std::string> screen(HEIGHT, std::string(WIDTH, ' '));
    std::lock_guard<std::mutex> lock(draw_mutex);

    for (const auto &p : particles) {
        int px = static_cast<int>(p.x);
        int py = static_cast<int>(p.y);
        if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
            screen[py][px] = '*';
        }
    }

    std::cout << "\x1B[2J\x1B[H"; // ANSI clear screen
    for (const auto &line : screen) {
        std::cout << line << '\n';
    }
}

int main() {
    ThreadManager threadManager(std::thread::hardware_concurrency());
    particles.resize(NUM_PARTICLES);

    for (auto &p : particles) {
        p.x = rand() % WIDTH;
        p.y = rand() % HEIGHT;
        p.vx = (rand() % 3 - 1) * 0.5f;
        p.vy = (rand() % 3 - 1) * 0.5f;
    }

    threadManager.start();

    while (true) {
        int numThreads = threadManager.getNumThreads();
        int chunk = NUM_PARTICLES / numThreads;

        for (int i = 0; i < numThreads; ++i) {
            int start = i * chunk;
            int end = (i == numThreads - 1) ? NUM_PARTICLES : start + chunk;
            threadManager.addTask([start, end]() { update_particles(start, end); });
        }

        threadManager.waitForCompletion();
        draw();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    threadManager.stop();
    return 0;
}
