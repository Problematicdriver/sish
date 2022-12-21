#include "sh.h"

#define OPTSTR "xc:"
#define SISH   "sish$ "

char *
getinput(char *buffer, size_t buflen) {
	printf(SISH);
	return fgets(buffer, buflen, stdin);
}

void
print_cmd(Command *c)
{
    char buff[20], in[20], out[20];
    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    int i;
    Redirect *r;
    printf("[%s] ", c->args[0]);
    for (i = 1; i < MAX_ARGC && c->args[i] != NULL; i++) {
        printf("%s ", c->args[i]);
    }
    for (r = c->redirects; r != NULL; r=r->next) {
        if (r->fd == STDIN_FILENO) {
            strlcat(in, r->file, 20);
            strlcat(in, " ", 20);
        } else {
            strlcat(out, r->file, 20);
            strlcat(in, " ", 20);
        }
    }
    printf("{>%s<%s}", in, out); 
}

void
print_list()
{
    Command* c;
    for (c = head; c != NULL; c = c->next) {
        print_cmd(c);
        printf("%s", c->next == NULL ? "\n" : " -> ");   
    }
}

void
run(Command *c)
{
    char name[20];
    Redirect *r;
    if (c->args[0] == NULL) {
        return;
    }
    for (r = c->redirects; r != NULL; r = r->next) {
        r->fd = open(r->file, O_RDWR | O_CREAT, S_IRWXU);
        lseek(r->fd, 0, r->f_cat == 1 ? SEEK_END : SEEK_CUR);
        dup2(r->fd, r->red_fileno);
    }
    if (execvp(c->args[0], c->args) < 0) {
       perror("execvp"); 
    }
    for (r = c->redirects; r != NULL; r = r->next) {
       close(r->fd);
    }
}

void
run_list()
{
    Command *c;
    pid_t child[n_cmd];
    int pipefd[2], i;

    memset(&child, 0, sizeof(child));
    if (pipe(pipefd) < 0) {
        perror("pipe");
    }
    
    for (c = head, i = 0; c != NULL; c = c->next, i++) {
        Redirect *r;
        child[i] = fork();
        if (child[i] == 0) {
            if (c != head) {
                dup2(pipefd[0], STDIN_FILENO);
            } 
            if (c != tail) {
                dup2(pipefd[1], STDOUT_FILENO);
            }
            close(pipefd[0]);
            close(pipefd[1]);
            run(c);
        }
        assert(child[i] > 0);
    }
    close(pipefd[0]);
    close(pipefd[1]);
    if (n_cmd > 0) {
        waitpid(child[n_cmd-1], NULL, 0);
    }
}

void
free_list()
{
    Command *tmp;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}



void
sig_quit(int signo) {
    (void)signo;
}

void
sig_int(int signo) {
	(void)signo;
}

int
main(int argc, char **argv)
{
    int ch, f_trace = 0, f_cmd = 0;
    char *c, buf[BUFSIZ];
    
    signal(SIGQUIT, sig_quit);
    signal(SIGINT, sig_int);

    while ((ch = getopt(argc, argv, OPTSTR)) != -1) {
        switch (ch) {
            case 'c':
                f_cmd = 1;
                c = optarg;
                break;
            case 'x':
                f_trace = 1;
                break;
            case '?':
                (void)fprintf(stderr, "Usage: %s [ -x ] [ -c command ]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }
    if (opterr != 0 && optind != argc) {
        (void)fprintf(stderr, "Usage: %s [ -x ] [ -c command ]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (f_cmd) {
        if (execlp(c, c) < 0) {
            perror("execvp");
        }
        return EXIT_SUCCESS;
    }

    while (getinput(buf, sizeof(buf))) {
        buf[strlen(buf) - 1] = '\0';    
        if (strlen(buf) == 0) {
            continue;
        }
        yyin = fmemopen(&buf, strlen(buf), "r");
        if (yyparse() == 1) {
            perror("yyparse");
        }
        run_list();
        free_list();
    }

    /* not reached */
    return EXIT_SUCCESS;
}
