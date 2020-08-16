#pragma once

#include "sprite.h"

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct  {
   Sprite sprite;
} QrCode;

extern int qrCodeSizeForLevel(uint8_t qrCodeLevel);
extern int qrCodeInit( QrCode * s, Mat3 t, Vector2I size, Pixel * buf, uint8_t * tempBuf, uint8_t qrCodeLevel, char * dataString);
extern Renderable qrCodeAsRenderable( QrCode * s);

#ifdef __cplusplus
    }
#endif
