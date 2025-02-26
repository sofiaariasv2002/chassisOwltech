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
/**
 * @brief Constructor de la clase chassisMove.
 *
 * Inicializa los punteros de motor, las velocidades máximas, y crea las colas de envío y recepción.
 */
chassisMove::chassisMove(IntfMotor* leftFrontMotor, IntfMotor* rightFrontMotor, IntfMotor* leftBackMotor,
                         IntfMotor* rightBackMotor, float maxMotorSpeed_rpm)
    : leftFrontMotor(leftFrontMotor),
      rightFrontMotor(rightFrontMotor),
      leftBackMotor(leftBackMotor),
      rightBackMotor(rightBackMotor),
      maxMotorSpeed_rpm(maxMotorSpeed_rpm) {
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

    // Crear las colas
    sendQueueCAN = xQueueCreate(10, sizeof(TDB));
    receiveQueueCAN = xQueueCreate(10, sizeof(TDB));
    if (sendQueueCAN == NULL || receiveQueueCAN == NULL) {
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
 * @param wheel_speed Vector de velocidades de los motores.
 */
void chassisMove::normalizeSpeed(gsl_vector* wheel_speed) {
    double max_speed = gsl_vector_max(wheel_speed);
    if (max_speed > maxMotorSpeed_rpm) {
        gsl_vector_scale(wheel_speed, maxMotorSpeed_rpm / max_speed);
    }
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
 * @param x1 Entrada del joystick 1 (eje X para desplazamiento en el plano horizontal).
 * @param y1 Entrada del joystick 1 (eje Y para desplazamiento en el plano vertical).
 * @param x2 Entrada del joystick 2 (eje X para control de torsión).
 * @param y2 Entrada del joystick 2 (eje Y para control de torsión).
 */
void chassisMove::chassisMoveThread(float x1, float y1, float x2, float y2) {
    // Verificar que los punteros a los motores no sean nulos
    if (!leftFrontMotor || !rightFrontMotor || !leftBackMotor || !rightBackMotor) {
        printf("Error: Punteros a motores no inicializados.\n");
        return;
    }

    // Cálculo del ángulo deseado
    float w = atan2(y2, x2);

    // Asignar los valores del joystick al vector (ya inicializado)
    gsl_vector_set(joystick_input, 0, x1);  // Eje X
    gsl_vector_set(joystick_input, 1, y1);  // Eje Y
    gsl_vector_set(joystick_input, 2, w);   // Ángulo

    // Calcular las velocidades de las ruedas: wheel_speed = control_matrix * joystick_input
    gsl_blas_dgemv(CblasNoTrans, 1.0, control_matrix, joystick_input, 0.0, wheel_speed);

    // Normalizar las velocidades
    normalizeSpeed(wheel_speed);

    // Recibir las velocidades actuales de los motores desde la cola del RTOS a vector gsl
    TDB currentSpeeds;
    if (xQueueReceive(receiveQueueCAN, &currentSpeeds, portMAX_DELAY) == pdPASS) {
        // Asignar las velocidades actuales al vector
        gsl_vector_set(currentMotorSpeeds, 0, currentSpeeds.motor1);
        gsl_vector_set(currentMotorSpeeds, 1, currentSpeeds.motor2);
        gsl_vector_set(currentMotorSpeeds, 2, currentSpeeds.motor3);
        gsl_vector_set(currentMotorSpeeds, 3, currentSpeeds.motor4);

        // Calcular el error de velocidad
        gsl_vector_memcpy(
            error_speed,
            wheel_speed);  // Copia el contenido del vector wheel_speed (velocidades deseadas) al vector error_speed.
        gsl_vector_sub(
            error_speed,
            currentMotorSpeeds);  // Resta el vector currentMotorSpeeds (velocidades actuales) del vector error_speed.

        // Actuar sobre los motores
        leftFrontMotor->actuate(gsl_vector_get(error_speed, 0));   // Delantera izquierda
        rightFrontMotor->actuate(gsl_vector_get(error_speed, 1));  // Delantera derecha
        rightBackMotor->actuate(gsl_vector_get(error_speed, 2));   // Trasera derecha
        leftBackMotor->actuate(gsl_vector_get(error_speed, 3));    // Trasera izquierda

        // Enviar el error de velocidad a la cola
        TDB error_TDB = {gsl_vector_get(error_speed, 0), gsl_vector_get(error_speed, 1), gsl_vector_get(error_speed, 2),
                         gsl_vector_get(error_speed, 3)};
        xQueueSend(sendQueueCAN, &error_TDB, portMAX_DELAY);
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
