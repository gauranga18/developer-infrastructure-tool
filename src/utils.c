#include<stdio.h>
#include<stdlib.h>
#include "utils.h"
int run_command(const char *cmd){
	int ret = system(cmd);
return ret;
}