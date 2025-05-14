#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stddef.h>

extern char **environ;
int exit_status = 0;
char *program;

#define DELIM " \n"
#define BUFFER_SIZE 1024

void	*_realloc(void *ptr, unsigned int old_size, unsigned int new_size)
{
	char	*new;
	unsigned int	i;
	unsigned int	size;

	if (ptr == NULL)
	{
		new = malloc(new_size);
		if (new == NULL)
			return (NULL);
		return ((void *)new);
	}

	if (new_size == 0)
	{
		free(ptr);
		return (NULL);
	}

	if (new_size == old_size)
	{
		return (ptr);
	}

	size = (new_size < old_size) ? new_size : old_size;

	new = malloc(new_size);
	if (new == NULL)
		return (NULL);

	i = 0;
	while (i < size)
	{
		new[i] = ((char *)ptr)[i];
		i++;
	}

	free(ptr);
	return ((void *)new);
}

void print_prompt(void)
{
    write(STDOUT_FILENO, "cisfun$ ", 8);
}

ssize_t _getline(char **lineptr, size_t *n, int fd)
{
    static char buffer[BUFFER_SIZE];
    static size_t buffer_pos = 0;
    static size_t buffer_size = 0;
    size_t line_pos = 0;
    ssize_t bytes_read;
    char *new_ptr;
    
    if (!lineptr || !n || fd < 0)
        return -1;
    
    if (*lineptr == NULL || *n == 0) {
        *n = 128;
        *lineptr = malloc(*n);
        if (*lineptr == NULL)
            return -1;
    }
    
    while (1) {
        if (buffer_pos >= buffer_size) {
            bytes_read = read(fd, buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                if (line_pos == 0)
                    return -1;
                (*lineptr)[line_pos] = '\0';
                return line_pos;
            }
            buffer_size = bytes_read;
            buffer_pos = 0;
        }
        
        if (line_pos + 1 >= *n) {
            *n *= 2;
            new_ptr = _realloc(*lineptr, *n / 2, *n);
            if (new_ptr == NULL)
                return -1;
            *lineptr = new_ptr;
        }
        
        (*lineptr)[line_pos] = buffer[buffer_pos];
        buffer_pos++;
        
        if ((*lineptr)[line_pos] == '\n') {
            (*lineptr)[line_pos + 1] = '\0';
            return line_pos + 1;
        }
        
        line_pos++;
    }
}

char *read_command(void)
{
    char *cmd = NULL;
    size_t n = 0;
    ssize_t len = _getline(&cmd, &n, STDIN_FILENO);
    
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

int custom_strcmp(char *s1, char *s2)
{
	while (*s1 && *s2 && *s1 == *s2)
	{
		s1++;
		s2++;
	}
	return (*s1 - *s2);
}

char *find_in_path(char *cmd, char **env)
{
    static char full_path[1024];
    char *path = NULL;
    char *dir;
    size_t i, j, len;

    if (cmd[0] == '/' || 
        (cmd[0] == '.' && (cmd[1] == '/' || 
                          (cmd[1] == '.' && cmd[2] == '/')))) {
        if (access(cmd, X_OK) == 0)
            return cmd;

        write(STDERR_FILENO, program, custom_strlen(program));
        write(STDERR_FILENO, cmd, custom_strlen(cmd));
        write(STDERR_FILENO, ": not found\n", 12);
        return NULL;
    }

    i = 0;
    while (env[i]) {
        if (env[i][0] == 'P' && env[i][1] == 'A' && 
            env[i][2] == 'T' && env[i][3] == 'H' && 
            env[i][4] == '=') {
            path = env[i] + 5;
            break;
        }
        i++;
    }

    if (!path || !*path) {
        write(STDERR_FILENO, "./hsh: 1: ", 10);
        write(STDERR_FILENO, cmd, custom_strlen(cmd));
        write(STDERR_FILENO, ": not found\n", 12);
        return NULL;
    }

    while (*path) {
        dir = path;
        while (*path && *path != ':') path++;
        len = path - dir;

        if (len == 0) {
            full_path[0] = '.';
            full_path[1] = '/';
            j = 2;
        } else {
            for (j = 0; j < len; j++)
                full_path[j] = dir[j];
            full_path[j++] = '/';
        }
        i = 0;
        while (cmd[i] && j < sizeof(full_path) - 1)
            full_path[j++] = cmd[i++];
        full_path[j] = '\0';

        if (access(full_path, X_OK) == 0)
            return full_path;

        if (*path == ':') path++;
    }

    write(STDERR_FILENO, "./hsh: 1: ", 10);
    write(STDERR_FILENO, cmd, custom_strlen(cmd));
    write(STDERR_FILENO, ": not found\n", 12);
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

void	execute_env(char **env)
{
	int i;

	i = 0;
	while (env[i])
	{
		write(STDOUT_FILENO, env[i], custom_strlen(env[i]));
		write(STDOUT_FILENO, "\n", 1);
		i++;
	}
}

int is_builtin(char *cmd)
{
	if (custom_strcmp(cmd, "env") == 0)
		return (1);
	return (0);
}

void execute_command(char **argv)
{
    char *full_path;
    int status;
    pid_t pid;
    
    if (!argv || !argv[0])
        return;

    if (is_builtin(argv[0]))
    {
	    if (custom_strcmp(argv[0], "env") == 0)
	    {
		    execute_env(environ);
		    exit_status = 0;
		    return ;
	    }
    }
    
    full_path = find_in_path(argv[0], environ);
    if (!full_path) {
        exit_status = 127;
        return;
    }
    
    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit_status = 1;
        return;
    }
    else if (pid == 0) {
        execve(full_path, argv, environ);
        perror("execve");
        exit(EXIT_FAILURE);
    }
    else {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            exit_status = WEXITSTATUS(status);
    }
}

int main(int ac, char **av)
{
    char *cmd;
    char **argv;
    int is_interactive;
    char *ptr;
    int i;
    
    (void)ac;
    program = av[0];
    is_interactive = isatty(STDIN_FILENO);
    
    while (1)
    {
        if (is_interactive)
            print_prompt();
        
        cmd = read_command();
        if (!cmd) {
            if (is_interactive)
                write(STDOUT_FILENO, "\n", 1);
            break;
        }
        if (cmd[0] == '\n') {
            free(cmd);
            continue;
        }
        ptr = cmd;
        while (*ptr == ' ')
            ptr++;
        if (ptr[0] == 'e' && ptr[1] == 'x' && ptr[2] == 'i' && ptr[3] == 't' && 
             (ptr[4] == '\n' || ptr[4] == ' ' || ptr[4] == '\0')) {
            free(cmd);
            break;
        }
        
        argv = build_argv(cmd);
        free(cmd);
        
        if (argv) {
            execute_command(argv);
            i = 0;
            while (argv[i]) {
                free(argv[i]);
                i++;
            }
            free(argv);
        }
    }
    return exit_status;
}
