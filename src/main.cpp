#include "main.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pinBeeper, OUTPUT);

  SDFSConfig c2;
  c2.setCSPin(SD_CS_PIN);
  c2.setSPI(SPI1);
  SDFS.setConfig(c2);

  delay(100);  // while (!Serial) { delay(10);}
  Serial1.begin(921600);
  Serial1.println(F("Comecando..."));

  pinMode(outputDimmer, OUTPUT);                            // Set the Triac pin as output
  attachInterrupt(inputDimmer, zero_cross_detect, RISING);  // Attach an Interupt to Pin 2 (interupt 0) for Zero Cross Detection
  ITimer1.attachInterruptInterval(freqStep, dim_check);

  tft.begin();
  tft.setRotation(0);
  tft.initDMA();
  // calibrate_touch();
  tft.setTouch(calData);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf1, buf2, screenWidth * screenHeight / 6);

  /* Initialize the display */
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /* Change the following line to your display resolution */
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /* Initialize the (dummy) input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  //   if (!SDFS.begin()) {  // Monta
  //     Serial1.println(F("SD failed!"));
  //   }
  //   else {
  //     Serial1.println(F("SD montado"));
  //     myFile = SDFS.open("Dados.txt", "w");
  //          temCartao = true;
  //     SDFS.end();
  //   }

  ui_init();
}

long tempoPassado = 0;
char tempArredFormatada[5];
unsigned long tempoInicial;
char tempoRestante[5] = " :  ";

void loop() {
  unsigned long temPassado;
  int potLida = 100;
  potenciaAtual = map(potLida, 4, 1023, 0, 100);
  int potCompensada = arrayCompensacao[potenciaAtual];
  dim = map(potCompensada, 0, 100, 128, 0);

  // verificarSeguranca();
  // verificarDHT22();
  if ((millis() - tempoPassado) > 10000) {
    tempoPassado = millis();
    tempArredFormatada[5];
    dtostrf(tempAtual2, 4, 1, tempArredFormatada);

    if (temCartao) {
      myFile.print(potenciaAtual);
      myFile.print(",");
      myFile.println(tempArredFormatada);
    }

    temPassado = millis() - tempoInicial;
    int horasFaltantes = ((tempoMaterialAtual * 3600) - (temPassado / 1000)) / 3600;
    int minutosFaltantes = ((((tempoMaterialAtual * 3600) - (temPassado / 1000)) / 60) % 60);
    char buff1[1];
    itoa(horasFaltantes, buff1, 10);
    tempoRestante[0] = buff1[0];
    char buff2[3];
    itoa(minutosFaltantes, buff2, 10);
    tempoRestante[2] = buff2[0];
    tempoRestante[3] = buff2[1];

    char buffer[30];  // %i %s %f %u %c
    // snprintf_P(buffer, sizeof(buffer), PSTR("%i - %i  -- %s   ========   "), horasFaltantes, tempoMaterialAtual, tempoRestante);
    // debug(buffer);

    snprintf_P(buffer, sizeof(buffer), PSTR("%i    TEMP: "), potenciaAtual);
    Serial1.print(buffer);
    Serial1.println(tempArredFormatada);
  }
  lv_timer_handler();
}

void setup1() {}

void loop1() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

void secando() {}

void verificarDHT22() {
  if (measure_environment(&tempAtual2, &umidadeAtual2)) {
    umidadeAtual = round(umidadeAtual2);
    tempAtual = round(tempAtual2);  // Read Temperature(°C)
  }
}

void alterarTipoMaterial() {
  int tamanhoArray = sizeof(materiais) / sizeof(materiais[1]) - 1;
  tipoMaterialAtual++;

  if (tipoMaterialAtual < 0) {
    tipoMaterialAtual = tamanhoArray;
  }
  else if (tipoMaterialAtual > tamanhoArray) {
    tipoMaterialAtual = 0;
  }

  strcpy(nomeMaterialAtual, materiais[tipoMaterialAtual]);
  temperaturaMaterialAtual = temperaturaMateriais[tipoMaterialAtual];
  tempoMaterialAtual = tempoMateriais[tipoMaterialAtual];

  lv_label_set_text(ui_labelTextoBotaoFilamento, nomeMaterialAtual);
  char temp[7];
  lv_label_set_text(ui_labelTime, itoa(tempoMaterialAtual, temp, 10));
  lv_label_set_text(ui_labelTemp, itoa(temperaturaMaterialAtual, temp, 10));
}

// ---------------------------------------------------------------------------------------------------- Eventos Tela
void btnMudarFilamentoClick(lv_event_t *e) { alterarTipoMaterial(); }

void btnVoltarTelainicialClick(lv_event_t *e) { estado = 0; }

//=========================================================================== Outros
/*
 * Poll for a measurement, keeping the state machine alive.  Returns
 * true if a measurement is available.
 */
static bool measure_environment(float *temperature, float *humidity) {
  /* Measure once every 5 seconds. */
  if (millis() - measurement_timestamp > 3000ul) {
    if (dht_sensor.measure(temperature, humidity)) {
      measurement_timestamp = millis();
      return true;
    }
  }

  return false;
}

void zero_cross_detect() {
  zero_cross = true;  // set the boolean to true to tell our dimming function that a zero cross has occured
  i = 0;
  if (potenciaAtual < 100) digitalWrite(outputDimmer, LOW);  // turn off TRIAC (and AC)
}

// Turn on the TRIAC at the appropriate time
bool dim_check(struct repeating_timer *t) {
  if (zero_cross == true) {
    if (i >= dim) {
      digitalWrite(outputDimmer, HIGH);  // turn on light
      i = 0;                             // reset time step counter
      zero_cross = false;                // reset zero cross detection
    }
    else {
      i++;  // increment time step counter
    }
  }
  return true;
}

/*void verificarSeguranca() {
    if (tempAtual2 + 2 > temperaturaMaterialAtual) //Desligamento emergencia
    {
        // -------- Desligar relé ------------
        debug(tempAtual2);
        debug(" - Seguranca - ");
        debugLn(temperaturaMaterialAtual);
        dim = 130;
        menu.change_screen(&telaOverheating);
        menu.update();
        playAlert();
        while (true) {}
    }
}*/

//_______________________
/* display flash */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  // tft.setAddrWindow(area->x1, area->y1, w, h);
  // tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.pushImageDMA(area->x1, area->y1, w, h, (uint16_t *)&color_p->full);  // Use DMA for pushing colors
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/*touch read*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  bool touched = tft.getTouch(&touchX, &touchY, 600);
  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  }
  else {
    data->state = LV_INDEV_STATE_PR;

    /*set location*/
    data->point.x = touchX;
    data->point.y = touchY;

    Serial1.print("Data x ");
    Serial1.println(touchX);

    Serial1.print("Data y ");
    Serial1.println(touchY);
  }
}

void playAlert() {
  for (int x = 0; x < 10; x++) {
    tone(pinBeeper, 500, 1000);  // D4
    delay(300);
    tone(pinBeeper, 600, 1000);  // D4
    delay(400);
  }
}

void playEndSong() {
  for (int x = 0; x < 10; x++) {
    tone(pinBeeper, 294, 125);  // D4
    delay(125);
    tone(pinBeeper, 294, 125);  // D4
    delay(125);
    tone(pinBeeper, 587, 250);  // D5
    delay(250);
    tone(pinBeeper, 440, 250);  // A4
    delay(375);
    tone(pinBeeper, 415, 125);  // Ab4
    delay(250);
    tone(pinBeeper, 392, 250);  // G4
    delay(250);
    tone(pinBeeper, 349, 250);  // F4
    delay(250);
    tone(pinBeeper, 294, 125);  // D4
    delay(125);
  }
}

// void calibrate_touch() {
//   if (!calData[1]) {
//     tft.fillScreen(TFT_BLACK);
//     tft.calibrateTouch(calData, TFT_YELLOW, TFT_BLACK, 20);
//     Serial1.printf("calData[5] = {%d, %d, %d, %d, %d};\n", calData[0], calData[1], calData[2], calData[3], calData[4]);
//   }
// }