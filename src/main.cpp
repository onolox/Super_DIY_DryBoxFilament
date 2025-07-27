#include "main.h"

void setup() {
  pinMode(pinBeeper, OUTPUT);

  SDFSConfig c2;
  c2.setCSPin(SD_CS_PIN);
  c2.setSPI(SPI1);
  SDFS.setConfig(c2);

#if DEBUG == 1
  delay(100);  // while (!Serial) { delay(10);}
  Serial1.begin(921600);
  debugLn(F("Comecando..."));
#endif

  pinMode(outputDimmer, OUTPUT);                            // Set the Triac pin as output
  attachInterrupt(inputDimmer, zero_cross_detect, RISING);  // Attach an Interupt to Pin  for Zero Cross Detection
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

  myPID.SetSampleTimeUs(100000);  // 1s
  myPID.SetOutputLimits(0, 100);

  ui_init();
  alterarTipoMaterial();
}

void loop() {
  if (estado == 1) {
    if (temPassado <= 0) {
      btnVoltarTelainicialClick();
      playEndSong();
    }

    atualizarDadosDisplay();
    verificarSeguranca();

    input = tempAtual;

    if (myPID.Compute()) {
      potenciaAtual = round(output);

      dim = map(arrayCompensacao[potenciaAtual], 0, 100, 128, 0);  // O dimmer vai de 0 a 128
    }

    logCartao();
#if DEBUG == 1
    logSerial();
#endif
  }
  lv_timer_handler();
}

// +++++++++++++++++++++++++++++++++++ Core 1 +++++++++++++++++++
void setup1() {
  pinMode(LED_BUILTIN, OUTPUT_2MA);

  mySensor.setType(11);
}

void loop1() {
  if (estado == 1) {
    verificarDHT22();
    if (alerta) {
      playAlert();
    }
  }

  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
}

u_long tempoPassadoTela;
u_long tempoPassadoTelaGrafico;
void atualizarDadosDisplay() {
  if ((millis() - tempoPassadoTela) > 500) {  // Atualiza tela
    tempoPassadoTela = millis();

    formatarTempoRestante();
    char temp[8];
    char temp2[4];
    lv_label_set_text(ui_labelTemperatura, dtostrf(tempAtual, 4, 1, temp));
    lv_label_set_text(ui_labelUmidade, itoa(umidadeAtual, temp2, 10));
    lv_label_set_text(ui_labelTempo, tempoRestante);
    int minutosRestantes = (tempoMaterialAtual * 60) - ((temPassado / 1000) / 60);
    lv_bar_set_value(ui_barraTempo, minutosRestantes, LV_ANIM_OFF);

    if ((millis() - tempoPassadoTelaGrafico) > 10000) {  // Atualiza grafico
      tempoPassadoTelaGrafico = millis();
      lv_chart_set_next_value(ui_grafico, ui_grafico_series_1, potenciaAtual);
      lv_chart_set_next_value(ui_grafico, ui_grafico_series_2, tempAtual * 10);
      lv_chart_refresh(ui_grafico);
    }
  }
}

char tempArredFormatada[5] = "";
long tempoPassadoCartao = 0;
void logCartao() {
  if ((millis() - tempoPassadoCartao) > 10000) {
    tempoPassadoCartao = millis();
    dtostrf(tempAtual, 4, 1, tempArredFormatada);

    if (temCartao) {
      myFile.print(potenciaAtual);
      myFile.print(",");
      myFile.println(tempArredFormatada);
      myFile.flush();
    }
  }
}

long tempoPassadoSerial = 0;
void logSerial() {
  if ((millis() - tempoPassadoSerial) > 10000) {
    tempoPassadoSerial = millis();

    Serial1.printf("Pot: %i   TEMP: %s \n", potenciaAtual, tempArredFormatada);
  }
}

void formatarTempoRestante() {
  temPassado = millis() - tempoInicial;
  int horasFaltantes = ((tempoMaterialAtual * 3600) - (temPassado / 1000)) / 3600;
  int minutosFaltantes = (((tempoMaterialAtual * 3600) - (temPassado / 1000)) / 60) % 60;
  char buff1[1];
  itoa(horasFaltantes, buff1, 10);
  tempoRestante[0] = buff1[0];
  char buff2[3];
  itoa(minutosFaltantes, buff2, 10);
  tempoRestante[2] = buff2[0];
  tempoRestante[3] = buff2[1];
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
  setpoint = temperaturaMaterialAtual;

  int totalMinutos = tempoMaterialAtual * 60;
  lv_bar_set_range(ui_barraTempo, 0, totalMinutos);
  lv_label_set_text(ui_labelTextoBotaoFilamento, nomeMaterialAtual);
  char temp[7];
  lv_label_set_text(ui_labelTime, itoa(tempoMaterialAtual, temp, 10));
  lv_label_set_text(ui_labelTemp, itoa(temperaturaMaterialAtual, temp, 10));
  lv_label_set_text(ui_labelMaterial, nomeMaterialAtual);
}

// ---------------------------------------------------------------------------------------------------- Eventos Tela
void btnIniciarClick() {
  if (!SDFS.begin()) {  // Monta
    playButtonTone();
    debugLn(F("SD failed!"));
    temCartao = false;
    static const char *btns[] = {"Retentar", ""};
    lv_obj_t *mbox1 = lv_msgbox_create(NULL, "Erro Cartao!", "Nao foi possivel ler cartao.", btns, false);
    lv_obj_add_event_cb(mbox1, eventoBotaoCartao, LV_EVENT_PRESSED, NULL);
    lv_obj_center(mbox1);
  }
  else {
    estado = 1;
    debugLn(F("SD montado"));
    myFile = SDFS.open("Dados.txt", "w");
    temCartao = true;
    tempoInicial = millis();

    myPID.SetMode(QuickPID::Control::automatic);

    _ui_screen_change(&ui_TelaRodando, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_TelaRodando_screen_init);
    playStartSong();
  }
}

void btnMudarFilamentoClick(lv_event_t *e) {
  playButtonTone();
  alterarTipoMaterial();
}

void btnVoltarTelainicialClick() {
  playButtonTone();
  estado = 0;
  myFile.close();
  SDFS.end();
  myPID.SetMode(QuickPID::Control::manual);
  _ui_screen_change(&ui_TelaInicial, LV_SCR_LOAD_ANIM_NONE, 500, 0, &ui_TelaInicial_screen_init);
}

void btnEncerrarAlertaClick(lv_event_t *e) {
  playButtonTone();
  alerta = false;
  estado = 0;
}

void clicarGrafico() { playButtonTone(); }

void eventoBotaoCartao(lv_event_t *e) {
  lv_obj_t *btn = lv_event_get_current_target(e);
  debugLn(lv_msgbox_get_active_btn_text(btn));

  // if (lv_msgbox_get_active_btn_text(btn) == "Retentar") {
  //  if (!SDFS.begin()) {  // Monta
  //    debugLn(F("SD failed!"));
  //  }
  //  else {
  //    File arquivoPID = SDFS.open("ArquivoPID.txt", "r");  // Seta os dados do PID para tunning, temporario

  //   float parametros[3];  // Kp Ki Kd
  //   int contador = 0;

  //   while (arquivoPID.available() && contador < 3) {
  //     String linha = arquivoPID.readStringUntil('\n');
  //     linha.trim();
  //     parametros[contador] = linha.toFloat();
  //     contador++;
  //   }
  //   arquivoPID.close();
  //   SDFS.end();

  //   myPID.SetTunings(parametros[0], parametros[1], parametros[2]);
  //   debug(F("kp, ki, kd"));
  //   debug(parametros[0]);
  //   debug(parametros[1]);
  //   debugLn(parametros[2]);
  // }
  lv_msgbox_close(lv_obj_get_parent(btn));
  //}
  // else {
  //}
}

//=========================================================================== Outros
unsigned long measurement_timestamp = 0;
void verificarDHT22() {
  if ((millis() - measurement_timestamp) > 5000) {
    measurement_timestamp = millis();

    int chk = mySensor.read();
    if (chk == DHTLIB_OK) {
      umidadeAtual = round(mySensor.getHumidity());
      tempAtual = round(mySensor.getTemperature() * 10.0f) / 10.0f;
    }
  }
}

/* void readEncoder() {
  int dtValue = digitalRead(ENCODER_DT);
  if (dtValue == HIGH) {
    tempAtual += 0.1;
  }
  if (dtValue == LOW) {
    tempAtual -= 0.1;
  }
} */

// -------------------------------------------------------------------------------- Dimmers
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

void verificarSeguranca() {
  if (tempAtual > (temperaturaMaterialAtual + 2) && !alerta) {  // Desligamento emergencia
    dim = 0;
    debugLn(" - Seguranca - ");
    debug("Temperatura atingiu: ");
    debugLn(tempAtual);
    lv_scr_load(ui_TelaOverheating);
    alerta = true;
  }
}

//_________________________________________________________ Display
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

    // debug("Data x ");
    // debugLn(touchX);

    // debug("Data y ");
    // debugLn(touchY);
  }
}

void playAlert() {
  for (int x = 0; x < 10; x++) {
    tone(pinBeeper, 500, 1000);  // D4
    delay(300);
    tone(pinBeeper, 600, 1000);  // D4
    delay(400);
  }
  noTone(pinBeeper);
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
  noTone(pinBeeper);
}

void playButtonTone() {
  tone(pinBeeper, 2000, 200);
  noTone(pinBeeper);
}

void playStartSong() {
  tone(pinBeeper, 300, 200);
  delay(100);
  tone(pinBeeper, 400, 200);
  delay(100);
  tone(pinBeeper, 500, 200);
  delay(100);
  tone(pinBeeper, 600, 300);
  noTone(pinBeeper);
}

// void calibrate_touch() {
//   if (!calData[1]) {
//     tft.fillScreen(TFT_BLACK);
//     tft.calibrateTouch(calData, TFT_YELLOW, TFT_BLACK, 20);
//     Serial1.printf("calData[5] = {%d, %d, %d, %d, %d};\n", calData[0], calData[1], calData[2], calData[3], calData[4]);
//   }
// }