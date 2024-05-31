/* Generate 60Hz 33% PWM output
* TPM0 uses MCGFLLCLK which is 41.94 MHz.

* The prescaler is set to divide by 16.

* The modulo register is set to 43702 and the CnV

* register is set to 14568. See Example 3 for

* the calculations of these values.*/

#include <MKL25Z4.H>

void keypad_init(void);
char keypad_getkey(void);
void delayMs(int n);

int main (void) {
	int mod = 43702;
	int DT0;
	int DT25;
	int DT50;
	int DT75;
	int DT100;

	keypad_init();

SIM->SCGC5 |= 0x1000; /* enable clock to Port D */
PORTD->PCR[1] = 0x0400; /* PTD1 used by TPM0 */
SIM->SCGC6 |= 0x01000000; /* enable clock to TPM0 */
SIM->SOPT2 |= 0x01000000; /* use MCGFLLCLK as timer counter clock */

TPM0->SC = 0; /* disable timer */
/* edge-aligned, pulse high */
TPM0->CONTROLS[1].CnSC = 0x20 | 0x08;
/* Set up modulo register for 60 kHz */
TPM0->MOD = mod;

TPM0->SC = 0x0C; /* enable TPM0 with prescaler /16 */

// 0% DT
DT0 = 0;

// 25% DT
DT25 = mod / 4;

// 50% DT
DT50 = mod / 2;

// 75% DT
DT75 = mod / 4 * 3;

// 100% DT
DT100 = mod;

while (1) {
	//TPM0->CONTROLS[1].CnV = DT100; // Set up channel value for 0% dutycycle

	char key = keypad_getkey(); // Leer la tecla presionada

	switch (key) {
		case '1':
			TPM0->CONTROLS[1].CnV = DT0; // Set up channel value for 0% dutycycle
		case '2':
			TPM0->CONTROLS[1].CnV = DT25; // Set up channel value for 25% dutycycle
		case '3':
			TPM0->CONTROLS[1].CnV = DT50; // Set up channel value for 50% dutycycle
		case '4':
			TPM0->CONTROLS[1].CnV = DT75; // Set up channel value for 75% dutycycle
		case '5':
			TPM0->CONTROLS[1].CnV = DT100; // Set up channel value for 100% dutycycle
		default:
			TPM0->CONTROLS[1].CnV = DT0; // Set up channel value for 0% dutycycle
	}

}
}

/* Delay n milliseconds */
void delayMs(int n) {
    int i, j;
    for (i = 0; i < n; i++)
        for (j = 0; j < 7000; j++) {}
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
