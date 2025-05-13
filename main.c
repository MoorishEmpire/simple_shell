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

char **build_argv(char *cmd, int argc)
{
	char **argv = malloc(sizeof(char *) * (argc + 1));
	char *token = strtok(cmd, DELIM);
	int i;

	if (!argv)
		return (NULL);

	for (i = 0; i < argc; i++)
	{
		argv[i] = token;
		token = strtok(NULL, DELIM);
	}
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
			perror("execve");
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

