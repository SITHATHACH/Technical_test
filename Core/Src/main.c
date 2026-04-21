
#include "main.h"

#define TABLE_SIZE 120
#define PWM_MAX    999


TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_tim4_up;



uint16_t dma_burst_buffer[TABLE_SIZE * 3]; 
volatile uint8_t is_running = 1;

const uint16_t sine_raw[TABLE_SIZE] = {
    500,526,552,578,603,628,652,675,698,719, 
    740,759,777,794,809,823,835,846,855,862, 
    868,872,874,875,874,872,868,862,855,846, 
    835,823,809,794,777,759,740,719,698,675, 
    652,628,603,578,552,526,500,473,447,421, 
    396,371,347,324,301,280,259,240,222,205, 
    190,176,164,153,144,137,131,127,125,124, 
    125,127,131,137,144,153,164,176,190,205, 
    222,240,259,280,301,324,347,371,396,421, 
    447,473,500,526,552,578,603,628,652,675, 
    698,719,740,759,777,794,809,823,835,846 };

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM4_Init(void);

void Wave_Data_Init(void) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        dma_burst_buffer[i * 3 + 0] = sine_raw[i];                    
        dma_burst_buffer[i * 3 + 1] = sine_raw[(i + 40) % TABLE_SIZE]; 
        dma_burst_buffer[i * 3 + 2] = sine_raw[(i + 80) % TABLE_SIZE]; 
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == PIN_SETUP_Pin) {
        static uint32_t last_press = 0;
        if (HAL_GetTick() - last_press < 200) return; 
        last_press = HAL_GetTick();

        is_running = !is_running;
        if(is_running) 
		{
            HAL_DMA_Start_IT(&hdma_tim4_up, (uint32_t)dma_burst_buffer, (uint32_t)&htim4.Instance->DMAR, TABLE_SIZE * 3);
            __HAL_TIM_ENABLE_DMA(&htim4, TIM_DMA_UPDATE);
        } 
		else 
		{
            HAL_DMA_Abort(&hdma_tim4_up);
            __HAL_TIM_DISABLE_DMA(&htim4, TIM_DMA_UPDATE);
            htim4.Instance->CCR1 = 0; htim4.Instance->CCR2 = 0; htim4.Instance->CCR3 = 0;
        }
    }
}

int main(void)
{

  
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM4_Init();
  
  Wave_Data_Init();
  htim4.Instance->DCR = (2 << TIM_DCR_DBL_Pos) | (0x0D << TIM_DCR_DBA_Pos);
  HAL_DMA_Start_IT(&hdma_tim4_up, (uint32_t)dma_burst_buffer, (uint32_t)&htim4.Instance->DMAR, TABLE_SIZE * 3);
  __HAL_TIM_ENABLE_DMA(&htim4, TIM_DMA_UPDATE);
    
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  
  while (1)
  {
    
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM4_Init(void)
{

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 79;
  htim4.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
  htim4.Init.Period = 999;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim4);

}

static void MX_DMA_Init(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();

  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  GPIO_InitStruct.Pin = PIN_SETUP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PIN_SETUP_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}

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
