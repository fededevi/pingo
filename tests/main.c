#include "test_math.h"

int main() {
    printf("=== Pingo Math Library Test Suite ===\n\n");
    
    int success = run_all_tests();
    
    if (success) {
        printf("\n🎉 All tests passed!\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed!\n");
        return 1;
    }
}
