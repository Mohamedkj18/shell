#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>

int process_arglist(int count, char **arglist);
int prepare(void);
int finalize(void);
int single_piping(char **arglist, int symbol_idx);

int process_arglist(int count, char **arglist)
{
	int i, symbol_idx, fileID;
	pid_t id;
	char *symbol, *filename;

	// find if the arg list contains a symbol
	symbol_idx = -1;
	symbol = "";
	for (i = count - 1; i >= 0; i--)
	{
		if (strcmp(arglist[i], "&") == 0 || strcmp(arglist[i], "|") == 0 || strcmp(arglist[i], "<") == 0 || strcmp(arglist[i], ">") == 0)
		{
			symbol = arglist[i];
			symbol_idx = i;
			arglist[symbol_idx] = NULL;
			break;
		}
	}

	if (strcmp(symbol, "&") == 0)
	{
		if ((id = fork()) == -1)
		{
			perror("Error in forking");
			return 0;
		}
		if (id == 0)
		{
			if (signal(SIGCHLD, SIG_DFL) == SIG_ERR)
			{
				perror("Error in SIGCHLD handling");
				exit(1);
			}

			execvp(arglist[0], arglist);
			perror("Error in executing the command");
			exit(1);
		}

		return 1;
	}
	else if (strcmp(symbol, "|") == 0)
	{
		if (!single_piping(arglist, symbol_idx))
		{
			perror("Error in piping");
			return 0;
		}
	}
	else if (strcmp(symbol, "<") == 0)
	{
		filename = arglist[symbol_idx + 1];

		if ((id = fork()) == -1)
		{
			printf("Error in forking");
			return 0;
		}
		if (id == 0)
		{
			if (signal(SIGINT, SIG_DFL) == SIG_ERR)
			{
				perror("Error in SIGINT handling");
				exit(1);
			}
			if (signal(SIGCHLD, SIG_DFL) == SIG_ERR)
			{
				perror("Error in SIGCHLD handling");
				exit(1);
			}
			if ((fileID = open(filename, O_RDONLY)) < 0)
			{
				perror("Error in openning the file");
				exit(1);
			}
			if (dup2(fileID, STDIN_FILENO) == -1)
			{
				perror("Error in dup2\n");
				close(fileID);
				exit(1);
			}

			close(fileID);

			execvp(arglist[0], arglist);
			perror("Error in executing the command");
			exit(1);
		}
		else
		{
			waitpid(id, NULL, 0);
			return 1;
		}
	}
	else if (strcmp(symbol, ">") == 0)
	{
		filename = arglist[symbol_idx + 1];

		if ((id = fork()) == -1)
		{
			perror("Error in forking");
			return 0;
		}
		if (id == 0)
		{
			if (signal(SIGINT, SIG_DFL) == SIG_ERR)
			{
				perror("Error SIGINT handling");
				exit(1);
			}
			if (signal(SIGCHLD, SIG_DFL) == SIG_ERR)
			{
				perror("Error SIGCHLD handling");
				exit(1);
			}
			if ((fileID = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
			{
				perror("Error in openning the file");
				exit(1);
			}
			if (dup2(fileID, STDOUT_FILENO) == -1)
			{
				perror("Error in dup2\n");
				close(fileID);
				exit(1);
			}

			close(fileID);

			execvp(arglist[0], arglist);
			perror("Error in executing command");
			exit(1);
		}

		waitpid(id, NULL, 0);
		return 1;
	}
	else
	{
		if ((id = fork()) == -1)
		{
			perror("Error in forking");
			return 0;
		}
		if (id == 0)
		{
			if (signal(SIGINT, SIG_DFL) == SIG_ERR)
			{
				perror("Error SIGINT handling");
				exit(1);
			}
			if (signal(SIGCHLD, SIG_DFL) == SIG_ERR)
			{
				perror("Error in SIGCHLD handling");
				exit(1);
			}
			execvp(arglist[0], arglist);
			perror("Error in Executing command");
			exit(1);
		}

		waitpid(id, NULL, 0);
		return 1;
	}

	return 1;
}

int single_piping(char **arglist, int symbol_idx)
{
	int fd[2];
	pid_t id1, id2;

	if (pipe(fd) == -1)
	{
		perror("Error in piping");
		return 0;
	}

	if ((id1 = fork()) == -1)
	{
		perror("Error in forking\n");
		exit(1);
	}
	if (id1 == 0)
	{
		// First child process
		if (signal(SIGINT, SIG_DFL) == SIG_ERR)
		{
			perror("Error in SIGINT handling");
			exit(1);
		}
		if (signal(SIGCHLD, SIG_DFL) == SIG_ERR)
		{
			perror("Error in SIGCHLD handling");
			exit(1);
		}

		close(fd[0]); // we won't be reading anything

		// direct the output to the pipe
		if (dup2(fd[1], STDOUT_FILENO) == -1)
		{
			perror("Error in dup2\n");
			exit(1);
		}

		close(fd[1]);

		execvp(arglist[0], arglist);
		perror("Error executing first command");
		exit(1);
	}

	if ((id2 = fork()) == -1)
	{
		perror("Error in forking\n");
		exit(1);
	}

	if (id2 == 0)
	{
		// Second child process
		if (signal(SIGINT, SIG_DFL) == SIG_ERR)
		{
			perror("Error in SIGINT handling");
			exit(1);
		}
		if (signal(SIGCHLD, SIG_DFL) == SIG_ERR)
		{
			perror("Error in SIGCHLD handling");
			exit(1);
		}

		close(fd[1]); // we won't be writing anything

		// direct the input to the pipe
		if (dup2(fd[0], STDIN_FILENO) == -1)
		{
			perror("Error in dup2");
			exit(1);
		}

		close(fd[0]);
		execvp(arglist[symbol_idx + 1], &arglist[symbol_idx + 1]);
		perror("Error executing second command");
		exit(1);
	}

	// only the parent process should get here
	close(fd[0]);
	close(fd[1]);

	waitpid(id1, NULL, 0);
	waitpid(id2, NULL, 0);

	return 1; // successefully finished the process
}

int prepare(void)
{
	if (signal(SIGINT, SIG_IGN) == SIG_ERR)
	{
		perror("Error in SIGINT handling");
		return -1;
	}
	if (signal(SIGCHLD, SIG_IGN) == SIG_ERR)
	{
		perror("Error in SIGCHLD handling");
		return -1;
	}
	return 0;
}

int finalize(void)
{
	return 0;
}
