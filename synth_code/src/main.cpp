#include <Arduino.h>
#include <SPIFFS.h>
#include "WaveFormGenerator.h"
#include "I2SOutput.h"

#define BUTTON_PIN 34 // GPIO21 pin connected to button
// Task handle
SemaphoreHandle_t sampleSourceMutex;
TaskHandle_t ButtonTaskHandle = NULL;
TaskHandle_t ADSRTaskHandle = NULL; 

// i2s pins
i2s_pin_config_t i2sPins = {
    .bck_io_num = GPIO_NUM_27,
    .ws_io_num = GPIO_NUM_25,
    .data_out_num = GPIO_NUM_26,
    .data_in_num = -1};

I2SOutput *output;
WaveFormGenerator *sampleSource;

void handleButtonPress() {
    Serial.println("Button pressed!");
    if (xSemaphoreTake(sampleSourceMutex, portMAX_DELAY)) {
        sampleSource->m_envelope.noteOn();
        xSemaphoreGive(sampleSourceMutex);
    }
}

void buttonTask(void *parameter) {
    pinMode(BUTTON_PIN, INPUT);

    while (true) {
        if (digitalRead(BUTTON_PIN) == LOW) {
            handleButtonPress();
            vTaskDelay(10 / portTICK_PERIOD_MS);
            while (digitalRead(BUTTON_PIN) == LOW) {
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
        }

        if (xSemaphoreTake(sampleSourceMutex, portMAX_DELAY)) {
            sampleSource->m_envelope.noteOff();
            xSemaphoreGive(sampleSourceMutex);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void ADSRTask(void *parameter) {
    while (true) {
        float deltaTime = 1.0f / sampleSource->sampleRate();
        if (xSemaphoreTake(sampleSourceMutex, portMAX_DELAY)) {
            sampleSource->m_envelope.tick(deltaTime);
            xSemaphoreGive(sampleSourceMutex);
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up");

  sampleSourceMutex = xSemaphoreCreateMutex();

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!SPIFFS.begin()) {
      Serial.println("Failed to mount SPIFFS");
      while (true); // Halt execution
  }

  Serial.println("Created sample source");

  sampleSource = new WaveFormGenerator(40000, 400, 0);

  // sampleSource = new WAVFileReader("/sample.wav");

  Serial.println("Starting I2S Output");
  output = new I2SOutput();
  output->start(I2S_NUM_1, i2sPins, sampleSource);

    // Create the button task pinned to core 1
    xTaskCreate(
      buttonTask,        // Task function
      "Button Task",     // Name of task
      2048,              // Stack size
      NULL,              // Parameters
      1,                 // Priority
      &ButtonTaskHandle // Task handle
    );

    xTaskCreate(
      ADSRTask,        // Task function
      "ADSR Task",     // Name of task
      2048,              // Stack size
      NULL,              // Parameters
      1,                 // Priority
      &ADSRTaskHandle // Task handle
    );

}

void loop()
{
  // nothing to do here - everything is taken care of by tasks
}