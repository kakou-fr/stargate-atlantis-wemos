// PINS

#define DATA_PIN1 D1   // gate symbol
#define DATA_PIN2 D2   // gate chevron
#define DATA_PIN3 D3   // light Front Right
#define DATA_PIN4 D4   // light Middle Right
#define DATA_PIN5 D5   // light Middle Left 
#define DATA_PIN6 D6   // light Front Left
#define DATA_PIN7 D7   // light STAIRS (5,4,4,3,3,3,2,2)
#define DATA_PIN8 D8   // vortex

#define NUM_LEDS1 72
#define NUM_LEDS2 27
#define NUM_LEDS3 3
#define NUM_LEDS4 5
#define NUM_LEDS5 5
#define NUM_LEDS6 3
#define NUM_LEDS7 26
#define NUM_LEDS8 56

#define NUM_SYMBOLS (NUM_LEDS1/2)
#define SYMBOL1 5
#define SYMBOL2 9
#define SYMBOL3 13
#define SYMBOL4 17
#define SYMBOL5 21
#define SYMBOL6 25
#define SYMBOL7 29
#define SYMBOL8 33
#define SYMBOL9 37

#define FADE_SYMBOLS 30
#define SPEED_SYMBOLS 20
int old_symbols = SYMBOL4;

// define variables

int R=255, G=165, B=0;
int R_symbols=0, G_symbols=255, B_symols=255;
int R_chevrons=0, G_chevrons=128, B_chevrons=255;
int R_stairs=0, G_stairs=0, B_stairs=255;

//Sample Startgate addresses. un-comment the address you want.
//Abydos
int Address[] = {SYMBOL5,SYMBOL6,SYMBOL7,SYMBOL1,SYMBOL2,SYMBOL3,SYMBOL4,0,0};
int Address_Length = 7;

const int Address_Abydos[] = {SYMBOL1,SYMBOL2,SYMBOL3,SYMBOL4,SYMBOL5,SYMBOL6,SYMBOL7};
const int R_Abydos=255, G_Abydos=165, B_Abydos=0;

//Othala (Asgard homeworld)
const int Address_Asgard[] = {SYMBOL7,SYMBOL6,SYMBOL5,SYMBOL4,SYMBOL3,SYMBOL2,SYMBOL1};
const int R_Asgard=0, G_Asgard=255, B_Asgard=0;

//Destiny
const int Address_Destiny[] = {SYMBOL4,SYMBOL5,SYMBOL3,SYMBOL6,SYMBOL2,SYMBOL7,SYMBOL1,SYMBOL8,SYMBOL9};
const int R_Destiny=0, G_Destiny=0, B_Destiny=255;

int nothing_to_do=0;
int dialing = 0;
int Dialling = 0;

String ipStr = "";
String respMsg = "";
String formattedTime = "";
bool noInit = true;

//function
void dial();