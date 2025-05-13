#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

extern char **environ;

#define DELIM " \n"

void print_prompt(void)
{
	printf("cisfun$ ");
	fflush(stdout);
}

char *read_command(void)
{
	char *cmd = NULL;
	size_t n = 0;
	if (getline(&cmd, &n, stdin) == -1)
		return (NULL);
	return (cmd);
}

int count_tokens(char *cmd)
{
	int count = 0;
	char *copy = strdup(cmd);
	char *token = strtok(copy, DELIM);

	while (token)
	{
		count++;
		token = strtok(NULL, DELIM);
	}
	free(copy);
	return (count);
}

char *resolve_path(char *cmd)
{
    char *path = getenv("PATH");
    char *path_copy = strdup(path);
    char *dir = strtok(path_copy, ":");
    char *full_path = NULL;

    if (strchr(cmd, '/')) {
        if (access(cmd, X_OK) == 0) {
            full_path = strdup(cmd);
            free(path_copy);
            return full_path;
        }
        free(path_copy);
        return NULL;
    }
    while (dir) {
        full_path = malloc(strlen(dir) + strlen(cmd) + 2);
        sprintf(full_path, "%s/%s", dir, cmd);
        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return full_path;
        }
        free(full_path);
        dir = strtok(NULL, ":");
    }
    free(path_copy);
    return NULL;
}

char **build_argv(char *cmd)
{
    char **argv = NULL;
    char *token;
    int i = 0;
    char *cmd_copy = strdup(cmd);

    int count = 0;
    token = strtok(cmd_copy, DELIM);
    while (token)
    {
        count++;
        token = strtok(NULL, DELIM);
    }
    free(cmd_copy);
    argv = malloc(sizeof(char *) * (count + 1));
    if (!argv)
        return NULL;
    cmd_copy = strdup(cmd);
    token = strtok(cmd_copy, DELIM);
    while (token)
    {
        argv[i++] = strdup(token);
        token = strtok(NULL, DELIM);
    }
    argv[i] = NULL;
    free(cmd_copy);
    return argv;
}

void execute_command(char **argv)
{
    char *full_path;
    int pid;
    int status;

    full_path = resolve_path(argv[0]);
    if (!full_path)
    {
        fprintf(stderr, "%s: command not found\n", argv[0]);
        return;
    }
    pid = fork();
    if (pid < 0) {
        perror("fork");
        free(full_path);
        return;
    }
    if (pid == 0) {
        if (execve(full_path, argv, environ) == -1) {
            perror("execve");
            exit(EXIT_FAILURE);
        }
    } else {
        wait(&status);
        free(full_path);
    }
}

int main(void)
{
    char *cmd;
    char **argv;
    int is_interactive = isatty(STDIN_FILENO);
    int	i;

    while (1)
    {
        if (is_interactive)
            print_prompt();
        cmd = read_command();
        if (!cmd)
            break;
        if (strcmp(cmd, "exit\n") == 0) {
            free(cmd);
            break;
        }
        argv = build_argv(cmd);
        if (!argv) {
            free(cmd);
            continue;
        }
        execute_command(argv);
	i = 0;
	while (argv[i])
	{
		free(argv[i]);
		i++;
	}
        free(argv);
        free(cmd);
    }
    return 0;
}
