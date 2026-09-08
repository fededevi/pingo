#pragma once

/**
 * The contract every example backend implements, declared in one place so the
 * compiler checks the backends against what main.c calls. These used to be
 * bare `extern` declarations inside main.c, which meant a signature could
 * drift from its definition without anything noticing.
 */

#include "expected.h"

#include "math/vec2.h"
#include "render/fwd.h"

/**
 * Creates and initializes the backend, storing it in *out on success.
 * *out is only written when PG_OK is returned.
 */
PgError create_backend(Vec2i size, Backend **out);

/** Releases everything create_backend acquired. Accepts NULL. */
void destroy_backend(Backend *backend);

void backend_sleep(int microseconds);
