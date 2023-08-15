#include "math/vec2.h"
#include "render/backend.h"

typedef struct JpegBackend {
  Backend backend;
  char *jpegFilename;
} JpegBackend;

extern int jpeg_backend_init(JpegBackend *this, Vec2i size,
                             const char *filename);
