#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

extern char **environ;

int main(void)
{
	char *cmd = NULL, *cmd_copy, *token;
	int pid;
	char **argv;
	int argc;
	int i;
	int status;
	char *delim = " \n";
	size_t n = 0;

	while (1)
	{
		argc = 0;
		printf("cisfun$ ");
		fflush(stdout);
		if (getline(&cmd, &n, stdin) == -1)
			break;
		cmd_copy = strdup(cmd);
		if (!cmd_copy)
			continue;
		token = strtok(cmd_copy, delim);
		while (token)
		{
			argc++;
			token = strtok(NULL, delim);
		}
		free(cmd_copy);
		if (argc == 0)
			continue;
		argv = malloc(sizeof(char *) * (argc + 1));
		if (!argv)
			return (-1);
		token = strtok(cmd, delim);
		i = 0;
		while (i < argc)
		{
			argv[i] = token;
			token = strtok(NULL, delim);
			i++;
		}
		argv[i] = NULL;
		pid = fork();
		if (pid < 0)
		{
			perror("fork");
			free(argv);
			continue;
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
		free(argv);
	}
	free(cmd);
	return (0);
}
