const int numReadings = 25;    // Number of voltage readings to collect
float voltageReadings[numReadings];
int currentReading = 0;
bool readingsComplete = false;

const float alfa = 0.1;        // Exponential moving average factor
float ema = 0.0;               // Exponential moving average

void setup() {
  Serial.begin(9600);
  analogRead(A2);
}

void loop() {
  // Read the sensor value
  int sensorValue = analogRead(A2);
  float voltage = convertAnalogToVoltage(sensorValue);

  // Store the voltage reading
  voltageReadings[currentReading] = voltage;
  currentReading++;

  // Check if all readings have been collected
  if (currentReading >= numReadings) {
    currentReading = 0;
    readingsComplete = true;
  }

  // Calculate the exponential moving average of the collected voltage readings
  if (readingsComplete) {
    ema = voltageReadings[0];
    for (int i = 1; i < numReadings; i++) {
      ema = (alfa * voltageReadings[i]) + ((1 - alfa) * ema);
    }

    // Print the analog value and filtered voltage
    Serial.print("Analog Value: ");
    Serial.print(sensorValue);
    Serial.print("\tVoltage: ");
    Serial.println(ema, 3);  // Print filtered voltage with 3 decimal points

    // Convert voltage mean to MPa using the function y = 0.262x - 0.265
    float mpa = 0.262 * ema - 0.265;

    // Publish the mean value in MPa
    Serial.print("Pressure: ");
    Serial.print(mpa, 3);  // Print pressure with 3 decimal points
    Serial.println(" MPa");
  }

  // Reset the readingsComplete flag
  readingsComplete = false;

  // Wait for a short delay
  delay(100);
}

float convertAnalogToVoltage(float analogValue) {
  // Apply the quadratic curve conversion
  float voltage = analogValue * (5.0 / 1023.0);

  return voltage;
}
