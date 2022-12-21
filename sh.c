#include "sh.h"

#define OPTSTR "xc:"
#define SISH   "sish$ "

int interrupted;
int f_trace;
int f_cmd;

int 
cd(int argc, char* args[]) 
{
    char *path;
    if (argc != 1 && argc != 2) {
        fprintf(stderr, "Usage: cd [directory]\n");
        return EXIT_FAILURE;
    }
    if ((path = args[1]) == NULL) {
        if ((path = getenv("HOME")) == NULL) {
            return EXIT_FAILURE;
        }
    }
    if (chdir(path) < 0) {
        perror("chdir");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int
echo(int argc, char* args[])
{
    int i;
    for (i = 1; i < argc; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    return EXIT_SUCCESS;
}

int
_exit_(int argc, char* args[])
{
    (void)argc;
    (void)args;
    free_list();
    exit(EXIT_SUCCESS);
    
    /* not reached */
    return 0;
}

int (*find_builtin(char *name))(int, char* []) 
{ 
    if (strncmp(name, "cd", strlen("cd")) == 0) { 
        return cd;  
    }
    if (strncmp(name, "echo", strlen("echo")) == 0) {
        return echo;
    }
    if (strncmp(name, "exit", strlen("exit")) == 0) {
        return _exit_;
    }
    return NULL; 
}

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
trace(int argc, char* args[])
{
    int i;
    printf("+ ");
    for (i = 0; i < argc; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
}

void
run_list()
{
    Command *c;
    int (*builtin)(int, char* []);
    pid_t child[n_cmd];
    int pipefd[2], i;

    memset(&child, 0, sizeof(child));
    if (pipe(pipefd) < 0) {
        perror("pipe");
    }
    
    for (c = head, i = 0; c != NULL; c = c->next, i++) {
        Redirect *r;
        
        if (f_trace == 1) {
            trace(c->argc, c->args);
        }

        builtin = find_builtin(c->args[0]);
        if (builtin != NULL && c == tail) {
            builtin(c->argc, c->args);
            return;
        }
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
            if (builtin != NULL) {
                builtin(c->argc, c->args);
                exit(EXIT_SUCCESS);
            }
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
sig_handler(int signum) {
    if (signum == SIGTERM) {
        free_list();
        exit(EXIT_SUCCESS);
    } else {
        interrupted = 1;
        printf("\n");
    }
}

int
main(int argc, char **argv)
{
    int ch;
    char *c, buf[BUFSIZ];
    
    struct sigaction act = { 0 };
    act.sa_handler = &sig_handler;
    sigaction(SIGINT, &act, 0);
    sigaction(SIGQUIT, &act, 0);
    sigaction(SIGTERM, &act, 0);
    sigaction(SIGTSTP, &act, 0);

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
    
    while (getinput(buf, sizeof(buf)) || interrupted) {
        if (interrupted) {
            interrupted = 0;
            continue;
        }
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
