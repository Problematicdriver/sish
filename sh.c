#include "sh.h"

#define OPTSTR "xc:"

char *
getinput(char *buffer, size_t buflen) {
	printf("$$ ");
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
        if (r->fd == STDIN) {
            strlcat(in, r->file, sizeof(in));
            strlcat(in, " ", sizeof(in));
        } else {
            strlcat(out, r->file, sizeof(out));
            strlcat(in, " ", sizeof(out));
        }
    }
    printf("{>%s,<%s}", out, in); 
}

void
print_list()
{
    Command* c;
    printf("%d commands in total\n", n_cmd);
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
        r->fd = open(r->file, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG);
        lseek(r->fd, 0, r->f_cat == 1 ? SEEK_END : SEEK_CUR);
        dup2(r->fd, r->red_fileno);
    }
    if (execvp(c->args[0], c->args) < 0) {
        perror("execvp"); 
    }
    /*
    for (r = tail->redirects; r != NULL; r = r->next) {
        close(r->fd);
    }
    */
}

void
run_list()
{
    Command *c;
    pid_t child[n_cmd];
    int pipefd[2], i;
    Redirect *r;

    memset(&child, 0, sizeof(child));
    if (pipe(pipefd) < 0) {
        perror("pipe");
    }
    
    for (c = head, i = 0; c != NULL; c = c->next, i++) {
        
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
    waitpid(child[n_cmd-1], NULL, 0);
}

int
main(int argc, char **argv)
{
    int ch, f_trace = 0, f_cmd = 0;
    char *c, buf[BUFSIZ];

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
        yyin = fmemopen(&buf, strlen(buf), "r");
        if (yyparse() == 1) {
            perror("yyparse");
        }
        // print_list();
        run_list();
    }
    
    return EXIT_SUCCESS;
}
