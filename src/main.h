#pragma once

#include <Arduino.h>
#include <SDFS.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include "DHT_Async.h"
#include "ui.h"

// _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
// Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0
#include "RPi_Pico_TimerInterrupt.h"

// Forward reference.
void verificarDHT22();
static bool measure_environment(float *, float *);
void zero_cross_detect();
bool dim_check(struct repeating_timer *t);
void playEndSong();
void playAlert();
void verificarSeguranca();
void secando();
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);

#define pinDHT22 6
#define pinBeeper 9
#define pin

// #define SDCARD_SCK_PIN 14
// #define SDCARD_MISO_PIN 15
// #define SDCARD_MOSI_PIN 12
#define SD_CS_PIN 13

int Potread;
static unsigned long measurement_timestamp = 0;
DHT_Async dht_sensor(pinDHT22, DHT_TYPE_22);

TFT_eSPI tft = TFT_eSPI();
static const uint16_t screenWidth = 240;
static const uint16_t screenHeight = 320;
uint16_t calData[5] = {412, 3388, 380, 3303, 4};
int mode = 1;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[screenWidth * screenHeight / 6];
static lv_color_t buf2[screenWidth * screenHeight / 6];
uint16_t touchX, touchY;

File myFile;
bool temCartao = false;

RPI_PICO_Timer ITimer1(1);        // Init RPI_PICO_Timer
volatile int i = 0;               // Variable to use as a counter volatile as it is in an interrupt
volatile boolean zero_cross = 0;  // Boolean to store a "switch" to tell us if we have crossed zero
int dim = 0;                      // Dimming level (0-128)  0 = on, 128 = 0ff
float freqStep = 65;              // This is the delay-per-brightness step in microseconds.
int inputDimmer = 4;              // Input from dimmer
int outputDimmer = 3;             // Output to Opto Triac

int arrayCompensacao[101] = {0,  4,  7,  11, 14, 18, 19, 20, 22, 23, 24, 25, 26, 27, 28, 29, 29, 30, 31, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 39, 39, 40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 47, 47, 48, 48, 49,
                             49, 50, 50, 51, 51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 56, 56, 57, 57, 58, 58, 59, 59, 60, 60, 61, 62, 62, 63, 63, 64, 65, 65, 66, 67, 68, 68, 69, 70, 70, 71, 72, 73, 75, 76, 77, 82, 86, 91, 95, 100};
char materiais[5][7] = {" PLA ", "PETG", "Silica", "Nylon", " ABS "};
int temperaturaMateriais[5] = {55, 65, 75, 75, 70};
int tempoMateriais[5] = {4, 4, 8, 4, 5};

int tipoMaterialAtual = 0;
char nomeMaterialAtual[7] = "PLA";
int temperaturaMaterialAtual = 55;
int tempoMaterialAtual = 4;
float tempAtual = 20, umidadeAtual = 40;
volatile int potenciaAtual = 0;
float tempAtual2, umidadeAtual2;
int estado = 1;  // 0=Escolha Material, 1=Rodando
