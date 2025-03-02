/*
 * RobomasterCanProtocol.h
 *
 *  Created on: May 24, 2023
 *      Author: @yourName
 */

#ifndef RobomasterCanProtocol_H
#define RobomasterCanProtocol_H

// TODO: Fix assignment
#define CAN_ID_1 0x1U
#define CAN_ID_2 0x2U
#define CAN_ID_3 0x3U
#define CAN_ID_4 0x4U
#define CAN_ID_5 0x5U
#define CAN_ID_6 0x6U
#define CAN_ID_7 0x7U
#define CAN_ID_8 0x8U

// Read information from a given motor
// The MSG is defined by
// FEEDBACK_ID + MotorID
#define FEEDBACK_ID     0x204
#define FEEDBACK_ID_ALT 0x200

// Sent control output to motors
#define CONTROL_ID_1FF 0x1FF
#define CONTROL_ID_2FF 0x2FF
#define CONTROL_ID_200 0x200

// Read motor status
#define CONTROL_ANGLE_BEND 0x0
#define CONTROL_ANGLE_LEND 0x1

#define CONTROL_SPEED_BEND 0x2
#define CONTROL_SPEED_LEND 0x3

// TODO: SET AS TORQUE?
#define READ_CURRENT_BEND 0x4
#define READ_CURRENT_LEND 0x5

#define READ_TEMP 0x6
// Data Length Code 8 BYTES
#define RM_DLC 0x8

typedef struct {
    uint16_t ecd;
    int16_t speed_rpm;
    int16_t current;
    uint8_t temperature;
    int16_t last_ecd;
} rmMotorFeedback;

#endif /* RobomasterCanProtocol_H */