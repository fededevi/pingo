#pragma once

enum State {
  OK,           // Ok state
  INIT_ERROR,   // Error during an init call
  RENDER_ERROR, // Error during a render call
  SET_ERROR     // Error during a set call
};

#define IF_NULL_RETURN(ptr, retval)                                            \
  if ((ptr) == 0)                                                              \
    return retval;
