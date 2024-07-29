/**
 * @file  : FATfs.h
 * @author: Nguyen The Anh.
 * @brief : Declare struct, typedef and function using in FATfs.c.
 * @version: 0.0
 *
 * @copyright Copyright (c) 2024.
 *
 */

/*******************************************************************************
 * Include
 ******************************************************************************/

#include <stdint.h>

/*******************************************************************************
 * Header guard
 ******************************************************************************/

#ifndef _FATFS_H_
#define _FATFS_H_

/*******************************************************************************
 * Macro
 ******************************************************************************/

#define ENTRIES_PER_SECTOR 16
#define FAT12_CLUSTER_OFFSET_FACTOR 31

/*******************************************************************************
 * Enum
 ******************************************************************************/

typedef enum disk_state
{
    GOOD_CONDITION,
    FAILED_TO_OPEN,
    BAD_BOOT_SECTOR
} disk_state_enum_t;

typedef enum entry_discription
{
    DELETED_ENTRY = 0xE5,
    UNUSED_ENTRY = 0x00,
    FAKE_ENTRY = 0X0F,
    FOLDER_ENTRY = 0x10,
    FILE_ENTRY = 0x00
} fatfs_entry_discr_enum_t;

typedef enum FAT12_base_address
{
    BOOT_SECTOR_BASE_ADDRESS = 0,
    ROOT_DIR_12_PHYSC_BASE_INDEX = 19,
    ROOT_DIR_12_LOGICAL_BASE_INDEX = 0,
    DATA_REGION_12_PHYSC_BASE_INDEX = 33,
    DATA_REGION_12_LOGICAL_BASE_INDEX = 2,
    FAT_TABE_PHYSC_BASE_INDEX = 1
} fatfs_fat12_enum_base_index_t;

/*******************************************************************************
 * Struct
 ******************************************************************************/

typedef struct node
{
    uint16_t logical_cluster;
    struct node *next;
} fatfs_node_struct_t;

typedef struct boot_sector_t
{
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;

    uint16_t reserved_sectors_quantity;
    uint8_t num_of_FATs;
    uint16_t max_root_dir_entries;
    uint16_t total_sectors;
    uint16_t sectors_per_FAT;

    uint8_t signature;
    uint8_t fat_type[8];
} fatfs_boot_sector_struct_t;

typedef struct entry_dir_information
{
    uint8_t **entry_name;
    uint8_t *attribute;
    uint16_t *first_logical_cluster;
    uint32_t *entry_size;
    uint16_t list_count;
} fatfs_entry_list_struct_t;

/*******************************************************************************
 * Typedef callback function
 ******************************************************************************/

typedef void (*callback_print_filecontent)(uint8_t *file_content, uint32_t bytes_read);

/*******************************************************************************
 * Variable
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * @brief Register for a callback function that will print the content of a file.
 *
 * @param func is the address of the file printing function.
 *
 * @return: This function return nothing.
 */
void ResgisterPrint_file_func(callback_print_filecontent func);


/**
 * @brief Call the init function in HAL, read boot sector, allocate space and read FAT table.
 *
 * @param file_name is the name of the file/disk image.
 *
 * @return the status of the file/disk image.
 */
disk_state_enum_t fatfs_init(uint8_t *file_name);


/**
 * @brief Get the entry list(directory entries) at the position stored first logical cluster.
 *
 * @param firs_logical_cluster has the value of where the directory started.
 *
 * @return the entry list.
 */
fatfs_entry_list_struct_t fatfs_read_dir(uint16_t first_logical_cluster);


/**
 * @brief CLear the memory allocate for current entry list(directory list).
 *
 * @param: This function has no param.
 *
 * @return: This function return nothing.
 */
void fatfs_clear_dir_list(void);


/**
 * @brief Read a file in the disk.
 *
 * @param first_logical_cluster has the value of where the file started.
 *
 * @return: This function return nothing.
 */
void fatfs_read_file(uint16_t first_logical_cluster);

/**
 * @brief De-initialize the FATfs layer, free the memory allocated for FAT table.
 *
 * @param: This function has no param.
 *
 * @return: This function return nothing.
 */
void fatfs_de_init(void);

/*End of Header Guard*/
#endif
/*End of file*/
