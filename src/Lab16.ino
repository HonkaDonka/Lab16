#include "oled-wing-adafruit.h"

SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);

bool prevConnection = true;
bool disconnect = false;
// Set up BLE UUIDs
const size_t UART_TX_BUF_SIZE = 20;

void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context);

const BleUuid serviceUuid("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");

const BleUuid rxUuid("6E400002-B5A3-F393-E0A9-E50E24DCCA9E");
BleCharacteristic rxCharacteristic("rx", BleCharacteristicProperty::WRITE_WO_RSP, rxUuid, serviceUuid, onDataReceived, NULL);

const BleUuid txUuid("6E400003-B5A3-F393-E0A9-E50E24DCCA9E");
BleCharacteristic txCharacteristic("tx", BleCharacteristicProperty::NOTIFY, txUuid, serviceUuid);

OledWingAdafruit display;

void setup()
{
  display.setup();
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  RGB.control(true);

  setupBLE();
  BLE.on();
}

void loop()
{
  display.loop();
  if (BLE.connected() && !prevConnection)
  {
    prevConnection = true;
    RGB.color(0, 0, 255);
    delay(5000);
    uint8_t txBuf[UART_TX_BUF_SIZE];
    String message = "Hello!\n";
    message.toCharArray((char *)txBuf, message.length() + 1);
    txCharacteristic.setValue(txBuf, message.length() + 1);
  }
  else if (!BLE.connected() && prevConnection)
  {
    prevConnection = false;
    RGB.color(255, 255, 0);
  }
  if (disconnect)
  {
    BLE.disconnect();
    BLE.off();
    setupBLE();
    BLE.on();
    disconnect = false;
  }
}

void onDataReceived(const uint8_t *data, size_t len, const BlePeerDevice &peer, void *context)
{
  for (size_t i = 0; i < len; i++)
  {
    char value = data[i];
    if (value == *"1")
    {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Hello World!");
      display.display();
    }
    else if (value == *"0")
    {
      display.clearDisplay();
      display.display();
      disconnect = true;
    }
  }
}

void setupBLE()
{
  BLE.addCharacteristic(txCharacteristic);
  BLE.addCharacteristic(rxCharacteristic);

  BleAdvertisingData data;
  data.appendServiceUUID(serviceUuid);
  BLE.advertise(&data);
  BLE.setDeviceName("Vinson's Argon");
}