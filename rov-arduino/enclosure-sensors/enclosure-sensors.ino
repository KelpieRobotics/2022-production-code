// Import Libraries
#include <DHT.h>
#include "PMBus.h"

// Define Pins
const int PIN_DHT = 2;  // DHT22 sensor pin
const int PIN_Leak = 3; // Leak sensor pin
DHT dht(PIN_DHT, DHT22);

PMBus pmbus;

// Define Variables
String outputData;

void setup()
{
    // Start Serial
    Serial.begin(9600);
    // Prepare Pins
    dht.begin();
    pinMode(PIN_Leak, INPUT);

    // begin i2c comms
    Wire.begin();
    Wire.setClock(400000); // set clock frequency, 400k is fast mode

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
        String pmbus_data = String(pmbus.vin_request()) + "\t" + String(pmbus.vout_request())+ "\t" + String(pmbus.current_out_request())+ "\t" + String(pmbus.temp_request())+ "\t" + String(pmbus.power_out_request()); 
        return String(getHumidity()) + "\t" + String(getTemperature()) + "\t" + String(getLeak()) + "\t" + pmbus_data;
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
