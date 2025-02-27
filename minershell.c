#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_COMMANDS 10 //Max number of commands in a pipeline for now

/*Split the string by space and return the array of tokens**/

char **tokenize(char *line){
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(int i=0; i < strlen(line); i++){
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
     perror("Sleep missing argument\n");
   }
}

void f_exit(char **tokens){
  exit(0);
}

void execute_pipeline(char *line){

  //1. Parse into pipelines
  char *commands[MAX_COMMANDS];
  int num_commands = 0;
  char *token; //pointer to next token
  char *rest = line; //pointer to the remaining line

  while((token = strtok_r(rest, '|', &rest))){
    commands[num_commands++] = strdup(token);
  }
  int pipes[num_commands -1][2]; /*[num_c -1] because there will be one less pippe than the number of commands, and [2] because there are 2 file descriptors per pipe (read & write)*/


  //2. Loop through pipeline stages
  for(i = 0; i < num_commands; i++){
    //3. Create pipes except for last command

    //Last command, no pipe created
    if((i<num_commands-1) && pipe(pipes[i] == -1)){
      exit(1);
    }
  
    //4. Fork
    pid_t pid = fork();
    if(pid == 0){
      char **tokens = tokenize(commands[i]);

      //Redirection logic
      char *in_file = NULL;
      char *out_file = NULL;

      for(int j=0; tokens[j] != NULL; j++){
	if(strcmp(tokens[j],"<") == 0){
	  in_file = tokens[j+1];
	  tokens[j] = NULL;
	  break;
	}else if(strcmp(tokens[j], ">") == 0){
	  out_file = tokens[j+1];
	  tokens[j] = NULL;
	  break;
	}
      }
      if(in_file != NULL){
	int fd = open(in_file, ORDONLY);
	if(fd == -1){ //throw error if input file can't be opened
	  perror("Failure to open input file");
	  exit(1);
	}
	dup2(fd, STDIN_FILENO);
	close(fd);
      }
      if(out_file != NULL){
	int fd = open(out_file, O_WRONLY | O_CREAT, 0644);
	if(fd == -1){
	  perror("Failed to open output file");
	  exit(1);
	}
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	close(fd);
      }

      //Pipe redirection logic
      if(i>0){ //If this is not the first command
	dup2(pipes[i-1][0], STDIN_FILENO); //Read from previous pipe
	close(pipes[i-1][1]); //close write end
      }
      if(i<num_commands -1){//if this is not the last command
	dup2(pipes[i][1], STDOUT_FILENO);//write to next pipe
	close(pipes[i][0]); //close read end
      }

      //Close all other pipe ends
      for (int j = 0; j < num_commands -1; j++){
	if(j!=i-1 && j != i){
	  close(pipes[j][0]);
	  close(pipes[j][1]);
	}
      }

	//Execute command
	execvp(tokens[0], tokens);
	exit(0);

      }else if(pid >0){//Parent process
	//Close unused pipe ends
	if(i>0){
	  close(pipes[i-1][0]);
	  close(pipes[i-1][1]);
	}
	if(i==num_commands-1){
	  for(int j = 0; j<num_commands-1;j++){
	    close(pipes[j][0]);
	    close(pipes[j][1]);
	  }
	}
	if(i==num_commands-1){
	  for(int j = 0; j <num_commands;j++){
	    wait(NULL);}
	}
      }

      for(int i = 0; i<num_commands; i++){
	free(commands[i]);
      }
    


}

void execute_command(char *line){


}

int main(int arfc, char* argv[]){
  char line[MAX_INPUT_SIZE];
  char **tokens;
  int i;


  const char *commands[] = {"ls", "cd", "pwd", "cat", "ps", "echo", "wc", "top", "grep", "sleep", "exit", NULL};
  void (*command_funct[])(char **) = {execute_command, f_cd, execute_command, execute_command, execute_command, execute_command, execute_command, execute_command, execute_command, execute_command, f_exit};
  
  while(1){

    /*BEGIN: TAKING INPUT */
    bzero(line, sizeof(line));
    printf("$ ");
    scanf("%[^\n]", line);
    getchar();
    /* END: TAKING INPUT */

    //if there is a '|' pipe
    char *pipe_char = strchr(line, '|');
    if (pipe_char != NULL){
      execute_pipeline(line);
    } else {
    
      line[strlen(line)] = '\n';
      tokens = tokenize(line);

      for(i=0; commands[i]!=NULL;i++){
	if(strcmp(tokens[0], commands[i]) == 0){
	  command_funct[i](tokens); /*call corresponding function. Notice that execute_command is called for the commands that require fork(), execvp() and or redirection. It will also handle redirection*/
	}        
      }

    }
    
    
    // Freeing the allocated memory
    for(i=0;tokens[i]!=NULL;i++){
      free(tokens[i]);
     }
    free(tokens);
  }  
  return 0;
}
