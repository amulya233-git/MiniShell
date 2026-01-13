#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "main.h"

// Global variables
extern int pid;            // PID of the currently running foreground process
extern jobs_t *head;       // Head of the linked list storing stopped background jobs
int last_status = 0;       // Stores exit status of last command
extern char prompt[];      // Shell prompt string

// Stores the last input command to manage jobs with Ctrl+Z
char last_input[1024];  

// ---------------------- SCAN INPUT FUNCTION ----------------------
// Reads user input, detects command type, and executes it
void scan_input(char *prompt, char *input_string)
{
    // Register signal handlers
    signal(SIGINT, my_handler);   // Handle Ctrl+C
    signal(SIGTSTP, my_handler);  // Handle Ctrl+Z
    signal(SIGCHLD, my_handler);  // Handle child termination

    while (1) // Infinite shell loop
    {
        printf("%s ", prompt);  // Print the prompt
        fflush(stdout);

        // Read user input
        if (fgets(input_string, 1024, stdin) == NULL)
        {
            printf("\n");  // Ctrl+D pressed, exit shell
            exit(0);
        }

        // Remove trailing newline
        input_string[strcspn(input_string, "\n")] = 0;

        // Store last input for job control
        strcpy(last_input, input_string);

        // ---------------------- Change Prompt Command ----------------------
        if (strncmp(input_string, "PS1=", 4) == 0)
        { 
            if (input_string[4] == '\0') 
            { 
                printf("invalid prompt\n"); 
                continue; 
            } 
            strcpy(prompt, input_string + 4); // Update prompt
            continue; 
        } 
        else if (strncmp(input_string, "PS1", 3) == 0 && input_string[3] != '=')
        { 
            printf("invalid prompt\n");
            continue; 
        }

        // ---------------------- Command Execution ----------------------
        char *cmd = get_command(input_string);     // Extract first word
        int type = check_command_type(cmd);        // Detect if built-in or external

        if (type == BUILTIN)
        {
            printf("it is internal cmd\n");
            int ret = execute_internal_commands(input_string); // Execute built-in

            // Save last_status unless echo $?, echo $$, echo $SHELL, echo $
            if (strcmp(input_string, "echo $?") != 0 &&
                strcmp(input_string, "echo $$") != 0 &&
                strcmp(input_string, "echo $SHELL") != 0 &&
                strcmp(input_string, "echo $") != 0)
            {
                last_status = ret;
            }
        }
        else if (type == EXTERNAL)
        {
            pid = fork(); // Create child process to execute external command

            if (pid < 0)
            {
                perror("fork failed");
            }
            else if (pid == 0) // Child process
            {
                // Restore default signal handling in child
                signal(SIGINT, SIG_DFL);   // Ctrl+C kills child
                signal(SIGTSTP, SIG_DFL);  // Ctrl+Z stops child

                execute_external_commands(input_string); // Execute command
                exit(1);
            }
            else // Parent process
            {
                int status;
                waitpid(pid, &status, WUNTRACED); // Wait for child (stopped or exited)

                if (WIFEXITED(status))
                    last_status = WEXITSTATUS(status); // Save exit status
            }
        }
        else
        {
            // Command not found
            printf("%s: command not found\n", cmd);
            last_status = 127;
        }
        pid = 0; // Reset foreground process PID
    }
}

// ---------------------- SIGNAL HANDLER ----------------------
// Handles Ctrl+C, Ctrl+Z, and child termination
void my_handler(int signum)
{
    int status;

    if (signum == SIGINT) // Ctrl+C
    {
        if(pid == 0)
        {
            printf("\n%s ", prompt); // Print prompt again
            fflush(stdout);
        }
    }
    else if (signum == SIGTSTP) // Ctrl+Z
    {
        if (pid == 0)
        {
            printf("\n%s ", prompt); // No foreground process, just refresh prompt
            fflush(stdout);
        }
        else
        {
            // Stop the foreground process
            kill(pid, SIGSTOP);

            // Save stopped process in jobs list
            insert_at_first(&head, pid, last_input);

            printf("\n");
        }
    }
    else if (signum == SIGCHLD)
    {
        // Reap terminated background children to avoid zombies
        while (waitpid(-1, &status, WNOHANG) > 0);
    }
}

// ---------------------- JOB CONTROL FUNCTIONS ----------------------
// Insert a stopped job at the beginning of the linked list
int insert_at_first(jobs_t **head, pid_t pid, char *cmd)
{
    jobs_t *new;
    new = malloc(sizeof(jobs_t));
    if (new == NULL)
        return -1; // malloc failed

    new->pid = pid;

    // Copy command into struct
    if (cmd != NULL)
        strncpy(new->command, cmd, sizeof(new->command) - 1);
    new->command[sizeof(new->command) - 1] = '\0'; // null-terminate

    new->link = *head; // link list
    *head = new;

    return 0;
}

// Delete first job from linked list (used after fg/bg)
int delete_first(jobs_t **head)
{
    if (*head == NULL)
        return -1;

    jobs_t *temp = *head;
    *head = temp->link;
    free(temp);
    return 0;
}
