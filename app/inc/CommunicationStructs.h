
/**
 * CANTask.c
 *
 * Created on: January 15, 2024
 *     Author: Erick Daniel Ortiz Cervantes
 */

#pragma once

#include "cmsis_os.h"
#include "main.h"

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

typedef struct control_data {
    int8_t joystickA_x;
    int8_t joystickA_y;
    int8_t joystickB_x;
    int8_t joystickB_y;
    int8_t knobA;
    int8_t knobB;
    int8_t switchA;
    int8_t switchB;
    int8_t switchC;
    int8_t switchD;
} control_data;

extern osMessageQId remoteQueue;
extern osPoolId joystick_mpool;

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

extern osMessageQId outputQueueChassis;  // chassis: me envia el can
extern osMessageQId inputQueueChassis;   // chassis: lo envio al vcan