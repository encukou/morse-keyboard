#include "lookup.h"

const int REF_PIN = A0;
const int DAT_PIN = A5;
const int THRESHOLD = 50;
const int DIT = 50;
const int EPSILON = 5;

#define MODE_DEBUG_READINGS 0
#define MODE_DEBUG_LENGTHS 1
#define MODE_MORSE 2
#define MODE_LETTERS 3

int mode = MODE_LETTERS;

unsigned long prevms;
int calibration_value = 0;

bool state = false;
unsigned long cooldown_start = 0;
unsigned long pulse_start = 0;

unsigned num_s;
unsigned index;
unsigned stride;

int read_value() {
    return analogRead(REF_PIN) - analogRead(DAT_PIN);
}

char get_char() {
    if (num_s >= num_tables) return '\xff';
    if (index >= sizeof(*morselookup) / sizeof(**morselookup)) return '\xff';
    return morselookup[num_s][index];
}

void setup() {
    Serial.begin(9600);
    prevms = millis();
    for (int i = 0; i < 10; i++) {
        calibration_value += read_value();
        delay(10);
    }
    calibration_value /= 10;
}

int counter = 0;

void loop() {
    unsigned long nowms = millis();
    int value = read_value() - calibration_value;

    if (mode == MODE_DEBUG_READINGS) Serial.println(value);

    if (Serial.available()) {
        char command = Serial.read();
        if ('1' < command < '4') {
            mode = command - '1';
        }
    }

    bool value_read = value > THRESHOLD;
    if (value_read == state) {
        cooldown_start = 0;
    } else {
        if (cooldown_start == 0) {
            cooldown_start = nowms;
        } else if (cooldown_start + EPSILON < nowms) {
            unsigned length = nowms - pulse_start;
            if (state) {
                if (length < DIT / 2 + EPSILON) {
                    if (mode == MODE_MORSE) Serial.print('s');
                    num_s++;
                } else if (length < 2 * DIT + EPSILON) {
                    if (mode == MODE_MORSE) Serial.print('.');
                    index++;
                    stride >>= 1;
                } else {
                    if (mode == MODE_MORSE) Serial.print('-');
                    index += 1 + stride;
                    stride >>= 1;
                }
            } else {
                if (length > 2 * DIT + EPSILON) {
                    if (mode == MODE_MORSE) Serial.print(' ');
                }
            }
            if (mode == MODE_DEBUG_LENGTHS) Serial.println(length);
            state = value_read;
            pulse_start = nowms;
        }
    }
    if (!state && (index || num_s) && pulse_start + 2 * DIT < nowms) {
        if (mode == MODE_LETTERS) Serial.print(get_char());
        num_s = 0;
        index = 0;
        stride = initial_stride;
    }

    prevms = millis();
}
