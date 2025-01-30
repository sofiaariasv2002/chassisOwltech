#include "chassisMove.hpp"

/**
 * @brief Constructor de la clase chassisMove.
 * 
 * Inicializa los punteros de motor, las velocidades máximas, y crea las colas de envío y recepción
 * 
 * cada instancia de la clase necesita tener su propia cola privada. 
 * Esto asegura que cada objeto de tipo chassisMove maneje su propia cola,
 * sin interferir con otras instancias.
 * 
*/
chassisMove::chassisMove(IntfMotor* leftFrontMotor, IntfMotor* rightFrontMotor,
                         IntfMotor* leftBackMotor, IntfMotor* rightBackMotor, 
                         float maxMotorSpeed_rpm)
    : leftFrontMotor(leftFrontMotor), rightFrontMotor(rightFrontMotor),
      leftBackMotor(leftBackMotor), rightBackMotor(rightBackMotor),
      maxMotorSpeed_rpm(maxMotorSpeed_rpm) {
    sendQueue = xQueueCreate(10, sizeof(TDB));
    receiveQueue = xQueueCreate(10, sizeof(TDB));
    if (sendQueue == NULL || receiveQueue == NULL) {
        printf("Error: No se pudieron crear las colas.\n");
    }
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
 * @param wheel_speed Vector de velocidades de los motores (Eigen::VectorXf).
 * @return Un vector Eigen::VectorXf con las velocidades normalizadas.
 */
Eigen::VectorXf chassisMove::normalizeSpeed(const Eigen::VectorXf& wheel_speed) {
    float maxSpeed = wheel_speed.cwiseAbs().maxCoeff();
    if (maxSpeed > maxMotorSpeed_rpm) {
        float scale = maxMotorSpeed_rpm / maxSpeed; 
        return wheel_speed * scale; // Escala todo el vector manteniendo los signos
    }
    return wheel_speed;
}


/**
 * @brief Envía datos de velocidad de motores a la cola especificada.
 * 
 * @param queue Cola donde se enviarán los datos.
 * @param data Estructura TDB con las velocidades de los motores.
 * @param xTicksToWait Tiempo de espera máximo en ticks.
 * @return `pdPASS` si el envío fue exitoso, `pdFAIL` en caso contrario.
 */
BaseType_t chassisMove::xQueueSend(QueueHandle_t queue, const TDB* data, TickType_t xTicksToWait) {
    if (xQueueSend(queue, (void*)data, xTicksToWait) != pdPASS) {
        printf("Error: No se pudo enviar los datos a la cola de CAN.\n");
        return pdFAIL;
    }
    return pdPASS;
}

/**
 * @brief Recibe datos de velocidad de motores desde la cola especificada.
 * 
 * @param queue Cola desde donde se recibirán los datos.
 * @param data Estructura TDB donde se almacenarán las velocidades recibidas.
 * @param xTicksToWait Tiempo de espera máximo en ticks.
 * @return `pdPASS` si la recepción fue exitosa, `pdFAIL` en caso contrario.
 */
BaseType_t chassisMove::xQueueReceive(QueueHandle_t queue, TDB* data, TickType_t xTicksToWait) {
    if (xQueueReceive(queue, (void*)data, xTicksToWait) != pdPASS) {
        printf("Error: No se pudo recibir los datos de la cola de CAN.\n");
        return pdFAIL;
    }
    return pdPASS;
}

/**
 * @brief Convierte las entradas de los joysticks en velocidades de los motores.
 * 
 * Calcula las velocidades para cada rueda en función de las entradas de joystick, 
 * las normaliza y las envía a la cola de envío.
 * 
 * @param x1 Entrada del joystick 1 (eje X para desplazamiento en el plano horizontal).
 * @param y1 Entrada del joystick 1 (eje Y para desplazamiento en el plano vertical).
 * @param x2 Entrada del joystick 2 (eje X para control de torsión).
 * @param y2 Entrada del joystick 2 (eje Y para control de torsión).
 * @param w Ángulo de orientación del joystick2 (en radianes).
 */
void chassisMove::chassisMoveThread(float x1, float y1, float x2, float y2) {
    // Cálculo del ángulo deseado 
    float w=atan2(y2, x2);
    // u
    Eigen::Vector3f joystick_input(x1, y1, w);
    // M
    Eigen::MatrixXf control_matrix(4, 3);
    control_matrix << -1, -1,  CHASSIS_RADIUS,  // Delantera izquierda
                       1, -1,  CHASSIS_RADIUS,  // Delantera derecha
                       1,  1,  CHASSIS_RADIUS,  // Trasera derecha
                      -1,  1,  CHASSIS_RADIUS;  // Trasera izquierda

    //v=M*u
    Eigen::VectorXf wheel_speed = control_matrix * joystick_input;
    wheel_speed = normalizeSpeed(wheel_speed);

    TDB currentSpeeds;
    if (xQueueReceive(receiveQueue, &currentSpeeds, portMAX_DELAY) == pdPASS) {
        Eigen::VectorXf currentMotorSpeeds(4);
        currentMotorSpeeds << currentSpeeds.motor1, 
                              currentSpeeds.motor2, 
                              currentSpeeds.motor3, 
                              currentSpeeds.motor4;

    Eigen::Vector4f error_speed = wheel_speed.cast<float>() - currentMotorSpeeds;
    leftFrontMotor->actuate(error_speed[0]);   // Delantera izquierda
    rightFrontMotor->actuate(error_speed[1]);  // Delantera derecha
    rightBackMotor->actuate(error_speed[2]);   // Trasera derecha
    leftBackMotor->actuate(error_speed[3]);    // Trasera izquierda

        TDB error_TDB = {error_speed[0], error_speed[1], error_speed[2], error_speed[3]};
        xQueueSend(sendQueue, &error_TDB, portMAX_DELAY);
    } else {
        printf("Error: No se pudieron obtener las velocidades actuales desde la cola de CAN.\n");
    }
}



void chassisMove::stop() {
    leftFrontMotor->stop(0);
    rightFrontMotor->stop(0);
    leftBackMotor->stop(0);
    rightBackMotor->stop(0);
}
