#pragma once

#include "benchmark_math.h"
#include "../example/linux_window/linux_window_backend.h"
#include "../render/renderer.h"
#include "../render/entity.h"
#include "../render/material.h"
#include "../render/mesh.h"
#include "../render/object.h"
#include "../render/texture.h"
#include "../assets/viking.h"

// Visual benchmark structure
typedef struct {
    // Rendering components
    LinuxWindowBackend backend;
    Renderer renderer;
    Entity root_entity;
    Object object;
    Material material;
    Texture texture;
    Pixel* texture_data;
    
    // Scene parameters
    float rotation_angle;
    Vec2i window_size;
    
    // Benchmark metrics
    int frame_count;
    double total_time;
    double fps;
    
    // Performance breakdown
    double avg_transform_time;
    double avg_render_time;
    double avg_present_time;
    
    // Timing
    BenchmarkTimer frame_timer;
    BenchmarkTimer component_timer;
} VisualBenchmark;

// Function declarations
void visual_benchmark_init(VisualBenchmark* vb, int width, int height);
void visual_benchmark_cleanup(VisualBenchmark* vb);
void visual_benchmark_run(VisualBenchmark* vb, double duration_seconds);
void visual_benchmark_update_scene(VisualBenchmark* vb);
void visual_benchmark_render_frame(VisualBenchmark* vb);
void visual_benchmark_display_stats(VisualBenchmark* vb);

// Texture loading (simplified version of the example)
Pixel* create_test_texture(Vec2i size);

// Main function
void run_visual_benchmark();