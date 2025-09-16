#include "pid_controller.h"
#include <math.h>
#include <string.h>

void pid_controller_init(pid_controller_t* pid, float Kp, float Ki, float Kd) {
    pid->kp = Kp;
    pid->ki = Ki;
    pid->kd = Kd;
    pid->setpoint = 0.0f;
    pid->integral = 0.0f;
    pid->previous_error = 0.0f;
    pid->output = 0.0f;
    pid->output_min = -255.0f;
    pid->output_max = 255.0f;
    pid->last_time = 0;
    pid->first_run = true;
}

void pid_controller_set_tunings(pid_controller_t* pid, float Kp, float Ki, float Kd) {
    pid->kp = Kp;
    pid->ki = Ki;
    pid->kd = Kd;
}

void pid_controller_set_setpoint(pid_controller_t* pid, float sp) {
    pid->setpoint = sp;
}

void pid_controller_set_output_limits(pid_controller_t* pid, float min, float max) {
    pid->output_min = min;
    pid->output_max = max;
    
    if (pid->output > pid->output_max) pid->output = pid->output_max;
    else if (pid->output < pid->output_min) pid->output = pid->output_min;
    
    if (pid->integral > pid->output_max) pid->integral = pid->output_max;
    else if (pid->integral < pid->output_min) pid->integral = pid->output_min;
}

float pid_controller_compute(pid_controller_t* pid, float input) {
#ifdef NATIVE_BUILD
    static uint32_t tick = 0;
    uint32_t now = ++tick;
#else
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
#endif
    
    if (pid->first_run) {
        pid->last_time = now;
        pid->previous_error = pid->setpoint - input;
        pid->first_run = false;
        return 0.0f;
    }
    
    float time_change = (float)(now - pid->last_time) / 1000.0f;
    if (time_change <= 0.0f) return pid->output;
    
    float error = pid->setpoint - input;
    
    pid->integral += error * time_change;
    if (pid->integral > pid->output_max) pid->integral = pid->output_max;
    else if (pid->integral < pid->output_min) pid->integral = pid->output_min;
    
    float derivative = (error - pid->previous_error) / time_change;
    
    pid->output = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;
    
    if (pid->output > pid->output_max) pid->output = pid->output_max;
    else if (pid->output < pid->output_min) pid->output = pid->output_min;
    
    pid->previous_error = error;
    pid->last_time = now;
    
    return pid->output;
}

void pid_controller_reset(pid_controller_t* pid) {
    pid->integral = 0.0f;
    pid->previous_error = 0.0f;
    pid->output = 0.0f;
    pid->first_run = true;
}

void balance_pid_init(balance_pid_t* balance_pid) {
    pid_controller_init(&balance_pid->pitch_pid, 50.0f, 0.0f, 2.0f);
    pid_controller_init(&balance_pid->velocity_pid, 1.0f, 0.1f, 0.0f);
    
    balance_pid->target_velocity = 0.0f;
    balance_pid->max_tilt_angle = 45.0f;
    
    pid_controller_set_output_limits(&balance_pid->pitch_pid, -255.0f, 255.0f);
    pid_controller_set_output_limits(&balance_pid->velocity_pid, -10.0f, 10.0f);
}

void balance_pid_set_balance_tunings(balance_pid_t* balance_pid, float Kp, float Ki, float Kd) {
    pid_controller_set_tunings(&balance_pid->pitch_pid, Kp, Ki, Kd);
}

void balance_pid_set_velocity_tunings(balance_pid_t* balance_pid, float Kp, float Ki, float Kd) {
    pid_controller_set_tunings(&balance_pid->velocity_pid, Kp, Ki, Kd);
}

void balance_pid_set_target_velocity(balance_pid_t* balance_pid, float velocity) {
    balance_pid->target_velocity = velocity;
    pid_controller_set_setpoint(&balance_pid->velocity_pid, velocity);
}

void balance_pid_set_max_tilt_angle(balance_pid_t* balance_pid, float angle) {
    balance_pid->max_tilt_angle = angle;
}

float balance_pid_compute_balance(balance_pid_t* balance_pid, float current_angle, float gyro_rate, float current_velocity) {
    if (fabsf(current_angle) > balance_pid->max_tilt_angle) {
        return 0.0f; // Robot has fallen, stop motors
    }
    
    float velocity_adjustment = pid_controller_compute(&balance_pid->velocity_pid, current_velocity);
    
    pid_controller_set_setpoint(&balance_pid->pitch_pid, velocity_adjustment);
    float motor_output = pid_controller_compute(&balance_pid->pitch_pid, current_angle);
    
    return motor_output;
}

void balance_pid_reset(balance_pid_t* balance_pid) {
    pid_controller_reset(&balance_pid->pitch_pid);
    pid_controller_reset(&balance_pid->velocity_pid);
}