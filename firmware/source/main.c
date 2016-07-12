/*******************************************************************************
 *
 * Copyright (c) 2015, Regents of the University of California
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the <organization>.
 * 4. Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ImageProc2.5 Main Loop
 * 'roach' project for Biomimetics Millisystesm Lab
 *
 * This is the main entry point for the 'roach' project, targeting the
 * ImageProc2.5 board and applications with VelociRoACH robots.
 *
 * This code calls to imageproc-lib to do chip, clock, and board setup.
 * After that, module setup functions are called.
 * Finally, the main loop is entered, where low-priority queues are serviced.
 * 
 * Note that a large amount of UART, radio, and cmd module code has crept into 
 * this main loop implementation.
 *
 * Notes:
 *   For minimal startup requirements, see 'imageproc-basic':
 *   https://github.com/biomimetics/imageproc-basic
 * 
 *
*******************************************************************************/

#include <xc.h>
//Library includes
#include "timer.h"
#include <stdlib.h>
//imageproc-lib
#include "init.h"  // TODO : init.h and init.c need to be obsoleted
#include "init_default.h"
#include "utils.h"
#include "radio.h"
#include "tih.h"
#include "ams-enc.h" // JY commented
#include "settings.h"
#include "dfmem.h"
#include "telem.h"
#include "interrupts.h"
#include "mpu6000.h"
#include "sclock.h"
#include "spi_controller.h"
#include "pid-ip2.5.h"
#include "adc_pid.h"
#include "cmd.h"
#include "uart_driver.h"
#include "ppool.h"
#include "carray.h"

// JY edits
#include "sync_servo.h"


volatile MacPacket uart_tx_packet;
volatile unsigned char uart_tx_flag;

int main() {

    // Processor Initialization
    SetupClock();
    SwitchClocks();
    SetupPorts();
    sclockSetup();

    LED_1 = 1;
    LED_2 = 1;
    LED_3 = 1;

    // Message Passing
    cmdSetup();

    // Radio setup
    radioInit(RADIO_RXPQ_MAX_SIZE, RADIO_TXPQ_MAX_SIZE);
    radioSetChannel(RADIO_CHANNEL);
    radioSetSrcAddr(RADIO_SRC_ADDR);
    radioSetSrcPanID(RADIO_PAN_ID);

    //TODO: Move to UART module, or UART init function.
    //uart_tx_packet = NULL;
    //uart_tx_flag = 0;
    //uartInit(&cmdPushFunc);

    // Need delay for encoders to be ready
    delay_ms(100);
    amsEncoderSetup();
    mpuSetup();
    tiHSetup();
    dfmemSetup();
    telemSetup();
    adcSetup();
    pidSetup();

    // JY edits
    servoSetup(); // JY edits for servo
    servoStart(); // JY edits for servo

    // Power down unused modules
    PMD3bits.AD2MD = 1;
    PMD1bits.C1MD = 1;
    PMD1bits.QEIMD = 1;
    PMD3bits.I2C2MD = 1;
    PMD2 = 0xffff; // input/output compare
    PMD1bits.T2MD = 1;
    PMD1bits.T3MD = 1;
    //PMD1bits.T4MD = 1; // JY edits
    PMD1bits.T5MD = 1;
    PMD3bits.T6MD = 1;
    PMD3bits.T7MD = 1;

    LED_1 = 0;
    LED_2 = 0;
    LED_3 = 1;
    while(1){
        // Send outgoing radio packets
        radioProcess();

        //Service pending commands
        cmdHandleRadioRxBuffer();

        // Send outgoing uart packets
//        if(uart_tx_flag) {
//            uartSendPacket(uart_tx_packet);
//            uart_tx_flag = 0;
//        }

        if(radioRxQueueEmpty() && radioTxQueueEmpty())
        {
            //There is no "command queue", only the RadioRxQueue
            Idle(); //Interrupts will bring CPU out of idle in 6 cycles
        }
   
    }
    return 0;
}
