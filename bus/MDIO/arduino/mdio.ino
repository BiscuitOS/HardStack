/*
 * MDIO/SMI/MIIM on Arduino
 *
 * (C) 2018.12.20 BuddyZhang1 <buddy.zhang@aliyun.com>
 * (C) Sword <xxx@jjj.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DIR    4 // 74HCT245 Dir
#define MDC    3 // D3 PIN for MDC
#define MDIO   2 // D2 PIN for MDIO
#define OUT    1
#define IN     0

int smi_init(void)
{
    pinMode(DIR, OUTPUT);
    digitalWrite(DIR, OUT);	//245 A to Y
    pinMode(MDC, OUTPUT);
    pinMode(MDIO, OUTPUT);
    return 0;
}


/*! Generates a rising edge pulse on MDC */
void pulse_mdc(void)
{
    volatile uint8_t i;
    //pinMode(MDC, OUTPUT);
    //i++;
    digitalWrite(MDC, 0);
    //delay(1);  change from 1Kbit/s to 10Kbit/s
    delayMicroseconds(100);
    i++;
    digitalWrite(MDC, 1);
    //delay(1);
    delayMicroseconds(100);
    i++;
}

/*
 * SMI/MDIO/MIIM write
 *  @phy: PHY id
 *  @reg: Register address.
 *  @data: data need to write.
 */
void write_smi(uint8_t phy, uint8_t reg, uint16_t data)
{
    uint8_t byte;
    uint16_t word;

    /* MDIO pin is output */
    digitalWrite(DIR, OUT);
    pinMode(MDIO, OUTPUT);

    digitalWrite(MDIO, 1);
    digitalWrite(MDC, 1);
    for (byte = 0;byte < 32; byte++)
        pulse_mdc();

    /* Stat code */
    digitalWrite(MDIO, 0);
    pulse_mdc();
    digitalWrite(MDIO, 1);
    pulse_mdc();

    /* Write OP Code */
    digitalWrite(MDIO, 0);
    pulse_mdc();
    digitalWrite(MDIO, 1);
    pulse_mdc();

    /* PHY address - 5 bits */
    for (byte = 0x10; byte != 0; byte = byte >> 1) {
        if (byte & phy)
            digitalWrite(MDIO, 1);
        else
            digitalWrite(MDIO, 0);
        pulse_mdc();
    }

    /* REG address - 5 bits */
    for (byte = 0x10; byte != 0; byte = byte >> 1) {
        if (byte & reg)
            digitalWrite(MDIO, 1);
        else
            digitalWrite(MDIO, 0);

        pulse_mdc();
    }
    /* Turn around bits */
    digitalWrite(MDIO, 1);
    pulse_mdc();
    digitalWrite(MDIO, 0);
    pulse_mdc();

    /* Data - 16 bits */
    for(word = 0x8000; word != 0; word = word >> 1) {
        if (word & data)
            digitalWrite(MDIO, 1);
        else
            digitalWrite(MDIO, 0);

        pulse_mdc();
    }

    /* This is needed for some reason... */
    pulse_mdc();
    /* Stay in 0 state */
    //MDC = 0;
    digitalWrite(DIR, IN);
    pinMode(MDIO, INPUT);
}

/*
 * SMI/MDIO/MIIM read
 *  @phy: PHY id.
 *  @reg: Register address.
 */
uint16_t read_smi(uint8_t phy, uint8_t reg)
{
    uint8_t byte;
    volatile uint16_t word, data;
    data = 0;

    /* MDIO pin is output */
    digitalWrite(DIR, OUT);
    pinMode(MDIO, OUTPUT);

    digitalWrite(MDIO, 1);
    digitalWrite(MDC, 1);
    for (byte = 0; byte < 32; byte++)
        pulse_mdc();

    /* Stat code */
    digitalWrite(MDIO, 0);
    pulse_mdc();
    digitalWrite(MDIO, 1);
    pulse_mdc();

    /* Read OP Code */
    digitalWrite(MDIO, 1);
    pulse_mdc();
    digitalWrite(MDIO, 0);
    pulse_mdc();

    /* PHY address - 5 bits */
    for (byte = 0x10; byte != 0; ) {
        if (byte & phy) {
            digitalWrite(MDIO, 1);
            pulse_mdc();
        } else {
            digitalWrite(MDIO, 0);
            pulse_mdc();
        }
        byte = byte >> 1;
    }

    /* REG address - 5 bits */
    for (byte = 0x10; byte != 0; ){
        if (byte & reg){
            digitalWrite(MDIO, 1);
            pulse_mdc();
        }else{
            digitalWrite(MDIO, 0);
            pulse_mdc();
        }
        byte = byte >> 1;
    }

    /* Turnaround bits */

    /* MDIO now is input */
    digitalWrite(DIR, IN);
    pinMode(MDIO, INPUT);
    pinMode(MDC, OUTPUT);
    pulse_mdc();
    pulse_mdc();

    /* Data - 16 bits */
    for(word = 0x8000; word != 0; ) {

        if (digitalRead(MDIO)) {
            data |= word;
        }
        pulse_mdc();
        word = word >> 1;
    }

    /* This is needed for some reason... */
    pulse_mdc();
    /* Stay in 0 state */
    //MDC = 0;
    digitalWrite(DIR, IN);
    pinMode(MDIO, INPUT);

    return data;
}

/* setup entence */
void setup()
{
    uint8_t phy, reg, val, i, sel;
    String inStr = "";
    uint16_t reg_val = 0;
  
    /* start serial port at 9600 bps: */
    Serial.begin(9600);
    Serial.print("MDIO (SMI/MIIM) Initialization ....\n");
    smi_init();
    for(;;){
        Serial.print("===========================================\r\n");
        Serial.print("Arduino MDIO (SMI/MIIM) Bus tools\r\n");
        Serial.print("1. Read register\r\n");
        Serial.print("2. Write register\r\n");
        Serial.print("3. Dump register\r\n");

        Serial.setTimeout(100000);
        sel = Serial.parseInt();

        switch(sel) {
        case 1:
            Serial.print("Read-PHY: ");
            Serial.setTimeout(100000);
            phy = Serial.parseInt();
            Serial.print(phy);
            if (phy > 31) 
                break;
            Serial.print(" Register: ");
            Serial.setTimeout(100000);
            reg = Serial.parseInt();
            Serial.print(reg);
            if (reg > 31) 
                break;
            reg_val = read_smi(phy, reg);
            Serial.print("\n\r\n\rRead-PHY: ");
            Serial.print(phy);
            Serial.print(" Register: ");
            Serial.print(reg);
            Serial.print(" Value [0x");
            Serial.print(reg_val, HEX);
            Serial.print("]\r\n\r\n");
            break;
        case 2:
            Serial.print("Write-PHY: ");
            Serial.setTimeout(100000);
            phy = Serial.parseInt();
            Serial.print(phy);
            if (phy > 31) 
                break;
            Serial.print(" Register: ");
            Serial.setTimeout(100000);
            reg = Serial.parseInt();
            Serial.print(reg);
            if (reg > 31) 
                break;
            Serial.print(" Value: ");
            reg_val = Serial.parseInt();
            Serial.print(reg_val, HEX);
            write_smi(phy, reg, reg_val);
            Serial.print("\n\r\n\rWrite Port: ");
            Serial.print(phy);
            Serial.print(" Register: ");
            Serial.print(reg);
            Serial.print(" Value [0x");
            Serial.print(reg_val, HEX);
            Serial.print("]\r\n\r\n");
            break;
        case 3:
            Serial.print("Dump Start PHY: ");
            phy = Serial.parseInt();
            Serial.print(phy);
            Serial.print(" End PHY: ");
            Serial.setTimeout(100000);
            i = Serial.parseInt();
            Serial.print(i);
            Serial.print("\r\n");
            for (; phy < i; phy++) {
                Serial.print("/*0x");
                Serial.print(phy, HEX);
                Serial.print("*/");
                Serial.print("{");
                for (reg = 0; reg < 32; reg++) {
                    Serial.print("0x");
                    reg_val = read_smi(phy, reg);
                    Serial.print((reg_val & 0xffff), HEX);
                    if (reg < 31)
                        Serial.print(",");
                    delay(10);
                }
                Serial.print("}\r\n");
            }
            break;
        default:
            Serial.print("input wrong\r\n");
            break;
        }
    }
}

/* loop entence */
void loop()
{
}
