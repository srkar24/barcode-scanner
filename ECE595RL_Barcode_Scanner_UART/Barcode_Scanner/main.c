/**
 * @file main.c
 * @brief Main source code for the Barcode_Scanner program.
 *
 * This file contains the main entry point and function definitions for the Barcode_Scanner program.
 *
 * The following EUSCI_A modules are used:
 *  - EUSCI_A0_UART: Used to provide UART communication using the USB and for printf implementation
 *  - EUSCI_A2_UART: Used to interface the Barcode Scanner module
 *  - EUSCI_A3_UART: Used for observing UART TX data on the oscilloscope and for loopback testing
 *
 * This lab interfaces with the SparkFun 2D Barcode Scanner module that will be used to read a barcode
 * or a QR code. For more information regarding the Barcode Scanner module, refer to the following link:
 * Link: https://www.mouser.com/new/sparkfun/sparkfun-2d-barcode-scanner-board/
 *
 * The Piezo Buzzer is also used to generate tones.
 *
 * @note Derived from Tymofii and Eduardo's final project (Group 11) in ECE 595R/L during Fall 2023.
 * The original project used the following components:
 *  - Barcode Scanner (UART)
 *  - Piezo Buzzer (GPIO)
 *  - Nokia5110 LCD (SPI)
 *  - DC Motors (GPIO + PWM)
 *
 * @author Srushti Wadekar, Arshia P, Aaron Nanas
 *
 */

#include <stdint.h>
#include "msp.h"
#include "inc/Clock.h"
#include "inc/CortexM.h"
#include "inc/GPIO.h"
#include "inc/SysTick_Interrupt.h"
#include "inc/Bumper_Switches.h"
#include "inc/Motor.h"
#include "inc/EUSCI_A0_UART.h"
#include "inc/Buzzer.h"
#include "inc/Barcode_Scanner.h"
#include "inc/EUSCI_A3_UART.h"

//#define EUSCI_A3_UART_TEST 1
//#define EUSCI_A3_UART_LOOPBACK 1
#define BARCODE_SCANNER 1

/**
 * @brief Processes barcode scanner commands and takes corresponding actions
 *
 * This function checks the Barcode_Scanner_Buffer for specific commands and performs
 * the associated actions. Supported commands include changing the color of an RGB LED,
 * playing a note pattern, and controlling the movement of a robot.
 *
 * @param Barcode_Scanner_Buffer The buffer containing the received barcode scanner command.
 *
 * @return None
 */
void Process_Barcode_Scanner_Command(char Barcode_Scanner_Buffer[])
{
    if (Check_Barcode_Scanner_Command(Barcode_Scanner_Buffer, "RGB LED GREEN"))
    {
        LED2_Output(RGB_LED_GREEN);
    }

    else if (Check_Barcode_Scanner_Command(Barcode_Scanner_Buffer, "RGB LED BLUE"))
    {
        LED2_Output(RGB_LED_BLUE);
    }

    else if (Check_Barcode_Scanner_Command(Barcode_Scanner_Buffer, "RGB LED RED"))
    {
        LED2_Output(RGB_LED_RED);
    }

    else if (Check_Barcode_Scanner_Command(Barcode_Scanner_Buffer, "RGB LED OFF"))
    {
        LED2_Output(RGB_LED_OFF);
    }

    else if (Check_Barcode_Scanner_Command(Barcode_Scanner_Buffer, "PLAY NOTE PATTERN"))
    {
        Play_Note_Pattern();
    }

    else if(Check_Barcode_Scanner_Command(Barcode_Scanner_Buffer, "MOVE FORWARD"))
    {
        Motor_Forward(4500, 4500);
        Clock_Delay1ms(3000);
        Motor_Stop();
    }

    else if(Check_Barcode_Scanner_Command(Barcode_Scanner_Buffer, "MOVE BACKWARD"))
    {
        Motor_Backward(4500, 4500);
        Clock_Delay1ms(3000);
        Motor_Stop();
    }

    else if(Check_Barcode_Scanner_Command(Barcode_Scanner_Buffer, "SPIN CLOCKWISE"))
    {
        Motor_Right(3000, 3000);
        Clock_Delay1ms(3000);
        Motor_Stop();
    }

    else if(Check_Barcode_Scanner_Command(Barcode_Scanner_Buffer, "SPIN COUNTERCLOCKWISE"))
    {
        Motor_Left(3000, 3000);
        Clock_Delay1ms(3000);
        Motor_Stop();
    }

    else
    {
        printf("Barcode Scanner Command Invalid!\n");
    }
}

int main(void)
{
    // Initialize the 48 MHz Clock
    Clock_Init48MHz();

    // Initialize the built-in red LED and the RGB LEDs
    LED1_Init();
    LED2_Init();

    // Initialize the user buttons
    Buttons_Init();

    // Initialize EUSCI_A0_UART to use the printf function
    EUSCI_A0_UART_Init_Printf();

#if defined EUSCI_A3_UART_TEST || EUSCI_A3_UART_LOOPBACK
    #if defined BARCODE_SCANNER
        #error "Only EUSCI_A3_UART_TEST, EUSCI_A3_UART_LOOPBACK, or BARCODE_SCANNER can be active at the same time."
    #endif

    // Use a flag to run a specific code block once
    uint8_t run_once = 0x01;

    // Initialize local variables to store UART TX and RX Data
    uint8_t UART_TX_Data = 0x00;
    uint8_t UART_RX_Data = 0x00;

    // Initialize buffers for UART TX and RX Data
    uint8_t TX_Buffer[BUFFER_LENGTH];
    uint8_t RX_Buffer[BUFFER_LENGTH];

    // Initialize the EUSCI_A3 module to use UART mode
    EUSCI_A3_UART_Init();

#elif defined BARCODE_SCANNER
    #if defined EUSCI_A3_UART_TEST || EUSCI_A3_UART_LOOPBACK
        #error "Only EUSCI_A3_UART_TEST, EUSCI_A3_UART_LOOPBACK, or BARCODE_SCANNER can be active at the same time."
    #endif

    // Initialize a buffer to store the received UART data from the Barcode Scanner module
    char Barcode_Scanner_Buffer[BARCODE_SCANNER_BUFFER_SIZE] = {0};

    // Initialize the Piezo Buzzer
    Buzzer_Init();

    // Initialize the Barcode Scanner module which uses the EUSCI_A2 module in UART mode
    Barcode_Scanner_Init();

    // Initialize the DC motors
    Motor_Init();

    // Enable the interrupts used by the SysTick and Timer_A timers
    EnableInterrupts();

#else
    #error "Define either one of the two options: EUSCI_A3_UART_TEST, EUSCI_A3_UART_LOOPBACK, or BARCODE_SCANNER"
#endif

    while(1)
    {

#if defined EUSCI_A3_UART_TEST
    #if defined BARCODE_SCANNER || EUSCI_A3_UART_LOOPBACK
        #error "Only EUSCI_A3_UART_TEST, EUSCI_A3_UART_LOOPBACK, or BARCODE_SCANNER can be active at the same time."
    #endif

        UART_TX_Data = EUSCI_A3_UART_Transmit_Data();
        Clock_Delay1ms(100);

#elif defined EUSCI_A3_UART_LOOPBACK
    #if defined EUSCI_A3_UART_TEST || BARCODE_SCANNER
        #error "Only EUSCI_A3_UART_TEST, EUSCI_A3_UART_LOOPBACK, or BARCODE_SCANNER can be active at the same time."
    #endif

//        UART_TX_Data = EUSCI_A3_UART_Transmit_Data();
//        UART_RX_Data = EUSCI_A3_UART_InChar();
//        printf("TX Data: 0x%02X | RX Data: 0x%02X\n", UART_TX_Data, UART_RX_Data);
//        Clock_Delay1ms(100);

        if (run_once == 0x01)
        {
            run_once = 0x00;

            // Print a message to indicate the start of the loopback test
            printf("Loopback Test Started\n");

            // Perform a loopback test
            EUSCI_A3_UART_Ramp_Data(TX_Buffer, RX_Buffer);
            EUSCI_A3_UART_Validate_Data(TX_Buffer, RX_Buffer);

            // Print a message to indicate the end of the loopback test
            printf("Loopback Test Ended\n");
        }

#elif defined BARCODE_SCANNER
    #if defined EUSCI_A3_UART_TEST || EUSCI_A3_UART_LOOPBACK
        #error "Only EUSCI_A3_UART_TEST, EUSCI_A3_UART_LOOPBACK, or BARCODE_SCANNER can be active at the same time."
    #endif

        int string_size = Barcode_Scanner_Read(Barcode_Scanner_Buffer, BARCODE_SCANNER_BUFFER_SIZE);

        printf("Barcode Scanner Command: ");
        for (int i = 0; i < string_size; i++)
        {
            printf("%c", Barcode_Scanner_Buffer[i]);
        }
        printf("\n");

        Process_Barcode_Scanner_Command(Barcode_Scanner_Buffer);

#else
    #error "Define either one of the two options: EUSCI_A3_UART_TEST, EUSCI_A3_UART_LOOPBACK, or BARCODE_SCANNER"
#endif
    }
}
