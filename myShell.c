#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<sys/wait.h>
#include<stdbool.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

bool pro=true;
void stop(){
    pro=false;
    printf("\n");
    return;
}
void reaper(int cid[],int n){
    int k,i;   
    pid_t p;
    for(i=0;i<n;i++){
        if((cid[i]!=-1) && (p=waitpid(cid[i],&k,WNOHANG)!=0)){
            printf("Shell:Background process finished\n");
            cid[i]=-1;
        }
    }
  return ;
}
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

int main(int argc, char* argv[]) {
    signal(SIGINT,stop);
	char  line[MAX_INPUT_SIZE];            
	char  **token;              
	int i;
    int *cid=(int *)malloc(MAX_NUM_TOKENS*sizeof(int));
    int n=0,yeet=0,bg=0;;
	while(1) {

    int normal=1;
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		line[strlen(line)] = '\n'; //terminate with new line
		token = tokenize(line);
        //##############################################  MY code #########################################################
		if(strcmp(token[0],"cd")==0){/////////////////CHDIR
			normal=0;
            if(chdir(token[1])!=0){
                printf("Shell:Incorrect command\n");
            };

		}
        else if(strcmp(token[0],"exit")==0){///////////////////EXIT
            normal=0;
            for(int i=0;i<n;i++){
                if(cid[i]!=-1)
                    kill(cid[i],SIGTERM);
            }
            yeet=-1;
        }
        else{
            for(i=0;token[i]!=NULL;i++){
                if(strcmp(token[i], "&&")==0){////////////SERIES/////////////////////////////
                    normal=0;
                    char *command=strtok(line,"&&");
                    while((command!=NULL) && pro){
                        char** t=tokenize(command);
                        pid_t pid = fork(); 
                        if (pid == -1) {//this whole thing can be relaced by execute
                            printf("\nFailed forking child..\n");
                        } else if (pid == 0) {
                            if (execvp(t[0], t) < 0) {//execute command
                                printf("Could not execute command..\n");
                            }
                            exit(0);
                        } 
                            waitpid(pid,NULL,0);//wait for each child b4 creating new child
                            command=strtok(NULL,"&&");
                            for(int i=0;t[i]!=NULL;i++){
                                free(t[i]);
                            }
                            free(t);
                    }  
                    break;
                }
                else if(strcmp(token[i],"&&&")==0){///PARALLEL/////////////////////////////////
                    normal=0;
                    int num=0;
                    int *child_id=(int *)malloc(MAX_NUM_TOKENS*sizeof(int));
                    char *command=strtok(line,"&&");
                    while((command!=NULL) && pro){
                        char **t=tokenize(command);
                        pid_t pid = fork(); 
                        if (pid == -1) {
                            printf("\nFailed forking child..\n");
                        } else if (pid == 0) {
                            if (execvp(t[0], t) < 0) {//execute command
                                printf("Could not execute command..\n");
                            }
                            exit(0);
                        }
                            //wait(NULL);//wait for each child b4 creating new child
                            child_id[num++]=pid;
                            command=strtok(NULL,"&&");
                            for(int i=0;t[i]!=NULL;i++){
                                free(t[i]);
                            }
                            free(t);
                    }
                    for(int i=0;i<num;i++){
                    waitpid(child_id[i],NULL,0);
                    }
                    break;
                }
            }
            if((strcmp(token[--i],"&")==0) && (normal==1)){///////////////BACKGROUND//////////////////
                normal=0;
                char *command=strtok(line,"&");
                char **t=tokenize(command);
                pid_t pid=fork();
                if (pid == -1) {
                    printf("\nFailed forking child..\n");
                    return 0;
                } 
                else if (pid == 0) {//create child and run multiple times
                    if (execvp(t[0], t) < 0) {
                        printf("Could not execute command..\n");
                    }
                    exit(0);
                }
                else{
                    for(int i=0;t[i]!=NULL;i++){
                        free(t[i]);
                    }
                    free(t); 
                    if(bg==0){
                        setpgid(pid,bg);
                        bg=pid;
                    }
                    else
                        setpgid(pid,0);
                            //printf("%d ",bg);
                }
                cid[n++]=pid;
            }
        }
        if(normal==1){////////////////FOREGROUND
            pid_t pid=fork();
            if (pid == -1) {
                printf("\nFailed forking child..\n");
            } else if (pid == 0) {//create child and run multiple times
                if (execvp(token[0], token) < 0) {
                    printf("Could not execute command..\n");
                }
                exit(0);
            }
            else
                waitpid(pid,NULL,0);
        }
		reaper(cid,n);

        for(i=0;token[i]!=NULL;i++){
                free(token[i]);
        }
            free(token);
        if(yeet==-1){
            free(cid);
            break;
        }
	}
	return 0;
}
