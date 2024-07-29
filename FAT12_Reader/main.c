/**
 * @file  : main.c
 * @author: Nguyen The Anh.
 * @brief : Definition of function using in file main.c
 * @version: 0.0
 *
 * @copyright Copyright (c) 2024.
 *
 */

/*******************************************************************************
 * Include
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include "HAL.h"

#include "FATfs.h"

/*******************************************************************************
 * Variable
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * @brief Print the entry list to the console.
 *
 * @param entry_list the directory entry list.
 *
 * @return: This function return nothing.
 */
void app_print_entry_list(fatfs_entry_list_struct_t *entry_list);

/**
 * @brief Print the file content to the console.
 *
 * @param file_content The files data.
 * @param bytes_read number of bytes in the file.
 *
 * @return: This function return nothing.
 */
void app_print_file_content(uint8_t *file_content, uint32_t bytes_read);

/**
 * @brief Print the disk's status to the console.
 *
 * @param disk_state status of the disk.
 *
 * @return: This function return nothing.
 */
void app_print_disk_state(disk_state_enum_t disk_state);

/*******************************************************************************
 * Functions
 ******************************************************************************/

/*Functions*********************************************************************
*
* Function name: app_print_entry_list.
* Description: Print each entry in directory list to the console.
*
END***************************************************************************/

void app_print_entry_list(fatfs_entry_list_struct_t *entry_list)
{
    uint32_t i = 0;  /*i used for traversaling the directory list*/
    uint8_t type[7]; /*type stores the type of entry*/

    printf("\n+-----------+-------------------------------------------------------+");
    printf("\n|  MY DISK  | Select the options below to access or press 0 to exit |");
    printf("\n+-----------+-------------------------------------------------------+");
    printf("\n|  Option   |         Name          |    Type     |       size      |");
    printf("\n+-----------+-------------------------------------------------------+");

    /*Traversal the directory list*/
    for (i = 0; i < entry_list->list_count; i++)
    {
        /*If the entry is folder*/
        if (FOLDER_ENTRY == entry_list->attribute[i])
        {
            strcpy(type, "Folder");
            printf("\n|  %4d     |%12s           |%-6s       |         %c       |", i + 1, entry_list->entry_name[i], type, '#');
        }
        /*If the entry is file*/
        else
        {
            strcpy(type, "File");
            printf("\n|  %4d     |%12s           |%-6s       | %8d Bytes  |", i + 1, entry_list->entry_name[i], type, entry_list->entry_size[i]);
        }
    }
    printf("\n+-----------+-------------------------------------------------------+");

    return;
}

/*Functions*********************************************************************
*
* Function name: app_print_file_content.
* Description: Print the file content to the console.
*
END***************************************************************************/

void app_print_file_content(uint8_t *file_content, uint32_t bytes_read)
{
    uint32_t i = 0; /*i used for traversaling in the file*/

    /*Traversal the file*/
    for (i = 0; i < bytes_read; i++)
    {
        /*Print each byte in file*/
        printf("%c", file_content[i]);
    }

    return;
}

/*Functions*********************************************************************
*
* Function name: app_print_disk_state.
* Description: Print the status of the disk to console.
*
END***************************************************************************/

void app_print_disk_state(disk_state_enum_t disk_state)
{
    /*Evaluate the disk state*/
    switch (disk_state)
    {
    case FAILED_TO_OPEN:
    {
        printf("\n\n\t\tFAILED TO OPEN DISK!");
        break;
    }
    case BAD_BOOT_SECTOR:
    {
        printf("\n\n\t\tDISK HAS BAD BOOT SECTOR!");
        break;
    }
    default:
        break;
    }
}

/*Functions*********************************************************************
*
* Function name: main.
* Description: function main of the program.
*
END***************************************************************************/

int main(void)
{
    fatfs_entry_list_struct_t dir_list; /*dir_list stores the directory entry list*/
    disk_state_enum_t disk_state;       /*disk_state stores the status of the disk*/
    int32_t choice = 0;                 /*choice stores the choice of user*/
    uint32_t check_choice = 0;          /*check_choice is used to check if user enter a right format input*/

    /*Initial the FATfs layer*/
    disk_state = fatfs_init("floppy.img");

    /*Check if the disk is in good state*/
    if (GOOD_CONDITION == disk_state)
    {
        /*Register the print file function to FATfs layer*/
        ResgisterPrint_file_func(app_print_file_content);

        /*Get the directory entry list*/
        dir_list = fatfs_read_dir(ROOT_DIR_12_LOGICAL_BASE_INDEX);

        /*Print the root directory entry list*/
        app_print_entry_list(&dir_list);

        while (1)
        {
            /*Loop until user enter a right input*/
            do
            {
                /*Get the user's choice*/
                printf("\n\n[OPTION] >> ");
                fflush(stdin);
                check_choice = scanf("%d", &choice);

                if ((0 == check_choice) || (check_choice < 0))
                {
                    printf("\n\n\tPlease re-enter your option or press 0 to exit!");
                }
            } while ((0 == check_choice) || (check_choice < 0));

            /*If user choose to exit the program*/
            if (choice == 0)
            {
                fatfs_de_init();
                exit(0);
            }
            /*If the user choice is a folder entry*/
            else if (FOLDER_ENTRY == dir_list.attribute[choice - 1])
            {
                /*Clear the screen*/
                system("cls");

                /*Clear the current directory entry list*/
                fatfs_clear_dir_list();

                /*Get the directory entry list of the user choice*/
                dir_list = fatfs_read_dir(dir_list.first_logical_cluster[choice - 1]);

                /*Print the directory entry list to console*/
                app_print_entry_list(&dir_list);
            }
            /*If the user choice is a file entry*/
            else if (FILE_ENTRY == dir_list.attribute[choice - 1])
            {
                printf("\n\n=>> [Read file ... ]");
                printf("\n\n");
                printf("\nFile: ");
                printf("\n\n");

                /*Read and print the file to console*/
                fatfs_read_file(dir_list.first_logical_cluster[choice - 1]);

                printf("\n");

                printf("\n\nPress any key to continue...");

                /*Pause screen*/
                getch();

                /*Clear screen*/
                system("cls");

                /*Print the current directory entry list to console*/
                app_print_entry_list(&dir_list);
            }
            else
            {
                /*Do nothing*/
            }
        }
    }
    else
    {
        /*Print the disk state to console*/
        app_print_disk_state(disk_state);
    }

    return 0;
}
/*End of file*/
