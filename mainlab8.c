/* 
 * File:   mainlab8.c
 * Author: santr
 *
 * Created on 18 de abril de 2022, 06:10 PM
 */
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>

/*------------------------------------------------------------------------------
 * CONSTANTES 
 ------------------------------------------------------------------------------*/
#define _XTAL_FREQ 4000000

/*------------------------------------------------------------------------------
 * VARIABLES 
 ------------------------------------------------------------------------------*/
int8_t var_tmr0;
int8_t bandera;
int8_t unidades;
int8_t decenas;
int8_t centenas;
int8_t numero;
int8_t operacion;

/*------------------------------------------------------------------------------
 * PROTOTIPO DE FUNCIONES 
 ------------------------------------------------------------------------------*/
void setup(void);
uint8_t Tabla(uint8_t numero7);
void multi(void);
void convertir(void);

/*------------------------------------------------------------------------------
 * INTERRUPCIONES 
 ------------------------------------------------------------------------------*/
void __interrupt() isr (void){
    if(PIR1bits.ADIF){              // Fue interrupción del ADC?
        if(ADCON0bits.CHS == 0){    // Verificamos sea AN0 el canal seleccionado
            PORTC = ADRESH;         // Mostramos ADRESH en PORTC
                     
        }
        
        else if (ADCON0bits.CHS == 1){
            numero = ADRESH;
        }
        if(INTCONbits.T0IF){
        multi();
        TMR0 = var_tmr0;
        INTCONbits.T0IF = 0;
        }
    
        PIR1bits.ADIF = 0;          // Limpiamos bandera de interrupción
    }
    return;
}

/*------------------------------------------------------------------------------
 * CICLO PRINCIPAL
 ------------------------------------------------------------------------------*/
void main(void) {
    setup();
    while(1){
        if(ADCON0bits.GO == 0){             // No hay proceso de conversion
            
            if(ADCON0bits.CHS == 0b0000)    
                ADCON0bits.CHS = 0b0001;    // Cambio de canal
            else if(ADCON0bits.CHS == 0b0001)
                ADCON0bits.CHS = 0b0000;    // Cambio de canal
            __delay_us(40);                 // Tiempo de adquisición
            
            ADCON0bits.GO = 1;              // Iniciamos proceso de conversión
        } 
        convertir();
    }
    return;
}

/*------------------------------------------------------------------------------
 * CONFIGURACION 
 ------------------------------------------------------------------------------*/
void setup(void){
    var_tmr0 = 248;
    ANSEL = 0b00000011; // AN0 como entrada analógica
    ANSELH = 0;         // I/O digitales)
    
    //ANSEL = 0b00000111; // AN0, AN1 y AN2 como entrada analógica
    
    TRISA = 0b00000011; // AN0 como entrada
    //TRISA = 0b00000111; // AN0, AN1 y AN2 como entrada
    PORTA = 0; 
    
    TRISC = 0;
    TRISE = 0x00;
    PORTC = 0;
    TRISD = 0;
    PORTD = 0;
    PORTE = 0;
    
    // Configuración reloj interno
    OSCCONbits.IRCF = 0b0110;   // 4MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuración ADC
    ADCON0bits.ADCS = 0b01;     // Fosc/8
    ADCON1bits.VCFG0 = 0;       // VDD
    ADCON1bits.VCFG1 = 0;       // VSS
    ADCON0bits.CHS = 0b0000;    // Seleccionamos el AN0
    ADCON1bits.ADFM = 0;        // Justificado a la izquierda
    ADCON0bits.ADON = 1;        // Habilitamos modulo ADC
    __delay_us(40);             // Sample time
    
    // Configuracion interrupciones
    PIR1bits.ADIF = 0;          // Limpiamos bandera de ADC
    PIE1bits.ADIE = 1;          // Habilitamos interrupcion de ADC
    INTCONbits.PEIE = 1;        // Habilitamos int. de perifericos
    INTCONbits.GIE = 1;         // Habilitamos int. globales
    INTCONbits.T0IF = 0;
    
    //CONFIGURACION TMR0
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.PSA = 0;
    OPTION_REGbits.PS2 = 1;
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
    TMR0 = var_tmr0;
    
}
void multi(void){
    PORTE = 0;
    bandera++;
    switch(bandera){
        case 1:
            PORTD = 0;
            PORTE = 0b001;
            centenas = Tabla(centenas);  //busacar el valor del 7seg
            PORTD = centenas;   //Mostrar el valor en el 7seg
            break;
        case 2:
            PORTD = 0;
            PORTE = 0b010;
            decenas = Tabla(decenas);
            PORTD = decenas;
            break;
        default:
            PORTD = 0;
            PORTE = 0b100;
            unidades = Tabla(unidades);
            PORTD= unidades;
            bandera = 0;
    }
            
    }
uint8_t Tabla(uint8_t numero7){
    //recibe un valor de 8 bits que se desea ver en el 7 segmentos
    //devulve el valor apropiado para usar en un 7 segmentos
    
    uint8_t valor, seg;
    seg = numero7;
    
    switch(seg){
        case 0:
            valor= 0b00111111;
            break;
        case 1:
            valor= 0b00000110;
            break;
        case 2:
            valor= 0b01011011;
            break;
        case 3:
            valor= 0b01001111;
            break;
        case 4:
            valor= 0b01100110;
            break;
        case 5:
            valor= 0b01101101;
            break;
        case 6:
            valor= 0b01111101;
            break;
        case 7:
            valor= 0b00000111;
            break;
        case 8:
            valor= 0b01111111;
            break;
        case 9:
            valor= 0b01101111;
            break;
        case 10:
            valor= 0b01110111;
            break;
        case 11:
            valor= 0b01111100;
            break;
        case 12:
            valor= 0b00111001;
            break;
        case 13:
            valor= 0b01011110;
            break;
        case 14:
            valor= 0b01111001;
            break;
        case 15:
            valor= 0b01110001;
            break;     
    }
    return valor;
}
void convertir(void){ 
    centenas = numero / 100;
    numero = numero - (centenas*100);
    decenas = numero / 10;
    unidades = numero - (decenas*10);
    }
