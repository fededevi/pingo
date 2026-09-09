#include "array.h"
#include "state.h"

int array_init(Array *this, size_t count, void *data) {
  IF_NULL_RETURN(this, INIT_ERROR);

  this->count = count;

  if (count == 0) {
    data = 0;
    return OK;
  }

  IF_NULL_RETURN(data, INIT_ERROR);
  this->data = data;

  return OK;
}
