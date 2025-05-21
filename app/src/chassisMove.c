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
void normalizeSpeed(gsl_vector* speeds) {
    double max = 0.0;
    for(size_t i = 0; i < speeds->size; i++) {
        double val = fabs(gsl_vector_get(speeds, i));
        if(val > max) max = val;
    }
    
    if(max > maxMotorSpeed_rpm) {
        gsl_vector_scale(speeds, maxMotorSpeed_rpm/max);
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
void chassisMove_thread(void const* arg) {
    control_data* joystick_msg;
    ChassisControlMessage* motor_msg;
    osEvent evt;

    for(;;) {
        // 1. Obtener datos del joystick
        evt = osMessageGet(remoteQueue, osWaitForever);
        if(evt.status != osEventMessage) continue;
        
        joystick_msg = evt.value.p;
        
        // 2. Procesar entradas (normalizar de -100 a 1)
        float vx = joystick_msg->joystickA_x / 100.0f;
        float vy = joystick_msg->joystickA_y / 100.0f;
        float w = joystick_msg->joystickB_x / 100.0f;  // Rotación
        
        osPoolFree(joystick_mpool, joystick_msg);

        // 3. Calcular velocidades
        gsl_vector_set(joystick_input, 0, vx);
        gsl_vector_set(joystick_input, 1, vy);
        gsl_vector_set(joystick_input, 2, w);
        
        gsl_blas_dgemv(CblasNoTrans, 1.0, control_matrix, joystick_input, 0.0, wheel_speed);
        
        // 4. Ajustar a RPM máximas
        gsl_vector_scale(wheel_speed, maxMotorSpeed_rpm);
        normalizeSpeed(wheel_speed);

        // 5. Obtener velocidades actuales
        evt = osMessageGet(outputQueueChassis, 0);  
        if(evt.status == osEventMessage) {
            motor_msg = evt.value.p;
            
            gsl_vector_set(currentMotorSpeeds, 0, motor_msg->vMotor_FL);
            gsl_vector_set(currentMotorSpeeds, 1, motor_msg->vMotor_FR);
            gsl_vector_set(currentMotorSpeeds, 2, motor_msg->vMotor_BL);
            gsl_vector_set(currentMotorSpeeds, 3, motor_msg->vMotor_BR);
            
            osPoolFree(can_rx_mpool, motor_msg);
            
            // 6. Calcular error (P solamente)
            gsl_vector_memcpy(error_speed, wheel_speed);
            gsl_vector_sub(error_speed, currentMotorSpeeds);
        }

        // 7. Enviar nuevas velocidades
        ChassisControlMessage* new_speeds = osPoolAlloc(can_tx_mpool);
        if(new_speeds) {
            new_speeds->vMotor_FL = (int16_t)gsl_vector_get(error_speed, 0);
            new_speeds->vMotor_FR = (int16_t)gsl_vector_get(error_speed, 1);
            new_speeds->vMotor_BL = (int16_t)gsl_vector_get(error_speed, 2);
            new_speeds->vMotor_BR = (int16_t)gsl_vector_get(error_speed, 3);
            
            osMessagePut(inputQueueChassis, (uint32_t)new_speeds, osWaitForever);
        }

        osDelay(10);
    }
}

