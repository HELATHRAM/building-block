/** Sensor.h
 * Ultrasonic Sensor  Header File
 *
 * rightE - 6.5 > echo
 * rightT - 5.4 > trigger
 *
 */
//------------------------------- GLOBAL VARIABLES --------------------------------------------------------------------

volatile int frontInterrupt = 0;
 int i = 0;

void init_Sensors(void)
{
    /* Configure front ultra sonic */
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN4);//Trig
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);//Trig

    //Set echo pins
    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P6, GPIO_PIN5);//Right Echo
}

int frontTime(void) //get the pulse time of the sensor
{
    int pulsetime = 0;
    pulsetime = frontInterrupt * 1000; // to convert from ms to microsecond
    pulsetime = pulsetime + Timer_A_getCounterValue(TIMER_A1_BASE);
    Timer_A_clearTimer(TIMER_A1_BASE);
 // Delay(3000); //1s delay //(3000000/3000)=1000micorsec????? not sure about that // this step is for timeout
    for (i=0; i<3000; i++){};
    return pulsetime;
}

float getFrontDistance(void)
{
    int pulseduration = 0;
    float dist = 0;
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN4); //generate pulse
   // Delay(30); //10 microseconds with a 3 MHz clock  //(3000000/30)=10micro//we will have to set the trigger high for 10us. This will send an 8 cycle sonic burst at 40 kHz which will hit the object and is then received by the echo.
  for (i=0; i<30; i++){};
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN4);
    while(GPIO_getInputPinValue(GPIO_PORT_P6, GPIO_PIN5) == 0){}; //wait for response _ rising edge
    frontInterrupt = 0; //reset interrupt counter after response
    Timer_A_clearTimer(TIMER_A1_BASE); //clear the timer and start it again to wait for another measurement
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    while(GPIO_getInputPinValue(GPIO_PORT_P6, GPIO_PIN5) == 1){}; //wait for response _ falling edge
    Timer_A_stopTimer(TIMER_A1_BASE); //stop timer
    pulseduration = frontTime();
    dist = (float)pulseduration / 58.0; //divide by 58.0 to get distance in cm //0.034cm/us
    return dist;

}


