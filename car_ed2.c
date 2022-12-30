/* =====================================================
 Fundamentals of Mechatronics
This program for Turn X% left/right for 10 seconds
*/

//Libraries
#include <msp.h>
#include <driverlib.h>
#include <stdio.h>
#include <stdbool.h>

//Defines
#define timerA_period0    60000 //fixed frequency for PWM1
#define SWITCH GPIO_PORT_P1
#define S1 GPIO_PIN1
#define S2 GPIO_PIN4

int a =0; //Timer A0 Variable used for toggle
int b=0; //Timer A2 Variable used for toggle
volatile unsigned int result1 = 0;
volatile int PWM1 = 0;
int count=0;


//Timer A0 configuration
const Timer_A_UpModeConfig upConfig_0 = // Configure Timer A counter in Up mode
		{ TIMER_A_CLOCKSOURCE_SMCLK,  // Tie Timer A0 to sub-master clock (SMCLK)
			TIMER_A_CLOCKSOURCE_DIVIDER_1, // Increment counter every 1 clock cycles; Means counter is incremented at 3E+6Hz
			timerA_period0, // Period of Timer A0 (this value placed in TA0CCR0); To define a timer frequency=3000000/60000=50Hz
			TIMER_A_TAIE_INTERRUPT_DISABLE, // Disable Timer A rollover interrupt
			TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE, // Enable Capture Compare interrupt
			TIMER_A_DO_CLEAR            // Clear counter upon initialization
		};

// Main Loop
void main(){

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

    Interrupt_disableMaster();
    ///// *** Configure Timer A0 and its interrupt *** /////
    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig_0);
    Interrupt_disableInterrupt(INT_TA0_0);
    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);

    ///// *** ANALOG to DIGITAL CONVERSION SETUP *** /////
    ADC14_enableModule(); // Enable and initialize ADC14
    ADC14_initModule(ADC_CLOCKSOURCE_SMCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P5, GPIO_PIN5, GPIO_TERTIARY_MODULE_FUNCTION);  // Set up ADC14 GPIO pin
    ADC14_setResolution(ADC_14BIT);     // Configure ADC output
    ADC14_configureMultiSequenceMode(ADC_MEM0,ADC_MEM2,true);  // Configure ADC sampling
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A0, false); // Configure ADC conversion
    ADC14_enableSampleTimer(ADC_MANUAL_ITERATION);
    ADC14_enableInterrupt(ADC_INT0);  // Configure ADC interrupt
    ADC14_enableConversion(); // Enable ADC operation and conversion
    ADC14_toggleConversionTrigger();

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
    Interrupt_setPriority(INT_PORT1,1);
    // 6. Enable interrupts on those ports
    Interrupt_enableInterrupt(INT_PORT1);
    // 7. Enable NVIC
    Interrupt_enableMaster();

    // Infinite loop
    while(1){}

}

// ===== ISR for Port 1 Button Presses ===== //
void PORT1_IRQHandler(void){
	//If port 1.1 button is pressed, toggle enable/disable timer A0 interrupts for PWM direction 1
    if(GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN1) == 0){

    	    		a=a^1; //toggle
    	    		//printf("a1 is %i \r\n",a);
    	    		if (a==1){
    	    			count=0;
    	    	    GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN2);
    	    		GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN5);
    	    		Interrupt_enableInterrupt(INT_TA0_0);   // Enable Timer A0 interrupt
    	    	    	    				}
    	    		if (a==0){
    	    		TA0CCR1 = 0;
    	    	    TA0CCR2 = 0;
    	    	    GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN2);
    	    	    GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN5);
    	    		Interrupt_disableInterrupt(INT_TA0_0);   // Disable Timer A0 interrupt
    	    	    //printf("a2 is %i \r\n",a);
    	    		         }
        GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN1); //Don't forget to debounce; is that the debounce?
    }

    // If button 1.4 is pressed, toggle enable/disable timer A2 interrupts for PWM direction 2
    if(GPIO_getInputPinValue(GPIO_PORT_P1,GPIO_PIN4) == 0){

    	    		b=b^1; //toggle
    	    		//printf("b1 is %i \r\n",b);
    	    		if (b==1){
    	    			count=0;
    	    	    GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN3);
        	        GPIO_setOutputHighOnPin(GPIO_PORT_P3,GPIO_PIN6);
    	    	    Interrupt_enableInterrupt(INT_TA0_0);   // Enable Timer A2 interrupt


    	    	   		        }
    	    		if (b==0) {
    	    		TA0CCR1 = 0;
    	    		TA0CCR2 = 0;
    	    		GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN3);
    	    		GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN6);
    				Interrupt_disableInterrupt(INT_TA0_0);   // Disable Timer A0 interrupt

    				//printf("b2 is %i \r\n",b);
    	    		}
    	   GPIO_setOutputHighOnPin(GPIO_PORT_P1,GPIO_PIN4); //Don't forget to debounce;
    }
   GPIO_clearInterruptFlag(GPIO_PORT_P1,GPIO_PIN1|GPIO_PIN4);   // Clear all interrupts on Port 1 (switch1 & switch2 ...etc)
}

//////////////// *** Timer A0 Interrupt Service Routine Used for Direction 1 Control *** ///////////////////
void TA0_0_IRQHandler(void){
count++;

	if (count<500) {
    ADC14_toggleConversionTrigger();     // Start ADC conversion
    while(ADC14_isBusy()){}              // Use polling to wait for the ADC14 to finish sampling
    result1 = ADC14_getResult(ADC_MEM0);       // Retrive and convert results stored in ADC_MEM0
  //  printf("ADC Inputs1: %i \r\n", result1) ;   // Display result1
    PWM1 = 60000*(int)result1/16384;     // Update PWM1 Value
    TA0CCR1 = PWM1;  // Save new PWM1 value to register
    TA0CCR2 = 0;

  //  printf("PWM Value1: %i \r\n\n", PWM1) ; // Verify PWM1 values
	}

	else {
		count=0;
		GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN2);
		GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN3);
		GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN5);
		GPIO_setOutputLowOnPin(GPIO_PORT_P3,GPIO_PIN6);
		 TA0CCR1 = 0;
		 TA0CCR2 = 0;
	}

	Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0); // Clear Timer A0 Interrupt Flag

}
