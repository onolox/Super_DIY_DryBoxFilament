#include "main.h"

void setup() {
    pinMode(pinBeeper, OUTPUT);

    SDFSConfig c2;
    c2.setCSPin(SD_CS_PIN);
    c2.setSPI(SPI1);
    SDFS.setConfig(c2);

    pinMode(OUTPUT_DIMMER, OUTPUT);                            // Set the Triac pin as output
    attachInterrupt(INPUT_DIMMER, zero_cross_detect, RISING);  // Attach an Interupt to Pin  for Zero Cross Detection
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

    // lv_obj_add_event_cb(ui_grafico, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);  // To generate fade in setpoint

    myPID.SetSampleTimeUs(1000000);  // 1s
    myPID.SetOutputLimits(0, 100);

    ui_init();
    changeMaterialType();

#if DEBUG == 1
    // delay(100);  // while (!Serial) { delay(10);}
    Serial1.begin(921600);
    debugLn(F("Starting..."));
#endif
}

float maxTemp = 0, minTemp = 0;
int offsetTemp = 0;
bool cicleLess = true, cicleMore = false, enteredCicleMore = false;
float tempCut = 0;
void loop() {
    if (state == 1) {
        float diffTemp = currentTemp - currentMaterialTemperature;

        if (timePast <= 0) {
            btnBackToStartScreenClick();
            playEndSong();
        }

        if (initially) {  // In the begining maintain power in 100%
            setPower(100);

            if (diffTemp >= tempCut) {
                initially = false;

                if (currentMaterialTemperature > 55) {  // PETG
                    pidFree = true;
                    setPower(65);
                    myPID.SetOutputLimits(50, 70);
                }
                else {
                    setPower(0);
                    myPID.SetOutputLimits(30, 60);
                }

                playButtonTone();
                debugLn("End holding temp");
            }
        }
        else {               // ----------------------------Start of the Main loop
            if (!pidFree) {  // Maintain manually for an hour
                if (tempNowIsLess() && cicleLess) {
                    if (diffTemp > 0.4) setPower(40);
                    else setPower(100 - offsetTemp);

                    cicleLess = false;
                    cicleMore = true;
                    enteredCicleMore = false;
                    maxTemp = lastTemp;
                    debug(" -1- ");
                }
                else if (tempNowIsMore() && cicleMore) {
                    if (!enteredCicleMore) {
                        enteredCicleMore = true;
                        minTemp = lastTemp;
                    }

                    if (diffTemp >= tempCut) {
                        setPower(offsetTemp);  // zero
                        offsetTemp += 10;
                        tempCut = tempCut - 0.03;
                        debug(" -2- ");
                        cicleLess = true;
                        cicleMore = false;

                        if (maxTemp - minTemp < 0.4) {
                            pidFree = true;
                            debugLn("PID liberado");
                        }
                    }
                }

                if (offsetTemp >= 40) {
                    offsetTemp = 40;
                    debugLn(" -5- ");
                }
            }

            if (pidFree) {
                input = currentTemp;

                if (myPID.Compute()) {
                    setPower(round(output));
                }
            }
        }

        verifySecurity();
        updateDataDisplay();
        logCard();
    }

#if DEBUG == 1
    logSerial();
#endif

    lv_timer_handler();
    sleep_ms(50);
}

// +++++++++++++++++++++++++++++++++++ Core 1 +++++++++++++++++++
void setup1() {
    pinMode(LED_BUILTIN, OUTPUT_2MA);

    mySensor.setType(22);  // Sensor 11 ou  22
    playButtonTone();
}

void loop1() {
    verifyDHT22();

    if (state == 1) {
        if (alert) playAlert();
    }

    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}

u_long timePastScreen = 0;
u_long timePastScreenChart = 0;
void updateDataDisplay() {
    if ((millis() - timePastScreen) > 500) {  // Refresh screen
        timePastScreen = millis();

        formatRemainingTime();
        char temp[8];
        char temp2[4];
        lv_label_set_text(ui_labelTemperatura, dtostrf(currentTemp, 4, 1, temp));
        lv_label_set_text(ui_labelUmidade, itoa(currentHumidity, temp2, 10));
        lv_label_set_text(ui_labelTempo, timeRemainingString);
        int minutesRemaining = (currentMaterialTime * 60) - ((timePast / 1000) / 60);
        lv_bar_set_value(ui_barraTempo, minutesRemaining, LV_ANIM_OFF);

        if ((millis() - timePastScreenChart) > 10000) {  // Refresh chart
            timePastScreenChart = millis();

            int minor = 100, larger = 0;
            lv_coord_t *arrSer1 = lv_chart_get_y_array(ui_grafico, ui_grafico_series_1);
            lv_coord_t *arrSer2 = lv_chart_get_y_array(ui_grafico, ui_grafico_series_2);

            for (int x = 0; x < 30; x++) {
                if (arrSer1[x] < minor) minor = arrSer1[x];
                if (arrSer1[x] > larger) larger = arrSer1[x];

                if ((arrSer2[x] / 10) < minor) minor = arrSer2[x] / 10;
                if ((arrSer2[x] / 10) > larger) larger = arrSer2[x] / 10;
            }

            if (minor > 100) minor = 99;
            if (larger > 100) larger = 100;

            if (minor > 4) minor -= 5;
            if (larger < 96) larger += 5;

            if (minor > 35) minor = 35;
            if (larger < 75) larger = 75;

            lv_chart_set_range(ui_grafico, LV_CHART_AXIS_PRIMARY_Y, minor, larger);
            lv_chart_set_range(ui_grafico, LV_CHART_AXIS_SECONDARY_Y, minor * 10, larger * 10);

            lv_chart_set_next_value(ui_grafico, ui_grafico_series_1, currentPower);
            lv_chart_set_next_value(ui_grafico, ui_grafico_series_2, currentTemp * 10);
            lv_chart_refresh(ui_grafico);
        }
    }
}

char tempRoundedFormatted[5] = "";
u_long timePastCard = 0;
void logCard() {
    if ((millis() - timePastCard) > 10000) {
        timePastCard = millis();
        dtostrf(currentTemp, 4, 1, tempRoundedFormatted);

        if (haveCard) {
            myFile.print(currentPower);
            myFile.print(",");
            myFile.println(tempRoundedFormatted);
            myFile.flush();
        }
    }
}

long timePastSerial = 0;
void logSerial() {
    if ((millis() - timePastSerial) > 10000) {
        timePastSerial = millis();
        dtostrf(currentTemp, 4, 1, tempRoundedFormatted);

        Serial1.printf("Pot: %i   TEMP: %s \n", currentPower, tempRoundedFormatted);
    }
}

void formatRemainingTime() {
    timePast = millis() - inititalTime;
    int hoursRemaining = ((currentMaterialTime * 3600) - (timePast / 1000)) / 3600;
    int minutesRemaining = (((currentMaterialTime * 3600) - (timePast / 1000)) / 60) % 60;
    char buff1[1];
    itoa(hoursRemaining, buff1, 10);
    timeRemainingString[0] = buff1[0];
    char buff2[3];
    itoa(minutesRemaining, buff2, 10);
    timeRemainingString[2] = buff2[0];
    timeRemainingString[3] = buff2[1];
}

void changeMaterialType() {
    int tamanhoArray = sizeof(materials) / sizeof(materials[1]) - 1;
    currentMaterialType++;

    if (currentMaterialType < 0) currentMaterialType = tamanhoArray;
    else if (currentMaterialType > tamanhoArray) currentMaterialType = 0;

    strcpy(currentMaterialName, materials[currentMaterialType]);
    currentMaterialTemperature = materialTemperatures[currentMaterialType];
    currentMaterialTime = materialTimes[currentMaterialType];
    setpoint = currentMaterialTemperature;

    if (currentMaterialTemperature > 55) tempCut = 0.2;
    else tempCut = -0.2;

    int totalMinutes = currentMaterialTime * 60;
    lv_bar_set_range(ui_barraTempo, 0, totalMinutes);
    lv_label_set_text(ui_labelTextoBotaoFilamento, currentMaterialName);
    char temp[7];
    lv_label_set_text(ui_labelTime, itoa(currentMaterialTime, temp, 10));
    lv_label_set_text(ui_labelTemp, itoa(currentMaterialTemperature, temp, 10));
    lv_label_set_text(ui_labelMaterial, currentMaterialName);

    lv_chart_set_all_value(ui_grafico, ui_grafico_series_sp, currentMaterialTemperature);
}

// ---------------------------------------------------------------------------------------------------- Screen Events
void btnStartClick() {
    playButtonTone();
    if (!SDFS.begin()) {  // Mount
        playButtonTone();
        debugLn(F("SD failed!"));
        haveCard = false;
        static const char *btns[] = {"Retry", ""};
        lv_obj_t *mbox1 = lv_msgbox_create(NULL, "Card Error!", "Not possible to read the card.", btns, false);
        lv_obj_add_event_cb(mbox1, eventButtonCard, LV_EVENT_PRESSED, NULL);
        lv_obj_center(mbox1);
    }
    else {
        state = 1;
        initially = true;
        debugLn(F("SD mounted"));
        myFile = SDFS.open("Data.txt", "w");
        haveCard = true;
        inititalTime = millis();

        File PIDfile = SDFS.open("PID.txt", "r");  // Set PID data
        float parameters[3];                                 // Kp Ki Kd
        int counter = 0;

        while (PIDfile.available() && counter < 3) {
            String line = PIDfile.readStringUntil('\n');
            line.trim();
            parameters[counter] = line.toFloat();
            counter++;
        }
        PIDfile.close();
        myPID.SetTunings(parameters[0], parameters[1], parameters[2]);
        debugLn("-------------------------------");
        debug("Starting... Kp=");
        debug(myPID.GetKp());
        debug(" Ki=");
        debug(myPID.GetKi());
        debug(" Kd=");
        debugLn(myPID.GetKd());

        myPID.SetMode(QuickPID::Control::automatic);

        lv_chart_set_all_value(ui_grafico, ui_grafico_series_1, LV_CHART_POINT_NONE);
        lv_chart_set_all_value(ui_grafico, ui_grafico_series_2, LV_CHART_POINT_NONE);
        _ui_screen_change(&ui_TelaRodando, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, &ui_TelaRodando_screen_init);
        playStartSong();
    }
}

void btnFilamentChangeClick(lv_event_t *e) {
    playButtonTone();
    changeMaterialType();
}

void btnBackToStartScreenClick() {
    playButtonTone();
    state = 0;
    setPower(0);
    myFile.close();
    SDFS.end();
    myPID.SetMode(QuickPID::Control::manual);
    _ui_screen_change(&ui_TelaInicial, LV_SCR_LOAD_ANIM_NONE, 500, 0, &ui_TelaInicial_screen_init);
}

void btnEndAlertClick(lv_event_t *e) {
    playButtonTone();
    alert = false;
    state = 0;
}

void clickChart() { playButtonTone(); }

void eventButtonCard(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_current_target(e);
    debugLn(lv_msgbox_get_active_btn_text(btn));

    lv_msgbox_close(lv_obj_get_parent(btn));
}

//============================================================= Others
u_long measurement_timestamp = 0;
float temp1 = 0, temp2 = 0, temp3 = 0;
void verifyDHT22() {
    if ((millis() - measurement_timestamp) > 3000) {
        measurement_timestamp = millis();

        if (mySensor.read() == DHTLIB_OK) {
            currentHumidity = round(mySensor.getHumidity());
            currentTemp = round(mySensor.getTemperature() * 10) / 10;

            temp1 = temp2;  // Making an average
            temp2 = temp3;
            temp3 = currentTemp;
            currentTemp = round(floatToIntX10((temp1 + temp2 + temp3) / 3)) / 10;
        }
    }
}

void setPower(int potencia) {
    currentPower = potencia;
    dim = map(arrayCompensation[currentPower], 0, 100, 128, 0);
}  // The dimmer goes from 128 to 0

/* void readEncoder() {
  int dtValue = digitalRead(ENCODER_DT);
  if (dtValue == HIGH) {
    currentTemp += 0.1;
  }
  if (dtValue == LOW) {
    currentTemp -= 0.1;
  }
} */

// -------------------------------------------------------------------------------- Dimmers
void zero_cross_detect() {
    zero_cross = true;  // set the boolean to true to tell our dimming function that a zero cross has occured
    interruptCounter = 0;
    if (dim > 0) digitalWrite(OUTPUT_DIMMER, LOW);  // turn off TRIAC (and AC)
}

// Turn on the TRIAC at the appropriate time
bool dim_check(struct repeating_timer *t) {
    if (zero_cross == true) {
        if (dim < 128 && interruptCounter >= dim) {
            digitalWrite(OUTPUT_DIMMER, HIGH);  // turn on light
            interruptCounter = 0;               // reset time step counter
            zero_cross = false;                 // reset zero cross detection
        }
        else {
            interruptCounter++;  // increment time step counter
        }
    }
    return true;
}

void verifySecurity() {
    if (currentTemp > (currentMaterialTemperature + 2) && !alert) {  // Emergency OFF
        dim = 128;
        currentPower = 0;
        debugLn(" - Security - ");
        debug("Temperature reached: ");
        debugLn(currentTemp);
        myFile.println("Overheating!");
        myFile.close();
        SDFS.end();
        myPID.SetMode(QuickPID::Control::manual);
        alert = true;
        lv_scr_load(ui_TelaOverheating);
    }

    if (floatToIntX10(currentTemp) != floatToIntX10(lastTemp)) {
        lastTemp = currentTemp;
        lastTimeTempModified = millis();
    }
    /*else if ((millis() - lastTimeTempModified) > 360000) {
        dim = 0;
        debugLn(" - Seguranca - ");
        debug("Temperatura atingiu: ");
        debugLn(currentTemp);
        myFile.println("Ficou tempo demais sem alterar a temperatura.");
        myFile.close();
        SDFS.end();
        myPID.SetMode(QuickPID::Control::manual);
        state = 0;
        alert = true;
        lv_scr_load(ui_TelaOverheating);
    }*/
}

bool tempNowIsLess() { return floatToIntX10(currentTemp) < floatToIntX10(lastTemp) ? true : false; }
bool tempNowIsMore() { return floatToIntX10(currentTemp) > floatToIntX10(lastTemp) ? true : false; }
int floatToIntX10(float num) { return round(num * 10); }

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

/* Usado para renderizar o degrade do setpoint no grafico*/
static void draw_event_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);

    /*Add the faded area before the lines are drawn*/
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
    if (dsc->part == LV_PART_ITEMS) {
        if (!dsc->p1 || !dsc->p2) return;
        // if (dsc->line_dsc->color != lv_color_hex(0xFFFFFF)) return;

        /*Add a line mask that keeps the area below the line*/
        lv_draw_mask_line_param_t line_mask_param;
        lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y, dsc->p2->x, dsc->p2->y, LV_DRAW_MASK_LINE_SIDE_BOTTOM);
        int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

        /*Add a fade effect: transparent bottom covering top*/
        lv_coord_t h = lv_obj_get_height(obj);
        lv_draw_mask_fade_param_t fade_mask_param;
        lv_draw_mask_fade_init(&fade_mask_param, &obj->coords, LV_OPA_COVER, obj->coords.y1 + h / 8, LV_OPA_TRANSP, obj->coords.y2);
        int16_t fade_mask_id = lv_draw_mask_add(&fade_mask_param, NULL);

        /*Draw a rectangle that will be affected by the mask*/
        lv_draw_rect_dsc_t draw_rect_dsc;
        lv_draw_rect_dsc_init(&draw_rect_dsc);
        draw_rect_dsc.bg_opa = LV_OPA_20;
        draw_rect_dsc.bg_color = dsc->line_dsc->color;

        lv_area_t a;
        a.x1 = dsc->p1->x;
        a.x2 = dsc->p2->x - 1;
        a.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
        a.y2 = obj->coords.y2;
        lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);

        /*Remove the masks*/
        lv_draw_mask_free_param(&line_mask_param);
        lv_draw_mask_free_param(&fade_mask_param);
        lv_draw_mask_remove_id(line_mask_id);
        lv_draw_mask_remove_id(fade_mask_id);
    }
    /*Hook the division lines too*/
    else if (dsc->part == LV_PART_MAIN) {
        if (dsc->line_dsc == NULL || dsc->p1 == NULL || dsc->p2 == NULL) return;

        /*Vertical line*/
        if (dsc->p1->x == dsc->p2->x) {
            dsc->line_dsc->color = lv_palette_lighten(LV_PALETTE_GREY, 1);
            if (dsc->id == 3) {
                dsc->line_dsc->width = 2;
                dsc->line_dsc->dash_gap = 0;
                dsc->line_dsc->dash_width = 0;
            }
            else {
                dsc->line_dsc->width = 1;
                dsc->line_dsc->dash_gap = 6;
                dsc->line_dsc->dash_width = 6;
            }
        }
        /*Horizontal line*/
        else {
            if (dsc->id == 2) {
                dsc->line_dsc->width = 2;
                dsc->line_dsc->dash_gap = 0;
                dsc->line_dsc->dash_width = 0;
            }
            else {
                dsc->line_dsc->width = 2;
                dsc->line_dsc->dash_gap = 6;
                dsc->line_dsc->dash_width = 6;
            }

            if (dsc->id == 1 || dsc->id == 3) {
                dsc->line_dsc->color = lv_palette_main(LV_PALETTE_GREEN);
            }
            else {
                dsc->line_dsc->color = lv_palette_lighten(LV_PALETTE_GREY, 1);
            }
        }
    }
}

// --------------------------------------------------------------------------------------------- Musicas
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
    tone(pinBeeper, 1000, 1000);
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