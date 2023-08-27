#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define OK 0
#define KO 1

#define OUT 0
#define IN 1

#define END 0
#define PIPE 1
#define BREAK 2

typedef struct s_list_elem {
	int     type;
	int     pipes[2];
	int     argc;
	char    **argv;
	struct  s_list_elem *prev;
	struct  s_list_elem *next;
}             cmd;

int   ft_strlen(char *str) {
	for (int i = 0; 1; i++)
		if (!str[i])
			return i;
}

void print_err(char *str) {
	write(STDERR_FILENO, str, ft_strlen(str)); /* dup2 fail */
}

void  free_list(cmd *list) {
	cmd *prev;

	for (cmd *p = list; p != NULL; p = prev) {
		prev = p->prev;
		free(p->argv);
		free(p);
	}
}

int   push(char *av, cmd **list) {
	cmd *elem = malloc(sizeof(cmd));
	if (!elem)
		return KO;
	elem->type = END;
	elem->argc = 1;
	elem->argv = malloc(sizeof(char*) * 2);
	if (!elem->argv)
		return KO;
	elem->argv[0] = av;
	elem->argv[1] = NULL;
	elem->next = NULL;
	if (!*list) {
		elem->prev = NULL;
	} else {
		(*list)->next = elem;
		elem->prev = *list;
	}
	*list = elem;
	return OK;
}

int   add_arg(char *av, cmd *elem) {
	char **new_argv = malloc(sizeof(char*) * (elem->argc + 2));
	if (!new_argv)
		return KO;
	for(int i = 0; i < elem->argc; i++) {
		new_argv[i] = elem->argv[i];
	}
	new_argv[elem->argc] = av;
	new_argv[elem->argc + 1] = NULL;
	free(elem->argv);
	elem->argv = new_argv;
	elem->argc++;
	return OK;
}

int   parse_arg(char *av, cmd **list) {
	int what = -1;

	if (strcmp(av, ";") == 0)
		what = BREAK;
	if (strcmp(av, "|") == 0)
		what = PIPE;

	if ((what == BREAK || what == PIPE) && !*list) {
		return OK;
	} else if (what != BREAK && what != PIPE && (!*list || (*list)->type != END)) {
		return push(av, list);
	} else if (what == BREAK || what == PIPE) {
		(*list)->type = what;
	} else {
		return add_arg(av, *list);
	}
	return OK;
}

int   child(cmd *elem, char **env) {
	//	reassign pipe's in to stdout (if output should go to pipe)
	if (elem->type == PIPE && dup2(elem->pipes[IN], STDOUT_FILENO) < 0) {
		print_err("error: fatal\n"); /* dup2 fail */
		exit(KO);
	}
	//	reassign pipe's out to stdin (if input should come from the pipe)
	if (elem->prev && elem->prev->type == PIPE && dup2(elem->prev->pipes[OUT], STDIN_FILENO) < 0) {
		print_err("error: fatal\n"); /* dup2 fail */
		exit(KO);
	}
	int ret = execve(elem->argv[0], elem->argv, env);
	print_err("error: cannot execute "); /* execve fail */
	print_err(elem->argv[0]);
	print_err("\n");
	exit(ret);
}

int   parent(cmd *elem, pid_t pid, int pipe_on) {
	int status;
	int ret = KO;

	waitpid(pid, &status, 0);

	if (pipe_on) {
		close(elem->pipes[IN]);
		if (! elem->next || elem->type == BREAK)
			close(elem->pipes[OUT]);
	}
	if (elem->prev && elem->prev->type == PIPE)
		close(elem->prev->pipes[OUT]);
	if (WIFEXITED(status))
		ret = WEXITSTATUS(status);
	return ret;
}

int	my_cd(cmd *elem) {
    if (elem->argc != 2 || !elem->argv[1]) {
        print_err("error: cd: bad arguments\n");
        return KO;
    } else if (chdir(elem->argv[1]) == -1) {
        print_err("error: cd: cannot change directory to ");
        print_err(elem->argv[1]);
        print_err("\n");
    }
	return OK;
}

int   exec(cmd *elem, char **env) {
	if (!elem || elem->argc < 1) {
		return OK;
	} else if (strcmp(elem->argv[0], "cd") == 0) {
		return (my_cd(elem));
	} else {
		int pipe_on = 0;

		if (elem->type == PIPE || (elem->prev && elem->prev->type == PIPE)) {
			pipe_on = 1;
			if (pipe(elem->pipes)) {
				print_err("error: fatal\n"); /* pipe fail */
				return KO;
			}
		}

		pid_t pid = fork();

		if (pid < 0) {
			print_err("error: fatal\n"); /* fork fail */
			return KO;
		} else if (pid == 0) {
			return child(elem, env);
		} else {
			return parent(elem, pid, pipe_on);
		}
	}
}

int   main(int ac, char **av, char **env) {
	cmd *list = NULL;
	cmd *start_ptr = NULL;
    int ret = OK;

	for (int i = 1; i < ac; i++) {
		if (parse_arg(av[i], &list) == KO) {
			print_err("error: fatal\n"); /* malloc fail */
			free_list(list);
			return(KO);
		} else if (start_ptr == NULL) {
			start_ptr = list;
		}
	}
	for (cmd *p = start_ptr; p != NULL; p = p->next) {
		ret = exec(p, env);
	}
	free_list(list);

	return ret;
}
