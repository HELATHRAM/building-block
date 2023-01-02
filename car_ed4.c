/* =====================================================
 Fundamentals of Mechatronics (using Ultrasonic sensor)
This program grab some sort of range-finder and stick it on a servo.
Write a little application that tells you “There’s something X cm away at a bearing of Y degrees”.
You can test that by sticking it on a base and using it to build a map of the area around the sensor without moving it.
*/

//Libraries
#include <msp.h>
#include <driverlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "sensor.h"

//Defines
#define timerA_period0    60000 //fixed frequency for PWM1
#define SWITCH GPIO_PORT_P1
#define S1 GPIO_PIN1
#define S2 GPIO_PIN4
#define TICKPERIOD 1000

int a=0; //Timer A0 Variable used for toggle
int b=0; //Timer A2 Variable used for toggle
volatile int PWM1 = 0;
volatile float front_distance = 0; //distance from right ultrasonic sensor ?????

//Timer A0 configuration
//-------------------- DC motors 0 Timer--------------------
const Timer_A_UpModeConfig upConfig_0 = // Configure Timer A counter in Up mode
		{ TIMER_A_CLOCKSOURCE_SMCLK,  // Tie Timer A0 to sub-master clock (SMCLK)
			TIMER_A_CLOCKSOURCE_DIVIDER_3, // Increment counter every 1 clock cycles; Means counter is incremented at 3E+6Hz
			timerA_period0, // Period of Timer A0 (this value placed in TA0CCR0); To define a timer frequency=3000000/60000=50Hz
			TIMER_A_TAIE_INTERRUPT_DISABLE, // Disable Timer A rollover interrupt
			TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE, // Enable Capture Compare interrupt
			TIMER_A_DO_CLEAR            // Clear counter upon initialization
		};

//--------------------Sensor Timer--------------------
const Timer_A_UpModeConfig upConfig_1 =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_3,          // SMCLK/3 = 1MHz
        TICKPERIOD,                                   //
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};

// Main Loop
void main(){
    Interrupt_disableMaster();

    //Set output mode to Reset/Set
	TA0CCTL1=OUTMOD_7; //Macro which is equal to 0x00e0
	TA0CCTL2=OUTMOD_7; //Macro which is equal to 0x00e0

	TA0CCR1=0; //controls the Duty Cycle for 2.4
	TA0CCR2=0; //controls the Duty Cycle for 2.5

	P2SEL0 |= 0xF0 ;    // Set bit 4 of P2SEL0 to enable TA0.1 functionality on P2.4
	P2SEL1 &= ~0xF0 ;   // Clear bit 4 of P2SEL1 to enable TA0.1 functionality on P2.4
	P2DIR |= 0xF0 ;     // Set pin 2.4 as an output pin //11110000

	P2SEL0 |= 0xE0 ; // pin 2.5
	P2SEL1 &= ~0xE0 ; // pin 2.5
	P2DIR |= 0xE0 ; // pin 2.5

    WDT_A_holdTimer() ;  // Disable the Watchdog Timer
    CS_setDCOFrequency(3E+6); // Set DCO Frequency to 3 Mhz
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1); // Set up Sub-Master Clock

    GPIO_setAsOutputPin(GPIO_PORT_P3,GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN2);
    GPIO_setAsOutputPin(GPIO_PORT_P3,GPIO_PIN3);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN3);
    GPIO_setAsOutputPin(GPIO_PORT_P3,GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN5);
    GPIO_setAsOutputPin(GPIO_PORT_P3,GPIO_PIN6);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN6);

    // Set P1.1 as a digital input pin & set P1.4 as a digital input pin
    GPIO_setAsInputPinWithPullUpResistor( SWITCH, S1); // Switch 1 (S1)
    GPIO_setAsInputPinWithPullUpResistor( SWITCH, S2); // Switch 2 (S2)

    ///// *** Configure Timer A0 and its interrupt *** /////
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig_0);
    Interrupt_disableInterrupt(INT_TA0_0);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    //Configuring Timer_A1 for Up Mode, front ultrasonic sensors ?????
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig_1); //Front sensor
    Interrupt_enableInterrupt(INT_TA1_0);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE); //start counter for front sensor

    //Initialize sensor pins using header file functions
    void init_Sensors();

    //Setting up interrupt for Switch 1.1 & Switch 1.4
    // 1. Disable All Interrupts
    Interrupt_disableMaster();
    // 2. Select falling edge for push button
    GPIO_interruptEdgeSelect(GPIO_PORT_P1,GPIO_PIN1|GPIO_PIN4,GPIO_HIGH_TO_LOW_TRANSITION);
    // 3. Clear interrupt flags
    GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN1|GPIO_PIN4);
    // 4. Arm interrupts on those pins
    GPIO_enableInterrupt(GPIO_PORT_P1,GPIO_PIN1|GPIO_PIN4);
    // 5. Set priority for Port 1 Switches
    Interrupt_setPriority(INT_PORT1,1); //set priority of button press
    Interrupt_setPriority(INT_TA1_0,0); //set priority of front sensor interrupt
    // 6. Enable interrupts on those ports
    Interrupt_enableInterrupt(INT_PORT1);
    // 7. Enable NVIC
    Interrupt_enableMaster();

    // Infinite loop
    while(1){
    	 front_distance = getFrontDistance();
          printf("front distance: %f \r\n\n", front_distance);
    }

}

// ===== ISR for Port 1 Button Presses ===== //
void PORT1_IRQHandler(void){
	//If port 1.1 button is pressed, toggle enable/disable timer A0 interrupts for PWM direction 1
    if(GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1) == 0){

    	    		a=a^1; //toggle

    	    		if (a==1){
    	    	    GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN2);
    	    		GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN5);

    	    	    	PWM1 = 60000;     // Update PWM1 Value
    	    		    TA0CCR1 = PWM1;  // Save new PWM1 value to register
    	    		    TA0CCR2 = PWM1;
    	    		  //  front_distance = getFrontDistance();
    	    		  //  printf("front distance: %f \r\n\n", front_distance);
      	    	    				}
    	    		if (a==0){
    	    		TA0CCR1 = 0;
    	    	    TA0CCR2 = 0;
    	    	    GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN2);
    	    	    GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN5);

    	    		         }
        GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN1); //Don't forget to debounce; is that the debounce?
    }

    // If button 1.4 is pressed, toggle enable/disable timer A2 interrupts for PWM direction 2
    if(GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4) == 0){

    	    		b=b^1; //toggle

    	    		if (b==1){
    	    		GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN3);
        	        GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN6);

        	        PWM1 = 60000;     // Update PWM1 Value
        	       TA0CCR1 = PWM1;  // Save new PWM1 value to register
        	       TA0CCR2 = PWM1;


    	    	   		        }
    	    		if (b==0) {
    	    		TA0CCR1 = 0;
    	    		TA0CCR2 = 0;
    	    		GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN3);
    	    		GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN6);

    	    		}
    	   GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN4); //Don't forget to debounce;
    }
   GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN1|GPIO_PIN4);   // Clear all interrupts on Port 1 (switch1 & switch2 ...etc)
}


void TA1_0_IRQHandler(void) //Front ultrasonic sensor
{
	frontInterrupt = frontInterrupt + 1; //count the number of time this interrupt has occurred

    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0); //clear timer A1 Register 0 interrupt flag
}
