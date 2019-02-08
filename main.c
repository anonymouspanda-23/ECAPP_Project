#pragma config OSC = XT // Oscillator Selection bits (XT oscillator)
#pragma config LVP = OFF // Single-Supply ICSP Enable bit (Single-Supply ICSP disabled)
#pragma config WDT = OFF // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))

#include <xc.h>

#define _XTAL_FREQ 4000000

#define LCD_DATA PORTC
#define LCD_RS RD2
#define LCD_E RD3

#define enterButton PORTAbits.RA1
#define lightControl PORTBbits.RB1

#define SPK PORTCbits.RC2
#define LED_R1 RA2
#define LED_R2 RA5

char welcome[] = "Please enter    ";
char welcomeHome[] = "Welcome Home!   ";

void Init_LCD();
void W_ctr_4bit(char);
void W_data_4bit(char);
void speakerON();

void interrupt ISR();

int keyHeld = 0;
int maxBrightness = 144;
int brightness = 144;
int speakerOn = 0;

unsigned char LCD_TEMP;

void main(void) {
    ADCON0 = 0b00000001;
    ADCON1 = 0x0E;
    ADCON2 = 0b10011100;
    TRISA = 0b11000010;
    TRISB = 0b11111111;
    TRISC = 0b00001001;
    TRISD = 0b11110011;
    TRISE = 0b11101111;
    
    Init_LCD();
    
    GIE = 0;
    IPEN = 0;
    
    INT1IP = 1;
    INT1IF = 0;
    INT1IE = 1;
    INTEDG1 = 0;
    
    INT2IP = 1;
    INT2IF = 0;
    INT2IE = 1;
    INTEDG2 = 0;
    
    TMR0IP = 1;
    TMR0IE = 1;
    TMR0IF = 0;
    
    T2CON = 0b00000101;
    PR2 = 124;
    
    CCPR1L = 0b10010000;
    CCP1CON = 0b00010000;
    
    GIE = 1;
    GIEL = 1;
    
    PORTCbits.RC2 = 0;
    
    W_ctr_4bit(0b10000000);
    for(int loop = 0; welcome[loop]!=0; loop++)
        W_data_4bit(welcome[loop]);
    while(enterButton);
    W_ctr_4bit(0b10000000);
    for(int loop = 0; welcomeHome[loop]!=0; loop++)
        W_data_4bit(welcomeHome[loop]);
    CCP1CON = 0b00011100;
    brightness = 144;
    while(1) {
        if(PORTBbits.RB0 == 1)
            T0CON = 0b10001100; 
        else
            speakerON();
    }
}

void interrupt ISR() {
    if(INT1IF) {
        if(brightness < maxBrightness) {
            CCPR1L+=12;
            brightness+=12;
        }
        INT1IF = 0;
    } else if(INT2IF) {
        if(brightness > 0) {
            CCPR1L-=12;
            brightness-=12;
        }
        INT2IF = 0;
    } else if(TMR0IF){
        CCP2CON = 0b00100000;
        LED_R1 = 0;
        LED_R2 = 0;
        TMR0H = 0x0B;
        TMR0L = 0xDC;
        TMR0IF = 0;
    }
}

void speakerON(void){
   int result;
   T2CON = 0b00000111;

   if(PORTDbits.RD0 == 1){
        LED_R1 = 1;
        LED_R2 = 1;

        ADCON0bits.GO = 1;    
        while(ADCON0bits.DONE);  
        result = ADRESL;
        result >>= 1;
        for(int j=0;j<1000;j++)
        {
            CCPR2L = 0b00101101 + result;
            CCP2CON = 0b00101100;
        }
    } else {
       LED_R1 = 0;
       LED_R2 = 0;
    }
}

void Init_LCD(){
    _delay(15);
    W_ctr_4bit(0x03);
    _delay(5);
    W_ctr_4bit(0x02);
    W_ctr_4bit(0b00101000);
    W_ctr_4bit(0b00001100);
    W_ctr_4bit(0b00000110);
    W_ctr_4bit(0b00000001);
}

void W_ctr_4bit(char x){
    LCD_RS = 0;
    LCD_TEMP = x;
    LCD_E = 1;
    LCD_DATA = LCD_TEMP  & 0b11110000;
    _delay(1000);
    LCD_E = 0;
    _delay(1000);
    LCD_TEMP = x;
    LCD_TEMP <<= 4;
    LCD_E = 1;
    LCD_DATA = LCD_TEMP & 0b11110000;
    _delay(1000);
    LCD_E = 0;
    _delay(1000);
}

void W_data_4bit(char x){
    LCD_RS = 1;
    LCD_TEMP = x;
    LCD_E = 1;
    LCD_DATA = LCD_TEMP & 0b11110000;
    _delay(1000);
    LCD_E = 0;
    _delay(1000);
    LCD_TEMP = x;
    LCD_TEMP <<= 4;
    LCD_E = 1;
    LCD_DATA = LCD_TEMP & 0b11110000;
    _delay(1000);
    LCD_E = 0;
    _delay(1000);
}