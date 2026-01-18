#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "help.h"
#include "init.h"
#include "deploy.h"
#include "clone.h"
int main(int argc, char *argv[]) {
    // Print the help command when writing wrong syntax
   if(argc == 1){
    print_help(argv[0]);
    return 0;
   }
   if(strcmp(argv[1],"help")==0 || strcmp(argv[1], "--help")==0 || strcmp(argv[1], "-h")==0){
    print_help(argv[0]);
    return 0;
   }
   // Check for 'deploy' command
   if(strcmp(argv[1], "deploy") == 0){
    if(argc <3){
        fprintf(stderr,"Usage: %s deploy <repo_url> \n", argv[0]);
        printf("Run Help command for more information.\n");
        return 1;
    }
    return deploy_repo(argv[2]);
   }
   // Check for 'init' command
   if(strcmp(argv[1],"init") == 0){
    if(argc <3){
        fprintf(stderr,"Usage: %s init <project_name>\n", argv[0]);
        printf("Run Help command for more information.\n");
        return 1;
    }if(init_project(argv[2]) !=0){
        return 1;
   }
   return 0;
}
  if(strcmp(argv[1],"deploy") != 0){
        fprintf(stderr, "Error: Unknown command '%s'\n", argv[1]);
        printf("Run Help command for more information.\n");
        return 1;
    }
    return 0;
}