#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>


#include <sys/wait.h>
#include <sys/types.h>
#include <fnmatch.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64



int main(int argc, char *argv[])
{

    const char *filename = argv[1];

    int fd = open(filename, O_RDONLY);

    if (fd == -1)
    {
        perror("Error opening file");
        EXIT_FAILURE;
    }

    if (argc > 2)
    {
        printf("Failure, too many arguments\n");
        EXIT_FAILURE;
    }
    else if (argc == 1)
    {
    int isattyResult = isatty(fd);
        if(isattyResult == 1){
            interactive_mode(filename); // Interactivemode is when the programming is using the terminal and the commands the user inputs.
        }
        if(isattyResult == 0){
            batch_mode(filename); // Batchmode is when there is 1 argument in the command and that is the file that will be read.
        }
    }
    else if (argc == 2)
    {
        batch_mode(filename); // Batchmode is when there is 1 argument in the command and that is the file that will be read.
    }

    return EXIT_SUCCESS;
}

void welcome_message()
{
    printf("Welcome to my shell!\n");
}

void goodbye_message()
{
    printf("Exiting my shell.\n");
}

void prompt()
{
    printf("mysh> ");
}

int execute_command(char *args[], char *redirect_input, char *redirect_output)
{
    int pid, status;

    if ((pid = fork()) < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        // Child process
        if (redirect_input)
        {
            int fd_in = open(redirect_input, O_RDONLY);
            if (fd_in < 0)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (redirect_output)
        {
            int fd_out = open(redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0)
            {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {
        // Parent process
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}
// wildcards 2.2
void process_wildcard(char *pattern, char *args[], int *arg_count)
{
    DIR *dir;
    struct dirent *entry;
    dir = opendir(".");
    if (dir == NULL)
    {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (fnmatch(pattern, entry->d_name, FNM_PERIOD) == 0 && entry->d_name[0] != '.')
        {
            args[(*arg_count)++] = strdup(entry->d_name);
        }
    }

    closedir(dir);
}

void process_command(char *command)
{
    char *token;
    char *args[MAX_ARGS];
    char *redirect_input = NULL;
    char *redirect_output = NULL;
    int is_pipeline = 0;

    token = strtok(command, " \n");
    int i = 0;
    while (token != NULL)
    {
        if (strcmp(token, "<") == 0)
        {
            token = strtok(NULL, " \n");
            redirect_input = token;
        }
        else if (strcmp(token, ">") == 0)
        {
            token = strtok(NULL, " \n");
            redirect_output = token;
        }
        else if (strcmp(token, "|") == 0)
        {
            is_pipeline = 1;
            break;
        }
        else if (strchr(token, '*') != NULL || strchr(token, '?') != NULL)
            {
                //process wildcard
                int arg_count = i;
                process_wildcard(token, args, &arg_count);
                i = arg_count;
            }
            else
            {
                args[i++] = token;
        }
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;

    if (strcmp(args[0], "cd") == 0)
    {
        // Change directory command  "cd"
        if (i != 2)
        {
            fprintf(stderr, "cd: Wrong number of arguments\n");
            return;
        }
        if (chdir(args[1]) != 0)
        {
            perror("chdir");
        }
        return;
    }

    if (strcmp(args[0], "pwd") == 0)
    {
        // Print working directory command
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
        }
        else
        {
            perror("getcwd");
        }
        return;
    }

    if (strcmp(args[0], "which") == 0)
    {
        // Print command location command
        if (i != 2)
        {
            fprintf(stderr, "which: Wrong number of arguments\n");
            return;
        }
        // Ignore any existing commands in PATH
        printf("%s\n", args[1]);
        return;
    }

    if (strcmp(args[0], "exit") == 0)
    {
        // Exit command
        exit(EXIT_SUCCESS);
    }

    if (is_pipeline)
    {
        int pipe_fd[2];
        pipe(pipe_fd);

        char *args2[MAX_ARGS];
        int j = 0;
        token = strtok(NULL, " \n");
        while (token != NULL)
        {
            args2[j++] = token;
            token = strtok(NULL, " \n");
        }
        args2[j] = NULL;

        int pid1 = fork();
        if (pid1 < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid1 == 0)
        {
            // Child process 1
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]);
            execvp(args[0], args);
            perror("execv");
            exit(EXIT_FAILURE);
        }

        int pid2 = fork();
        if (pid2 < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid2 == 0)
        {
            // Child process 2
            close(pipe_fd[1]);
            dup2(pipe_fd[0], STDIN_FILENO);
            close(pipe_fd[0]);
            execvp(args2[0], args2);
            perror("execv");
            exit(EXIT_FAILURE);
        }

        close(pipe_fd[0]);
        close(pipe_fd[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    }
    else
    {
        execute_command(args, redirect_input, redirect_output);
    }
}

void interactive_mode()
{
    welcome_message();
    
    char command[MAX_COMMAND_LENGTH];
    while (1)
    {
        prompt();
        if (read(command, sizeof(command), stdin) == NULL)
        {
            // End of input stream
            break;
        }
        if (strcmp(command, "exit\n") == 0)
        {
            break;
        }
        process_command(command);
    }
    goodbye_message();
}

void batch_mode(char *filename)
{
    FILE *batch_file = fopen(filename, "r");
    if (batch_file == NULL)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char line[MAX_COMMAND_LENGTH];
    while (read(line, sizeof(line), batch_file) != NULL)
    {
        process_command(line);
    }

    fclose(batch_file);
}
