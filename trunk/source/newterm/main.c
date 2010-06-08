/*
 * This file is part of the Bus Pirate project (http://code.google.com/p/the-bus-pirate/).
 *
 * Written and maintained by the Bus Pirate project.
 *
 * To the extent possible under law, the project has
 * waived all copyright and related or neighboring rights to Bus Pirate. This
 * work is published from United States.
 *
 * For details see: http://creativecommons.org/publicdomain/zero/1.0/.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "base.h"
#include "busPirateCore.h"
#include "procMenu.h"
//#include "procSyntax.h"
#include "selftest.h"
//#include "binIO.h"
//#include "SUMP.h"

//set custom configuration for PIC 24F (now always set in bootloader page, not needed here)
//_CONFIG2(FNOSC_FRCPLL & OSCIOFNC_ON &POSCMOD_NONE & I2C1SEL_PRI)		// Internal FRC OSC = 8MHz
//_CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx1) //turn off junk we don't need

unsigned char irqFlag=0;
void _T1Interrupt(void);
void ISRTable(); //Pseudo function to hold ISR remap jump table

struct _bpConfig bpConfig; //holds persistant bus pirate settings (see buspiratecore.h)
struct _modeConfig modeConfig; //holds mode info, cleared between modes
struct _command bpCommand; //holds the current active command so we don't ahve to put so many variables on the stack

void Initialize(void);

unsigned char binmodecnt=0;//, terminalInput[TERMINAL_BUFFER];
unsigned int currentByte;

#pragma code
//this loop services user input and passes it to be processed on <enter>






int main(void){

	Initialize();//setup bus pirate

	serviceuser();

/*
	while(1){ //this is the main bus pirate loop
		if(U1STAbits.OERR) U1STA &= (~0b10); //clear overrun error if exists

		switch(bpGetUserInput(&currentByte, TERMINAL_BUFFER, bpConfig.terminalInput)){//service user prompt in baseIO.c
			case 0x01://got enter, process user input
				switch(currentByte){
					case 0://no bytes, error
						bpWmessage(MSG_ERROR_SYNTAX);
						break;
					case 1://1 byte, try to process as a menu option
						if(checkMenuCommand(bpConfig.terminalInput[0]))break;
					default://multiple bytes, process as syntax
						processSyntaxString(currentByte, bpConfig.terminalInput);//process a syntax string
				}
				currentByte=0;
				binmodecnt=0; //reset any null characters
				bpEchoCurrentBusMode(); //print the bus mode
				UART1TX('>');//echo prompt
				break;
			case 0xff://got rawIO mode trigger
				binmodecnt++;
				if(binmodecnt>19){
					binBB();//hand control to binary mode service loop. resume as normal on return
					//binmodecnt=0; //this line is unreachable, exit with hardware reset
				}
				break;
			case 0xfe:
				if(binmodecnt>=5) SUMP();
				binmodecnt=0;
				break;	
		}

		//send the periodic service command to the current protocol
		//allows to check UART for async RX bytes, etc, independent of user input
		if(modeConfig.periodicService==1){
			bpCommand.cmd=CMD_PERIODIC_SERVICE;
			bpProcess();
		}


	}//while

	*/


	return 0;
}

//bus pirate initialization
//setup clock, terminal UART, pins, LEDs, and display version info
void Initialize(void){

	CLKDIVbits.RCDIV0=0; //clock divider to 0
	AD1PCFG = 0xFFFF;                 // Default all pins to digital
	OSCCONbits.SOSCEN=0;	

	//set pin configuration using peripheral pin select
	BP_TERM_RX=BP_TERM_RX_RP;   //Inputs UART1 RX RPINR18bits.U1RXR=4;
	BP_TERM_TX_RP=BP_TERM_TX; 		// Outputs UART1 TX RPOR1bits.RP3R=U1TX_IO;
 
	//put startup values in config (do first)
	bpConfig.termSpeed=8;//default PC side port speed, startup in 115200, or saved state (later)....
	bpConfig.displayMode=HEX;

	bpInit();//put startup values in config (do first)clean up, exit in HI-Z
		
	InitializeUART1(); //init the PC side serial port

	// Get the chip type and revision
	bpConfig.dev_type = bpReadFlash(DEV_ADDR_UPPER, DEV_ADDR_TYPE);
	bpConfig.dev_rev = bpReadFlash(DEV_ADDR_UPPER, DEV_ADDR_REV);

	bpConfig.quiet=0;		// turn output on (default)

	bpWBR; 	//send a line feed

	TBLPAG=0; // we need to be in page 0 (somehow this isn't set)

	versionInfo();//prints hardware and firmware version info (base.c)

//	bpEchoCurrentBusMode(); //print the bus mode
//	UART1TX('>');//prompt

}

//Interrupt Remap method 1:  Using direct interrupt address
/*void __attribute__ ((interrupt,address(0xF00), no_auto_psv)) _T1Interrupt(){
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 0;
	PR1 = 0xFFFF;
	T1CON = 0;
	irqFlag=1;
	
}
*/

//Interrupt Remap method 2:  Using Goto and jump table
/*
void __attribute__ ((interrupt,no_auto_psv)) _T1Interrupt(){
	IFS0bits.T1IF = 0;
	IEC0bits.T1IE = 0;
	PR1 = 0xFFFF;
	T1CON = 0;
	irqFlag=1;
	
}
*/

/*
 *	ISR JUMP TABLE
 *
 *	It is necessary to define jump table as a function because C30 will
 *	not store 24-bit wide values in program memory as variables.
 *
 *	This function should be stored at an address where the goto instructions 
 *	line up with the remapped vectors from the bootloader's linker script.
*//*
void __attribute__ ((address(0x1000))) ISRTable(){

	asm("reset"); //reset instruction to prevent runaway code
	asm("goto %0"::"i"(&_T1Interrupt));  //T2Interrupt's address
} 
*/