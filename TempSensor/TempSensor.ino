// BLE-IoT Temperature Sensor
//   Waveshare ESP32-C3-Zero
//
// Required Libraries:
//    Adafruit SHT31
//
//  BLE Services:
//    - Environmental Sensing Service (ESS)
//


#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_SHT31.h>

// ===== Device Info  =====
#define DEVICE_NAME "ble-iot-A001"  


// ===== PIN Definitions  =====
#define RGB_PIN     10
#define GND_PIN     0
#define I2C_SCL_PIN 1
#define I2C_SDA_PIN 2

// ===== BLE UUIDs  =====
#define ESS_SERVICE_UUID          BLEUUID((uint16_t)0x181A)     // Environmental Sensing Service (ESS)
#define TEMPERATURE_CHAR_UUID     BLEUUID((uint16_t)0x2A6E)     // Temperature in Celsius
#define HUMIDITY_CHAR_UUID        BLEUUID((uint16_t)0x2A6F)     // Humidity in %


// ===== TIMING CONSTANTS =====
#define SENSOR_READ_INTERVAL    (1 * 60 * 1000)  // 5 minutes in milliseconds
#define HEARTBEAT_INTERVAL      (    10 * 1000)  // 10 seconds in milliseconds
#define LED_BLINK_DURATION      10               // 10ms LED blink



BLEServer* pServer = NULL;
BLECharacteristic* pTemperatureCharacteristic = NULL;
BLECharacteristic* pHumidityCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

unsigned long lastSensorRead = 0;
unsigned long lastHeartbeat = 0;

Adafruit_SHT31 sht31 = Adafruit_SHT31();

// ===== BLE SERVER CALLBACKS =====
class BLECallbacks: public BLEServerCallbacks 
{
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("  *** BLE Client Connected");
    }

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println(" *** BLE Client Disconnected");
    }
};

// ---- SETUP --------------------------------------
void setup() 
{

    //Create Virtual Ground on board
    pinMode(GND_PIN, OUTPUT);
    digitalWrite(GND_PIN, LOW); 

    rgbLedWrite(RGB_PIN, 0, 20, 0);  // Red, indicating startup


    Serial.begin(115200);
    delay(500);
    Serial.println("");
    Serial.println("");
    Serial.println(" === BLE-IoT Temperature Sensor ===");
    Serial.print("    Device Name: ");
    Serial.println(DEVICE_NAME);

    //Initialize SHT31
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if( !sht31.begin(0x44) ) 
    {
        Serial.println(" ** SHT31 ERROR **");
        while (1){ delay(1); }
    }
    Serial.println("Found SHT31");

    // Initialize BLE
    initBLE();
    
    rgbLedWrite(RGB_PIN, 0, 0, 0);  // Off, startup successful
}

// ---- LOOP --------------------------------------
void loop() 
{
    unsigned long currentMillis = millis();
    
    // Handle BLE connection state changes
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);  
        pServer->startAdvertising();
        Serial.println("Restarting BLE advertising...");
        oldDeviceConnected = deviceConnected;
    }
    
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
    
    // Sensor read and BLE notify
    if (currentMillis - lastSensorRead >= SENSOR_READ_INTERVAL) {
        lastSensorRead = currentMillis;
        lastHeartbeat  = currentMillis; 
        readAndNotifySensors();
    }
    
    // Heartbeat LED blink
    if (currentMillis - lastHeartbeat >= HEARTBEAT_INTERVAL) {
        lastHeartbeat = currentMillis;
        blinkHeartbeat();
    }

    delay(100); 
}



// ===== BLE INITIALIZATION =====
void initBLE() 
{
    Serial.println("Init BLE...");
    
    // Create BLE Device
    BLEDevice::init(DEVICE_NAME);
    
    // Create BLE Server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new BLECallbacks());
    

    // Create BLE Service 
    BLEService *pESS_Service = pServer->createService(ESS_SERVICE_UUID);
    
    // Create Temperature Characteristic
    pTemperatureCharacteristic = pESS_Service->createCharacteristic(
        TEMPERATURE_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pTemperatureCharacteristic->addDescriptor(new BLE2902());
    
    // Create Humidity Characteristic
    pHumidityCharacteristic = pESS_Service->createCharacteristic(
        HUMIDITY_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pHumidityCharacteristic->addDescriptor(new BLE2902());
    



    // Start the service
    pESS_Service->start();
    

    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(ESS_SERVICE_UUID);
    pAdvertising->setScanResponse(true);

    // ADVERTISING interval  (Units: 0.625ms)
    pAdvertising->setMinInterval(800);      // 500ms
    pAdvertising->setMaxInterval(1600);     // 1s

    // CONNECTION interval  (Units: 1.25ms) 
    pAdvertising->setMinPreferred(800);     // 1s
    pAdvertising->setMaxPreferred(4000);    // 5s


    BLEDevice::startAdvertising();
    
    Serial.println("BLE initialized successfully");
    Serial.println("Waiting for client connection...");
}

// ===== SENSOR READING AND NOTIFICATION =====
void readAndNotifySensors() 
{
    rgbLedWrite(RGB_PIN, 20, 0, 0); //green
    

    
    // Generate random sensor data
    float temp     = readTemperature();
    float humidity = readHumidity();
        
    // Log raw values
    Serial.print("T: ");
    Serial.print(temp, 1);
    Serial.print("  ");
    Serial.print("RH: ");
    Serial.print(humidity, 1);
    Serial.println("");
    
    // Encode according to ESS standard
    // Temperature: int16, 0.01°C resolution
    int16_t tempEncoded = (int16_t)(temp * 100);
    
    // Humidity: uint16, 0.01% resolution
    uint16_t humidityEncoded = (uint16_t)(humidity * 100);
    

    uint8_t tempData[2];
    tempData[0] = tempEncoded & 0xFF;       
    tempData[1] = (tempEncoded >> 8) & 0xFF; 
    
    uint8_t humidityData[2];
    humidityData[0] = humidityEncoded & 0xFF;  
    humidityData[1] = (humidityEncoded >> 8) & 0xFF; 
    
    // Update BLE characteristics
    pTemperatureCharacteristic->setValue(tempData, 2);
    pHumidityCharacteristic->setValue(humidityData, 2);
    
    // Notify connected clients
    if (deviceConnected) {
        pTemperatureCharacteristic->notify();
        pHumidityCharacteristic->notify();
    } else {
        Serial.println("No connected clients");
    }

    delay(LED_BLINK_DURATION);
    rgbLedWrite(RGB_PIN, 0, 0, 0);  // Turn off
    
}


void blinkHeartbeat() 
{
    rgbLedWrite(RGB_PIN, 0, 0, 20); //blue
    delay(LED_BLINK_DURATION);
    rgbLedWrite(RGB_PIN, 0, 0, 0);
}


float readHumidity()
{
  float rh = sht31.readHumidity();  //Percent
  if (isnan(rh)) {return -1.0; }
  return rh;
}


float readTemperature()
{
  float tc = sht31.readTemperature(); // Celsius
  if (isnan(tc)) { return -1.0; }
  return tc;
}

