#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <stdbool.h> 
#include <fcntl.h> 
#include <readline/history.h>
#include <readline/readline.h>
#define PRMTSIZ 255
#define MAXARGS 63

char direct[1024];
int i = 0;
int numOfProcess = 0;
int process[1000] = { 0 };
bool showedMsgDone[1000] = { 0 };

typedef struct Command
{
	char* args[MAXARGS + 1];
	char *piping_args[MAXARGS+1];
	bool is_redirecting_out;
	bool is_redirecting_in;
	bool is_piping;	
	char* out_file;
	char *in_file;
	bool is_cd;
	bool is_history;
	bool is_run_async;
} Command;
 
void reset_command(Command *command)
{
 
	int i=0;
	for (;i<MAXARGS;i++)
	{
		command->args[i]=NULL;
		command->piping_args[i]=NULL;
	}
	command->is_redirecting_out=false;
	command->is_redirecting_in=false;
	command->is_piping=false;
	command->out_file=NULL;
	command->in_file=NULL;
	command->is_cd=false;
	command->is_history=false;
	command->is_run_async=false;
}

void turn_on_redirecting_in(Command *command)
{
	command->is_redirecting_in=1;
}
void turn_on_redirecting_out(Command *command)
{
	command->is_redirecting_out=1;
}
void turn_on_piping(Command *command)
{	
	command->is_piping=1;
}
 
 
// s : include aguement, and may be '>', '<', '|' 
int get_index_and_set_redirect_or_pipe(Command *command,char *s[])
{
	int i=0;
	for (i=0;s[i]!=NULL;i++)
	{       
		if (strcmp(s[i],"cd")==0) {
			command->is_cd = true;
		}
		if (strcmp(s[i],"!!")==0) {
			command->is_history = true;
		}	

		if(strcmp(s[i],"<")==0)
		{
			turn_on_redirecting_in(command);
			command->in_file=s[i+1];
			return i;
		}
		else if(strcmp(s[i],">")==0)
		{
			turn_on_redirecting_out(command);
			command->out_file=s[i+1];
			return i;
		}
		else if(strcmp(s[i],"|")==0)
		{
			turn_on_piping(command);
			return i;
		}
	}
	// not thing (not redirecting, piping,...)
	return i;
 
}
void parse_line_to_tokens(Command *command, char *toks[],char *input)
{
	char* tok;
	tok = strtok(input," ");	
	int i = 0;
	while (tok) 
	{
		char *temp = strdup(tok);
		if (strcmp(temp, "&") == 0) 
		{
			command->is_run_async = true;
			break;
		}
		toks[i++] = strdup(tok);
		tok = strtok(NULL, " ");
	}
}
void parse_line_to_arguements(Command *command,char *line_input)
{
	char *toks[MAXARGS]={0};
	parse_line_to_tokens(command,toks,line_input);
	int index_of_redirecting_or_piping=get_index_and_set_redirect_or_pipe(command,toks);
	int i=0;
	for (i=0;i<index_of_redirecting_or_piping;i++)
	{
		command->args[i]=toks[i];
		printf("\"%s\"\n",toks[i]);	
	}
	// go to the token next to ">", or "<", or "|"
	i++;
	if (command->is_redirecting_out)
	{
		command->out_file=toks[i];
 
	}
	else if (command->is_redirecting_in)
	{	
		command->in_file=toks[i];
	}		
	else if (command->is_piping)
	{
		int j=i;
		for (;toks[i]!=NULL;i++)	
		{
			command->piping_args[i-j]=toks[i];
		}		
 
	}
}
void run_command(Command *command)
{
	if (command->is_redirecting_out)
	{
		int f = open(command->out_file,O_WRONLY|O_CREAT,0777); 	
		dup2(f,1);
		close(f);
		execvp(command->args[0], command->args);
	}
	else if (command->is_redirecting_in)
	{
		int f = open(command->in_file,O_RDWR,0777); 	
		dup2(f,0);	
		close(f);
		execvp(command->args[0], command->args);
 
	}
	else if (command->is_piping)
	{
		int child_val;
		int pipefd[2];
		pipe(pipefd);
		if (child_val=fork()==0)
		{
			dup2(pipefd[1],1);
			close(pipefd[0]);
			execvp(command->args[0], command->args);
		}
		else
		{
			dup2(pipefd[0],0);
			close(pipefd[1]);
			execvp(command->piping_args[0], command->piping_args);
		}
 
	}				
	else 
	{
		execvp(command->args[0], command->args);
	}
 
	reset_command(command);
}
int child_val;
Command old_command={0};

void checkProcessDone(Command *command) 
{ 
	int index = 0;
	int status;
	for (index; index < numOfProcess; index++) {
		int return_pid = waitpid(process[index], &status, WNOHANG);
		if ((return_pid == process[index]) && (showedMsgDone[index] == 0)) {
			printf("[%d] Done\n", index + 1);
			showedMsgDone[index] = 1;
			if ((index == numOfProcess - 1) && (command->is_run_async == false)) {
				numOfProcess = 0;
			}
		}
	}	
} 


void excute_process(Command *command)
{
	int cstatus;
	int c;
	if (command->is_run_async)
	{
		c = waitpid(child_val, &cstatus, 1);
		showedMsgDone[numOfProcess] = 0;
		process[numOfProcess] = child_val;
		numOfProcess += 1;
		printf("[%d] %d\n", numOfProcess, child_val);
	} else {
		c = waitpid(child_val, &cstatus, 0);
	}

	checkProcessDone(command);

}

int main(void) {
 
	while (1) {
		
		//char line[PRMTSIZ + 1] = { 0 };
		getcwd(direct, sizeof(direct));
		direct[0] = ' ';
		char path[100];
		sprintf(path,"[%s ]$ ", direct); 
		path[strlen(path)] = '\0';
		
		char *line = readline(path);
		add_history(line);
	
		// declare and init command
		Command command={0};
	
		parse_line_to_arguements(&command,line);
		// fork child to excute command
		

		if (command.is_history) 
		{
			command = old_command;
		} else
		{
			old_command = command;
		}			

		child_val=fork();

		if (child_val==-1)
		{
			printf("error"); 
		 	exit(1);
		}
		

		if (child_val == 0) 
		{
			//printf("I'm child %d, my parent %d\n",getpid(),getppid());	
			if (command.is_cd) 
			{
				chdir(command.args[1]);
				excute_process(&command);
			}		
			run_command(&command);
			
		}
		//parent
		else{
			excute_process(&command);
		}
	}
 
}