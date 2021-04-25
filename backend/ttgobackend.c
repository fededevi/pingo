#include <string.h>

#include "ttgobackend.h"

#include "../render/renderer.h"
#include "../render/texture.h"
#include "../render/pixel.h"
#include "../render/depth.h"

#include "tft_espi/tft.h"

#include "freertos/task.h"

#define SPI_BUS TFT_VSPI_HOST
spi_lobo_device_handle_t spi;

uint16_t copyBuffer[2][DEFAULT_TFT_DISPLAY_HEIGHT][DEFAULT_TFT_DISPLAY_WIDTH];
uint32_t ping = 0;

uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t e = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return (e>>8) | (e<<8);
}

void _ttgoBackendInit( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {


}

void _ttgoBackendBeforeRender( Renderer * ren, BackEnd * backEnd) {
}

void _ttgoBackendAfterRender( Renderer * ren,  BackEnd * backEnd) {
    int xOff = 52;
    int yOff = 40;
    int xSize = 135;
    int ySize = 240;

    //WAIT FOR DMA FINISH IF THE PREVIOUS ONE STILL ISNT
    wait_trans_finish(1);
    spi_lobo_device_deselect(disp_spi);
    wait_trans_finish(1);
    spi_lobo_device_select(disp_spi, 0);
    wait_trans_finish(1);
    //STart DMA transfer of frame from current buffer
    send_data2(xOff, yOff, xSize+xOff-1, yOff+ySize, xSize*ySize-1, &copyBuffer[ping%2][0][0]);

    //Swap render buffer
    ping++;

    //Clear manually because the fb is double the size of the real one
    memset(copyBuffer[ping%2], 0, sizeof (copyBuffer[ping%2]));
    //printf("Frame: %d\n", ping);

}

Pixel * _ttgoBackendGetFrameBuffer( Renderer * ren,  BackEnd * backEnd) {
    return copyBuffer[ping%2];
}

Depth * _ttgoBackendGetZetaBuffer( Renderer * ren,  BackEnd * backEnd) {
    return ((TTGOBackend *) backEnd) -> zetaBuffer;
}

#ifndef WIN32
void texture_draw(Texture *f, Vec2i pos, Pixel color)
{
   copyBuffer[ping%2][pos.x][ pos.y ] = color565(color.g,color.g,color.g);
}
#endif


void ttgoBackendInit( TTGOBackend * this,  Vec2i size) {
    // Initialize backend callbacks
    this->backend.init = &_ttgoBackendInit;
    this->backend.beforeRender = &_ttgoBackendBeforeRender;
    this->backend.afterRender = &_ttgoBackendAfterRender;
    this->backend.getFrameBuffer = &_ttgoBackendGetFrameBuffer;
    this->backend.getZetaBuffer = &_ttgoBackendGetZetaBuffer;

    this -> zetaBuffer = malloc(size.x*size.y*sizeof (Depth));

    //Initialize spi-screen-dma communication
    tft_disp_type = DISP_TYPE_ST7789V;
    max_rdclock = 8000000;
    TFT_PinsInit();

    spi_lobo_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,				// set SPI MISO pin
        .mosi_io_num=PIN_NUM_MOSI,				// set SPI MOSI pin
        .sclk_io_num=PIN_NUM_CLK,				// set SPI CLK pin
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz = DEFAULT_TFT_DISPLAY_HEIGHT * DEFAULT_TFT_DISPLAY_WIDTH * 2 + 8,
    };

    spi_lobo_device_interface_config_t devcfg={
        .clock_speed_hz=8000000,                // Initial clock out at 8 MHz
        .mode=0,                                // SPI mode 0
        .spics_io_num=-1,                       // we will use external CS pin
        .spics_ext_io_num=PIN_NUM_CS,           // external CS pin
        .flags=LB_SPI_DEVICE_HALFDUPLEX,        // ALWAYS SET  to HALF DUPLEX MODE!! for display spi
    };

    vTaskDelay(500 / portTICK_RATE_MS);

    esp_err_t ret=spi_lobo_bus_add_device(SPI_BUS, &buscfg, &devcfg, &spi);
    assert(ret==ESP_OK);
    disp_spi = spi;

    ret = spi_lobo_device_select(spi, 1);
    assert(ret==ESP_OK);
    ret = spi_lobo_device_deselect(spi);
    assert(ret==ESP_OK);

    TFT_display_init();
    max_rdclock = find_rd_speed();
    printf( "Max SPI read clock speed: %dMHz\n", max_rdclock/1000000 );
    spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);
    TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
    TFT_setRotation(PORTRAIT_FLIP);
}
