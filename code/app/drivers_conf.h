
#ifndef _DRIVERS_CONF_H
#define _DRIVERS_CONF_H

/**
 * @brief   Enables the Drivers.
 */
#if !defined(DRIVER_USE_DAC) || defined(__DOXYGEN__)
#define DRIVER_USE_DAC                 FALSE
#endif

#if !defined(DRIVER_USE_IWDG) || defined(__DOXYGEN__)
#define DRIVER_USE_IWDG                 TRUE
#endif

#if !defined(DRIVER_USE_TIMCAP) || defined(__DOXYGEN__)
#define DRIVER_USE_TIMCAP                 TRUE
#endif

#if !defined(DRIVER_USE_IUART) || defined(__DOXYGEN__)
#define DRIVER_USE_IUART                 FALSE
#endif

#if !defined(DRIVER_USE_EICU) || defined(__DOXYGEN__)
#define DRIVER_USE_EICU                FALSE
#endif

#if !defined(DRIVER_USE_EEPROM) || defined(__DOXYGEN__)
#define DRIVER_USE_EEPROM           TRUE
#endif


/*
 * DAC driver system settings.
 */
#define STM32_DAC_USE_CHN1                  TRUE
#define STM32_DAC_USE_CHN2                  TRUE
#define STM32_DAC_CHN1_IRQ_PRIORITY         10
#define STM32_DAC_CHN2_IRQ_PRIORITY         10
#define STM32_DAC_CHN3_IRQ_PRIORITY         10
#define STM32_DAC_CHN1_DMA_PRIORITY         2
#define STM32_DAC_CHN2_DMA_PRIORITY         2
#define STM32_DAC_CHN3_DMA_PRIORITY         2
#define STM32_DAC_CHN1_DMA_STREAM           STM32_DMA_STREAM_ID(2, 3)
#define STM32_DAC_CHN2_DMA_STREAM           STM32_DMA_STREAM_ID(2, 4)
#define STM32_DAC_CHN3_DMA_STREAM           STM32_DMA_STREAM_ID(2, 5)

/*
 * TIMCAP driver system settings.
 */
#define STM32_TIMCAP_USE_TIM1                  FALSE
#define STM32_TIMCAP_USE_TIM2                  FALSE
#define STM32_TIMCAP_USE_TIM3                  TRUE
#define STM32_TIMCAP_TIM1_IRQ_PRIORITY         2
#define STM32_TIMCAP_TIM2_IRQ_PRIORITY         2
#define STM32_TIMCAP_TIM3_IRQ_PRIORITY         2

/*
 * IUART driver system settings.
 */
#define STM32_IUART_USE_USART1             FALSE
#define STM32_IUART_USE_USART2             FALSE
#define STM32_IUART_USART1_IRQ_PRIORITY      3
#define STM32_IUART_USART2_IRQ_PRIORITY      3

/*
 * Extended ICU driver system settings.
 */
#define STM32_EICU_USE_TIM1                 FALSE
#define STM32_EICU_USE_TIM2                 FALSE
#define STM32_EICU_USE_TIM3                 FALSE
#define STM32_EICU_USE_TIM4                 FALSE
#define STM32_EICU_USE_TIM5                 FALSE
#define STM32_EICU_USE_TIM8                 FALSE
#define STM32_EICU_USE_TIM9                 FALSE
#define STM32_EICU_USE_TIM12               FALSE
#define STM32_EICU_TIM1_IRQ_PRIORITY        3
#define STM32_EICU_TIM2_IRQ_PRIORITY        3
#define STM32_EICU_TIM3_IRQ_PRIORITY        3
#define STM32_EICU_TIM4_IRQ_PRIORITY        3
#define STM32_EICU_TIM5_IRQ_PRIORITY        3
#define STM32_EICU_TIM8_IRQ_PRIORITY        3
#define STM32_EICU_TIM9_IRQ_PRIORITY        3
#define STM32_EICU_TIM12_IRQ_PRIORITY      3

/*
 * EEPROM driver system settings.
 */
#define EEPROM_DRV_USE_25XX             TRUE
#define EEPROM_DRV_USE_24XX             FALSE

#endif /* _DRIVERS_CONF_H */
