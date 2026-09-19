#include <adc.h>
#include <rcc.h>
#include <gpio.h>
#include <system_init.h>

static const uint8_t ir_channels[ADC_CH_NUM] = {0, 1, 2, 3, 4};

void adc_init(void) {
    rcc_enable_adc(1);
    rcc_enable_gpio(GPIOA_EN);

    for(int i = 0; i < ADC_CH_NUM; i++) {
        gpio_set_mode(GPIOA_BASE, ir_channels[i], 3);
    }

    ADC_CR1(ADC1_BASE) &= ~ADC_CR1_RES;
    ADC_CR1(ADC1_BASE) |= (0 << 24); // Set it for 12 bits

    ADC_CR2(ADC1_BASE) |= ADC_CR2_ADON;
    ADC_CR2(ADC1_BASE) &= ~ADC_CR2_ALIGN;
    ADC_CR2(ADC1_BASE) &= ~ADC_CR2_CONT;

    for(int i = 0; i < ADC_CH_NUM; i++) {
        ADC_SMPR2(ADC1_BASE) |= (7 << (3 * ir_channels[i]));
    }

    ADC_SQR3(ADC1_BASE) &= ~0x1FFFFFF;

    for(int i = 0; i < ADC_CH_NUM; i++) {
        ADC_SQR3(ADC1_BASE) |= ((uint32_t)ir_channels[i] << (5 * i));
    }

    ADC_SQR1(ADC1_BASE) &= ~(0xF << 20);
    ADC_SQR1(ADC1_BASE) |= ((ADC_CH_NUM - 1) << 20);
}

uint16_t adc_read_channel(uint8_t channel) {
    ADC_SQR3(ADC1_BASE) &= ~0x1F;
    ADC_SQR3(ADC1_BASE) |= channel;

    while(1) {
        ADC_CR2(ADC1_BASE) |= ADC_CR2_SWSTART;
        while(!(ADC_SR(ADC1_BASE) & ADC_SR_EOC))
            ; // Wait until conversion is complete
        return (uint16_t)ADC_DR(ADC1_BASE);
    }
}

void adc_read(uint16_t out[ADC_CH_NUM]) {
    for(int i = 0; i < ADC_CH_NUM; i++) {
        out[i] = adc_read_channel(ir_channels[i]);
    }
}