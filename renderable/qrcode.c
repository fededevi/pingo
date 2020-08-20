#include "qrcode.h"

#include "../qrCodeGen/c/qrcodegen.h"

Renderable qrCodeAsRenderable(QrCode * qrc) {
    return (Renderable){ .renderableType = RENDERABLE_QRCODE, .impl = qrc};
}


int qrCodeInit(QrCode *s, Mat3 t, Vector2I size, Pixel * buf, uint8_t * tempBuf, uint8_t qrCodeLevel, char * dataString)
{
    int e;
    e = frameInit(&s->sprite.frame, size,buf);
    if (e) return 1; //Failed to init frame

    e = spriteInit(&s->sprite, s->sprite.frame, t);
    if (e) return 2; //Failed to init sprite

    int offset = qrcodegen_BUFFER_LEN_FOR_VERSION(qrCodeLevel);

    e = qrcodegen_encodeText(dataString, tempBuf+offset, tempBuf, qrcodegen_Ecc_LOW, qrCodeLevel, qrCodeLevel, qrcodegen_Mask_AUTO, true);

    if (e != 1) return 3; //Failed to create qrCode

    int qrCodeEdgeSize = qrcodegen_getSize(tempBuf);

    if (size.x != qrCodeEdgeSize || size.y != qrCodeEdgeSize) return 4; //Wrong size framebuffer for this level of qrcode

    //Copy qrCode data to frame
    for (uint16_t y = 0; y < qrCodeEdgeSize; y++) {
        for (uint16_t x = 0; x < qrCodeEdgeSize; x++) {
            Vector2I pos = {x,y};
            if (qrcodegen_getModule(tempBuf, x, y)) {
                frameDraw(&s->sprite.frame,pos,PIXELBLACK);
            } else {
                frameDraw(&s->sprite.frame,pos,PIXELWHITE);
            }
        }
    }

    return 0;
}

int qrCodeSizeForLevel(uint8_t qrCodeLevel)
{
    return 2 * qrcodegen_BUFFER_LEN_FOR_VERSION(qrCodeLevel);
}
