#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include"driverlib/adc.h"
#include "driverlib/pwm.h"


/********************VARIABLES*********************/
 volatile int pot = 0 , photo = 0;
 volatile long int photo_AvgCounterVal = 0,pot_AvgCounterVal = 0, array_size = 40;
 volatile int photo_array[40],pot_array[40];
 /**************************************************/

 /********************FUNCTIONS********************/
 uint32_t potAvgValue(uint32_t val);  uint32_t photoAvgValue(uint32_t val); void Pot(void);
 void Photoresistor(void);void LightLED(uint32_t value); void setupPWM_PE5_PB4();void LEDTest(void);
 /**************************************************/
/**
 * main.c
 */
 uint32_t ui32ADC0Value[1];
 volatile    uint32_t avg_potvalue;
 volatile    uint32_t avg_photovalue;
 int main(void) {
//setup LEDs

 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

 //LEDs
 GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5);//PA5  LSB
 GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6);//PA6
 GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_7);//PA7
 GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2);//PA2
 GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);//PA3
 GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_4);//PA4
 GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);//PE0
 GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2);//PB2
                                                    //PE5      NOT GPIO
                                                    //PB4  MSB NOT GPIO
 setupPWM_PE5_PB4();

 //https://e2e.ti.com/support/microcontrollers/f/908/t/351475?Problem-using-interrupt-with-SW1-on-Tiva-Launchpad
 SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
 HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY; //configures sw2
 HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x1;

GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

uint32_t SW2=0;
bool SW2_Flag = false;
bool pot_adc_setup = false;
bool photo_adc_setup = false;
//LEDTest();
      while(1)
      {
          //read switch toggle
          SW2 = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
          if(!(SW2 & GPIO_PIN_0))
          {
              SW2_Flag = !SW2_Flag;
              pot_adc_setup = false;
              photo_adc_setup = false;
          }
          if(SW2_Flag == 1)
          {
              //setup for pot

                if (!pot_adc_setup) {

                    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}
                    GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5); //enable pin for pot PB5
                    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
                    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_IE | ADC_CTL_CH11 | ADC_CTL_END);
                    ADCSequenceEnable(ADC0_BASE, 1);
                    pot_adc_setup = true;
                }
              Pot();
              SysCtlDelay(3000);
          }
          else
          {

            if (!photo_adc_setup) {
                SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
                while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}
                SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
                GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_0); //enable pin for photo PD0
                ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 1);
                ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH7 );
                ADCSequenceEnable(ADC0_BASE, 1);
                photo_adc_setup = true;
            }
              Photoresistor();//starts first
              SysCtlDelay(3000);
          }
      
      }     
}



// void ADC_Setup(void)
//  {
//     //setup for pot
//       SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
//       SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
////	    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); //enable GPIO port D for photo
////      SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); //enable GPIO port E for pot
////      Wait for the ADC0 module to be ready
//       while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}
////      GPIOPinTypeADC(GPIO_PORTA_BASE, GPIO_PIN_3);
////       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //enable GPIO port for LED
////
////       GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0); // enable pin for LED
////      GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_0); //enable pin for photo PD0
//       GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5); //enable pin for pot PB5
//       ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
//       ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_IE | ADC_CTL_CH11| ADC_CTL_END );
//       ADCSequenceEnable(ADC0_BASE, 1);
//
//       //setup for photoresistor
//       SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
//       SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
//       while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)) {}
//       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
//       GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_0); //enable pin for pot PD0
//       ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
//       ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_IE | ADC_CTL_CH7 );
//       ADCSequenceEnable(ADC0_BASE, 1);
//  }
//



void Pot(void)
{
    uint32_t val = 0;
    // CLEAR INTERRUPT FLAG FOR ADC0, SEQUENCER 1
    ADCIntClear(ADC0_BASE, 1);
    // TRIGGER IS GIVEN FOR ADC 0 MODULE, SEQUENCER 1
    ADCProcessorTrigger(ADC0_BASE, 1);
    // STORE THE CONVERTED VALUE FOR ALL DIFFERENT SAMPLING IN ARRAY
    //ui32ADC0Value
    ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
    avg_potvalue = potAvgValue(ui32ADC0Value[0]);
    val = (uint32_t)(avg_potvalue / 4);
    LightLED(val);
}

/**************************************************/
//Purpose:Takes the current value of the potentiometer, stores it in an array,
//takes the average of the array and outputs the average value.
/**************************************************/
uint32_t potAvgValue(uint32_t val)
{
    uint32_t sum = 0;
    char addr = pot_AvgCounterVal % array_size;
    pot_array[addr] = val;
    uint32_t var;
    for ( var = 0; var < array_size; var++)
    {
        sum += pot_array[var];
    }
    pot_AvgCounterVal++;
    if(pot_AvgCounterVal <= array_size)
    {
        sum /= pot_AvgCounterVal;
    }
    else
    {
        sum /= array_size;
    }
    return sum;
}


void Photoresistor(void)
{
    uint32_t val=0;
    // CLEAR INTERRUPT FLAG FOR ADC0, SEQUENCER 1
    ADCIntClear(ADC0_BASE, 1);
    // TRIGGER IS GIVEN FOR ADC 0 MODULE, SEQUENCER 1
    ADCProcessorTrigger(ADC0_BASE, 1);
    // STORE THE CONVERTED VALUE FOR ALL DIFFERENT SAMPLING IN ARRAY
    //ui32ADC0Value
    ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);
    avg_photovalue = photoAvgValue(ui32ADC0Value[0]);
    val = (uint32_t)(avg_photovalue / 4);
    LightLED(val);
}

uint32_t photoAvgValue(uint32_t val)
{
    uint32_t sum = 0;
    char addr = photo_AvgCounterVal % array_size;
    photo_array[addr] = val;
    uint32_t var;
    for ( var = 0; var < array_size; var++)
    {
        sum += photo_array[var];
    }
    photo_AvgCounterVal++;
    if(photo_AvgCounterVal <= array_size)
    {
        sum /= photo_AvgCounterVal;
    }
    else
    {
        sum /= array_size;
    }
    return sum;
}


void LEDTest(void)
{
    int var = 0;
    while (1) {
        for (var = 0; var < 1023; var+= 100) {
            LightLED(var);
            SysCtlDelay(3000000);
        }
    }
}

 /**************************************************/
 //Purpose: Lights LEDs based on the input value.
 /**************************************************/
 void LightLED(uint32_t value)
 {
     //LEDs OFF
     GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, 0);//PA-2 3 4 5 6 7
     GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2 | GPIO_PIN_4, 0);//PB-2 4
     GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0 | GPIO_PIN_5, 0);//PE-0 5
     PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT,   false);//PE4
     PWMOutputState(PWM0_BASE, PWM_OUT_5_BIT,   false);//PE5

     if((value >= 0) && (value <= 50 ))
     {
         return;
     }
     if((value >= 50) && (value <= 150 ))
     {
         GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_5, GPIO_PIN_5);//PA5
     }
     else if((value > 150) && (value  <= 207))
     {
         GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_6, GPIO_PIN_6);//PA6
     }
     else if((value > 207) && (value <= 309))
     {
         GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_7, GPIO_PIN_7);//PA7
     }
     else if((value > 309) && (value  <= 411))
     {
         GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2, GPIO_PIN_2);//PA2
     }
     else if((value > 411) && (value  <= 513))
     {
         GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3, GPIO_PIN_3);//PA3
     }
     else if((value > 513) && (value  <= 650))
     {
         GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4, GPIO_PIN_4);//PA4
     }
     else if((value > 650) && (value <=  700))
     {
         GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0, GPIO_PIN_0);//PE0
     }
     else if((value > 700) && (value  <=  800))
     {
         GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2, GPIO_PIN_2);//PB2
     }
     else if((value > 800) && (value <=  900))
     {
          PWMOutputState(PWM0_BASE, PWM_OUT_5_BIT,   true);//PE5
     }
     else if((value > 900) && (value <=  1023))
     {
         PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT,   true);//PB4
     }
 }

 void setupPWM_PE5_PB4()
{
    //http://e2e.ti.com/support/microcontrollers/other_microcontrollers/f/908/p/293307/1024699
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    GPIOPinConfigure(GPIO_PB4_M0PWM2);

    GPIOPinConfigure(GPIO_PE5_M0PWM5);

    GPIOPinTypePWM(GPIO_PORTB_BASE,GPIO_PIN_4 );

    GPIOPinTypePWM(GPIO_PORTE_BASE, GPIO_PIN_5);

    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, (PWM_GEN_MODE_DOWN| PWM_GEN_MODE_NO_SYNC));

    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, (PWM_GEN_MODE_DOWN| PWM_GEN_MODE_NO_SYNC));

    PWMGenEnable(PWM0_BASE, PWM_GEN_1);

    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
}

