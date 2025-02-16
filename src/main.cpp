#include "main.hpp"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // pinMode(pinBeeper, OUTPUT);

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

  SPI.begin(true);
  SPI1.begin(true);
  tft.begin();
  tft.setRotation(1);
  // calibrate_touch();
  tft.setTouch(calData);
  tft.setTextSize(3);

  tft.fillScreen(TFT_WHITE);

  tft.fillCircle(25, 25, 25, TFT_BLUE);
  tft.fillRect(51, 0, 49, 49, TFT_BLUE);
  tft.fillTriangle(124, 0, 101, 50, 150, 50, TFT_BLUE);

  //   if (!SDFS.begin()) {  // Monta
  //     Serial1.println(F("SD failed!"));
  //   }
  //   else {
  //     Serial1.println(F("SD montado"));
  //     myFile = SDFS.open("Dados.txt", "w");
  //          temCartao = true;
  //     SDFS.end();
  //   }

  pinMode(pinBeeper, INPUT);
}

long tempoPassado = 0;
char tempArredFormatada[5];
unsigned long tempoInicial;
char tempoRestante[5] = " :  ";

void loop() {
  unsigned long temPassado;
  short potLida = 100;
  potenciaAtual = map(potLida, 4, 1023, 0, 100);
  short potCompensada = arrayCompensacao[potenciaAtual];
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
    short horasFaltantes = ((tempoMaterialAtual * 3600) - (temPassado / 1000)) / 3600;
    short minutosFaltantes = ((((tempoMaterialAtual * 3600) - (temPassado / 1000)) / 60) % 60);
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

void alterarTipoMaterial(short qtd) {
  tipoMaterialAtual += qtd;
  if (tipoMaterialAtual < 0) {
    tipoMaterialAtual = 5;
  }
  else if (tipoMaterialAtual > 5) {
    tipoMaterialAtual = 0;
  }

  strcpy(nomeMaterialAtual, materiais[tipoMaterialAtual]);
  temperaturaMaterialAtual = temperaturaMateriais[tipoMaterialAtual];
  tempoMaterialAtual = tempoMateriais[tipoMaterialAtual];
}

/*
 * Poll for a measurement, keeping the state machine alive.  Returns
 * true if a measurement is available.
 */
static bool measure_environment(float* temperature, float* humidity) {
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
bool dim_check(struct repeating_timer* t) {
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

void playAlert() {
  for (short x = 0; x < 10; x++) {
    tone(pinBeeper, 500, 1000);  // D4
    delay(300);
    tone(pinBeeper, 600, 1000);  // D4
    delay(400);
  }
}

void playEndSong() {
  for (short x = 0; x < 10; x++) {
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