void taskReadSensor(void *pvParameters);
void taskWriteSD(void *pvParameters);
void taskReadTermopar(void *pvParameters);

#include <Arduino.h>
#include "driver/ledc.h"
#include <HX711.h>
#include <SD.h>
#include "max6675.h"


#define PWM_PIN 2 // ALTERAR O PINO
//#define PWM_CHANNEL 0 - Canal LEDC (VERIFICAR SE PRECISA)

#define pinSO  19 
#define pinCS  16  
#define pinCLK 18  


const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

MAX6675 sensorTemp(pinCLK, pinCS, pinSO); 
HX711 scl;
File df;

SemaphoreHandle_t xsmfr;
TaskHandle_t xsnsr, xSD, xtrmpr;

float brt, liquido, temp;

void IRAM_ATTR isr() {
  xSemaphoreGiveFromISR(xsmfr, NULL);
}

void setup() {
  Serial.begin(9600); // 115200
  ledcSetup(0, 20000000 , 1);
  ledcAttachPin(PWM_PIN, 0);
  ledcWrite(0, 1);
  scl.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scl.set_scale(100.0);
  scl.tare();
  SD.begin();

  xsmfr = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(
      taskReadSensor,
      "TaskSensor",
      10000,
      NULL,
      1,
      &xsnsr,
      1);
  xTaskCreatePinnedToCore(
      taskWriteSD,
      "TaskSD",
      10000,
      NULL,
      1,
      &xSD,
      1);
  xTaskCreatePinnedToCore(
      taskReadTermopar, 
      "TaskTermopar",
      10000,
      NULL,
      1,
      &xtrmpr,
      1);
}

void loop() {
  // Nada precisa ser feito aqui, as tarefas irão executar em paralelo
}

void taskReadSensor(void *pvParameters) {
  (void)pvParameters;
  while (true) {
    if (xSemaphoreTake(xsmfr, portMAX_DELAY) == pdTRUE) {
      brt = scl.read();
      liquido = brt - scl.get_offset();
      int pwmValue = map(brt, 0, 1023, 0, 255); // Mapear a leitura do sensor para o ciclo de trabalho do PWM
      ledcWrite(0, pwmValue);  // Atualizar o PWM com base na leitura do sensor
      xTaskNotifyGive(xSD);
    }
  }
}

void taskWriteSD(void *pvParameters) {
  (void)pvParameters;
  while (true) {
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY) != 0) {
      df = SD.open("Teste_estatico.txt", FILE_WRITE);
      df.print(brt);
      df.print(", ");
      df.println(liquido);
      df.print(temp);
      df.print(", ");
      df.close();
    }
  }
}

void taskReadTermopar(void *pvParameters) {
  (void)pvParameters;
  while (true) {
    if (xSemaphoreTake(xsmfr, portMAX_DELAY) == pdTRUE) { 
      temp = sensorTemp.readCelsius(); 
      delayMicroseconds(2000000);  
      xTaskNotifyGive(xSD);
    }
  }
}
