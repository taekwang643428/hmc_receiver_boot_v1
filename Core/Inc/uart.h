/**
 * @file    uart.h
 * @brief	.
 */

#ifndef UART_H_
#define UART_H_


#include "stm32f4xx_hal.h"
#include "ringbuf.h"

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern ringbuf uart4_buffer; 
extern ringbuf uart5_buffer; 

/* Timeout for HAL. */
#define UART_TIMEOUT ((uint16_t)1000u)

/* Status report for the functions. */
typedef enum {
  UART_OK     = 0x00u, /**< The action was successful. */
  UART_ERROR  = 0xFFu  /**< Generic error. */
} uart_status;

uart_status uart_receive(uint8_t *data, uint16_t length);
uart_status uart_transmit_str(uint8_t *data);
uart_status uart_transmit_ch(uint8_t data);

uart_status uart5_transmit_ch(uint8_t data);

void uart1_Init(void);
void uart4_Init(void);
void uart5_Init(void);

void uart4_interupt_set(uint8_t enable);
void uart5_interupt_set(uint8_t enable);

#endif /* UART_H_ */
