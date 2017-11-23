/*
 * Evangelos Stratigakes
 * 
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

char* GetLine(FILE *stream);
int ProcessLine(char* line);
int ShellCommands(char* command);
int ChangePath(char **args);
int CountHistory();
int PrintHistory();
int BangOp(char **args);
int LinuxCommands(char* command);
int ParseLine(char* line, char ***args);
int ChangeDir(char **args);
int ExecuteCmd(char** args);


int main()
{
  char* line;
  int rcDone = 0;
  int i = 0;
  FILE* mshrc;
  FILE* histW;

  //Open mshrc for startup commands
  if(access("mshrc", R_OK) != -1)
    {
      mshrc = fopen("mshrc", "r");
    }
  else
    {
      printf("No mshrc file.\n");
      rcDone = 1;
    }
  
  do
    {
      //do the commands in mshrc first
      if(rcDone == 0)
	{
	  line = GetLine(mshrc);

	  if(line[0] == '\0' || line[0] == EOF || line == NULL)
	    {
	      rcDone = 1;
	    }
	}

      if(rcDone == 1) //get user commands once mshrc is done
	{
	  printf("?: "); //prompt user
	  line = GetLine(stdin);
	  
	  //save command
	  if(line[0] != '\0' && line[0] != '!')
	    {
	      if(access("history", W_OK) != -1)
		{
		  histW = fopen("history", "a");
		}
	      else
		{
		  histW = fopen("history", "a");
		}
	      fputs(line, histW);
	      fputc('\n',histW);
	      fclose(histW);
	    }
	}

      ProcessLine(line);

      free(line);
    } while (1);
  return 0;
}

char* GetLine(FILE* stream)
{
  int i = 0;
  char temp[256];
  char* line;
  char c;

  while(isspace(c = fgetc(stream)));
  temp[0] = c;

  //fill temp with the input
  for(i = 1; (temp[i] = fgetc(stream)) != '\n' && temp[i] != EOF; i++);
  temp[i] = '\0';

  //allocate the size for input line
  line = malloc(sizeof(char)*(strlen(temp)+1));

  //copy temp into line and return it!
  strcpy(line, temp);
  return line;
}


int ProcessLine(char* line)
{  
  char* command = strtok(line, ";");
 
  while(command != NULL)
    {
      //see if its a shell command, if it isn't try linux command.
      if(!(ShellCommands(command)))
      {
	LinuxCommands(command);
      }
      command = strtok(NULL, ";");
    }
  return 0;
}


int ShellCommands(char* command)
{
  FILE* hist;
  char **args;
  int tokens;
  char *tmp;
  char* path;
  int i;

  if(command != NULL)
    {
      tokens = ParseLine(command, &args);
      if(strcmp(args[0],"exit") == 0)
	{
	  exit(0);
	}
      else if(strcmp(args[0], "export") == 0)
	{
	  if(args[1] != NULL)
	    {
	      ChangePath(args);
	      return 1;
	    }
	  else
	    {
	      printf("ERROR: no argument for export.\n");
	      return 1;
	    }
	}
      else if(strcmp(args[0], "history") == 0)
	{
	  PrintHistory();

	  return 1;
	}
      else if(args[0][0] ==  '!')
	{
	  if(BangOp(args))
	    return 1;
	}
      else if(strcmp(args[0], "cd") == 0)
      {
          ChangeDir(args);
      }
   
      }
      return 0;
    }  



int ChangePath(char **args)
{
  int i, j = 0;
  char* comp = "PATH=$PATH:";
  char* path = getenv("PATH");
  char local[strlen(args[1])-9];
  char* newPath;

  for(i = 0; i < strlen(comp); i++)
    {
      if(args[1][i] != comp[i])
	{
	  printf("ERROR: incorrect syntax \n");
	  return 1;
	}
    }
  for(i = 0; i < strlen(local); i++)
    {
      local[i] = args[1][i+10];
    }
  
  newPath = malloc(sizeof(char) * (sizeof(local)+sizeof(path)+1));

  for(i = 0; i < strlen(path); i++)
    {
      newPath[i] = path[i];
    }
  for(j = 0; j < strlen(local); j++)
    {
      newPath[i] = local[j];
      i++;
    }

  setenv("PATH", newPath, 1);

  printf("New path has been set to:\n%s\n", getenv("PATH"));
}


int CountHistory()
{
  FILE* hist = fopen("history", "r");
  int i;
  char c;
  int count = 0;
  while(!feof(hist))
    {
      c = fgetc(hist);
      if(c == '\n')
	count++;
    }
  fclose(hist);
  return count;
}

int PrintHistory()
{
  FILE* hist;
  if(access("history", R_OK) != -1)
    {
      hist = fopen("history", "r");
    }
  else
    {
      printf("ERROR: No history to display\n");
      return 1;
    }

  int i = 0;
  int count = CountHistory();
  int offset = count-20;
  char c = '\n', prev;

  if(offset < 0)
    offset =0;

  while(i < count)
    {
      prev = c;
      c = fgetc(hist);
      if(i >= offset  && c != EOF)
	{
	  if(prev == '\n')
	    {
	      printf("%d  %c", i, c);
	    }
	  else
	    printf("%c");
	}
      if(c == '\n')
        i++;
    }
  fclose(hist);

}


int BangOp(char **args)
{
  FILE* hist = fopen("history", "r");
  char line[256];
  int i;
  int count = CountHistory();
  int offset = 0;
  char c;

  if(args[0][1] == '!')
    {
      offset = count-1;
      if(offset < 0)
	offset = 0; 
    }
  else if(isspace(args[0][1]) || args[0][1] == '\0')
    {
      printf("ERROR: no argument to !\n");
      return 1;
    }
  else
    {
      for(i = 1; i < strlen(args[0]); i++)
	{
	  if(!isdigit(args[0][i]))
	    {
	      printf("ERROR: incorrect argument to !\n");
	      return 1;
	    }
	  offset = offset * 10 + (args[0][i] - '0');
	}
    }

  if(offset < 0)
    {
      printf("ERROR: no command at that point in history.");
      return 1;
    }

  while(!feof(hist) && offset > 0)
    {
      c = fgetc(hist);
      if(c == '\n')
	offset--;
    }
  
  for(i = 0; (c = fgetc(hist)) != '\n'; i++)
    {
      line[i] = c;
    }
  line[i] = '\0';
  
  printf("%s\n", line);
  ProcessLine(line);  
  fclose(hist);
  
  return 1;
}

int LinuxCommands(char* command)
{
  char **args;
  int tokens;
  int i;
  if (command != NULL)
    {
      tokens = ParseLine(command, &args);
      
      ExecuteCmd(args);
      //free args memory
      for(i = 0; i < tokens; i++)
	{
	  free(args[i]);
	}
      free(args);
    }
}


int ParseLine(char* line, char ***args)
{
  int i = 0;
  static char **tokens;
  int tokensI = 0;
  char *token;
  int tokenCount = 0;
  char temp[256];
  int tempI; //index 

  //count how many tokens before allocating them
  for (i = 0; line[i] != '\0'; i++)
    {
      if(line[i] != ' ')
	{
	  tokenCount++;
	  while(line[i] != ' ' && line[i] != '\0')
	    {
	      i++;
	    }
	}
    }
 
  
  tokens = malloc(sizeof(char*) * tokenCount+1);

  for (i = 0; i < strlen(line); i++)
    {
      
      while(isspace(line[i]) && line[i] != '\0')
	{
	  i++;
	}

      if(!isspace(line[i]))
        {
         
          temp[tempI] = line[i];
          tempI++;
          temp[tempI] = '\0';

	  
	  if(isspace(line[i+1]) || line[i+1] == '\0')
	    {
	      //allocate a string for the token
	      
	      token = malloc(sizeof(char) * (strlen(temp)+1));
	      strcpy(token, temp);
	      tokens[tokensI] = token;
	      tokensI++;

	      //clear temp
	      temp[0] = '\0';
	      tempI = 0;
	    }
        }
    }

  
  tokens[tokensI] = NULL;

  *args = tokens;
  return tokenCount;
}

int ChangeDir(char **args)
{
    if (args[1] == NULL) 
    {
        fprintf(stderr, expected argument to \"cd\"\n");
    } 
    else 
    {
        if (chdir(args[1]) != 0) 
        {
          perror("msh");
        }
    }
  return 1;
}


int ExecuteCmd(char** args)
{
 
  pid_t pid, wpid;
  int status;

  pid = fork();

  if(pid == 0)      //child
    {
      if(execvp(args[0], args) == -1)
	{
	  printf("ERROR: Command failed. %s.\n", strerror(errno));
	  exit(EXIT_FAILURE);
	}
    }
  else if (pid < 0) //failed
    {
      printf("ERROR: Fork failed.\n");
    }
  else              //parent
    {
      //Wait
      waitpid(pid, &status, 0);
    }
 }
