#include "pwm.h"
#include "rcc.h"
#include "gpio.h"

#define APB1_Hz rcc_get_apb1_freq()
#define APB2_Hz rcc_get_apb2_freq()

int pwm_set_af(pwm_config_t *config) {
    uint32_t timer = config->timer;
    switch(timer) {
    case TIM1_BASE:
        gpio_set_af(config->port, config->pin, AF1);
        break;
    case TIM2_BASE:
        gpio_set_af(config->port, config->pin, AF1);
        break;
    case TIM3_BASE:
        gpio_set_af(config->port, config->pin, AF2);
        break;
    case TIM4_BASE:
        gpio_set_af(config->port, config->pin, AF2);
        break;
    case TIM5_BASE:
        gpio_set_af(config->port, config->pin, AF2);
        break;
    case TIM8_BASE:
        gpio_set_af(config->port, config->pin, AF3);
        break;
    case TIM9_BASE:
        gpio_set_af(config->port, config->pin, AF3);
        break;
    case TIM10_BASE:
        gpio_set_af(config->port, config->pin, AF3);
        break;
    case TIM11_BASE:
        gpio_set_af(config->port, config->pin, AF3);
        break;
    case TIM12_BASE:
        gpio_set_af(config->port, config->pin, AF9);
        break;
    case TIM13_BASE:
        gpio_set_af(config->port, config->pin, AF9);
        break;
    case TIM14_BASE:
        gpio_set_af(config->port, config->pin, AF9);
        break;
    default:
        return -1;
    }
    return 0;
}

int pwm_configure_channel(uint32_t channel, uint32_t timer, uint32_t PWM_MODE) {
    switch(channel) {
    case CH1:
        TIM_CCMR1(timer) &= ~(7 << 4);
        TIM_CCMR1(timer) |= (PWM_MODE << 4) | (1 << 3);
        TIM_CCER(timer) |= (1 << 0);
        break;
    case CH2:
        TIM_CCMR1(timer) &= ~(7 << 12);
        TIM_CCMR1(timer) |= (PWM_MODE << 12) | (1 << 11);
        TIM_CCER(timer) |= (1 << 4);
        break;
    case CH3:
        TIM_CCMR2(timer) &= ~(7 << 4);
        TIM_CCMR2(timer) |= (PWM_MODE << 4) | (1 << 3);
        TIM_CCER(timer) |= (1 << 8);
        break;
    case CH4:
        TIM_CCMR2(timer) &= ~(7 << 12);
        TIM_CCMR2(timer) |= (PWM_MODE << 12) | (1 << 11);
        TIM_CCER(timer) |= (1 << 12);
        break;
    default:
        return -1;
    }
    return 0;
}

int pwm_set_compare_value(uint32_t compare_value, uint32_t channel, uint32_t timer) {
    switch(channel) {
    case CH1:
        TIM_CCR1(timer) = compare_value;
        break;
    case CH2:
        TIM_CCR2(timer) = compare_value;
        break;
    case CH3:
        TIM_CCR3(timer) = compare_value;
        break;
    case CH4:
        TIM_CCR4(timer) = compare_value;
        break;
    default:
        return -1;
    }
    return 0;
}

void pwm_enable_timer(uint32_t timer) {
    if(timer == TIM1_BASE || timer == TIM8_BASE) {
        TIM_BDTR(timer) |= (1 << 15); // main output enable (MOE)
    }
    TIM_EGR(timer) = (1 << 0);  // set update event flag
    TIM_CR1(timer) |= (1 << 0); // counter enable
}

void pwm_config_timer(uint32_t timer, uint32_t freq) {
    if(freq == 0)
        return;
    if(timer == TIM1_BASE || timer == TIM8_BASE || timer == TIM9_BASE || timer == TIM10_BASE ||
       timer == TIM11_BASE) {
        TIM_PSC(timer) = (APB2_Hz / 1000000) - 1;
        TIM_ARR(timer) = (1000000 / freq) - 1;
    }
    else {
        TIM_PSC(timer) = (APB1_Hz / 1000000) - 1;
        TIM_ARR(timer) = (1000000 / freq) - 1;
    }
}

int os_pwm_init(pwm_config_t *config) {
    uint32_t timer = config->timer;
    gpio_set_mode(config->port, config->pin, GPIO_MODE_AF);
    rcc_enable_timer(config->timer);
    pwm_set_af(config);
    pwm_config_timer(timer, config->freq);
    pwm_configure_channel(config->channel, timer, config->pwm_mode);
    pwm_enable_timer(timer);
    return 0;
}

void os_pwm_set_duty(pwm_config_t *config, uint8_t duty) {
    uint32_t timer = config->timer;
    uint32_t compare_value = ((uint32_t)(TIM_ARR(timer)) * duty) / 100;
    pwm_set_compare_value(compare_value, config->channel, timer);
}

void os_pwm_set_pulse_us(pwm_config_t *config, uint32_t pulse_us) {
    uint32_t timer = config->timer;
    uint32_t compare_value = pulse_us;
    pwm_set_compare_value(compare_value, config->channel, timer);
}
