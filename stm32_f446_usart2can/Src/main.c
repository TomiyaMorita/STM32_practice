/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
 CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */
//CAN関連
CAN_FilterTypeDef sFilterConfig1; //フィルタ
CAN_FilterTypeDef sFilterConfig2; //フィルタ
CAN_TxHeaderTypeDef TxHeader; //Txのヘッダ
CAN_RxHeaderTypeDef RxHeader; //Rxのヘッダ
uint8_t TxData[8]; //CANにて送るデータ
uint8_t RxData[8]; //CANにて受け取ったデータ
uint32_t TxMailbox;
uint8_t can_id_list[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C}; //odrive axis node id
uint8_t can_separator=0x06;//0x08以下がcan1,
uint8_t can_number =0;//can1ならflagを0, can2ならflagを1
uint8_t can_id = 0;
//uint8_t control_mode = 0x00C;//位置制御モード
//uint8_t control_mode = 0x009;//エンコーダ読み込み
//uint8_t control_mode = 0x017;//電圧取得
uint8_t cmd_data = 0x00;
uint8_t get_can_flag = 0; //canを受信したかどうかのflag

//uart関連
uint8_t UART1_Data[6];
uint8_t get_uart_flag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN1_Init(void);
static void MX_CAN2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  //CANの設定
   //can1のフィルタの設定
   sFilterConfig1.FilterBank = 0;
   sFilterConfig1.FilterMode = CAN_FILTERMODE_IDMASK;
   sFilterConfig1.FilterScale = CAN_FILTERSCALE_32BIT;
   sFilterConfig1.FilterIdHigh = 0x0000;
   sFilterConfig1.FilterIdLow = 0x0000;
   sFilterConfig1.FilterMaskIdHigh = 0x0000;
   sFilterConfig1.FilterMaskIdLow = 0x0000;
   sFilterConfig1.FilterFIFOAssignment = CAN_RX_FIFO0;
   sFilterConfig1.FilterActivation=ENABLE;
   sFilterConfig1.SlaveStartFilterBank=14;

   //can2フィルタ
   sFilterConfig2.FilterBank = 14;
   sFilterConfig2.FilterMode = CAN_FILTERMODE_IDMASK;
   sFilterConfig2.FilterScale = CAN_FILTERSCALE_32BIT;
   sFilterConfig2.FilterIdHigh = 0x0000;
   sFilterConfig2.FilterIdLow = 0x0000;
   sFilterConfig2.FilterMaskIdHigh = 0x0000;
   sFilterConfig2.FilterMaskIdLow = 0x0000;
   sFilterConfig2.FilterFIFOAssignment = CAN_RX_FIFO0;
   sFilterConfig2.FilterActivation=ENABLE;
   //sFilterConfig2.SlaveStartFilterBank=27;

   //フィルタをcan1に適用
   if(HAL_CAN_ConfigFilter(&hcan1,&sFilterConfig1) != HAL_OK)
   {
     Error_Handler();
   }
   //フィルタをcan2に適用
   if(HAL_CAN_ConfigFilter(&hcan2,&sFilterConfig2) != HAL_OK)
   {
     Error_Handler();
   }

   //can1をスタート
   if(HAL_CAN_Start(&hcan1)!=HAL_OK)
   {
     Error_Handler();
   }
   //can2をスタート
   if(HAL_CAN_Start(&hcan2)!=HAL_OK)
   {
     Error_Handler();
   }

   //can1の割り込みを許可
   if(HAL_CAN_ActivateNotification(&hcan1,CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
   {
 	Error_Handler();
   }
   //can2の割り込みを許可
   if(HAL_CAN_ActivateNotification(&hcan2,CAN_IT_RX_FIFO0_MSG_PENDING | CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK)
   {
 	Error_Handler();
   }


  //HAL_UART_Receive_DMA(&huart2, UART1_Data, 6);//6文字受信したら割り込み発生させる


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  //受信割り込みの開始=6byte受け取ったら次の処理へ
	  get_uart_flag=0;
	  HAL_UART_Receive_DMA(&huart2, UART1_Data, 6);
	  //受信するまで待つ
	  while(!get_uart_flag){}

	  //can通信の処理
	  //todo can通信を二つに対応させる=>UARTがある数より大きい=CAN2へ
	  can_id = can_id_list[UART1_Data[0]]; //uartで受け取った値の0~7bit:can_id_listのデータ
	  //can_idが0の場合は何もしない処理
	  if(can_id == 0){
		  continue;
	  }
	  if(can_id <= can_separator){
		  can_number=0;//0;
	  }else{
		  can_number=1;//1;
	  }

	  cmd_data = UART1_Data[1];//uartで受け取った値の8~15bit:canのコマンド
	  get_can_flag=0; //canデータ受信用のフラグを0に
	  switch(cmd_data){

	  	  case 0x01://ポジションを送る//位置制御的な感じ
	  		  get_can_flag = 1;//canデータを受け取らないので1に
	  		  TxHeader.StdId=(can_id << 5) + (0x00C); //can_id, コントロールcmd
	  		  TxHeader.RTR = 0;//CAN_RTR_DATA;
	  		  TxHeader.IDE = CAN_ID_STD;
	  		  TxHeader.DLC = 0x08;
	  		  TxHeader.TransmitGlobalTime = DISABLE;
	  		  TxData[0] = UART1_Data[2];
	  		  TxData[1] = UART1_Data[3];
	  		  TxData[2] = UART1_Data[4];
	  		  TxData[3] = UART1_Data[5];
	  		  TxData[4] = 0;
	  		  TxData[5] = 0;
	  		  TxData[6] = 0;
	  		  TxData[7] = 0;
	  		  if(can_number == 1){
	  			HAL_CAN_AddTxMessage(&hcan2,&TxHeader,TxData,&TxMailbox);//todo can2への対応
	  		  }else{
	  			HAL_CAN_AddTxMessage(&hcan1,&TxHeader,TxData,&TxMailbox);//todo can2への対応
	  		  }
	  		  break;

	  	  case 0x02://ポジションを受け取る
	  		  TxHeader.StdId=(can_id << 5) + (0x009); //can_id, コントロールcmd
	  		  TxHeader.RTR = 2;//CAN_RTR_DATA_;
	  		  TxHeader.IDE = CAN_ID_STD;
	  		  TxHeader.DLC = 0x08;
	  		  TxHeader.TransmitGlobalTime = DISABLE;
	  		  TxData[0] = 0;
	  		  TxData[1] = 0;
	  		  TxData[2] = 0;
	  		  TxData[3] = 0;
	  		  TxData[4] = 0;
	  		  TxData[5] = 0;
	  		  TxData[6] = 0;
	  		  TxData[7] = 0;
	  		  if(can_number == 1){
	  			HAL_CAN_AddTxMessage(&hcan2,&TxHeader,TxData,&TxMailbox);//todo can2への対応
	  		  }else{
	  			HAL_CAN_AddTxMessage(&hcan1,&TxHeader,TxData,&TxMailbox);//todo can2への対応
	  		  }
	  		  break;

	  	  case 0x03://電圧を受け取る
	  		  TxHeader.StdId=(can_id << 5) + (0x017); //can_id, コントロールcmd
	  		  TxHeader.RTR = 2;//CAN_RTR_DATA;
	  		  TxHeader.IDE = CAN_ID_STD;
	  		  TxHeader.DLC = 0x08;
	  		  TxHeader.TransmitGlobalTime = DISABLE;
	  		  TxData[0] = 0;
	  		  TxData[1] = 0;
	  		  TxData[2] = 0;
	  		  TxData[3] = 0;
	  		  TxData[4] = 0;
	  		  TxData[5] = 0;
	  		  TxData[6] = 0;
	  		  TxData[7] = 0;
	  		  if(can_number == 1){
	  			HAL_CAN_AddTxMessage(&hcan2,&TxHeader,TxData,&TxMailbox);//todo can2への対応
	  		  }else{
	  			HAL_CAN_AddTxMessage(&hcan1,&TxHeader,TxData,&TxMailbox);//todo can2への対応
	  		  }
	  		  break;

	  	  case 0x04: //モードを変更する (0x00:IDLEモード )
	  		  get_can_flag = 1;//canデータを受け取らないので1に
	  		  TxHeader.StdId=(can_id << 5) + (0x007); //can_id, コントロールcmd
	  		  TxHeader.RTR = 2;//CAN_RTR_DATA;
	  		  TxHeader.IDE = CAN_ID_STD;
	  		  TxHeader.DLC = 0x08;
	  		  TxHeader.TransmitGlobalTime = DISABLE;
	  		  TxData[0] = UART1_Data[2];
	  		  TxData[1] = UART1_Data[3];
	  		  TxData[2] = UART1_Data[4];
	  		  TxData[3] = UART1_Data[5];
	  		  TxData[4] = 0;
	  		  TxData[5] = 0;
	  		  TxData[6] = 0;
	  		  TxData[7] = 0;
	  		  if(can_number == 1){
	  			HAL_CAN_AddTxMessage(&hcan2,&TxHeader,TxData,&TxMailbox);
	  		  }else{
	  			HAL_CAN_AddTxMessage(&hcan1,&TxHeader,TxData,&TxMailbox);
	  		  }
	  		  break;

	  	  default:
	  		 get_can_flag=1;
	  }
	  while(!get_can_flag){}
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 8;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_4TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_16TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief CAN2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 8;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_4TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_16TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  /* USER CODE END CAN2_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef*UartHandle)
{
	HAL_UART_Transmit(&huart2,UART1_Data,6,1000);
	get_uart_flag=1;
}

void HAL_CAN_TxMailbox0CompleteCallack(CAN_HandleTypeDef *hcan)
{
	  //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,1);
}

//CAN通信の受信割り込み
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan_)
{
	HAL_CAN_GetRxMessage(hcan_,CAN_RX_FIFO0,&RxHeader,RxData);
	HAL_UART_Transmit(&huart2,RxData,sizeof(RxData),1000);//受け取ったデータを送信
	//HAL_UART_Transmit(&huart2,&RxHeader,4);
	get_can_flag=1;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	 __disable_irq();
		  while (1)
		  {
		  }
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
