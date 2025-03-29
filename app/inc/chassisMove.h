

#ifndef CHASSIS_MOVE_H
#define CHASSIS_MOVE_H

#include <gsl_blas.h>
#include <gsl_matrix.h>
#include <gsl_vector.h>

#include "cmsis_os.h"

#define CHASSIS_RADIUS  0.3f    // Radio del chasis (distancia del centro a una rueda) en metros
#define MAX_MOTOR_SPEED 465.0f  // Velocidad máxima del motor rpm
#define PI              3.14159265358979323846

typedef struct ChassisControlMessage ChassisControlMessage;

void normalizeSpeed(gsl_vector* wheel_speed);

void chassisMove_init(float maxMotorSpeed_rpm);

void chassisMoveThread(float x1, float y1, float x2, float y2);

void send_thread(void const* argument);

void recv_thread(void const* argument);

#endif  // CHASSIS_MOVE_H
