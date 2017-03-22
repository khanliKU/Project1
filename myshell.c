/**
 * myshell interface program
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LINE       	80 /* 80 chars per line, per command, should be enough. */
#define MAX_PATH_NO		20
#define MAX_PAT_LENGTH  200

int parseCommand(char inputBuffer[], char *args[],int *background);
void printPATH();

char* paths[MAX_PATH_NO];
int pathLenght;

int main(void)
{
	char inputBuffer[MAX_LINE]; 	        /* buffer to hold the command entered */
	int background;             	        /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2 + 1];	        /* command line (of 80) has max of 40 arguments */
	pid_t pid;            		/* process id of the child process */
	int status;           		/* result from execv system call*/
	int shouldrun = 1;
		
	int i, j, upper;

	char* pathEV = getenv("PATH");
	char* token;
	char path[MAX_PAT_LENGTH];
	char cwd[MAX_PAT_LENGTH];
	int success = 0;
   
	using_history();
	register HIST_ENTRY **histList;
	rl_readline_name = "myshell> ";

   /* get the first token */
   	token = strtok(pathEV, ":");
   	paths[1] = token;
   	pathLenght = 2;

   /* walk through other tokens */
   	while( token != NULL ) 
   	{
    	token = strtok(NULL, ":");
    	if (token != NULL)
    	{
    		paths[pathLenght] = token;
    		pathLenght++;
    	}
   	}

	while (shouldrun)
	{            		/* Program terminates normally inside setup */
	    background = 0;
		int result;

		memset(cwd,'\0',sizeof(cwd));
		getcwd(cwd,MAX_PAT_LENGTH);
		paths[0] = cwd;
		do
		{
			shouldrun = parseCommand(inputBuffer,args,&background);       /* get next command */
		} while (!args[0]);

	    if (strncmp(args[0], "exit", 4) == 0)
	    {
	      shouldrun = 0;     /* Exiting from myshell*/
	    }

	    if (shouldrun)
	    {
	    	//-----------------------------------------------------------------
	    	// get contents of path file
	    	if (strcmp(args[0],"$PATH") == 0)
	    	{
	    		printPATH();
	    	}
	    	// cd command implementation
	    	else if (strcmp(args[0],"cd") == 0)
	    	{
	    		if (chdir(args[1]) != 0 && args[1] != NULL)
	    		{
	    			printf("No such directory: %s\n", args[1]);
	    		}
	    	}
		    else if (strncmp(args[0],"history",7) == 0)
		    {
		    	histList = history_list();
	    		j = 0;
	    		while (histList[j] != NULL)
		    	{
		    		printf("%s\n", histList[j]->line);
	    			j++;
	    		}
	    	}
	    	else
	    	{
		    	pid = fork();
		    	if (pid < 0)
				{
					fprintf(stderr, "Fork Failed");
					return 1;
				}
				else if (pid == 0)
				{
					for (int pathIndex=0;pathIndex<pathLenght;pathIndex++)
			    	{
			    		setpgid(0,0);
			    		memset(path,'\0',sizeof(path));
			    		strcpy(path,paths[pathIndex]);
		    			strncat(path,"/",1);
			    		strncat(path,args[0],MAX_LINE);
						success = execv(path, args);
					}
					exit(0);
				}
				else
				{
				// wait or background
					if (!background)
					{
						waitpid(pid);
					}
					background = 0;
				}
			}
	    }
	}
	return 0;
}

/** 
 * The parseCommand function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings. 
 */

int parseCommand(char inputBuffer[], char *args[],int *background)
{
    int length,			/* # of characters in the command line */
      ct;	        	/* index of where to place the next parameter into args[] */

	char* token;

    ct = 0;
	
    /* read what the user enters on the command line */
    do
    {
		fflush(stdout);
		inputBuffer = readline("myshell> ");
		length = strlen(inputBuffer);
    } while (!inputBuffer[0] || inputBuffer[0] == '\n'); /* swallow newline characters */
	
	add_history(inputBuffer);
	
	// get rid off &
    if (inputBuffer[length-1] == '&')
    {
    	*background = 1;
    	inputBuffer[length-1] = '\0';
    }

    if (length == 0)
    {
    	exit(0);            /* ^d was entered, end of user command stream */
    }
    
    /** 
     * the <control><d> signal interrupted the read system call 
     * if the process is in the read() system call, read returns -1
     * However, if this occurs, errno is set to EINTR. We can check this  value
     * and disregard the -1 value 
     */

    if ( (length < 0) && (errno != EINTR) )
    {
	    perror("error reading the command");
	    exit(-1);           /* terminate with error code of -1 */
    }
    
    token = strtok(inputBuffer, " \t");
   	ct = 0;
   /* walk through other tokens */
   	while( token != NULL ) 
   	{
    	args[ct] = token;
    	ct++;
    	token = strtok(NULL, " \t");
   	}

    args[ct] = NULL; /* just in case the input line was > 80 */
    return 1;
} /* end of parseCommand routine */

void printPATH()
{
	for (int i=0;i<pathLenght;i++)
   	{
   		printf( "%s\n", paths[i] );
   	}
}