#include <util/atomic.h> 
#include <EEPROM.h>
#include "iface_nrf24l01.h"
#include <string.h>

#define MOSI_pin 3
#define SCK_pin 4
#define CE_pin 5
#define MISO_pin A0
#define CS_pin A1
#define ledPin 13

#define MOSI_on PORTD |= _BV(3)
#define MOSI_off PORTD &= ~_BV(3)
#define SCK_on PORTD |= _BV(4)
#define SCK_off PORTD &= ~_BV(4)
#define CE_on PORTD |= _BV(5)
#define CE_off PORTD &= ~_BV(5)
#define CS_on PORTC |= _BV(1)
#define CS_off PORTC &= ~_BV(1)
#define MISO_on (PINC & _BV(0))

#define RF_POWER TX_POWER_80mW

# define CHANNELS 12

enum chan_order {
	THROTTLE,
	AILERON,
	ELEVATOR,
	RUDDER,
	AUX1,
	AUX2,
	AUX3,
	AUX4,
	AUX5,
	AUX6,
	AUX7,
	AUX8
};

#define PPM_MIN 1000
#define PPM_SAFE_THROTTLE 1050
#define PPM_MID 1500
#define PPM_MAX 2000
#define PPM_MIN_COMMAND 1300
#define PPM_MAX_COMMAND 1700
#define GET_FLAG(ch, mask) (ppm[ch] > PPM_MAX_COMMAND ? mask : 0)

enum {
	PROTO_BAYANG
};

enum {
	ee_PROTOCOL_ID = 0,
	ee_TXID0,
	ee_TXID1,
	ee_TXID2,
	ee_TXID3
};

uint16_t overrun_cnt = 0;
uint8_t transmitterID[4];
uint8_t current_protocol;
static volatile bool ppm_ok = false;
uint8_t packet[32];
static bool reset = true;
volatile uint16_t Servo_data[12];
static uint16_t ppm[12] = {
	PPM_MIN,
	PPM_MID,
	PPM_MID,
	PPM_MID,
	PPM_MID,
	PPM_MID,
	PPM_MID,
	PPM_MID,
	PPM_MID,
	PPM_MID,
	PPM_MID,
	PPM_MID,
};

String inputString = "";
boolean stringComplete = false;
char * p, * i;
char * c = new char[200 + 1];
char * errpt;
uint8_t ppm_cnt;

void setup() {

	randomSeed((analogRead(A4) & 0x1F) | (analogRead(A5) << 5));
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, LOW);
	pinMode(MOSI_pin, OUTPUT);
	pinMode(SCK_pin, OUTPUT);
	pinMode(CS_pin, OUTPUT);
	pinMode(CE_pin, OUTPUT);
	pinMode(MISO_pin, INPUT);

	TCCR1A = 0;
	TCCR1B = 0;
	TCCR1B |= (1 << CS11);

	set_txid(false);

	Serial.begin(115200);
	
	Serial.println("======================================");
	Serial.println("           Welcome to Neudron         ");
	Serial.println("               Version 0.2            ");
	Serial.println("         https://bit.ly/2HRnWEr       ");
	Serial.println("       (c) 2018 Milan Kragujevic      ");
	Serial.println("======================================");
	
	inputString.reserve(200);
}

void loop() {
	uint32_t timeout;
	if (reset || ppm[AUX8] > PPM_MAX_COMMAND) {
		reset = false;
		selectProtocol();
		NRF24L01_Reset();
		NRF24L01_Initialize();
		init_protocol();
	}
	timeout = process_Bayang();
	overrun_cnt = 0;

	if (stringComplete) {
		strcpy(c, inputString.c_str());
		p = strtok_r(c, ",", & i);
		ppm_cnt = 0;
		while (p != 0) {
			int val = strtol(p, & errpt, 10);
			if (! * errpt) {
				Serial.print(val);
				ppm[ppm_cnt] = val;
			}
			p = strtok_r(NULL, ",", & i);
			ppm_cnt += 1;
		}
		inputString = "";
		stringComplete = false;
	}

	while (Serial.available()) {
		char inChar = (char) Serial.read();
		if (inChar == '\n') {
			stringComplete = true;
		} else {
			inputString += inChar;
		}

	}
	while (micros() < timeout) {};
}

void set_txid(bool renew) {
	uint8_t i;
	for (i = 0; i < 4; i++)
		transmitterID[i] = EEPROM.read(ee_TXID0 + i);
	if (renew || (transmitterID[0] == 0xFF && transmitterID[1] == 0x0FF)) {
		for (i = 0; i < 4; i++) {
			transmitterID[i] = random() & 0xFF;
			EEPROM.update(ee_TXID0 + i, transmitterID[i]);
		}
	}
}

void selectProtocol() {
	ppm_ok = false;
	set_txid(true);
	EEPROM.update(ee_PROTOCOL_ID, PROTO_BAYANG);
}

void init_protocol() {
	Bayang_init();
	Bayang_bind();
}
