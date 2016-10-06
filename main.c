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
/* Private macro -------------------------------------------------------------*/

#define LED_GPIO_CLK_ENABLE __HAL_RCC_GPIOB_CLK_ENABLE
#define LED_GPIO_PORT GPIOB
#define LED_GPIO_PIN GPIO_PIN_12

#define LED_THREAD 0


/* Private variables ---------------------------------------------------------*/
static GPIO_InitTypeDef  GPIO_InitStruct;
static UART_HandleTypeDef UartHandle;
osThreadId LEDThreadHandle;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void UART_Config(void);
static void LED_Thread(void const *argument);

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

    LED_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, 1);

    UART_Config();

    osThreadDef(LED_THREAD, LED_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);

    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, 0);

    LEDThreadHandle = osThreadCreate(osThread(LED_THREAD), NULL);

    printf("osKernelStart.\r\n");
    osKernelStart();

    printf("osKernel broken.\r\n");
    Error_Handler();
}


static void LED_Thread(void const *argument)
{
  uint32_t count = 0;
  (void) argument;

  for (;;)
  {
    count = osKernelSysTick() + 10000;

    while (count >= osKernelSysTick())
    {
      HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
      osDelay(1000);
      HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
      osDelay(1000);
    }
  }
}

int _write(int file, char *ptr, int len)
{
    int i;
    file = file;
    HAL_UART_Transmit(&UartHandle, ptr, len, 0xFFFF);
    return len;
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
  /*  8/1 * 9 = 72 MHz */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState        = RCC_HSE_ON;
  oscinitstruct.LSEState        = RCC_LSE_OFF;
  oscinitstruct.HSIState        = RCC_HSI_OFF;
  oscinitstruct.HSEPredivValue    = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL9;
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

    UartHandle.Instance        = USART1;
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


static void Error_Handler(void){
    while (1) {
        HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
        HAL_Delay(100);
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


