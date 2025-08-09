#include "benchmark_math.h"
#include "../example/linux_window/linux_window_backend.h"
#include "../render/renderer.h"
#include "../render/entity.h"
#include "../render/material.h"
#include "../render/object.h"
#include "../render/texture.h"
#include "../assets/viking.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MAX_OBJECTS 25

// Test the actual library with optimizations
typedef struct {
    LinuxWindowBackend backend;
    Renderer renderer;
    Entity root_entity;
    
    // Multiple objects for realistic culling test
    Object objects[MAX_OBJECTS];
    Entity entities[MAX_OBJECTS];
    Material materials[MAX_OBJECTS];
    Texture textures[MAX_OBJECTS];
    Pixel* texture_data[MAX_OBJECTS];
    
    // Object positions and rotations
    Vec3f positions[MAX_OBJECTS];
    Vec3f rotations[MAX_OBJECTS];
    float rotation_speeds[MAX_OBJECTS];
    
    float global_time;
    Vec2i window_size;
    
    int frame_count;
    double total_time;
    BenchmarkTimer frame_timer;
} LibraryBenchmark;

// Create test texture
Pixel* create_test_texture(Vec2i size) {
    Pixel* image = malloc(size.x * size.y * sizeof(Pixel));
    if (!image) return NULL;
    
    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            int index = y * size.x + x;
            unsigned char intensity = ((x ^ y) & 0x10) ? 255 : 128;
            image[index] = pixelFromRGBA(intensity, intensity, intensity, 255);
        }
    }
    return image;
}

void library_benchmark_init(LibraryBenchmark* lb, int width, int height) {
    printf("Initializing multi-object library benchmark (%dx%d)...\n", width, height);
    
    memset(lb, 0, sizeof(LibraryBenchmark));
    lb->window_size = (Vec2i){width, height};
    
    // Initialize backend and renderer first
    linuxWindowBackendInit(&lb->backend, lb->window_size);
    renderer_init(&lb->renderer, lb->window_size, (Backend*)&lb->backend);
    
    // Setup camera
    float aspect_ratio = (float)width / (float)height;
    lb->renderer.camera_projection = mat4Perspective(3.0f, 50.0f, aspect_ratio, 0.1f);
    lb->renderer.camera_view = mat4Translate((Vec3f){0, 0, 0});
    
    // Create multiple objects distributed in 3D space
    Vec2i texture_size = {128, 128}; // Smaller textures for performance
    
    for (int i = 0; i < MAX_OBJECTS; i++) {
        // Create unique texture for each object
        lb->texture_data[i] = create_test_texture(texture_size);
        texture_init(&lb->textures[i], texture_size, lb->texture_data[i]);
        material_init(&lb->materials[i], &lb->textures[i]);
        object_init(&lb->objects[i], &viking_mesh, &lb->materials[i]);
        
        // Distribute objects in a 5x5 grid with depth variation
        int row = i / 5;
        int col = i % 5;
        
        // Position objects in a grid from -40 to +40 on X and Z axes
        float x = (col - 2) * 20.0f + ((float)rand() / RAND_MAX - 0.5f) * 10.0f;
        float y = -7.0f + ((float)rand() / RAND_MAX - 0.5f) * 5.0f;
        float z = -30.0f - row * 20.0f + ((float)rand() / RAND_MAX - 0.5f) * 15.0f;
        
        lb->positions[i] = (Vec3f){x, y, z};
        
        // Random rotation and rotation speed
        lb->rotations[i] = (Vec3f){
            ((float)rand() / RAND_MAX) * 2.0f * M_PI,
            ((float)rand() / RAND_MAX) * 2.0f * M_PI,
            ((float)rand() / RAND_MAX) * 2.0f * M_PI
        };
        
        lb->rotation_speeds[i] = ((float)rand() / RAND_MAX) * 2.0f + 0.5f;
        
        // Set initial transform
        Mat4 translation = mat4Translate(lb->positions[i]);
        Mat4 rotation = mat4RotateY(lb->rotations[i].y);
        Mat4 transform = mat4MultiplyM(&rotation, &translation);
        
        entity_init(&lb->entities[i], (Renderable*)&lb->objects[i], transform);
    }
    
    // Create a simple scene graph - for now, we'll render objects individually
    // In a real implementation, you'd create a proper scene graph
    entity_init(&lb->root_entity, NULL, mat4Identity());
    renderer_set_root_renderable(&lb->renderer, (Renderable*)&lb->root_entity);
    
    printf("Library benchmark initialized with %d objects\n", MAX_OBJECTS);
    printf("Objects distributed in 3D space for realistic culling test\n");
}

void library_benchmark_cleanup(LibraryBenchmark* lb) {
    for (int i = 0; i < MAX_OBJECTS; i++) {
        if (lb->texture_data[i]) {
            free(lb->texture_data[i]);
            lb->texture_data[i] = NULL;
        }
    }
}

double library_benchmark_run_with_config(LibraryBenchmark* lb, double duration, 
                                         bool backface, bool frustum, bool early_z, 
                                         const char* config_name) {
    printf("Testing %s...\n", config_name);
    
    // Configure optimizations
    renderer_enable_backface_culling(&lb->renderer, backface);
    renderer_enable_frustum_culling(&lb->renderer, frustum);
    renderer_enable_early_z_test(&lb->renderer, early_z);
    
    // Reset counters
    lb->frame_count = 0;
    lb->global_time = 0;
    
    benchmark_start(&lb->frame_timer);
    double elapsed = 0.0;
    
    while (elapsed < duration) {
        // Update global time
        lb->global_time += 0.016f; // ~60 FPS time step
        
        // Update each object's transform
        for (int i = 0; i < MAX_OBJECTS; i++) {
            // Update rotation
            lb->rotations[i].y += lb->rotation_speeds[i] * 0.016f;
            
            // Create transform matrix
            Mat4 translation = mat4Translate(lb->positions[i]);
            Mat4 rotation = mat4RotateY(lb->rotations[i].y);
            Mat4 scale = mat4Scale((Vec3f){0.8f, 0.8f, 0.8f}); // Scale down objects
            
            Mat4 temp = mat4MultiplyM(&rotation, &scale);
            Mat4 transform = mat4MultiplyM(&temp, &translation);
            
            lb->entities[i].transform = transform;
            
            // Render each object individually (simulating scene graph traversal)
            lb->renderer.root_renderable = (Renderable*)&lb->entities[i];
            renderer_render(&lb->renderer);
        }
        
        // Present the frame
        lb->backend.backend.afterRender(&lb->renderer, &lb->backend.backend);
        
        lb->frame_count++;
        elapsed = benchmark_end(&lb->frame_timer);
        
        if (lb->frame_count % 30 == 0) {
            printf("  Frame %d - FPS: %.1f (rendering %d objects)\n", 
                   lb->frame_count, lb->frame_count / elapsed, MAX_OBJECTS);
        }
    }
    
    lb->total_time = elapsed;
    double fps = lb->frame_count / elapsed;
    
    printf("  %s: %.2f FPS (%d frames in %.2fs)\n", 
           config_name, fps, lb->frame_count, lb->total_time);
    
    return fps;
}

void library_benchmark_compare_optimizations(LibraryBenchmark* lb) {
    printf("\n=== Library Optimization Comparison ===\n");
    
    const double test_duration = 3.0;
    
    // Test different optimization combinations
    double fps_none = library_benchmark_run_with_config(lb, test_duration, 
                                                       false, false, false, 
                                                       "No Optimizations");
    
    double fps_backface = library_benchmark_run_with_config(lb, test_duration, 
                                                           true, false, false, 
                                                           "Backface Culling Only");
    
    double fps_early_z = library_benchmark_run_with_config(lb, test_duration, 
                                                          false, false, true, 
                                                          "Early Z-Test Only");
    
    double fps_frustum = library_benchmark_run_with_config(lb, test_duration, 
                                                          false, true, false, 
                                                          "Frustum Culling Only");
    
    double fps_all = library_benchmark_run_with_config(lb, test_duration, 
                                                      true, true, true, 
                                                      "All Optimizations");
    
    double fps_best = library_benchmark_run_with_config(lb, test_duration, 
                                                       true, false, true, 
                                                       "Best Combination (Backface + Early Z)");
    
    // Results summary
    printf("\n=== Library Performance Results ===\n");
    printf("No Optimizations:       %.2f FPS (baseline)\n", fps_none);
    printf("Backface Culling:       %.2f FPS (+%.1f%%)\n", fps_backface, 
           ((fps_backface - fps_none) / fps_none) * 100.0);
    printf("Early Z-Test:           %.2f FPS (+%.1f%%)\n", fps_early_z, 
           ((fps_early_z - fps_none) / fps_none) * 100.0);
    printf("Frustum Culling:        %.2f FPS (+%.1f%%)\n", fps_frustum, 
           ((fps_frustum - fps_none) / fps_none) * 100.0);
    printf("All Optimizations:      %.2f FPS (+%.1f%%)\n", fps_all, 
           ((fps_all - fps_none) / fps_none) * 100.0);
    printf("Best Combination:       %.2f FPS (+%.1f%%)\n", fps_best, 
           ((fps_best - fps_none) / fps_none) * 100.0);
    printf("=====================================\n");
}

void run_library_optimization_benchmark() {
    printf("=== Pingo Multi-Object Library Optimization Benchmark ===\n\n");
    printf("Testing the actual library with integrated optimizations\n");
    printf("Rendering %d Viking objects distributed in 3D space\n", MAX_OBJECTS);
    printf("This creates realistic conditions for culling optimizations\n\n");
    
    LibraryBenchmark lb;
    library_benchmark_init(&lb, 640, 480);
    
    // Compare optimization configurations
    library_benchmark_compare_optimizations(&lb);
    
    library_benchmark_cleanup(&lb);
    
    printf("\nMulti-object library optimization benchmark completed!\n");
}

int main() {
    printf("=== Pingo Multi-Object Library Performance Test ===\n\n");
    printf("This benchmark tests the actual Pingo library\n");
    printf("with integrated rendering optimizations using multiple objects.\n");
    printf("Objects are distributed in 3D space to create realistic\n");
    printf("conditions where culling optimizations can demonstrate their value.\n\n");
    
    run_library_optimization_benchmark();
    
    printf("\n✅ Multi-object library optimization test completed!\n");
    return 0;
}
