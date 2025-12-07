// Minimal shim for driver/spi_common.h - Arduino ESP32 SDK provides the real
// definitions in hal/spi_types.h and other SDK headers

#ifndef _DRIVER_SPI_COMMON_H_
#define _DRIVER_SPI_COMMON_H_

#include <stdint.h>
#include <stdbool.h>

#if __has_include("hal/spi_types.h")
#include "hal/spi_types.h"
#endif

#if __has_include("esp_err.h")
#include "esp_err.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Handle types if not already defined
#ifndef _SPI_DEVICE_HANDLE_T_
#define _SPI_DEVICE_HANDLE_T_
typedef void* spi_device_handle_t;
#endif

// spi_dma_chan_t enum (from ESP-IDF) - defines DMA channel constants
typedef enum {
    SPI_DMA_DISABLED = 0,
    SPI_DMA_CH1 = 1,
    SPI_DMA_CH2 = 2,
    SPI_DMA_CH_AUTO = 3
} spi_dma_chan_t;

// spi_bus_config_t structure (from ESP-IDF driver/spi_common.h)
typedef struct {
    int mosi_io_num;
    int miso_io_num;
    int sclk_io_num;
    int quadwp_io_num;
    int quadhd_io_num;
    int max_transfer_sz;
    uint32_t flags;
    int intr_flags;
} spi_bus_config_t;

#ifdef __cplusplus
}
#endif

#endif // _DRIVER_SPI_COMMON_H_
