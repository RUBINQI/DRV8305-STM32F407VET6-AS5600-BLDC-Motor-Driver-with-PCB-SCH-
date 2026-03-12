/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.h
  * @brief   This file contains all the function prototypes for
  *          the adc.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN Private defines */
#define ADC_COUNTS_MAX (4095.0f)
#define ADC_COUNTS_FS  (4096.0f)
#define ADC_VREF_V     (3.3f)

#define CURRENT_SENSE_AMP_GAIN (80.0f)
#define CURRENT_SENSE_RSHUNT_OHM (0.005f)
#define CURRENT_COEFF (ADC_VREF_V/(ADC_COUNTS_FS*CURRENT_SENSE_AMP_GAIN*CURRENT_SENSE_RSHUNT_OHM))

/* USER CODE END Private defines */

void MX_ADC1_Init(void);

/* USER CODE BEGIN Prototypes */
extern volatile uint16_t adc_ia_raw;
extern volatile uint16_t adc_ib_raw;
extern volatile uint16_t adc_ic_raw;

extern volatile int32_t adc_ia_offset;
extern volatile int32_t adc_ib_offset;
extern volatile int32_t adc_ic_offset;

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __ADC_H__ */

