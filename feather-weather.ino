
#include <Simple-LoRaWAN.h>

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

using namespace SimpleLoRaWAN;

// TTN mapper
uint32_t devAddr    = 0x7DD44BFC;
uint8_t nwksKey[16] = { 0xEE, 0x1E, 0xF2, 0x15, 0x12, 0x5F, 0xD5, 0xCD, 0xAD, 0xA7, 0xBB, 0xE9, 0x3E, 0x4F, 0x53, 0x52 }; // MSBF
uint8_t appKey[16]  = { 0x6F, 0x45, 0x4E, 0x31, 0x3D, 0xB4, 0xE5, 0x75, 0x02, 0x55, 0x6C, 0x62, 0xB0, 0xCB, 0xD0, 0xD8 }; // MSBF

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 8,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN,
    .dio = {3, 6, LMIC_UNUSED_PIN},
};

Node* node;

Adafruit_BME280 bme; // I2C
#define SEALEVELPRESSURE_HPA (1013.25)

static const unsigned long REFRESH_INTERVAL = 10000; // ms
static unsigned long lastRefreshTime = 0;

void getValues(){
    uint16_t temperature = (uint16_t) ((bme.readTemperature() + 273.15f) * 100.0f);
    uint32_t pressure = (uint32_t) (bme.readPressure());
    uint16_t humidity = (uint16_t) (bme.readHumidity()*100.0f);

    uint8_t data[7] = {0};

    memcpy(data + 0, &((uint8_t*) (&temperature))[1], 1);
    memcpy(data + 1, &((uint8_t*) (&temperature))[0], 1);
    memcpy(data + 2, &((uint8_t*) (&pressure))[2], 1);
    memcpy(data + 3, &((uint8_t*) (&pressure))[1], 1);
    memcpy(data + 4, &((uint8_t*) (&pressure))[0], 1);
    memcpy(data + 5, &((uint8_t*) (&humidity))[1], 1);
    memcpy(data + 6, &((uint8_t*) (&humidity))[0], 1);

    node->send(data,7);

    for(int i = 0; i < 8; i++){
      Serial1.print(data[i], HEX);
      Serial1.print(", ");
    }
    Serial1.println("");

    Serial1.print("Temperature = ");
    Serial1.print(bme.readTemperature());
    Serial1.println(" *C");

    Serial1.print("Pressure = ");

    Serial1.print(bme.readPressure() / 100.0F);
    Serial1.println(" hPa");

    Serial1.print("Approx. Altitude = ");
    Serial1.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial1.println(" m");

    Serial1.print("Humidity = ");
    Serial1.print(bme.readHumidity());
    Serial1.println(" %");    
}

void setup()
{
    Serial1.begin(115200);
    Serial1.println("--- Feather-Weather ---");
    node = new ABP::Node(devAddr, nwksKey, appKey);
    //node = new OTAA::Node(appEui, devEui, appKey);
    node->disableLinkCheck();

    if (!bme.begin()) {
      Serial1.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
    }
}

void loop(){
  node->process();
  if(millis() - lastRefreshTime >= REFRESH_INTERVAL)
  {
    lastRefreshTime += REFRESH_INTERVAL;
    getValues();
  }
}

