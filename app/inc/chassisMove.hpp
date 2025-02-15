 /*
 * chassisMove.hpp
 *
 *  Created on: Jan 02, 2025
 *      Author: @sofiaariasv2002
 *              @anaValeria098       
 * 
 * For mor information:
 * https://learning.oreilly.com/library/view/wheeled-mobile-robotics/9780128042380/B9780128042045000020_1.xhtml#s0070
 * https://www.robomaster.com/en-US/products/components/general/M3508
 *
 */


#ifndef CHASSIS_MOVE_HPP
#define CHASSIS_MOVE_HPP

#include <Eigen/Dense> 
#include "IntfMotor.hpp" 
#include "ControllerCAN.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define CHASSIS_RADIUS 0.3f  // Radio del chasis (distancia del centro a una rueda) en metros
#define MAX_MOTOR_SPEED 465.0f // Velocidad máxima del motor rpm
#define PI 3.14159265358979323846

/**
 * @brief Estructura Vector de velocidades de los motores.
 */
struct TDB {
    int motor1; // Velocidad motor delantero izquierdo
    int motor2; // Velocidad motor delantero derecho
    int motor3; // Velocidad motor trasero derecho
    int motor4; // Velocidad motor trasero izquierdo
};

/**
 * @brief Clase para controlar el movimiento de un chasis mecanum utilizando joysticks.
 * 
 * Esta clase permite controlar el movimiento de un robot con chasis mecanum. La clase proporciona 
 * métodos para convertir las entradas de joystick en velocidades de motor, además de normalizar 
 * las velocidades y calcular la torsión en base a las entradas de control mediante colas de FreeRtos
 */
class chassisMove {
private:
    IntfMotor* leftFrontMotor;
    IntfMotor* rightFrontMotor;
    IntfMotor* leftBackMotor;
    IntfMotor* rightBackMotor;
    float maxMotorSpeed_rpm; 

    QueueHandle_t sendQueueCAN; 
    QueueHandle_t receiveQueueCAN;

    Eigen normalizeSpeed(const Eigen::VectorXf& wheel_speed);

    BaseType_t xQueueSend(
        QueueHandle_t queue, 
        const TDB* data, 
        TickType_t xTicksToWait);
        
    BaseType_t xQueueReceive(
        QueueHandle_t queue, 
        TDB* data, 
        TickType_t xTicksToWait);


public:
    chassisMove(IntfMotor* leftFrontMotor, IntfMotor* rightFrontMotor,
                IntfMotor* leftBackMotor, IntfMotor* rightBackMotor, 
                float maxMotorSpeed_rpm = MAX_MOTOR_SPEED);

    void chassisMoveThread(float x1, float y1, float x2, float y2);

    void stop();
};

#endif // CHASSIS_MOVE_HPP
