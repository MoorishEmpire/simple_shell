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
	char *copy = strdup(cmd);
	char *token = strtok(copy, DELIM);
	int count = token ? 1 : 0;

	free(copy);
	return (count);
}

char **build_argv(char *cmd, int argc)
{
	char **argv;
	char *token;
	int i;

	argv = malloc(sizeof(char *) * 2);
	if (!argv)
		return (NULL);
	token = strtok(cmd, DELIM);
	argv[0] = token;
	argv[i] = NULL;
	return (argv);
}

void execute_command(char **argv)
{
	int pid = fork();
	int status;

	if (pid < 0)
	{
		perror("fork");
		return;
	}
	if (pid == 0)
	{
		if (execve(argv[0], argv, environ) == -1)
		{
			fprintf(stderr, "./shell: No such file or directory\n");
			exit(EXIT_FAILURE);
		}
	}
	else
		wait(&status);
}

int main(void)
{
	char *cmd;
	int argc;
	char **argv;

	while (1)
	{
		print_prompt();
		cmd = read_command();
		if (!cmd)
			break;
		if(strcmp(cmd, "exit\n") == 0)
		{
			free(cmd);
			break;
		}
		argc = count_tokens(cmd);
		if (argc == 0)
		{
			free(cmd);
			continue;
		}

		argv = build_argv(cmd, argc);
		if (!argv)
		{
			free(cmd);
			continue;
		}

		execute_command(argv);

		free(argv);
		free(cmd);
	}

	return (0);
}

