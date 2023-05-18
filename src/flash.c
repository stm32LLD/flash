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


////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

/**
 *  Initialization flag
 */
static bool gb_is_init = false;


////////////////////////////////////////////////////////////////////////////////
// Function prototypes
////////////////////////////////////////////////////////////////////////////////
static uint32_t flash_count_page(const uint32_t addr, const uint32_t size);


////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

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
        __HAL_RCC_FLASH_CLK_ENABLE();

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
            __HAL_RCC_FLASH_CLK_DISABLE();

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
    flash_status_t  status      = eFLASH_OK;
    uint64_t        flash_data  = 0UL;

    FLASH_ASSERT( true == gb_is_init );
    FLASH_ASSERT(( addr >= FLASH_CFG_START_ADDR ) && ( size <= FLASH_CFG_SIZE_BYTE ));
    FLASH_ASSERT( NULL != p_data );

    if  (   ( true == gb_is_init )
        &&  (( addr >= FLASH_CFG_START_ADDR ) && ( size <= FLASH_CFG_SIZE_BYTE ))
        &&  ( NULL != p_data ))
    {
        // Write all double words - 64bit
        for ( uint32_t dword = 0; dword < size; dword+=8U )
        {
            // Calculate address
            const uint32_t flash_addr = ( addr + dword );

            // Copy data
            flash_data = 0UL;
            memcpy( &flash_data, &p_data[dword], sizeof( uint64_t ));

            // Program flash
            if ( HAL_OK != HAL_FLASH_Program( FLASH_TYPEPROGRAM_DOUBLEWORD, flash_addr, flash_data ))
            {
                status = eFLASH_ERROR;
                break;

                FLASH_ASSERT(0);
            }
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
            const uint32_t flash_data = *(__IO uint32_t*)( flash_addr );

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
    flash_status_t          status          = eFLASH_OK;
    uint32_t                sector_error    = 0U;
    FLASH_EraseInitTypeDef  flash_erase     = {0};

    FLASH_ASSERT( true == gb_is_init );
    FLASH_ASSERT(( addr >= FLASH_CFG_START_ADDR ) && ( size <= FLASH_CFG_SIZE_BYTE ));

    if  (   ( true == gb_is_init )
        &&  (( addr >= FLASH_CFG_START_ADDR ) && ( size <= FLASH_CFG_SIZE_BYTE )))
    {
        // Calculate start page
        const uint32_t start_page = (uint32_t)(( addr - FLASH_BASE ) / FLASH_CFG_PAGE_SIZE_BYTE );

        // Calcualte number of pages
        const uint32_t num_of_pages = flash_count_page( addr, size );

        FLASH_ASSERT( num_of_pages <= FLASH_PAGE_NB );

        // Setup flash erase
        flash_erase.TypeErase   = FLASH_TYPEERASE_PAGES;
        flash_erase.Page        = start_page;
        flash_erase.NbPages     = num_of_pages;
        flash_erase.Banks       = FLASH_BANK_1;

        // Erase flash
        if( HAL_OK != HAL_FLASHEx_Erase( &flash_erase, &sector_error ))
        {
            status = false;
        }
    }
    else
    {
        status = eFLASH_ERROR;
    }

    return status;
}

////////////////////////////////////////////////////////////////////////////////
/**
* @} <!-- END GROUP -->
*/
////////////////////////////////////////////////////////////////////////////////
