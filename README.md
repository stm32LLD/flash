# **STM32 Flash Low Level Driver**
Following repository constains STM32 Flash Low Level (LL) driver C implementation based on STM32 HAL library. It is used for internal STM32 Flash interface.


STM32 UART LL driver is supporting following STM32 device family:
- STM32G4

## **Dependencies**

### **1. STM32 HAL library**
STM32 Flash LL driver module uses STM32 HAL library.



## **API**
| API Functions | Description | Prototype |
| --- | ----------- | ----- |
| **flash_init** | Initialization of module | flash_status_t flash_init(void) |
| **flash_deinit** | De-initialization of module | flash_status_t flash_deinit(void) |
| **flash_is_init** | Get initialization state of flash module | flash_status_t flash_is_init(bool * const p_is_init) |
| **flash_write** | Write to STM32 internal flash memory | flash_status_t flash_write(const uint32_t addr, const uint32_t size, const uint8_t * const p_data) |
| **flash_read** | Read data from STM32 internal flash memory | flash_status_t flash_read(const uint32_t addr, const uint32_t size, uint8_t * const p_data) |
| **flash_erase** | Erase (page) in STM32 internal flash memory | flash_status_t flash_erase(const uint32_t addr, const uint32_t size) |


## **Usage**

**GENERAL NOTICE: Put all user code between sections: USER CODE BEGIN & USER CODE END!**

**1. Copy template files to root directory of the module**

Copy configuration file *flash_cfg* to root directory and replace file extension (.htmp/.ctmp -> .h/.c).

**2. Change default HAL library include to target microprocessor inside ***flash_cfg.h***:**

Following example shows HAL library include for STM32G4 family:
```C
// USER INCLUDE BEGIN...

#include "stm32g4xx_hal.h"

// USER INCLUDE END...
```

**3. Configure Flash module for application needs by changing ***flash_cfg.h***. Configuration options are following:**

| Configuration | Description |
| --- | --- |
| **FLASH_CFG_PAGE_SIZE_BYTE** 			| Flash page size in bytes |
| **FLASH_CFG_START_ADDR** 			    | User Flash region start address |
| **FLASH_CFG_SIZE_BYTE** 			    | User Flash region size in bytes |
| **FLASH_CFG_ASSERT_EN** 		        | Enable/Disable assertions |
| **FLASH_ASSERT** 		                | Assert definition |

**4. Initialize Flash module**
```C
if ( eFLASH_OK != flash_init())
{
    // Initialization failed...

    // Further actions here...
}
```

**5. Write/Read/Erase from internal Flash**
```C
// Write to flash
flash_write( 0x0801F000, 12, (const uint8_t*) "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x10\x11\x12" );

// Read from flash
flash_read( 0x0801F000, 32, ( uint8_t*) &flash_data );

// Page erase 
flash_erase( 0x0801F000, 0x800 );
```