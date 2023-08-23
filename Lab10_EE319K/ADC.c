// ADC.c
// Runs on TM4C123
// Provide functions that initialize ADC0
// Last Modified: 1/16/2021
// Student names: change this to your names or look very silly
// Last modification date: change this to the last modification date or look very silly

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(void){ 
SYSCTL_RCGCGPIO_R |= 0x8; // activate Port D
	while((SYSCTL_PRGPIO_R & 0x8) == 0){};
	GPIO_PORTD_DIR_R &= ~0x4; // PD2 input
	GPIO_PORTD_AFSEL_R |= 0x4; // enable alternate function on PD2
	GPIO_PORTD_DEN_R &= ~0x4; // disable digital I/O on PD2
	GPIO_PORTD_AMSEL_R |= 0x4; // enable analog function on PD2
	
	SYSCTL_RCGCADC_R |= 0x01; // activate ADC0
		uint32_t volatile delay; 
	delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;		
			delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;		
			delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;		
			delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;			
	delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;			
	delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R; //NOP
	delay = SYSCTL_RCGCADC_R;		
		
	ADC0_PC_R = 0x1; // confiture for 125k
	ADC0_SSPRI_R = 0x0123; // seq3 is highest priori
	ADC0_ACTSS_R &= ~0x0008;//disable sample sequencer 3
	ADC0_EMUX_R &= ~0xF000;//seq3 is software trigger
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0) + 5; // Ain5
	ADC0_SSCTL3_R = 0x0006; // no TS0 Do, yes IE0 END0
	ADC0_IM_R &= ~0x0008; // disable SS3 interrupt
	ADC0_ACTSS_R |= 0x0008; // enable sample sequencer 3
	//ADC0_SAC_R  = sac;

}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
uint16_t data;
  ADC0_PSSI_R = 0x0008;
	while((ADC0_RIS_R & 0x08) == 0){};
		data = ADC0_SSFIFO3_R & 0xFFF;
		ADC0_ISC_R = 0x0008;
		return data;//replace this line
  
}


