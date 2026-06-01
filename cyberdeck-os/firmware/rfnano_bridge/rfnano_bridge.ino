#include <RF24.h>
#include <SPI.h>

/*
 * RF-Nano serial bridge for Busteg OS.
 *
 * Arduino IDE:
 *   Board: Arduino Nano
 *   Processor: ATmega328P, or ATmega328P (Old Bootloader) if upload fails
 *   Library: RF24 by TMRh20
 *
 * Serial protocol at 115200 baud:
 *   PING
 *   STATUS
 *   CH <0-125>
 *   ADDR <5-character-address>
 *   SCAN START
 *   SCAN STOP
 *   TX <hex payload, max 32 bytes>
 *
 * Async receive lines:
 *   RX <length> <hex payload>
 *   SCAN <channel> RPD
 */

RF24 radio_9_10(9, 10);
RF24 radio_10_9(10, 9);
RF24 radio_7_8(7, 8);
RF24 radio_8_7(8, 7);
RF24 *radio = &radio_9_10;

static const uint8_t MAX_PAYLOAD = 32;
static char line[96];
static uint8_t line_len = 0;
static uint8_t channel = 76;
static byte address[6] = "BUSTG";
static uint8_t ce_pin = 9;
static uint8_t csn_pin = 10;
static bool radio_ok = false;
static bool scan_enabled = false;
static uint8_t scan_channel = 0;
static unsigned long last_scan_ms = 0;

static int hex_value(char ch) {
  if (ch >= '0' && ch <= '9') return ch - '0';
  if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
  if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
  return -1;
}

static uint8_t parse_hex_payload(const char *hex, uint8_t *out, uint8_t max_len) {
  uint8_t count = 0;
  while (*hex == ' ') hex++;
  while (hex[0] && hex[1] && count < max_len) {
    int hi = hex_value(hex[0]);
    int lo = hex_value(hex[1]);
    if (hi < 0 || lo < 0) break;
    out[count++] = (uint8_t)((hi << 4) | lo);
    hex += 2;
    while (*hex == ' ') hex++;
  }
  return count;
}

static void print_hex(const uint8_t *data, uint8_t length) {
  static const char hex[] = "0123456789ABCDEF";
  for (uint8_t i = 0; i < length; ++i) {
    Serial.print(hex[data[i] >> 4]);
    Serial.print(hex[data[i] & 0x0F]);
  }
}

static void apply_radio_config() {
  if (!radio_ok) {
    return;
  }
  scan_enabled = false;
  radio->setChannel(channel);
  radio->setDataRate(RF24_250KBPS);
  radio->setPALevel(RF24_PA_LOW);
  radio->setRetries(5, 15);
  radio->setPayloadSize(MAX_PAYLOAD);
  radio->openWritingPipe(address);
  radio->openReadingPipe(1, address);
  radio->startListening();
}

static bool try_radio(RF24 *candidate, uint8_t ce, uint8_t csn) {
  radio = candidate;
  ce_pin = ce;
  csn_pin = csn;
  if (!radio->begin()) {
    return false;
  }
  radio_ok = true;
  radio->enableDynamicPayloads();
  apply_radio_config();
  return true;
}

static void detect_radio() {
  radio_ok = false;
  if (try_radio(&radio_9_10, 9, 10)) return;
  if (try_radio(&radio_10_9, 10, 9)) return;
  if (try_radio(&radio_7_8, 7, 8)) return;
  if (try_radio(&radio_8_7, 8, 7)) return;
}

static void start_scan() {
  if (!radio_ok) {
    Serial.println(F("ERR SCAN RADIO"));
    return;
  }
  scan_enabled = true;
  scan_channel = 0;
  last_scan_ms = 0;
  radio->setAutoAck(false);
  radio->setDataRate(RF24_2MBPS);
  radio->setPALevel(RF24_PA_MIN);
  radio->startListening();
  Serial.println(F("OK SCAN START"));
}

static void stop_scan() {
  scan_enabled = false;
  apply_radio_config();
  Serial.println(F("OK SCAN STOP"));
}

static void handle_command(char *cmd) {
  if (strcmp(cmd, "PING") == 0) {
    Serial.println(F("OK RFNANO_BRIDGE 1"));
    return;
  }

  if (strcmp(cmd, "STATUS") == 0) {
    Serial.print(F("OK STATUS CH "));
    Serial.print(channel);
    Serial.print(F(" ADDR "));
    Serial.write(address, 5);
    Serial.print(F(" RADIO "));
    Serial.print(radio_ok ? F("OK") : F("ERR"));
    Serial.print(F(" CE "));
    Serial.print(ce_pin);
    Serial.print(F(" CSN "));
    Serial.print(csn_pin);
    Serial.print(F(" SCAN "));
    Serial.print(scan_enabled ? F("ON") : F("OFF"));
    Serial.println();
    return;
  }

  if (strcmp(cmd, "RETRY") == 0) {
    detect_radio();
    Serial.println(radio_ok ? F("OK RADIO") : F("ERR RADIO"));
    return;
  }

  if (strncmp(cmd, "CH ", 3) == 0) {
    int next_channel = atoi(cmd + 3);
    if (next_channel < 0 || next_channel > 125) {
      Serial.println(F("ERR CH"));
      return;
    }
    channel = (uint8_t)next_channel;
    apply_radio_config();
    Serial.print(F("OK CH "));
    Serial.println(channel);
    return;
  }

  if (strcmp(cmd, "SCAN START") == 0) {
    start_scan();
    return;
  }

  if (strcmp(cmd, "SCAN STOP") == 0) {
    stop_scan();
    return;
  }

  if (strncmp(cmd, "ADDR ", 5) == 0) {
    if (strlen(cmd + 5) < 5) {
      Serial.println(F("ERR ADDR"));
      return;
    }
    memcpy(address, cmd + 5, 5);
    apply_radio_config();
    Serial.print(F("OK ADDR "));
    Serial.write(address, 5);
    Serial.println();
    return;
  }

  if (strncmp(cmd, "TX ", 3) == 0) {
    if (!radio_ok) {
      Serial.println(F("ERR TX RADIO"));
      return;
    }
    uint8_t payload[MAX_PAYLOAD];
    uint8_t length = parse_hex_payload(cmd + 3, payload, sizeof(payload));
    if (length == 0) {
      Serial.println(F("ERR TX EMPTY"));
      return;
    }
    radio->stopListening();
    bool ok = radio->write(payload, length);
    radio->startListening();
    Serial.println(ok ? F("OK TX") : F("ERR TX NOACK"));
    return;
  }

  Serial.println(F("ERR UNKNOWN"));
}

static void poll_serial() {
  while (Serial.available() > 0) {
    char ch = (char)Serial.read();
    if (ch == '\r') {
      continue;
    }
    if (ch == '\n') {
      line[line_len] = '\0';
      handle_command(line);
      line_len = 0;
    } else if (line_len + 1 < sizeof(line)) {
      line[line_len++] = ch;
    } else {
      line_len = 0;
      Serial.println(F("ERR LINE"));
    }
  }
}

static void poll_radio() {
  if (!radio_ok || scan_enabled || !radio->available()) {
    return;
  }

  uint8_t payload[MAX_PAYLOAD];
  uint8_t length = radio->getDynamicPayloadSize();
  if (length == 0 || length > MAX_PAYLOAD) {
    length = MAX_PAYLOAD;
  }
  radio->read(payload, length);

  Serial.print(F("RX "));
  Serial.print(length);
  Serial.print(' ');
  print_hex(payload, length);
  Serial.println();
}

static void poll_scan() {
  if (!radio_ok || !scan_enabled) {
    return;
  }

  unsigned long now = millis();
  if (now - last_scan_ms < 8) {
    return;
  }
  last_scan_ms = now;

  radio->setChannel(scan_channel);
  radio->startListening();
  delayMicroseconds(180);
  if (radio->testRPD()) {
    Serial.print(F("SCAN "));
    Serial.print(scan_channel);
    Serial.println(F(" RPD"));
  }
  scan_channel = (uint8_t)((scan_channel + 1) % 126);
}

void setup() {
  Serial.begin(115200);
  detect_radio();
  Serial.println(radio_ok ? F("OK RFNANO_BRIDGE 1") : F("ERR RADIO"));
}

void loop() {
  poll_serial();
  poll_scan();
  poll_radio();
}
