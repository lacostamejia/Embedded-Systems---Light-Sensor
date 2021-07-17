
/*
 *  UCF EEL4742C Summer 2021 Lab 9
 *  Copyright 2021 Luis Andres Acosta Mejia
 */

Light Sensor Booster pack MSP430FR6989

We are going to start reading the values captured from the light sensor (OPT3001) by reading the result register (0x00), which contains the result of the most recent light to digital conversion; but first we are going to write to the configuration register(0x01), in order to make some changes for our output. We are going to configure the register with the configuration below, and we are going to find the hexadecimal value to be wrote to the configuration register depending on the values below.
RN=0111b=7 The LSB bit is worth 1.28 
CT=0 Result produced in 100 ms
 M=11b=3 Continuous readings 
ME=1 Mask (hide) the Exponent from the result
