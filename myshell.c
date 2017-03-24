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
#include <dirent.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sched.h>

#define MAX_LINE       	80 /* 80 chars per line, per command, should be enough. */
#define MAX_PATH_NO		20
#define MAX_PAT_LENGTH  200
#define MAX_BOOKMARK	10
#define MAX_TASK		20

int parseCommand(char inputBuffer[],
					char *args[],
					int *background,
					int *argumentCount,
					int *bookmark);
void printPATH();
void codeSearch(const char *name, int isRecursive, const char *searchFor);
char *trimwhitespace(char *str);
int removeModule();
int insertCron(char* hour, char* min, char* exe, char* file);
int removeCron(char* hour, char* min);
int printCron();

char* paths[MAX_PATH_NO];
char* bookmarks[MAX_BOOKMARK] = {NULL};
int pathLenght;

struct muzik_task
{
	char m[3];
	char h[3];
	char filePath[MAX_PAT_LENGTH];
};

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
	int argumentCount;
	int bookmark = -1;
	int currentPID = -1;

	const char* user = getenv("USER");

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

	// get current saved crontabs
	system("/usr/bin/crontab -l > mycron");

	while (shouldrun)
	{            		/* Program terminates normally inside setup */
	    background = 0;
		int result;

		memset(cwd,'\0',sizeof(cwd));
		getcwd(cwd,MAX_PAT_LENGTH);
		paths[0] = cwd;
		do
		{
			shouldrun = parseCommand(inputBuffer,
										args,
										&background,
										&argumentCount,
										&bookmark);       /* get next command */
		} while (!args[0]);

	    if (strncmp(args[0], "exit", 4) == 0)
	    {
	      shouldrun = 0;     /* Exiting from myshell*/
//	    	removeModule();
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
	    	else if (strcmp(args[0],"bookmark") == 0 && argumentCount > 1)
	    	{
	    		// list bookmarks
	    		if (strcmp(args[1],"-l") == 0)
	    		{
	    			for (int k = 0; k < MAX_BOOKMARK; k++)
	    			{
	    				if (bookmarks[k] == NULL)
	    				{
	    					break;
	    				}
	    				printf("%d: %s\n",k, bookmarks[k]);
	    			}
	    		}
	    		// execute at index
	    		else if (strcmp(args[1],"-i") == 0 && argumentCount > 2)
	    		{
	    			j = atoi(args[2]);
	    			if (j >= 0 && j < MAX_BOOKMARK && bookmarks[j] != NULL)
					{
						bookmark = j;
					}
	    		}
	    		// delete at index
	    		else if (strcmp(args[1],"-d") == 0 && argumentCount > 2)
	    		{
	    			j = atoi(args[2]);
					if (j >= 0 && j < MAX_BOOKMARK)
					{
						bookmarks[j] = NULL;
					}
				}
				// inser bookmark
	    		else
	    		{
	    			for (int k = 0; k < MAX_BOOKMARK; k++)
	    			{
	    				if (bookmarks[k] == NULL)
	    				{
	    					bookmarks[k] = args[1];
	    					break;
	    				}
	    			}
//					printf("Bookmarks is full, can't insert.\n");
	    		}
	    	}
			else if (strcmp(args[0],"codesearch") == 0)
			{
				if (args[1] != NULL)
				{
					if (strcmp(args[1],"-r") == 0 && args[2] != NULL)
					{
						codeSearch(".",1,args[2]);
					}
					else
					{
						codeSearch(".",0,args[1]);
					}
				}
			}
			else if (strcmp(args[0],"muzik") == 0)
			{
				if (args[1] != NULL)
				{
					if (strcmp(args[1],"-l") == 0)
					{
						printCron();
					}
					else if (strcmp(args[1],"-r") == 0 && args[2] != NULL)
					{
						token = strtok(args[2],".");
						if (token != NULL && strlen(token) < 3)
						{
							char m[3];
							char h[3];
							j = 0;
							strcpy(h,token);
							token = strtok(NULL,".");
							strcpy(m,token);
							removeCron(h,m);
						}
					}
					else if (argumentCount > 2)
					{
						char *h,*m;
						h = strtok(args[1],".");
						if (h != NULL && strlen(h) < 3)
						{
							m = strtok(NULL,".");
							insertCron(h,m,"/usr/bin/play",args[2]);
						}
					}

					pid = fork();
					if (pid < 0)
					{
						
					}
					else if (pid == 0)
					{
						char* dum[3] = {"/usr/bin/crontab","./mycron",NULL};
						execv(dum[0],dum);
					}
					else
					{
						waitpid(pid);
					}
				}
			}
			else if (strcmp(args[0],"processInfo") == 0)
			{
				if (argumentCount > 2)
				{ // 2337
					pid = fork();
					if (pid < 0)
					{
						fprintf(stderr, "Fork Failed");
						return 1;
					}
					else if (pid == 0)
					{
						if (currentPID != atoi(args[1]))
						{
							if (currentPID != -1)
							{
								removeModule();
							}
							char dum1[50] = "processID=";
							char dum2[50] = "processPrio=";
							strcat(dum1,args[1]);
							strcat(dum2,args[2]);
							args[0] = "/usr/bin/sudo";
							args[1] = "insmod";
							args[2] = "processInfo.ko";
							args[3] = dum1;
							args[4] = dum2;
							args[5] = NULL;
							execv(args[0], args);
						}
						exit(0);
					}
					else
					{
						waitpid(pid);
						currentPID = atoi(args[1]);
					}
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
						execv(path, args);
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
	if (currentPID != -1)
	{
		removeModule();
	}
	return 0;
}

/** 
 * The parseCommand function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings. 
 */

int parseCommand(char inputBuffer[],
					char *args[],
					int *background,
					int *argumentCount,
					int *bookmark)
{
    int length,			/* # of characters in the command line */
      ct;	        	/* index of where to place the next parameter into args[] */

	char* token;
	char* quoteToken;
	char* arguments;

    ct = 0;
	
    /* read what the user enters on the command line */
    if (*bookmark == -1)
    {
		do
		{
			inputBuffer = readline("myshell> ");
		} while (!inputBuffer[0] || inputBuffer[0] == '\n'); /* swallow newline characters */
		add_history(inputBuffer);
    }
    else
    {
    	strcpy(inputBuffer, bookmarks[*bookmark]);
    	*bookmark = -1;
    }
	
	length = strlen(inputBuffer);
	
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
    
	arguments = strtok(inputBuffer,"\"");
	quoteToken = strtok(NULL,"\"");
    token = strtok(arguments, " ");
   	ct = 0;
   /* walk through other tokens */
   	while( token != NULL ) 
   	{
    	args[ct] = token;
    	ct++;
    	token = strtok(NULL, " ");
   	}
   	if (quoteToken != NULL)
   	{
   		args[ct] = quoteToken;
   		ct++;
   	}
   	*argumentCount = ct;
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

void codeSearch(const char *currentDir, int isRecursive, const char *searchFor)
{
    DIR *dir;
    struct dirent *entry;
    char* token;
    char dummy[MAX_PAT_LENGTH];
    char path[1024];
    FILE* fp;
    char line[MAX_LINE];
    int lineNumber, lineStart = *line;

    if (!(dir = opendir(currentDir)))
        return;
    if (!(entry = readdir(dir)))
        return;

    do
    {
    	int len = snprintf(path, sizeof(path)-1, "%s/%s", currentDir, entry->d_name);
    	path[len] = 0;
    	lineNumber = 1;
//		printf("%s\n", path);
    	// if its a dir
        if (entry->d_type == DT_DIR)
        {
            if (strcmp(entry->d_name, ".") == 0 ||		// ignore "."
            	strcmp(entry->d_name, "..") == 0 ||		// ignore ".."
            	strcmp(entry->d_name, ".git") == 0)		// ignore ".git"
                continue;
            if (isRecursive)
            {
            	codeSearch(path, isRecursive, searchFor);
            }
			
        }
        // if its a file
        else
        {
        	strcpy(dummy,entry->d_name);
        	token = strtok(dummy,".");
        	if (token != NULL)
        	{
        		token = strtok(NULL,".");
        		if (token != NULL)
	        	{
	        		if (strcmp(token,"c") == 0 ||
	        			strcmp(token,"C") == 0 ||
	        			strcmp(token,"h") == 0 ||
	        			strcmp(token,"H") == 0 ||
	        			strcmp(token,"cpp") == 0 ||
	        			strcmp(token,"c++") == 0)
	        		{
//						printf("%s\n", token);
	        			fp = fopen(path,"r");
	        			while(fgets(line,sizeof(line),fp))
	        			{
	        				if (strstr(line,searchFor) != NULL)
	        				{
	        					printf("%s -> %d: %s", path, lineNumber, trimwhitespace(line));
	        					*line = lineStart;
							}
							lineNumber++;
	        			}
	        			fclose(fp);
						
	        		}
	        	}
        	}
			
        }
    } while (entry = readdir(dir));
    closedir(dir);
}

int isIndent(char c)
{
	if (c == ' ' || c == '\t')
		return 1;
	return 0;
}

char *trimwhitespace(char *str)
{
	char *end;
	// Trim leading space
	while(isIndent((unsigned char)*str)) str++;
	// All spaces?
	if(*str == 0)
		return str;
	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isIndent((unsigned char)*end)) end--;
	// Write new null terminator
	*(end+1) = 0;
	return str;
}

int removeModule()
{
	int pid = fork();
	char* args[4] = {"/usr/bin/sudo","rmmod","processInfo",NULL};
	if (pid < 0)
	{
		printf("Fork Failed");
		return 1;
	}
	else if (pid == 0)
	{
		execv(args[0],args);
	}
	else
	{
		waitpid(pid);
	}
	return 0;
}

int insertCron(char* hour, char* min, char* exe, char* file)
{
	FILE *fp1, *fp2;
	char h[20], m[20], d[20], mo[20], w[20], e[MAX_PAT_LENGTH], f[MAX_PAT_LENGTH];
	fp1 = fopen("mycron","r");
	fp2 = fopen("mycronnew","wr+");
	if (fp1 && fp2)
	{
		while(1)
		{
			if(fscanf(fp1, "%s %s %s %s %s %s %s\n",m, h, d, mo, w, e, f) == EOF)
			{
			 break;
			}
			fprintf(fp2, "%s %s %s %s %s %s %s\n",m, h, d, mo, w, e, f);
		}
		fprintf(fp2, "%s %s %s %s %s %s %s\n",min, hour, "*", "*", "*", exe, file);
//		printf("%s %s %s %s\n", min, hour, exe, file);
		fscanf(fp2, "%s %s %s %s %s %s %s\n",m, h, d, mo, w, e, f);
//		printf("%s %s %s %s %s %s %s\n",m, h, d, mo, w, e, f);
		fclose(fp1);
		fclose(fp2);
		remove("mycron");
		rename("mycronnew","mycron");
	}
	return 0;
}

int removeCron(char* hour, char* min)
{
	FILE *fp1, *fp2;
	char h[20], m[20], d[20], mo[20], w[20], e[MAX_PAT_LENGTH], f[MAX_PAT_LENGTH];
	fp1 = fopen("mycron","r");
	fp2 = fopen("mycronnew","w+");
	if (fp1 && fp2)
	{
		while(1)
		{
			if(fscanf(fp1, "%s %s %s %s %s %s %s\n",m, h, d, mo, w, e, f) == EOF)
			{
			 break;
			}
			if (strcmp(m,min) != 0 || strcmp(h,hour) != 0)
			{
				fprintf(fp2, "%s %s %s %s %s %s %s\n",m, h, d, mo, w, e, f);
			}
		}
		fclose(fp1);
		fclose(fp2);
		remove("mycron");
		rename("mycronnew","mycron");
	}
	return 0;
}

int printCron()
{
	FILE *fp1;
	char h[20], m[20], d[20], mo[20], w[20], e[MAX_PAT_LENGTH], f[MAX_PAT_LENGTH];
	fp1 = fopen("mycron","rw+");
	if (fp1)
	{
		while(1)
		{
			if(fscanf(fp1, "%s %s %s %s %s %s %s\n",m, h, d, mo, w, e, f) == EOF)
			{
			 break;
			}
			printf( "%s %s %s %s %s %s %s\n",m, h, d, mo, w, e, f);
		}
		fclose(fp1);
	}
	return 0;
}