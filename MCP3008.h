#include <msp430.h>
/*
 * MCP3008 uses SPI to send serial data to the main device
 * it uses 4 pins to communicate with the main device
 *      | MCP..SCLK|<-------|MAIN|
 *      |       SDO|------->|    |
 *      |       SDI|<-------|    |
 *      |        CS|<-------|    |
 */
//see table 5-1 from datasheet for initialization info
//0x11 is 5 beginning bits that should be clocked off the the LSB side of the BYTE
//0x01 is the Start bit
char STRTSeq = 0x03;
//this struct is used for ease of changing pinouts throughout the code.
struct SPIModule{
    char SCLKPin;
    char SDOPin;
    char SDIPin;
    char CSPin;
};
//could include channel selection in this
void initializeCh(struct SPIModule mod){
    int i = 0;
    //make sure chip is selected corrctley
    P2OUT|= mod.CSPin;
    P2OUT &=~ mod.CSPin;
    //send configuration information to the data input pin
    for(i = 0;i<5;i++){
        P2OUT &=~ mod.SCLKPin;
        //compare the LSB of the sequence to
        if((STRTSeq>>i) & 0x01){
            P2OUT |= mod.SDIPin;
        } else {
            P2OUT &=~ mod.SDIPin;
        }
        P2OUT|= mod.SCLKPin;
    }
    //extra clock cycle
    P2OUT&=~mod.SCLKPin;
    P2OUT|=mod.SCLKPin;
    P2OUT&=~mod.SCLKPin;
    //returning to the read function on the null bit
    return;
}
/*int test(int channel, struct SPIModule mod){
    //clock off some garbage data to make sure device is working properly
    if(/*condition: test succeeded){
        return 1;
    }else{
        return 0;
    }
}*/
int ReadADC(struct SPIModule mod){
    //could be useful to uninitialize the comm pins when outside of one of these functions.
    //research how long it takes to change these registers.
    int value;
    int i = 0;

    //send channel info to the MCP4008
    initializeCh(mod);
    //raise serial clock from 0 to 1 the next falling edge will clock off the MSB.
    P2OUT |= mod.SCLKPin;
    //clock bits into an int value
    for(i = 0;i<10;i++){
        P2OUT &=~ mod.SCLKPin;
        //setting the MSB
        if(P2IN & mod.SDOPin){
            //bitshifting all the way to the MSB for the first input.
            //b0 0 0 0 0 0 MSB 0 0 0 0 0 0 0 0 LSB
            value |= (0x01<<(9-i));
        }
        P2OUT |= mod.SCLKPin;
    }
    P2OUT&=~ mod.SCLKPin+mod.SDIPin;

    P2OUT |= mod.CSPin;
    //return the int

    return value;
}
