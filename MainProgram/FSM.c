#include <MKL25Z4.h>

//FreeRTOS how it works -> task for Pablo


// Modo de operacion automatico, semiautomatico y manual. Deberia funcionar en semitautomatico si le picas 1 se va a la posicion x
// Letras modo de operacion y numeros posicion

// Modo Automático : Lo que tiene que hacer agarrar el vasito pesar en algun lado. dependiendo su esta vacio o no va a definir la posición.

// Modo Semiautomático : Tu pones el vasito donde quieras con el teclado matricial, en que matriz ponerte de las 2. usar * o #

// Modo Manual : El usuario elegimos todo. No se pesa el vasito automaticamente.



// The machine will have 2 matrix

//Define the FSM States
typedef enum {
    STATE_IDLE,
    STATE_SELECT_MODE,
    STATE_WEIGHT, //In this state the object must be weighted
    STATE_MATRIX, // In this state the machine moves to the assigned matrix
    STATE_SELECT_POSITION, // In this state the machine goes to the xyz predefined position
    STATE_OPERATION,
    STATE_ERROR
} State;

//Define the Modes
typedef enum {
    MODE_NONE,
    MODE_AUTOMATIC,
    MODE_SEMIAUTOMATIC,
    MODE_MANUAL
} Mode;

volatile State currentState = STATE_IDLE;
volatile Mode currentMode = MODE_NONE;
volatile int currentPosition = 0;
volatile bool errorFlag = false;
volatile int matrix = 0; // 1 for [matrix 1] & 2 for [matrix 2]
volatile int weight = 0;



//Transition Logic of the State Machine
void handleStateMachine() {
    LCD_Display("Select Mode of Operation : ");
    char key = getKeyPressed();  // Get the key pressed from the keypad

    switch (currentState) {
        case STATE_IDLE:
            if (key == 'A') {
                currentMode = MODE_AUTOMATIC;
                currentState = STATE_WEIGHT;
            } else if (key == 'B') {
                currentMode = MODE_SEMIAUTOMATIC;
                currentState = STATE_MATRIX;
            } else if (key == 'C') {
                currentMode = MODE_MANUAL;
                currentState = STATE_SELECT_POSITION;
            }
            break;
        
        case STATE_WEIGHT:
            // moveToWeightPlace(); Function to move object to the weighting place
            weight = getWeight();
            currentState = STATE_MATRIX;

        case STATE_MATRIX:
            if (currentMode == MODE_AUTOMATIC){
                //Check Weight of object to decide what matrix to choose ?
                if (weight > 5){
                    matrix = 1;
                } else if (weight < 5) {
                    matrix = 2;
                }
            }
            else{
                LCD_Display("Select Matrix: ");
                char char_matrix = getKeyPressed();
                if (char_matrix == "*") {
                    matrix = 1;
                } else if (char_matrix == "#") {
                    matrix = 2;
                }
                break;
            }
            currentState = STATE_SELECT_POSITION;

        case STATE_SELECT_POSITION:
            LCD_Display("Select Position : ");
            char position = getKeyPressed();
            if (matrix  = 1){
                // moveToMatrix(matrix);// Function to move the grapper to matrix 1 (NEED TO BE ADDED)
                if (position >= '1' && position <= '9') {
                currentPosition = getPosition();
                currentState = STATE_OPERATION;
                }
            } else if (matrix = 2){
                // moveToMatrix(matrix); // Function to move the grapper to matrix 2 (NEED TO BE ADDED)
                if (position >= '1' && position <= '9') {
                currentPosition = getPosition();
                currentState = STATE_OPERATION;
                }
            } else {
                //Automatic MODE
                currentPosition = getPosition(); //Must take as arguments the weight of object, chosen matrix, record of already used spaces
                currentState = STATE_OPERATION;
            }
            break;

        case STATE_OPERATION:
            // Actions based on currentMode and currentPosition
            if (key == '0') {
                currentState = STATE_IDLE;
                currentMode = MODE_NONE;
                currentPosition = 0;
            }
            goToPosition(currentPosition); // Take as argument the currentPosition
            dropObject();
            goInitialPosition();
            currentState = STATE_IDLE;

            break;

        case STATE_ERROR:
            // Actions for error handling -> Should do something
            errorFlag = false;
            currentState = STATE_IDLE;
            currentMode = MODE_NONE;
            currentPosition = 0;
            break;
    }
}


//Setup the Interrupts
void configureInterrupts() {
    // Configure the interrupt settings for error detection
    NVIC_EnableIRQ(IRQ_NUMBER);
}

void IRQ_HANDLER() {
    // ISR for the specific interrupt
    errorFlag = true;
    currentState = STATE_ERROR;
}

int main() {
    configureInterrupts();

    while (1) {
        handleStateMachine();

        // Check for errors and transition to the ERROR state if needed
        if (errorFlag) {
            currentState = STATE_ERROR;
        }
    }

    return 0;
}