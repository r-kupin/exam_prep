#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>

#define OK 0
#define KO 1

#define OUT 0
#define IN 1

#define PIPE 0
#define BREAK 1
#define END 2

typedef struct  element {
    char            **av;
    int             ac;
    int             type;
    int             pipes[2];
    struct element  *prv;
    struct element  *nxt;
}               cmd;

int     ft_strlen(char *str) {
    for (int i = 0; 1; i++)
        if (!str[i])
            return i;
}

void    p_err(char *str) {
    write(STDERR_FILENO, str, ft_strlen(str));
}

void    free_list(cmd *list) {
    for (cmd *prv = NULL; list; list = prv) {
        prv = list->prv;
        free(list->av);
        free(list);
    }
}


int     push(char *arg, cmd **list) {
    printf("push %s\n", arg);
    cmd *new_elem = malloc(sizeof(cmd));
    if (!new_elem)
        return KO;
    new_elem->ac = 1;
    new_elem->av = malloc(sizeof(char *) * (new_elem->ac + 1));
    if (!new_elem->av) {
        free(new_elem);
        return KO;
    }
    new_elem->av[0] = arg;
    new_elem->av[1] = NULL;
    new_elem->type = END;
    new_elem->nxt = NULL;
    if (*list) {
        (*list)->nxt = new_elem;
        new_elem->prv = *list;
    } else {
        new_elem->prv = NULL;
    }
    *list = new_elem;
    return OK;
}


int     add(char *arg, cmd *list) {
    printf("add %s\n", arg);
    char **new_arg = malloc(sizeof(char *) * (list->ac + 2));
    if (!new_arg)
        return KO;
    for(int i = 0; i < list->ac; i++) {
        new_arg[i] = list->av[i];
    }
    new_arg[list->ac] = arg;
    new_arg[list->ac + 1] = NULL;
    free(list->av);
    list->av = new_arg;
    list->ac++;
    return OK;
}

int     parse(char *arg, cmd **list) {
    fflush(stdout);
    int what = -1;

    if (strcmp(arg, ";") == 0) {
        what = BREAK;
    } else if (strcmp(arg, "|") == 0) {
        what = PIPE;
    }
    if ((what == BREAK || what == PIPE) && *list == NULL) {
        return OK;
    } else if (what != BREAK && (*list == NULL || (*list && (*list)->type != END))) {
        return push(arg, list);
    } else if ((what == BREAK || what == PIPE) && *list ) {
        (*list)->type = what;
        printf("set %s\n", what == PIPE ? "pipe" : "break");
    } else {
        return(add(arg, *list));
    }
    return OK;
}

int     main(int ac, char **av, char **ev) {
    (void)ev;
    cmd *list = NULL;
    cmd *start = NULL;

    for(int i = 1; i < ac; i++) {
        if (parse(av[i], &list) == KO) {
            p_err("error: fatal\n");
            free_list(list);
            return KO;
        } else if (!start) {
            start = list;
        }
    }
    free_list(list);
    return OK;
}