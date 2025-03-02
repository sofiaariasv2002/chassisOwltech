
/**
 * CANTask.c
 *
 * Created on: January 15, 2024
 *     Author: Erick Daniel Ortiz Cervantes
 */

#pragma once

#include "main.h"
#include "cmsis_os.h"

/**
 * @brief Represents a CAN bus message splitted into the velocity
 * in in each motor
 *
 * @param vMotorx: Value to set to the speed controller
 */
typedef struct {
    uint8_t vMotor_FL;
    uint8_t vMotor_FR;
    uint8_t vMotor_BL;
    uint8_t vMotor_BR;
} ChassisControlMessage;

/**
 * @brief Represents a CAN bus message
 *
 * @param node: Node or subsystem where the message is sent to
 * @param vMotorx: Value to set to the speed controller
 */
typedef struct {
    uint8_t vMotor_yaw;
    uint8_t vMotor_pitch;
} GimballControlMessage;


// Message queues from protocols 
extern osPoolId can_rx_mpool;
extern osPoolId can_tx_mpool;

extern osMessageQId outputQueueChassis; 
extern osMessageQId inputQueueChassis; 