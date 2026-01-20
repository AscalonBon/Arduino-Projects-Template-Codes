#include "DHT.h"

#define DHTPIN 2          // Pin where the sensor is connected
#define DHTTYPE DHT11     // Sensor type
#define FAN_PWM_PIN 3     // PWM pin for fan control
#define BUZZER_PIN 4      // Pin for buzzer
#define LED_PIN 5         // Optional LED pin (for visual alert)

// Temperature thresholds
#define NORMAL_TEMP 25.0      // Normal temperature threshold
#define FAN_ON_THRESHOLD 40.0 // Fan turns ON at 35°C
#define TEMP_HYSTERESIS 1.0   // Hysteresis to prevent rapid on/off switching

// Buzzer parameters - Different patterns for different temps
#define NORMAL_BEEP_INTERVAL 5000   // Beep every 5 seconds at normal temp (25°C and below)
#define WARNING_BEEP_INTERVAL 2000  // Beep every 2 seconds at warning temp (25-35°C)
#define ALARM_BEEP_INTERVAL 500     // Fast beep when fan is ON (35°C+)
#define CONTINUOUS_BEEP_THRESHOLD 25.0  // Continuous beep above this temp
#define BUZZER_FREQUENCY 1000      // Frequency in Hz
#define BUZZER_DURATION 200        // Duration of each beep in ms

// Fan control parameters
#define FAN_START_PWM 100    // Starting PWM value
#define FAN_MAX_PWM 255      // Maximum PWM value
#define FAN_RAMP_UP_TIME 500 // Ramp-up time

DHT dht(DHTPIN, DHTTYPE);

// State variables
bool fanRunning = false;
bool buzzerActive = false;
unsigned long fanStartTime = 0;
unsigned long lastBuzzerTime = 0;
bool buzzerState = false;
float lastValidTemp = 0;
unsigned long beepInterval = NORMAL_BEEP_INTERVAL;
float currentTemperatureC = 0;  // Declared globally

void setup() {
  Serial.begin(9600); // Initialize the serial monitor
  dht.begin();        // Initialize the sensor
  
  // Initialize pins
  pinMode(FAN_PWM_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Start with all outputs off
  digitalWrite(FAN_PWM_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("DHT11 Temperature Monitor with Fan and Buzzer Control");
  Serial.println("======================================================");
  Serial.println("System Behavior:");
  Serial.println("  • ≤25°C: Single beep every 5 seconds, Fan OFF");
  Serial.println("  • 25-35°C: Continuous beeping, Fan OFF");
  Serial.println("  • ≥35°C: Continuous beeping, Fan ON");
  Serial.println("======================================================");
}

void loop() {
  float humidity = dht.readHumidity();           // Read humidity
  currentTemperatureC = dht.readTemperature();   // Temperature in Celsius
  float temperatureF = dht.readTemperature(true); // Temperature in Fahrenheit
  float temperatureK = currentTemperatureC + 273.15;   // Temperature in Kelvin

  if (isnan(humidity) || isnan(currentTemperatureC)) {
    Serial.println("Error reading data!");
    // Use last valid temperature for control during sensor errors
    currentTemperatureC = lastValidTemp;
  } else {
    lastValidTemp = currentTemperatureC;
  }

  // Output data to the serial monitor
  Serial.print("Temperature: ");
  
  // Visual indicators for temperature levels
  if (currentTemperatureC <= NORMAL_TEMP) {
    Serial.print("[NORMAL] ");
  } else if (currentTemperatureC < FAN_ON_THRESHOLD) {
    Serial.print("[WARNING] ");
  } else {
    Serial.print("[FAN ACTIVE] ");
  }
  
  Serial.print(currentTemperatureC);
  Serial.print("°C, ");
  Serial.print(temperatureF);
  Serial.print("°F, ");
  Serial.print(temperatureK);
  Serial.println("K");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println("%");

  // BUZZER CONTROL LOGIC (Always active with different patterns)
  if (currentTemperatureC <= NORMAL_TEMP) {
    // Normal temperature (25°C and below) - beep every 5 seconds
    beepInterval = NORMAL_BEEP_INTERVAL;
    handleBuzzerPattern(currentTemperatureC);
    
  } else if (currentTemperatureC < FAN_ON_THRESHOLD) {
    // Warning temperature (25-35°C) - beep every 2 seconds
    beepInterval = WARNING_BEEP_INTERVAL;
    handleBuzzerPattern(currentTemperatureC);
    
  } else {
    // Temperature 35°C or above - fast beeping
    beepInterval = ALARM_BEEP_INTERVAL;
    
    // Continuous beeping
    unsigned long currentTime = millis();
    if (!buzzerActive) {
      buzzerActive = true;
      Serial.println("!!! ALARM: Temperature ≥35°C - Fan will turn ON !!!");
    }
    
    // For continuous beep, we beep at the specified interval
    if (currentTime - lastBuzzerTime >= beepInterval) {
      // Active buzzer: short pulse
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      delay(BUZZER_DURATION);
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
      
      lastBuzzerTime = currentTime;
      buzzerState = !buzzerState;
    }
  }

  // FAN CONTROL LOGIC (Activates at 35°C)
  if (currentTemperatureC >= FAN_ON_THRESHOLD) {
    if (!fanRunning) {
      fanRunning = true;
      fanStartTime = millis();
      Serial.println("!!! FAN ACTIVATED: Temperature reached 35°C !!!");
    }
    
    // Calculate fan speed based on temperature
    int fanSpeed = calculateFanSpeed(currentTemperatureC);
    analogWrite(FAN_PWM_PIN, fanSpeed);
    
    // Display fan status
    Serial.print("FAN: ON (PWM: ");
    Serial.print(fanSpeed);
    Serial.print("/255) - ");
    Serial.print("Speed: ");
    Serial.print(map(fanSpeed, 0, 255, 0, 100));
    Serial.println("%");
    
  } else if (currentTemperatureC <= (FAN_ON_THRESHOLD - TEMP_HYSTERESIS)) {
    // Turn off fan when temp drops below threshold with hysteresis
    if (fanRunning) {
      fanRunning = false;
      analogWrite(FAN_PWM_PIN, 0);
      Serial.println("FAN: DEACTIVATED - Temperature below 35°C");
    }
  }

  // Display status
  displaySystemStatus(currentTemperatureC);

  delay(100); // Small delay for responsiveness
}

void handleBuzzerPattern(float tempC) {
  unsigned long currentTime = millis();
  
  // Beep at specified interval
  if (currentTime - lastBuzzerTime >= beepInterval) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(BUZZER_DURATION);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    
    lastBuzzerTime = currentTime;
    buzzerState = true;
    
    // Reset buzzer active flag for normal temps
    if (tempC <= NORMAL_TEMP) {
      buzzerActive = false;
    } else {
      buzzerActive = true;
    }
  }
}

int calculateFanSpeed(float temperature) {
  // Simple fan control - increases with temperature above 35°C
  
  if (temperature <= FAN_ON_THRESHOLD) {
    return 0;
  } else {
    // Linear increase from 35°C to 50°C
    float maxTemp = 50.0; // Temperature for full speed
    if (temperature >= maxTemp) {
      return FAN_MAX_PWM;
    }
    
    float tempRange = maxTemp - FAN_ON_THRESHOLD;
    float tempAboveThreshold = temperature - FAN_ON_THRESHOLD;
    float ratio = tempAboveThreshold / tempRange;
    
    int speed = FAN_START_PWM + (int)((FAN_MAX_PWM - FAN_START_PWM) * ratio);
    
    // Smooth ramp-up when fan just started
    if (millis() - fanStartTime < FAN_RAMP_UP_TIME) {
      unsigned long elapsed = millis() - fanStartTime;
      float rampRatio = (float)elapsed / FAN_RAMP_UP_TIME;
      speed = (int)(FAN_START_PWM + (speed - FAN_START_PWM) * rampRatio);
    }
    
    return constrain(speed, FAN_START_PWM, FAN_MAX_PWM);
  }
}

void displaySystemStatus(float temp) {
  static unsigned long lastStatusTime = 0;
  unsigned long currentTime = millis();
  
  // Update status every 3 seconds
  if (currentTime - lastStatusTime >= 3000) {
    lastStatusTime = currentTime;
    
    Serial.println("\n=== SYSTEM STATUS ===");
    
    // Temperature status
    if (temp <= NORMAL_TEMP) {
      Serial.println("Temperature: NORMAL (≤25°C)");
      Serial.println("Buzzer: Periodic beep (every 5s)");
      Serial.println("Fan: OFF");
    } 
    else if (temp < FAN_ON_THRESHOLD) {
      Serial.println("Temperature: ELEVATED (25-35°C)");
      Serial.println("Buzzer: Continuous slow beep (every 2s)");
      Serial.println("Fan: OFF");
    }
    else {
      Serial.println("Temperature: HIGH (≥35°C)");
      Serial.println("Buzzer: Continuous fast beep (every 0.5s)");
      Serial.println("Fan: ON (speed varies with temperature)");
    }
    
    // Device status
    Serial.print("Fan: ");
    if (fanRunning) {
      Serial.print("ACTIVE - ");
      Serial.print(map(analogRead(FAN_PWM_PIN), 0, 255, 0, 100));
      Serial.println("% speed");
    } else {
      Serial.println("INACTIVE");
    }
    
    Serial.print("Buzzer Mode: ");
    if (temp <= NORMAL_TEMP) {
      Serial.println("PERIODIC");
    } else {
      Serial.println("CONTINUOUS");
    }
    
    Serial.println("====================\n");
  }
}

// Manual test functions
void testAllComponents() {
  Serial.println("Testing all components...");
  
  // Test buzzer
  Serial.println("Testing buzzer...");
  for (int i = 0; i < 5; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    delay(100);
  }
  
  // Test fan
  Serial.println("Testing fan...");
  analogWrite(FAN_PWM_PIN, 100);
  delay(1000);
  analogWrite(FAN_PWM_PIN, 200);
  delay(1000);
  analogWrite(FAN_PWM_PIN, 0);
  Serial.println("Component test complete");
}