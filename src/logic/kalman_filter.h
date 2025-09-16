#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float Q_angle;
    float Q_bias;
    float R_measure;
    float angle;
    float bias;
    float rate;
    float P[2][2];
    float K[2];
    float y;
    float S;
} kalman_filter_t;

void kalman_filter_init(kalman_filter_t* kf);
void kalman_filter_set_angle(kalman_filter_t* kf, float angle);
float kalman_filter_get_angle(kalman_filter_t* kf, float new_angle, float new_rate, float dt);
void kalman_filter_set_qangle(kalman_filter_t* kf, float Q_angle);
void kalman_filter_set_qbias(kalman_filter_t* kf, float Q_bias);
void kalman_filter_set_rmeasure(kalman_filter_t* kf, float R_measure);

#ifdef __cplusplus
}
#endif

#endif