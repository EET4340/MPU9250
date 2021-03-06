#include <xc.h>
#include "MPU9250.h"

unsigned long tickCount;

int pic18_i2c_enable(void) {
    TRISD |= 0b01100000; //MMSP2 uses RD5 as SDA, RD6 as SCL, both set as inputs
    SSP2ADD = 19; //400kHz
    SSP2CON1bits.SSPM = 0b1000; //I2C Master mode
    SSP2CON1bits.SSPEN = 1; //Enable MSSP
    return 0;
}

int pic18_i2c_disable(void) {
    SSP2CON1bits.SSPEN = 0; //Disable MSSP
    return 0;
}

int pic18_i2c_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char const *data) {
    SSP2CON2bits.SEN = 1;
    while (SSP2CON2bits.SEN == 1);
    SSP2BUF = slave_addr << 1;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    if (SSP2CON2bits.ACKSTAT == 1) {
        SSP2CON2bits.PEN = 1;
        while (SSP2CON2bits.PEN == 1);
        return -1;
    }
    SSP2BUF = reg_addr;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    while (length > 0) {
        SSP2BUF = *data;
        while (SSP2STATbits.BF || SSP2STATbits.R_W);
        --length;
        ++data;
    }
    SSP2CON2bits.PEN = 1;
    while (SSP2CON2bits.PEN == 1);
    return 0;
}

int pic18_i2c_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data) {
    SSP2CON2bits.SEN = 1;
    while (SSP2CON2bits.SEN == 1);
    SSP2BUF = slave_addr << 1;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    if (SSP2CON2bits.ACKSTAT == 1) {
        SSP2CON2bits.PEN = 1;
        while (SSP2CON2bits.PEN == 1);
        return -1;
    }
    SSP2BUF = reg_addr;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    SSP2CON2bits.RSEN = 1;
    while (SSP2CON2bits.RSEN == 1);
    SSP2BUF = (slave_addr << 1) + 1; //address with R/W set for read
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    while (length > 0) {
        SSP2CON2bits.RCEN = 1;
        while (!SSP2STATbits.BF);
        *data = SSP2BUF;
        if (length > 1) {
            SSP2CON2bits.ACKDT = 0;
        } else {
            SSP2CON2bits.ACKDT = 1;
        }
        SSP2CON2bits.ACKEN = 1;
        while (SSP2CON2bits.ACKEN != 0);
        --length;
        ++data;
    }
    SSP2CON2bits.PEN = 1;
    while (SSP2CON2bits.PEN == 1);
    return 0;
}

int pic18_delay_ms(unsigned long num_ms) {
    while (num_ms > 0) {
        __delay_ms(1);
        --num_ms;
    }
    return 0;
}

int pic18_get_ms(unsigned long *count) {
    INTCONbits.GIE = 0;
    *count = tickCount;
    INTCONbits.GIE = 1;
    return 0;
}

void mpuReset(void) {
    unsigned char reg;
    reg = 0x80;
    pic18_i2c_write(0x68, 107, 1, &reg); //Reset
    __delay_ms(200);
}