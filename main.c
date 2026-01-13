/* 
 Name: Amulya M
 Description: MiniShell Project
*/

#include <stdio.h>
#include <stdlib.h>   // for system()
#include "main.h"

/* Default shell prompt */
char prompt[] = "minishell$";

/* Array holding all external commands read from file */
extern char *external_commands[155];

int main()
{
    /* 
     * Load all external Linux commands from external.txt
     * These commands are used to identify external commands
     */
    extract_external_commands(external_commands);

    /* Clear terminal screen */
    system("clear");

    /* Buffer to store user input command */
    char input_string[1024];

    /* 
     * Start the shell loop
     * - Displays prompt
     * - Reads user input
     * - Executes builtin or external commands
     * - Handles signals and job control
     */
    scan_input(prompt, input_string);

    return 0;  // Program never actually reaches here
}
