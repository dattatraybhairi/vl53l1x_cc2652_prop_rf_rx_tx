/*
 * Copyright (c) 2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/***** Includes *****/
/* Standard C Libraries */
#include <stdlib.h>
#include <unistd.h>

/* TI Drivers */
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
// #include <ti/drivers/SPI.h>
// #include <ti/drivers/UART.h>
// #include <ti/drivers/Watchdog.h>

/* Driverlib Header files */
#include DeviceFamily_constructPath(driverlib/rf_prop_mailbox.h)

/* Board Header files */
#include "ti_drivers_config.h"
#include <ti_radio_config.h>

/* vl53l1x Header Files */
#include "vl53l1x/VL53L1X_api.h"
/***** Defines *****/

/*************vl53l1x variables declarations***********/
const uint16_t dev = 0x29;
static uint8_t byteData;
static uint8_t sensorState;
static uint16_t wordData;
static uint16_t Distance;
static uint8_t RangeStatus;
static uint8_t dataReady;
static int8_t status;
/********************************************************/
Semaphore_Handle getdistance;
/*function Prototypes*/
static void delay(uint32_t duration); static void i2cScanner(I2C_Handle i2cHandle);

/*I2C Instance Creation*/
I2C_Handle i2c; I2C_Params i2cParams; I2C_Transaction i2cTransaction;

/* Do power measurement */
//#define POWER_MEASUREMENT
/* Packet TX Configuration */
#define PAYLOAD_LENGTH      2
#ifdef POWER_MEASUREMENT
#define PACKET_INTERVAL     5  /* For power measurement set packet interval to 5s */
#else
#define PACKET_INTERVAL     500000  /* Set packet interval to 500000us or 500ms */
#endif

/***** Prototypes *****/
extern void* vl53l1x_task(void *arg0);

/***** Variable declarations *****/
static RF_Object rfObject; static RF_Handle rfHandle;

/* Pin driver handle */
static PIN_Handle ledPinHandle; static PIN_State ledPinState;

static uint8_t packet[PAYLOAD_LENGTH];
static uint16_t seqNumber;

/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 */
PIN_Config pinTable[] = { CONFIG_PIN_GLED | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX
,PIN_TERMINATE
};

/***** Function definitions *****/

void* mainThread(void *arg0)
{
    RF_Params rfParams;
    RF_Params_init(&rfParams);

    /* Open LED pins */
    ledPinHandle = PIN_open(&ledPinState, pinTable);
    if (ledPinHandle == NULL)
    {
        while (1)
            ;
    }

    RF_cmdPropTx.pktLen = PAYLOAD_LENGTH;
    RF_cmdPropTx.pPkt = packet;
    RF_cmdPropTx.startTrigger.triggerType = TRIG_NOW;

    /* Request access to the radio */
#if defined(DeviceFamily_CC26X0R2)
    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdPropRadioSetup, &rfParams);
#else
    rfHandle = RF_open(&rfObject, &RF_prop,
                       (RF_RadioSetup*) &RF_cmdPropRadioDivSetup, &rfParams);
#endif// DeviceFamily_CC26X0R2

    /* Set the frequency */
    RF_postCmd(rfHandle, (RF_Op*) &RF_cmdFs, RF_PriorityNormal, NULL, 0);

    while (1)
    {
        printf("Running task1 function\n");

//        if (Semaphore_getCount(getdistance) == 0)
//        {
//            printf("Sem blocked in task1\n");
//        }

        Semaphore_pend(getdistance, BIOS_WAIT_FOREVER);

        /* Create packet with incrementing sequence number and random payload */
        packet[0] = Distance >> 8;
        packet[1] = Distance & 0x00ff ;
        //uint8_t i;
//        for (i = 2; i < PAYLOAD_LENGTH; i++)
//        {
//            packet[i] = rand();
//        }

        /* Send packet */
        RF_EventMask terminationReason = RF_runCmd(rfHandle,
                                                   (RF_Op*) &RF_cmdPropTx,
                                                   RF_PriorityNormal, NULL, 0);

        Semaphore_post(getdistance);

        switch (terminationReason)
        {
        case RF_EventLastCmdDone:
            // A stand-alone radio operation command or the last radio
            // operation command in a chain finished.
            break;
        case RF_EventCmdCancelled:
            // Command cancelled before it was started; it can be caused
            // by RF_cancelCmd() or RF_flushCmd().
            break;
        case RF_EventCmdAborted:
            // Abrupt command termination caused by RF_cancelCmd() or
            // RF_flushCmd().
            break;
        case RF_EventCmdStopped:
            // Graceful command termination caused by RF_cancelCmd() or
            // RF_flushCmd().
            break;
        default:
            // Uncaught error event
            while (1)
                ;
        }

        uint32_t cmdStatus = ((volatile RF_Op*) &RF_cmdPropTx)->status;
        switch (cmdStatus)
        {
        case PROP_DONE_OK:
            // Packet transmitted successfully
            break;
        case PROP_DONE_STOPPED:
            // received CMD_STOP while transmitting packet and finished
            // transmitting packet
            break;
        case PROP_DONE_ABORT:
            // Received CMD_ABORT while transmitting packet
            break;
        case PROP_ERROR_PAR:
            // Observed illegal parameter
            break;
        case PROP_ERROR_NO_SETUP:
            // Command sent without setting up the radio in a supported
            // mode using CMD_PROP_RADIO_SETUP or CMD_RADIO_SETUP
            break;
        case PROP_ERROR_NO_FS:
            // Command sent without the synthesizer being programmed
            break;
        case PROP_ERROR_TXUNF:
            // TX underflow observed during operation
            break;
        default:
            // Uncaught error event - these could come from the
            // pool of states defined in rf_mailbox.h
            while (1)
                ;
        }

#ifndef POWER_MEASUREMENT
        PIN_setOutputValue(ledPinHandle, CONFIG_PIN_GLED,
                           !PIN_getOutputValue(CONFIG_PIN_GLED));
#endif
        /* Power down the radio */
        RF_yield(rfHandle);

#ifdef POWER_MEASUREMENT
        /* Sleep for PACKET_INTERVAL s */
        sleep(PACKET_INTERVAL);
#else
        /* Sleep for PACKET_INTERVAL us */
        usleep(PACKET_INTERVAL);
#endif

    }
}

/*********************************************************************
 * @fn      vl53l1x_app
 *
 * @brief   Task to read the data from the VL53L1X TOF Sensor.
 *
 * @param   none
 *
 * @return  none
 */

void* vl53l1x_app(void *arg0)
{

    I2C_init();
    /* Create I2C for usage */
    printf("this is task\n");
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_100kHz;
    i2c = I2C_open(CONFIG_I2C_0, &i2cParams);

    /* Scan the Address of the available I2C devices */
    i2cScanner(i2c);
    // Those basic I2C read functions can be used to check your own I2C functions */
    status = VL53L1_RdByte(i2c, dev, 0x010F, &byteData);
    delay(2);
    //printf("VL53L1X Model_ID: %X\n", byteData);
    status = VL53L1_RdByte(i2c, dev, 0x0110, &byteData);
    delay(2);
    //printf("VL53L1X Module_Type: %X\n", byteData);
    status = VL53L1_RdWord(i2c, dev, 0x010F, &wordData);
    //printf("VL53L1X: %X\n", wordData);
    while (sensorState == 0)
    {
        status = VL53L1X_BootState(i2c, dev, &sensorState);
        delay(2);
    }
    //printf("Chip booted\n");

    /* Initialize and configure the device according to people counting need */
    status = VL53L1X_SensorInit(i2c, dev);
    delay(2);
    status += VL53L1X_SetDistanceMode(i2c, dev, 2); /* 1=short, 2=long */
    delay(2);
    status += VL53L1X_SetTimingBudgetInMs(i2c, dev, 20); /* in ms possible values [20, 50, 100, 200, 500] */
    delay(2);
    status += VL53L1X_SetInterMeasurementInMs(i2c, dev, 20);
    delay(2);
    if (status != 0)
    {
        //printf("Error in Initialization or configuration of the device\n");
    }

    /* This function has to be called to enable the ranging */
    status = VL53L1X_StartRanging(i2c, dev);
    while (1)
    {
        printf("Running task2 function\n");

        if (Semaphore_getCount(getdistance) == 0)
        {
            printf("Sem blocked in task2\n");
        }

        /* Get access to resource */
        //Semaphore_pend(getdistance, BIOS_WAIT_FOREVER);

        printf("thisis task inside while 1\n");

        status = VL53L1X_CheckForDataReady(i2c, dev, &dataReady);
        if (dataReady == 0)
        {
            //return;
        }
        dataReady = 0;
        status += VL53L1X_GetRangeStatus(i2c, dev, &RangeStatus);
        status += VL53L1X_GetDistance(i2c, dev, &Distance);

        /* clear interrupt has to be called to enable next interrupt*/
        status += VL53L1X_ClearInterrupt(i2c, dev);

        if (status != 0)
        {
            //printf("Error in operating the device\n");
        }
        printf("Distance : %d\n ", Distance);

        Semaphore_post(getdistance);

        //sleep(1);
    }

    /* Deinitialized I2C */
    //I2C_close (i2c);
}

/*********************************************************************
 * @fn      i2cScanner
 *
 * @brief   scans the i2c bus for devices
 *
 * @param   i2cHandle
 *
 * @return  none
 */
static void i2cScanner(I2C_Handle i2cHandle)
{
    //Scan the bus for I2c devices.
    uint8_t result = 0;
    int nDevices = 0;
    I2C_Transaction i2cTransaction = { 0 };
    uint8_t i;
    for (i = 1; i < 127; i++)
    {
        i2cTransaction.slaveAddress = i;
        i2cTransaction.writeBuf = 0;
        i2cTransaction.writeCount = 1;
        i2cTransaction.readBuf = NULL;
        i2cTransaction.readCount = 0;
        result = I2C_transfer(i2c, &i2cTransaction);
        if (result)
        {
            nDevices++;
            printf("Device add : %02X\n", i);
        }
    }
    printf("found %d devices\n", nDevices);
}

/*********************************************************************
 * @fn      delay
 *
 * @brief   delay in milliseconds
 *
 * @param   duration in milliseconds
 *
 * @return  none
 */
static void delay(uint32_t duration)
{
    //iterator
    uint32_t i = 0;
    //enter while loop and execute NOP cycles
    while (i < (1000 * duration))
    {
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        asm(" NOP");
        i++;
    }

}

