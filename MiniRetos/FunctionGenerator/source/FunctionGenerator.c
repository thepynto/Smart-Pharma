#include <MKL25Z4.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RS 0x04
#define RW 0x10
#define EN 0x20

#define WAVEFORM_LENGTH 256

// Prototipos de funciones
void DAC0_init(void);

void delayUs(int n);
void delayMs(int n);
void delayPar(int n, float factor);

void keypad_init(void);
char keypad_getkey(void);

void LCD_data(unsigned char data);
void LCD_command_noWait(unsigned char command);
void LCD_command(unsigned char command);
void LCD_ready(void);
void LCD_init(void);

void displayMessage(char* message);

void button_init(void);
void PORTA_IRQHandler(void);

void generateSawtooth(float factor);
void generateSinewave(float factor);
void generateMaxMin(float factor);

int sinewave[WAVEFORM_LENGTH];
volatile int emergency_stop = 0; // Global declaration emergency signal

float scaler = 0.0; // Se manda al delay para que sea de más o de menos
float scalerComplex = 0.0;
int ms = 7000;
int second = 1000;
char frequencyText[5];

// Variables globales para el estado del modo
enum {MODE_SAWTOOTH, MODE_SINEWAVE, MODE_MAXMIN} mode = MODE_SAWTOOTH;

int main(void) {
    int i;
    int r = 1;
    int frequency = 0;

    char ready;

    float fRadians;
    const float M_PI = 3.1415926535897;

    // Inicializaciones
    keypad_init();
	LCD_init();
	DAC0_init();
	button_init();
	LCD_command(0x01);
	delayMs(500);
	LCD_command(0x80);
	delayMs(500);

	SIM->SCGC5 |= 0x400; /* enable clock to Port B */
	PORTB->PCR[19] = 0x100; /* make PTB19 pin as GPIO)*/
	PTB->PDDR |= 0x80000; /* make PTB19 as output pin */

    // Construir la tabla de datos para la onda sinusoidal
    fRadians = ((2 * M_PI) / WAVEFORM_LENGTH);
    for (i = 0; i < WAVEFORM_LENGTH; i++) {
        sinewave[i] = 2047 * (sinf(fRadians * i) + 1);
    }

    while (1) {
    	LCD_command(1); /* clear display */
		delayMs(50);
		LCD_command(0x80); /* set cursor at first line */
		delayMs(50);

		displayMessage(" Set Wave Form ");
		delayMs(50);
		LCD_command(0xC0); /* set cursor at second line */
		delayMs(50);
		displayMessage(" A:ST B:SN C:SQ ");
		delayMs(50);

		// Hasta que no se presione #, se queda en este bucle
	   while (r) {
		   ready = keypad_getkey();
		   if (ready == 'A' || ready == 'B' || ready == 'C') {
			   r = 0;
		   }
	   }

        switch (ready) {
            case 'A':
                mode = MODE_SAWTOOTH;
                break;
            case 'B':
                mode = MODE_SINEWAVE;
                break;
            case 'C':
                mode = MODE_MAXMIN;
                break;
            default:
                break;
        }

       LCD_command(1); /* clear display */
	   delayMs(50);
	   LCD_command(0x80); /* set cursor at first line */
	   delayMs(50);
	   displayMessage(" INTRODUCE FREQ ");
	   delayMs(100);

	   char input[6] = {0}; // Arreglo para almacenar hasta 5 dígitos + terminador nulo
	   // Es aquí donde se termina guardando el número, pero en char
	   int index = 0;

	   delayMs(100);
	   LCD_command(0xC0); /* set cursor at second line */
	   delayMs(50);

	   delayMs(100);
	   LCD_command(0xC0); /* set cursor at second line */
	   delayMs(50);

	   while (index < 5) {
		   char key = keypad_getkey();
		   if (key >= '0' && key <= '9') {
			   input[index] = key;
			   index++;
			   LCD_data(key); // Mostrar el dígito en el LCD
		   } else if (key == '#') {
			   break; // Finaliza la entrada si se presiona #
		   }
		   delayMs(100); // Pequeña demora para evitar múltiples registros
	   }

	   input[index] = '\0'; // Añadir el terminador nulo al final del arreglo

	   // Si tengo ganas agregar un if para cuando input está vacío

	   frequency = atoi(input); // Convertir la cadena de caracteres a un número entero

	   // Convertir el número entero a texto usando sprintf()
	   sprintf(frequencyText, "%d", frequency);

	   scaler = second / frequency; // Scaler para multiplicar por 700 en el delay

	   scalerComplex = (second / frequency) / 256;

	   // Hasta que no se presione #, se queda en este bucle
	   r = 1;
	   while (r) {
		   char ready = keypad_getkey();
		   if (ready == '#') {
			   r = 0;
		   }
	   }

	   delayMs(50);
	   displayMessage(" Hz");
	   delayMs(100);

        // Ejecutar el modo seleccionado
        switch (mode) {
            case MODE_SAWTOOTH:
                generateSawtooth(scaler);
                break;
            case MODE_SINEWAVE:
                generateSinewave(scaler);
                break;
            case MODE_MAXMIN:
                generateMaxMin(scaler); // Square
                break;
        }
    }
}

void generateSawtooth(float) {
	LCD_command(1); /* clear display */
	delayMs(50);
	LCD_command(0x80); /* set cursor at first line */
	delayMs(50);

	displayMessage("ACTUAL FUNCTION:");
	delayMs(50);
	LCD_command(0xC0); /* set cursor at second line */
	delayMs(50);
	displayMessage("SawTooth ");
	delayMs(50);
	displayMessage(frequencyText);
	delayMs(50);
	displayMessage("Hz");
	delayMs(50);

	while (1) {
    int i;
    for (i = 0; i < 0x1000; i += 0x0010) {
        DAC0->DAT[0].DATL = i & 0xff; /* write low byte */
        DAC0->DAT[0].DATH = (i >> 8) & 0x0f; /* write high byte */
        PTB->PTOR = 0x80000; /* turn on green LED */
        delayPar(1, scalerComplex);
    }
	}
}

void generateSinewave(float) {
	LCD_command(1); /* clear display */
	delayMs(50);
	LCD_command(0x80); /* set cursor at first line */
	delayMs(50);

	displayMessage("ACTUAL FUNCTION:");
	delayMs(50);
	LCD_command(0xC0); /* set cursor at second line */
	delayMs(50);
	displayMessage("Sine ");
	delayMs(50);
	displayMessage(frequencyText);
	delayMs(50);
	displayMessage("Hz");
	delayMs(50);

	while (1) {
    int i;
    for (i = 0; i < WAVEFORM_LENGTH; i++) {
        DAC0->DAT[0].DATL = sinewave[i] & 0xff; /* write low byte */
        DAC0->DAT[0].DATH = (sinewave[i] >> 8) & 0x0f; /* write high byte */
        PTB->PTOR = 0x80000; /* turn on green LED */
        delayPar(1, scalerComplex); /* delay 1ms */
    }
	}
}

void generateMaxMin(float) { // Square
	LCD_command(1); /* clear display */
	delayMs(50);
	LCD_command(0x80); /* set cursor at first line */
	delayMs(50);

	displayMessage("ACTUAL FUNCTION:");
	delayMs(50);
	LCD_command(0xC0); /* set cursor at second line */
	delayMs(50);
	displayMessage("Square ");
	delayMs(50);
	displayMessage(frequencyText);
	delayMs(50);
	displayMessage("Hz");
	delayMs(50);
	while (1) {
    // Set DAC output to maximum value
    DAC0->DAT[0].DATL = 0xFF; /* write low byte */
    DAC0->DAT[0].DATH = 0x0F; /* write high byte */
    PTB->PCOR = 0x80000; /* turn on green LED */
    delayPar(1, scaler); /* delay 128ms */

    // Set DAC output to minimum value
    DAC0->DAT[0].DATL = 0x00; /* write low byte */
    DAC0->DAT[0].DATH = 0x00; /* write high byte */
    PTB->PSOR = 0x80000; /* turn on green LED */
    delayPar(1, scaler); /* delay 128ms */
	}
}

void keypad_init(void) {
	SIM->SCGC5 |= 0x0800;  /* enable clock to Port C */
	PORTC->PCR[0] = 0x103; /* PTD0, GPIO, enable pullup*/
	PORTC->PCR[1] = 0x103; /* PTD1, GPIO, enable pullup*/
	PORTC->PCR[2] = 0x103; /* PTD2, GPIO, enable pullup*/
	PORTC->PCR[3] = 0x103; /* PTD3, GPIO, enable pullup*/
	PORTC->PCR[4] = 0x103; /* PTD4, GPIO, enable pullup*/
	PORTC->PCR[5] = 0x103; /* PTD5, GPIO, enable pullup*/
	PORTC->PCR[6] = 0x103; /* PTD6, GPIO, enable pullup*/
	PORTC->PCR[7] = 0x103; /* PTD7, GPIO, enable pullup*/
	PTC->PDDR = 0x0F; /* make PTD7-0 as input pins */
}

char keypad_getkey(void) {
	int row, col;
	const char row_select[] = {0x01, 0x02, 0x04, 0x08};
	/* one row is active */
	/* check to see any key pressed */

	PTC->PDDR |= 0x0F; /* enable all rows */
	PTC->PCOR = 0x0F;
	delayMs(2); /* wait for signal return */
	col = PTC-> PDIR & 0xF0; /* read all columns */
	PTC->PDDR = 0; /* disable all rows */
	if (col == 0xF0)
	return 0; /* no key pressed */
	/* If a key is pressed, we need find out which key.*/
	for (row = 0; row < 4; row++)
	{ PTC->PDDR = 0; /* disable all rows */

	PTC->PDDR |= row_select[row]; /* enable one row */
	PTC->PCOR = row_select[row]; /* drive active row low*/

	delayMs(2); /* wait for signal to settle */
	col = PTC->PDIR & 0xF0; /* read all columns */

	if (col != 0xF0) break;
	/* if one of the input is low, some key is pressed. */
	}

	PTC->PDDR = 0; /* disable all rows */

	if (row == 4)
	return 0; /* if we get here, no key is pressed */

	/* gets here when one of the rows has key pressed*/
	/*check which column it is*/

	if (col == 0xE0 && row == 0x00) return 0x31; /* key in column 0 */
	if (col == 0xE0 && row == 0x01) return 0x34; /* key in column 0 */
	if (col == 0xE0 && row == 0x02) return 0x37; /* key in column 0 */
	if (col == 0xE0 && row == 0x03) return 0x2A; /* key in column 0 */
	if (col == 0xD0 && row == 0x00) return 0x32; /* key in column 1 */
	if (col == 0xD0 && row == 0x01) return 0x35; /* key in column 1 */
	if (col == 0xD0 && row == 0x02) return 0x38; /* key in column 1 */
	if (col == 0xD0 && row == 0x03) return 0x30; /* key in column 1 */
	if (col == 0xB0 && row == 0x00) return 0x33; /* key in column 2 */
	if (col == 0xB0 && row == 0x01) return 0x36; /* key in column 2 */
	if (col == 0xB0 && row == 0x02) return 0x39; /* key in column 2 */
	if (col == 0xB0 && row == 0x03) return 0x23; /* key in column 2 */
	if (col == 0x70 && row == 0x00) return 0x41; /* key in column 3 */
	if (col == 0x70 && row == 0x01) return 0x42; /* key in column 3 */
	if (col == 0x70 && row == 0x02) return 0x43; /* key in column 3 */
	if (col == 0x70 && row == 0x03) return 0x44; /* key in column 3 */

	return 0; /* just to be safe */
}

void LCD_init(void) {
	SIM->SCGC5 |= 0x1000;

	PORTD->PCR[4] = 0x100;
	PORTD->PCR[5] = 0x100;
	PORTD->PCR[6] = 0x100;
	PORTD->PCR[7] = 0x100;
	PTD->PDDR = 0xFF;
	SIM->SCGC5 |= 0x0200;
	PORTA->PCR[2] = 0x100;
	PORTA->PCR[4] = 0x100;
	PORTA->PCR[5] = 0x100;
	PTA->PDDR = 0x34;

	/* initialization sequence*/
	delayMs(20);
	LCD_command_noWait(0x03);
	delayMs(5);
	LCD_command_noWait(0x03);
	delayMs(1);
	LCD_command_noWait(0x03);
	LCD_command_noWait(0x02); // 8 bits 2 lines

	//LCD_command(0x08);
	//LCD_command(0x01);

	delayMs(2);

	LCD_command(0x28);
	LCD_command(0x06); // move cursor right
	LCD_command(0x01); // clear cursor home
	delayMs(2);
	LCD_command(0x0F); // turn on display, blink display
}

void LCD_ready(void) {
	char status;
	PTD->PDDR = 0; // PTD input
	PTA->PCOR = RS; // RS 0 (command)
	PTA->PSOR = RW; // RW 1 (read)
	do { /* stay in the loop until it is not busy */
	PTA->PSOR = EN; /* raise E */
	delayMs(0);
	status = PTD->PDIR; /* read status register */
	PTA->PCOR = EN;
	delayMs(0); /* clear E */
	} while (status & 0x80); /* check busy bit */
	PTA->PCOR = RW; // RW 0 (write)
	PTD->PDDR = 0xFF; // PTD output
}

void LCD_command(unsigned char command) {
	LCD_ready(); /* wait until LCD is ready */
	PTA->PCOR = RS | RW; /* RS = 0, R/W = 0 */
	PTD->PDOR = (command & 0xF0);
	PTA->PSOR = EN; /* pulse E */
	delayMs(0);
	PTA->PCOR = EN;

	PTD->PDOR = ((command << 4) & 0xF0);
	PTA->PSOR = EN;
	delayMs(0);
	PTA->PCOR = EN;
}

void LCD_command_noWait(unsigned char command){
	PTA->PCOR = RS | RW; /* RS = 0, R/W = 0 */
	PTD->PDOR = (command & 0xF0);
	PTA->PSOR = EN; /* pulse E */
	delayMs(0);
	PTA->PCOR = EN;

	PTD->PDOR = ((command << 4) & 0xF0);
	PTA->PSOR = EN;
	delayMs(0);
	PTA->PCOR = EN;
}

void LCD_data(unsigned char data){
	LCD_ready(); /* wait until LCD is ready */
	PTA->PSOR = RS; /* RS = 1, R/W = 0 */
	PTA->PCOR = RW;

	PTD->PDOR = (data & 0xF0);
	PTA->PSOR = EN;
	delayMs(0);
	PTA->PCOR = EN;

	PTD->PDOR = ((data << 4) & 0xF0);
	PTA->PSOR = EN;
	delayMs(0);
	PTA->PCOR = EN;
}

void displayMessage(char* message) {
    int i = 0;
    while (message[i] != '\0') {
        LCD_data(message[i]);
        delayMs(1); // Se usa para el modo de 4 bits
        i++;
    }
}

// Este lo quiero cambiar para alguna tecla del teclado
void button_init(void) { // Se puede cambiar para hacer cualquier otra interrupción
    SIM->SCGC5 |= 0x0200;     // Habilitar el reloj para el puerto A
    PORTA->PCR[1] = 0x103;    // PTA1 como GPIO y habilitar pull-up
    PTA->PDDR &= ~0x02;       // PTA1 como entrada

    // Habilitar interrupción en el flanco de subida
    PORTA->PCR[1] |= 0xA0000; // ISF (Interrupción por flanco de subida) // 0x90000 para subida
    NVIC_EnableIRQ(PORTA_IRQn); // Habilitar interrupción en el NVIC
}

void PORTA_IRQHandler(void) {
    if (PORTA->ISFR & 0x02) { // Verificar si la interrupción es por PTA1
    	// Realizar el reinicio del microcontrolador
    	LCD_command(1); // clear display
		delayMs(50);
		LCD_command(0x80); // set cursor at first line
		delayMs(50);
    	displayMessage("      RESET     ");
    	delayMs(100);
    	NVIC_SystemReset();
    }
}

void DAC0_init(void) {
    SIM->SCGC6 |= 0x80000000; /* clock to DAC module */
    DAC0->C1 = 0; /* disable the use of buffer */
    DAC0->C0 = 0x80 | 0x20; /* enable DAC and use software trigger */
}

void delayUs(int n) {
    int i;
    int j;
    for(i = 0 ; i < n; i++)
        for (j = 0; j < 700; j++) {}
}

void delayMs(int n) {
    int i;
    int j;
    for(i = 0 ; i < n; i++)
        for (j = 0; j < 7000; j++) {}
}

void delayPar(int n, float factor) {
    int i;
    int j;
    for (i = 0; i < n; i++)
        for (j = 0; j < (ms*factor); j++) {}
}
