#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

//Mark
//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
#include <sys/wait.h>
//#include <fcntl.h>
//#include <string.h>
#include <fnmatch.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64

// Mark

void interactiveMode(const char *filename){        //Interactivemode is when the programming is using the terminal and the commands the user inputs. 

    int fd = open(filename, O_RDONLY);
    int isattyResult = isatty(fd);

    if(isattyResult == 1){
        printf("Welcome to my shell!\n");
    }

}

void batchMode(const char *filename){               //Batchmode is when there is 1 argument in the command and that is the file that will be read. 

    int fd = open(filename, O_RDONLY);
    int isattyResult = isatty(fd);

    



}







int main(int argc, char* argv[]){

const char* filename = argv[1];

int fd = open(filename, O_RDONLY);


    if(fd == -1){
        perror("Error opening file");
        EXIT_FAILURE;

    }


    if(argc > 2){
        printf("Failure, too many arguments\n");
        EXIT_FAILURE;
    }
    else if(argc == 1){
        interactiveMode(filename);
    }
    else if(argc == 2){
        batchMode(filename);
    }

return EXIT_SUCCESS;

}

// Mark

void welcome_message() {
    printf("Welcome to my shell!\n");
}

void goodbye_message() {
    printf("Exiting my shell.\n");
}

void prompt() {
    printf("mysh> ");
}

int execute_command(char *args[], char *redirect_input, char *redirect_output) {
    int pid, status;

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        if (redirect_input) {
            int fd_in = open(redirect_input, O_RDONLY);
            if (fd_in < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (redirect_output) {
            int fd_out = open(redirect_output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
}

void process_command(char *command) {
    char *token;
    char *args[MAX_ARGS];
    char *redirect_input = NULL;
    char *redirect_output = NULL;
    int is_pipeline = 0;

    token = strtok(command, " \n");
    int i = 0;
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \n");
            redirect_input = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \n");
            redirect_output = token;
        } else if (strcmp(token, "|") == 0) {
            is_pipeline = 1;
            break;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " \n");
    }
    args[i] = NULL;

    if (is_pipeline) {
        int pipe_fd[2];
        pipe(pipe_fd);

        char *args2[MAX_ARGS];
        int j = 0;
        token = strtok(NULL, " \n");
        while (token != NULL) {
            args2[j++] = token;
            token = strtok(NULL, " \n");
        }
        args2[j] = NULL;

        int pid1 = fork();
        if (pid1 < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid1 == 0) {
            // Child process 1
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]);
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        int pid2 = fork();
        if (pid2 < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid2 == 0) {
            // Child process 2
            close(pipe_fd[1]);
            dup2(pipe_fd[0], STDIN_FILENO);
            close(pipe_fd[0]);
            execvp(args2[0], args2);
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        close(pipe_fd[0]);
        close(pipe_fd[1]);
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    } else {
        execute_command(args, redirect_input, redirect_output);
    }
}

void interactive_mode() {
    welcome_message();
    char command[MAX_COMMAND_LENGTH];
    while (1) {
        prompt();
        if (fgets(command, sizeof(command), stdin) == NULL) {
            // End of input stream
            break;
        }
        if (strcmp(command, "exit\n") == 0) {
            break;
        }
        process_command(command);
    }
    goodbye_message();
}

void batch_mode(char *filename) {
    FILE *batch_file = fopen(filename, "r");
    if (batch_file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    char line[MAX_COMMAND_LENGTH];
    while (fgets(line, sizeof(line), batch_file) != NULL) {
        process_command(line);
    }

    fclose(batch_file);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        // Batch mode
        batch_mode(argv[1]);
    } else {
        // Interactive mode
        interactive_mode();
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// Mark
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
// #include <fcntl.h>
// #include <string.h>
// #include <fnmatch.h>

#define MAX_COMMAND_LENGTH 1024
// #define MAX_ARGS 64
#define DELIM " \t\r\n\a"

// Functions
void print_prompt(void);
char *read_command(void);
char **split_command(char *line) int execute_command(char **args);
int handle_redirection(char **args);
int execute_pipeline(char **args);

int main(int argc, char *argv[])
{
    char *line = NULL;
    size_t = 0;
    ssize_t nread;
    char **args;
    int status = 1;
    FILE *stream = stdin;

    // beginning of batch mode
    if (argc == 2)
    {
        queue = fopen(argv[1], "r");
        if (!queue)
        {
            fprintf(stderr, "Error opening file %s\n", argv[1]);
            return EXIT_FAILURE;
        }
    }
    else if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [script file]\n", argv[0]);
        return EXIT_FAILURE;
    }
    // beginning of interactive mode
    do
    {
        if (queue == stdin)
        {
            printf("Welcome to my Shell \n");
            print_prompt(); // Only print prompt in interactive mode
        }
        nread = getline(&line, &len, queue);
        if (nread == -1)
        {
            break; // end of file or read error
        }

        // removes new-line character from getline
        line[strcspn(line, "\n")] = 0;

        args = split_command(line);
        status = execute_command(args);

        free(line);
        line = NULL;
        free(args);
    } while (status);

    // close the script file if it was opened
    if (queue != stdin)
    {
        fclose(stream);
    }
    return EXIT_SUCCESS;
}

void print_prompt(void)
{
    printf("mysh> ");
    fflush(stdout);
}

char *read_command(void)
{
    char *line = NULL;
    size_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

char **split_command(char *line)
{
    int bufsize = MAX_LENGTH, position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;

    if (!tokens)
    {
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIM);
    while (token != NULL)
    {
        tokens[position++] = token;
        if (position >= bufsize)
        {
            bufsize += MAX_LENGTH;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                fprintf(stderr, "mysh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int execute_command(char **args)
{
    if (args[0] == NULL)
    {
        return 1; // Empty command
    }

    // Check for built-in commands
    if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL)
        {
            fprintf(stderr, "Expected argument to \"cd\"\n");
        }
        else
        {
            if (chdir(args[1]) != 0)
            {
                char cwd[MAX_LENGTH];
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                {
                    fprintf(stderr, "%s: %s: No such file or directory\n", args[1], cwd);
                }
                else
                {
                    perror("getcwd");
                }
            }
        }
        return 1; // Continue execution
    }

    if (strcmp(args[0], "pwd") == 0)
    {
        char cwd[MAX_LENGTH];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
        }
        else
        {
            perror("pwd");
        }
        return 1;
    }

    if (strcmp(args[0], "exit") == 0)
    {
        printf("You may Exit now!!! \n");
        return 0;
    }

    // Handle I/O redirection and pipelines
    if (handle_redirection(args) == 0 || execute_pipeline(args) == 0)
    {
        return 1; // Redirection or pipeline handled
    }

    // External command execution
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            perror("mysh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("mysh");
    }
    else
    {
        // Parent process
        wait(NULL);
    }

    return 1;
}

int handle_redirection(char **args)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], ">") == 0)
        {
            args[i] = NULL; // Truncate args at '>'
            int fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
            if (fd == -1)
            {
                perror("open");
                return -1;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            return 0; // Handled output redirection
        }
        else if (strcmp(args[i], "<") == 0)
        {
            args[i] = NULL; // Truncate args at '<'
            int fd = open(args[i + 1], O_RDONLY);
            if (fd == -1)
            {
                perror("open");
                return -1;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            return 0; // Handled input redirection
        }
    }
    return -1; // No redirection found
}

int execute_pipeline(char **args)
{
    int pipefd[2], status;
    pid_t pid1, pid2;
    int split_pos = -1;

    // Find the position of '|'
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "|") == 0)
        {
            split_pos = i;
            break;
        }
    }

    if (split_pos == -1)
        return -1; // No pipeline found

    args[split_pos] = NULL; // Split the args at '|'
    char **cmd1 = args;
    char **cmd2 = &args[split_pos + 1];

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid1 = fork();
    if (pid1 == 0)
    {
        // First command
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(cmd1[0], cmd1);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    pid2 = fork();
    if (pid2 == 0)
    {
        // Second command
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        execvp(cmd2[0], cmd2);
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);
    return 0; // Pipeline executed
}
/** Mark

void interactiveMode(const char *filename)
{ // Interactivemode is when the programming is using the terminal and the commands the user inputs.

    int fd = open(filename, O_RDONLY);
    int isattyResult = isatty(fd);

    if (isattyResult == 1)
    {
        printf("Welcome to my shell!\n");
    }
}

void batchMode(const char *filename)
{ // Batchmode is when there is 1 argument in the command and that is the file that will be read.

    int fd = open(filename, O_RDONLY);
    int isattyResult = isatty(fd);
}

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
        interactiveMode(filename);
    }
    else if (argc == 2)
    {
        batchMode(filename);
    }

    return EXIT_SUCCESS;
}

// Mark

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
        if (fnmatch(pattern, entry->d_name, FNM_PATHNAME) == 0 && entry->d_name[0] != '.')
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
            perror("execvp");
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
            perror("execvp");
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
        if (fgets(command, sizeof(command), stdin) == NULL)
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
    while (fgets(line, sizeof(line), batch_file) != NULL)
    {
        process_command(line);
    }

    fclose(batch_file);
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        // Batch mode
        batch_mode(argv[1]);
    }
    else
    {
        // Interactive mode
        interactive_mode();
    }

    return 0;
}
*/