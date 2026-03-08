#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "utils.h"
int run_command_fork(const char **argv){
pid_t pid = fork();
if(pid == -1){
	//Fork failed
	return -1;
}
if(pid ==0){
	//Execute the command
	execvp(argv[0],(char **)argv);
	//Only reached if exec fails
	exit(127);
}
//Parent : wait for child
int status;
waitpid(pid,&status,0);
if(WIFEXITED(status)){
	return WEXITSTATUS(status);
}
return -1;
}