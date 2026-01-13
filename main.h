#ifndef MAIN_H
#define MAIN_H

#include <sys/types.h>  // For pid_t type

/* -------------------- Command Types -------------------- */
#define BUILTIN  1       // Internal shell command (cd, pwd, echo, etc.)
#define EXTERNAL 2       // External system command (ls, grep, etc.)
#define NO_COMMAND 3     // Command not recognized

/* -------------------- Job Structure -------------------- */
typedef struct jobs
{
    pid_t pid;           // Process ID of the job
    char command[100];   // Command string executed
    struct jobs *link;   // Pointer to next job in linked list
} jobs_t;

/* -------------------- Global Variables -------------------- */
extern jobs_t *head;              // Head of job linked list (for bg/fg jobs)
extern pid_t pid;                 // Current foreground process ID
extern int last_status;           // Last executed command status
extern char prompt[];             // Shell prompt string
extern char *external_commands[155]; // Array to store list of external commands

/* -------------------- Scan Functions -------------------- */
void scan_input(char *prompt, char *input_string);  
// Scans user input, handles signals, decides whether command is builtin or external

/* -------------------- Signal Handling -------------------- */
void my_handler(int signum);  
// Handles Ctrl+C (SIGINT), Ctrl+Z (SIGTSTP), and terminated background processes (SIGCHLD)

/* -------------------- Job Helper Functions -------------------- */
int insert_at_first(jobs_t **head, pid_t pid, char *cmd); 
// Inserts a new job at the start of job list (used for stopped jobs)

int delete_first(jobs_t **head); 
// Deletes the first job from the job list (after fg/bg execution)

/* -------------------- Command Helper Functions -------------------- */
char *get_command(char *input_string);  
// Extracts the first word from input string (command name)

void extract_external_commands(char **external_commands);  
// Reads external commands from file and stores in array

int check_command_type(char *command);  
// Determines if a command is BUILTIN, EXTERNAL, or NO_COMMAND

void execute_external_commands(char *input_string);  
// Executes external commands, supports pipes

int execute_internal_commands(char *input_string);  
// Executes builtin commands like cd, pwd, echo, fg, bg, jobs, etc.

#endif
