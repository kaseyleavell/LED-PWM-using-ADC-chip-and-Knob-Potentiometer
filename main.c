#include <msp430.h> 
#include <stdlib.h>
#include "MCP3008.h"
#define LED1 0x20
#define BUTTON 0x10
#define KNOB 0x08
#define INTERVAL 50000
/**
 * main.c
 * Using TI MSP430G2553 as master
 * Create a variable brightness LED with an External ADC reading the value of a knob potentiometer
 * The ADC chip is an MCP3008 which is polled via SPI
 * There is a button that when pushed will block all other code until it is pressed again
 */
//struct for ADC
struct analog{
    char firstByte;
    char secondbyte;
};
//instantiating SPI comms module
struct SPIModule module = {0x01,0x02,0x04,0x08};

int valADC = 0;
int buttonFlag;
int count = 0;
//functions
int pollButton();      //used for checking the button
int readADC();
void LEDWrapper();
void sleep(int multiplier);
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	//initialize registers
	P1DIR &=~ KNOB|BUTTON;
	P1DIR|=LED1;
	//initialize SPI iO registers
	P2DIR |= module.SCLKPin + module.CSPin + module.SDIPin;
	P2DIR &=~ module.SDOPin;
	//Initialize timer a interrupt
	TA0R = 0;
	TACCR0 = 0;
	TACCTL0 = CCIE;   //enables capture and compare registers for the timer
	TACCR0 = INTERVAL;   //this register holds the value the clock compares to
	TACTL = TASSEL_2 + MC_1;    //this establishes the subsystem master clock (SMCLK) as the clock being used. MC_1 is an up count.


	//Initialize port 1 vector interrupt
	P1IE |= BUTTON; // P1.4 interrupt enabled
	P1IES |= BUTTON; // P1.4 Hi/lo edge
	P1IFG &= ~BUTTON; // P1.4 IFG cleared
	//initialize the ADC
	//Didn't need the MSP430 ADC after the external chip was implimented.
	//Code is left in for reference
	/*ADC10CTL0 &=~ ENC;              //must disable ADC to change settings.
	ADC10CTL0  = ADC10ON +ADC10IE; // Vref Vr+=3v,Vr-=VSS,
	ADC10CTL1  = ADC10DIV_7+ INCH_3; // INCH =0000->A0,ADCCLK src = ADC10CLK,
	//ADC10AE0 &=~ 0xFFFF;
	ADC10AE0  = INCH_3;     // channel A0 selecting p1.1*/



	_BIS_SR(LPM0_bits + GIE);
	return 0;
}
//Timer A0 service routine
//At an increment determined by the function Macro "INTERVAL" This timer interrupt fires and polls the ADC
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A(void){
    TACCR0 = 0;     //unenabling the timer interrupt vector
    int temp = ReadADC(module);	//see "MCP3008.h" for the code regarding that chip
    //Only call this function if the button has been pressed
    if((!buttonFlag) && (count>temp)){
        LEDWrapper();   //xor the led
        count = 0;
    }
    else{
        count++;
    }
    TACCR0 = INTERVAL;
    return;
}
/*
 * This still feels clunky
 * doesn't work exactly every time
 */
//port 1 vector routine
//Button interrupt
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void){
    P1IE &=~BUTTON;
    P1IFG &=~BUTTON;
    //clear any button holding
    sleep(1000);
    buttonFlag^=1;
    sleep(1000);
    //reset interrupt
    P1IE |= BUTTON;
}
int pollButton(){
    //variables
    int value = 0;
    //function returns a 1 if the button connected to p1.4 is high or if it is low.
    if(BUTTON & P1IN){
        value = 1;
    }
    return value;
}
void LEDWrapper(){
    //set LED value to the value of sts.
    P1OUT ^=LED1;
    return;
}
void sleep(int multiplier){
    int j = 0;
    int i = 0;
    for(j = 0;j<multiplier;j++){
        for(i = 0;i<100;i++);
    }
}
