#include <stdio.h>
#include <stdbool.h>
#include "driverlib.h"
#include "LCD.h"


int temp = 27;
int n=0;

void initTimers(void);

int main(void) {


    WDT_A_hold(WDT_A_BASE);
    FRCTL0 = FRCTLPW | NWAITS_0;
    LCD_init();
    LCD_E_clearMemory(LCD_E_BASE, LCD_E_MEMORY_BLINKINGMEMORY_12, LCD_HEART);
    displayScrollText("WELCOME TO THE AC SYSTEM");
    displayScrollText("ROOM TEMPERATURE IS 27°C");

    GPIO_setAsOutputPin( GPIO_PORT_P1, GPIO_PIN0 );  // Red LED (LED1)
    GPIO_setOutputLowOnPin( GPIO_PORT_P1, GPIO_PIN0 );
    GPIO_setAsOutputPin( GPIO_PORT_P4, GPIO_PIN0 );  // Green LED (LED2)
    GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN0 );
    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P1, GPIO_PIN2 );
    GPIO_selectInterruptEdge ( GPIO_PORT_P1, GPIO_PIN2, GPIO_LOW_TO_HIGH_TRANSITION );
    GPIO_clearInterrupt ( GPIO_PORT_P1, GPIO_PIN2);
    GPIO_enableInterrupt ( GPIO_PORT_P1, GPIO_PIN2 );
	
    PMM_unlockLPM5();

    initTimers();

    __bis_SR_register( GIE );         // Enable interrupts globally


    while (1){

        while(temp > 24){
            LCD_displayNumber(temp);
            GPIO_setOutputLowOnPin( GPIO_PORT_P1, GPIO_PIN0 );
            GPIO_setOutputHighOnPin( GPIO_PORT_P4, GPIO_PIN0 );
            n=0;
            while(n<5);
                temp--;
        }
        GPIO_setOutputLowOnPin( GPIO_PORT_P4, GPIO_PIN0 );
        GPIO_setOutputHighOnPin( GPIO_PORT_P1, GPIO_PIN0 );
        LCD_displayNumber(temp);


    }

    return (0);
}

void initTimers(void)
{
    // Set up the interrupt using CCROIFG to toggle red LED1
    // Set up the interrupt using TA1IFG toggle green LED2
    Timer_A_initUpModeParam initUpParam = { 0 };
    initUpParam.clockSource =                 TIMER_A_CLOCKSOURCE_ACLK;       // Use ACLK (slower clock)
    initUpParam.clockSourceDivider =          TIMER_A_CLOCKSOURCE_DIVIDER_1;  // Input clock = ACLK / 1 = 32KHz
    initUpParam.timerPeriod =                 0xFFFD/2;                       // Half the time
    initUpParam.timerInterruptEnable_TAIE =   TIMER_A_TAIE_INTERRUPT_ENABLE;  // Enable TAR -> 0 interrupt
    initUpParam.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE; //Enable compare interrupt
    initUpParam.timerClear =                  TIMER_A_DO_CLEAR;               // Clear TAR & clock divider
    initUpParam.startTimer =                  false;                          // Don't start the timer, yet

    Timer_A_initUpMode( TIMER_A1_BASE, &initUpParam );
    Timer_A_clearTimerInterrupt( TIMER_A1_BASE );
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0); //Clear CCROIFG

    Timer_A_startCounter(
            TIMER_A1_BASE,
            TIMER_A_UP_MODE
    );
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void CCR0_ISR (void)
{
   n++;  // Toggle LED1 (RED) LED on/off

}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void timer1_ISR (void)
{
    switch( __even_in_range( TA1IV, TA1IV_TAIFG )) {
        case TA1IV_NONE: break;                 // (0x00) None
        case TA1IV_TACCR1:                      // (0x02) CCR1 IFG
            _no_operation();
            break;
        case TA1IV_TACCR2:                      // (0x04) CCR2 IFG
            _no_operation();
            break;
        case TA1IV_3: break;                    // (0x06) Reserved
        case TA1IV_4: break;                    // (0x08) Reserved
        case TA1IV_5: break;                    // (0x0A) Reserved
        case TA1IV_6: break;                    // (0x0C) Reserved
        case TA1IV_TAIFG:                       // (0x0E) TA1IFG - TAR overflow
            _no_operation();//GPIO_toggleOutputOnPin( GPIO_PORT_P4, GPIO_PIN0 );  // Toggle LED2 (Green) LED on/off
            break;
        default: _never_executed();
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void pushbutton_ISR (void) {
    switch( __even_in_range( P1IV, P1IV_P1IFG7 )) {
    case P1IV_NONE:   break;// None
    case P1IV_P1IFG0:       // Pin 0
        __no_operation();
        break;
    case P1IV_P1IFG1:       // Pin 1
        __no_operation();
        break;
    case P1IV_P1IFG2:       // Pin 2 (button 1)
        temp = temp+1;
        LCD_displayNumber(temp);
        break;
    case P1IV_P1IFG3:       // Pin 3
        __no_operation();
        break;
    case P1IV_P1IFG4:       // Pin 4
        __no_operation();
        break;
    case P1IV_P1IFG5:       // Pin 5
        __no_operation();
        break;
    case P1IV_P1IFG6:       // Pin 6
        __no_operation();
        break;
    case P1IV_P1IFG7:       // Pin 7
        __no_operation();
        break;
    default:   _never_executed();
    }
}
