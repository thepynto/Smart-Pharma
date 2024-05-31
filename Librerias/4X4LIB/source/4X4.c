#include <MKL25Z4.h>

void keypad_init(void);
char keypad_getkey(void);

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
delayUs(2); /* wait for signal return */
col = PTC-> PDIR & 0xF0; /* read all columns */
PTC->PDDR = 0; /* disable all rows */
if (col == 0xF0)
return 0; /* no key pressed */
/* If a key is pressed, we need find out which key.*/
for (row = 0; row < 4; row++)
{ PTC->PDDR = 0; /* disable all rows */

PTC->PDDR |= row_select[row]; /* enable one row */
PTC->PCOR = row_select[row]; /* drive active row low*/

delayUs(2); /* wait for signal to settle */
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
