// Copyright (c) 2023 Ziga Miklosic
// All Rights Reserved
////////////////////////////////////////////////////////////////////////////////
/**
*@file      flash.c
*@brief     Flash LL drivers based on STM32 HAL library
*@author    Ziga Miklosic
*@email     ziga.miklosic@gmail.si
*@date      17.05.2023
*@version   V0.1.0
*/
////////////////////////////////////////////////////////////////////////////////
/*!
* @addtogroup FLASH
* @{ <!-- BEGIN GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "flash.h"
#include "../../flash_cfg.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////

#if ( 1 == FLASH_CFG_DUAL_BANK_MODE_EN )

    /**
     *  Bank data structure
     */
    typedef struct
    {
        uint32_t addr;  /**<Address */
        uint32_t size;  /**<Size of data from address */
    } flash_bank_data_t;

    /**
     *  Bank enumerations
     */
    enum
    {
        eFLASH_BANK_1 = 0,
        eFLASH_BANK_2,

        eFLASH_BANK_NUM_OF
    };

#endif // ( 1 == FLASH_CFG_DUAL_BANK_MODE_EN )

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/**
 *  Initialization flag
 */
static bool gb_is_init = false;


#if ( 1 == FLASH_CFG_DUAL_BANK_MODE_EN )

    /**
     *  Flash base address in dual-bank mode
     */
    const uint32_t gu32_flash_base[eFLASH_BANK_NUM_OF] =
    {
        [eFLASH_BANK_1] = FLASH_CFG_BANK1_START_ADDR,
        [eFLASH_BANK_2] = FLASH_CFG_BANK2_START_ADDR,
    };

#endif

////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
//static uint32_t         flash_count_page            (const uint32_t addr, const uint32_t size);

#if ( 0 == FLASH_CFG_DUAL_BANK_MODE_EN )
    //static flash_status_t   flash_erase_single_bank     (const uint32_t addr, const uint32_t size);
#else
    static flash_status_t   flash_erase_dual_bank       (const uint32_t addr, const uint32_t size);
#endif

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

#if 0

////////////////////////////////////////////////////////////////////////////////
/**
*       Calculate number of sectors to overlap
*
* @param[in]    addr        - Start address of memory section to analyse
* @param[in]    size        - Size of memory section to analyse
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
static uint32_t flash_count_page(const uint32_t addr, const uint32_t size)
{
    uint32_t sector_count = 0;

    // Calculate start and end sector number
    const uint32_t start_sector_num = (uint32_t)( addr / FLASH_CFG_PAGE_SIZE_BYTE );
    const uint32_t end_sector_num   = (uint32_t)(( addr + size -1U ) / FLASH_CFG_PAGE_SIZE_BYTE );

    // Sector count that following address space is taken
    sector_count = (( end_sector_num - start_sector_num ) + 1U );

    return sector_count;
}

#endif

#if ( 0 == FLASH_CFG_DUAL_BANK_MODE_EN )

#if 0
    ////////////////////////////////////////////////////////////////////////////////
    /**
    *       Erase flash memory in single bank configuration
    *
    * @param[in]    addr        - Start address of memory
    * @param[in]    size        - Size of memory section in bytes
    * @return       status      - Status of operation
    */
    ////////////////////////////////////////////////////////////////////////////////
    static flash_status_t flash_erase_single_bank(const uint32_t addr, const uint32_t size)
    {
        flash_status_t          status          = eFLASH_OK;
        FLASH_EraseInitTypeDef  flash_erase     = {0};
        uint32_t                sector_error    = 0U;

// On STM32L4/G4 flash pages are used when erasing
#if defined(STM32L4) || defined(STM32G4)

            // Calculate start page
            const uint32_t start_page = (uint32_t)(( addr - FLASH_BASE ) / FLASH_CFG_PAGE_SIZE_BYTE );

            // Calculate number of pages
            const uint32_t num_of_pages = flash_count_page( addr, size );

            FLASH_ASSERT( num_of_pages <= FLASH_PAGE_NB );

            // Setup flash erase
            flash_erase.TypeErase   = FLASH_TYPEERASE_PAGES;
            flash_erase.Page        = start_page;
            flash_erase.NbPages     = num_of_pages;

// On STM32H7 flash sectors are used when erasing
#elif defined(STM32H7)

            // Calculate start page
            const uint32_t start_sector = (uint32_t)(( addr - FLASH_BASE ) / FLASH_CFG_PAGE_SIZE_BYTE );

            // calculate number of sectors
            const uint32_t num_of_sectors = flash_count_page( addr, size );

            FLASH_ASSERT( num_of_sectors <= FLASH_SECTOR_TOTAL );

            // Setup flash erase
            flash_erase.TypeErase       = FLASH_TYPEERASE_SECTORS;
            flash_erase.Sector          = start_sector;
            flash_erase.NbSectors       = num_of_sectors;
            flash_erase.VoltageRange    = FLASH_VOLTAGE_RANGE_1;

            // TODO: Check this options

            //#define FLASH_VOLTAGE_RANGE_1        0x00000000U       /*!< Flash program/erase by 8 bits  */
            //#define FLASH_VOLTAGE_RANGE_2        FLASH_CR_PSIZE_0  /*!< Flash program/erase by 16 bits */
            //#define FLASH_VOLTAGE_RANGE_3        FLASH_CR_PSIZE_1  /*!< Flash program/erase by 32 bits */
            //#define FLASH_VOLTAGE_RANGE_4        FLASH_CR_PSIZE    /*!< Flash program/erase by 64 bits */

#endif

        // Single bank operation
        flash_erase.Banks = FLASH_BANK_1;

        // Erase flash
        if( HAL_OK != HAL_FLASHEx_Erase( &flash_erase, &sector_error ))
        {
            status = false;
        }

        return status;
    }
#endif

#endif

#if ( 1 == FLASH_CFG_DUAL_BANK_MODE_EN )

    ////////////////////////////////////////////////////////////////////////////////
    /**
    *       Erase flash memory in dual bank configuration
    *
    * @param[in]    addr        - Start address of memory
    * @param[in]    size        - Size of memory section in bytes
    * @return       status      - Status of operation
    */
    ////////////////////////////////////////////////////////////////////////////////
    static flash_status_t flash_erase_dual_bank(const uint32_t addr, const uint32_t size)
    {
        flash_status_t          status                          = eFLASH_OK;
        FLASH_EraseInitTypeDef  flash_erase                     = {0};
        flash_bank_data_t       bank_data[eFLASH_BANK_NUM_OF]   = {0};
        uint32_t                sector_error                    = 0U;

        // Address starts in bank 1
        if ( addr < FLASH_CFG_BANK2_START_ADDR )
        {
            bank_data[eFLASH_BANK_1].addr = addr;

            // Single bank operation
            if (( addr + size ) < FLASH_CFG_BANK2_START_ADDR )
            {
                bank_data[eFLASH_BANK_1].size = size;
            }

            // Dual-bank operation
            else
            {
                bank_data[eFLASH_BANK_1].size = ( FLASH_CFG_BANK2_START_ADDR - addr );

                bank_data[eFLASH_BANK_2].addr = FLASH_CFG_BANK2_START_ADDR;
                bank_data[eFLASH_BANK_2].size = ( size - bank_data[eFLASH_BANK_1].size );
            }
        }

        // Address start in bank 2 -> single bank operation
        else
        {
            bank_data[eFLASH_BANK_2].addr = addr;
            bank_data[eFLASH_BANK_2].size = size;
        }

        // Perform erase opration by bank
        for ( uint8_t bank = 0; bank < eFLASH_BANK_NUM_OF; bank++ )
        {
            // Anything to do?
            if ( bank_data[bank].size > 0 )
            {
                // Calculate start page
                const uint32_t start_page = (uint32_t)(( bank_data[bank].addr - gu32_flash_base[bank] ) / FLASH_CFG_PAGE_SIZE_BYTE );

                // Calcualte number of pages
                const uint32_t num_of_pages = flash_count_page( bank_data[bank].addr, bank_data[bank].size );

                FLASH_ASSERT( num_of_pages <= FLASH_PAGE_NB );

                // Mass erase if all pages in bank needs to be erased
                flash_erase.TypeErase = ((num_of_pages == FLASH_PAGE_NB) ? FLASH_TYPEERASE_MASSERASE : FLASH_TYPEERASE_PAGES );

                // Select bank
                flash_erase.Banks = (( bank == eFLASH_BANK_1 ) ? FLASH_BANK_1 : FLASH_BANK_2 );

                // Only if page type erase
                if ( FLASH_TYPEERASE_PAGES == flash_erase.TypeErase  )
                {
                    flash_erase.Page        = start_page;
                    flash_erase.NbPages     = num_of_pages;
                }

                // Erase flash
                if( HAL_OK != HAL_FLASHEx_Erase( &flash_erase, &sector_error ))
                {
                    status = false;
                }
            }
        }

        return status;
    }

#endif

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*@addtogroup FLASH_API
* @{ <!-- BEGIN GROUP -->
*
* 	Following function are part of Flash API.
*/
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/**
*       Initialize STM32 internal flash
*
* @return       status - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
flash_status_t flash_init(void)
{
    flash_status_t status = eFLASH_OK;

    if ( false == gb_is_init )
    {
        // Enable flash clock
#if defined(STM32L4) || defined(STM32G4)
        __HAL_RCC_FLASH_CLK_ENABLE();
#endif

        // Wait for flash to be ready
        while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY) != RESET);

        // Unlock flash
        if ( HAL_OK != HAL_FLASH_Unlock())
        {
            status = eFLASH_ERROR;
        }
        else
        {
            // Init success
            gb_is_init = true;
        }
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
*       De-Initialize STM32 internal flash
*
* @return       status - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
flash_status_t flash_deinit(void)
{
    flash_status_t status = eFLASH_OK;

    if ( true == gb_is_init )
    {
        // Lock flash
        if ( HAL_OK != HAL_FLASH_Lock())
        {
            status = eFLASH_ERROR;
        }
        else
        {
            // Disable flash clock
#if defined(STM32L4) || defined(STM32G4)
            __HAL_RCC_FLASH_CLK_DISABLE();
#endif

            // De-init success
            gb_is_init = false;
        }
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/*!
* @brief        Get Flash initialization flag
*
* @param[out]   p_is_init   - Pointer to init flag
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
flash_status_t flash_is_init(bool * const p_is_init)
{
    flash_status_t status = eFLASH_OK;

    FLASH_ASSERT( NULL != p_is_init );

    if ( NULL != p_is_init )
    {
        *p_is_init = gb_is_init;
    }
    else
    {
        status = eFLASH_ERROR;
    }

    return status;

    return status;
}

#if 0

////////////////////////////////////////////////////////////////////////////////
/*!
* @brief        Write to flash
*
* @param[in]    addr        - Flash address
* @param[in]    size        - Size of data to write in bytes
* @param[in]    p_data      - Data to write
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
flash_status_t flash_write(const uint32_t addr, const uint32_t size, const uint8_t * const p_data)
{
    flash_status_t status = eFLASH_OK;

    FLASH_ASSERT( true == gb_is_init );
    FLASH_ASSERT(( addr >= FLASH_CFG_START_ADDR ) && ( size <= FLASH_CFG_SIZE_BYTE ));
    FLASH_ASSERT( NULL != p_data );

    if  (   ( true == gb_is_init )
        &&  (( addr >= FLASH_CFG_START_ADDR ) && ( size <= FLASH_CFG_SIZE_BYTE ))
        &&  ( NULL != p_data ))
    {
#if defined(STM32L4) || defined(STM32G4)

        // Address shall be aligned by 8bytes
        if ( 0U == ( addr % 8 ))
        {
            // Write data - 8 bytes at once
            for ( uint32_t dword = 0; dword < size; dword+=8U )
            {
                // Calculate address
                const uint32_t flash_addr = ( addr + dword );

                // Copy data
                const uint64_t flash_data = 0UL;
                memcpy( &flash_data, &p_data[dword], sizeof( uint64_t ));

                // Program flash with 8 bytes
                if ( HAL_OK != HAL_FLASH_Program( FLASH_TYPEPROGRAM_DOUBLEWORD, flash_addr, flash_data ))
                {
                    status = eFLASH_ERROR;
                    break;

                    FLASH_ASSERT(0);
                }
            }
        }

        // Alignment problem
        else
        {
            status = eFLASH_ERROR;
        }


#elif defined(STM32H7)

        // Address shall be aligned by 32bytes
        if ( 0U == ( addr % 32 ))
        {
            // Write data - 32 bytes at once
            for (uint32_t address_offset = 0; address_offset < size; address_offset += 32)
            {
                // Program the flash using the current aligned address and data pointer
                if ( HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, addr + address_offset, (uint32_t)(p_data + (address_offset / 4))))
                {
                    status = eFLASH_ERROR;
                    break;

                    FLASH_ASSERT(0);
                }
            }
        }

        // Alignment problem
        else
        {
            status = eFLASH_ERROR;
        }
#endif
    }
    else
    {
        status = eFLASH_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/*!
* @brief        Read data from flash
*
* @param[in]    addr        - Flash address
* @param[in]    size        - Size of data to read in bytes
* @param[out]   p_data      - Data to write
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
flash_status_t flash_read(const uint32_t addr, const uint32_t size, uint8_t * const p_data)
{
    flash_status_t status = eFLASH_OK;

    FLASH_ASSERT( true == gb_is_init );
    FLASH_ASSERT(( addr >= FLASH_CFG_START_ADDR ) && ( size <= FLASH_CFG_SIZE_BYTE ));
    FLASH_ASSERT( NULL != p_data );

    if  (   ( true == gb_is_init )
        &&  (( addr >= FLASH_CFG_START_ADDR ) && ( size <= FLASH_CFG_SIZE_BYTE ))
        &&  ( NULL != p_data ))
    {
        for ( uint32_t word = 0; word < size; word+=4U )
        {
            // Calculate flash address
            const uint32_t flash_addr = (uint32_t) ( addr + word );

            // Get flash data
            const uint32_t flash_data = *(uint32_t*)( flash_addr );

            // Copy data
            memcpy( &p_data[word], &flash_data, 4U );
        }
    }
    else
    {
        status = eFLASH_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/*!
* @brief        Erase data from flash
*
* @param[in]    addr        - Flash address
* @param[in]    size        - Size of data to read in bytes
* @return       status      - Status of operation
*/
////////////////////////////////////////////////////////////////////////////////
flash_status_t flash_erase(const uint32_t addr, const uint32_t size)
{
    flash_status_t status = eFLASH_OK;

    FLASH_ASSERT( true == gb_is_init );
    FLASH_ASSERT(( addr >= FLASH_CFG_START_ADDR ) && ( size <= FLASH_CFG_SIZE_BYTE ));

    if  (   ( true == gb_is_init )
        &&  (( addr >= FLASH_CFG_START_ADDR ) && ( size <= FLASH_CFG_SIZE_BYTE )))
    {
        #if ( 0 == FLASH_CFG_DUAL_BANK_MODE_EN )

            // Single bank operation
            status = flash_erase_single_bank( addr, size );

        #else

            // Dual-bank operation
            status = flash_erase_dual_bank( addr, size );

        #endif
    }
    else
    {
        status = eFLASH_ERROR;
    }

    return status;
}
#endif


/**
 * @brief Erases a specified number of bytes in Flash memory.
 * @note  The erase size is based on Flash sectors.
 * @param addr The starting address of the region to erase.
 * @param size The size in bytes of the region to erase.
 * @retval flash_status_t The status of the erase operation.
 */
flash_status_t flash_erase(const uint32_t addr, const uint32_t size)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0;

    // --- Step 1: Unlock Flash memory ---
    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return eFLASH_ERROR;
    }

    // --- Step 2: Configure Erase parameters ---
    // Calculate the start sector from the address.
    // The STM32H7 sectors are 128KB on Bank 1 and 256KB on Bank 2.
    // A simpler approach is to iterate through the sectors based on address.
    // This assumes FLASH_SECTOR_SIZE is defined.
    uint32_t start_sector_addr = addr - (addr % FLASH_SECTOR_SIZE);
    uint32_t end_sector_addr = addr + size;

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3; // VDD = 2.7 to 3.6V
    EraseInitStruct.Banks = FLASH_BANK_1; // Assuming Bank 1 for this example, adjust if needed.

    // Iterate through all sectors in the specified range.
    for (uint32_t current_addr = start_sector_addr; current_addr < end_sector_addr; current_addr += FLASH_SECTOR_SIZE)
    {
        // Find the sector number for the current address.
        EraseInitStruct.Sector = (current_addr - FLASH_BASE) / FLASH_SECTOR_SIZE;
        EraseInitStruct.NbSectors = 1;

        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return eFLASH_ERROR;
        }
    }

    // --- Step 3: Lock Flash memory ---
    HAL_FLASH_Lock();
    return eFLASH_OK;
}

/**
 * @brief Writes a block of data to Flash memory.
 * @note  This function handles the necessary Flash operations including
 * unlocking, writing in FLASH_WORD (32-byte) chunks, and locking.
 * The write operation requires the Flash region to be erased first.
 * @param addr The starting address in Flash to write to. Must be 32-byte aligned.
 * @param size The number of bytes to write. Must be a multiple of 32.
 * @param p_data A pointer to the source data buffer.
 * @retval flash_status_t The status of the write operation.
 */
flash_status_t flash_write(const uint32_t addr, const uint32_t size, const uint8_t * const p_data)
{
    uint32_t address_offset = 0;

    // --- Step 1: Perform alignment checks ---
    // A FLASH_WORD (32 bytes) write requires a 32-byte aligned address.
    if ((addr % 32) != 0) {
        return eFLASH_ERROR;
    }

    // The size of the data to be written must also be a multiple of 32 bytes.
    if ((size % 32) != 0) {
        return eFLASH_ERROR;
    }

    // --- Step 3: Unlock Flash memory for writing ---
    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return eFLASH_ERROR;
    }

    // --- Step 4: Write the data in chunks of 32 bytes (FLASH_WORD) ---
    for (address_offset = 0; address_offset < size; address_offset += 32)
    {
        // Program the flash using the current aligned address and data pointer
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                              addr + address_offset,
                              (uint32_t)(p_data + address_offset)) != HAL_OK)
        {
            // Lock flash and return error immediately on failure
            HAL_FLASH_Lock();
            return eFLASH_ERROR;
        }
    }

    // --- Step 5: Lock Flash memory ---
    HAL_FLASH_Lock();
    return eFLASH_OK;
}

/**
 * @brief Reads a specified number of bytes from Flash memory.
 * @param addr The starting address in Flash to read from.
 * @param size The number of bytes to read.
 * @param p_data A pointer to the destination data buffer.
 * @retval flash_status_t The status of the read operation.
 */
flash_status_t flash_read(const uint32_t addr, const uint32_t size, uint8_t * const p_data)
{
    // Simply copy the data from Flash memory to the destination buffer.
    memcpy(p_data, (const uint8_t *)addr, size);
    return eFLASH_OK;
}


////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
