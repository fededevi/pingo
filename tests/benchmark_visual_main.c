#include "benchmark_visual.h"
#include <stdio.h>

int main() {
    printf("=== Pingo Visual Performance Benchmark ===\n\n");
    printf("This benchmark will open an X11 window and render the Viking model\n");
    printf("while measuring real-time performance metrics.\n\n");
    printf("Press Ctrl+C to stop the benchmark early if needed.\n\n");
    
    printf("Starting visual rendering benchmark...\n");
    run_visual_benchmark();
    
    printf("\n🎬 Visual benchmark completed!\n");
    return 0;
}
