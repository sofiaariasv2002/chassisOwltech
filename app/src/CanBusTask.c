/*
 * BaseFileC.h
 *
 *  Created on: May 24, 2023
 *      Author: @yourName
 */

#ifndef CANBUSTASK_C
#define CANBUSTASK_C

#include "CanBusTask.h"
#ifdef __cplusplus
extern "C" {
#endif

// YOUR CODE
// We need at least the following principles:
// * StartReceiveInt: Sets up an interrupt-based reception for USART2 and configures the necessary variables used by the
// ISR for the transfer.
// * StartCAN4Traffic:  Starts a continuous stream of data transmitted from UART4 to be received by USART2 (provided the
// jumpers are correctly set).
// * canActionTask: This function initializes USART2 and associated hardware, starts a reception, and waits for
// completion (with a deadline of 100 ms). The complete message is either printed or a timeout occurs and an error is
// printed.
// * CAN2_IRQHandler: An ISR is issued when an interrupt occurs for the USART2 peripheral.

#ifdef __cplusplus
}
#endif

#endif /* TemplateCFile_C */

osSemaphoreId_t I2C_semaphore;
I2C_HandleTypeDef hi2c1;

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->State == HAL_I2C_STATE_READY) {
        osSemaphoreRelease(I2C_semaphore);
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->State == HAL_I2C_STATE_READY) {
        osSemaphoreRelease(I2C_semaphore);
    }
}

static void Task1(void* argument) {
    while (1) {
        HAL_I2C_Master_Transmit_IT(&hi2c1, I2C_addr, write_data_array, sizeof(write_data_array));
        osSemaphoreAcquire(I2C_semaphore, 100);
        HAL_I2C_Master_Receive_IT(&hi2c1, I2C_addr, read_data_array, sizeof(read_data_array));
        osSemaphoreAcquire(I2C_semaphore, 100);
    }
}

osSemaphoreId_t I2C_semaphore;
I2C_HandleTypeDef hi2c1;

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->State == HAL_I2C_STATE_READY) {
        osSemaphoreRelease(I2C_semaphore);
    }
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->State == HAL_I2C_STATE_READY) {
        osSemaphoreRelease(I2C_semaphore);
    }
}

static void Task1(void* argument) {
    while (1) {
        HAL_I2C_Master_Transmit_IT(&hi2c1, I2C_addr, write_data_array, sizeof(write_data_array));
        osSemaphoreAcquire(I2C_semaphore, 100);
        HAL_I2C_Master_Receive_IT(&hi2c1, I2C_addr, read_data_array, sizeof(read_data_array));
        osSemaphoreAcquire(I2C_semaphore, 100);
    }
}