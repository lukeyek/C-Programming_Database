#include <stdio.h>  // Standard I/O library
#include <ctype.h>  // Char classification and conversion library (e.g., isalpha, tolower)
#include <string.h> // String manipulation functions (e.g., strcmp)
#include <stdlib.h> // Program control, memory management, and basic utilities
#include <math.h>   // Math functions

#define FILE_NAME "P14_8-CMS.txt"
#define DB_NAME "StudentRecords"
#define MAX_ID_LEN 7
#define MAX_NAME_LEN 30
#define MAX_PROGRAMME_LEN 50
#define FILE_HEADER_LINES 5

// Structure representing student node in the linked list
typedef struct student_node {
    int id;
    char name[MAX_NAME_LEN + 1];           // +1 for null terminator
    char programme[MAX_PROGRAMME_LEN + 1]; // +1 for null terminator
    float marks;
    char grade[3]; // +1 for null terminator, +1 for (+/-) symbols
    struct student_node* next;
} STUDENT_NODE;

// Global variables
STUDENT_NODE* head = NULL; // Initialize head pointer for linked list
STUDENT_NODE* tail = NULL; // Initialize tail pointer for linked list
int node_count = 0; // Number of nodes in linked list
int is_file_open = 0; // Track whether database has been loaded to linked list
int is_changes_made = 0; // Track whether changes has been made to linked list

// Main function prototypes
void open_db();
void show_all_records();
void insert_record();
void query_record();
void update_record();
void delete_record();
void save_db();
void close_db();

// Get input function prototypes
int get_id(int* id);
int get_name(char* name);
int get_programme(char* programme);
int get_marks(float* marks);
int get_choice(); // Get 'y' or 'n' input 

// Utiltiy function prototypes
char* calculate_grade(float marks);
void reset_list();
void skip_header_lines(FILE* file_ptr);
void display_press_enter();
void clean_fgets(char* input);
void display_menu();
void run_cmd(char* cmd);

// Program starts here
int main() {
    char cmd[16];
    while (1) {
        display_menu(); // Display different menu depending if db file is open or not
        fgets(cmd, sizeof(cmd), stdin);
        clean_fgets(cmd);
        run_cmd(cmd);
    }
    return 0;
}

void open_db() {
    FILE* file_ptr = fopen(FILE_NAME, "r");
    if (!file_ptr) { // Handle file not found error
        fprintf(stderr, "\n[Error] Database file \"%s\" not found! Ensure correct file path is provided!\n", FILE_NAME);
        return;
    }
    skip_header_lines(file_ptr); // Skip header information for database
    char header_line_buffer[256];
    // Loop through opened file and start reading and loading student records into linked list
    while (1) {
        STUDENT_NODE* new_student_node = malloc(sizeof(STUDENT_NODE)); // Memory allocation for new student node
        if (!new_student_node) {
            fprintf(stderr, "\n[Error] Memory allocation failure!\n");
            fclose(file_ptr); // Close file before exiting
            return;
        }
        // Scan file lines, seperating fields based on commas
        int read_result = fscanf(file_ptr, "%7d,%30[^,],%50[^,],%f,%s",
            &new_student_node->id,
            new_student_node->name,
            new_student_node->programme,
            &new_student_node->marks,
            &new_student_node->grade);

        if (read_result == EOF) {
            free(new_student_node); // Free unused memory allocation
            break; // Exit loop on end of file
        }
        else if (read_result != 5) { // Ensure proper fields
            fprintf(stderr, "\n[Error] Malformed line in \"%s\" database!\n", DB_NAME);
            free(new_student_node); // Free unused memory allocation
            // Skip the rest of the line to move the file pointer forward
            fgets(header_line_buffer, sizeof(header_line_buffer), file_ptr);
            continue;
        }

        // Insert new student node to back of linked list
        new_student_node->next = NULL; // Initialize next pointer to NULL
        if (head == NULL) { // Check if linked list is empty
            head = new_student_node; // Set new node as head
        }
        else {
            tail->next = new_student_node; // Set new node to end of linked list
        }
        tail = new_student_node; // Update tail pointer
        node_count++;
    }
    fclose(file_ptr);
    is_file_open = 1;
    printf("\nCMS: Database file \"%s\" successfully opened! Found %d records!\n", FILE_NAME, node_count);
}

void show_all_records() {
    if (!head) {
        printf("\nCMS: No records found! 'INSERT' to add records!\n");
        return;
    }

    STUDENT_NODE* current = head;
    printf("\n%-7s  %-30s  %-50s  %-10s %-10s\n", "[ID]", "[Name]", "[Programme]", "[Marks]", "[Grade]");
    printf("===============================================================================================================\n");
    while (current) {
        printf("%-7d  %-30s  %-50s  %-10.1f %-10s\n",
            current->id, current->name, current->programme, current->marks, current->grade);
        current = current->next;
    }
    printf("===============================================================================================================\n");
    printf("CMS <SHOW ALL>: Found %d records in \"%s\" database!\n", node_count, DB_NAME);
    display_press_enter();
}

void insert_record() {
    // Display quick insert guide
    printf("\n==================== INSERT MENU =====================\n");
    printf("You will be prompted to provide the following details:\n");
    printf("%-12s %s\n", "- Student ID", "(7 digits)");
    printf("%-12s %s\n", "- Name", "(up to 30 characters)");
    printf("%-12s %s\n", "- Programme", "(up to 50 characters)");
    printf("%-12s %s\n", "- Marks", "(0.0 to 100.0)");
    printf("======================================================\n");

    int id;
    char name[MAX_NAME_LEN + 1]; // +1 for null terminator
    char programme[MAX_PROGRAMME_LEN + 1]; // +1 for null terminator
    float marks;

    // Prompt user for student ID
    while (1) {
        printf("CMS <INSERT 1/4>: Enter a 7-Digit Student ID ('Q' to cancel)\n>> P14_8: ");
        int id_status = get_id(&id); // Prompts user for student ID and pass it through validation, and returns status code
        if (id_status == 1) { // User enters input
            // After passing ID validation, check for duplicate student ID in linked list
            int duplicate_found = 0;
            STUDENT_NODE* current = head;
            while (current) {
                if (current->id == id) {
                    printf("\nCMS <INSERT>: Record with student ID=\"%d\" already exists! Please try again!\n", id);
                    duplicate_found = 1;
                    break;
                }
                current = current->next; // Move to next node
            }
            if (!duplicate_found) break; // Valid, non-duplicate student ID found
        }
        else if (id_status == 0) continue; // User enters invalid input, continue prompting
        else { // User cancels
            printf("\nCMS <INSERT>: Insert operation cancelled!\n");
            return;
        }
    }

    // Prompt user for student name
    while (1) {
        printf("CMS <INSERT 2/4>: Enter Student Name ('Q' to cancel)\n>> P14_8: ");
        int name_status = get_name(name); // Prompt user for student name and pass it through validation
        if (name_status == 1) break; // User enters valid input
        else if (name_status == 0) continue; // Invalid input, continue prompting
        else { // User cancels
            printf("\nCMS <INSERT>: Insert operation cancelled!\n");
            return;
        }
    }

    // Prompt user for student programme
    while (1) {
        printf("CMS <INSERT 3/4>: Enter Programme Name ('Q' to cancel)\n>> P14_8: ");
        int programme_status = get_programme(programme); // Prompt user for student programme and pass it through validation
        if (programme_status == 1) break; // User enters valid input
        else if (programme_status == 0) continue; // Invalid input, continue prompting
        else { // User cancels
            printf("\nCMS <INSERT>: Insert operation cancelled!\n");
            return;
        }
    }

    // Prompt user for student marks
    while (1) {
        printf("CMS <INSERT 4/4>: Enter Marks ('Q' to cancel)\n>> P14_8: ");
        int marks_status = get_marks(&marks); // Prompt user for marks and pass it through validation
        if (marks_status == 1) break; // User enters valid input
        else if (marks_status == 0) continue; // Invalid input, continue prompting
        else { // User cancels
            printf("\nCMS <INSERT>: Insert operation cancelled!\n");
            return;
        }
    }
    // Insert Confirmation
    while (1) {
        printf("================== CONFIRM INSERT ==================\n");
        printf("%11s %d\n", "Student ID:", id);
        printf("%11s %s\n", "Name:", name);
        printf("%11s %s\n", "Programme:", programme);
        printf("%11s %.1f\n", "Marks:", marks);
        printf("%11s %s (Auto-Calculated)\n", "Grade:", calculate_grade(marks));
        printf("====================================================\n");
        printf("CMS <INSERT>: Confirm Insert? (Y/N)\n>> P14_8: ");
        int choice_status = get_choice(); // Get 'Y' or 'N' from user, validates and prints any needed error msg
        if (choice_status == 1) break;// User say yes
        if (choice_status == 0) { // User say no
            printf("\nCMS <INSERT>: Insert operation cancelled!\n");
            return;
        }
    }


    STUDENT_NODE* new_student_node = malloc(sizeof(STUDENT_NODE)); // Memory allocation for new student node
    if (!new_student_node) {
        fprintf(stderr, "\n[Error] Memory allocation failure!\n");
        return;
    }
    // Fill new student node
    new_student_node->id = id;
    strncpy(new_student_node->name, name, MAX_NAME_LEN);
    strncpy(new_student_node->programme, programme, MAX_PROGRAMME_LEN);
    new_student_node->marks = marks;
    strcpy(new_student_node->grade, calculate_grade(marks));

    // Add new student to the end of linked list using tail pointer
    new_student_node->next = NULL;
    if (head == NULL) { // Linked list is empty
        head = new_student_node;
    }
    else { // Linked list has elements
        tail->next = new_student_node; // Point current tail to new student node
    }
    tail = new_student_node;
    node_count++;
    is_changes_made = 1;
    printf("\nCMS <INSERT>: Student record inserted successfully!\n");
}

void query_record() {
    // Check if the linked list is empty
    if (!head) {
        printf("\nCMS <QUERY>: No records to query! The database \"%s\" is empty!\n", DB_NAME);
        return;
    }

    char option[3]; // Variable to store the user's menu option
    while (1) {
        // Display the query menu
        printf("================= QUERY MENU ==================\n");
        printf("[1] Student ID [2] Name [3] Programme [4] Grade\n");
        printf("===============================================\n");
        printf("CMS <QUERY>: Enter Query Option [1-4] ('Q' to cancel)\n>> P14_8: ");
        fgets(option, sizeof(option), stdin);
        clean_fgets(option); // Clean user input (remove trailing newline)

        // Option 1: Query by Student ID
        if (strcmp(option, "1") == 0) {
            while (1) { // Loop to ensure valid input
                char id_input[10]; // Buffer for user input
                printf("CMS <QUERY>: Enter numeric keyword to query Student ID ('Q' to cancel)\n>> P14_8: ");
                fgets(id_input, sizeof(id_input), stdin);
                clean_fgets(id_input); // Clean user input

                if (strcasecmp(id_input, "q") == 0) { // Check if user wants to cancel
                    printf("\nCMS <QUERY>: Query by Student ID cancelled! Returning to query menu.\n");
                    break;
                }

                // Validate input (only numeric values allowed, max 7 digits)
                if (strlen(id_input) == 0) {
                    printf("\n[Error] Query is empty! Please try again.\n");
                    continue; // Prompt again
                }

                int valid = 1;
                if (strlen(id_input) > 7) { // Check if input exceeds maximum length
                    valid = 0;
                }
                for (int i = 0; id_input[i] != '\0'; i++) {
                    if (!isdigit(id_input[i])) { // Check if input contains non-numeric characters
                        valid = 0;
                        break;
                    }
                }
                if (!valid) {
                    printf("\n[Error] Invalid input! Only numeric values (max 7 digits) are allowed for Student ID search. Please try again.\n");
                    continue; // Prompt again
                }

                // Search for matching Student IDs in the linked list
                STUDENT_NODE* current = head;
                int record_found = 0; // Flag to check if any records are found
                while (current) {
                    char id_str[20];
                    snprintf(id_str, sizeof(id_str), "%d", current->id); // Convert numeric ID to string

                    if (strstr(id_str, id_input)) { // Check if input matches part of the ID
                        if (!record_found) { // Display header if it's the first matching record
                            printf("\n%-7s  %-30s  %-50s  %-10s  %-10s\n", "[ID]", "[Name]", "[Programme]", "[Marks]", "[Grade]");
                            printf("===============================================================================================================\n");
                            record_found = 1;
                        }
                        printf("%-7d  %-30s  %-50s  %-10.1f  %-10s\n", current->id, current->name, current->programme, current->marks, current->grade);
                    }
                    current = current->next; // Move to the next node
                }
                if (!record_found) { // If no records are found
                    printf("\nCMS <QUERY>: No records found with Student ID containing \"%s\". Please try again.\n", id_input);
                }
                else {
                    printf("===============================================================================================================\n");
                    display_press_enter();
                    break; // Exit the loop after successful query
                }
            }
        }

        // Option 2: Query by Name
        else if (strcmp(option, "2") == 0) {
            while (1) { // Loop to ensure valid input
                char name[100]; // Buffer for user input
                printf("CMS <QUERY>: Enter name to query ('Q' to cancel)\n>> P14_8: ");
                fgets(name, sizeof(name), stdin);
                clean_fgets(name); // Clean user input

                if (strcasecmp(name, "q") == 0) { // Check if user wants to cancel
                    printf("\nCMS <QUERY>: Query by name cancelled! Returning to query menu.\n");
                    break;
                }

                // Validate input (only alphabetic values and spaces allowed, max 30 characters)
                if (strlen(name) == 0) {
                    printf("\n[Error] Query is empty! Please try again.\n");
                    continue; // Prompt again
                }

                int valid = 1;
                if (strlen(name) > 30) { // Check if input exceeds maximum length
                    valid = 0;
                }
                for (int i = 0; name[i] != '\0'; i++) {
                    if (!isalpha(name[i]) && name[i] != ' ') { // Check if input contains invalid characters
                        valid = 0;
                        break;
                    }
                }
                if (!valid) {
                    printf("\n[Error] Invalid input! Only alphabetic values (max 30 characters) are allowed for name search. Please try again.\n");
                    continue; // Prompt again
                }

                // Convert input to lowercase for case-insensitive comparison
                char lowercase_name[100];
                for (int i = 0; name[i]; i++) {
                    lowercase_name[i] = tolower(name[i]);
                }
                lowercase_name[strlen(name)] = '\0';

                // Search for matching names in the linked list
                STUDENT_NODE* current = head;
                int record_found = 0;
                while (current) {
                    char lowercase_student_name[100];
                    for (int i = 0; current->name[i]; i++) {
                        lowercase_student_name[i] = tolower(current->name[i]);
                    }
                    lowercase_student_name[strlen(current->name)] = '\0';

                    if (strstr(lowercase_student_name, lowercase_name)) { // Check if input matches part of the name
                        if (!record_found) { // Display header if it's the first matching record
                            printf("\n%-7s  %-30s  %-50s  %-10s  %-10s\n", "[ID]", "[Name]", "[Programme]", "[Marks]", "[Grade]");
                            printf("===============================================================================================================\n");
                            record_found = 1;
                        }
                        printf("%-7d  %-30s  %-50s  %-10.1f  %-10s\n", current->id, current->name, current->programme, current->marks, current->grade);
                    }
                    current = current->next; // Move to the next node
                }
                if (!record_found) { // If no records are found
                    printf("\nCMS <QUERY>: No records found with name containing \"%s\". Please try again.\n", name);
                }
                else {
                    printf("===============================================================================================================\n");
                    display_press_enter();
                    break; // Exit the loop after successful query
                }
            }
        }

        // Option 3: Query by Programme
        else if (strcmp(option, "3") == 0) {
            while (1) { // Loop to ensure valid input
                char programme[100]; // Buffer for user input
                printf("CMS <QUERY>: Enter programme to query ('Q' to cancel)\n>> P14_8: ");
                fgets(programme, sizeof(programme), stdin);
                clean_fgets(programme); // Clean user input

                if (strcasecmp(programme, "q") == 0) { // Check if user wants to cancel
                    printf("\nCMS <QUERY>: Query by programme cancelled! Returning to query menu.\n");
                    break;
                }

                // Validate input (only alphabetic values and spaces allowed, max 50 characters)
                if (strlen(programme) == 0) {
                    printf("\n[Error] Query is empty! Please try again.\n");
                    continue; // Prompt again
                }

                int valid = 1;
                if (strlen(programme) > 50) { // Check if input exceeds maximum length
                    valid = 0;
                }
                for (int i = 0; programme[i] != '\0'; i++) {
                    if (!isalpha(programme[i]) && programme[i] != ' ') { // Check if input contains invalid characters
                        valid = 0;
                        break;
                    }
                }
                if (!valid) {
                    printf("\n[Error] Invalid input! Only alphabetic values (max 50 characters) are allowed for programme search. Please try again.\n");
                    continue; // Prompt again
                }

                // Convert input to lowercase for case-insensitive comparison
                char lowercase_programme[100];
                for (int i = 0; programme[i]; i++) {
                    lowercase_programme[i] = tolower(programme[i]);
                }
                lowercase_programme[strlen(programme)] = '\0';

                // Search for matching programmes in the linked list
                STUDENT_NODE* current = head;
                int record_found = 0;
                while (current) {
                    char lowercase_student_programme[100];
                    for (int i = 0; current->programme[i]; i++) {
                        lowercase_student_programme[i] = tolower(current->programme[i]);
                    }
                    lowercase_student_programme[strlen(current->programme)] = '\0';

                    if (strstr(lowercase_student_programme, lowercase_programme)) { // Check if input matches part of the programme
                        if (!record_found) { // Display header if it's the first matching record
                            printf("\n%-7s  %-30s  %-50s  %-10s  %-10s\n", "[ID]", "[Name]", "[Programme]", "[Marks]", "[Grade]");
                            printf("===============================================================================================================\n");
                            record_found = 1;
                        }
                        printf("%-7d  %-30s  %-50s  %-10.1f  %-10s\n", current->id, current->name, current->programme, current->marks, current->grade);
                    }
                    current = current->next; // Move to the next node
                }
                if (!record_found) { // If no records are found
                    printf("\nCMS <QUERY>: No records found with programme containing \"%s\". Please try again.\n", programme);
                }
                else {
                    printf("===============================================================================================================\n");
                    display_press_enter();
                    break; // Exit the loop after successful query
                }
            }
        }
        // Option 4: Query by Grade
        else if (strcmp(option, "4") == 0) {
            while (1) { // Loop to ensure valid input
                char grade[5]; // Buffer for user input
                printf("CMS <QUERY>: Enter grade to query (e.g., 'A+', 'B') ('Q' to cancel)\n>> P14_8: ");
                fgets(grade, sizeof(grade), stdin);
                clean_fgets(grade);

                if (strcasecmp(grade, "q") == 0) { // Check if user wants to cancel
                    printf("\nCMS <QUERY>: Query by grade cancelled! Returning to query menu.\n");
                    break;
                }

                // Validate input (must be a valid grade)
                const char* valid_grades[] = { "A+", "A", "A-", "B+", "B", "B-", "C+", "C", "D+", "D", "F" };
                int valid = 0;
                if (strlen(grade) == 0) {
                    printf("\n[Error] Query is empty! Please try again.\n");
                    continue; // Prompt again
                }
                for (int i = 0; i < 11; i++) {
                    if (strcasecmp(grade, valid_grades[i]) == 0) {
                        valid = 1;
                        break;
                    }
                }
                if (!valid) {
                    printf("\n[Error] Invalid input! Allowed grades are: A+, A, A-, B+, B, B-, C+, C, D+, D, F.\n");
                    continue; // Prompt again
                }

                // Search for matching grades in the linked list
                STUDENT_NODE* current = head;
                int record_found = 0;
                while (current) {
                    // If the query is a specific grade, match it exactly
                    if (strcasecmp(current->grade, grade) == 0) {
                        if (!record_found) { // Display header if it's the first matching record
                            printf("\n%-7s  %-30s  %-50s  %-10s  %-10s\n", "[ID]", "[Name]", "[Programme]", "[Marks]", "[Grade]");
                            printf("===============================================================================================================\n");
                            record_found = 1;
                        }
                        printf("%-7d  %-30s  %-50s  %-10.1f  %-10s\n", current->id, current->name, current->programme, current->marks, current->grade);
                    }

                    // If the query is a general grade, match any subgrade
                    else if ((strcasecmp(grade, "A") == 0 && (strcasecmp(current->grade, "A+") == 0 || strcasecmp(current->grade, "A-") == 0 || strcasecmp(current->grade, "A") == 0)) ||
                        (strcasecmp(grade, "B") == 0 && (strcasecmp(current->grade, "B+") == 0 || strcasecmp(current->grade, "B-") == 0 || strcasecmp(current->grade, "B") == 0)) ||
                        (strcasecmp(grade, "C") == 0 && (strcasecmp(current->grade, "C+") == 0 || strcasecmp(current->grade, "C") == 0)) ||
                        (strcasecmp(grade, "D") == 0 && (strcasecmp(current->grade, "D+") == 0 || strcasecmp(current->grade, "D") == 0)) ||
                        (strcasecmp(grade, "F") == 0 && (strcasecmp(current->grade, "F") == 0))) {
                        if (!record_found) { // Display header if it's the first matching record
                            printf("\n%-7s  %-30s  %-50s  %-10s  %-10s\n", "[ID]", "[Name]", "[Programme]", "[Marks]", "[Grade]");
                            printf("===============================================================================================================\n");
                            record_found = 1;
                        }
                        printf("%-7d  %-30s  %-50s  %-10.1f  %-10s\n", current->id, current->name, current->programme, current->marks, current->grade);
                    }

                    current = current->next; // Move to the next node
                }
                if (!record_found) { // If no records are found
                    printf("\nCMS <QUERY>: No records found with grade \"%s\". Please try again.\n", grade);
                }
                else {
                    printf("===============================================================================================================\n");
                    display_press_enter();
                    break; // Exit the loop after successful query
                }
            }
        }

        // Exit the query menu
        else if (strcasecmp(option, "q") == 0) {
            printf("\nCMS <QUERY>: Returning to the main menu...\n");
            break;
        }

        // Handle invalid inputs
        else {
            fprintf(stderr, "\n[Error] Invalid input! Please enter option [1-4] only!\n");
        }
    }
}

void update_record() {
    if (!head) {
        printf("\nCMS <UPDATE>: No records to update! The database \"%s\" is empty!\n", DB_NAME);
        return;
    }
    int id;
    char name[MAX_NAME_LEN + 1]; // +1 for null terminator
    char programme[MAX_PROGRAMME_LEN + 1]; // +1 for null terminator
    float marks;

    while (1) {
        printf("CMS <UPDATE>: Enter 7-Digit Student ID to Update ('Q' to stop UPDATE)\n>> P14_8: ");
        int id_status = get_id(&id); // Prompt user for student ID and pass it through validation
        if (id_status == -1) { // User cancels
            printf("\nCMS <UPDATE>: Update operation cancelled!\n");
            return;
        }
        if (id_status == 0) { // Invalid input, continue prompting
            continue;
        }

        char option[3];
        int record_found = 0;
        STUDENT_NODE* current = head;

        while (current) {
            if (current->id == id) { // Record found
                record_found = 1;
                while (1) {
                    printf("========================== STUDENT FOUND ===========================\n");
                    printf("%11s %d\n", "Student ID:", current->id);
                    printf("%11s %s\n", "Name:", current->name);
                    printf("%11s %s\n", "Programme:", current->programme);
                    printf("%11s %.1f\n", "Marks:", current->marks);
                    printf("%11s %s\n", "Grade:", current->grade);
                    printf("====================================================================\n");
                    printf("[1] Update Name [2] Update Programme [3] Update Marks [4] Update All\n");
                    printf("====================================================================\n");
                    printf("CMS <UPDATE>: Enter Update Option [1-4] ('Q' to cancel)\n>> P14_8: ");
                    fgets(option, sizeof(option), stdin);
                    clean_fgets(option);

                    if (strcmp(option, "1") == 0) { // User chooses to update name
                        while (1) {
                            printf("CMS <UPDATE>: Enter New Student Name ('Q' to stop updating Name)\n>> P14_8: ");
                            int name_status = get_name(name); // Prompt user for student name and pass it through validation
                            if (name_status == -1) { // User cancels
                                printf("\nCMS <UPDATE>: Update by name cancelled!\n");
                                break;
                            }
                            if (name_status == 0) { // Invalid input, prompt again
                                printf("\n[Error] Invalid name input. Please try again.\n");
                                continue;
                            }
                            while(1){
                                 printf("CMS <UPDATE>: Confirm name update from \"%s\" to \"%s\"? (Y/N)\n>> P14_8:  ", current->name, name);
                                int confirm_status = get_choice();
                                if (confirm_status == 1) { // User confirms
                                    strncpy(current->name, name, MAX_NAME_LEN);
                                    printf("\nCMS <UPDATE>: Name successfully updated!\n");
                                    is_changes_made = 1;
                                    break;
                                }
                                else if(confirm_status == 0){
                                    printf("\nCMS <UPDATE>: Update by name cancelled!\n");
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    else if (strcmp(option, "2") == 0) { // User chooses to update programme
                        while (1) {
                            printf("CMS <UPDATE>: Enter New Programme ('Q' to stop updating Programme)\n>> P14_8: ");
                            int programme_status = get_programme(programme); // Prompt user for programme name and pass it through validation
                            if (programme_status == -1) { // User cancels
                                printf("\nCMS <UPDATE>: Update by programme cancelled!\n");
                                break;
                            }
                            if (programme_status == 0) { // Invalid input, prompt again
                                printf("\n[Error] Invalid programme input. Please try again.\n");
                                continue;
                            }
                            while(1){
                                printf("CMS <UPDATE>: Confirm programme update from \"%s\" to \"%s\"? (Y/N)\n>> ", current->programme, programme);
                                int confirm_status = get_choice();
                                if (confirm_status == 1) { // User confirms
                                    strncpy(current->programme, programme, MAX_PROGRAMME_LEN);
                                    printf("\nCMS <UPDATE>: Programme successfully updated!\n");
                                    is_changes_made = 1;
                                    break;
                                }
                                else if (confirm_status == 0){
                                    printf("\nCMS <UPDATE>: Update by programme cancelled!\n");
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    else if (strcmp(option, "3") == 0) { // User chooses to update marks
                        while (1) {
                            printf("CMS <UPDATE>: Enter New Marks ('Q' to stop updating Marks)\n>> P14_8: ");
                            int marks_status = get_marks(&marks); // Prompt user for marks and pass it through validation
                            if (marks_status == -1) { // User cancels
                                printf("\nCMS <UPDATE>: Update by marks cancelled!\n");
                                break;
                            }
                            if (marks_status == 0) { // Invalid input, prompt again
                                continue;
                            }
                            while(1){
                                printf("CMS <UPDATE>: Confirm updating marks from \"%.1f\" to \"%.1f\"? (Y/N)\n>> P14_8: ", current->marks, marks);
                                int confirm_status = get_choice();
                                if (confirm_status == 1) { // User confirms
                                    current->marks = marks;
                                    strcpy(current->grade, calculate_grade(marks));
                                    printf("\nCMS <UPDATE>: Marks successfully updated!\n");
                                    is_changes_made = 1;
                                    break;
                                }
                                else if(confirm_status == 0){
                                    printf("\nCMS <UPDATE>: Update by marks cancelled!\n");
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    
                    else if (strcmp(option, "4") == 0) { // User chooses to update marks
                        int if_cancel = 0;
                        // Update name
                        while (1) {
                            printf("CMS <UPDATE>: Enter New Name ('Q' to stop updating)\n>> P14_8: ");
                            int name_status = get_name(name); // Prompt user for marks and pass it through validation
                            if (name_status == -1) { // User cancels
                                printf("\nCMS <UPDATE>: Update operation cancelled!\n");
                                if_cancel = 1;
                                break;
                            }
                            if (name_status == 0) { // Invalid input, prompt again
                                continue;
                            }
                            break;
                        }
                        if(if_cancel) continue;

                        // Update programme
                        while (1) {
                            printf("CMS <UPDATE>: Enter New Programme ('Q' to stop updating)\n>> P14_8: ");
                            int programme_status = get_programme(programme); // Prompt user for marks and pass it through validation
                            if (programme_status == -1) { // User cancels
                                printf("\nCMS <UPDATE>: Update operation cancelled!\n");
                                if_cancel = 1;
                                break;
                            }
                            if (programme_status == 0) { // Invalid input, prompt again
                                continue;
                            }
                            break;
                        }
                        if(if_cancel) continue;

                        // Update marks
                        while (1) {
                            printf("CMS <UPDATE>: Enter New Marks ('Q' to stop updating)\n>> P14_8: ");
                            int marks_status = get_marks(&marks); // Prompt user for marks and pass it through validation
                            if (marks_status == -1) { // User cancels
                                printf("\nCMS <UPDATE>: Update operation cancelled!\n");
                                if_cancel = 1;
                                break;
                            }
                            if (marks_status == 0) { // Invalid input, prompt again
                                continue;
                            }
                            break;
                        }
                        if(if_cancel) continue;

                        while(1){
                            printf("==================== CONFIRM UPDATE =====================\n");
                            printf("%10s %s -> %s\n", "Name:", current->name, name);
                            printf("%10s %s -> %s\n", "Programme:", current->programme, programme);
                            printf("%10s %.1f -> %.1f\n", "Marks:", current->marks, marks);
                            printf("==========================================================\n");
                      
                            printf("CMS <UPDATE>: Confirm update? (Y/N)\n>> P14_8: ");
                            int confirm_status = get_choice();
                            if (confirm_status == 1) { // User confirms
                                strncpy(current->name, name, MAX_NAME_LEN);
                                strncpy(current->programme, programme, MAX_PROGRAMME_LEN);
                                current->marks = marks;
                                strcpy(current->grade, calculate_grade(marks));
                                printf("\nCMS <UPDATE>: Update successful!\n");
                                is_changes_made = 1;
                                return;
                            }
                            else if(confirm_status == 0){
                                printf("\nCMS <UPDATE>: Update cancelled!\n");
                                if_cancel = 1;
                                break;
                            }

                        } 
                        if(if_cancel) continue;
                    }
                    else if (strcasecmp(option, "q") == 0) { // User cancels
                        printf("\nCMS <UPDATE>: Update operation cancelled!\n");
                        return;
                    }
                    else { // User enters invalid input
                        printf("\n[Error] Invalid option. Please enter [1-4] or 'Q' to cancel.\n");
                        continue;
                    }
                }
            }
            current = current->next;
        }
        if (!record_found) {
            printf("CMS: Record with student ID=\"%d\" not found!\n", id);
        }
    }
}

void delete_record() {
    int id;
    if (!head) {
        printf("\nCMS <DELETE>: No records to delete! The database \"%s\" is empty!\n", DB_NAME);
        return;
    }

    while (1) {
        printf("CMS <DELETE>: Enter 7-Digit Student ID to Delete ('Q' to cancel)\n>> P14_8: ");
        int id_status = get_id(&id); // Prompt user for student ID and pass it through validation
        if (id_status == -1) { // User cancels
            printf("\nCMS <DELETE>: Delete operation cancelled!\n");
            return;
        }
        if (id_status == 0) { // Invalid input, continue prompting
            continue;
        }

        STUDENT_NODE* current = head; // Setup current pointer to start from head
        STUDENT_NODE* prev = NULL;  // Setup prev pointer for deleting node in linked list
        while (current) {
            // If student id input matches student id in database file
            if (current->id == id) {
                // Confirmation for delete
                while (1) {
                    printf("================== STUDENT FOUND ===================\n");
                    printf("%11s %d\n", "Student ID:", current->id);
                    printf("%11s %s\n", "Name:", current->name);
                    printf("%11s %s\n", "Programme:", current->programme);
                    printf("%11s %.1f\n", "Marks:", current->marks);
                    printf("%11s %s (Auto-Calculated)\n", "Grade:", current->grade);
                    printf("====================================================\n");
                    printf("CMS <DELETE>: Confirm Delete? (Y/N)\n>> P14_8: ");
                    int choice_status = get_choice(); // Get 'Y' or 'N' from user, validates and prints any needed error msg
                    if (choice_status == 1) break;// User say yes
                    if (choice_status == 0) { // User say no
                        printf("\nCMS <DELETE>: Delete operation cancelled!\n");
                        return;
                    }
                }
                if (prev == NULL) { // Indicates that head node is the matched node
                    head = current->next; // Delete current node which is head
                }
                else {
                    prev->next = current->next; // Delete current node
                }

                if (current->next == NULL) { // Delete tail need if the last node happens to be matched node
                    tail = prev;
                }

                free(current); // Free allocated memory for deleted node
                current = NULL;

                node_count--;
                is_changes_made = 1; // Change status of changes made

                // Handle empty list case
                if (!head) {
                    tail = NULL; // Update tail if the list becomes empty
                }
                printf("\nCMS <DELETE>: Record with student ID=\"%d\" successfully deleted!\n", id);
                return;
            }

            // If student id input does not match id in database file, move forward to next node
            prev = current;
            current = current->next;
        }
        // If student id input not found in database file
        printf("\nCMS <DELETE>: Record with student ID=\"%d\" not found!\n", id);
    }
}

void save_db() {
    FILE* file_ptr = fopen(FILE_NAME, "w");
    if (!file_ptr) { // Handle file not found error
        fprintf(stderr, "\n[Error] Database file \"%s\" not found! Ensure correct file path is provided!\n", FILE_NAME);
        return;
    }
    // Write new database file header
    fprintf(file_ptr, "==============================\n");
    fprintf(file_ptr, "File Name: %s\n", FILE_NAME);
    fprintf(file_ptr, "Database Name: %s\n", DB_NAME);
    fprintf(file_ptr, "==============================\n");
    fprintf(file_ptr, "[ID],[Name],[Programme],[Marks],[Grade]\n");

    if (head) {

    }
    STUDENT_NODE* current = head;
    while (current) {
        fprintf(file_ptr, "%d,%s,%s,%.1f,%s\n", current->id, current->name, current->programme, current->marks, current->grade);
        current = current->next;
    }

    fclose(file_ptr); // Close file after writing
    is_changes_made = 0; // Reset status for changes made
    printf("\nCMS: Saved successfully to database file \"%s\"!\n", FILE_NAME);
}

void close_db() {
    if (is_changes_made == 1) {
        while (1) {
            printf("CMS <CLOSE>: You have unsaved changes! Are you sure you want to close the database file? (Y/N)\n>> P14_8: ");
            int choice_status = get_choice(); // returns 1 for 'y', 0 for 'n', -1 for invalid input
            if (choice_status == 1) {
                break;
            }
            if (choice_status == 0) {
                printf("\nCMS <CLOSE>: Close operation cancelled! Unsaved changes remain!\n");
                return;
            }
        }
    }
    // If linked list is not empty, free its memory and reset node count
    if (head != NULL) {
        reset_list();
    }
    is_file_open = 0; // Reset loaded file status
    is_changes_made = 0; // Reset changes made status
    printf("\nCMS: Database file \"%s\" successfully closed! Returning to the main menu!\n", FILE_NAME);
}


void remove_extra_spaces(char *str) {
    int i = 0, j = 0;
    int len = strlen(str);

    // Trim leading spaces
    while (isspace(str[i])) {
        i++;
    }

    // Process the string
    for (; i < len; i++) {
        if (!isspace(str[i])) { // If it's not a space, copy the character
            str[j++] = str[i];
        } else if (j > 0 && !isspace(str[j - 1])) { // Add a single space if previous char wasn't a space
            str[j++] = ' ';
        }
    }

    // Remove trailing spaces (if any)
    if (j > 0 && str[j - 1] == ' ') {
        j--;
    }

    // Null-terminate the result string
    str[j] = '\0';
}

int get_id(int* id) {
    char id_input[MAX_ID_LEN + 2]; // +1 for null terminator, +1 for buffer
    // Get input for student ID
    fgets(id_input, sizeof(id_input), stdin);
    clean_fgets(id_input);

    if(id_input[0] == '0'){
        fprintf(stderr, "\n[Error] Student ID cannot start with \"0\"! Please try again!\n");
        return 0;
    }
    // Check if user cancel operation
    if (strcasecmp(id_input, "Q") == 0) {
        return -1;
    }

    // Student ID validation
    int len = strlen(id_input);
    if (len == 0) {
        fprintf(stderr, "\n[Error] Student ID cannot be empty! Please try again!\n");
        return 0;
    }
    if (!(len == MAX_ID_LEN && strspn(id_input, "0123456789") == MAX_ID_LEN)) {
        fprintf(stderr, "\n[Error] Student ID must be exactly 7 numeric characters! Please try again!\n");
        return 0;
    }

    // Valid student id input, convert string input to int, assign it to value of id pointer
    *id = atoi(id_input);
    return 1;
}

int get_name(char* name) {
    char name_input[MAX_NAME_LEN + 2]; // +1 for null terminator, +1 for buffer
    // Get input for student name
    fgets(name_input, sizeof(name_input), stdin);
    // Student name validation
    int len = strlen(name_input);
    if (len > MAX_NAME_LEN) {
        fprintf(stderr, "\n[Error] Student name exceeds %d character limit! Please try again!\n", MAX_NAME_LEN);
        clean_fgets(name_input);
        return 0;
    }
    clean_fgets(name_input);
    len = strlen(name_input);
    // Check if user cancel operation
    if (strcasecmp(name_input, "Q") == 0) {
        return -1;
    }
    if (len == 0) {
        fprintf(stderr, "\n[Error] Student name cannot be empty! Please try again!\n");
        return 0;
    }
    for (int i = 0; i < len; i++) {
        if (!isalpha(name_input[i]) && !isspace(name_input[i])) {
            fprintf(stderr, "\n[Error] Student name contains non-alphabet characters! Please try again!\n");
            return 0;
        }
    }

    remove_extra_spaces(name_input);
    // Valid student name input, copy name input to value of name pointer
    strncpy(name, name_input, MAX_NAME_LEN);
    return 1;
}

int get_programme(char* programme) {
    char programme_input[MAX_PROGRAMME_LEN + 2]; // +1 for null terminator + 1 for buffer
    // Get input for programme name
    fgets(programme_input, sizeof(programme_input), stdin);

    // Programme name validation
    int len = strlen(programme_input);
    if (len > MAX_PROGRAMME_LEN) {
        fprintf(stderr, "\n[Error] Programme name exceeds %d character limit! Please try again!\n", MAX_PROGRAMME_LEN);
        clean_fgets(programme_input);
        return 0;
    }
    clean_fgets(programme_input);
    len = strlen(programme_input);
    // Check if user cancel operation
    if (strcasecmp(programme_input, "Q") == 0) {
        return -1;
    }
    if (len == 0) {
        fprintf(stderr, "\n[Error] Programme name cannot be empty! Please try again!\n");
        return 0;
    }
    for (int i = 0; i < len; i++) {
        if (!isalpha(programme_input[i]) &&
            !isspace(programme_input[i]) &&
            programme_input[i] != '-' &&
            programme_input[i] != '&' &&
            programme_input[i] != '.' &&
            programme_input[i] != '(' &&
            programme_input[i] != ')') {
            fprintf(stderr, "\n[Error] Programme name contains invalid character: \"%c\"! Please try again!\n", programme_input[i]);
            return 0;
        }
    }
    remove_extra_spaces(programme_input);
    // Valid programme name input, copy programme input to value of programme pointer
    strncpy(programme, programme_input, MAX_PROGRAMME_LEN);
    return 1;
}

int get_marks(float* marks) {
    char marks_input[7]; // 5 for marks (e.g., "100.0"), +1 for null terminator, +1 for buffer
    fgets(marks_input, sizeof(marks_input), stdin);
    clean_fgets(marks_input);

    // Check if user cancel operation
    if (strcasecmp(marks_input, "Q") == 0) {
        return -1;
    }

    // Marks input validation
    int len = strlen(marks_input);
    int dot_count = 0;
    if (len == 0) {
        fprintf(stderr, "\n[Error] Marks cannot be empty! Please try again!\n");
        return 0;
    }
    for (int i = 0; i < len; i++) {
        if (isdigit(marks_input[i])) {
            continue;
        }
        if (marks_input[i] == '.') {
            if (dot_count == 1) { // Only one dot allowed
                fprintf(stderr, "\n[Error] Marks cannot contain multiple decimal points! Please try again!\n");
                return 0;
            }
            dot_count++;
        }
        else { // Invalid characters present
            fprintf(stderr, "\n[Error] Invalid marks format! Marks must be between 0.0 and 100.0! Please try again!\n");
            return 0;
        }
    }
    float temp_marks = atof(marks_input); // Convert string to float after validation
    if (temp_marks < 0.0 || temp_marks > 100.0) { // Check range
        fprintf(stderr, "\n[Error] Marks must be between 0.0 and 100.0! Please try again.\n");
        return 0;
    }

    // Valid marks input, assign to value of marks pointer
    temp_marks = round(temp_marks * 10) / 10; // Round to 1 decimal place
    *marks = temp_marks;
    return 1;
}

// Prompts and validates user input for 'y' or 'n', returns status
int get_choice() {
    char choice_input[3]; // +1 for char, +1 for newline, +1 for buffer
    fgets(choice_input, sizeof(choice_input), stdin);
    clean_fgets(choice_input);

    // Choice validation
    if (strcasecmp(choice_input, "Y") == 0) return 1; // Valid input, user enter y
    if (strcasecmp(choice_input, "N") == 0) return 0; // Valid input, user enter n
    else {
        fprintf(stderr, "\n[Error] Invalid input! Please enter 'Y' or 'N'!\n");
        return -1; // Invalid input, user did not enter y/n
    }
}

// Determine student grade based on marks
char* calculate_grade(float marks) {
    if (marks >= 85) return "A+";
    if (marks >= 80) return "A";
    if (marks >= 75) return "A-";
    if (marks >= 70) return "B+";
    if (marks >= 65) return "B";
    if (marks >= 60) return "B-";
    if (marks >= 55) return "C+";
    if (marks >= 50) return "C";
    if (marks >= 45) return "D+";
    if (marks >= 40) return "D";
    if (marks >= 0) return "F";
}

// Reset linked list by deallocating memory for nodes and resetting node count
void reset_list() {
    node_count = 0; // Reset node count (counter tracking number of nodes in list)
    STUDENT_NODE* current = head; // Initialize pointer to access linked list starting from head
    while (current) { // Loop until end of list
        STUDENT_NODE* temp = current;
        current = current->next; // Move to next node in linked list
        free(temp); // Free up memory for temp (previous "current" node)
    }
    head = NULL; // Reset head pointer to NULL as list is now empty
}

// Skip header information for database (assume file pointer is already validated)
void skip_header_lines(FILE* file_ptr) {
    char buffer[128]; // Buffer to read and discard metadata
    for (int i = 0; i < FILE_HEADER_LINES; i++) {
        if (!fgets(buffer, sizeof(buffer), file_ptr)) {
            fprintf(stderr, "\n[Error] Reached EOF or encountered error while skipping header lines!\n");
            break;
        };
    }
}

void display_press_enter() {
    printf(">> P14_8: Press [Enter] to continue..");
    while (getchar() != '\n'); // Wait for user to press Enter
    printf("\n");
}

void clean_fgets(char* input) {
    // Check for input overflow before trimming as trimming would affect input[len - 1]
    int len = strlen(input);
    int has_overflow = len > 0 && input[len - 1] != '\n';

    // Trim any leading and trailing white spaces
    char* start = input; // Initialize pointer to start of input
    while (isspace((unsigned char)*start)) {
        start++;  // Move start pointer forward passing any leading white space
    }

    // Handle case where input is only spaces
    if (*start == '\0') {
        input[0] = '\0';  // Empty the string
        return;
    }
    char* end = start + strlen(start) - 1; // Initialize end pointer
    while (isspace((unsigned char)*end) && end > start) { // Remove trailing spaces
        *end = '\0';
        end--;
    }
    if (start != input) {
        // Shift the trimmed string to the start of the buffer
        memmove(input, start, strlen(start) + 1); // +1 for null terminator
    }

    // Handle input buffer overflow
    if (has_overflow) {
        int ch;
        while ((ch = getchar()) != '\n'); // Clear any input overflow
    }
    else {
        input[strcspn(input, "\n")] = '\0'; // If no input overflow, remove trailing newline
    }
}

void display_menu() {
    if (is_file_open) {
        printf("=========================================\n");
        printf("     P14_8 - Class Management System\n");
        printf("=========================================\n");
        printf("   %-12s %-12s %-12s\n", "[1] SHOW ALL", "[2] INSERT", "[3] QUERY");
        printf("   %-12s %-12s %-12s\n", "[4] UPDATE", "[5] DELETE", "[6] SAVE");
        printf("   %-12s %-12s %-12s\n", "[7] CLOSE", "[8] EXIT", "[9] HELP");
        printf("=========================================\n");
        printf("CMS: Enter an option [1-9] or type command:\n>> P14_8: ");
    }
    else {
        printf("================ WELCOME ================\n");
        printf("     P14_8 - Class Management System\n");
        printf("   %-12s %-12s %-12s\n", "[1] OPEN", "[2] EXIT", "[3] HELP");
        printf("=========================================\n");
        printf("CMS: Enter an option [1-3] or type command:\n>> P14_8: ");
    }
}

void run_cmd(char* cmd) {
    if (is_file_open) {
        if (strcmp(cmd, "1") == 0 || strcasecmp(cmd, "SHOW ALL") == 0) show_all_records();
        else if (strcmp(cmd, "2") == 0 || strcasecmp(cmd, "INSERT") == 0) insert_record();
        else if (strcmp(cmd, "3") == 0 || strcasecmp(cmd, "QUERY") == 0) query_record();
        else if (strcmp(cmd, "4") == 0 || strcasecmp(cmd, "UPDATE") == 0) update_record();
        else if (strcmp(cmd, "5") == 0 || strcasecmp(cmd, "DELETE") == 0) delete_record();
        else if (strcmp(cmd, "6") == 0 || strcasecmp(cmd, "SAVE") == 0) save_db();
        else if (strcmp(cmd, "7") == 0 || strcasecmp(cmd, "CLOSE") == 0) close_db();
        else if (strcmp(cmd, "8") == 0 || strcasecmp(cmd, "EXIT") == 0) {
            printf("\n=========================================\n");
            printf("   Exiting program! Have a great day!     \n");
            printf("=========================================\n");
            exit(0);
        }
        else if (strcmp(cmd, "9") == 0 || strcasecmp(cmd, "HELP") == 0) {
            printf("\nCMS: (Available Commands)\n");
            printf("  %-8s - %-50s\n", "SHOW ALL", "Display all student records");
            printf("  %-8s - %-50s\n", "INSERT", "Add a new student record");
            printf("  %-8s - %-50s\n", "QUERY", "Find student records by id, name, programme or grade");
            printf("  %-8s - %-50s\n", "UPDATE", "Modify existing student record");
            printf("  %-8s - %-50s\n", "DELETE", "Delete existing student record");
            printf("  %-8s - %-50s\n", "SAVE", "Save changes made to student records");
            printf("  %-8s - %-50s\n", "CLOSE", "Close the database file and return to main menu");
            printf("  %-8s - %-50s\n", "EXIT", "Exit the program");
            printf("  %-8s - %-50s\n", "HELP", "View list of available commands");
            display_press_enter();
        }
        else {
            fprintf(stderr, "\n[Error] Invalid input! Please enter option [1-9] only!\n");
        }
    }
    else {
        if (strcmp(cmd, "1") == 0 || strcasecmp(cmd, "OPEN") == 0) open_db();
        else if (strcmp(cmd, "2") == 0 || strcasecmp(cmd, "EXIT") == 0) {
            printf("\n=========================================\n");
            printf("   Exiting program! Have a great day!     \n");
            printf("=========================================\n");
            exit(0);
        }
        else if (strcmp(cmd, "3") == 0 || strcasecmp(cmd, "HELP") == 0) {
            printf("\nCMS: (Available Commands)\n");
            printf("  %-8s - %-50s\n", "OPEN", "Open the database file");
            printf("  %-8s - %-50s\n", "EXIT", "Exit the program");
            printf("  %-8s - %-50s\n", "HELP", "View list of available commands");
            display_press_enter();
        }
        else {
            fprintf(stderr, "\n[Error] Invalid input! Please enter option [1-3] only!\n");
        }
    }
}
