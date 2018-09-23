#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include"driverlib/adc.h"


/********************VARIABLES*********************/
uint32_t ADCValue[1];

/**
 * main.c
 */
 int main(void) {


ADC_Setup();
     
  while(1) {
         
    //Before starting the ADC conversion clear the Interrupt status using the below command.
    ADCIntClear ( ADC0_BASE , 3);
         
    //trigger the ADC conversion by using ADCProcessorTrigger command
    ADCProcessorTrigger(ADC0_BASE,3);
    ADCSequenceDataGet(ADC0_BASE, 3, ADCValue);
         
    
    printf("yes %d\n", ADCValue[0]);
         
    SysCtlDelay(20000000);
         
         
         
  }
     
}



 void ADC_Setup(void)
 {
      SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
      SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
     // Wait for the ADC0 module to be ready
     while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {
         
     }
      GPIOPinTypeADC(GPIO_PORTA_BASE, GPIO_PIN_3);
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //enable GPIO port for LED
     
      GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_5);
      GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); //enable pin for LED PF2
      GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_4); //enable pin for LED PF4
      ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
      ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_IE | ADC_CTL_CH4 );
      ADCSequenceEnable(ADC0_BASE, 0);)
 }


 /**************************************************/
 //Purpose:Takes the current value of the potentiometer, stores it in an array,
 //takes the average of the array and outputs the average value.
 /**************************************************/
 int potAvgValue(int val)
 {
     int sum = 0;
     char addr = pot_AvgCounterVal % 10;
     pot_array[addr] = val;
     int var;
     for ( var = 0; var < 10; var++)
     {
         sum += pot_array[var];
     }
     pot_AvgCounterVal++;
     if(pot_AvgCounterVal <= 10)
     {
         sum /= pot_AvgCounterVal;
     }
     else
     {
         sum /= 10;
     }
     return sum;
 }


 /**************************************************/
 //Purpose: Lights LEDs based on the input value.
 /**************************************************/
 int LightLED(int value)
 {
     if((value >= 0) && (value <= 23 ))
          return 0;
     if((value >= 23) && (value <= 123 ))
     {
         return 1;
     }
     else if((value > 123) && (value  <= 223))
     {
         return 2;
     }
     else if((value > 223) && (value <= 323))
     {
         return 3;
     }
     else if((value > 323) && (value  <= 423))
     {
         return 4;
     }
     else if((value > 423) && (value  <= 523))
     {
         return 5;
     }
     else if((value > 523) && (value  <= 623))
     {
         return 6;
     }
     else if((value > 623) && (value <=  723))
     {
         return 7;
     }
     else if((value > 723) && (value  <=  823))
     {
         return 8;
     }
     else if((value > 823) && (value <=  923))
     {
         return 9;
     }
     else if((value > 923) && (value <=  1023))
     {
         return 10;
     }
     return 0;
 }

