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
     
     uint32_t ui32ADC0Value[1];
     uint32_t avg_adcvalue;
     uint32_t LED_value_current = 0;
     uint32_t LED_value_sent = 0;

         
      while(1)
      {
          // CLEAR INTERRUPT FLAG FOR ADC0, SEQUENCER 1
          ADCIntClear(ADC0_BASE, 1);
          // TRIGGER IS GIVEN FOR ADC 0 MODULE, SEQUENCER 1
          ADCProcessorTrigger(ADC0_BASE, 1);
          // STORE THE CONVERTED VALUE FOR ALL DIFFERENT SAMPLING IN ARRAY
          ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
            
             if(SW2 == 0x00) { //check to see if switch is pressed
                 avg_adcvalue = potAvgValue(ui32ADC0Value[1]);
                 LightLED(potAvgValue);
                 
             }
             else {
                 avg_adcvalue = photoAvgValue(ui32ADC0Value[1]);
                 LightLED(photoAvgValue);
                 
             }
      }
     
     
}



 void ADC_Setup(void)
 {
      SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
      SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); //enable GPIO port B for photo
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); //enable GPIO port E for pot
     
     // Wait for the ADC0 module to be ready
     while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {
         
     }
      GPIOPinTypeADC(GPIO_PORTA_BASE, GPIO_PIN_3);
      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //enable GPIO port for LED
     
      GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0); // enable pin for LED
      SW2 = GPIO_PORTF_DATA_R&0x01; // read PF0 into SW2
     
      GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0); //enable pin for photo PD0
      GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_5); //enable pin for pot PE2
      ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
      ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0);
      ADCSequenceEnable(ADC0_BASE, 1);)
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
//Purpose:Takes the current value of the photoresistor, stores it in an array,
//takes the average of the array and outputs the average value.
/**************************************************/
int photoAvgValue(int val)
{
    int sum = 0;
    char addr = photo_AvgCountVal % 10;
    photo_array[addr] = val;
    int var;
    for ( var = 0; var < 10; var++)
    {
        sum += photo_array[var];
    }
    photo_AvgCountVal++;
    if(photo_AvgCountVal <= 10)
    {
        sum /= photo_AvgCountVal;
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

