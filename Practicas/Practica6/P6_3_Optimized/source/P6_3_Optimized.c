#include <MKL25Z4.H>
#include <stdio.h> // Incluir la librería para sprintf()

#define RS 0x04
#define RW 0x10
#define EN 0x20

// Prototipos de funciones
void keypad_init(void);
char keypad_getkey(void);

void LCD_init(void);
void LCD_ready(void);
void LCD_command(unsigned char command);
void LCD_command_noWait(unsigned char command);
void LCD_data(unsigned char data);
void displayMessage(char* message);

void delayMs(int n);

void controlMotor(int dutyCycle, char* speedMessage);

void ADC0_init(void);

void button_init(void);
void PORTA_IRQHandler(void);

volatile int emergency_stop = 0; // Global declaration emergency signal

int main (void) {
    int mod = 43702;
    int DT0 = 0;
    int DT25 = mod / 4;
    int DT50 = mod / 2;
    int DT75 = mod / 4 * 3;
    int DT100 = mod;
    int result; // For ADC

    SIM->SCGC5 |= 0x1000; /* enable clock to Port D */
    PORTD->PCR[1] = 0x0400; /* PTD1 used by TPM0 */
    SIM->SCGC6 |= 0x01000000; /* enable clock to TPM0 */
    SIM->SOPT2 |= 0x01000000; /* use MCGFLLCLK as timer counter clock */

    TPM0->SC = 0; /* disable timer */
    TPM0->CONTROLS[1].CnSC = 0x20 | 0x08; /* edge-aligned, pulse high */
    TPM0->MOD = mod; /* Set up modulo register for 60 kHz */
    TPM0->SC = 0x0C; /* enable TPM0 with prescaler /16 */

    TPM0->CONTROLS[1].CnV = DT0; // Set up channel value for 0% dutycycle

    keypad_init();
	LCD_init();
	ADC0_init();
	button_init();
	LCD_command(0x01);
	delayMs(500);
	LCD_command(0x80);
	delayMs(500);

    while (1) {
        LCD_command(1); /* clear display */
        delayMs(50);
        LCD_command(0x80); /* set cursor at first line */
        delayMs(50);

        displayMessage(" Set Input Mode ");
        delayMs(50);
        LCD_command(0xC0); /* set cursor at second line */
        delayMs(50);
        displayMessage(" MD1: M  MD2: A ");
        delayMs(1000);

        char mode = keypad_getkey();
        delayMs(10);

        if (mode == '1' || mode == '2') {
		   LCD_command(1); /* clear display */
		   delayMs(50);
		   LCD_command(0x80); /* set cursor at first line */
		   delayMs(50);
		   displayMessage("     MODE ");
		   delayMs(100);

		   if (mode == '1') {
			   displayMessage("1");
		   } else {
			   displayMessage("2");
		   }

		   delayMs(100);
		   LCD_command(0xC0); /* set cursor at second line */
		   delayMs(50);
		   displayMessage("    PRESS  #    ");
		   delayMs(100);

		   int r = 1;
		   while (r) {
			   char ready = keypad_getkey();
			   if (ready == '#') {
				   r = 0;
			   }
		   }
	   }

	   delayMs(1000);

	   if (mode == '1') {
		   LCD_command(1); /* clear display */
		   delayMs(50);
		   LCD_command(0x80); /* set cursor at first line */
		   delayMs(50);
		   displayMessage("  Manual  Mode  ");
		   delayMs(1000);
		   LCD_command(1); /* clear display */
		   delayMs(50);
		   LCD_command(0x80); /* set cursor at first line */
		   delayMs(50);
		   displayMessage("  Select Speed  ");
		   delayMs(50);
		   LCD_command(0xC0); /* set cursor at second line */
		   delayMs(50);
		   displayMessage("1:L 2:M 3:MH 4:H");
		   delayMs(1000);

		   char key = keypad_getkey();
		   int correct = 1;

		   while (correct) {
			   if (key == '1' || key == '2' || key == '3' || key == '4') {
				   correct = 0;
			   }
		   }

		   switch (key) {
			   case '1':
				   controlMotor(DT25, "LOW SPEED 25%");
				   break;
			   case '2':
				   controlMotor(DT50, "MEDIUM SPEED 50%");
				   break;
			   case '3':
				   controlMotor(DT75, "MH SPEED 75%");
				   break;
			   case '4':
				   controlMotor(DT100, "HIGH SPEED 100%");
				   break;
			   default:
				   TPM0->CONTROLS[1].CnV = DT0; // Set up channel value for 0% dutycycle
				   break;
		   }
	   }
	   if (mode == '2') {
		   int max = 4096;
		   int min = 0;
		   int range = max-min;
		   int division = range / 4;

		   while (1) {
			   ADC0->SC1[0] = 0; /* start conversion on channel 0 */

			   while(!(ADC0->SC1[0] & 0x80)) { } /* wait COCO */
				   result = ADC0->R[0];
				   /* read conversion result and clear COCO flag */

				   char controlKey = keypad_getkey();
				   if (controlKey == 'A') {
					   break; // Salir del while
				   }

				   if (result >= min && result < min + division) {
					   LCD_command(1); /* clear display */
					   delayMs(50);
					   LCD_command(0x80); /* set cursor at first line */
					   delayMs(50);
					   displayMessage("LOW SPEED 25%");
					   delayMs(50);
					   LCD_command(0xC0); /* set cursor at second line */
					   delayMs(50);
					   displayMessage("A:RETURN TO MENU");
					   TPM0->CONTROLS[1].CnV = DT25;
					   delayMs(1000);
				   }
				   else if (result >= min + division && result < min + 2 * division) {
					   LCD_command(1); /* clear display */
					   delayMs(50);
					   LCD_command(0x80); /* set cursor at first line */
					   delayMs(50);
					   displayMessage("MEDIUM SPEED 50%");
					   delayMs(50);
					   LCD_command(0xC0); /* set cursor at second line */
					   delayMs(50);
					   displayMessage("A:RETURN TO MENU");
					   TPM0->CONTROLS[1].CnV = DT50;
					   delayMs(1000);
				   }
				   else if (result >= min + 2 * division && result < min + 3 * division) {
					   LCD_command(1); /* clear display */
					   delayMs(50);
					   LCD_command(0x80); /* set cursor at first line */
					   delayMs(50);
					   displayMessage("MH SPEED 75%");
					   delayMs(50);
					   LCD_command(0xC0); /* set cursor at second line */
					   delayMs(50);
					   displayMessage("A:RETURN TO MENU");
					   TPM0->CONTROLS[1].CnV = DT75;
					   delayMs(1000);
				   }
				   else if (result >= min + 3 * division && result < min + 4 * division) {
					   LCD_command(1); /* clear display */
					   delayMs(50);
					   LCD_command(0x80); /* set cursor at first line */
					   delayMs(50);
					   displayMessage("HIGH SPEED 100%");
					   delayMs(50);
					   LCD_command(0xC0); /* set cursor at second line */
					   delayMs(50);
					   displayMessage("A:RETURN TO MENU");
					   TPM0->CONTROLS[1].CnV = DT100;
					   delayMs(1000);
				   }
				   else {
					   LCD_command(1); /* clear display */
					   delayMs(50);
					   LCD_command(0x80); /* set cursor at first line */
					   delayMs(50);
					   displayMessage("OUT OF BOUNDS");
					   delayMs(50);
					   LCD_command(0xC0); /* set cursor at second line */
					   delayMs(50);
					   displayMessage("A:RETURN TO MENU");
					   TPM0->CONTROLS[1].CnV = DT0; // Set up channel value for 0% dutycycle
					   delayMs(1000);
					   // En realidad no puede existir este
				   }
		   }

	   }
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

void controlMotor(int dutyCycle, char* speedMessage) {
    int mark = 0;
    while (1) {
        if (mark) {
            TPM0->CONTROLS[1].CnV = 0; // Shut down the motor
        }
        TPM0->CONTROLS[1].CnV = dutyCycle; // Set up channel value for the given duty cycle
        LCD_command(1); /* clear display */
        delayMs(50);
        LCD_command(0x80); /* set cursor at first line */
        delayMs(50);
        displayMessage(speedMessage);
        delayMs(50);
        LCD_command(0xC0); /* set cursor at second line */
	    delayMs(50);
	    displayMessage("A:Return   B:Off");
	    delayMs(1000);
        char controlKey = keypad_getkey();
        if (controlKey == 'A' || controlKey == 'B') {
            switch (controlKey) {
                case 'A':
                    break;
                case 'B':
                    mark = 1; // Signal to shut down the motor
                    TPM0->CONTROLS[1].CnV = 0; // Shut down the motor
            }
        }
        if (controlKey == 'A') {
            break; // Salir del while
        }
    }
}

void ADC0_init(void) {
	SIM->SCGC5 |= 0x2000; /* clock to PORTE */
	PORTE->PCR[20] = 0; /* PTE20 analog input */
	SIM->SCGC6 |= 0x8000000; /* clock to ADC0 */
	ADC0->SC2 &= ~0x40; /* software trigger */

	/* clock div by 4, long sample time, single ended 12 bit, bus clock */
	ADC0->CFG1 = 0x40 | 0x10 | 0x04 | 0x00;
}


void button_init(void) {
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
    	LCD_command(1); /* clear display */
		delayMs(50);
		LCD_command(0x80); /* set cursor at first line */
		delayMs(50);
    	displayMessage(" EMERGENCY STOP ");
    	delayMs(100);
    	NVIC_SystemReset();
    }
}

void displayMessage(char* message) {
    int i = 0;
    while (message[i] != '\0') {
        LCD_data(message[i]);
        delayMs(1); // Se usa para el modo de 4 bits
        i++;
    }
}

void delayMs(int n) {
    int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < 7000; j++) {}
}
