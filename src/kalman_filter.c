#include "kalman_filter.h"

void kalman_filter_init(kalman_filter_t* kf) {
    kf->Q_angle = 0.001f;
    kf->Q_bias = 0.003f;
    kf->R_measure = 0.03f;

    kf->angle = 0.0f;
    kf->bias = 0.0f;

    kf->P[0][0] = 0.0f;
    kf->P[0][1] = 0.0f;
    kf->P[1][0] = 0.0f;
    kf->P[1][1] = 0.0f;
}

void kalman_filter_set_angle(kalman_filter_t* kf, float angle) {
    kf->angle = angle;
}

float kalman_filter_get_angle(kalman_filter_t* kf, float new_angle, float new_rate, float dt) {
    kf->rate = new_rate - kf->bias;
    kf->angle += dt * kf->rate;

    kf->P[0][0] += dt * (dt * kf->P[1][1] - kf->P[0][1] - kf->P[1][0] + kf->Q_angle);
    kf->P[0][1] -= dt * kf->P[1][1];
    kf->P[1][0] -= dt * kf->P[1][1];
    kf->P[1][1] += kf->Q_bias * dt;

    kf->S = kf->P[0][0] + kf->R_measure;
    kf->K[0] = kf->P[0][0] / kf->S;
    kf->K[1] = kf->P[1][0] / kf->S;

    kf->y = new_angle - kf->angle;

    kf->angle += kf->K[0] * kf->y;
    kf->bias += kf->K[1] * kf->y;

    float P00_temp = kf->P[0][0];
    float P01_temp = kf->P[0][1];

    kf->P[0][0] -= kf->K[0] * P00_temp;
    kf->P[0][1] -= kf->K[0] * P01_temp;
    kf->P[1][0] -= kf->K[1] * P00_temp;
    kf->P[1][1] -= kf->K[1] * P01_temp;

    return kf->angle;
}

void kalman_filter_set_qangle(kalman_filter_t* kf, float Q_angle) {
    kf->Q_angle = Q_angle;
}

void kalman_filter_set_qbias(kalman_filter_t* kf, float Q_bias) {
    kf->Q_bias = Q_bias;
}

void kalman_filter_set_rmeasure(kalman_filter_t* kf, float R_measure) {
    kf->R_measure = R_measure;
}