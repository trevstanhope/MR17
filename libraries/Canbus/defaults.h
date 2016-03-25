#ifndef	DEFAULTS_H
#define	DEFAULTS_H

/* (Un)comment to select the board type */
#define UNO
//#define MEGA

/*
	P_MOSI 		--> 	B,2 on Mega2560; B,3 on atmega328
	P_MISO 		--> 	B,3 on Mega2560; B,4 on atmega328
	P_SCK  		-->		B,1 on Mega2560; B,5 on atmega328
	MCP2515_CS	-->		B,0 on Mega2560; B,2 on atmega328
	MCP2515_INT	-->		??? on Mega2560; D,2 on atmega328
	LED2_HIGH	-->		B,0 on Mega2560; B,0 on atmega328
	LED2_LOW	-->		B,0 on Mega2560; B,0 on atmega328
*/

#if defined(UNO)
	#define	P_MOSI				B,3
	#define	P_MISO				B,4
	#define	P_SCK				B,5
	#define	MCP2515_CS			B,2
	#define	MCP2515_INT			D,2
	#define LED2_HIGH			B,0
	#define LED2_LOW			B,0
#elif defined(MEGA)
	#define	P_MOSI				B,2
	#define	P_MISO				B,3
	#define	P_SCK				B,1
	#define	MCP2515_CS			B,0
	#define	MCP2515_INT			D,0 // Working on E,4 (Pin #2)
	#define LED2_HIGH			H,5
	#define LED2_LOW			H,5
#else
	#error "Modify defaults.h to select Arduino BOARD TYPE"
#endif

#endif	// DEFAULTS_H