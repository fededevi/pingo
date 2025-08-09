#include "benchmark_visual.h"
#include "../render/pixel.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Create a simple test texture (procedural)
Pixel* create_test_texture(Vec2i size) {
    Pixel* image = malloc(size.x * size.y * sizeof(Pixel));
    if (!image) {
        printf("Error: Could not allocate texture memory\n");
        exit(-1);
    }
    
    // Create a simple procedural texture
    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            int index = y * size.x + x;
            
            // Create a checkerboard pattern with some variation
            int checker_size = 64;
            int checker_x = (x / checker_size) % 2;
            int checker_y = (y / checker_size) % 2;
            int checker = checker_x ^ checker_y;
            
            // Add some color variation
            unsigned char r = checker ? (x % 256) : 128;
            unsigned char g = checker ? 128 : (y % 256);
            unsigned char b = checker ? (y % 256) : (x % 256);
            unsigned char a = 255;
            
            image[index] = pixelFromRGBA(r, g, b, a);
        }
    }
    
    return image;
}

void visual_benchmark_init(VisualBenchmark* vb, int width, int height) {
    printf("Initializing visual benchmark (%dx%d)...\n", width, height);
    
    // Initialize benchmark data
    vb->window_size = (Vec2i){width, height};
    vb->rotation_angle = 0.0f;
    vb->frame_count = 0;
    vb->total_time = 0.0;
    vb->fps = 0.0;
    vb->avg_transform_time = 0.0;
    vb->avg_render_time = 0.0;
    vb->avg_present_time = 0.0;
    
    // Create test texture
    Vec2i texture_size = {256, 256}; // Smaller texture for better performance
    vb->texture_data = create_test_texture(texture_size);
    
    // Initialize texture
    texture_init(&vb->texture, texture_size, vb->texture_data);
    
    // Initialize material
    material_init(&vb->material, &vb->texture);
    
    // Initialize object with viking mesh
    object_init(&vb->object, &viking_mesh, &vb->material);
    
    // Initialize root entity
    entity_init(&vb->root_entity, (Renderable*)&vb->object, mat4Identity());
    
    // Initialize backend
    linuxWindowBackendInit(&vb->backend, vb->window_size);
    
    // Initialize renderer
    renderer_init(&vb->renderer, vb->window_size, (Backend*)&vb->backend);
    renderer_set_root_renderable(&vb->renderer, (Renderable*)&vb->root_entity);
    
    // Setup camera
    float aspect_ratio = (float)width / (float)height;
    vb->renderer.camera_projection = mat4Perspective(3.0f, 50.0f, aspect_ratio, 0.1f);
    vb->renderer.camera_view = mat4Translate((Vec3f){0, 0, 0});
    
    printf("Visual benchmark initialized successfully\n");
}

void visual_benchmark_cleanup(VisualBenchmark* vb) {
    if (vb->texture_data) {
        free(vb->texture_data);
        vb->texture_data = NULL;
    }
    
    printf("Visual benchmark cleanup completed\n");
}

void visual_benchmark_update_scene(VisualBenchmark* vb) {
    benchmark_start(&vb->component_timer);
    
    // Create rotation matrix
    Mat4 rotation = mat4RotateY(vb->rotation_angle);
    
    // Create translation matrix to move object back so it's visible
    Mat4 translation = mat4Translate((Vec3f){0, -7, -50});
    
    // Combine transforms
    Mat4 model = mat4MultiplyM(&rotation, &translation);
    
    // Update entity transform
    vb->root_entity.transform = model;
    
    // Update rotation for next frame
    vb->rotation_angle += 0.02f; // Slightly faster rotation for benchmark
    if (vb->rotation_angle > 2.0f * M_PI) {
        vb->rotation_angle -= 2.0f * M_PI;
    }
    
    double transform_time = benchmark_end(&vb->component_timer);
    vb->avg_transform_time = (vb->avg_transform_time * vb->frame_count + transform_time) / (vb->frame_count + 1);
}

void visual_benchmark_render_frame(VisualBenchmark* vb) {
    // Update scene transformations
    visual_benchmark_update_scene(vb);
    
    // Measure rendering time
    benchmark_start(&vb->component_timer);
    renderer_render(&vb->renderer);
    double render_time = benchmark_end(&vb->component_timer);
    
    vb->avg_render_time = (vb->avg_render_time * vb->frame_count + render_time) / (vb->frame_count + 1);
    
    // Update frame count
    vb->frame_count++;
    
    // Print progress every 60 frames
    if (vb->frame_count % 60 == 0) {
        printf("Frame %d - FPS: %.1f - Transform: %.4fms - Render: %.4fms\n",
               vb->frame_count, vb->fps, 
               vb->avg_transform_time * 1000.0, 
               vb->avg_render_time * 1000.0);
    }
}

void visual_benchmark_display_stats(VisualBenchmark* vb) {
    printf("\n=== Visual Rendering Benchmark Results ===\n");
    printf("Window Size: %dx%d\n", vb->window_size.x, vb->window_size.y);
    printf("Total Time: %.2f seconds\n", vb->total_time);
    printf("Total Frames: %d\n", vb->frame_count);
    printf("Average FPS: %.2f\n", vb->fps);
    printf("Average Frame Time: %.4f ms\n", (1.0 / vb->fps) * 1000.0);
    printf("\nPerformance Breakdown:\n");
    printf("  Transform Time: %.4f ms (%.1f%%)\n", 
           vb->avg_transform_time * 1000.0, 
           (vb->avg_transform_time / (1.0 / vb->fps)) * 100.0);
    printf("  Render Time: %.4f ms (%.1f%%)\n", 
           vb->avg_render_time * 1000.0, 
           (vb->avg_render_time / (1.0 / vb->fps)) * 100.0);
    printf("\nTriangles per Frame: %d\n", viking_mesh.indexes_count / 3);
    printf("Triangles per Second: %.0f\n", (viking_mesh.indexes_count / 3) * vb->fps);
    printf("Vertices per Second: %.0f\n", viking_mesh.indexes_count * vb->fps);
    printf("==========================================\n\n");
}

void visual_benchmark_run(VisualBenchmark* vb, double duration_seconds) {
    printf("Starting visual benchmark for %.1f seconds...\n", duration_seconds);
    printf("Press Ctrl+C to stop early if needed.\n\n");
    
    benchmark_start(&vb->frame_timer);
    double elapsed = 0.0;
    
    while (elapsed < duration_seconds) {
        // Render frame
        visual_benchmark_render_frame(vb);
        
        // Small delay to prevent overwhelming the system
        usleep(1000); // 1ms delay instead of 16ms for higher FPS
        
        // Update timing
        elapsed = benchmark_end(&vb->frame_timer);
        vb->fps = vb->frame_count / elapsed;
    }
    
    vb->total_time = elapsed;
    visual_benchmark_display_stats(vb);
}

void run_visual_benchmark() {
    printf("=== Pingo Visual Rendering Benchmark ===\n\n");
    
    VisualBenchmark vb;
    
    // Initialize with a reasonable window size
    visual_benchmark_init(&vb, 640, 480);
    
    // Run benchmark for 15 seconds
    visual_benchmark_run(&vb, 15.0);
    
    visual_benchmark_cleanup(&vb);
    
    printf("Visual benchmark completed!\n");
}
