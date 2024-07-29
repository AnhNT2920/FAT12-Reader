/**
 * @file  : HAL.h
 * @author: Nguyen The Anh.
 * @brief : Declare struct, typedef and function using in HAL.c.
 * @version: 0.0
 *
 * @copyright Copyright (c) 2024.
 *
 */

/*******************************************************************************
 * Include
 ******************************************************************************/

#include <stdint.h>
#include <stdio.h>

/*******************************************************************************
 * Header guard
 ******************************************************************************/

#ifndef _HAL_H_
#define _HAL_H_

/*******************************************************************************
 * Macro
 ******************************************************************************/

#define KMC_DEFAULT_SECTOR_SIZE 512

/*******************************************************************************
 * Variable
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * @brief Read a sector in the disk image starting with the "index" position and store it to buffer.
 *
 * @param index the position to read in the disk image.
 * @param buff the buffer that stores the sector.
 *
 * @return the number of bytes read succesfully.
 */
int32_t kmc_read_sector(uint32_t index, uint8_t *buff);


/**
 * @brief Read multiple sector in the disk image starting with the index position and store it to buffer.
 *
 * @param index the position to read in the disk image.
 * @param num the amount of sector to read.
 * @param buff a buffer that stores the sectors.
 *
 * @return: the number of bytes read succesfully.
 */
int32_t kmc_read_multi_sector(uint32_t index, uint32_t num, uint8_t *buff);

/**
 * @brief Open the disk image and set the size of sector to the default value (512).
 *
 * @param file_name the name of the image.
 *
 * @return the FILE pointer points to the file/disk image.
 */
FILE *kmc_init(uint8_t *file_name);

/**
 * @brief Update size of the sector.
 *
 * @param byte_per_sector the total bytes in 1 sector.
 *
 * @return return the size of sector after updated.
 */
uint32_t kmc_update_sector_size(uint16_t bytes_per_sector);

/**
 * @brief Close the current stream.
 *
 * @param: This function has no param.
 *
 * @return: This function return nothing
 */
void kmc_de_init(void);

/*Header guard*/
#endif
/*End of file*/
