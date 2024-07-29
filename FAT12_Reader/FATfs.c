/**
 * @file  : FATfs.c
 * @author: Nguyen The Anh.
 * @brief : Definition of function using in file FATfs.c
 * @version: 0.0
 *
 * @copyright Copyright (c) 2024.
 *
 */

/*******************************************************************************
 * Include
 ******************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "HAL.h"
#include "FATfs.h"

/*******************************************************************************
 * Static function prototype
 ******************************************************************************/

/**
 * @brief Clear the current linked list "cluster_chain" by free it's memory space.
 *
 * @param cluster_chain is the linked list of cluster chain we want to clear.
 *
 * @return: This function return nothing
 */
static void fatfs_clear_cluster_chain(void);


/**
 * @brief Read hex value in little endian to decimal value.
 *
 * @param hex_line is a buffer that hold a hex string.
 * @param index is the position we want to start reading.
 * @param bytes_count is number of bytes we want to read.
 *
 * @return the status of the id (CORRECT/!= CORRECT)
 */
static uint64_t hex_to_decimal(uint8_t *hex_line, uint32_t index, uint8_t bytes_count);

/**
 * @brief Read 12-bit element in FAT table at position logical_cluster to a decimal value.
 *
 * @param logical_cluster is the position we want to read.
 *
 * @return the value of the element at "logical_cluster" position in FAT table.
 */
static uint16_t read_FAT_entry(uint16_t logical_cluster);

/**
 * @brief Add new node(cluster index) to the cluster_chain.
 *
 * @param logical_cluster is the logical number of a cluster.
 *
 * @return: This function return nothing.
 */
static void fatfs_add_node(uint16_t logical_cluster);


/**
 * @brief Set up a linked list that stores the cluster chain of a file or subdirectory.
 *
 * @param first_logical_cluster is the firs logical cluster number of the file/subdirectory .
 *
 * @return the length of the list.
 */
static uint32_t fatfs_get_cluster_chain(uint16_t first_logical_cluster);

/*******************************************************************************
 * Variable
 ******************************************************************************/

/*This variable stores the information of boot sector*/
static fatfs_boot_sector_struct_t s_FAT12Infor;

/*This variable stores the FAT table data*/
static uint8_t *s_fat_table = NULL;

/*This varible stores the entry list of a directory*/
static fatfs_entry_list_struct_t s_dirlist;

/*This variable stores a cluster chain*/
static fatfs_node_struct_t *s_cluster_chain = NULL;

/*Call back pointer*/
static callback_print_filecontent print_file_callback = NULL;

/*******************************************************************************
 * Static functions
 ******************************************************************************/

/*Static functions*************************************************************
*
* Function name: fatfs_clear_cluster_chain.
* Description: Clear the linked list "cluster_chain" by freeing it's memory space.
*
END***************************************************************************/
static void fatfs_clear_cluster_chain(void)
{
    fatfs_node_struct_t *temp = NULL; /*temp is used for traversaling the list*/

    /*Traversal the list*/
    while (s_cluster_chain != NULL)
    {
        /*Set temp to the new head node*/
        temp = s_cluster_chain;

        /*Move head to next node*/
        s_cluster_chain = s_cluster_chain->next;

        /*Free the previous node*/
        free(temp);
    }

    /*Set the head node to NULL*/
    s_cluster_chain = NULL;

    return;
}

/*Static functions*************************************************************
*
* Function name: hex_to_decimal.
* Description: Read a hex value in little endian form to decimal value.
*
END***************************************************************************/
static uint64_t hex_to_decimal(uint8_t *hex_line, uint32_t index, uint8_t bytes_count)
{
    uint64_t decimal_value = 0; /*decimal_value stores the decimal value of a hex string*/
    uint32_t i = 0;             /*i used for traversaling the hex string*/
    uint32_t byte_offset = 0;   /*byte_offset is the position we want to start reading in the hex-buffer*/
    uint32_t line_length = 0;   /*line_length stores the length of the hex buffer string*/

    /*Get the length of the hex buffer string*/
    line_length = strlen(hex_line);

    /*Check if the index is valid*/
    if (index >= line_length && index < 0)
    {
        /*Do nothing*/
    }
    else
    {
        /*Convert hex string in little endian to decimal value*/
        for (i = 0; i < bytes_count; i++)
        {
            byte_offset = index + i;
            decimal_value += hex_line[byte_offset] << (8 * i);
        }
    }

    return decimal_value;
}

/*Static functions*************************************************************
*
* Function name: read_FAT_entry.
* Description: Read 12-bit element (litter endian) in FAT table at the position
*              "logical_cluster" to a decimal value.
*
END***************************************************************************/
static uint16_t read_FAT_entry(uint16_t logical_cluster)
{
    uint16_t FAT_entry = 0;  /*FAT_entry stores the value of the element at "logical_cluster" position in FAT table*/
    uint16_t four_bits = 0;  /*four_bits stores the value of 4-bit part of an FAT entry element*/
    uint16_t eight_bits = 0; /*eight_bits stores the value of 8-bit part of an FAT entry element */

    /*If the logical number is odd*/
    if (logical_cluster & 1)
    {
        four_bits = s_fat_table[(3 * logical_cluster) / 2] >> 4;

        eight_bits = s_fat_table[(3 * logical_cluster) / 2 + 1] << 4;

        FAT_entry = four_bits + eight_bits;
    }
    /*If it's even*/
    else
    {
        four_bits = (s_fat_table[(3 * logical_cluster) / 2 + 1] & 0x0f) << 8;

        eight_bits = s_fat_table[(3 * logical_cluster) / 2];

        FAT_entry = four_bits + eight_bits;
    }

    return FAT_entry;
}

/*Static functions*************************************************************
*
* Function name: fatfs_add_node.
* Description: Add new node(cluster index) to the cluster_chain.
*
END***************************************************************************/
static void fatfs_add_node(uint16_t logical_cluster)
{
    fatfs_node_struct_t *temp = NULL;              /*temp is used for creating new node*/
    fatfs_node_struct_t *travel = s_cluster_chain; /*travel is used for traversaling the list*/

    /*Allocate for new node*/
    temp = (fatfs_node_struct_t *)malloc(sizeof(fatfs_node_struct_t));

    /*Assign value for the new node(cluster index)*/
    temp->logical_cluster = logical_cluster;
    temp->next = NULL;

    /*If the list is currently empty*/
    if (NULL == s_cluster_chain)
    {
        s_cluster_chain = temp;
    }
    /*If it's not empty*/
    else
    {
        /*Traversal the list*/
        while (NULL != travel->next)
        {
            /*Move to last node*/
            travel = travel->next;
        }

        /*Set the new node to be the last node*/
        travel->next = temp;
    }

    return;
}

/*Static functions*************************************************************
*
* Function name: fatfs_get_cluster_chain.
* Description: Set up a linked list that stores the cluster chain of a file
*              or subdirectory.
*
END***************************************************************************/
static uint32_t fatfs_get_cluster_chain(uint16_t first_logical_cluster)
{
    uint16_t logical_cluster = 0; /*logical_cluster is the logical cluster number*/
    uint32_t chain_length = 0;    /*chain_length stores the length of the cluster chain*/

    logical_cluster = first_logical_cluster;

    /*Add firt_logical_cluster to the list*/
    fatfs_add_node(logical_cluster);

    chain_length++;

    /*Add the remaining clusters to the list*/
    while (logical_cluster < 0xFF8)
    {
        logical_cluster = read_FAT_entry(logical_cluster);
        fatfs_add_node(logical_cluster);
        chain_length++;
    }

    return chain_length - 1;
}

/*******************************************************************************
 * Functions
 ******************************************************************************/

/*Functions*********************************************************************
*
* Function name: ResgisterPrint_file_func.
* Description: Register the print file function to the call back function pointer.
*
END***************************************************************************/
void ResgisterPrint_file_func(callback_print_filecontent func)
{
    print_file_callback = func;
}

/*Functions*********************************************************************
*
* Function name: fatfs_init.
* Description: Initial the FATfs layer. Read boot sector, update sector size,
*              allocate and read FAT table. The function will return the state
*              of the disk image.
*
END***************************************************************************/
disk_state_enum_t fatfs_init(uint8_t *file_name)
{
    FILE *disk_ptr = NULL;       /*disk_ptr stores the FILE pointer points to the current disk*/
    uint8_t buffer[512];         /*buffer stores a sector content*/
    uint32_t i = 0;              /*i used for getting the name of FAT type*/
    uint32_t offset = 0;         /*offset stores the offset value in the buffer*/
    uint32_t sector_size = 0;    /*sector_size stores the size of sector after updating*/
    disk_state_enum_t state = 0; /*state stores the status of the disk*/

    /*Initial the HAL layer*/
    disk_ptr = kmc_init(file_name);

    /*If the disk image failed to open*/
    if (NULL == disk_ptr)
    {
        state = FAILED_TO_OPEN;
    }
    /*If it opend succesfully*/
    else
    {
        /*Read the boot sector*/
        kmc_read_sector(BOOT_SECTOR_BASE_ADDRESS, buffer);

        s_FAT12Infor.bytes_per_sector = hex_to_decimal(buffer, 11, 2);

        s_FAT12Infor.sectors_per_cluster = hex_to_decimal(buffer, 13, 1);

        s_FAT12Infor.reserved_sectors_quantity = hex_to_decimal(buffer, 14, 2);

        s_FAT12Infor.num_of_FATs = hex_to_decimal(buffer, 16, 1);

        s_FAT12Infor.max_root_dir_entries = hex_to_decimal(buffer, 17, 2);

        s_FAT12Infor.total_sectors = hex_to_decimal(buffer, 19, 2);

        s_FAT12Infor.sectors_per_FAT = hex_to_decimal(buffer, 22, 2);

        s_FAT12Infor.signature = hex_to_decimal(buffer, 38, 1);

        for (i = 0; i < 8; i++)
        {
            offset = 54 + i;

            s_FAT12Infor.fat_type[i] = buffer[offset];
        }
    }

    /*Check if the boot sector is invalid*/
    if ((s_FAT12Infor.bytes_per_sector % 512 != 0) || (s_FAT12Infor.bytes_per_sector < 1) && (s_FAT12Infor.reserved_sectors_quantity < 1) && (s_FAT12Infor.num_of_FATs < 2) && (s_FAT12Infor.max_root_dir_entries % 16 != 0))
    {
        state = BAD_BOOT_SECTOR;
    }
    /*If it's valid*/
    else
    {
        state = GOOD_CONDITION;

        /*Get sector size after updating*/
        sector_size = kmc_update_sector_size(s_FAT12Infor.bytes_per_sector);

        /*Allocate memory space for FAT table*/
        s_fat_table = (uint8_t *)malloc(sizeof(uint8_t) * sector_size * s_FAT12Infor.sectors_per_FAT);

        /*Read the FAT table*/
        kmc_read_multi_sector(FAT_TABE_PHYSC_BASE_INDEX, s_FAT12Infor.sectors_per_FAT, s_fat_table);
    }

    return state;
}

/*Functions*********************************************************************
*
* Function name: fatfs_read_dir.
* Description: Get the entry list(directory entries) of the root directory or
*              a subdirectory and return it to the application layer.
*
END***************************************************************************/
fatfs_entry_list_struct_t fatfs_read_dir(uint16_t first_logical_cluster)
{
    fatfs_node_struct_t *temp = NULL;    /*temp is used for traversaling the list*/
    uint8_t *buffer = NULL;              /*buffer stores the content of the directory*/
    uint16_t *entries_index = NULL;      /*entries_index stores the index of all entry in buffer*/
    uint32_t buffer_size = 0;            /*buffer_size is the size of the buffer*/
    uint32_t chain_length = 0;           /*chain_length stores the length of the cluster_chain*/
    uint32_t root_dir_cluster_count = 0; /*root_dir_cluster_count stores the nubmer of cluster in root directory*/
    uint32_t offset = 0;                 /*offset stores the value of the offset position in buffer*/
    uint32_t i = 0;                      /*i is used for traversaling the buffer*/
    uint32_t j = 0;                      /*j is used for traversaling the entries_index*/

    /*If the directory is root directory*/
    if (ROOT_DIR_12_LOGICAL_BASE_INDEX == first_logical_cluster)
    {
        /*Get the number of cluster in the root directory*/
        root_dir_cluster_count = s_FAT12Infor.max_root_dir_entries / ENTRIES_PER_SECTOR;

        /*Get the buffer size*/
        buffer_size = s_FAT12Infor.bytes_per_sector * root_dir_cluster_count;

        /*Allocate memory space for buffer*/
        buffer = (uint8_t *)malloc(sizeof(uint8_t) * buffer_size);

        /*Allocate memory space for entries_index*/
        entries_index = (uint16_t *)malloc(sizeof(uint16_t) * s_FAT12Infor.max_root_dir_entries);

        /*Read the content of root directory to buffer*/
        kmc_read_multi_sector(ROOT_DIR_12_PHYSC_BASE_INDEX, root_dir_cluster_count, buffer);
    }
    /*If the directory is subdirectory*/
    else if (first_logical_cluster > ROOT_DIR_12_LOGICAL_BASE_INDEX)
    {
        /*Get the cluster chain of the subdirectory and it's length*/
        chain_length = fatfs_get_cluster_chain(first_logical_cluster);

        /*Get the buffer size*/
        buffer_size = chain_length * s_FAT12Infor.bytes_per_sector;

        /*Allocate memory space for buffer*/
        buffer = (uint8_t *)malloc(sizeof(uint8_t) * buffer_size);

        /*Allocate memory space for entries_index*/
        entries_index = (uint16_t *)malloc(sizeof(uint16_t) * chain_length * ENTRIES_PER_SECTOR);

        /*Set temp node to head node*/
        temp = s_cluster_chain;

        /*Traversal the list*/
        while (temp->next != NULL)
        {
            /*Read content of each sector in cluster chain*/
            kmc_read_sector(temp->logical_cluster + FAT12_CLUSTER_OFFSET_FACTOR, buffer + i);

            /*Move to next node*/
            temp = temp->next;

            /*Set i as the offset value to move in the buffer*/
            i += s_FAT12Infor.bytes_per_sector;
        }
    }
    else
    {
        /*Do nothing*/
    }

    /*Traversal the buffer*/
    for (i = 0; i < buffer_size; i += 32)
    {
        /*Check the entry*/
        if ((DELETED_ENTRY != buffer[i]) && (UNUSED_ENTRY != buffer[i]) && (FAKE_ENTRY != buffer[i + 11]))
        {
            /*Store the index of the entry to entries_index*/
            entries_index[j++] = i;

            /*Increase the entry list count*/
            s_dirlist.list_count++;
        }
    }

    /*Allocate memory space for the directory list*/
    s_dirlist.entry_name = (uint8_t **)malloc(sizeof(uint8_t *) * s_dirlist.list_count);
    for (i = 0; i < s_dirlist.list_count; i++)
    {
        /*Allocate memory space for each entry name in directory list*/
        s_dirlist.entry_name[i] = (uint8_t *)malloc(sizeof(uint8_t) * 12);
    }

    /*Allocate memory space for field entry_attribute in directory list*/
    s_dirlist.attribute = (uint8_t *)malloc(sizeof(uint8_t) * s_dirlist.list_count);

    /*Allocate memory space for field entry_size in directory list*/
    s_dirlist.entry_size = (uint32_t *)malloc(sizeof(uint32_t) * s_dirlist.list_count);

    /*Allocate memory space for field first_logical_cluster in directory list*/
    s_dirlist.first_logical_cluster = (uint16_t *)malloc(sizeof(uint16_t) * s_dirlist.list_count);

    /*Store each entry to each element in directory list*/
    for (i = 0; i < s_dirlist.list_count; i++)
    {
        /*Get entry name*/
        memcpy(s_dirlist.entry_name[i], (buffer + entries_index[i]), 11);
        s_dirlist.entry_name[i][11] = '\0';

        /*Get entry attribute*/
        s_dirlist.attribute[i] = buffer[entries_index[i] + 11];

        offset = entries_index[i] + 26;

        /*Get entry first logical cluster*/
        s_dirlist.first_logical_cluster[i] = hex_to_decimal(buffer, offset, 2);

        offset = entries_index[i] + 28;

        /*Get entry size*/
        s_dirlist.entry_size[i] = hex_to_decimal(buffer, offset, 4);
    }

    /*Free the buffer*/
    free(buffer);

    /*Free the entries_index*/
    free(entries_index);

    /*Free the cluster_chain*/
    fatfs_clear_cluster_chain();

    return s_dirlist;
}

/*Functions*********************************************************************
*
* Function name: fatfs_clear_dir_list.
* Description: CLear the memory allocate for current entry list(directory list)
*
END***************************************************************************/
void fatfs_clear_dir_list(void)
{
    uint32_t i = 0; /*i used for traversaling the list*/

    /*Traversal the entry list name*/
    for (i = 0; i < s_dirlist.list_count; i++)
    {
        /*Free the field entry_name*/
        free(s_dirlist.entry_name[i]);
    }

    /*Free the field attribute*/
    free(s_dirlist.attribute);

    /*Free the field entry size*/
    free(s_dirlist.entry_size);

    /*Free the field first logical cluster*/
    free(s_dirlist.first_logical_cluster);

    /*Clear the list count*/
    s_dirlist.list_count = 0;

    return;
}

/*Functions*********************************************************************
*
* Function name: fatfs_read_file.
* Description: Get the cluster chain of a file. Use the callback function of file
*              printing to print the file to console.
*
END***************************************************************************/
void fatfs_read_file(uint16_t first_logical_cluster)
{
    fatfs_node_struct_t *temp = NULL; /*temp is used for traversaling the list*/
    uint32_t bytes_read = 0;          /*bytes_read is the number of bytes in the file*/
    uint8_t *file_content = NULL;     /*file_content stores the content of file*/
    uint16_t chain_length = 0;        /*chain_length stores the length of the chain*/
    uint32_t offset = 0;              /*offset stores the offset value to move in the file_content*/

    /*Get the cluster chain of file and it's length*/
    chain_length = fatfs_get_cluster_chain(first_logical_cluster);

    /*Set temp to head node*/
    temp = s_cluster_chain;

    /*Allocate memory space for file_content*/
    file_content = (uint8_t *)malloc(sizeof(uint8_t) * s_FAT12Infor.bytes_per_sector);

    /*Traversal the list*/
    while (NULL != temp->next)
    {
        /*Read the sector in the node and store it to file_content*/
        bytes_read += kmc_read_sector(temp->logical_cluster + FAT12_CLUSTER_OFFSET_FACTOR, file_content);

        /*Print each cluster to console*/
        print_file_callback(file_content, s_FAT12Infor.bytes_per_sector);

        /*Move to next node(cluster)*/
        temp = temp->next;
    }

    /*Free the file_content*/
    free(file_content);

    /*Free the cluster_chain*/
    fatfs_clear_cluster_chain();

    return;
}

/*Functions*********************************************************************
*
* Function name: fatfs_de_init.
* Description: Free the memory space using for FAT table and de-initial the FATfs
*              layer.
*
END***************************************************************************/
void fatfs_de_init(void)
{
    /*Free memory space for s_fat_table*/
    free(s_fat_table);

    /*De-init the HAL layer*/
    kmc_de_init();
}
/*End of file*/
