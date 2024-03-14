//
// Created by root on 2022/11/13.
//


#include <Arduino.h>
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "Audio.h"
#include <Ticker.h>

#define SD_CS         2
#define SPI_MOSI      11
#define SPI_MISO      13
#define SPI_SCK       12


#define I2S_DOUT      -1
#define I2S_BCLK      -1
#define I2S_LRC       -1

extern Audio audio;
// extern SPIClass spi;

void sdcard_init();
