#include <MKL25Z4.H>

#define RS 0x04
#define RW 0x10
#define EN 0x20

void LCD_init(void);
void LCD_ready(void);
void LCD_command(unsigned char command);
void LCD_command_noWait(unsigned char command);
void LCD_data(unsigned char data);
void displayMessage(char* message);

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
