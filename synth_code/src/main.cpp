#include <Arduino.h>
#include <SPIFFS.h>
#include "WaveFormGenerator.h"
#include "I2SOutput.h"
#include "utilities.h" 
#include <TFT_eSPI.h>

// Use hardware SPI
TFT_eSPI tft = TFT_eSPI();

#define TRIG_BUTTON 34 // GPIO21 pin connected to button
#define POT_PIN 35 // GPIO35 pin connected to potentiometer
#define SWITCH_PARAM_BUTTON 32 // GPIO32 pin connected to switch parameter button
#define SWITCH_WAVEFORM_BUTTON 39 // GPIO33 pin connected to switch waveform button
// Task handle

TaskHandle_t trigTaskHandle = NULL;
TaskHandle_t potentiometerTaskHandle = NULL;
TaskHandle_t switchADSRTaskHandle = NULL;
TaskHandle_t switchWaveformTaskHandle = NULL;
TaskHandle_t screenTaskHandler = NULL;

enum paramToModify {
  ATTACK,
  DECAY,
  SUSTAIN,
  RELEASE
};

int currentParam = ATTACK; // Start with attack parameter

// i2s pins
i2s_pin_config_t i2sPins = {
    .bck_io_num = GPIO_NUM_27,
    .ws_io_num = GPIO_NUM_25,
    .data_out_num = GPIO_NUM_26,
    .data_in_num = -1};

I2SOutput *output;
WaveFormGenerator *osc1;

void switchADSRTask(void *parameter) {
  pinMode(SWITCH_PARAM_BUTTON, INPUT); // Use INPUT (no internal pull-ups on GPIO32)

  while (true) {

    if (digitalRead(SWITCH_PARAM_BUTTON) == LOW) {
      // Switch to the next parameter
      currentParam = (currentParam + 1) % 4; // Cycle through 0-3

      switch (currentParam) {
        case ATTACK:
          Serial.println("Switched to Attack");
          break;
        case DECAY:
          Serial.println("Switched to Decay");
          break;
        case SUSTAIN:
          Serial.println("Switched to Sustain");
          break;
        case RELEASE:
          Serial.println("Switched to Release");
          break;
      }

      vTaskDelay(10 / portTICK_PERIOD_MS); // Debounce delay

      // Wait until button is released to prevent retriggering
      while (digitalRead(SWITCH_PARAM_BUTTON) == LOW) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS); // Polling interval
  }
}
void trigTask(void *parameter) {
  pinMode(TRIG_BUTTON, INPUT); // Use INPUT (no internal pull-ups on GPIO34)

  while (true) {
    if (digitalRead(TRIG_BUTTON) == LOW) {
      osc1->m_envelope.noteOn(); // Trigger the envelope

      vTaskDelay(10 / portTICK_PERIOD_MS); // Debounce delay

      // Wait until button is released to prevent retriggering
      while (digitalRead(TRIG_BUTTON) == LOW) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }

    osc1->m_envelope.noteOff(); // Reset magnitude when button is not pressed

    vTaskDelay(10 / portTICK_PERIOD_MS); // Polling interval
  }
}

void potentiometerTask(void *parameter) {


  int lastPotValue = -1; // Previous raw analog value

  while (true) {
    int potValue = analogRead(POT_PIN);

    // Only act if the pot actually changed
    if (potValue != lastPotValue) { // Threshold to avoid noise
      lastPotValue = potValue;

      switch (currentParam) {
        case ATTACK: {
          double attackTime = mapExp(potValue, 0, 4095, 0.001, 5);
          osc1->m_envelope.setAttackTime(attackTime);
          Serial.printf("Attack Time: %f seconds\n", attackTime);
          break;
        }
        case DECAY: {
          double decayTime = mapExp(potValue, 0, 4095, 0.001, 5);
          osc1->m_envelope.setDecayTime(decayTime);
          Serial.printf("Decay Time: %f seconds\n", decayTime);
          break;
        }
        case SUSTAIN: {
          double sustainLevel = (double) potValue / 4095.0; // Normalize to 0-1 range)
          osc1->m_envelope.setSustainLevel(sustainLevel);
          Serial.printf("Sustain Level: %f\n", sustainLevel);
          break;
        }
        case RELEASE: {
          double releaseTime = mapExp(potValue, 0, 4095, 0.001, 5);
          osc1->m_envelope.setReleaseTime(releaseTime);
          Serial.printf("Release Time: %f seconds\n", releaseTime);
          break;
        }
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS); // Adjust as needed
  }
}

void switchWaveFormTask(void *parameter) {
  pinMode(SWITCH_WAVEFORM_BUTTON, INPUT); // Use INPUT (no internal pull-ups on GPIO39)

  while (true) {
    if (digitalRead(SWITCH_WAVEFORM_BUTTON) == LOW) {
      // Switch to the next waveform type
      WaveType currentWaveType = osc1->getWaveType();
      WaveType newWaveType = static_cast<WaveType>((static_cast<int>(currentWaveType) + 1) % 4);
      osc1->setWaveType(newWaveType);

      switch (newWaveType) {
        case SINE:
          Serial.println("Switched to Sine Wave");
          break;
        case SQUARE:
          Serial.println("Switched to Square Wave");
          break;
        case TRIANGLE:
          Serial.println("Switched to Triangle Wave");
          break;
        case SAWTOOTH:
          Serial.println("Switched to Sawtooth Wave");
          break;
        default:
          Serial.println("Unknown Waveform Type");
          break;
      }

      vTaskDelay(10 / portTICK_PERIOD_MS); // Debounce delay

      // Wait until button is released to prevent retriggering
      while (digitalRead(SWITCH_WAVEFORM_BUTTON) == LOW) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS); // Polling interval
  }
}

void screenTask(void *parameter) {

  float prevAttackTime = 0; // Previous attack time for line drawing
  float prevDecayTime = 0; // Previous decay time for line drawing
  float prevSustainLevel = 0; // Previous sustain level for line drawing
  float prevReleaseTime = 0; // Previous release time for line drawing

  while (true) {
    Serial.println("Screen Task Running");

    float attackTime = osc1->m_envelope.getAttackTime(); 
    float decayTime = osc1->m_envelope.getDecayTime();
    float sustainLevel = osc1->m_envelope.getSustainLevel();
    float releaseTime = osc1->m_envelope.getReleaseTime();

    // clear old line 
    tft.drawLine(0, 240, prevAttackTime, 0, TFT_BLACK); 
    tft.drawLine(prevAttackTime, 0, prevAttackTime + prevDecayTime, sustainLevel , TFT_BLACK);
    // draw new line 
    tft.drawLine(0, 240, attackTime, 0, TFT_GREEN);
    tft.drawLine(attackTime, 0, attackTime + decayTime, 100, TFT_GREEN); 

    prevAttackTime = attackTime; 
    prevDecayTime = decayTime;

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}



void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up");

  pinMode(TRIG_BUTTON, INPUT_PULLUP);

  if (!SPIFFS.begin()) {
      Serial.println("Failed to mount SPIFFS");
      while (true); // Halt execution
  }

  Serial.println("Created sample source");

  osc1 = new WaveFormGenerator(40000, 400, 0);

  Serial.println("Starting I2S Output");
  output = new I2SOutput();
  output->start(I2S_NUM_1, i2sPins, osc1);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  // Create the button task pinned to core 1
  xTaskCreate(
    trigTask,        // Task function
    "Trigger Task",     // Name of task
    2048,              // Stack size
    NULL,              // Parameters
    1,                 // Priority
    &trigTaskHandle // Task handle
  );

  xTaskCreatePinnedToCore(
    potentiometerTask,        // Task function
    "potentiometerTask",     // Name of task
    2048,              // Stack size
    NULL,              // Parameters
    1,                 // Priority
    &potentiometerTaskHandle, // Task handle
    1
  );

  xTaskCreatePinnedToCore(
    switchADSRTask,        // Task function
    "switchADSRTask",     // Name of task
    1024,              // Stack size
    NULL,              // Parameters
    1,                 // Priority
    &switchADSRTaskHandle, // Task handle
    1
  );

  xTaskCreatePinnedToCore(
    switchWaveFormTask,        // Task function
    "switchWaveFormTask",     // Name of task
    1024,              // Stack size
    NULL,              // Parameters
    1,                 // Priority
    &switchWaveformTaskHandle, // Task handle
    1
  );

  xTaskCreatePinnedToCore(
    screenTask,        // Task function
    "screen task",     // Name of task
    4096,              // Stack size
    NULL,              // Parameters
    1,                 // Priority
    &screenTaskHandler, // Task handle
    0
  );

}

void loop()
{
  // nothing to do here - everything is taken care of by tasks
}