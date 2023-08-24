#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define OK 0
#define KO 1

#define OUT 0
#define IN 1

#define PIPE 0
#define BREAK 1
#define END 2

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

void  free_list(cmd *list) {
	cmd *prev;

	for (cmd *p = list; p != NULL; p = prev) {
		prev = p->prev;
		free(p->argv);
		free(p);
	}
}

int   push(char *av, cmd **list) {
	printf("push command: %s\n", av);

	cmd *elem = malloc(sizeof(cmd));
	if (!elem)
		return KO;
	elem->type = END;
	elem->argc = 1;
	elem->argv = malloc(sizeof(char*));
	elem->argv[0] = av;
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
	printf("add arg: %s\n", av);

	char **new_argv = malloc(sizeof(char*) * (elem->argc + 1));
	if (!new_argv)
		return KO;
	for(int i = 0; i < elem->argc; i++) {
		new_argv[i] = elem->argv[i];
	}
	new_argv[elem->argc] = av;
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
		printf("\033[0;35m semicolon or pipe at the beginning\033[0m\n");
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
  int ret;

  if (elem->type == PIPE && dup2(elem->pipes[IN], STDOUT) < 0)
    return KO;
  if (elem->prev && elem->previous->type == PIPE && dup2(elem->pipes[OUT], STDIN) < 0)
    return KO;
  if ((ret = execve(elem->args[0], elem->args, env)) < 0) {
    // show errors
  }
  return ret;
}

int   parent(cmd *elem, int pid, int pipe_on) {
  int status;

  waitpid(pid, &status, 0);
  
  if (pipe_on) {
    close(elem->pipes[IN]);
    if (! elem->next || elem->type == BREAK)
      close(elem->pipes[OUT]);
  }
  if (elem->prev && elem->prev->type = PIPE)
    close(elem->prev->pipes[OUT]);
  if (WIFEXITED(status))
			ret = WEXITSTATUS(status);
  return ret;
}

int   exec(cmd *elem, char **env) {
  if (!elem || elem->argc < 1) {
    return OK;
  } else if (strcmp(elem->argv[0], "cd") == 0) {
    return (my_cd(elem));
  } else {
    
    int ret = KO;
    int pipe_on = 0;

    if (elem->type == PIPE || (elem->prev && elem->prev->type == PIPE)) {
        int pipe_on = 1;
        if (pipe(elem->pipes))
          return KO;
    }

    pid = fork();
    
    if (pid < 0) {
      return KO;
    } else if (pid == 0) {
      exit(child(elem, env));
    } else {
      parent(elem, pid, pipe_on);
    }
  }
  return ret;
}



int   main(int ac, char **av, char **env) {
	cmd *list = NULL;
	cmd *start_ptr = NULL;

	for (int i = 1; i < ac; i++) {
		if (parse_arg(av[i], &list) == KO) {
			free_list(list);
			return(KO);
		} else if (start_ptr == NULL) {
			start_ptr = list;
		}
	}
	for (cmd *p = start_ptr; p != NULL; p = p->next) {
		if (exec(p, env) == KO) {
			free_list(list);
			return(KO);
		}
	}
	free_list(list);
	return OK;
}
