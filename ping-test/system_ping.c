#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int child;
    // child = fork();
    // if(child==0){
    execlp("echo", "echo", "hwllo world", NULL);
    // execlp("ping", , "google.com", NULL)
    // }
    // int status;
    // wait(&child);
}