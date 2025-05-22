#include <Arduino.h>
#include <SPIFFS.h>
#include "WaveFormGenerator.h"
#include "I2SOutput.h"

#define BUTTON_PIN 34 // GPIO21 pin connected to button
// Task handle
TaskHandle_t ButtonTaskHandle = NULL;

// i2s pins
i2s_pin_config_t i2sPins = {
    .bck_io_num = GPIO_NUM_27,
    .ws_io_num = GPIO_NUM_25,
    .data_out_num = GPIO_NUM_26,
    .data_in_num = -1};

I2SOutput *output;
WaveFormGenerator *sampleSource;

void handleButtonPress() {

  sampleSource->setMagnitude(0.5); 

}

void buttonTask(void *parameter) {
  pinMode(BUTTON_PIN, INPUT); // Use INPUT (no internal pull-ups on GPIO34)

  while (true) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      handleButtonPress();
      // Simple debounce delay
      vTaskDelay(10 / portTICK_PERIOD_MS);

      // Wait until button is released to prevent retriggering
      while (digitalRead(BUTTON_PIN) == LOW) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }

    sampleSource->setMagnitude(0.0); // Reset magnitude when button is not pressed

    vTaskDelay(10 / portTICK_PERIOD_MS); // Polling interval
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up");

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
    xTaskCreatePinnedToCore(
      buttonTask,        // Task function
      "Button Task",     // Name of task
      2048,              // Stack size
      NULL,              // Parameters
      1,                 // Priority
      &ButtonTaskHandle, // Task handle
      1                  // Core ID (core 1)
    );
}

void loop()
{
  // Serial.println("Looping");
  // delay(1000);
  // nothing to do here - everything is taken care of by tasks
}