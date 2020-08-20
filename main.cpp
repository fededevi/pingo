#include <QCoreApplication>

#include <Windows.h>
#include <wingdi.h>
#include <gdiplus.h>

#include "render/renderer.h"
#include "renderable/sprite.h"
#include "renderable/qrcode.h"

#define HEIGHT 400
#define WIDTH 400



int main()
{
    //Renderer contains 2 frame buffers for switching and handles the drawing of Renderables in the current Scene
    Vector2I frameSize = {WIDTH,HEIGHT};
    Renderer r;
    rendererInit(&r, frameSize, fb0[0], fb1[0]);



    Pixel spriteFrameBuffer[100][100]; //Sprite graphics memory
    Frame f;
    frameInit(&f, (Vector2I){100,100}, spriteFrameBuffer[0]); //Init Frame for Sprite
    //Create the sprite
    Sprite sprite;
    spriteInit(&sprite,f, transformTranslate((Vec2f){50,50})); //Sprite position is 50 50
    spriteRandomize(&sprite); //Draw random stuff on sprite






    //Build QrCode Renderable
    QrCode qr;
    Pixel qrCodeFrameBuffer[41][41];
    uint8_t qrCodeLevel = 6;
    uint8_t tempBufferSize = qrCodeSizeForLevel(qrCodeLevel);
    uint8_t qrCodeTempBuffer[tempBufferSize];
    char qrCodeDataString[] = "Pingo!";
    //transformRotate(0.25)

    Mat3 m1 = transformTranslate((Vec2f){-20,-20}); //Translate to center
    Mat3 m2 = transformScale((Vec2f){2.5,2});         //Double size
    Mat3 m3 = transformMultiplyM(&m1, &m2); //first translate then scale
    m2 = transformRotate(3.14 * 0.15); //rotate about 45 degrees
    m1 = transformMultiplyM(&m3, &m2);
    m3 = transformTranslate((Vec2f){200,200});
    m2 = transformMultiplyM(&m1, &m3);

    qrCodeInit(&qr,m2,(Vector2I){41,41}, qrCodeFrameBuffer[0], qrCodeTempBuffer, qrCodeLevel, qrCodeDataString );



    Scene s; //Scene Contains a list of rRenderables
    sceneInit(&s);
    rendererSetScene(&r, &s); //Assign Scene to Renderer

    sceneAddRenderable(&s, spriteAsRenderable(&sprite)); //Add sprite to the scene
    sceneAddRenderable(&s, qrCodeAsRenderable(&qr)); //Add qrCode


    //Copy buffer to window
    HWND window = winMain(nullptr,nullptr,nullptr, SW_NORMAL );
    windowsHDC = GetDC(window);
    prepareBitMap();
    while (true ) {

        rendererRender(&r); //actually render scene to draw buffer
        rendererSwap(&r); //swap draw buffer and read buffer
        Frame currentBuffer = rendererCurrentBuffer(&r);

        for (uint16_t y = 0; y < currentBuffer.size.y; y++ ) {
            for (uint16_t x = 0; x < currentBuffer.size.x; x++ ) {
                Pixel p = frameRead(&currentBuffer,(Vector2I){x,y}); //Read data from draw buffer
                SetPixel(bitmapHDC,x,y, RGB(p.r,p.g,p.b) );
            }
        }

        writeBitmap();

    }

}
