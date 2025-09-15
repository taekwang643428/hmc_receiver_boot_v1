/**
 * @file    uart.c
 * @brief   .
 */

#include "main.h"
#include "uart.h"
#include "ringbuf.h"

#define RBSIZE  1000

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;

ringbuf uart4_buffer;                // 
static uint8_t uart4_rawbuffer[RBSIZE];    // 
static uint8_t uart4_data;

ringbuf uart5_buffer;                // 
static uint8_t uart5_rawbuffer[RBSIZE];    // 
static uint8_t uart5_data;

static uint8_t uart4_interupt_enabled = 0; // 0: disable, 1: enable
static uint8_t uart5_interupt_enabled = 0; // 0: disable, 1: enable

/**
 * @brief   Receives data from UART.
 * @param   *data: Array to save the received data.
 * @param   length:  Size of the data.
 * @return  status: Report about the success of the receiving.
 */
uart_status uart_receive(uint8_t *data, uint16_t length)
{
	uart_status status = UART_ERROR;

	if (HAL_OK == HAL_UART_Receive(&huart4, data, length, UART_TIMEOUT))
	{
		status = UART_OK;
	}

	return status;
}

/**
 * @brief   Transmits a string to UART.
 * @param   *data: Array of the data.
 * @return  status: Report about the success of the transmission.
 */
uart_status uart_transmit_str(uint8_t *data)
{
	uart_status status = UART_ERROR;
	uint16_t length = 0u;

	/* Calculate the length. */
	while ('\0' != data[length])
	{
		length++;
	}

	if (HAL_OK == HAL_UART_Transmit(&huart4, data, length, UART_TIMEOUT))
	{
		status = UART_OK;
	}

	return status;
}

/**
 * @brief   Transmits a single char to UART.
 * @param   *data: The char.
 * @return  status: Report about the success of the transmission.
 */
uart_status uart_transmit_ch(uint8_t data)
{
	uart_status status = UART_ERROR;

	/* Make available the UART module. */
	if (HAL_UART_STATE_TIMEOUT == HAL_UART_GetState(&huart4))
	{
		HAL_UART_Abort(&huart4);
	}

	if (HAL_OK == HAL_UART_Transmit(&huart4, &data, 1u, UART_TIMEOUT))
	{
		status = UART_OK;
	}
	return status;
}

void uart5_send(uint8_t *data, int length)
{
	if (HAL_UART_Transmit(&huart5, (uint8_t*) data, length, 1000) != HAL_OK)
	{
	}
}

uart_status uart5_transmit_ch(uint8_t data)
{
	uart_status status = UART_ERROR;

	/* Make available the UART module. */
	if (HAL_UART_STATE_TIMEOUT == HAL_UART_GetState(&huart5))
	{
		HAL_UART_Abort(&huart5);
	}

	if (HAL_OK == HAL_UART_Transmit(&huart5, &data, 1u, UART_TIMEOUT))
	{
		status = UART_OK;
	}
	return status;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	if (huart->Instance == UART4)
	{
		ringbuf_put(&uart4_buffer, (uint8_t) (uart4_data));
		HAL_UART_Receive_IT(&huart4, &uart4_data, 1);
		return;
	}


	if (uart5_interupt_enabled == 1)
	{
		if (huart->Instance == UART5)
		{
			ringbuf_put(&uart5_buffer, (uint8_t) (uart5_data));
			HAL_UART_Receive_IT(&huart5, &uart5_data, 1);
			return;
		}
	}

}

void uart4_Init(void)
{
	ringbuf_init(&uart4_buffer, uart4_rawbuffer, RBSIZE);     //

}

void uart5_Init(void)
{

	ringbuf_init(&uart5_buffer, uart5_rawbuffer, RBSIZE);     //

}

void uart4_interupt_set(uint8_t enable)
{
	if (enable == 1)
	{
		uart4_interupt_enabled = 1;
		HAL_UART_Receive_IT(&huart4, &uart4_data, 1);
	}
	else
	{
		uart4_interupt_enabled = 0;
		HAL_UART_AbortReceive_IT(&huart4);
	}
}
void uart5_interupt_set(uint8_t enable)
{
	if (enable == 1)
	{
		uart5_interupt_enabled = 1;
		HAL_UART_Receive_IT(&huart5, &uart5_data, 1);
	}
	else
	{
		uart5_interupt_enabled = 0;
		HAL_UART_AbortReceive_IT(&huart5);
	}
}

