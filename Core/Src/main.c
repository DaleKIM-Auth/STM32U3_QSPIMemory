/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "py25q64ha.h"
#include "generated_data_50KB.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define QSPI_MEMEMORY_DEVICE_ID   0x16U
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

XSPI_HandleTypeDef hxspi1;
UART_HandleTypeDef huart1;
/* USER CODE BEGIN PV */
#pragma section=".bss"
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ICACHE_Init(void);
/* USER CODE BEGIN PFP */
int SystemClockInit(void);
int Init(void);
KeepInComplilation int Read(uint32_t Address, uint32_t Size, uint16_t* buffer);
KeepInComplilation int Write(uint32_t Address, uint32_t Size, uint8_t* buffer);
KeepInComplilation int SectorErase(uint32_t StartAddress, uint32_t EndAddress);
KeepInComplilation int MassErase(uint32_t Parallelism);
KeepInComplilation uint64_t Verify(uint32_t FlashAddr, uint32_t RAMBufferAddr, uint32_t Size);

static void MX_OCTOSPI1_DeInit(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


HAL_StatusTypeDef HAL_InitTick(uint32_t HAL_InitTick)
{ 
  (void)HAL_InitTick;

  /* DWT initialization */
  if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)){
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  }
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  
  return HAL_OK;
}

uint32_t HAL_GetTick(void)
{
  uint32_t cycles_per_ms = SystemCoreClock / 1000000;
  uwTick = (DWT->CYCCNT) / cycles_per_ms;
  
  return uwTick;
}

int Write (uint32_t Address, uint32_t Size, uint8_t* buffer)
{
  __disable_irq();

  /* QuadSPI De-Init */
  MX_OCTOSPI1_DeInit();

  /* QuadSPI Init */
  MX_OCTOSPI1_Init();

  Address = Address & 0x0FFFFFFF;

  PY25Q64_AutoPollingMemReady();

  /* Writes an amount of data to the QSPI memory */
  if(PY25Q64_Program(buffer, Size, Address) != PY25Q64_OK){
    return 0;
  }

  __enable_irq();

  return 1;
}

/**
 * @brief 	 Full erase of the device 						
 * @param 	 Parallelism : 0 																		
 * @retval  1           : Operation succeeded
 * @retval  0           : Operation failed											
 */
int MassErase (uint32_t Parallelism )
{ 
  __disable_irq();

  /* QuadSPI De-Init */
  MX_OCTOSPI1_DeInit();

  /* QuadSPI Init */
  MX_OCTOSPI1_Init();
  
  PY25Q64_AutoPollingMemReady();
  
  /* Erase the entire QSPI memory */
  if (PY25Q64_MassErase() != PY25Q64_OK){
      return 0;
  }

  __enable_irq();

   return 1;
}

int Init(void)
{
  int32_t result=0;
  
  __disable_irq();  
  
  char *startadd = __section_begin(".bss");
  unsigned int size = __section_size(".bss");
  memset(startadd, 0, size);

  SystemInit();
  HAL_InitTick(0);

  result = SystemClockInit();  
  if(result != 1){
    return 0;
  }
  
  //MX_GPIO_Init();
  MX_ICACHE_Init();

  MX_OCTOSPI1_DeInit();

  MX_OCTOSPI1_Init();

  if(PY25Q64_Init() != PY25Q64_OK){
    return 0;
  }

  __enable_irq();

  return 1;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
#if 1
  Init();  
#else
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
  MX_ICACHE_Init();
  /* USER CODE BEGIN 2 */
#endif
  /* External memory test code */
  {
     __IO uint8_t *mem_addr;

    /* Erase block 0 */
    MassErase(0);

    Write(0x90000000UL, BUFFERSIZE, aTxBuffer);

     /* Set Memory Mapped Mode */
    PY25Q64_MemoryMappedMode();

    PY25Q64_Delay(10);
    
    /* Read operation */
    mem_addr = (uint8_t *)(OCTOSPI1_BASE);
    for (int index = 0; index < BUFFERSIZE; index ++){
      if(*mem_addr != aTxBuffer[index]){
        Error_Handler();
      }
      mem_addr++;
    }

  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Enable Epod Booster
  */
  if (HAL_RCCEx_EpodBoosterClkConfig(RCC_EPODBOOSTER_SOURCE_MSIS, RCC_EPODBOOSTER_DIV1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_PWREx_EnableEpodBooster() != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Set Flash latency before increasing MSIS
  */
  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSIS;
  RCC_OscInitStruct.MSISState = RCC_MSI_ON;
  RCC_OscInitStruct.MSISSource = RCC_MSI_RC0;
  RCC_OscInitStruct.MSISDiv = RCC_MSI_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSIS;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache (default 2-ways set associative cache)
  */
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief OCTOSPI1 Initialization Function
  * @param None
  * @retval None
  */
void MX_OCTOSPI1_Init(void)
{

  /* USER CODE BEGIN OCTOSPI1_Init 0 */

  /* USER CODE END OCTOSPI1_Init 0 */

  /* USER CODE BEGIN OCTOSPI1_Init 1 */

  /* USER CODE END OCTOSPI1_Init 1 */
  /* OCTOSPI1 parameter configuration*/
  hxspi1.Instance = OCTOSPI1;
  hxspi1.Init.FifoThresholdByte = 1;
  hxspi1.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
  hxspi1.Init.MemoryType = HAL_XSPI_MEMTYPE_MICRON;
  hxspi1.Init.MemorySize = HAL_XSPI_SIZE_64MB;
  hxspi1.Init.ChipSelectHighTimeCycle = 1;
  hxspi1.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
  hxspi1.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
  hxspi1.Init.WrapSize = HAL_XSPI_WRAP_NOT_SUPPORTED;
  hxspi1.Init.ClockPrescaler = 1;
  hxspi1.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hxspi1.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_DISABLE;
  hxspi1.Init.ChipSelectBoundary = HAL_XSPI_BONDARYOF_NONE;
  hxspi1.Init.DelayBlockBypass = HAL_XSPI_DELAY_BLOCK_BYPASS;
  hxspi1.Init.MaxTran = 0;
  hxspi1.Init.Refresh = 0;
  if (HAL_XSPI_Init(&hxspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OCTOSPI1_Init 2 */

  /* USER CODE END OCTOSPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief System Clock Configuration
  * @retval None
  */
int SystemClockInit(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  uint32_t msiclk = 0x0U;

  __HAL_RCC_PWR_CLK_ENABLE();

  /** Enable Epod Booster
  */
  if (HAL_RCCEx_EpodBoosterClkConfig(RCC_EPODBOOSTER_SOURCE_MSIS, RCC_EPODBOOSTER_DIV1) != HAL_OK)
  {
    return 0;
  }
  if (HAL_PWREx_EnableEpodBooster() != HAL_OK)
  {
    return 0;
  }

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    return 0;
  }

  /** Set Flash latency before increasing MSIS
  */
  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSIS;
  RCC_OscInitStruct.MSISState = RCC_MSI_ON;
  RCC_OscInitStruct.MSISSource = RCC_MSI_RC0;
  RCC_OscInitStruct.MSISDiv = RCC_MSI_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    return 0;
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSIS;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    return 0;
  }

  if (RCC_OscInitStruct.MSISSource == RCC_MSI_RC0){
    msiclk = 96000000U;
  } else {
    msiclk = 24000000U;
  }
  
  switch (RCC_OscInitStruct.MSISDiv){
  case RCC_MSI_DIV1:
    SystemCoreClock = msiclk;
    break;
  case RCC_MSI_DIV2:
    SystemCoreClock = msiclk / 2;
    break;
  case RCC_MSI_DIV4:
    SystemCoreClock = msiclk / 4;
    break;
  case RCC_MSI_DIV8:
    SystemCoreClock = msiclk / 8;
    break;
  default:
    SystemCoreClock = 24000000U;
  }

  return 1;
}

static void MX_OCTOSPI1_DeInit(void)
{
  if(hxspi1.State != HAL_XSPI_STATE_RESET){
    HAL_XSPI_DeInit(&hxspi1);
  }    
    
  HAL_XSPI_MspDeInit(&hxspi1);
    
  __HAL_RCC_OCTOSPI1_FORCE_RESET();
  __HAL_RCC_OCTOSPI1_RELEASE_RESET();
  __HAL_RCC_OCTOSPI1_CLK_DISABLE();  
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

#ifdef  USE_FULL_ASSERT
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
