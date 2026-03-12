/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "drv8305.h"
#include "as5600.h"
#include "vofa.h"
#include "led.h"
#include "debug.h"
#include "foc.h"
#include "pwm.h"
#include "arm_math.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
float vofa_data[8]; // 增加通道数以观察更多数据
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern float zero_offset_rad; 
extern int8_t sensor_direction;
extern volatile uint16_t adc_raw_angle;

static void CurrentSense_Calibration(void)
{
  uint32_t sum_a = 0;
  uint32_t sum_b = 0;
  uint32_t sum_c = 0;

  const uint16_t samples = 4000;

  printf("Current calibration start\r\n");

  DRV8305_EnableDCCal();
  HAL_Delay(5);

  for (uint16_t i = 0; i < samples; i++)
  {
    HAL_ADCEx_InjectedStart(&hadc1);
    HAL_ADCEx_InjectedPollForConversion(&hadc1, 10);
    sum_a += HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
    sum_b += HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);
    sum_c += HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_3);
  }

  adc_ia_offset = (int32_t)(sum_a / samples);
  adc_ib_offset = (int32_t)(sum_b / samples);
  adc_ic_offset = (int32_t)(sum_c / samples);

  DRV8305_DisableDCCal();


  printf("IA offset=%ld\r\n", adc_ia_offset);
  printf("IB offset=%ld\r\n", adc_ib_offset);
  printf("IC offset=%ld\r\n", adc_ic_offset);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM9_Init();
  /* USER CODE BEGIN 2 */
  LED_Init(); // LED初始化，默认为白灯常亮
  Debug_Init();
  AS5600_Init(&hi2c1); // 串口及传感器基础外设初始化
  VOFA_Init(&huart1);

  //PWM和DRV8305初始化
  PWM_Stop();
  DRV8305_Init(&hspi1);
  HAL_Delay(10); // 等待配置稳定
  //HAL_TIM_Base_Start(&htim1); 现在使用ADC中断更新FOC参数
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  DRV8305_Enable();    //必须要在校准程序之前开启DRV8305的GATE输出，不然电流放大器不工作，没法校准
  HAL_Delay(10); // 开始准备校准
  CurrentSense_Calibration();
  PWM_Init();   // 强制写入 50% CCR

  HAL_TIM_GenerateEvent(&htim1, TIM_EVENTSOURCE_UPDATE); //强制同步影子寄存器
  HAL_ADCEx_InjectedStart_IT(&hadc1);
  PWM_Start();
  HAL_Delay(5); // 确保波形稳定
  DRV8305_Enable(); //开启栅极输出，驱动NMOS
  Debug_Check_DRV8305();  // 检查状态，此时应该无故障


  /*printf("Start D-Axis Calibration...\r\n");
      float align_v = 0.25f; // 给 25%

      float v_a = 0.5f + align_v;
      float v_b = 0.5f - (align_v / 2.0f);
      float v_c = 0.5f - (align_v / 2.0f);

      // 写入 PWM 寄存器死锁电机
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (uint32_t)(v_a * 4199.0f));
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, (uint32_t)(v_b * 4199.0f));
      __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, (uint32_t)(v_c * 4199.0f));

      uint16_t true_zero_adc = AS5600_GetRawAngle();
      float true_zero_rad = (float)true_zero_adc * (6.28318530718f / 4096.0f);

      printf("True Zero Offset Rad: %f\r\n", true_zero_rad);

      // 死循环卡住，不要进入后面的控制
      while(1) { }
   	   	   */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // 启动 1kHz 控制环路中断
  HAL_TIM_Base_Start_IT(&htim9);

  uint32_t vofa_tick = 0;

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      // 通信任务：每 5ms 发送一次 (200Hz)
      if (HAL_GetTick() - vofa_tick >= 5)
      {
          vofa_tick = HAL_GetTick();

          //打印信息
          // 1. 目标转速 (RPM)
          vofa_data[0] = 0.0f;
          // 2. 实际转速 (RPM)
          vofa_data[1] = 0.0f;
          // 3. 实际 Iq (A) - 代表转矩电流
          vofa_data[2] = Iq;
          // 4. 实际流过电机的电流大小 (A) - 相电流矢量幅值
          // I_mag = sqrt(Id^2 + Iq^2)
          float I_mag;
          arm_sqrt_f32(Id*Id + Iq*Iq, &I_mag);
          vofa_data[3] = I_mag;
          
          VOFA_Send(vofa_data, 4);
      }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
extern void FOC_Interrupt_Routine(uint16_t adc_a, uint16_t adc_b, uint16_t adc_c);

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1)
    {
        // 极速读取三个通道的 ADC 原始值
        adc_ia_raw = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_1);
        adc_ib_raw = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_2);
        adc_ic_raw = HAL_ADCEx_InjectedGetValue(hadc, ADC_INJECTED_RANK_3);
        
        // FOC 
        FOC_Interrupt_Routine(adc_ia_raw, adc_ib_raw, adc_ic_raw);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM9)
    {
        // 1kHz 控制环路
        
        // 触发下一次 DMA 读取
        // 这次读到的数据将在【下一次】1ms 中断里被使用
        AS5600_ReadAngleDMA();
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
