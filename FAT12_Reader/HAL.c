/**
 * @file  : HAL.c
 * @author: Nguyen The Anh.
 * @brief : Definition of function using in file HAL.c
 * @version: 0.0
 *
 * @copyright Copyright (c) 2024.
 *
 */

/*******************************************************************************
 * Include
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "HAL.h"

/*******************************************************************************
 * Variable
 ******************************************************************************/

static FILE *s_file_img = NULL;
static uint16_t s_sector_size = 0;

/*******************************************************************************
 * Functions
 ******************************************************************************/

/*Functions*********************************************************************
*
* Function name: kmc_read_sector.
* Description: Read 1 sector in file from the position "index" and store it to buff.
*
END***************************************************************************/
int32_t kmc_read_sector(uint32_t index, uint8_t *buff)
{
    uint32_t bytes_read = 0; /*bytes_read stores the total bytes read successfully*/

    /*Check if file open successfully*/
    if (NULL != s_file_img)
    {
        /*Set the position of the cursor in the file to the index*/
        fseek(s_file_img, index * s_sector_size, SEEK_SET);

        /*Read 1 sector and get the num of bytes read*/
        bytes_read = fread(buff, 1, s_sector_size, s_file_img);
    }
    else
    {
        /*Do nothing*/
    }

    return bytes_read;
}

/*Functions*********************************************************************
*
* Function name: kmc_read_multi_sector.
* Description: Read multiple sector in file from the position "index" and store to buff.
*
END***************************************************************************/
int32_t kmc_read_multi_sector(uint32_t index, uint32_t num, uint8_t *buff)
{
    uint32_t total_bytes = 0; /*total_bytes stores the num of bytes read successfully =*/

    /*Check if the file opened succesfully*/
    if (NULL != s_file_img)
    {
        /*Set the cursor position to index*/
        fseek(s_file_img, index * s_sector_size, SEEK_SET);

        /*Read num of sector and get the total of bytes read*/
        total_bytes = fread(buff, 1, num * s_sector_size, s_file_img);
    }
    else
    {
        /*Do nothing*/
    }

    return total_bytes;
}

/*Functions*********************************************************************
*
* Function name: kmc_init.
* Description: Initial the HAL layer, set sector size to default value(512).
*              Return a FILE pointer points to the current opened file.
*
END***************************************************************************/
FILE *kmc_init(uint8_t *file_name)
{
    /*Open the file in "file_name"*/
    s_file_img = fopen(file_name, "rb+");

    /*Check if the file opened succesfully*/
    if (NULL != s_file_img)
    {
        /*Set sector size to default value*/
        s_sector_size = KMC_DEFAULT_SECTOR_SIZE;
    }
    else
    {
        /*Do nothing*/
    }

    return s_file_img;
}
/*Functions*********************************************************************
*
* Function name: kmc_update_sector_size.
* Description: Update the sector size if the field bytes_per_sector is difference
*              from the default value. This function will return the sector size
*              after updated.
*
END***************************************************************************/
uint32_t kmc_update_sector_size(uint16_t bytes_per_sector)
{
    /*Check if the field bytes per sector is valid*/
    if ((0 < bytes_per_sector) && (0 == (bytes_per_sector % KMC_DEFAULT_SECTOR_SIZE)) && (KMC_DEFAULT_SECTOR_SIZE != bytes_per_sector))
    {
        /*Update the sector size*/
        s_sector_size = bytes_per_sector;
    }
    else
    {
        /*Do nothing*/
    }

    return s_sector_size;
}

/*Functions*********************************************************************
*
* Function name: kmc_de_init.
* Description: De-initial the HAL layer, close the file stream.
*
END***************************************************************************/
void kmc_de_init(void)
{
    /*Close the file*/
    fclose(s_file_img);

    return;
}
/*End of file*/
