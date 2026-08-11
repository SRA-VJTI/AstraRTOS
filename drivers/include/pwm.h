#ifndef PWM_H
#define PWM_H

#include <stdint.h>

#define CH1 1
#define CH2 2
#define CH3 3
#define CH4 4

#define AF0 0b0000
#define AF1 0b0001
#define AF2 0b0010
#define AF3 0b0011
#define AF9 0b1001

#define PWM_MODE1 0b110
#define PWM_MODE2 0b111

#define TIM_CR1(base) (*(volatile uint32_t *)((base) + 0x00))
#define TIM_CNT(base) (*(volatile uint32_t *)((base) + 0x24))
#define TIM_ARR(base) (*(volatile uint32_t *)((base) + 0x2C))
#define TIM_PSC(base) (*(volatile uint32_t *)((base) + 0x28))
#define TIM_EGR(base) (*(volatile uint32_t *)((base) + 0x14))
#define TIM_CCR1(base) (*(volatile uint32_t *)((base) + 0x34))
#define TIM_CCR2(base) (*(volatile uint32_t *)((base) + 0x38))
#define TIM_CCR3(base) (*(volatile uint32_t *)((base) + 0x3C))
#define TIM_CCR4(base) (*(volatile uint32_t *)((base) + 0x40))
#define TIM_CCMR1(base) (*(volatile uint32_t *)((base) + 0x18))
#define TIM_CCMR2(base) (*(volatile uint32_t *)((base) + 0x1C))
#define TIM_CCER(base) (*(volatile uint32_t *)((base) + 0x20))
#define TIM_BDTR(base) (*(volatile uint32_t *)((base) + 0x44))

typedef struct pwm_config_t {
    uint8_t channel;
    uint32_t timer;
    uint32_t port;
    uint32_t pin;
    uint32_t freq;
    uint32_t pwm_mode;
} pwm_config_t;

int pwm_set_af(pwm_config_t *config);
int pwm_configure_channel(uint32_t channel, uint32_t timer, uint32_t PWM_MODE);
int pwm_set_compare_value(uint32_t compare_value, uint32_t channel, uint32_t timer);
void pwm_enable_timer(uint32_t timer);
void pwm_config_timer(uint32_t timer, uint32_t freq);
int os_pwm_init(pwm_config_t *config);
void os_pwm_set_duty(pwm_config_t *config, uint8_t duty);
void os_pwm_set_pulse_us(pwm_config_t *config, uint32_t pulse_us);
#endif
