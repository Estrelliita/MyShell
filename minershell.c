#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/*Split the string by space and return the array of tokens**/

char **tokenize(char *line){
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i=0; i < strlen(line); i++){
    char readChar = line[i];

    if(readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0'; //Null-terminate the current token
      if(tokenIndex != 0){ //If the token is not empty
	tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	strcpy(tokens[tokenNo++], token); //Copy the token to the tokens array
	tokenIndex = 0;
      }

    }else{
      token[tokenIndex++] = readChar; //Add character to the current token
    }

  }
  
  free(token);
  tokens[tokenNo] = NULL;
  return tokens;
}

void f_ls(char **tokens){
  pid_t pid = fork();

  if (pid == 0){ //Child process
    execvp(tokens[0], tokens);
    printf("execvp");
    exit(1);
  } else if(pid < 0){
    perror("Fork failed");
  } else {
    int status;
    waitpid(pid, &status, 0);  // Parent process waits for child to finish    
  }
}

void f_cd(char **tokens){
   if (tokens[1] != NULL) { //Check if argument (dir) is provided
        if (chdir(tokens[1]) != 0) {
            perror("chdir failed");
        }
    } else {
        chdir(getenv("HOME")); //Go to home directory of no arguments are provided
    }
}

void f_pwd(char **tokens){ //No need to fork here because pwd operates directly on current process's environment
    char cwd[1024]; //Char array for current working directory limited to 1024 bytes to prevent overflow
    if (getcwd(cwd, sizeof(cwd)) != NULL) { 
        printf("%s\n", cwd);
    } else {
        perror("getcwd");
    }
}

void f_cat(char **tokens){
  pid_t pid = fork();

  if (pid == 0){ //Child process
    execvp("/bin/cat", tokens);
    exit(1);
  } else if(pid < 0){
    perror("Fork failed");
  } else {
    int status;
    waitpid(pid, &status, 0);  // Parent process waits for child to finish    
  }
}

void f_ps(char **tokens){
  pid_t pid = fork();

  if (pid == 0){ //Child process
    execvp("/bin/ps", tokens);
    exit(1);
  } else if(pid < 0){
    perror("Fork failed");
  } else {
    int status;
    waitpid(pid, &status, 0);  // Parent process waits for child to finish    
  }

	
}

void f_echo(char **tokens){
	for (int i = 1; tokens[i] != NULL; i++) { //Start from second token since first was the command
        printf("%s ", tokens[i]);
    }
    printf("\n");
}

void f_wc(char **tokens){
  pid_t pid = fork();

  if (pid == 0){ //Child process
    execvp("/bin/wc", tokens);
    exit(1);
  } else if(pid < 0){
    perror("Fork failed");
  } else {
    int status;
    waitpid(pid, &status, 0);  // Parent process waits for child to finish    
  }
}

void f_top(char **tokens){} //Not implememted yet

void f_grep(char **tokens){} //Not implemented yet

void f_sleep(char **tokens){
   if(tokens[1] != NULL) {
      unsigned int duration = atoi(tokens[1]);
      sleep(duration); //sleep as long as user typed in second token
   } else{
	   perror("Sleep missing argument\n")
   }
}

void f_exit(char **tokens){
  exit(0);
}

int main(int arfc, char* argv[]){
  char line[MAX_INPUT_SIZE];
  char **tokens;
  int i;

  const char* commands[] = {"ls", "cd", "pwd", "cat", "ps", "echo", "wc", "top", "grep", "sleep", "exit"};

  void (*command_funct[])(char **) = {f_ls, f_cd, f_pwd, f_cat, f_ps, f_echo, f_wc, f_top, f_grep, f_sleep, f_exit};
  
  while(1){

    /*BEGIN: TAKING INPUT */
    bzero(line, sizeof(line));
    printf("$ ");
    scanf("%[^\n]", line);
    getchar();

    printf("Command entered: %s (remove this debug output later)\n", line);
      /* END: TAKING INPUT */
    
    line[strlen(line)] = '\n'; //terminate wi
    tokens = tokenize(line);

    //do what the commands, here we just print them
    for(i=0; commands[i]!=NULL;i++){
      if(strcmp(tokens[0], commands[i]) == 0){
	command_funct[i](tokens); //call corresponding function
      }
      //printf("found token %s (remove this debug output later)\n", token[i]);
    }
    
    
    // Freeing the allocated memory
    for(i=0;tokens[i]!=NULL;i++){
      free(tokens[i]);
     }
    free(tokens);
  }  
  return 0;
}
