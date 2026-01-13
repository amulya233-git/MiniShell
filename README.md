MiniShell

📌 Description

MiniShell is a command-line shell developed in C under the Advanced C Programming module. This project emulates basic Linux shell functionality, allowing users to execute built-in commands (cd, pwd, echo, jobs, fg, bg) as well as external commands with support for piping. It demonstrates process management, signal handling, and job control in a Linux environment.

The project was developed to showcase structured programming, process control, and signal handling in C.

⚙️ Features

Execute built-in commands like cd, pwd, echo, jobs, fg, and bg.

Execute external commands and support multiple pipes (|) between commands.

Foreground and background process management.

Handle signals: Ctrl+C (interrupt) and Ctrl+Z (stop).

Maintain a list of stopped jobs with job IDs.

Dynamic prompt customization (PS1=).

Command history support (last executed command stored).

🛠 Technologies Used

Language: C Programming

Modules Covered: Processes, Signals, File Handling, Pipes, Fork & Exec

Compiler/IDE: GCC / Linux Terminal

File Storage: Text file (external.txt) for storing external commands

📂 Project Structure

MiniShell/
├── main.c # Main shell program
├── external.txt # List of external commands
├── Makefile # Optional build instructions
└── README.md # Project documentation

▶️ Usage

Compile the program:

gcc main.c -o minishell


Run the shell:

./minishell


Supported commands:

Built-in: cd, pwd, echo, jobs, fg, bg, exit

External: Any Linux command available in external.txt

Piping: command1 | command2 | ...

Follow on-screen prompts to execute commands and manage processes.

📚 Learning Outcomes

Understanding of Linux process management (fork, exec, wait).

Signal handling for interactive shell control (SIGINT, SIGTSTP, SIGCHLD).

Implementation of foreground and background job control.

Parsing and executing commands with pipes.

Designing a custom shell prompt and command history.

🚧 Limitations

No advanced shell features like file redirection (>, <) or wildcards.

Command parsing is basic; supports up to 20 piped commands.

External commands list must be maintained in external.txt.

👨‍💻 Author

Amulya M

📜 License

Open-source and available for educational purposes
