#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int dummy_main(int argc, char **argv);

int main(int argc, char **argv) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset,SIGCONT);

    sigprocmask(SIG_BLOCK, &sigset, NULL);

    int sig;

    sigwait(&sigset, &sig);
    int ret = dummy_main(argc, argv);

    
    return ret;
}
#define main dummy_main