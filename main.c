/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32F1xx_HAL_Examples
  * @{
  */

/** @addtogroup GPIO_IOToggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define PUTCHAR_PROTOTYPE void __io_putchar(void* p, char ch)

/* Private macro -------------------------------------------------------------*/

#define LED_GPIO_PORT GPIOC
#define LED_GPIO_PIN GPIO_PIN_13

PUTCHAR_PROTOTYPE;

/* Private variables ---------------------------------------------------------*/
static GPIO_InitTypeDef  GPIO_InitStruct;
static UART_HandleTypeDef UartHandle;
__IO uint16_t aADCxConvertedValues[2];
static uint16_t cacheADCValues[2] = {0, 0};

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void UART_Config(void);
static void ADC_Config(void);
static int asix_change(char Axis, int index);


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    HAL_Init();
    SystemClock_Config();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);

    UART_Config();
    init_printf(0, __io_putchar);
    char banner[5] = {'O', 'K', 0x0A, 0x0D, 0x00};
    HAL_UART_Transmit(&UartHandle, (uint8_t *)&banner, 4, 0xFFFF);

    ADC_Config();
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
    {
      /* Calibration Error */
      Error_Handler();
    }

    int temp, tvalue = 0;
    while (1) {
        temp = 0;
        tvalue = 0;
        HAL_Delay(100);
        if (HAL_ADC_Start_DMA(&hadc1,
                            (uint32_t *)aADCxConvertedValues, 2) != HAL_OK)
        {
          /* Start Error */
          Error_Handler();
        }
        if(cacheADCValues[0] == 0 && cacheADCValues[1] == 0){
            cacheADCValues[0] = aADCxConvertedValues[0];
            cacheADCValues[1] = aADCxConvertedValues[1];
            printf("Axis Init ");
            continue;
        }
        
        tvalue = asix_change('X', 0);
        temp += tvalue;
        tvalue = asix_change('Y', 1);
        temp += tvalue;
        if (temp > 0){
            printf("\r\n");
        }
    }
    printf("Error While END.");
    Error_Handler();
    return -1;
}

int asix_change(char Axis, int index){
    #define DIFF_VALUE 0x200
    #define RESET_MAX 0x900
    #define RESET_MIN 0x600
    if ( aADCxConvertedValues[index] > RESET_MIN  && aADCxConvertedValues[index] < RESET_MAX){
        if ( cacheADCValues[index] < RESET_MIN  || cacheADCValues[index] > RESET_MAX){
            printf("Axis[%c] C {0x%x, 0x%x}  ", Axis, cacheADCValues[index], aADCxConvertedValues[index]);
            cacheADCValues[index] = aADCxConvertedValues[index];
            return 1;
        }
        cacheADCValues[index] = aADCxConvertedValues[index];
        return 0;
    }
    if ( cacheADCValues[index] < DIFF_VALUE){
        cacheADCValues[index] = DIFF_VALUE;
    }

    if ( (cacheADCValues[index] + DIFF_VALUE) < aADCxConvertedValues[index] &&
         (cacheADCValues[index] - DIFF_VALUE) > aADCxConvertedValues[index]) {
        return 0;
    }

    if ( (cacheADCValues[index] + DIFF_VALUE) < aADCxConvertedValues[index]){
        printf("Axis[%c] + {0x%x, 0x%x}  ", Axis, cacheADCValues[index], aADCxConvertedValues[index]);
        cacheADCValues[index] = aADCxConvertedValues[index];
        return 1;
    }

    if ( (cacheADCValues[index] - DIFF_VALUE) > aADCxConvertedValues[index]){
        printf("Axis[%c] - {0x%x, 0x%x}  ", Axis, cacheADCValues[index], aADCxConvertedValues[index]);
        cacheADCValues[index] = aADCxConvertedValues[index];
        return 1;
    }
    return 0;
}


PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF);
}


/**
  * @brief  System Clock Configuration
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};
  __PWR_CLK_ENABLE();

  /* Configure PLL ------------------------------------------------------*/
  /*  8/1 * 6 = 48 MHz */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState        = RCC_HSE_ON;
  oscinitstruct.LSEState        = RCC_LSE_OFF;
  oscinitstruct.HSIState        = RCC_HSI_OFF;
  oscinitstruct.HSEPredivValue    = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
}

static void UART_Config(void)
{

    UartHandle.Instance        = USART2;
    UartHandle.Init.BaudRate   = 115200;
    UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    UartHandle.Init.StopBits   = UART_STOPBITS_1;
    UartHandle.Init.Parity     = UART_PARITY_NONE;
    UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle.Init.Mode       = UART_MODE_TX;

    if (HAL_UART_Init(&UartHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }


}


static void ADC_Config(void) {
    ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
    hadc1.Instance = ADC1;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode          = ADC_SCAN_ENABLE;
  hadc1.Init.NbrOfConversion       = 2;                             /* Parameter discarded because sequencer is disabled */
  hadc1.Init.ContinuousConvMode    = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
  hadc1.Init.NbrOfDiscConversion   = 1;                             /* Parameter discarded because sequencer is disabled */
  hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;

    HAL_ADC_Init(&hadc1);

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
    sConfig.Channel = ADC_CHANNEL_1;
    sConfig.Rank = 2;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

}



static void Error_Handler(void) {
    while (1) {
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        HAL_Delay(100);
        printf("Error_Handler\r\n");
    }
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


