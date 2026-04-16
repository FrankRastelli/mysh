#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MYSH_RL_BUFSIZE 1024
#define MYSH_TOK_BUFSIZE 64
#define MYSH_TOK_DELIM " \t\r\n\a"

/*
Function Declarations for builtin shell commands:
*/
int mysh_cd(char **args);
int mysh_help(char **args);
int mysh_exit(char **args);

/*
List of builtin commands, followed by their corresponding functions
*/

char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int mysh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

/*
Builtin function implementations
*/
int mysh_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "mysh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("mysh");
        }
    }
    return 1;
}

int mysh_help(char **args)
{
    int i;
    printf("Frank Rastelli's MYSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < mysh_num_builtins(); i++) {
        printf(" %s\n", builtin_str[i]);
    }
    printf("Use the man command for information on other programs.\n");
    return 1;
}

int mysh_exit(char **args)
{
    return 0;
}

int mysh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if  (execvp(args[0], args) == -1) {
            perror("mysh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("mysh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

char *mysh_read_line(void)
{
    int bufsize = MYSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        // If we hit EOF, replace it with a null character and return
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate
        if (position >= bufsize) {
            bufsize += MYSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **mysh_split_line(char *line)
{
    int bufsize = MYSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "mysh: allocations error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, MYSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        
        if (position >= bufsize) {
            bufsize += MYSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize *sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, MYSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

void mysh_loop(void) 
{
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = mysh_read_line();
        args = mysh_split_line(line);
        status = mysh_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv) 
{
    // Load config files

    // Run command loop
    mysh_loop();

    return EXIT_SUCCESS;
}