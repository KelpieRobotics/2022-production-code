// Import Libraries
#include <DHT.h>

// Define Pins
const int PIN_DHT = 2;  // DHT22 sensor pin
const int PIN_Leak = 3; // Leak sensor pin
DHT dht(PIN_DHT, DHT22);

// Define Variables
String outputData;

void setup()
{
    // Start Serial
    Serial.begin(9600);
    // Prepare Pins
    dht.begin();
    pinMode(PIN_Leak, INPUT);
    // Serial Ready
    Serial.println("READY");
}

void loop()
{

    if (Serial.available() > 0)
    {
        String inputData = Serial.readStringUntil('\n');
        Serial.println(processData(inputData));
    }
}

String processData(String inputData)
{
    if (inputData == "TYPE")
    {
        return "SENSOR";
    }
    else
    {
        return String(getHumidity()) + "\t" + String(getTemperature()) + "\t" + String(getLeak());
    }
}

// Return current humidity
float getHumidity()
{
    return dht.readHumidity();
}

// Return current temperature
float getTemperature()
{
    return dht.readTemperature();
}

// Return state of Leak Sensor
int getLeak()
{
    return digitalRead(PIN_Leak);
}