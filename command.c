#include <stdio.h>      // Standard input/output functions
#include <string.h>     // String handling functions
#include <stdlib.h>     // malloc(), exit()
#include <sys/types.h>  // Data types used in system calls
#include <sys/stat.h>   // File information
#include <fcntl.h>      // open()
#include <unistd.h>     // fork(), exec(), pipe(), read(), write()
#include <signal.h>     // Signal handling
#include <sys/wait.h>   // wait(), waitpid()
#include "main.h"       // Project header file

/* Global job list head */
jobs_t *head = NULL;

/* Stores PID of currently running foreground process */
pid_t pid = 0;

/* Stores exit status of last command */
extern int last_status;

/* Shell prompt */
extern char prompt[];

/* Array to store all external commands read from file */
char *external_commands[155];

/* Buffer to store extracted command name */
char command[64];

/* List of built-in shell commands */
char *builtins[] = {
    "echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs",
    "let", "eval", "set", "unset", "export", "declare", "typeset",
    "readonly", "getopts", "source", "exit", "exec", "shopt",
    "caller", "true", "type", "hash", "bind", "help",
    "fg", "bg", "jobs",
    NULL
};

/*------------------------------------------------------------*/
/* Extract first word (command) from input string              */
/*------------------------------------------------------------*/
char *get_command(char *input_string)
{
    int i = 0, j = 0;
    static char command_local[50];   // Static so value persists after return

    /* Skip leading spaces and tabs */
    while (input_string[i] == ' ' || input_string[i] == '\t')
        i++;

    /* Copy characters until space/newline/tab */
    while (input_string[i] != ' ' &&
           input_string[i] != '\n' &&
           input_string[i] != '\t')
    {
        command_local[j++] = input_string[i++];
    }

    /* Null terminate command string */
    command_local[j] = '\0';

    return command_local;
}

/*------------------------------------------------------------*/
/* Read external commands from file and store in array        */
/*------------------------------------------------------------*/
void extract_external_commands(char **external_commands)
{
    /* Open file containing external commands */
    int fp = open("external.txt", O_RDONLY);
    if (fp == -1)
    {
        perror("error opening file");
        return;
    }

    char buffer[4096];

    /* Read file content */
    int n = read(fp, buffer, 4095);
    if (n < 0)
    {
        write(2, "read failed\n", 12);
        close(fp);
        exit(1);
    }

    buffer[n] = '\0';   // Null terminate buffer
    close(fp);

    int i = 0, j = 0, k = 0;
    char temp[100];

    /* Split file content into words */
    while (i < n)
    {
        char ch = buffer[i];

        /* If delimiter found */
        if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r')
        {
            if (k > 0)
            {
                temp[k] = '\0';
                external_commands[j] = malloc(k + 1);
                strcpy(external_commands[j], temp);
                j++;
                k = 0;
            }
        }
        else
        {
            temp[k++] = ch;
        }
        i++;
    }

    /* Store last word */
    if (k > 0)
    {
        temp[k] = '\0';
        external_commands[j] = malloc(k + 1);
        strcpy(external_commands[j], temp);
        j++;
    }

    external_commands[j] = NULL;  // Mark end of array
}

/*------------------------------------------------------------*/
/* Identify command type: BUILTIN / EXTERNAL / NO_COMMAND     */
/*------------------------------------------------------------*/
int check_command_type(char *command)
{
    /* Check in built-in list */
    for (int i = 0; builtins[i] != NULL; i++)
    {
        if (strcmp(command, builtins[i]) == 0)
            return BUILTIN;
    }

    /* Check in external command list */
    for (int i = 0; external_commands[i] != NULL; i++)
    {
        if (strcmp(command, external_commands[i]) == 0)
            return EXTERNAL;
    }

    return NO_COMMAND;
}

/*------------------------------------------------------------*/
/* Check if pipe '|' is present in command                    */
/*------------------------------------------------------------*/
int is_pipe_present(char *cmd)
{
    return strchr(cmd, '|') != NULL;
}

/*------------------------------------------------------------*/
/* Execute external commands (with pipe support)              */
/*------------------------------------------------------------*/
void execute_external_commands(char *input_string)
{
    char *commands[20];     // Store individual pipe commands
    int pipe_count = 0;

    char buffer[1024];
    strncpy(buffer, input_string, 1024);
    buffer[1023] = '\0';

    /* Split input by '|' */
    char *tok = strtok(buffer, "|");
    while (tok != NULL && pipe_count < 20)
    {
        while (*tok == ' ')   // Skip leading spaces
            tok++;

        commands[pipe_count++] = tok;
        tok = strtok(NULL, "|");
    }

    /* Create required pipes */
    int pipes[pipe_count - 1][2];
    for (int i = 0; i < pipe_count - 1; i++)
    {
        if (pipe(pipes[i]) < 0)
        {
            perror("pipe");
            exit(1);
        }
    }

    /* Create child processes */
    for (int i = 0; i < pipe_count; i++)
    {
        pid = fork();

        if (pid == 0)   // Child process
        {
            /* Read from previous pipe */
            if (i > 0)
                dup2(pipes[i - 1][0], 0);

            /* Write to next pipe */
            if (i < pipe_count - 1)
                dup2(pipes[i][1], 1);

            /* Close all pipe fds */
            for (int j = 0; j < pipe_count - 1; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            /* Prepare arguments */
            char *argv[64];
            int argc = 0;

            char *arg = strtok(commands[i], " ");
            while (arg != NULL && argc < 63)
            {
                argv[argc++] = arg;
                arg = strtok(NULL, " ");
            }
            argv[argc] = NULL;

            /* Execute command */
            execvp(argv[0], argv);
            perror("execvp failed");
            exit(1);
        }
    }

    /* Parent closes pipes */
    for (int i = 0; i < pipe_count - 1; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /* Wait for all children */
    for (int i = 0; i < pipe_count; i++)
        wait(NULL);
}

/*------------------------------------------------------------*/
/* Execute internal (built-in) shell commands                 */
/*------------------------------------------------------------*/
int execute_internal_commands(char *input_string)
{
    char buffer[100];

    /* pwd command */
    if (strcmp("pwd", input_string) == 0)
    {
        getcwd(buffer, sizeof(buffer));
        printf("%s\n", buffer);
        return 0;
    }

    /* exit command */
    else if (strcmp("exit", input_string) == 0)
    {
        exit(0);
    }

    /* cd <path> */
    else if (strncmp(input_string, "cd ", 3) == 0)
    {
        if (chdir(input_string + 3) != 0)
        {
            perror("cd");
            return 1;
        }

        getcwd(buffer, sizeof(buffer));
        printf("%s\n", buffer);
        return 0;
    }

    /* cd (no arguments) */
    else if (strcmp("cd", input_string) == 0)
    {
        getcwd(buffer, sizeof(buffer));
        printf("%s\n", buffer);
        return 0;
    }

    /* echo $? */
    else if (strcmp("echo $?", input_string) == 0)
    {
        if (WIFEXITED(last_status))
            printf("%d\n", WEXITSTATUS(last_status));
        else
            printf("Abnormal termination\n");
        return 0;
    }

    /* echo $$ */
    else if (strcmp("echo $$", input_string) == 0)
    {
        printf("current pid is : %d\n", getpid());
        return 0;
    }

    /* echo $SHELL */
    else if (strcmp("echo $SHELL", input_string) == 0)
    {
        char *shell = getenv("SHELL");
        if (shell)
            printf("%s\n", shell);
        else
            printf("SHELL not set\n");
        return 0;
    }

    /* jobs command */
    else if (strcmp("jobs", input_string) == 0)
    {
        jobs_t *temp = head;
        int job_id = 1;

        while (temp)
        {
            printf("[%d] Stopped %d %s\n",
                   job_id++, temp->pid, temp->command);
            temp = temp->link;
        }
        return 0;
    }

    /* fg command */
    else if (strcmp("fg", input_string) == 0)
    {
        if (head == NULL)
        {
            printf("fg: no jobs\n");
            return 1;
        }

        jobs_t *job = head;
        head = head->link;

        pid = job->pid;
        printf("%s\n", job->command);

        kill(pid, SIGCONT);
        int status;
        waitpid(pid, &status, WUNTRACED);

        free(job);
        return 0;
    }

    /* bg command */
    else if (strcmp("bg", input_string) == 0)
    {
        if (head == NULL)
        {
            printf("bg: no jobs\n");
            return 1;
        }

        jobs_t *job = head;
        head = head->link;

        kill(-job->pid, SIGCONT);
        printf("[%d] %s &\n", job->pid, job->command);

        free(job);
        return 0;
    }
}
