#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float kp, ki, kd;
    float setpoint;
    float integral;
    float previous_error;
    float output;
    float output_min, output_max;
    bool first_run;
} pid_controller_t;

typedef struct {
    pid_controller_t pitch_pid;
    pid_controller_t velocity_pid;
    float target_velocity;
    float max_tilt_angle;
} balance_pid_t;

void pid_controller_init(pid_controller_t* pid, float Kp, float Ki, float Kd);
void pid_controller_set_tunings(pid_controller_t* pid, float Kp, float Ki, float Kd);
void pid_controller_set_setpoint(pid_controller_t* pid, float sp);
void pid_controller_set_output_limits(pid_controller_t* pid, float min, float max);
float pid_controller_compute(pid_controller_t* pid, float input, float dt);
void pid_controller_reset(pid_controller_t* pid);

void balance_pid_init(balance_pid_t* balance_pid);
void balance_pid_set_balance_tunings(balance_pid_t* balance_pid, float Kp, float Ki, float Kd);
void balance_pid_set_velocity_tunings(balance_pid_t* balance_pid, float Kp, float Ki, float Kd);
void balance_pid_set_target_velocity(balance_pid_t* balance_pid, float velocity);
void balance_pid_set_max_tilt_angle(balance_pid_t* balance_pid, float angle);
float balance_pid_compute_balance(balance_pid_t* balance_pid, float current_angle, float gyro_rate, float current_velocity, float dt);
void balance_pid_reset(balance_pid_t* balance_pid);

#ifdef __cplusplus
}
#endif

#endif