/*
 * Author: Fowler, Tyler
 * Uvic ID: V00752565
 * Course: CSC 360
 * Assignment: 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_PROMPT_SIZE 11
#define MAX_LINE_LEN 81
#define MAX_ARGS 8
#define MAX_PATH 11

#define stripnl(str) {\
			if (str[strlen(str) - 1] == '\n') {\
				str[strlen(str) - 1] = '\0';\
			}\
}
#define fexists(pth) (!access(pth, X_OK))

int init(char *prompt);
int readline(char *input);
int prsinpt(char *input, char **argv);
int path(char *bin, char *cmdname);
int execor(char *argv[], char *envp[], char *outfile);
char *prsor(char **cmd_argv, char **old_argv);
int execpp2(char **cmd_1, char **cmd_2, char *envp[]);
int execpp3(char **cmd_1, char **cmd_2, char **cmd_3, char *envp[]);
int getcmd(char **argv, char **old_argv);
int prspp(char **cmd_1, char **cmd_2, char **cmd_3, char **argv);

char PATH[MAX_ARGS][MAX_LINE_LEN];

/**
 * This shell program repeatedly prompt the user for commands and
 * executes those commands in a child process.
 */
int main() {
	char prompt[MAX_PROMPT_SIZE];
	char *argv[MAX_ARGS+3];
	char *envp[] = { NULL };

	/* Initialize prompt and PATH */
	init(prompt);

	/* Repeatedly: */
	for (;;) {
		/* Prompt the user. */
		fprintf(stdout, "%s", prompt);
		fflush(stdout);

		/* Read input from the user. */
		char input[MAX_LINE_LEN];
		assert(readline(input) < MAX_LINE_LEN);
		stripnl(input);
		if (strcmp(input, "exit") == 0) {
			break;
		}

		/* Process command input. */
		if (prsinpt(input, argv) < 0) continue;

		/* Check for shell commands. */
		if (strcmp(argv[0], "OR") == 0) {
			char *cmd_argv[MAX_ARGS];
			char *outfile;

			/* Parse shell input to get the command to be run. */
			if ((outfile = prsor(cmd_argv, argv)) == NULL) continue;

			/* Execute the command. */
			execor(cmd_argv, envp, outfile);
		}
		else if (strcmp(argv[0], "PP") == 0) {
			char *cmd_1[MAX_ARGS], *cmd_2[MAX_ARGS], *cmd_3[MAX_ARGS];
			int num_cmd;

			/* Parse input to get commands. */
			if ((num_cmd = prspp(cmd_1, cmd_2, cmd_3, argv)) < 0) continue;

			/* Execute the commands. */
			if(num_cmd == 2) 		execpp2(cmd_1, cmd_2, envp);
			else if (num_cmd == 3) 	execpp3(cmd_1, cmd_2, cmd_3, envp);
		}
		else {  /* Execute a single command. */
			int pid, status;
			
			/* Search for binary file of the command. */
			char filepath[MAX_LINE_LEN];
			if (path(filepath, argv[0]) < 0) continue;
			
			/* Execute user command. */
			if((pid = fork()) == 0) {
				execve(filepath, argv, envp);
			}

			waitpid(pid, &status, 0); /* Wait for child. */
		}
	}
	return 0;
}

/**
 * This function initializes the shell with the ".sh360rc" run command file.
 * It will initialize the global variable "PATH" with the directories included
 * beneath the prompt in the run command file.
 * 
 * Parameters:
 * prompt  - a char * that will be initialized with the prompt to be
 *			 displayed to the user.
 */
int init(char *prompt) {
	/* Open run commands. */
	FILE *sh360rc = fopen(".sh360rc", "r");
	if (sh360rc == NULL) {
		fprintf(stderr, "Error opening run command file.\n");
		fprintf(stderr, ".sh360rc must be inlcuded in the programs directory.\n");
		exit(1);
	}
	/* Grab the prompt. */
	if(fgets(prompt, MAX_PROMPT_SIZE, sh360rc) == NULL) {
		fprintf(stderr, "Error getting prompt from \".sh360rc\".");
		exit(1);
	}
	stripnl(prompt);

	/* Grab the paths. */
	int i = 0;
	while (fgets(PATH[i], MAX_LINE_LEN, sh360rc) != NULL) {
		if (i == MAX_PATH - 1) {
			fprintf(stderr, "Too many paths in sh360rc file.\n");
			fprintf(stderr, "Taking the first %d paths.\n", MAX_PATH-1);
			break;
		}
		stripnl(PATH[i]);
		i++;
	}
	PATH[i][0] = '\0'; /* Null terminate the final entry. */
	
	fclose(sh360rc);  /* Close rc file. */
	return i;
}

/**
 * This function reads a line from the terminal and stores it in input.
 * readline returns the length of the input string.
 */
int readline(char *input) {
		if (fgets(input, MAX_LINE_LEN, stdin) == NULL) {
			fprintf(stderr, "Error getting from stdin.");
			exit(1);
		}
		return strlen(input);
}

/**
 * This function uses strtok to tokanize the command given to the sh360 shell.
 * i.e "ls -la" initializes argv to {"ls", "-la"}. Returns -1 on error or the 
 * number of arguments not including '->' indicators.
 * 
 * Parameters:
 * input	- a char * containing the user input to the shell.
 * argv		- an array of char * that will be initialized
 * 			with pointers to the actual executable commands.
 */
int prsinpt(char *input, char *argv[]) {
	const char s[2] = " ";

	/* Walk through the tokens. */
	int i = 0, num_args = 0;
	for (char *tk = strtok(input, s); tk != NULL; tk = strtok(NULL, s)) {
		argv[i++] = tk;
		if (strcmp(argv[i-1], "->") != 0)	++num_args;
		if (num_args == MAX_ARGS) {
			fprintf(stderr, "Too many args. Max length: %d\n", MAX_ARGS - 1);
			return -1;
		}
	} /* Null terminate the argument list. */
	argv[i] = NULL;
	return num_args;
}

/**
 * This function returns the index to the path that contains the binary file
 * with the name stored in bin. If the file does not exist as an executable in
 * any of the provided paths, this function returns -1.
 */
int path(char *filepath, char *filename) {
	if (filename == NULL) {  /* Formating error. No command given. */
		fprintf(stderr, "Error: no command given.\n");
		return -1;
	}
	for (int i = 0; i < MAX_PATH; i++) {
		if (PATH[i][0] == 0) {  /* End of PATH, executable not found. */
			fprintf(stderr, "%s could not be found in the PATH.\n", filename);
			return -1;
		}
		/* Build the path. */
		char pth[MAX_LINE_LEN];
		strncpy(pth, PATH[i], MAX_LINE_LEN);
		strcat(pth, "/");
		strcat(pth, filename);

		if (fexists(pth)) {  /* Binary file exists in path i */
			strncpy(filepath, pth, MAX_LINE_LEN);
			return i;
		}
	}
	return -1;
}

/**
 * This function execues commands that require output redirection. It copies
 * the argument parameter into a new array, skipping the tokens that pertain
 * only to the OR shell command. Then it redirects the output and finally,
 * executes the command in a new process.
 * 
 * Parameters:
 * argv		- An array of char* containing all the tokanized arguments
 * 			given as the shell command.
 * envp 	- The environment for running the command. For this
 * 			assignment, it will always contain a single NULL pointer.
 * filepath - A string containing the path to the executable file.
 * outfile  - A string containing the name of the output file.
 * 
 * Returns 0 for successfully executing the command and -1 if there was an error.
 */
int execor(char *argv[], char *envp[], char *outfile) {
	char filepath[MAX_LINE_LEN];
	int pid, fd, status;

	/* Get the filepath. */
	if (path(filepath, argv[0]) < 0) return -1;
	
	/* Save a copy stdout. */
	int sdout = dup(1);

	/* Redirect output and execute command in a new process. */
	if ((pid = fork()) == 0) {
		if ((fd = open(outfile, O_TRUNC|O_CREAT|O_RDWR, S_IRUSR|S_IWUSR)) < 0) {
			fprintf(stderr, "Error opening output file.\n");
			return -1;
		}
		dup2(fd, 1);
		close(fd);
		execve(filepath, argv, envp);
		fprintf(stderr, "Error: returned from execve in execor.\n");
		return -1;
	}
	/* Reset default stdout. */
	dup2(sdout, 1);
	close(sdout);

	waitpid(pid, &status, 0); /* Wait for child. */

	return 0;
}

/**
 * This function parses an [OR] command to separate the shell command input
 * from the execution command input. It returns a char * indicating the name
 * of the file for output redirection.
 */
char *prsor(char **cmd_argv, char **old_argv) {
	int argc;
	char *outfile = NULL;
	
	if (*++old_argv == NULL) {  /* Formatting error. No command. */
		fprintf(stderr, "[OR] commands require an executable name to be given.\n");
		return NULL;
	}
	for (argc = 0; *old_argv != NULL; old_argv++) {
		if (strcmp(*old_argv, "->") == 0) {  /* Get the output file. */
			if(*++old_argv == NULL) {  /* Check for missing outfile name. */
				fprintf(stderr,
						"Please provide a filename for output redirection [OR]."
						);
				fprintf(stderr, " The filename must follow '->'.\n");
				return NULL;
			}
			outfile = *old_argv;
			continue;
		}
		else  /* Copy only the executable command arguments. */
			cmd_argv[argc++] = *old_argv;
	}  /* Null terminate the argument list. */
	cmd_argv[argc] = NULL;

	if (outfile == NULL) {  /* Formatting error: No -> indicator for redirection. */
			fprintf(stderr, "For output redirection [OR], please provide a");
			fprintf(stderr, " '->' indicator followed by the output filename.\n");
			return NULL;
	}
	return outfile;
}

/* Following are 2 functions that execute commands that require piping. They
 * pipe the output of of the first command into that of the second command.
 * Ditto for the possible second to the possible third.
 * 
 * Parameters:	
 * cmd_1/2/3	- These are the argument lists for each command to be piped
 * path_1/2/3	- These are the absolute paths to 
 * 				envp 	 -	The environment for running the command. For this
 * 							assignment, it will always contain a single NULL pointer.
 */
int execpp2(char **cmd_1, char **cmd_2, char *envp[]) {
	char path_1[MAX_LINE_LEN], path_2[MAX_LINE_LEN];
	int pid, status;

	/* Get the paths. */
	if (path(path_1, cmd_1[0]) < 0) return -1;
	if (path(path_2, cmd_2[0]) < 0) return -1;

	/* Set up pipe. */
	int fd[2];
	if (pipe(fd) < 0) {
		fprintf(stderr, "Error: pipe.\n");
		return -1;
	}
	/* Ready to execute. Make copies of STDIN and STDOUT. */
	int stdindup = dup(0);
	int stdoutdup = dup(1);

	/* Execute the first command. */
	if ((pid = fork()) == 0) {
		/* Child P1: Reroute stdin to pipe. */
		dup2(fd[1], 1);
		close(fd[0]);
		close(fd[1]);
		execve(path_1, cmd_1, envp);
		fprintf(stderr, "Error: returned from execve in execpp2 P1.\n");
		return -1;
	}
	/* Execdute the second command. */
	if ((pid = fork()) == 0) {
		/* Child P2: Reroute pipe to stdout. */
		dup2(fd[0], 0);
		close(fd[0]);
		close(fd[1]);
		execve(path_2, cmd_2, envp);
		fprintf(stderr, "Error: returned from execve in execpp2 P2.\n");
		return -1;
	}
	/* Close file descriptors in the parent process. */
	close(fd[0]);
	close(fd[1]);

	/* Reset defaults. */
	dup2(stdindup, 0);
	dup2(stdoutdup, 1);
	close(stdindup);
	close(stdoutdup);
	
	/* Wait for final command to finish. */
	waitpid(pid, &status, 0); 
	return 0;
}

/* This is the second execpp function for piping together 3 commands. */
int execpp3(char **cmd_1, char **cmd_2, char **cmd_3, char *envp[]) {
	char path_1[MAX_LINE_LEN];
	char path_2[MAX_LINE_LEN];
	char path_3[MAX_LINE_LEN];
	int pid, status;

	/* Set up the paths. */
	if (path(path_1, cmd_1[0]) < 0) return -1;
	if (path(path_2, cmd_2[0]) < 0) return -1;
	if (path(path_3, cmd_3[0]) < 0) return -1;

	/* Set up pipe A. */
	int fda[2];
	if (pipe(fda) < 0) {
		fprintf(stderr, "Error: pipe.\n");
		return -1;
	}
	/* Ready to execute. Make copies of STDIN and STDOUT */
	int stdindup = dup(0);
	int stdoutdup = dup(1);

	/* Execute first command. */
	if ((pid = fork()) == 0) {
		/* Child P1: reroute stdout to pipe A. */
		dup2(fda[1], 1);
		close(fda[0]);
		close(fda[1]);
		execve(path_1, cmd_1, envp);
	}
	/* Set up pipe B. */
	int fdb[2];
	if (pipe(fdb) < 0) {
		fprintf(stderr, "Error: pipe.\n");
		return -1;
	}
	/* Execute second command. */
	if ((pid = fork()) == 0) {
		/* Child P2: Re-route pipe A to stdin. Re-route stdout to pipe B */
		dup2(fda[0], 0);
		dup2(fdb[1], 1);
		close(fda[0]);
		close(fda[1]);
		close(fdb[0]);
		close(fdb[1]);
		execve(path_2, cmd_2, envp);
	}
	/* Close pipe A. */
	close(fda[0]);
	close(fda[1]);

	/* Execute third command. */
	if ((pid = fork()) == 0) {
		/* Child P3: reroute pipe B to stdin. */
		dup2(fdb[0], 0);
		close(fdb[0]);
		close(fdb[1]);
		execve(path_3, cmd_3, envp);
	}
	/* Close pipe B. */
	close(fdb[0]);
	close(fdb[1]);

	/* Reset defaults. */
	dup2(stdindup, 0);
	dup2(stdoutdup, 1);
	close(stdindup);
	close(stdoutdup);
	
	/* Wait for final command to finish. */
	waitpid(pid, &status, 0); 
	return 0;
}

/**
 * This function parses the shell command to fetch the 2 or 3 commands 
 * in a PP request.
 * 
 * Parameters:
 * cmd_1/2/3	- The buffers for the argument list of each command.
 * 				cmd_3 may be uninitialized following this function.
 * argv			- The command input given to the shell. 
 */
int prspp(char **cmd_1, char **cmd_2, char **cmd_3, char **argv) {
	int argc, ind = 0;
	
	if (*++argv == NULL || strcmp(*argv, "->") == 0) {
		/* Formatting error. No commands. */
		fprintf(stderr, 
				"Error: [PP] must be followed by an executable command.\n"
				);
		return -1;
	}
	/* Get the first command. */
	if ((argc = getcmd(cmd_1, argv)) < 0) return -1;
	if (argc == 0) {  /* Formatting error. Pipe requires at least one -> */
		fprintf(stderr,
				"Error: [PP] must have a second command following a '->' indicator.\n"
				);
		return -1;
	}
	ind += argc;

	/* Get the second command. */
	if((argc = getcmd(cmd_2, argv + ind)) < 0) return -1;
	if (argc == 0) return 2;  /* Done 2 commands. */
	ind += argc;
	
	/* Get the third command. */
	if((argc = getcmd(cmd_3, argv + ind)) < 0) return -1;
	if (argc != 0) {  /* Too many commands. */
		fprintf(stderr, "Error: [PP] only supports up to 3 commands.\n");
		return -1;
	}
	return 3;
}

/**
 * This function takes destination argument list and a shell argument.
 * It copies the arguments up until the end of the list or the next
 * '->' indicator. It returns the index of the '->' next indicator.
 */
int getcmd(char **argv, char **old_argv) {
	if (*old_argv == NULL) {
		fprintf(stderr,
				"Error: Hanging '->' must be followed by an executable command.\n"
				);
		return -1;
	}
	int i;
	for (i = 0; *old_argv != NULL; old_argv++) {
		if (strcmp(*old_argv, "->") == 0) {
			break;
		}
		argv[i++] = *old_argv;
	}
	argv[i++] = NULL;
	if (*old_argv == NULL) return 0; /* No more '->' indicators. */
	return i; 
}
