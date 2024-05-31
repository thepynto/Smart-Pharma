/*  Initialize and display “Hello” on the LCD using 8-bit data mode.

* Data pins use Port D, control pins use Port A.
* This program does not poll the status of the LCD.
* It uses delay to wait out the time LCD controller is busy.

* Timing is more relax than the HD44780 datasheet to accommodate the variations among the LCD modules.

* You may want to adjust the amount of delay for your LCD controller. */

#include <MKL25Z4.H>

#define RS 0x04 /* PTA2 mask */
#define RW 0x10 /* PTA4 mask */
#define EN 0x20 /* PTA5 mask */

// Define los pines GPIO correspondientes a los LEDs
#define RED_LED    0x40000   // PTB18
#define GREEN_LED  0x80000   // PTB19
#define BLUE_LED   0x02    // PTD1

void delayMs(int n);
void LCD_command(unsigned char command);
void LCD_data(unsigned char data);
void LCD_init(void);
void keypad_init(void);
char keypad_getkey(void);
void show_message(char key);
void LED_init(void);

int main(void) {
    LCD_init();
    keypad_init();
    LED_init();

    while(1) {
    	char key = keypad_getkey(); // Leer la tecla presionada
        LCD_command(1); /* clear display */
        delayMs(50);
        LCD_command(0x80); /* set cursor at first line */
        delayMs(50);

        if (key == '1' || key == '2' || key == '3') {
                    show_message(key); // Mostrar mensaje correspondiente en el LCD
                }
        delayMs(50);

        LCD_command(1); /* clear display */
        char *message = "  PRESS BUTTON  ";
        int i = 0;
        while(message[i] != '\0') {
            LCD_data(message[i]);
            i++;
        }

        delayMs(100);

        // Move cursor to the second line
        LCD_command(0xC0);
        delayMs(50);

        // Print on second line
        char *message2 = " R: 1 G: 2 B: 3 ";
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

void keypad_init(void)
{
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

if (col == 0xE0 && row == 0x00) return '1'; /* key in column 0 */
if (col == 0xE0 && row == 0x01) return '4'; /* key in column 0 */
if (col == 0xE0 && row == 0x02) return '7'; /* key in column 0 */
if (col == 0xE0 && row == 0x03) return '*'; /* key in column 0 */
if (col == 0xD0 && row == 0x00) return '2'; /* key in column 1 */
if (col == 0xD0 && row == 0x01) return '5'; /* key in column 1 */
if (col == 0xD0 && row == 0x02) return '8'; /* key in column 1 */
if (col == 0xD0 && row == 0x03) return '0'; /* key in column 1 */
if (col == 0xB0 && row == 0x00) return '3'; /* key in column 2 */
if (col == 0xB0 && row == 0x01) return '6'; /* key in column 2 */
if (col == 0xB0 && row == 0x02) return '9'; /* key in column 2 */
if (col == 0xB0 && row == 0x03) return '#'; /* key in column 2 */
if (col == 0x70 && row == 0x00) return 'A'; /* key in column 3 */
if (col == 0x70 && row == 0x01) return 'B'; /* key in column 3 */
if (col == 0x70 && row == 0x02) return 'C'; /* key in column 3 */
if (col == 0x70 && row == 0x03) return 'D'; /* key in column 3 */

return 0; /* just to be safe */
}

void show_message(char key) {
    LCD_command(1); /* Limpiar pantalla */
    delayMs(50);

    switch (key) {
        case '1':
            LCD_command(1); /* clear display */
            delayMs(50);
            LCD_command(0x80); /* set cursor at first line */
            delayMs(50);

            char *messagek1 = "      RED      ";
            int i = 0;
            while(messagek1[i] != '\0') {
                LCD_data(messagek1[i]);
                i++;
            }

            delayMs(100);

            // Move cursor to the second line
            LCD_command(0xC0);
            delayMs(50);

            // Print on second line
            char *messagek12 = "   LED IS ON!   ";
            i = 0;
            while(messagek12[i] != '\0') {
                LCD_data(messagek12[i]);
                i++;
            }

            // Prender el LED rojo
			PTB->PCOR = RED_LED;

			delayMs(2000);

			// Apagar el LED rojo
			PTB->PSOR = RED_LED;

            break;
        case '2':
            LCD_command(1); /* clear display */
            delayMs(50);
            LCD_command(0x80); /* set cursor at first line */
            delayMs(50);

            char *messagek2 = "     GREEN     ";
            i = 0; // Utilizamos la misma variable i aquí
            while(messagek2[i] != '\0') {
                LCD_data(messagek2[i]);
                i++;
            }

            delayMs(100);

            // Move cursor to the second line
            LCD_command(0xC0);
            delayMs(50);

            // Print on second line
            char *messagek22 = "   LED IS ON!   ";
            i = 0; // Utilizamos la misma variable i aquí
            while(messagek22[i] != '\0') {
                LCD_data(messagek22[i]);
                i++;
            }

            // Prender el LED verde
            PTB->PCOR = GREEN_LED;

            delayMs(2000);

            // Apagar el LED verde
            PTB->PSOR = GREEN_LED;

            break;
        case '3':
            LCD_command(1); /* clear display */
            delayMs(50);
            LCD_command(0x80); /* set cursor at first line */
            delayMs(50);

            char *messagek3 = "      BLUE      ";
            i = 0; // Utilizamos la misma variable i aquí
            while(messagek3[i] != '\0') {
                LCD_data(messagek3[i]);
                i++;
            }

            delayMs(100);

            // Move cursor to the second line
            LCD_command(0xC0);
            delayMs(50);

            // Print on second line
            char *messagek32 = "   LED IS ON!   ";
            i = 0; // Utilizamos la misma variable i aquí
            while(messagek32[i] != '\0') {
                LCD_data(messagek32[i]);
                i++;
            }

            // Prender el LED azul
			PTB->PCOR = BLUE_LED;

			delayMs(2000);

			// Apagar el LED azul
			PTB->PSOR = BLUE_LED;

            break;
        default:
            break;
    }
}

// Función para inicializar los LEDs
void LED_init(void) {
    // Habilitar el reloj para los puertos B y D
    SIM->SCGC5 |= (SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK);

    // Configurar los pines como salida y apagar todos los LEDs
    PORTB->PCR[18] = PORT_PCR_MUX(1);   // Configurar PTB18 como GPIO
    PTB->PDDR |= RED_LED;                // Configurar PTB18 como salida
    PTB->PSOR = RED_LED;                 // Apagar el LED rojo
    PORTB->PCR[19] = PORT_PCR_MUX(1);   // Configurar PTB19 como GPIO
    PTB->PDDR |= GREEN_LED;              // Configurar PTB19 como salida
    PTB->PSOR = GREEN_LED;               // Apagar el LED verde
    PORTD->PCR[1] = PORT_PCR_MUX(1);    // Configurar PTD1 como GPIO
    PTD->PDDR |= BLUE_LED;               // Configurar PTD1 como salida
    PTD->PSOR = BLUE_LED;                // Apagar el LED azul
}

void delayMs(int n){
int i;
int j;
for(i = 0 ; i < n; i++)
for (j = 0; j < 7000; j++) {}
}
