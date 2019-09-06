/*
   <sys/wait.h>
        waitpid() and associated macros
   <unistd.h>
        chdir()
        fork()
        exec()
	getopt()
        pid_t
   <stdlib.h>
        strtol()
        malloc()
        realloc()
        free()
        exit()
        execvp()
		pipe()
		getcwd()
        EXIT_SUCCESS, EXIT_FAILURE
	WIFEXITED, WIFSIGNALED
   <stdio.h>
        fprintf()
        printf()
        stderr
        getchar()
        perror()
	getline()
   <string.h>
	strcmp()
        strncmp()
        strtok()
        strlen()
		strstr()
   <ctype.h>
	isdigit()
   <dirent.h>
	opendir()
	readdir()
   <sys/stat.h>
	mkdir()
*/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#define BUFSIZE 1024
#define MAXARGNUM 32
#define STACKSIZE 15
#define HISTSIZE 32
#define ALIASSIZE 15
#define DELIMITER " \t\r\n\a"

int my_cd(char **args);
int my_pushd(char **args,char **dirstack);
int my_popd(char **args,char **dirstack);
int my_dirs(char **args,char **dirstack);
int my_history(char **args,char **history);
int my_exclaexcla(char **args,char **dirstack,char **history,char **aliaslist);
int my_exclastring(char **args,char **dirstack,char **history,char **aliaslist);
int my_exclan(char **args,char **dirstack,char **history,char **aliaslist);
int my_alias(char **args,char **aliaslist);
int my_unalias(char **args,char **aliaslist);
int my_mkdir(char **args);
int my_pipe(char *line);
int my_help(char **args);

char *stdcommands[] = {"cd", "help", "exit","pushd","popd","dirs","history","!!","!string","!n","prompt","alias","unalias","mkdir"};
int counter=0;
int promptchange=0;
int histcount=-1;
int aliascount=-1;

char *parse(void)
{
  int bufsize = BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;
  if (!buffer) {
    fprintf(stderr, "dynamic memory allocation error\n");
    exit(EXIT_FAILURE);
  }
  while (1) {
    c = getchar();
    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    if (position >= bufsize) {
      bufsize += BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "dynamic memory allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **splitline(char *line)
{
  int bufsize = MAXARGNUM, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "dynamic memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, DELIMITER);
  while (token != NULL) {
    if(token[0]=='*'||token[strlen(token)-1]=='*'){
	DIR *d;
 	 struct dirent *dir;
 	 d = opendir(".");
 	 if (d) {
 	   while ((dir = readdir(d)) != NULL) {
		if(dir->d_name[0]!='.'){
			if(strlen(token)==1||(token[0]=='*'&&(!strncmp(token+1,(dir->d_name+strlen(dir->d_name)-strlen(token)+1),strlen(token)-1)))||(token[strlen(token)-1]=='*'&&(!strncmp(token,dir->d_name,strlen(token)-1)))){
 	     	 tokens[position] = dir->d_name;
    		position++;}}
 		}
 	   closedir(d);
 	 }
	}
    else{tokens[position] = token;
    position++;}

    if (position >= bufsize) {
      bufsize += MAXARGNUM;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "dynamic memory allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, DELIMITER);
  }
  tokens[position] = NULL;
  return tokens;
}

int my_cd(char **args)
{ 
  if (args[1] == NULL) {
    if(chdir(getenv("HOME"))!= 0){perror("change directory failed");return 3;}
  } else {
    if (chdir(args[1]) != 0) {
      perror("change directory failed");return 3;
    }
  }
  return 1;
}

int my_dirs(char **args,char **dirstack)
{
  int i;  char cwd[256];
  getcwd(cwd,sizeof(cwd));
  if (args[1] != NULL) {
    fprintf(stderr, "unrequired argument to \"dirs\"\n");return 3;
  } 
  else { 
       for (i = counter-1; i >= 0; i--) 
  	{
  	  printf("%s%s ",cwd,dirstack[i]);
  	}printf("%s\n",cwd);
  }
  return 1;
}

int my_pushd(char **args,char **dirstack)
{
  int i;   char cwd[256];
  getcwd(cwd,sizeof(cwd));
  if (args[1] == NULL) {
    strcpy(dirstack[counter],cwd);
      		counter++;
        	for (i = counter-1; i >= 0; i--) 
  		{
  	 	 	printf("%s ",dirstack[i]);
  		}	printf("%s\n",cwd);
  } 
  else if(chdir(args[1]) == 0){
	        strcpy(dirstack[counter],args[1]);
      		counter++;
        	for (i = counter-1; i >= 0; i--) 
  		{
  	 	 	printf("%s ",dirstack[i]);
  		}	printf("%s\n",cwd);
  }
  else{perror("pushd failed : directory does not exist");return 3;}
  return 1;
}

int my_popd(char **args,char **dirstack)
{
  int i;  char cwd[256];
  getcwd(cwd,sizeof(cwd));
  if(counter==0){printf("popd failed : stack has no directory paths in it\n");
  return 3;}
  else if (args[1] == NULL && (chdir(dirstack[counter-1]) == 0)) {
	dirstack[counter-1]="";
        counter--;
      for (i = counter-1; i >= 0; i--) 
  	{
  	  printf("%s ",dirstack[i]);
  	}printf("%s\n",cwd);
  }
  else if (args[1] != NULL && (chdir(dirstack[counter-1]) == 0)) {
        dirstack[counter-1]=NULL;
	counter--;
     for (i = counter-1; i >= 0; i--) 
  	{
  	  printf("%s ",dirstack[i]);
  	}printf("%s\n",cwd);
  }
  else{perror("change directory failed");return 3;}
  return 1;
}

int my_history(char **args,char **history){
  if (args[1] != NULL) {
    fprintf(stderr, "unrequired argument to \"history\"\n");return 3;
  } 
  int i; 
  if(histcount/HISTSIZE>0){
  for (i = (histcount%HISTSIZE)+1; i < HISTSIZE; i++) 
  	{
         if(history[i]!=NULL&&history[i]!=""&&history[i]!=" ")
  	  printf("- %d. %s\n",(i+1+HISTSIZE*(histcount/HISTSIZE-1)),history[i]);
  	}
  }
  for (i = 0; i < (histcount%HISTSIZE)+1; i++) 
  	{
         if(history[i]!=NULL)
  	  printf("- %d. %s\n",(i+1+HISTSIZE*(histcount/HISTSIZE)),history[i]);
  	}
  return 1;
}

int my_exclaexcla(char **args,char **dirstack,char **history,char **aliaslist){
  char** args2;
  char tmp[128];
  if (args[1] != NULL) {fprintf(stderr, "unrequired argument to \"!!\"\n");return 3;} 
  if(histcount<0) {printf("!! failed : no commands in history\n");return 3;}
  else if((histcount+1)%HISTSIZE==0) {
	strcpy(tmp,history[(HISTSIZE-1)]);
	args2 = splitline(tmp);
	}
  else {
	strcpy(tmp,history[histcount%HISTSIZE]);
	args2 = splitline(tmp);
	}
  return execute(args2,dirstack,history,aliaslist);
}

int my_exclastring(char **args,char **dirstack,char **history,char **aliaslist){
  char** args2;   int i;
  char* str=(char*)malloc((30)*sizeof(char));
  char tmp[128];
  str=strtok(args[0],"!");
  if (args[1] != NULL) {fprintf(stderr, "unrequired argument to \"!string\"\n");return 3;} 
  if(histcount<0||str==NULL) {printf("!%s failed : no commands in history\n",str);return 3;}
   for (i = (histcount%HISTSIZE); i >= 0; i--) 
   {
        if(history[i]!=NULL){
		if(strncmp(str,history[i],strlen(str))==0){
			strcpy(tmp,history[i]);
			args2 = splitline(tmp);
			return execute(args2,dirstack,history,aliaslist);
		}
        }
   }
   for (i = HISTSIZE; i >= (histcount%HISTSIZE)+1; i--) 
   {
        if(history[i]!=NULL){
		if(strncmp(str,history[i],strlen(str))==0){
			strcpy(tmp,history[i]);
			args2 = splitline(tmp);
			return execute(args2,dirstack,history,aliaslist);
		}
        }
    }
  printf("!%s failed : no such command in history\n",str);
  //free(str);
  return 3;
}

int my_exclan(char **args,char **dirstack,char **history,char **aliaslist){
   char** args2;   int i,n;
  char tmp[128];
  if (args[1] != NULL) {fprintf(stderr, "unrequired argument to \"!n\"\n");return 3;} 
  if(histcount<0) {printf("!n failed : no commands in history\n");return 3;}
  n=(int)strtol(*(args)+1,NULL,10);
   for (i = (histcount%HISTSIZE); i >= 0; i--) 
   {
        if(history[i]!=NULL&&(n<0&&n>-1*(HISTSIZE-1))&&histcount>=i&&n==-1*i){
			strcpy(tmp,history[histcount-i+1]);
			args2 = splitline(tmp);
			return execute(args2,dirstack,history,aliaslist);
	       }
	else if(history[(HISTSIZE-i)]!=NULL&&(n>(histcount+1-HISTSIZE)&&n<=histcount+1)&&(n%HISTSIZE)==(HISTSIZE-i)){
			strcpy(tmp,history[(HISTSIZE-i-1)]);
			args2 = splitline(tmp);
			return execute(args2,dirstack,history,aliaslist);
	       }
   }
   for (i = HISTSIZE-1; i >= (histcount%HISTSIZE)+1; i--) 
   {
        if(history[i]!=NULL&&(n<0&&n>-1*(HISTSIZE-1))&&histcount>=i&&n==-1*i){
			strcpy(tmp,history[histcount-i+1]);
			args2 = splitline(tmp);
			return execute(args2,dirstack,history,aliaslist);
	       }
	else if(history[(HISTSIZE-i)]!=NULL&&(n>(histcount+1-HISTSIZE)&&n<=histcount+1)&&(n%HISTSIZE)==(HISTSIZE-i)){
			strcpy(tmp,history[(HISTSIZE-i-1)]);
			args2 = splitline(tmp);
			return execute(args2,dirstack,history,aliaslist);
	       }
    }
  printf("!%d failed : no such command in history\n",n);
  //free(tmp);
  return 3;
}

int my_alias(char **args,char **aliaslist){
  if (args[1] == NULL) {
    int i;
    for (i = 0; i < aliascount+1; i++) 
  	{
         if(strcmp(aliaslist[i]," ")!=0)
  	  printf("%s\n",aliaslist[i]);
  	}
  } 
  else if(args[2] != NULL&&args[3] == NULL){
	aliascount++;
       	strcpy(aliaslist[aliascount],args[1]);
       	strcat(aliaslist[aliascount]," ");
	strcat(aliaslist[aliascount],args[2]);
  }
  else{fprintf(stderr, "enter exactly 0 or 2 arguments for \"alias\"\n");return 3;}
  return 1;
}

int my_unalias(char **args,char **aliaslist){
  int i,n=0,j;   
  char* token=(char*)malloc((64)*sizeof(char));
  char* tmp=(char*)malloc((64)*sizeof(char));
  if (args[1] == NULL) {
    fprintf(stderr, "required argument to \"unalias\"\n");return 3;
  } 
  for(i=0;i<aliascount+1;i++)
  {
       strcpy(tmp,aliaslist[i]);
       token=strtok(tmp,DELIMITER);
       if(strcmp(token,args[1])==0)
  	  {n=1;
           for(j=i;j<aliascount;j++)
           {strcpy(aliaslist[j],aliaslist[j+1]);}
           aliascount--;
           }
  }
  if(n==0){
    fprintf(stderr, "could not find alias \"%s\"\n",args[1]);return 3;
  }
  free(tmp);
  return 1;
}

int my_mkdir(char **args){
  int c,argc=0;
  mode_t mode=0777;
  char path[128];
  getcwd(path,sizeof(path));
  while(args[argc]!=NULL){argc++;}
  while ((c = getopt(argc, args, "m:p:")) != -1) {
		switch (c) {
		case 'm':mode=(mode_t)strtol(optarg,NULL,8);
			break;
		case 'p':strcpy(path,optarg);
			break;
		default:fprintf(stderr, "expected -m or -p options only for \"%s\"\n",args[0]);return 3;
			return 1;
			break;
		}
	}
	argc -= optind;
	args += optind;
  if (*args) {
        strcat(path,"/");strcat(path,*args);
	if(mkdir(path,mode)==-1){perror("mkdir failed ");return 3;}
  	}
  else{fprintf(stderr, "expected directory name argument for \"mkdir\"\n");return 3;}
  return 1;
}

int my_help(char **args)
{
  int i;
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");
  for (i = 0; i < (sizeof(stdcommands) / sizeof(char *)); i++) 
  {
    printf("%d.  %s\n", i,stdcommands[i]);
  }
  printf("Use the man command for information on other programs.\n");
  return 1;
}

int my_pipe(char *line)
{
	int i,commandc=0,numpipes=0,status;
	pid_t pid;
	char **args;
	for (i = 0; line[i]!='\0'; i++) 
    	{if(i>0){if(line[i]=='|'&&line[i+1]!='|'&&line[i-1]!='|'){numpipes++;}}}
	int* pipefds=(int*)malloc((2*numpipes)*sizeof(int));
	char* token=(char*)malloc((128)*sizeof(char));
	token=strtok_r(line,"|",&line);
	for( i = 0; i < numpipes; i++ ){	
    		if( pipe(pipefds+i*2) < 0 ){perror("pipe creation failed");return 3;}
	}
	do{
	    pid = fork();
	    if( pid == 0 ){//child process
	        if( commandc!=0 ){
	           	if( dup2(pipefds[(commandc-1)*2], 0) < 0){perror("child couldnt get input");exit(1);}
        	}
        	if( commandc!=numpipes){
            		if( dup2(pipefds[commandc*2+1], 1) < 0 ){perror("child couldnt output");exit(1);}
       	 	}
            for( i = 0; i < 2*numpipes; i++ ){close(pipefds[i]);}
	    args=splitline(token);
            execvp(args[0],args);
            perror("exec failed");exit(1);
 	    } 
	    else if( pid < 0 ){perror("fork() failed");return 3;}//fork error
 	    commandc++;//parent process
	}
	while(commandc<numpipes+1&&(token=strtok_r(NULL,"|",&line)));
	for( i = 0; i < 2*numpipes; i++ ){close(pipefds[i]);}
	free(pipefds);
	return 1;
}

int my_launch(char **args)
{
  pid_t pid;
  int status;
  pid = fork();
  if (pid == 0) { //child process
    if (execvp(args[0], args) == -1) 
    {   perror("child process: execution error");return 3; }
    exit(EXIT_FAILURE);
  } 
  else if (pid < 0) {
    perror("forking error");return 3;
  } 
  else {//parent process
    do {
         waitpid(pid, &status, WUNTRACED);
    } 
    while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  if(status==0)
  	return 1;
  return 3;
}

int execute(char **args,char **dirstack,char **history,char **aliaslist)
{
  int i;   
  if (args[0] == NULL) {   return 1; }
  else if(args[0][0]!='!'){
  histcount++;
  strcpy(history[(histcount%HISTSIZE)],args[0]);
  if (args[1] != NULL){
  	for (i = 1; i < MAXARGNUM; i++) {
   	 if (args[i] != NULL){
           strcat(history[(histcount%HISTSIZE)]," ");
   	   strcat(history[(histcount%HISTSIZE)],args[i]);
           }
         else{break;}
  	 }
      }
  }
  if(aliascount>=0){
  char* token=(char*)malloc((64)*sizeof(char));
  char* tmp=(char*)malloc((64)*sizeof(char));
  for(i=0;i<aliascount+1;i++)
  {
       strcpy(tmp,aliaslist[i]);
       token=strtok(tmp,DELIMITER);
       if(strcmp(token,args[0])==0)
  	  { token=strtok(NULL,DELIMITER);
            strcpy(args[0],token);}
  }
  //free(token);  
  free(tmp);
  }
  for (i = 0; i < (sizeof(stdcommands) / sizeof(char *)); i++) {
    if (strcmp(args[0], stdcommands[i]) == 0) {
      switch(i){
	case 0:return my_cd(args);break;	
	case 1:return my_help(args);break;	
	case 2:return 0;break;	
	case 3:return my_pushd(args,dirstack);break;	
	case 4:return my_popd(args,dirstack);break;	
	case 5:return my_dirs(args,dirstack);break;	
	case 6:return my_history(args,history);break;	
	case 7:return my_exclaexcla(args,dirstack,history,aliaslist);break;	
	case 8:return 1;break;
	case 9:return 1;break;
	case 10:return 2;break;
	case 11:return my_alias(args,aliaslist);break;
	case 12:return my_unalias(args,aliaslist);break;
	case 13:return my_mkdir(args);break;	
	}
    }
  }
  if(args[0][0]=='!'){
	if((isdigit(args[0][1])>0&&isdigit(args[0][2])>0)||(isdigit(args[0][1])>0&&isdigit(args[0][2])==0)&&isdigit(args[0][3])==0){
		return my_exclan(args,dirstack,history,aliaslist);
        }
	else if((isdigit(args[0][2])>0&&isdigit(args[0][3])>0)||(isdigit(args[0][2])>0&&isdigit(args[0][3])==0)&&args[0][1]=='-'&&isdigit(args[0][4])==0){
		return my_exclan(args,dirstack,history,aliaslist);
	}
        else{
		return my_exclastring(args,dirstack,history,aliaslist);
        }
    }
  return my_launch(args);
}

int main(int argc, char **argv)
{
  int i,j=0,scriptfn=0,status=1;
  char *line,*oneline,*tmp,**args,*saveptr,prompt[256],*gotline="s";
  char** dirstack=(char**)malloc((STACKSIZE+1)*sizeof(char*));
  for (i = 0; i < STACKSIZE+1; i++) 
    	dirstack[i]=(char*)malloc((30)*sizeof(char));
  char** history=(char**)malloc((HISTSIZE)*sizeof(char*));
  for (i = 0; i < HISTSIZE; i++) 
    	history[i]=(char*)malloc((128)*sizeof(char));
  char** aliaslist=(char**)malloc((ALIASSIZE+1)*sizeof(char*));
  for (i = 0; i < ALIASSIZE+1; i++) 
  	aliaslist[i]=(char*)malloc((30)*sizeof(char));
   //script function- one command per line
  FILE* fp;
  if(argc==3&&strcmp(argv[1],"<")==0){
	scriptfn++;printf("\n\n%d \n ",scriptfn);
	fp=fopen(argv[2],"r");
	if(fp==NULL){perror("could not open file");return EXIT_FAILURE;}
  }
  do {
    j=0;
    if(promptchange==0)
    	getcwd(prompt,sizeof(prompt));
    if(scriptfn==1){printf("\n");gotline=fgets(line,sizeof(line),fp);}
    else {
	printf("%s->  ",prompt);
	line = parse();
	}
    for (i = 0; line[i]!='\0'; i++) 
    	{
	if(line[i]==';'){j++;}
	else if(i>0&&line[i-1]=='&'&&line[i]=='&'){j++;line[i-1]=';';}
	else if(i>0&&line[i-1]=='|'&&line[i]=='|'){j++;line[i-1]=';';}
	else if(i>0&&line[i]=='|'&&line[i-1]!='|'&&line[i+1]!='|'){
		histcount++;
 		strcpy(history[(histcount%HISTSIZE)],line);
		my_pipe(line);
		goto piped;
		}
	}
    tmp=(char*)malloc((strlen(line)+1)*sizeof(char));
    strcpy(tmp,";");strcat(tmp,line);
    oneline=strtok_r(tmp,";",&tmp);
    do{
    	if(oneline!=NULL){
    	if(oneline[0]=='|'&&status!=3){}
	if(oneline[0]=='&'&&status!=1){break;}
    	else{
		if(oneline[0]=='&'){oneline++;}
    		args = splitline(oneline);
    		status = execute(args,dirstack,history,aliaslist);
	    }
    	}
    	oneline=strtok_r(NULL,";",&tmp);
    	j--;
    }
    while(j>-1&&status);
    if(status==2){ 
	promptchange=1;
        if(args[1]!=NULL){strcpy(prompt,args[1]);}
        else{strcpy(prompt,"Command: ");}
    }
    free(args);
    piped: free(line);
  } 
  while (status&&gotline!=NULL);
  if(scriptfn==1){fclose(fp);}
  free(tmp);
  free(dirstack);
  free(history);
  free(aliaslist);
  return EXIT_SUCCESS;
}
