/* Declarations for functions in ESP8266 ROM code
 *
 * Copyright (c) 2016-2019 Espressif Systems (Shanghai) PTE LTD & Cesanta Software Limited
 * All rights reserved
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


/* ROM function prototypes for functions in ROM which are
   called by the flasher stubs.
*/
#pragma once

#include <stdint.h>
#include "soc_support.h"

#if defined(ESP32S2)
#define WITH_USB 1
#endif

int uart_rx_one_char(uint8_t *ch);
uint8_t uart_rx_one_char_block();
int uart_tx_one_char(char ch);
void uart_div_modify(uint32_t uart_no, uint32_t baud_div);

int SendMsg(uint8_t *msg, uint8_t size);
int send_packet(const void *packet, uint32_t size);

void _putc1(char *ch);

void ets_delay_us(uint32_t us);

typedef enum { SPI_FLASH_RESULT_OK = 0,
               SPI_FLASH_RESULT_ERR = 1,
               SPI_FLASH_RESULT_TIMEOUT = 2 } SpiFlashOpResult;

SpiFlashOpResult SPILock();
SpiFlashOpResult SPIUnlock();
SpiFlashOpResult SPIRead(uint32_t addr, void *dst, uint32_t size);
SpiFlashOpResult SPIWrite(uint32_t addr, const uint8_t *src, uint32_t size);
SpiFlashOpResult SPIEraseChip();
SpiFlashOpResult SPIEraseBlock(uint32_t block_num);
SpiFlashOpResult SPIEraseSector(uint32_t sector_num);
uint32_t SPI_read_status();
uint32_t Wait_SPI_Idle();
void spi_flash_attach();

void SelectSpiFunction();
void SPIFlashModeConfig(uint32_t a, uint32_t b);
void SPIReadModeCnfig(uint32_t a);
uint32_t SPIParamCfg(uint32_t deviceId, uint32_t chip_size, uint32_t block_size, uint32_t sector_size, uint32_t page_size, uint32_t status_mask);

void ets_delay_us(uint32_t delay_micros);

void ets_isr_mask(uint32_t ints);
void ets_isr_unmask(uint32_t ints);
void ets_set_user_start(void (*user_start_fn)());

void software_reset();
void software_reset_cpu(int cpu_no);

struct MD5Context {
  uint32_t buf[4];
  uint32_t bits[2];
  uint8_t in[64];
};

void MD5Init(struct MD5Context *ctx);
void MD5Update(struct MD5Context *ctx, void *buf, uint32_t len);
void MD5Final(uint8_t digest[16], struct MD5Context *ctx);

typedef struct {
    uint32_t device_id;
    uint32_t chip_size;    // chip size in bytes
    uint32_t block_size;
    uint32_t sector_size;
    uint32_t page_size;
    uint32_t status_mask;
} esp_rom_spiflash_chip_t;


typedef void (*int_handler_t)(void *arg);
int_handler_t ets_isr_attach(uint32_t int_num, int_handler_t handler,
                             void *arg);
/* Some ESP32-onwards ROM functions */
#if ESP32_OR_LATER
uint32_t ets_get_detected_xtal_freq(void);
void uart_tx_flush(int uart);
uint32_t ets_efuse_get_spiconfig(void);

#if ESP32
SpiFlashOpResult esp_rom_spiflash_write_encrypted(uint32_t addr, const uint8_t *src, uint32_t size);
#else
void SPI_Write_Encrypt_Enable();
void SPI_Write_Encrypt_Disable();
SpiFlashOpResult SPI_Encrypt_Write(uint32_t flash_addr, const void* data, uint32_t len);
#endif

#if ESP32S2_OR_LATER
SpiFlashOpResult SPI_read_status_high(esp_rom_spiflash_chip_t *spi, uint32_t *status);
#else
/* Note: On ESP32 this was a static function whose first argument was elided by the
   compiler. */
SpiFlashOpResult SPI_read_status_high(uint32_t *status);
#endif

SpiFlashOpResult SPI_write_status(esp_rom_spiflash_chip_t *spi, uint32_t status_value);

void intr_matrix_set(int cpu_no, uint32_t module_num, uint32_t intr_num);
#endif /* ESP32 || ESP32S2 */


#ifdef ESP32S2
extern uint8_t UartDev_buff_uart_no; /* Member of UartDev, indicates which UART is used for SLIP communication */
#define UART_USB  2                  /* value of the above which indicates that USB CDC is in use */
#endif // ESP32S2

#ifdef WITH_USB
#define ACM_BYTES_PER_TX   64
#define ACM_STATUS_LINESTATE_CHANGED   -1
#define ACM_STATUS_RX                  -4
#define LINE_CTRL_BAUD_RATE   (1 << 0)
#define LINE_CTRL_RTS         (1 << 1)
#define LINE_CTRL_DTR         (1 << 2)
#define LINE_CTRL_DCD         (1 << 3)
#define LINE_CTRL_DSR         (1 << 4)
#define USBDC_PERSIST_ENA  (1<<31)
void usb_dw_isr_handler(void* arg);
typedef void cdc_acm_device;
extern cdc_acm_device *uart_acm_dev;
typedef void(*uart_irq_callback_t)(cdc_acm_device *dev, int status);
void cdc_acm_irq_callback_set(cdc_acm_device *dev, uart_irq_callback_t cb);
void cdc_acm_irq_rx_enable(cdc_acm_device *dev);
void cdc_acm_irq_rx_disable(cdc_acm_device *dev);
int cdc_acm_fifo_read(cdc_acm_device *dev, uint8_t *rx_data, const int size);
int cdc_acm_fifo_fill(cdc_acm_device *dev, const uint8_t *tx_data, int len);
int cdc_acm_line_ctrl_get(cdc_acm_device *dev, uint32_t ctrl, uint32_t *val);
int cdc_acm_rx_fifo_cnt(cdc_acm_device *dev);
void cdc_acm_irq_state_enable(cdc_acm_device *dev);
void usb_dc_check_poll_for_interrupts(void);
void chip_usb_set_persist_flags(uint32_t flags);
int usb_dc_prepare_persist(void);
#endif // WITH_USB
