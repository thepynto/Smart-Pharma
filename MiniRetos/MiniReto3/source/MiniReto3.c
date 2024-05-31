/*  Initialize and display “Hello” on the LCD using 8-bit data mode.

* Data pins use Port D, control pins use Port A.
* This program does not poll the status of the LCD.
* It uses delay to wait out the time LCD controller is busy.

* Timing is more relax than the HD44780 datasheet to accommodate the variations among the LCD modules.

* You may want to adjust the amount of delay for your LCD controller. */

#include <MKL25Z4.H>
#include <stdio.h> // Incluir la librería para sprintf()

#define RS 0x04 /* PTA2 mask */
#define RW 0x10 /* PTA4 mask */
#define EN 0x20 /* PTA5 mask */

void delayMs(int n);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_init(void);

void ADC0_init(void);

int main(void) {
	short int result;
	short int temperature;
	short int refV = 885; // Voltaje de referencia para los 24 grados

    LCD_init();
    ADC0_init(); /* Configure ADC0 */

    while(1) {
        LCD_command(1); /* clear display */
        delayMs(50);
        LCD_command(0x80); /* set cursor at first line */
        delayMs(50);

        char *message = "Temp: ";
		int i = 0;
		while(message[i] != '\0') {
			LCD_data(message[i]);
			i++;
		}

		delayMs(100);

        ADC0->SC1[0] = 26; /* start conversion on channel 26 temperature */
		while(!(ADC0->SC1[0] & 0x80)) { } /* wait for COCO */
		result = ADC0->R[0]; /* read conversion result and clear COCO flag */

		if (result >= refV){
			temperature = 24 - ((result - refV)/ 1.769);
		}

		else{
			temperature = 24 - ((result - refV)/ 1.646);
		}

        //int number = 123; // El número que deseas mostrar
        char text[3]; // Variable para almacenar el texto formateado

        // Convertir el número entero a texto usando sprintf()
        sprintf(text, "%d", temperature);

        // Mostrar el texto en la pantalla LCD
        i = 0;
        while(text[i] != '\0') {
            LCD_data(text[i]);
            i++;
        }
        delayMs(100);

        char *message2 = " Grados";
        i = 0;
		while(message2[i] != '\0') {
			LCD_data(message2[i]);
			i++;
		}

        delayMs(1500);
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

void ADC0_init(void)
{
SIM->SCGC6 |= 0x8000000; /* clock to ADC0 */
ADC0->SC2 &= ~0x40; /* software trigger */
/*CLKDIV/4, LS time, single ended 12 bit, bus clock */
ADC0->CFG1 = 0x40 | 0x10 | 0x04 | 0x00;
}

void delayMs(int n){
int i;
int j;
for(i = 0 ; i < n; i++)
for (j = 0; j < 7000; j++) {}
}
