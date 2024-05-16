#include "MKL25Z4.h"

#define RS 0x02 // Define el pin RS del LCD (PTA1)
#define RW 0x04 // Define el pin RW del LCD (PTA2)
#define EN 0x10 // Define el pin EN del LCD (PTA4)
#define LCD_DATA GPIOD // Define el puerto de datos del LCD

int main(void) {
    char counter[16];
    LCD_init();
    timer_init();

    while(1) {
        LCD_command(0x80);
        LCD_string("Freq Hz:");
        LCD_command(0xC0);
        LCD_string(counter);

        while((TPM1->SC & TPM_SC_TOF_MASK) == 0) {
            // Espera a que la bandera de desbordamiento del TPM1 se active
        }

        sprintf(counter, "%d", TPM0->CNT); // Actualiza el contador con el valor actual de TPM0->CNT
        TPM0->CNT = 0; // Reinicia el contador de TPM0
        TPM1->SC |= TPM_SC_TOF_MASK; // Limpia la bandera de desbordamiento del TPM1
    }
}


void timer_init(void) {
    // Enable clock for PORTC
    SIM->SCGC5 |= 0x800;
    PORTC->PCR[12] |= 0x400;
    // Enable clock for TPM0 and select TPM clock source
    SIM->SCGC6 |= 0x1000000;
    MCG->C1 |= 0x07; // Assuming this is MCG_C1, clear bits 2:0
    SIM->SOPT2 |= 0x3000000;

    // Configure TPM0 as desired
    TPM0->SC = 0;
    TPM0->SC = 0x00;
    TPM0->MOD = 0xFFFF;
    SIM->SOPT4 = 0;

    // Enable clock for TPM1 and configure TPM1
    SIM->SCGC6 |= 0x2000000;
    TPM1->SC = 0x0;
    TPM1->MOD = 32768 - 1;

    // Start the timers
    TPM1->SC |= 0x08; // Start TPM1
    TPM0->SC |= 0x10; // Start TPM0
}


void LCD_init(void) {
    SIM->SCGC5 |= 0x200;  // Habilita el reloj para el puerto A
    SIM->SCGC5 |= 0x1000; // Habilita el reloj para el puerto D
    PORTA->PCR[1] = 0x100;  // Pone el pin PTA1 como GPIO para RS
    PORTA->PCR[2] = 0x100;  // Pone el pin PTA2 como GPIO para RW
    PORTA->PCR[4] = 0x100;  // Pone el pin PTA4 como GPIO para EN
    PORTD->PCR[0] = 0x100;  // Pone el pin PTD0 como GPIO para D4
    PORTD->PCR[1] = 0x100;  // Pone el pin PTD1 como GPIO para D5
    PORTD->PCR[2] = 0x100;  // Pone el pin PTD2 como GPIO para D6
    PORTD->PCR[3] = 0x100;  // Pone el pin PTD3 como GPIO para D7
    GPIOD->PDDR |= 0x0F;    // Pone los pines PTD0-PTD3 como salida para datos
    GPIOA->PDDR |= RS | RW | EN; // Pone los pines PTA1, PTA2 y PTA4 como salida para control
    delayMs(30);  // Espera por más de 15 ms después de VCC sube a 4.5V
    LCD_command(0x03);  // Función set: modo de 8 bits
    delayMs(10);  // Espera 5ms
    LCD_command(0x03);  // Función set: modo de 8 bits
    delayMs(1);  // Espera 160us
    LCD_command(0x03);  // Función set: modo de 8 bits
    LCD_command(0x02);  // Función set: cambia a modo de 4 bits

    // Configuración del LCD
    LCD_command(0x28);  // DL=0 (4 bits), N=1 (2 líneas), F=0 (5x8 puntos)
    LCD_command(0x06);  // ID=1 (incrementa cursor), S=0 (no desplaza pantalla)
    LCD_command(0x01);  // Limpia pantalla
    LCD_command(0x0F);  // Enciende display, cursor parpadeante
}

void LCD_command(unsigned char command) {
    LCD_DATA->PDOR &= ~0xF;  // Limpia los datos
    LCD_DATA->PDOR |= (command & 0xF0) >> 4;  // Envía los 4 bits más significativos
    GPIOA->PCOR = RS | RW;  // RS = 0, RW = 0
    GPIOA->PSOR = EN;  // EN = 1
    delayMs(0);  // Espera 1ms
    GPIOA->PCOR = EN;  // EN = 0
    delayMs(1);  // Espera 1ms

    LCD_DATA->PDOR &= ~0xF;  // Limpia los datos
    LCD_DATA->PDOR |= (command & 0x0F);  // Envía los 4 bits menos significativos
    GPIOA->PSOR = EN;  // EN = 1
    delayMs(0);  // Espera 1ms
    GPIOA->PCOR = EN;  // EN = 0
    delayMs(1);  // Espera 1ms
}

void LCD_data(unsigned char data) {
    LCD_DATA->PDOR &= ~0xF;  // Limpia los datos
    LCD_DATA->PDOR |= (data & 0xF0) >> 4;  // Envía los 4 bits más significativos
    GPIOA->PSOR = RS;  // RS = 1
    GPIOA->PCOR = RW;  // RW = 0
    GPIOA->PSOR = EN;  // EN = 1
    delayMs(0);  // Espera 1ms
    GPIOA->PCOR = EN;  // EN = 0

    LCD_DATA->PDOR &= ~0xF;  // Limpia los datos
    LCD_DATA->PDOR |= (data & 0x0F);  // Envía los 4 bits menos significativos
    GPIOA->PSOR = EN;  // EN = 1
    delayMs(0);  // Espera 1ms
    GPIOA->PCOR = EN;  // EN = 0
    delayMs(1);  // Espera 1ms
}

void delayMs(int n) {
    int i;
    int j;
    for(i = 0 ; i < n; i++)
        for(j = 0 ; j < 7000; j++) {}
}

void LCD_string(const char *str) {
    while (*str) { // Mientras el carácter actual no sea nulo ('\0')
        LCD_data(*str); // Envía el carácter actual al LCD
        str++; // Avanza al siguiente carácter de la cadena
    }
}
