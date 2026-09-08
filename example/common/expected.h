#pragma once

/**
 * Error handling for the example programs.
 *
 * C has no templates, so a value-carrying expected<T, E> cannot be written
 * generically without either losing type safety (type erasure) or generating
 * one struct per T. Fallible functions here instead return PgError and hand
 * their result back through an out-parameter, which is generic over any T,
 * fully type-checked by the compiler, and valid C99.
 *
 * PgError is a plain value: every string it carries is a literal, so there is
 * nothing to own and nothing to free.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef enum PgStatus {
  PG_OK = 0,
  PG_INVALID_ARGUMENT,
  PG_OUT_OF_MEMORY,
  PG_NOT_FOUND,
  PG_PERMISSION_DENIED,
  PG_IO,
  PG_UNSUPPORTED,
  PG_BACKEND,
} PgStatus;

typedef struct PgError {
  PgStatus status;
  const char *context; // what was attempted, e.g. "open /dev/fb0"
  const char *hint;    // how the user might fix it, or NULL
  int errnum;          // errno at the point of failure, or 0
} PgError;

#define PG_SUCCESS ((PgError){PG_OK, NULL, NULL, 0})

static inline PgError pg_fail(PgStatus status, const char *context) {
  return (PgError){status, context, NULL, 0};
}

static inline PgError pg_fail_hint(PgStatus status, const char *context,
                                   const char *hint) {
  return (PgError){status, context, hint, 0};
}

// Captures errno, so call it before anything else can overwrite it.
static inline PgError pg_fail_errno(PgStatus status, const char *context) {
  return (PgError){status, context, NULL, errno};
}

static inline bool pg_failed(PgError error) { return error.status != PG_OK; }

static inline const char *pg_status_str(PgStatus status) {
  switch (status) {
  case PG_OK:
    return "ok";
  case PG_INVALID_ARGUMENT:
    return "invalid argument";
  case PG_OUT_OF_MEMORY:
    return "out of memory";
  case PG_NOT_FOUND:
    return "not found";
  case PG_PERMISSION_DENIED:
    return "permission denied";
  case PG_IO:
    return "I/O error";
  case PG_UNSUPPORTED:
    return "unsupported";
  case PG_BACKEND:
    return "backend failure";
  }
  return "unrecognized status";
}

// The single place that decides what a failure looks like on screen.
static inline void pg_error_report(PgError error, FILE *out) {
  if (error.status == PG_OK) {
    return;
  }
  fprintf(out, "error: %s: %s", error.context ? error.context : "(no context)",
          pg_status_str(error.status));
  if (error.errnum != 0) {
    fprintf(out, " (%s)", strerror(error.errnum));
  }
  fputc('\n', out);
  if (error.hint != NULL) {
    fprintf(out, "  hint: %s\n", error.hint);
  }
}

/**
 * Evaluates expr and, if it failed, returns that error to the caller
 * immediately - the C equivalent of Rust's `?`.
 *
 * Two things to know: it hides a return, and it does not clean up. A function
 * that has already acquired a resource must not use it; write that one with an
 * explicit `goto cleanup` instead. It also only compiles inside a function
 * returning PgError, so main() checks explicitly and calls pg_error_report.
 */
#define RETURN_IF_ERROR(expr)                                                  \
  do {                                                                         \
    PgError pg_error_ = (expr);                                                \
    if (pg_error_.status != PG_OK) {                                           \
      return pg_error_;                                                        \
    }                                                                          \
  } while (0)
