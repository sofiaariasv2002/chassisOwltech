

#ifndef CHASSIS_MOVE_H
#define CHASSIS_MOVE_H

#include <gsl/gsl_blas.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

#include "cmsis_os.h"

#define CHASSIS_RADIUS  0.3f    // Radio del chasis (distancia del centro a una rueda) en metros
#define MAX_MOTOR_SPEED 465.0f  // Velocidad máxima del motor rpm
#define PI              3.14159265358979323846
#define PI_2            (PI / 2)
#define PI_4            (PI / 4)

float atan_approx(float z) {
    // Coeficientes de la aproximación de atan(x) (Hart's approximation)
    const float a1 = 0.9998660f, a3 = -0.3302995f, a5 = 0.1801410f;
    const float a7 = -0.0851330f, a9 = 0.0208351f;

    // atan(z) ≈ z * (a1 + a3*z^2 + a5*z^4 + a7*z^6 + a9*z^8)
    float z2 = z * z;
    return z * (a1 + z2 * (a3 + z2 * (a5 + z2 * (a7 + z2 * a9))));
}

float atan2_approx(float y, float x) {
    if (x == 0) {
        return (y > 0) ? PI_2 : (y < 0 ? -PI_2 : 0);
    }

    float abs_y = (y > 0) ? y : -y;
    float angle;

    if (x > 0) {
        angle = atan_approx(abs_y / x);
    } else {
        angle = PI - atan_approx(abs_y / -x);
    }

    return (y < 0) ? -angle : angle;
}

void normalizeSpeed(gsl_vector* wheel_speed);

void chassisMove_init(float maxMotorSpeed_rpm);

// void chassisMove(void const* argument);
void chassisMove();

// void send_thread(void const* argument);

// void recv_thread(void const* argument);

#endif  // CHASSIS_MOVE_H
