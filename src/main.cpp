// *************************************************************************
// Title        : Lab02 LED Control
// File Name    : 'main.cpp'
// Target MCU   : Espressif ESP32 (Doit DevKit Version 1)
//
// EGRT 390     : Demonstration of GPIO progamming
//
// Revision History:
// When         Who         Description of change
// -----------  ----------- -----------------------
// 26-FEB-2025  [A.Reinert] Program start
// 26-FEB-2025  [A.Reinert] Updates to watchdog and debounce time
// 26-FEB-2025  [A.Reinert] Line notes added and Program Complete 
// *************************************************************************

// Include Files
// *************************************************************************
#include <Arduino.h>
#include <debounce.h>
#include <esp_task_wdt.h>

// Globals
// *************************************************************************
const uint8_t LED = 15;  //  ESP32-CAM LED
const uint8_t BUTTON = 17;  //   ESP32-CAM Button
const uint8_t LedBuiltin = 2; //   ESP32-CAM LED
bool ledState = false;  // LED state

uint8_t countpush = 0;
const uint16_t debounceTime = 5; /// 5ms debounce time
unsigned long debouncePrevious = 5; // millis()

unsigned long previousMillis = 0;  // will store last time LED was updated
unsigned long previousWdtMillis = 0; // will store last time LED was updated
const long interval = 1000; // interval at which to blink (milliseconds)
const long wdtinterval = 1000; // interval at which to blink (milliseconds)

bool logicLevel = HIGH;  // Button logic level
Debounce myButton(BUTTON, logicLevel);  // Debounce object

hw_timer_t * timer0 = NULL;  // Timer for button
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED; // Mutex for timer0
volatile bool flagTimer0 = false;  // Timer flag

// Interrupt Service Routine (ISR)
// *************************************************************************
void IRAM_ATTR timerISR0() // Timer ISR
{ 
     portENTER_CRITICAL_ISR(&timerMux0);  // Critical section
     flagTimer0 = true;   // Set flag
     portEXIT_CRITICAL_ISR(&timerMux0);  // End critical section
}

// Setup Code
// ****************************************************************************
void setup() // Setup
{
     Serial.begin(115200);  // Initialize serial port

     pinMode(LED, OUTPUT);  // Initialize LED
     pinMode(BUTTON, INPUT_PULLUP); // Initialize button
     pinMode(LedBuiltin, OUTPUT);  // Initialize LED

     timer0 = timerBegin(0, 80, true);  // 80 prescaler, count up
     timerAttachInterrupt(timer0, &timerISR0, true);  // Attach ISR
     timerAlarmWrite(timer0, 1000, true);  // 1ms period
     timerAlarmEnable(timer0);  // Enable timer

     esp_task_wdt_init(10, true);  // Initialize the watchdog timer,timeout = 10 sec
     esp_task_wdt_add(NULL);       // Add the current task to the watchdog timer
}

// Main program
// *************************************************************************
void loop() // Main program
{
     //Toggle LED if button is pressed
     unsigned long currentMillis = millis();   // Get the current time
 
     // Calculate the elapsed time in seconds
     unsigned long elapsedTime = currentMillis / 1000; // Convert milliseconds to seconds

     // Print the elapsed time to the terminal
     Serial.print("Elapsed time: ");   // Print the message
     Serial.print(elapsedTime);       // Print the elapsed time
     Serial.println(" seconds");    // Print a newline character

     if(flagTimer0) // Timer expired
     {
        portENTER_CRITICAL(&timerMux0);  // Critical section
        flagTimer0 = false;           // Clear flag
        portEXIT_CRITICAL(&timerMux0);   // End critical section
        myButton.update();      // Update button state
     }
     if (myButton.isPressed())  // Button pressed
     {
        ledState = !ledState;  // Toggle LED
        digitalWrite(LED, ledState);  //  Set LED
     }

     // Blink the LEDBUILTIN at a defined interval
     if (currentMillis - previousMillis >= interval) // Compare the time difference to the interval
     {
     previousMillis = currentMillis;     // Save the last time the LED was updated
     
     if (digitalRead(LedBuiltin) == LOW)  // If the LED is off
     {
      digitalWrite(LedBuiltin, HIGH);  // Turn it on
     } 
     else 
     {
      digitalWrite(LedBuiltin, LOW); // Otherwise, turn it off
     }
     }

     // watchdog timer
     if (currentMillis - previousWdtMillis >= wdtinterval) 
     {
          previousWdtMillis = currentMillis;
          Serial.println("Working...");    // Print a message every second
          esp_task_wdt_reset();       // Reset (feed) the watchdog timer
     }
}