#include<stdio.h>
#include<string.h>
#include "utils.h"
int main(int argc, char *argv[]) {
    // printf("argc = %d\n",argc);
    // if(argc<3){
    //     printf("Usage: tool deploy <repo_url>\n");
    //     return 1;
    // }
    // if(strcmp(argv[1],"deploy")!=0){
    //     printf("Unknown Command: %s\n",argv[1]);
    //     return 1;
    // }
    // char *repo_url = argv[2];
    // printf("Starting deployment for: %s\n", repo_url);
    // for(int i=0; i<argc;i++){
    //     printf("argv[%d] = %s\n",i ,argv[i]);    
    // }    
    run_command("git --version");
    run_command("ls");
    run_command("pwd");
    run_command("git clone http://github.com/gauranga18/forge.git");
    return 0;
}