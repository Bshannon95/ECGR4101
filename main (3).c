/************************************
*
* COURSE: ECGR 4101/5101
* PROJECT: Lab Three
* AUTHORS: Bryson Shannon | Margaret Reichard
* FILENAME: main.c
*
************************************/

#include<stdint.h>
#include<stdio.h>
#include<stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"


/********************VARIABLES*********************/
volatile int pot = 0 , photo = 0;
volatile long int photo_AvgCounterVal = 0,pot_AvgCounterVal = 0, array_size = 40;
volatile int photo_array[40],pot_array[40];
uint32_t ui32ADC0Value[1];
volatile uint32_t avg_potvalue;
volatile uint32_t avg_photovalue;
/**************************************************/

/********************FUNCTIONS********************/
void ADC_Setup(void);uint32_t potAvgValue(uint32_t val);  uint32_t photoAvgValue(uint32_t val); void Pot(void);
void Photoresistor(void);void LightLED(uint32_t value); void setupPWM_PE5_PB4();void LEDTest(void);
/**************************************************/

/**
 * main.c
 */

 int main(void)
 {
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
 //setup pin PE5 AND PB4 for PWM
 setupPWM_PE5_PB4();

 //configure SW2
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

//test that all LEDs are operational
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
              //check if the pin has already been configured for ADC
                if (!pot_adc_setup) {

                    //setup for pot for ADC
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
            //check if the pin has already been configured for ADC
            if (!photo_adc_setup) {
                //setup photo resistor ADC
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

/**************************************************/
//Purpose: Setup the ADC for pins PD0 and PB5
//Parameters: None.
//Output: None.
/**************************************************/

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


/**************************************************/
//Purpose: Measure the analog value of the pot and sends the value to
//the potAvgValue and the LightLED function.
//Parameters: None.
//Output: None.
/**************************************************/
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
    //take the average value of the pot
    avg_potvalue = potAvgValue(ui32ADC0Value[0]);
    //change the value to fit in a set range
    val = (uint32_t)(avg_potvalue / 4);
    //light an LED based on the value
    LightLED(val);
}

/**************************************************/
//Purpose:Takes the current value of the potentiometer, stores it in an array,
//takes the average of the array and outputs the average value.
//Parameters: uint32_t val - Current ADC value from the pot.
//Output: The averaged value of the pot.
/**************************************************/
uint32_t potAvgValue(uint32_t val)
{
    uint32_t sum = 0;
    //puts val in the array
    char addr = pot_AvgCounterVal % array_size;
    pot_array[addr] = val;
    uint32_t var;
    // take the average of the array
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
    //return averaged value
    return sum;
}

/**************************************************/
//Purpose: Measure the analog value of the photoresistor and sends the value to
//the photoAvgValue and the LightLED function.
//Parameters: None.
//Output: None.
/**************************************************/
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

/**************************************************/
//Purpose:Takes the current value of the photoresistor, stores it in an array,
//takes the average of the array and outputs the average value.
//Parameters: uint32_t val - Current ADC value from the photoresistor.
//Output: The averaged value of the photoresistor.
/**************************************************/
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

/**************************************************/
//Purpose: Test the functionality of the LEDs. For testing purposes only.
//Parameters: None.
//Output: None.
/**************************************************/
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
//Purpose: Light an LED based on the input value.
//Parameter: None.
//Output: None.
/**************************************************/
 void LightLED(uint32_t value)
 {
     //LEDs OFF
     GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, 0);//PA-2 3 4 5 6 7
     GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2 | GPIO_PIN_4, 0);//PB-2 4
     GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_0 | GPIO_PIN_5, 0);//PE-0 5
     PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT,   false);//PE4
     PWMOutputState(PWM0_BASE, PWM_OUT_5_BIT,   false);//PE5

     //turn on one LED based on the val
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

/**************************************************/
//Purpose: Setup pins PE5 and PB4 for PWM
//Parameter: None.
//Output: None.
/**************************************************/
 void setupPWM_PE5_PB4()
{
    //http://e2e.ti.com/support/microcontrollers/other_microcontrollers/f/908/p/293307/1024699
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    //Enable PWM0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    //Configure GPIO pins
    GPIOPinConfigure(GPIO_PB4_M0PWM2);

    GPIOPinConfigure(GPIO_PE5_M0PWM5);

    //Configure PWM pins
    GPIOPinTypePWM(GPIO_PORTB_BASE,GPIO_PIN_4 );

    GPIOPinTypePWM(GPIO_PORTE_BASE, GPIO_PIN_5);

    //Configure PWM
    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, (PWM_GEN_MODE_DOWN| PWM_GEN_MODE_NO_SYNC));

    PWMGenConfigure(PWM0_BASE, PWM_GEN_2, (PWM_GEN_MODE_DOWN| PWM_GEN_MODE_NO_SYNC));

    //Enable PWM
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);

    PWMGenEnable(PWM0_BASE, PWM_GEN_2);
}
