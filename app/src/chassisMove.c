/*
 * chassisMove.c
 *
 *  Created on: Jan 02, 2025
 *      Author: @sofiaariasv2002
 *
 * For mor information:
 * https://learning.oreilly.com/library/view/wheeled-mobile-robotics/9780128042380/B9780128042045000020_1.xhtml#s0070
 * https://www.robomaster.com/en-US/products/components/general/M3508
 *
 */

#include "chassisMove.h"

#include "CommunicationStructs.h"

// ######################################### variables #########################################################

static osThreadId chassisMoveThread;  // ID for thread

static float maxMotorSpeed_rpm;

// Estructuras GSL
gsl_vector* joystick_input;
gsl_matrix* control_matrix;
gsl_vector* wheel_speed;
gsl_vector* currentMotorSpeeds;
gsl_vector* error_speed;

// #######################################################################################################

/**
 * @brief Constructor de la clase chassisMove.
 *
 * Inicializa los punteros de motor, las velocidades máximas, y crea las colas de envío y recepción
 *
 * cada instancia de la clase necesita tener su propia cola privada.
 * Esto asegura que cada objeto de tipo chassisMove maneje su propia cola,
 * sin interferir con otras instancias.
 *
 **/
void chassisMove_init(float maxMotorSpeed_rpm) {
    maxMotorSpeed_rpm = maxMotorSpeed_rpm;
    // Inicializar la matriz de control
    control_matrix = gsl_matrix_alloc(4, 3);
    gsl_matrix_set(control_matrix, 0, 0, -1);  // Delantera izquierda
    gsl_matrix_set(control_matrix, 0, 1, -1);
    gsl_matrix_set(control_matrix, 0, 2, CHASSIS_RADIUS);
    gsl_matrix_set(control_matrix, 1, 0, 1);  // Delantera derecha
    gsl_matrix_set(control_matrix, 1, 1, -1);
    gsl_matrix_set(control_matrix, 1, 2, CHASSIS_RADIUS);
    gsl_matrix_set(control_matrix, 2, 0, 1);  // Trasera derecha
    gsl_matrix_set(control_matrix, 2, 1, 1);
    gsl_matrix_set(control_matrix, 2, 2, CHASSIS_RADIUS);
    gsl_matrix_set(control_matrix, 3, 0, -1);  // Trasera izquierda
    gsl_matrix_set(control_matrix, 3, 1, 1);
    gsl_matrix_set(control_matrix, 3, 2, CHASSIS_RADIUS);

    /*
    control_matrix << -1, -1,  CHASSIS_RADIUS,  // Delantera izquierda
                        1, -1,  CHASSIS_RADIUS,  // Delantera derecha
                        1,  1,  CHASSIS_RADIUS,  // Trasera derecha
                    -1,  1,  CHASSIS_RADIUS;  // Trasera izquierda

    */

    // Inicializar los vectores
    joystick_input = gsl_vector_alloc(3);
    wheel_speed = gsl_vector_alloc(4);
    currentMotorSpeeds = gsl_vector_alloc(4);
    error_speed = gsl_vector_alloc(4);
    // enviar por la otra queue al currentMotorSpeed
}

/**
 * @brief Normaliza las velocidades de los motores para que ninguna exceda el rango permitido.
 *
 * Si alguna velocidad supera el rango (-maxMotorSpeed_rpm a maxMotorSpeed_rpm),
 * se escalan todas proporcionalmente manteniendo sus signos originales.
 *
 * @param currentSpeeds Estructura TDB que contiene las velocidades de los motores.
 * @return Una estructura TDB con las velocidades normalizadas.
 */
/**
 * @brief Normaliza el vector de velocidades de las ruedas para que ninguna supere el rango permitido.
 *
 * Si alguna velocidad en el vector excede el rango (-maxMotorSpeed_rpm a maxMotorSpeed_rpm),
 * todo el vector se escala proporcionalmente manteniendo los signos.
 *
 * @param wheel_speed Vector de velocidades de los motores.
 */
void normalizeSpeed(gsl_vector* wheel_speed) {
    double max_speed = gsl_vector_max(wheel_speed);
    if (max_speed > maxMotorSpeed_rpm) {
        gsl_vector_scale(wheel_speed, maxMotorSpeed_rpm / max_speed);
    }
}

/**
 * @brief Convierte las entradas de los joysticks en velocidades de los motores.
 *
 * @param x1 Entrada del joystick 1 (eje X para desplazamiento en el plano horizontal).
 * @param y1 Entrada del joystick 1 (eje Y para desplazamiento en el plano vertical).
 * @param x2 Entrada del joystick 2 (eje X para control de torsión).
 * @param y2 Entrada del joystick 2 (eje Y para control de torsión).
 */
void chassisMove(float x1, float y1, float x2, float y2) {
    ChassisControlMessage* rptr;
    osEvent evt;

    // TODO: checar si es la pool correcta
    // TODO: porque se recibe los valores dos veces??
    for (;;) {
        evt = osMessageGet(outputQueueChassis, osWaitForever);  // wait for message
        if (evt.status == osEventMessage) {
            rptr = evt.value.p;
            printf("\nCurrent motor1: %u\n", rptr->vMotor_FL);
            printf("Current motor2: %u\n", rptr->vMotor_FR);
            printf("Current motor3: %u\n", rptr->vMotor_BL);
            printf("Current motor4: %u\n", rptr->vMotor_BR);
            osPoolFree(can_rx_mpool, rptr);  // free memory allocated for message
        }
    }

    // Cálculo del ángulo deseado
    float w = atan2_approx(y2, x2);

    // Asignar los valores del joystick al vector (ya inicializado)
    gsl_vector_set(joystick_input, 0, x1);  // Eje X
    gsl_vector_set(joystick_input, 1, y1);  // Eje Y
    gsl_vector_set(joystick_input, 2, w);   // Ángulo

    // Calcular las velocidades de las ruedas: wheel_speed = control_matrix * joystick_input
    gsl_blas_dgemv(CblasNoTrans, 1.0, control_matrix, joystick_input, 0.0, wheel_speed);

    // Normalizar las velocidades
    normalizeSpeed(wheel_speed);

    // Recibir las velocidades actuales de los motores desde la cola del RTOS a vector gsl
    evt = osMessageGet(outputQueueChassis, osWaitForever);
    if (evt.status == osEventMessage) {
        ChassisControlMessage* currentSpeeds = (ChassisControlMessage*)evt.value.p;
        gsl_vector_set(currentMotorSpeeds, 0, currentSpeeds->vMotor_FL);
        gsl_vector_set(currentMotorSpeeds, 1, currentSpeeds->vMotor_FR);
        gsl_vector_set(currentMotorSpeeds, 2, currentSpeeds->vMotor_BL);
        gsl_vector_set(currentMotorSpeeds, 3, currentSpeeds->vMotor_BR);
        osPoolFree(can_rx_mpool, currentSpeeds);

        // Calcular el error de velocidad
        gsl_vector_memcpy(error_speed, wheel_speed);      // Copia wheel_speed (velocidades deseadas) = error_speed.
        gsl_vector_sub(error_speed, currentMotorSpeeds);  // error_speed=wheel_speed−currentMotorSpeeds

        // Enviar velocidades corregidas a CAN (inputQueueChassis)
        ChassisControlMessage* msg = (ChassisControlMessage*)osPoolAlloc(can_tx_mpool);
        if (msg != NULL) {
            msg->vMotor_FL = gsl_vector_get(wheel_speed, 0);
            msg->vMotor_FR = gsl_vector_get(wheel_speed, 1);
            msg->vMotor_BL = gsl_vector_get(wheel_speed, 2);
            msg->vMotor_BR = gsl_vector_get(wheel_speed, 3);
            osMessagePut(inputQueueChassis, (uint32_t)msg, osWaitForever);
        }
    }
}
