#ifndef PWM_CTL_H
#define PWM_CTL_H

void fan_pwm_init(void);
void fan_rpm_init(void);
void fan_rpm_task(void *arg);
void fan_temp_task(void *arg);

#endif
