#pragma once

#include "array.h"
#include "state.h"

int array_init(Array *this, size_t size, void *data)
{
    IF_NULL_RETURN(this, INIT_ERROR);

    this->size = size;

    if (size == 0) {
        data = 0;
        return OK;
    }

    IF_NULL_RETURN(data, INIT_ERROR);
    this->data = data;

    return OK;
}
