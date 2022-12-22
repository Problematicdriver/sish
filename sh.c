#include "sh.h"

#define OPTSTR "xc:"
#define SISH   "sish$ "

int interrupted;
int f_trace;
int f_cmd;
int last_status;

int 
cd(Command *c) 
{
    char *path;
    if (c->argc != 1 && c->argc != 2) {
        fprintf(stderr, "Usage: cd [directory]\n");
        return EXIT_FAILURE;
    }
    if ((path = c->args[1]) == NULL) {
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
echo(Command *c)
{
    int fd, i;
    char buff[BUFSIZ];
    Redirect *r;
    
    /*
     * Won't fork for built-ins so we need to preserve STDIN and
     * STDOUT by duplicating the original fds and swap it back
     * before return
     */

    for (r = c->redirects; r != NULL; r = r->next) {
        fd = open(r->file, O_RDWR | O_CREAT, S_IRWXU);
        lseek(fd, 0, r->f_cat == 1 ? SEEK_END : SEEK_CUR);
        dup2(fd, r->red_fileno);
        close(fd);
    }
    for (i = 1; i < c->argc; i++) {
        if (strncmp(c->args[i], "$$", strlen("$$")) == 0) {
            printf("%d ", getpid());
            continue;
        }
        if (strncmp(c->args[i], "$?", strlen("$?")) == 0) {
            printf("%d ", last_status);
            continue;
        }
        printf("%s%s", c->args[i], i == c->argc - 1 ? "" : " ");
    }
    printf("\n");
   
    return EXIT_SUCCESS;
}

int
_exit_(Command *c)
{
    free_list();
    exit(EXIT_SUCCESS);
    
    /* not reached */
    return 0;
}

int (*find_builtin(char *name))(Command*) 
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
    int i;
    Redirect *r;
    printf("+ ");
    for (i = 0; i < MAX_ARGC && c->args[i] != NULL; i++) {
        printf("%s ", c->args[i]);
    }
    for (r = c->redirects; r != NULL; r = r->next) {
        if (r->red_fileno == STDIN_FILENO) {
            printf("<%s ", r->file);
        } else {
            if (r->f_cat) {
                printf(">>%s ", r->file);
            } else {
                printf(">%s ", r->file);
            }
        }
    }
    printf("\n");
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
    int fd;
    if (c->args[0] == NULL) {
        exit(EXIT_FAILURE);
    } 
    for (r = c->redirects; r != NULL; r = r->next) {
        fd = open(r->file, O_RDWR | O_CREAT, S_IRWXU);
        lseek(fd, 0, r->f_cat == 1 ? SEEK_END : SEEK_CUR);
        dup2(fd, r->red_fileno);
        close(fd);
    }
    if (execvp(c->args[0], c->args) < 0) {
       perror("execvp");
    }
}

int
run_list()
{
    Command *c;
    int (*builtin)(Command*);
    pid_t child[n_cmd];
    int pipefd[2], i, exit_status;
    int tmp_in, tmp_out;

    memset(&child, 0, sizeof(child));
    
    if (pipe(pipefd) < 0) {
        perror("pipe");
    }

    for (c = head, i = 0; c != NULL; c = c->next, i++) {
        
        if (f_trace == 1) {
            print_cmd(c);
        }

        builtin = find_builtin(c->args[0]);
        if (builtin != NULL && c == tail) {
            tmp_in = fcntl(STDIN_FILENO, F_DUPFD);
            tmp_out = fcntl(STDOUT_FILENO, F_DUPFD);
            
            if (c != head) {
                dup2(pipefd[0], STDIN_FILENO);
            }
            
            builtin(c);
            
            dup2(tmp_in, STDIN_FILENO);
            dup2(tmp_out, STDOUT_FILENO);
            close(tmp_in);
            close(tmp_out);
            
            break;
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
                builtin(c);
                exit(EXIT_SUCCESS);
            }
            run(c);
        }
        assert(child[i] > 0);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    if (bg == 0 && n_cmd > 0 && child[n_cmd - 1] > 0) {
        waitpid(child[n_cmd-1], &exit_status, 0);
    }
    return exit_status;
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
action(int signum, siginfo_t *info, void *secret) {
    if (signum == SIGTERM) {
        free_list();
        exit(EXIT_SUCCESS);
    } else if (signum == SIGCHLD) {
        interrupted = 1;
        printf("SIGCHLD\n");
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
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &action;
    sigaction(SIGINT, &act, 0);
    sigaction(SIGQUIT, &act, 0);
    sigaction(SIGTERM, &act, 0);
    sigaction(SIGTSTP, &act, 0);
    sigaction(SIGCHLD, &act, 0);

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
    
    fflush(stdin);

    while (getinput(buf, sizeof(buf)) || interrupted) {
        if (interrupted) {
            interrupted = 0;
            if (strlen(buf) == 0) 
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
        last_status = run_list();
        free_list();
        buf[0] = '\0';
    }

    /* not reached */
    return EXIT_SUCCESS;
}
