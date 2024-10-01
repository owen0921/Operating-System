#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<unistd.h>
#include<ctype.h>

#define MAX_LENGTH 300 		// limit the size of user input cmd
#define MAX_TOKEN_NUM 50 	// limit the number of parameters

#define NORMAL         0	// define the type of the cmd
#define IN_REDIRECT    1
#define OUT_REDIRECT   2
#define PIPE           3

// define the struct of the cmd that be parsed
typedef struct ParseInfo{
	int background; 		// mark the cmd if it is executing in background
	int type; 				// which type is the cmd ? NORMAL, IN_REDIRECT, OUT_REDIRECT or PIPE
	char *file; 			// the file that is in-redirect or out-redirect
	char **pipe_args; 		// store the parameters of pipe
}ParseInfo;

// define the function
char *read_cmd(); 										// read the cmd
char *remove_space(char *str); 							// remove the space of cmd
ParseInfo parse_cmd(char **argv); 						// parse the info of cmd
int execute_cmd(char **argv, ParseInfo info); 			// execute the cmd
char **split_cmd(char *cmd);							// split the cmd into parameters
void execute_outredirect(char **argv, ParseInfo info);  // execute the outredirect cmd
void execute_inredirect(char **argv, ParseInfo info);	// execute the inredirect cmd


int main(){
    char *cmd;
    char **argv;
    ParseInfo cmd_info;

	while(1){
		printf("[my_shell]: ");
		cmd = read_cmd(); 					// read the cmd entered by the user

		if(cmd == NULL) continue; 			// if the cmd is empty, try to get it again

		if(strcmp(cmd, "exit") == 0) break; // if the user input "exit", break the loop

		argv = split_cmd(cmd); 				// split the cmd into parameters
		cmd_info = parse_cmd(argv); 		// parse the cmd to gain the information
		execute_cmd(argv, cmd_info); 		// execute the cmd
	}
	return 0;
}

char *read_cmd(){
    static char cmd[MAX_LENGTH];
    memset(cmd, '\0', sizeof(char) * MAX_LENGTH);

    fgets(cmd, MAX_LENGTH, stdin); // read the cmd from the stdin
    if (feof(stdin)) return NULL; // if reach the end, return NULL

    char *temp = remove_space(cmd); // remove the space of the cmd
    if (strlen(temp) == 0) return NULL; // After remove the space, if it is NULL, then return NULL

    return temp;
}

// remove leading and trailing whitespace characters from the string
char *remove_space(char *str){
    if(!str) return str; // if the str is empty, just return it

    while (isspace(*str)) ++str; // skip the front whitespace

	// get the end position of the str
    char *last = str;
    while (*last != '\0') ++last;
    last--;

	// skip the end whitespace
    while (isspace(*last)) *last-- = '\0';

    return str;
}

char **split_cmd(char *cmd_seq) {
    char *token;
    static char *tokens[MAX_TOKEN_NUM];
    memset(tokens, '\0', sizeof(char *) * MAX_TOKEN_NUM); // initialize the tokens with all NULL

    int position = 0; // index of tokens
    token = strtok(cmd_seq, " \t\r\n\a"); // split the cmd
    while (token != NULL) {
        tokens[position] = token;
        position++;

		// if the CMD is too long, print the error messages
        if (position >= MAX_TOKEN_NUM) {
            fprintf(stderr, "Error: Command is too long.\n");
            exit(EXIT_FAILURE);
        }

        token = strtok(NULL, " \t\r\n\a");
    }

    return tokens;
}

// parse the cmd to get the information
ParseInfo parse_cmd(char **argv) {
    ParseInfo info = {0, NORMAL, NULL, NULL}; // initialize the ParseInfo
    int count = 0; // count the number of cmd's parameters
	int i = 0; // go through the argv
	int j = 0; // use to store the parameters after the pipe symbol
	int check = 0; // check if the format of cmd is correct

    while (argv[count]) ++count;
    count--; // justify to the correct number

 	// check if the cmd is executed in the background
    if (strcmp(argv[count], "&") == 0) {
        info.background = 1;
        argv[count] = NULL;
        count--;
    }

    // go through all the parameters to check if it is inredirect, outredirect or pipe
    for (i = 0, check = 0; i <= count; i++) {
        if (strcmp(argv[i], ">") == 0) {
            info.type = OUT_REDIRECT;
            info.file = argv[i + 1];
        }

        if (strcmp(argv[i], "<") == 0) {
            info.type = IN_REDIRECT;
            info.file = argv[i + 1];
        }

        if (strcmp(argv[i], "|") == 0) {
            static char *pipe_args[MAX_TOKEN_NUM];
            memset(pipe_args, '\0', sizeof(char *) * MAX_TOKEN_NUM);
            info.type = PIPE;

            // store cmd parameters after the pipe symbol
            for (j = i + 1; j <= count; j++){
                pipe_args[j - i - 1] = argv[j];
            }
            info.pipe_args = pipe_args;
        }

		// check if the format of cmd is correct
        if (!strcmp(argv[i], ">") || !strcmp(argv[i], "<") || !strcmp(argv[i], "|")) {
            argv[i] = NULL;
            check++;
        }

		// if the format of cmd is not correct, print the error messages
        if (check > 1 || (!strcmp(argv[0], "cd") && count > 1) ||
        (check == 1 && (info.pipe_args == NULL && info.file == NULL))) {
            fprintf(stderr, "Error: error command.\n");
            exit(EXIT_FAILURE);
        }
    }

    return info;
}

int execute_cmd(char **argv, ParseInfo info){
	if(strcmp(argv[0], "cd") == 0){
		char *path = argv[1]; // if there is any parameter, get the path argument

		// change to the user's home directory if no argument is given
		if(path == NULL){
			path = getenv("HOME");
		}

		// change to the user's home directory if the argument is ~
		else if(strcmp(path, "~") == 0){
			path = getenv("HOME");
		}

		// successfully changed directory
		 if(chdir(path) == 0) return 1;

		 else{
		 	fprintf(stderr, "Error! Unknown directory: %s\n", path);
            return 0;
		 }
	}

    int status, fd, pipe_fd[2];
    pid_t pid, pid2;

    pid = fork(); // create the child process
    if (pid < 0) {
    	// if cannot create the child process, print the error message
        fprintf(stderr, "Error: Unable to fork.\n");
        exit(EXIT_FAILURE);

    }

    else if(pid == 0){ // execute command in child process
        if(info.type == PIPE){
            if(pipe(pipe_fd) < 0){
                fprintf(stderr, "Error: Unable to pipe.\n");
                exit(EXIT_FAILURE);
            }

            pid2 = fork(); // create the child process again
            if (pid2 < 0) {
            	// if cannot create the child process, print the error message
                fprintf(stderr, "Error: Unable to fork.\n");
                exit(EXIT_FAILURE);
            }

            else if(pid2 == 0){  // execute command in second child process
                close(pipe_fd[0]);
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);

				// if the command does not exist, print the error message
                if(execvp(argv[0], argv) == -1){
                    fprintf(stderr, "Error: \"%s\" command not found\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
            }

            else{
            	// execute the first pipeline command in the parent process
                waitpid(pid2, &status, 0);
                close(pipe_fd[1]);
                dup2(pipe_fd[0], STDIN_FILENO);
                close(pipe_fd[0]);

				// if the command does not exist, print the error message
                if(execvp(info.pipe_args[0], info.pipe_args) == -1){
                    fprintf(stderr, "Error: \"%s\" command not found\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
        }

        else{ // IN_REDIRECT, OUT_REDIRECT

            if(info.type == IN_REDIRECT){
                execute_inredirect(argv, info);
            }

            else if(info.type == OUT_REDIRECT){
                execute_outredirect(argv, info);
            }

			// if the command does not exist, print the error message
            if(execvp(argv[0], argv) == -1){
                fprintf(stderr, "Error: \"%s\" command not found\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }
    else{ // parent process

    	// if running in the background, print the PID of the child process.
        if(info.background == 1)
            fprintf(stdout, "[PID]: %u\n", pid);
        else
            waitpid(pid, &status, 0);  // wait the child process finish
    }
    return 1;
}

void execute_inredirect(char **argv, ParseInfo info){
	int fd = open(info.file, O_RDONLY);

	// if the file can not open, print the error message
    if (fd == -1) {
        fprintf(stderr, "Error: Unable to open file \"%s\" for input redirection.\n", info.file);
        exit(EXIT_FAILURE);
    }

    // execute the inredirect
    else{
		dup2(fd, STDIN_FILENO);
		close(fd);
    }
}

void execute_outredirect(char **argv, ParseInfo info){
	// When a file is opened, 0666 means that the file will be set with read-write permissions,
	// which means that all users can read and write to the file
	int fd = open(info.file, O_WRONLY | O_CREAT | O_TRUNC, 0666);

	// if the file can not open, print the error message
    if (fd == -1) {
        fprintf(stderr, "Error: Unable to open file \"%s\" for output redirection.\n", info.file);
        exit(EXIT_FAILURE);
    }

	// execute the outredirect
	else{
        dup2(fd, STDOUT_FILENO);
        close(fd);
	}
}
