#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>
#include <assert.h>
#include "tokens.h"

#define MAX_LINE 256

void welcomeScreen();
void printHelpMessage();
int checkCommandType(char *cmd);
void handleCommand(char *cmd);
void runCommand(char *cmd);
int delimiterIndex(char **args, char *delimeter);
void runInputRedirect(char *cmd);
void runPipe(char *cmd);
void runOutputRedirect(char *cmd);

static char prevCommand[MAX_LINE];

void runSubsequentPipe(char *cmd, int prev_read_fd, int prev_write_fd);

void welcomeScreen() {
  printf("Welcome to mini-shell.\n");
}

void printHelpMessage() {
	printf("cd:\t Changes the current working directory of the shell to the specified path\n");
	printf("source:\t Executes a script. Takes a filename as an argument and processes each line of the file as a command\n");
	printf("prev:\t Prints the previous command and executes it again\n");
	printf("help:\t Explains all the built-in commands avaible in your shell\n");
}

int checkCommandType(char *cmd) {
  int inQuotes = 0;
  for (int i = 0; cmd[i] != '\0'; i++) {
    if (cmd[i] == '"') {
      inQuotes = inQuotes * -1 + 1;
    }
    if (cmd[i] == '|' && !inQuotes) {
      return 1;
    } else if (cmd[i] == '<' && !inQuotes) {
      return 2;
    } else if (cmd[i] == '>' && !inQuotes) {
      return 3;
    }
  }
  return 0;
}

void handleCommand(char *cmd) {
  int cmdType = checkCommandType(cmd);
  switch (cmdType) {
    case 1:
      runPipe(cmd);
      break;
    case 2:
      runInputRedirect(cmd);
      break;
    case 3:
      runOutputRedirect(cmd);
      break;
    default:
      runCommand(cmd);
      break;
  }
}

void runCommand(char *cmd) {
	char **tokens = get_tokens(cmd);

	if (strcmp(tokens[0], "cd") == 0) {
    	chdir(tokens[1]);
	} else if (strcmp(tokens[0], "source") == 0) {
		FILE *fp;
		fp = fopen(tokens[1], "r");
		if (fp == NULL) {
			printf("No such file: %s\n", tokens[1]);
		} else {
			char file_input[MAX_LINE];
			while (fgets(file_input, MAX_LINE, fp)) {
				printf("shell $ %s", file_input);
				handleCommand(file_input);
			}
			fclose(fp);
		}
	} else if (strcmp(tokens[0], "prev") == 0) {
		if (prevCommand[0] == '\0') {
    		printf("No previous command found.\n");
		} else {
			printf("shell $ %s", prevCommand);
			runCommand(prevCommand);
		}
	} else {
		int child = fork();
		if(child == -1) {
			printf("Error, failed forking child. \n");
		}
		else if(child == 0) {
			if (strcmp(tokens[0], "help") == 0) {
				printHelpMessage();
			}
			else if (execvp(tokens[0], tokens) < 0) { 
				printf("%s: command not found \n", tokens[0]); 
				exit(1); 
			} 
			exit(0);
		} else {
			wait(NULL); 
		}
	}
  free_tokens(tokens);
}

int delimiterIndex(char **args, char *delimeter) {
  int index = -1;
	int i = 0;
  while (args[i] != NULL) {
    if (strcmp(args[i], delimeter) == 0) {
      index = i;
    }
		i++;
  }
  return index;
}

void runInputRedirect(char *cmd) {
	char **tokens = get_tokens(cmd);
	int splitIndex = delimiterIndex(tokens, "<");

	char *inputRedirectCommand[256];
	for (int i=0; i<splitIndex; i++) {
		inputRedirectCommand[i] = tokens[i];
	}
	inputRedirectCommand[splitIndex] = NULL;

	int child = fork();
	if(child == -1) {
		printf("Error, failed forking child. \n");
	}
	else if(child == 0) {
		if (close(0) == -1) {
			perror("Error closing stdin");
			exit(1);
		}
		int fd = open(tokens[splitIndex + 1], O_RDONLY);
		assert(fd == 0);
		if (execvp(inputRedirectCommand[0], inputRedirectCommand) == -1) {
			perror("Error - execvp failed");
			exit(1);
		}
		exit(0);
	} else {
		wait(NULL);
	}
  free_tokens(tokens);
}

void runOutputRedirect(char *cmd) {
	char **tokens = get_tokens(cmd);
	int splitIndex = delimiterIndex(tokens, ">");

	char *outputRedirectCommand[256];

	for (int i=0; i<splitIndex; i++) {
		outputRedirectCommand[i] = tokens[i];
	}
	outputRedirectCommand[splitIndex] = NULL;

	int child = fork();
	if(child == -1) {
		printf("Error, failed forking child. \n");
	}
	else if(child == 0) {
		if (close(1) == -1) {
			perror("Error closing stdout");
			exit(1);
		}
		int fd = open(tokens[splitIndex + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
		assert(fd == 1);

		if (execvp(outputRedirectCommand[0], outputRedirectCommand) == -1) {
			perror("Error - execvp failed");
			exit(1);
		}
		exit(0);
	} else {
		wait(NULL); 
	}
  free_tokens(tokens);
}

void runPipe(char *cmd) {
	int childA_pid = fork(); // fork child A
  	if (childA_pid == 0) {
		int pipe_fds[2];
		assert(pipe(pipe_fds) == 0); // create pipe
		int read_fd = pipe_fds[0];
		int write_fd = pipe_fds[1];

		char thisPipeCmdBuf[MAX_LINE];
		strcpy(thisPipeCmdBuf, "");
		char nextPipeCmdBuf[MAX_LINE];
		strcpy(thisPipeCmdBuf, "");

		int inQuotes = 0;
		for (int i = 0; cmd[i] != '\0'; i++) {
			if (cmd[i] == '"') {
				inQuotes = inQuotes * -1 + 1;
			}
			if (cmd[i] == '|' && !inQuotes) {
				strcpy(nextPipeCmdBuf, &cmd[i+1]);
				break;
			} else {
				strncat(thisPipeCmdBuf, &cmd[i], 1);
			}
		}

		char **tokens = get_tokens(thisPipeCmdBuf);

		int childB_pid = fork(); // fork child B
		if (childB_pid == 0) {
			close(1); // close stdout
			assert(dup(write_fd) == 1); // hook write end of pipe to stdout
			close(read_fd); // close read end of pipe

			// execute first pipe arguments
			if (execvp(tokens[0], tokens) == -1) {
				printf("%s: command not found\n", tokens[0]);
				exit(1);
			}
		} else {
			wait(NULL);
			runSubsequentPipe(nextPipeCmdBuf, read_fd, write_fd);
    }
    free_tokens(tokens);
	} else {
		wait(NULL);
	}
}

void runSubsequentPipe(char *cmd, int prev_read_fd, int prev_write_fd) {  
	char thisPipeCmdBuf[MAX_LINE];
	strcpy(thisPipeCmdBuf, "");
	char nextPipeCmdBuf[MAX_LINE];
	strcpy(thisPipeCmdBuf, "");

	int inQuotes = 0;
	for (int i = 0; cmd[i] != '\0'; i++) {
		if (cmd[i] == '"') {
			inQuotes = inQuotes * -1 + 1;
		}
		if (cmd[i] == '|' && !inQuotes) {
			strcpy(nextPipeCmdBuf, &cmd[i+1]);
			break;
		} else {
			strncat(thisPipeCmdBuf, &cmd[i], 1);
		}
	}

	char **tokens = get_tokens(thisPipeCmdBuf);
	// if command contains no pipe
	if (checkCommandType(cmd) != 0) {
		close(prev_write_fd);
		close(0);
		assert(dup(prev_read_fd) == 0);
		int pipe_fds[2];
		assert(pipe(pipe_fds) == 0);
		int read_fd = pipe_fds[0];
		int write_fd = pipe_fds[1];

		int child_pid = fork();
		if (child_pid == 0) {
			close(1); // close stdout
			assert(dup(write_fd) == 1); // hook write end of pipe to stdout
			close(read_fd); // close read end of pipe

			// execute this pipe's command
			if (execvp(tokens[0], tokens) == -1) {
				printf("%s: command not found\n", tokens[0]);
				exit(1);
			}
		} else {
			wait(NULL);
			runSubsequentPipe(nextPipeCmdBuf, read_fd, write_fd);
		}
	} else { // else case writes the output to stdoutput
		close(0); // close stdin
		assert(dup(prev_read_fd) == 0); // hook read end of pipe to stdin
		close(prev_write_fd); // close write end of pipe

		// execute final pipe command
		if (execvp(tokens[0], tokens)  == -1) {
			printf("%s: command not found\n", tokens[0]);
			exit(1);
		}
	}
  free_tokens(tokens);
}

int main(int argc, char **argv) {
  welcomeScreen(); // welcome message
  strcpy(prevCommand, ""); // initialize prevCommand
  char *commands = (char *)malloc(MAX_LINE * sizeof(char));

  while(1) { // runs until exit
    printf("shell $ ");
    if(!fgets(commands, MAX_LINE, stdin)) { // if fgets returns 0, user pressed CTRL-D
      printf("\n"); 
      break;	
    }
    if (strcmp(commands, "exit\n") == 0) {
      break;
    }

    char buf[MAX_LINE];
    strcpy(buf, "");

    int inQuotes = 0;
    for (int i = 0; commands[i] != '\0'; i++) {
      if (commands[i] == '"') {
      inQuotes = inQuotes * -1 + 1;
      }
      if (commands[i] == ';' && !inQuotes) {
        char **tokens = get_tokens(buf);
        if (strcmp(tokens[0], "prev") != 0) {
        	strcpy(prevCommand, buf);
        	free_tokens(tokens);
        }
        handleCommand(buf);
        strcpy(buf, "");
      } else {
        strncat(buf, &commands[i], 1);
      }
    }
    if (buf[0] != '\0') {
      char **tokens = get_tokens(buf);
      if (strcmp(tokens[0], "prev") != 0) {
      	strcpy(prevCommand, buf);
      	free_tokens(tokens);
      }
      handleCommand(buf);
      strcpy(buf, "");
    }
  }
  printf("Bye bye.\n");
  free(commands); // free string buffer 
  return 0;
}