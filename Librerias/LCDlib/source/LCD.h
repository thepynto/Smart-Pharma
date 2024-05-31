void LCD_init(void);
void LCD_ready(void);
void LCD_command(unsigned char command);
void LCD_command_noWait(unsigned char command);
void LCD_data(unsigned char data);
void displayMessage(char* message);