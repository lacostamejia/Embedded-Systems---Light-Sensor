//Completed
//Luis Andres Acosta Mejia UCFID:5321663
//Reading the values from the light sensor.

#include <msp430fr6989.h>
#define FLAGS UCA1IFG // Contains the transmit & receive flags
#define RXFLAG UCRXIFG // Receive flag
#define TXFLAG UCTXIFG // Transmit flag
#define TXBUFFER UCA1TXBUF // Transmit buffer
#define RXBUFFER UCA1RXBUF // Receive buffer

/**
 * main.c
 */

// Configure eUSCI in I2C master mode


int i2c_read_word(unsigned char i2c_address, unsigned char i2c_reg, unsigned int * data);

int i2c_write_word(unsigned char i2c_address, unsigned char i2c_reg, unsigned int data);

void Initialize_I2C(void) {

// Enter reset state before the configuration starts...
UCB1CTLW0 |= UCSWRST;

// Divert pins to I2C functionality
P4SEL1 |= (BIT1|BIT0);
P4SEL0 &= ~(BIT1|BIT0);

// Keep all the default values except the fields below...

// (UCMode 3:I2C) (Master Mode) (UCSSEL 1:ACLK, 2,3:SMCLK)
UCB1CTLW0 |= UCMODE_3 | UCMST | UCSSEL_3;

// Clock divider = 8 (SMCLK @ 1.048 MHz / 8 = 131 KHz)
UCB1BRW = 8;

// Exit the reset mode
UCB1CTLW0 &= ~UCSWRST;
}


void Initialize_UART(void){

// Divert pins to UART functionality
P3SEL1 &= ~(BIT4|BIT5);
P3SEL0 |= (BIT4|BIT5);

// Use SMCLK clock; leave other settings default
UCA1CTLW0 |= UCSSEL_2;


// Configure the clock dividers and modulators
// UCBR=6, UCBRF=13, UCBRS=0x22, UCOS16=1 (oversampling)

UCA1BRW = 6;
UCA1MCTLW = UCBRS5|UCBRS1|UCBRF3|UCBRF2|UCBRF0|UCOS16;

// Exit the reset state (so transmission/reception can begin)
UCA1CTLW0 &= ~UCSWRST;

}

void uart_write_char(unsigned char ch){

// Wait for any ongoing transmission to complete
while ( (FLAGS & TXFLAG)==0 ) {}

// Write the byte to the transmit buffer
TXBUFFER = ch;
}

void uart_write_uint16(unsigned int n){

    int digit;

    //65535
    if(n>= 10000){
        digit = (n/10000) % 10; // 6
        uart_write_char('0' + digit);
    }
    if(n>=1000){
        digit = (n/1000) % 10;
        uart_write_char('0' + digit); // 5

    }
    if(n>= 100){
        digit = (n/100) % 10;
        uart_write_char('0' + digit); // 5
    }

    if(n>= 10){
        digit = (n/10) % 10;
        uart_write_char('0' + digit); // 3
    }
    digit = n % 10;
    uart_write_char('0' + digit); // 5
    return;
}
// The function returns the byte; if none received, returns NULL

unsigned char uart_read_char(void){

unsigned char temp;

// Return NULL if no byte received
    if((FLAGS & RXFLAG) == 0)
    {
        return '\0';
    }

// Otherwise, copy the received byte (clears the flag) and return it
temp = RXBUFFER;
return temp;

}

//**********************************
// Configures ACLK to 32 KHz crystal
void config_ACLK_to_32KHz_crystal() {

// By default, ACLK runs on LFMODCLK at 5MHz/128 = 39 KHz

// Reroute pins to LFXIN/LFXOUT functionality
    PJSEL1 &= ~BIT4;
    PJSEL0 |= BIT4;

// Wait until the oscillator fault flags remain cleared
    CSCTL0 = CSKEY; // Unlock CS registers

    do{
        CSCTL5 &= ~LFXTOFFG; // Local fault flag
        SFRIFG1 &= ~OFIFG; // Global fault flag
    }while((CSCTL5 & LFXTOFFG) != 0);

    CSCTL0_H = 0; // Lock CS registers

    return;
}




int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5; // Enable the GPIO pin

    Initialize_I2C();
    Initialize_UART();
    unsigned int i;
    unsigned int counter = 0; //65535

    config_ACLK_to_32KHz_crystal();
    TA0CCR0 = (32768 - 1);
    TA0CTL = TASSEL_1| ID_0 | MC_1 | TACLR; //UP Mode to have a 1s delay
    TA0CTL &= ~TAIFG;
    //TI OPT3001

    //For the light sensor
    //Configuration register = 0x01;

    //RN=0111b=7 The LSB bit is worth 1.28
   // CT=0 Result produced in 100 ms
   // M=11b=3 Continuous readings
    // ME=1 Mask (hide) the Exponent from the result

    //             0111 0110 0001 0100

    i2c_write_word(0x44,0x01,0x7614);



    for(;;){

    unsigned int data;

    // Empty while loop; waits here until TAIFG is raised
    while((TA0CTL & TAIFG) == 0) {}
    TA0CTL &= ~TAIFG; // Clear the flag

    uart_write_uint16(counter);
    uart_write_char(':');
    uart_write_char(' ');
    counter++;


    //Reading the result
    i2c_read_word(0x44,0x00,&data);
    uart_write_uint16(data); //I2C Address //(Register)Result ID

    uart_write_char('\n');
    uart_write_char('\r');

    }
}

////////////////////////////////////////////////////////////////////
// Read a word (2 bytes) from I2C (address, register)

int i2c_read_word(unsigned char i2c_address, unsigned char i2c_reg,
unsigned int * data) {

unsigned char byte1, byte2;
// Initialize the bytes to make sure data is received every time
byte1 = 111;
byte2 = 111;

//********** Write Frame #1 ***************************
UCB1I2CSA = i2c_address; // Set I2C address
UCB1IFG &= ~UCTXIFG0;
UCB1CTLW0 |= UCTR; // Master writes (R/W bit = Write)
UCB1CTLW0 |= UCTXSTT; // Initiate the Start Signal

while ((UCB1IFG & UCTXIFG0) ==0) {}

UCB1TXBUF = i2c_reg; // Byte = register address

while((UCB1CTLW0 & UCTXSTT)!=0) {}

if(( UCB1IFG & UCNACKIFG )!=0) return -1;

UCB1CTLW0 &= ~UCTR; // Master reads (R/W bit = Read)
UCB1CTLW0 |= UCTXSTT; // Initiate a repeated Start Signal

//****************************************************
//********** Read Frame #1 ***************************
while ( (UCB1IFG & UCRXIFG0) == 0) {}
byte1 = UCB1RXBUF;

//****************************************************
//********** Read Frame #2 ***************************
while((UCB1CTLW0 & UCTXSTT)!=0) {}

UCB1CTLW0 |= UCTXSTP; // Setup the Stop Signal
while ( (UCB1IFG & UCRXIFG0) == 0) {}

byte2 = UCB1RXBUF;
while ( (UCB1CTLW0 & UCTXSTP) != 0) {}

//****************************************************
// Merge the two received bytes
*data = ( (byte1 << 8) | (byte2 & 0xFF) );

return 0;

}

////////////////////////////////////////////////////////////////////
// Write a word (2 bytes) to I2C (address, register)
int i2c_write_word(unsigned char i2c_address, unsigned char i2c_reg,
unsigned int data) {

unsigned char byte1, byte2;

byte1 = (data >> 8) & 0xFF; // MSByte
byte2 = data & 0xFF; // LSByte

UCB1I2CSA = i2c_address; // Set I2C address

UCB1CTLW0 |= UCTR; // Master writes (R/W bit = Write)
UCB1CTLW0 |= UCTXSTT; // Initiate the Start Signal

while ((UCB1IFG & UCTXIFG0) ==0) {}

UCB1TXBUF = i2c_reg; // Byte = register address

while((UCB1CTLW0 & UCTXSTT)!=0) {}
//********** Write Byte #1 ***************************
UCB1TXBUF = byte1;

while ( (UCB1IFG & UCTXIFG0) == 0) {}
//********** Write Byte #2 ***************************
UCB1TXBUF = byte2;

while ( (UCB1IFG & UCTXIFG0) == 0) {}
UCB1CTLW0 |= UCTXSTP;

while ( (UCB1CTLW0 & UCTXSTP) != 0) {}

return 0;

}

