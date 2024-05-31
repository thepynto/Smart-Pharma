/* Counting pulses from PTC12.
* This is used as the base for P5_10.
* This program uses TPM0 to count the number of pulses
* from PTC12.
* The tri-color LEDs are used to display bit2-0 of
* the counter. At low frequency input, the change of
* LED color should be visible.
* Although the counter is counting pulses from PTC12,
* timer counter clock must be present.
*/

#include <MKL25Z4.H>
#include <stdio.h>

#define RS 0x04 /* PTA2 mask */
#define RW 0x10 /* PTA4 mask */
#define EN 0x20 /* PTA5 mask */

void delayMs(int n);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_init(void);

int main (void) {
unsigned short count;

LCD_init();
/* Initialize GPIO pins for tri-color LEDs */
SIM->SCGC5 |= 0x400; /* enable clock to Port B */
SIM->SCGC5 |= 0x1000; /* enable clock to Port D */
PORTB->PCR[18] = 0x100; /* make PTB18 pin as GPIO */
PTB->PDDR |= 0x40000; /* make PTB18 as output pin */
PORTB->PCR[19] = 0x100; /* make PTB19 pin as GPIO */
PTB->PDDR |= 0x80000; /* make PTB19 as output pin */
PORTD->PCR[1] = 0x100; /* make PTD1 pin as GPIO */
PTD->PDDR |= 0x02; /* make PTD1 as output pin */
/* end GPIO pin initialization for LEDs */


/* Start of Timer code */
SIM->SCGC5 |= 0x0800; /* enable clock to Port C */
PORTC->PCR[12] = 0x400; /* set PTC12 pin for TPM0 */
/* use TPM_CLKIN0 as timer counter clock */
SIM->SOPT4 &= ~0x01000000;
SIM->SCGC6 |= 0x01000000; /* enable clock to TPM0 */

/* counter clock must be present */
SIM->SOPT2 |= 0x01000000;
TPM0->SC = 0; /* disable timer while configuring */
TPM0->SC = 0x80; /* prescaler /1 and clear TOF */
TPM0->MOD = 0xFFFF; /* max modulo value */
TPM0->CNT = 0; /* clear counter */
TPM0->SC |= 0x10; /* enable timer and use LPTPM_EXTCLK */


while (1) {
LCD_command(1); /* clear display */
delayMs(1);
LCD_command(0x80); /* set cursor at first line */
delayMs(1);

char *message = "Cont: ";
int i = 0;
while(message[i] != '\0') {
	LCD_data(message[i]);
	i++;
}
delayMs(1);

count = TPM0->CNT;
char text[5]; // Variable para almacenar el texto formateado

// Convertir el número entero a texto usando sprintf()
sprintf(text, "%d", count);

// Mostrar el texto en la pantalla LCD
i = 0;
while(text[i] != '\0') {
	LCD_data(text[i]);
	i++;
}
delayMs(500);
}
}

void LCD_init(void)
{
SIM->SCGC5 |= 0x1000; /* enable clock to Port D */
PORTD->PCR[0] = 0x100; /* make PTD0 pin as GPIO */
PORTD->PCR[1] = 0x100; /* make PTD1 pin as GPIO */
PORTD->PCR[2] = 0x100; /* make PTD2 pin as GPIO */
PORTD->PCR[3] = 0x100; /* make PTD3 pin as GPIO */
PORTD->PCR[4] = 0x100; /* make PTD4 pin as GPIO */
PORTD->PCR[5] = 0x100; /* make PTD5 pin as GPIO */
PORTD->PCR[6] = 0x100; /* make PTD6 pin as GPIO */
PORTD->PCR[7] = 0x100; /* make PTD7 pin as GPIO */
PTD->PDDR = 0xFF; /* make PTD7-0 as output pins */
SIM->SCGC5 |= 0x0200; /* enable clock to Port A */
PORTA->PCR[2] = 0x100; /* make PTA2 pin as GPIO */
PORTA->PCR[4] = 0x100; /* make PTA4 pin as GPIO */
PORTA->PCR[5] = 0x100; /* make PTA5 pin as GPIO */
PTA->PDDR |= 0x34; /* make PTA5, 4, 2 as out pins*/

delayMs(30); /* initialization sequence */

LCD_command(0x30);
delayMs(10);
LCD_command(0x30);
delayMs(1);
LCD_command(0x30);
/* set 8-bit data, 2-line, 5x7 font */
LCD_command(0x38);
/* move cursor right */
LCD_command(0x06);
/* clear screen, move cursor to home */
LCD_command(0x01);
/* turn on display, cursor blinking */
LCD_command(0x0F);}

void LCD_command(unsigned char command)
{
PTA->PCOR = RS | RW; /* RS = 0, R/W = 0 */
PTD->PDOR = command;
PTA->PSOR = EN; /* pulse E */

delayMs(0);
PTA->PCOR = EN;

if (command < 4)
delayMs(4); /* command 1 and 2 needs up to 1.64ms */
else
delayMs(1); /* all others 40 us */
}

void LCD_data(unsigned char data)
{
PTA->PSOR = RS; /* RS = 1, R/W = 0 */

PTA->PCOR = RW;

PTD->PDOR = data;

PTA->PSOR = EN; /* pulse E */

delayMs(0);
PTA->PCOR = EN;
delayMs(1);
}

void delayMs(int n){
int i;
int j;
for(i = 0 ; i < n; i++)
for (j = 0; j < 7000; j++) {}
}
