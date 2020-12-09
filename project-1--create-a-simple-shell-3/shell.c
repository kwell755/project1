#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;
pid_t pid;

void timer(int signum){ 
	kill(pid,SIGKILL);
}

void handleSig(int signum){ 
	kill(pid,SIGKILL);
}


int main() {
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];
    char *arguments[MAX_COMMAND_LINE_ARGS];
    	
    while (true) {
      
        do{ 
            char maxPath[4096];
            char* dir = getcwd(maxPath, sizeof(maxPath));
            printf("%s ", dir);
            printf("%s", prompt);
            fflush(stdout);

            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "Something went wrong with fgets");
                exit(0);
            }
 
        }while(command_line[0] == 0x0A);

        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }

      
        char* itr = command_line;
        itr = strsep(&itr,"\n");
        int i;
        for(i = 0; i<MAX_COMMAND_LINE_LEN; i++){

            char* chr = strsep(&itr, " ");

            if (chr == NULL){break;}

            if (strchr(chr, '$')!=NULL){
            memmove(chr, chr+1, strlen(chr));
            chr = getenv(chr);
            };

            arguments[i] = chr;
        }
      

        char* comm = arguments[0];
       
      
        if (strcmp(comm, "echo") == 0){
            for(i = 1; i<MAX_COMMAND_LINE_ARGS; i++){
                if (arguments[i] == NULL){break;}

                printf("%s ", arguments[i]);
            }
        }else if (strcmp(comm, "cd") == 0){
            if (arguments[1] == NULL){
                printf("Missing argument");
            }
            else{
                chdir(arguments[1]);
            }
            
        }else if (strcmp(comm, "pwd") == 0){
            char maxPath[4096];
            char* dir = getcwd(maxPath, sizeof(maxPath));
            printf("%s ", dir);
        }
        else if (strcmp(comm, "exit") == 0){
            exit(0);
        }
        else if (strcmp(comm, "env") == 0){
            for (i = 0; environ[i]!=NULL; i++){
                printf("%s \n", environ[i]);
            }
        }
        else if (strcmp(comm, "setenv") == 0){
            char* var[2];
            char* rest = arguments[1];

            for (i = 0; i<2; i++){
                char *chr = strsep(&rest, "=");
                var[i] = chr;
            }
            setenv(var[0], var[1], 1);
        }
        else{
         
            pid = fork();
            int isBackground = 0;
         
            int i = 0;
            for (i = 0; i<MAX_COMMAND_LINE_ARGS; i++){
                if (arguments[i] == NULL){break;}
            }
            i--;
          
            if (arguments[i]){
                if (strcmp(arguments[i], "&") == 0){
                    isBackground = 1;
                    arguments[i] = NULL;
                }else{
                    isBackground = 0;
                }
            }

            int status;
            if (pid <0){
                printf("Something went wrong");
                exit(1);
            }else if (pid == 0){
                if (execvp(arguments[0], arguments) == -1){
                    printf("execvp() failed: No such file or dir");
                }
                signal(SIGINT, handleSig);	
                exit(1); 
            }

            else{
                signal(SIGINT, handleSig);
                signal(SIGALRM,timer);
                alarm(10);
                
                if (isBackground == 1){
                    waitpid(pid, &status, 0);
                }
                else {
                    wait(NULL);
                }

            }
        }
        printf("\n");

      }
  
    return -1;
}