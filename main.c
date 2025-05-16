#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

extern char	**environ;
int			exit_status = 0;
char		*program;

#define DELIM " \n"
#define BUFFER_SIZE 1024

char	*custom_strchr(const char *s, int c)
{
	while (*s)
	{
		if (*s == (char)c)
			return ((char *)s);
		s++;
	}
	if ((char)c == '\0')
		return ((char *)s);
	return (NULL);
}

int	custom_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	i;

	i = 0;
	while ((s1[i] || s2[i]) && i < n)
	{
		if (s1[i] != s2[i])
			return ((unsigned char)s1[i] - (unsigned char)s2[i]);
		i++;
	}
	return (0);
}

void custom_strcpy(char *dest, const char *src)
{
    while (*src)
        *dest++ = *src++;
    *dest = '\0';
}

void custom_strcat(char *dest, const char *src)
{
    while (*dest)
        dest++;
    while (*src)
        *dest++ = *src++;
    *dest = '\0';
}

long	_atol(char *str)
{
	long	nbr;
	int		signe;
	int		i;

	signe = 1;
	i = 0;
	while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
		i++;
	if (str[i] == '+' || str[i] == '-')
	{
		if (str[i] == '-')
			signe *= -1;
		i++;
	}
	nbr = 0;
	while (str[i] >= '0' && str[i] <= '9')
	{
		if (nbr > (LONG_MAX - (str[i] - '0')) / 10)
		{
			if (signe == 1)
				return (LONG_MAX);
			else
				return (LONG_MIN);
		}
		nbr = (nbr * 10) + (str[i] - '0');
		i++;
	}
	return (signe * nbr);
}

void	*_realloc(void *ptr, unsigned int old_size, unsigned int new_size)
{
	char			*new;
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

void	print_prompt(void)
{
	write(STDOUT_FILENO, "cisfun$ ", 8);
}

ssize_t	_getline(char **lineptr, size_t *n, int fd)
{
	static char		buffer[BUFFER_SIZE];
	static size_t	buffer_pos = 0;
	static size_t	buffer_size = 0;
	size_t			line_pos;
	ssize_t			bytes_read;
	char			*new_ptr;

	line_pos = 0;
	if (!lineptr || !n || fd < 0)
		return (-1);
	if (*lineptr == NULL || *n == 0)
	{
		*n = 128;
		*lineptr = malloc(*n);
		if (*lineptr == NULL)
			return (-1);
	}
	while (1)
	{
		if (buffer_pos >= buffer_size)
		{
			bytes_read = read(fd, buffer, BUFFER_SIZE);
			if (bytes_read <= 0)
			{
				if (line_pos == 0)
					return (-1);
				(*lineptr)[line_pos] = '\0';
				return (line_pos);
			}
			buffer_size = bytes_read;
			buffer_pos = 0;
		}
		if (line_pos + 1 >= *n)
		{
			*n *= 2;
			new_ptr = _realloc(*lineptr, *n / 2, *n);
			if (new_ptr == NULL)
				return (-1);
			*lineptr = new_ptr;
		}
		(*lineptr)[line_pos] = buffer[buffer_pos];
		buffer_pos++;
		if ((*lineptr)[line_pos] == '\n')
		{
			(*lineptr)[line_pos + 1] = '\0';
			return (line_pos + 1);
		}
		line_pos++;
	}
}

char	*read_command(void)
{
	char	*cmd;
	size_t	n;
	ssize_t	len;

	cmd = NULL;
	n = 0;
	len = _getline(&cmd, &n, STDIN_FILENO);
	if (len == -1)
	{
		free(cmd);
		return (NULL);
	}
	return (cmd);
}

int is_valid_number(char *str)
{
    int i;
    int has_digit = 0;

    if (!str || !*str)
        return (0);
    i = 0;
    if (str[i] == '-' || str[i] == '+')
        i++;
    while (str[i] && str[i] != ' ' && str[i] != '\n')
    {
        if (str[i] < '0' || str[i] > '9')
            return (0);
        has_digit = 1;
        i++;
    }
    return (has_digit);
}

int	count_tokens(char *cmd)
{
	int	count;
	int	in_token;

	count = 0;
	in_token = 0;
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
	return (count);
}

int	custom_strlen(const char *str)
{
	int	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}

int	custom_strcmp(const char *s1, const char *s2)
{
	while (*s1 && *s2 && *s1 == *s2)
	{
		s1++;
		s2++;
	}
	return (*s1 - *s2);
}

char	*find_in_path(char *cmd, char **env)
{
	static char	full_path[1024];
	char		*path;
	char		*dir;
	size_t i;
	size_t j;
	size_t len;

	path = NULL;
		if (cmd[0] == '/' || (cmd[0] == '.' && (cmd[1] == '/' || (cmd[1] == '.'
					&& cmd[2] == '/'))))
	{
		if (access(cmd, X_OK) == 0)
			return (cmd);
		write(STDERR_FILENO, program, custom_strlen(program));
		write(STDERR_FILENO, cmd, custom_strlen(cmd));
		write(STDERR_FILENO, ": not found\n", 12);
		return (NULL);
	}
	i = 0;
	while (env[i])
	{
		if (env[i][0] == 'P' && env[i][1] == 'A' && env[i][2] == 'T'
			&& env[i][3] == 'H' && env[i][4] == '=')
		{
			path = env[i] + 5;
			break ;
		}
		i++;
	}
	if (!path || !*path)
	{
		write(STDERR_FILENO, "./hsh: 1: ", 10);
		write(STDERR_FILENO, cmd, custom_strlen(cmd));
		write(STDERR_FILENO, ": not found\n", 12);
		return (NULL);
	}
	while (*path)
	{
		dir = path;
		while (*path && *path != ':')
			path++;
		len = path - dir;
		if (len == 0)
		{
			full_path[0] = '.';
			full_path[1] = '/';
			j = 2;
		}
		else
		{
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
		if (*path == ':')
			path++;
	}
	write(STDERR_FILENO, "./hsh: 1: ", 10);
	write(STDERR_FILENO, cmd, custom_strlen(cmd));
	write(STDERR_FILENO, ": not found\n", 12);
	return NULL;
}

char	**build_argv(char *cmd)
{
	int		count;
	char	**argv;
	int		in_token;
	int i;
	int start;
	int argc;
	int len;
	int j;

	count = count_tokens(cmd);
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
	int	i;

	i = 0;
	while (env[i])
	{
		write(STDOUT_FILENO, env[i], custom_strlen(env[i]));
		write(STDOUT_FILENO, "\n", 1);
		i++;
	}
	exit_status = 0;
}

char	*custom_strdup(const char *s1)
{
	char	*res;
	size_t	i;
	size_t	len;

	len = custom_strlen(s1);
	res = malloc(len + 1);
	if (!res)
		return (NULL);
	i = 0;
	while (i < len)
	{
		res[i] = s1[i];
		i++;
	}
	res[i] = '\0';
	return (res);
}

char *execute_getenv(const char *name)
{
	int i = 0;
	int name_len = custom_strlen(name);
	char *value;

	while (environ[i])
	{
		if (custom_strncmp(environ[i], name, name_len) == 0 && environ[i][name_len] == '=')
		{
			value = custom_strdup(environ[i] + name_len + 1);
			if (!value)
				return (NULL);
			return (value);
		}
		i++;
	}
	return (NULL);
}

int execute_setenv(const char *name, const char *value, int overwrite)
{
    int i = 0, name_len, env_size = 0;
    char *new_var;
    char **new_environ, **old_environ;

    if (!name || name[0] == '\0' || custom_strchr(name, '='))
    {
        write(STDERR_FILENO, "./hsh: 1: setenv: Invalid variable name\n", 39);
        exit_status = 1;
        return (-1);
    }

    name_len = custom_strlen(name);
    while (environ[env_size])
        env_size++;

    while (environ[i])
    {
        if (custom_strncmp(environ[i], name, name_len) == 0 && environ[i][name_len] == '=')
        {
            if (!overwrite)
            {
                exit_status = 0;
                return (0);
            }
            new_var = malloc(name_len + custom_strlen(value) + 2);
            if (!new_var)
            {
                exit_status = 1;
                return (-1);
            }
            custom_strcpy(new_var, name);
            custom_strcat(new_var, "=");
            custom_strcat(new_var, value);
            free(environ[i]);
            environ[i] = new_var;
            exit_status = 0;
            return (0);
        }
        i++;
    }

    new_environ = malloc(sizeof(char *) * (env_size + 2));
    if (!new_environ)
    {
        exit_status = 1;
        return (-1);
    }
    for (i = 0; i < env_size; i++)
        new_environ[i] = environ[i];
    new_var = malloc(name_len + custom_strlen(value) + 2);
    if (!new_var)
    {
        free(new_environ);
        exit_status = 1;
        return (-1);
    }
    custom_strcpy(new_var, name);
    custom_strcat(new_var, "=");
    custom_strcat(new_var, value);
    new_environ[env_size] = new_var;
    new_environ[env_size + 1] = NULL;

    old_environ = environ;
    environ = new_environ;
    free(old_environ);
    exit_status = 0;
    return (0);
}

int execute_unsetenv(const char *name)
{
    int i = 0, j = 0, name_len, env_size = 0, found = 0;
    char **new_environ, **old_environ;

    if (!name || name[0] == '\0' || custom_strchr(name, '='))
    {
        write(STDERR_FILENO, "./hsh: 1: unsetenv: Invalid variable name\n", 41);
        exit_status = 1;
        return (-1);
    }

    name_len = custom_strlen(name);
    while (environ[env_size])
        env_size++;

    new_environ = malloc(sizeof(char *) * (env_size + 1));
    if (!new_environ)
    {
        exit_status = 1;
        return (-1);
    }

    while (environ[i])
    {
        if (custom_strncmp(environ[i], name, name_len) == 0 && environ[i][name_len] == '=')
        {
            free(environ[i]);
            found = 1;
        }
        else
        {
            new_environ[j] = environ[i];
            j++;
        }
        i++;
    }
    new_environ[j] = NULL;

    if (!found)
    {
        free(new_environ);
        exit_status = 0;
        return (0);
    }

    old_environ = environ;
    environ = new_environ;
    free(old_environ);
    exit_status = 0;
    return (0);
}

int execute_cd(const char *path)
{
	char *target_path = NULL;
	char *old_pwd = NULL;
	char *new_pwd = NULL;
	int result;
	int free_target_path = 0;

	if (!path || path[0] == '\0')
	{
		target_path = execute_getenv("HOME");
		if (!target_path)
		{
			exit_status = 1;
			return (-1);
		}
		free_target_path = 1;
	}
	else if (path[0] == '-' && path[1] == '\0')
	{
		target_path = execute_getenv("OLDPWD");
		if (!target_path)
		{
			target_path = getcwd(NULL, 0);
			if (!target_path)
			{
				write(STDERR_FILENO, "./hsh: 1: cd: getcwd failed\n", 28);
				exit_status = 1;
				return (-1);
			}
			write(STDOUT_FILENO, target_path, custom_strlen(target_path));
			write(STDOUT_FILENO, "\n", 1);
			free(target_path);
			exit_status = 0;
			return (0);
		}
		write(STDOUT_FILENO, target_path, custom_strlen(target_path));
		write(STDOUT_FILENO, "\n", 1);
		free_target_path = 1;
	}
	else
		target_path = (char *)path;
	old_pwd = getcwd(NULL, 0);
	if (!old_pwd)
	{
		write(STDERR_FILENO, "./hsh: 1: cd: getcwd failed\n", 28);
		if (free_target_path)
			free(target_path);
		exit_status = 1;
		return (-1);
	}
	result = chdir(target_path);
	if (result == -1)
	{
		write(STDERR_FILENO, "./hsh: 1: cd: can't cd to", 26);
		write(STDERR_FILENO, target_path, custom_strlen(target_path));
		write(STDERR_FILENO, "\n", 1);
		free(old_pwd);
		if (free_target_path)
			free(target_path);
		exit_status = 1;
		return (-1);
	}
	new_pwd = getcwd(NULL, 0);
	if (!new_pwd)
	{
		write(STDERR_FILENO, "./hsh: 1: cd: getcwd failed\n", 28);
		free(old_pwd);	
		if (free_target_path)
			free(target_path);
		exit_status = 1;
		return (-1);
	}
	if (execute_setenv("PWD", new_pwd, 1) == -1)
    {
        write(STDERR_FILENO, "./hsh: 1: cd: setenv failed\n", 28);
        free(old_pwd);
        free(new_pwd);
        if (free_target_path)
            free(target_path);
        exit_status = 1;
        return (-1);
    }
    if (execute_setenv("OLDPWD", old_pwd, 1) == -1)
    {
        write(STDERR_FILENO, "./hsh: 1: cd: setenv failed\n", 28);
        free(old_pwd);
        free(new_pwd);
        if (free_target_path)
            free(target_path);
        exit_status = 1;
        return (-1);
    }
	free(old_pwd);
	free(new_pwd);
	if (free_target_path)
		free(target_path);
	exit_status = 0;
	return (0);
}

int	is_builtin(char *cmd)
{
	if (custom_strcmp(cmd, "env") == 0)
		return (1);
	else if (custom_strcmp(cmd, "setenv") == 0)
		return (1);
	else if (custom_strcmp(cmd, "unsetenv") == 0)
		return (1);
	else if (custom_strcmp(cmd, "cd") == 0)
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
        }
        else if (custom_strcmp(argv[0], "setenv") == 0)
        {
            if (!argv[1] || !argv[2])
            {
                write(STDERR_FILENO, "./hsh: setenv: Too few arguments\n", 33);
                exit_status = 1;
            }
            else
            {
                if (execute_setenv(argv[1], argv[2], 1) == -1)
                    exit_status = 1;
                else
                    exit_status = 0;
            }
        }
        else if (custom_strcmp(argv[0], "unsetenv") == 0)
        {
            if (!argv[1])
            {
                write(STDERR_FILENO, "./hsh: unsetenv: Too few arguments\n", 35);
                exit_status = 1;
            }
            else
            {
                if (execute_unsetenv(argv[1]) == -1)
                    exit_status = 1;
                else
                    exit_status = 0;
            }
        }
	else if (custom_strcmp(argv[0], "cd") == 0)
	{
		if (execute_cd(argv[1]) == -1)
			exit_status = 1;
		else
			exit_status = 0;
	}
        return;
    }

    full_path = find_in_path(argv[0], environ);
    if (!full_path)
    {
        exit_status = 127;
        return;
    }
    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit_status = 1;
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
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
            exit_status = WEXITSTATUS(status);
    }
}

char **copy_environ(void)
{
    int env_size = 0;
    char **new_environ;
    int i;

    while (environ[env_size])
        env_size++;
    new_environ = malloc(sizeof(char *) * (env_size + 1));
    if (!new_environ)
        return NULL;
    for (i = 0; i < env_size; i++)
    {
        new_environ[i] = malloc(custom_strlen(environ[i]) + 1);
        if (!new_environ[i])
        {
            while (--i >= 0)
                free(new_environ[i]);
            free(new_environ);
            return NULL;
        }
        custom_strcpy(new_environ[i], environ[i]);
    }
    new_environ[env_size] = NULL;
    return new_environ;
}

int main(int ac, char **av)
{
    char *cmd;
    char **argv;
    int is_interactive;
    char *ptr;
    int i;
    char *num_str;
    long num;

    (void)ac;
    program = av[0];
    environ = copy_environ();
    if (!environ)
    {
        write(STDERR_FILENO, "Failed to initialize environment\n", 32);
        exit(1);
    }
    is_interactive = isatty(STDIN_FILENO);
    while (1)
    {
        if (is_interactive)
            print_prompt();
        cmd = read_command();
        if (!cmd)
        {
            if (is_interactive)
                write(STDOUT_FILENO, "\n", 1);
            break;
        }
        if (cmd[0] == '\n')
        {
            free(cmd);
            continue;
        }
        ptr = cmd;
        while (*ptr == ' ')
            ptr++;
        if (ptr[0] == 'e' && ptr[1] == 'x' && ptr[2] == 'i' && ptr[3] == 't'
                && (ptr[4] == '\n' || ptr[4] == ' ' || ptr[4] == '\0'))
        {
            if (ptr[4] == ' ')
            {
                num_str = ptr + 5;
                while (*num_str == ' ')
                    num_str++;
                if (!is_valid_number(num_str))
                {
                    write(STDERR_FILENO, "./hsh: 1: exit: Illegal number: ", 32);
                    while (*num_str && *num_str != ' ' && *num_str != '\n')
                    {
                        write(STDERR_FILENO, num_str, 1);
                        num_str++;
                    }
                    write(STDERR_FILENO, "\n", 1);
                    exit_status = 2;
                }
                else
                {
                    num = _atol(num_str);
                    if (num < 0)
                    {
                        write(STDERR_FILENO, "./hsh: 1: exit: Illegal number: ",
                            32);
                        write(STDERR_FILENO, num_str, custom_strlen(num_str));
                        exit_status = 2;
                    }
                    else
                        exit_status = (int)(num % 256);
                }
            }
            free(cmd);
            i = 0;
            while (environ[i])
                free(environ[i++]);
            free(environ);
            exit(exit_status);
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
    i = 0;
    while (environ[i])
        free(environ[i++]);
    free(environ);
    return (exit_status);
}
