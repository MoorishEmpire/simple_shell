#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>

extern char **environ;

#define DELIM " \n"

void print_prompt(void)
{
    write(STDOUT_FILENO, "cisfun$ ", 8);
}

char *read_command(void)
{
    char *cmd = NULL;
    size_t n = 0;
    ssize_t len = getline(&cmd, &n, stdin);
    
    if (len == -1)
    {
        free(cmd);
        return NULL;
    }
    return cmd;
}

int count_tokens(char *cmd)
{
    int count = 0;
    int in_token = 0;
    
    while (*cmd)
    {
        if (*cmd == ' ' || *cmd == '\n')
        {
            if (in_token)
            {
                count++;
                in_token = 0;
            }
        }
        else
        {
            in_token = 1;
        }
        cmd++;
    }
    if (in_token)
        count++;
    return count;
}

int custom_strlen(char *str)
{
    int len = 0;
    while (str[len])
        len++;
    return len;
}

char *find_in_path(char *cmd, char **env)
{
    static char full_path[1024];
    char *path = NULL;
    char *dir;
    int len;
    int i;

    if (access(cmd, X_OK) == 0)
        return cmd;

    i = 0;
    while (env[i])
    {
        if (strncmp(env[i], "PATH=", (size_t)5) == 0)
        {
            path = env[i] + 5;
            break;
        }
	i++;
    }

    if (!path)
        return NULL;

    while (*path)
    {
        dir = path;
        while (*path && *path != ':')
            path++;
        len = path - dir;

        snprintf(full_path, sizeof(full_path), "%.*s/%s", len, dir, cmd);
        if (access(full_path, X_OK) == 0)
            return full_path;

        if (*path == ':')
            path++;
    }

    return NULL;
}

char **build_argv(char *cmd)
{
    int count = count_tokens(cmd);
    char **argv;
    int i, start, argc, len, j;
    int in_token;
    
    if (count == 0)
        return NULL;
    
    argv = malloc(sizeof(char *) * (count + 1));
    if (!argv)
        return NULL;
    
    i = 0;
    in_token = 0;
    start = 0;
    argc = 0;
    
    while (cmd[i])
    {
        if (cmd[i] == ' ' || cmd[i] == '\n')
        {
            if (in_token)
            {
                len = i - start;
                argv[argc] = malloc(len + 1);
                if (!argv[argc])
                {
                    for (j = 0; j < argc; j++)
                        free(argv[j]);
                    free(argv);
                    return NULL;
                }
                for (j = 0; j < len; j++)
                    argv[argc][j] = cmd[start + j];
                argv[argc][len] = '\0';
                argc++;
                in_token = 0;
            }
        }
        else if (!in_token)
        {
            start = i;
            in_token = 1;
        }
        i++;
    }
    
    if (in_token)
    {
        len = i - start;
        argv[argc] = malloc(len + 1);
        if (!argv[argc])
        {
            for (j = 0; j < argc; j++)
                free(argv[j]);
            free(argv);
            return NULL;
        }
        for (j = 0; j < len; j++)
            argv[argc][j] = cmd[start + j];
        argv[argc][len] = '\0';
        argc++;
    }
    
    argv[argc] = NULL;
    return argv;
}

void execute_command(char **argv)
{
    char *full_path;
    pid_t pid;
    
    if (!argv || !argv[0])
        return;
    
    full_path = find_in_path(argv[0], environ);
    if (!full_path)
    {
        write(STDERR_FILENO, argv[0], custom_strlen(argv[0]));
        write(STDERR_FILENO, ": command not found\n", 20);
        return;
    }
    
    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return;
    }
    else if (pid == 0)
    {
        execve(full_path, argv, environ);
        perror("execve");
        exit(EXIT_FAILURE);
    }
    else
    {
        waitpid(pid, NULL, 0);
    }
}

int main(void)
{
    char *cmd;
    char **argv;
    int is_interactive;
    int exit_cmd;
    char *ptr;
    int i;
    
    is_interactive = isatty(STDIN_FILENO);
    
    while (1)
    {
        if (is_interactive)
            print_prompt();
        
        cmd = read_command();
        if (!cmd)
            break;
        exit_cmd = 1;
        ptr = cmd;
        while (*ptr == ' ')
            ptr++;
        if (!(ptr[0] == 'e' && ptr[1] == 'x' && ptr[2] == 'i' && ptr[3] == 't' && 
             (ptr[4] == '\n' || ptr[4] == ' ' || ptr[4] == '\0')))
            exit_cmd = 0;
        
        if (exit_cmd)
        {
            free(cmd);
            break;
        }
        
        argv = build_argv(cmd);
        free(cmd);
        
        if (argv)
        {
            execute_command(argv);
            i = 0;
            while (argv[i])
            {
                free(argv[i]);
                i++;
            }
            free(argv);
        }
    }
    
    return 0;
}
