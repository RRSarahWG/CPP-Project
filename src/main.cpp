#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include <windows.h>

// Constants for simulation
const int WIDTH = 60;
const int HEIGHT = 20;
const int NUM_PARTICLES = 10;
const int FRAME_DELAY = 100; // milliseconds

std::mutex draw_mutex;

// Particle structure
struct Particle {
    float x, y;
    float vx, vy;
    char symbol;

    Particle(float x_, float y_, float vx_, float vy_, char sym = 'o')
        : x(x_), y(y_), vx(vx_), vy(vy_), symbol(sym) {}

    void update() {
        x += vx;
        y += vy;

        // Wall collision
        if (x < 0 || x >= WIDTH) vx *= -1;
        if (y < 0 || y >= HEIGHT) vy *= -1;

        // Keep in bounds
        x = std::max(0.0f, std::min(x, float(WIDTH - 1)));
        y = std::max(0.0f, std::min(y, float(HEIGHT - 1)));
    }
};

// Check and resolve collision between two particles
void resolve_collision(Particle& a, Particle& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 1.0f) {  // particles "touch"
        std::swap(a.vx, b.vx);
        std::swap(a.vy, b.vy);
    }
}

// Thread function to update a range of particles
void update_particles(std::vector<Particle>& particles, int start, int end) {
    while (true) {
        for (int i = start; i < end; ++i) {
            particles[i].update();
            // Check collision with other particles
            for (int j = 0; j < particles.size(); ++j) {
                if (i != j) {
                    resolve_collision(particles[i], particles[j]);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_DELAY));
    }
}

// Function to draw the particle grid
void draw(const std::vector<Particle>& particles) {
    std::vector<std::string> screen(HEIGHT, std::string(WIDTH, ' '));

    for (const auto& p : particles) {
        int x = int(p.x);
        int y = int(p.y);
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            screen[y][x] = p.symbol;
        }
    }

    std::lock_guard<std::mutex> lock(draw_mutex);
    system("cls");
    for (const auto& line : screen) {
        std::cout << line << '\n';
    }
}

int main() {
    // Initialize particles
    std::vector<Particle> particles;
    for (int i = 0; i < NUM_PARTICLES; ++i) {
        float x = rand() % WIDTH;
        float y = rand() % HEIGHT;
        float vx = (rand() % 3 - 1) * 0.5f;
        float vy = (rand() % 3 - 1) * 0.5f;
        particles.emplace_back(x, y, vx, vy);
    }

    // Launch threads
    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunk_size = NUM_PARTICLES / num_threads;
    for (int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1) ? NUM_PARTICLES : start + chunk_size;
        threads.emplace_back(update_particles, std::ref(particles), start, end);
    }

    // Main render loop
    while (true) {
        draw(particles);
        std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_DELAY));
    }

    // Join threads (never reached in this loop, but safe practice)
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    return 0;
}
