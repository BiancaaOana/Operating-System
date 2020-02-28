#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>

int open_file(char* path)
{
	FILE *fp = fopen(path, "rw");
	if (fp == NULL)
		return -1;
	return 1;
}

// command parser
int no_args(char* command)
{
	int i = 0, count = 0;
	for(i = 0 ; i < strlen(command); i++)
		if(command[i] == ' ')
			count++;
	return count;
}

char* substring(char* str, int begin, int end)
{
	if(begin < 0 || end >= strlen(str))
		return str;
	char *new_str = malloc(sizeof(char) * strlen(str));
	strcpy(new_str, str);
	new_str[end + 1] = '\0';
	strcpy(new_str, new_str + begin);
	return new_str;
}

int nthCharacter (char* sentence, int n, char ch)
{
	if(strchr(sentence, ch) == NULL)
		return -200;

	int i = 0;
	while(sentence[i] != ch)
		i++;

	if(n == 1)
		return i;

	return i + 1 + nthCharacter(sentence + i + 1, n - 1, ch);
}

char* nthWord (char* sentence, int n)
{
	int start_ch = nthCharacter(sentence, n - 1, ' ') + 1;
	int end_ch = nthCharacter(sentence, n, ' ') - 1;
	if (n == 1)
		start_ch = 0;
	if (n == no_args(sentence) + 1)
		end_ch = strlen(sentence) - 1;
	if (start_ch < 0 || end_ch < 0)
		return NULL;
	return substring(sentence, start_ch, end_ch);
}

char* get_command(char* command)
{
	return substring(command, 0, nthCharacter(command, 1, ' ') - 1);
}

char** get_args(char* command)
{
	int nr = no_args(command), i;
	char** args = (char **)malloc(nr * sizeof(char*));
	for(i = 0; i < nr; i++)
	{
		args[i] = malloc(sizeof(char) * strlen(command));
		args[i] = nthWord(command, i + 2);
	}
	return args;
}

// path parser
bool own_name (char *name)
{
	return strcmp(name, ".") != 0 && strcmp(name, "..") != 0;
}

int isFile (const char* name)
{
    DIR* directory = opendir(name);

    if(directory != NULL)
    {
    	closedir(directory);
    	return 0;
    }

    if(errno == ENOTDIR)
    	return 1;

    return -1;
}

char* get_month(int month)
{
	if(month == 1) return "ianuarie";
	if(month == 2) return "februarie";
	if(month == 3) return "martie";
	if(month == 4) return "aprilie";
	if(month == 5) return "mai";
	if(month == 6) return "iunie";
	if(month == 7) return "iulie";
	if(month == 8) return "august";
	if(month == 9) return "septembrie";
	if(month == 10) return "octombrie";
	if(month == 11) return "noiembrie";
	if(month == 12) return "decembrie";
}

char* current_dir (char *path)
{
	int i = strlen(path) - 1;
	while(path[i] != '/'){
		i--;
	}
	return path + i + 1;
}

char* parent_path (char *path)
{
	char *parent = malloc(sizeof(char) * strlen(path));
	strcpy(parent, path);
	int i = strlen(path) - 1;
	while(path[i] != '/'){
		i--;
	}
	parent[i] = '\0';
	return parent;
}

char* user_path (char *path)
{
	return substring(path, 0, nthCharacter(path, 3, '/') - 1);
}

// functions
char* pwd()
{
	char cwd[1024];
	char* pw = (char *)malloc(1024 * sizeof(char));
	getcwd(cwd, sizeof(cwd));
	strcpy(pw, cwd);
	return pw;
}

int cd (char *command)
{
	if(no_args(command) == 0)
	{
		if(!strcmp(command, "cd"))
			chdir(user_path(pwd()));
		return 1;
	}

	if(no_args(command) >= 2)
		return -2;

	char *dir_name = get_args(command)[0];
	if (opendir(dir_name) == NULL)
		return -1;

	chdir(dir_name);
	return 1;
}

int ls (char *command)
{
	char *wd = pwd();

	if (no_args(command) >= 1)
	{
		int cd_ret = cd(command);
		if (cd_ret < 0)
			return cd_ret;
	}

	DIR *parent = opendir("./");
	struct dirent *dir;
	int crt_file = 1;
	int crt_dir = 1;
	dir = readdir(parent);
	printf("___________________\n");
	do
	{
		if(own_name(dir->d_name) && !isFile(dir->d_name))
			printf("-- Dir %d: %s\n", crt_dir++, dir->d_name);
		dir = readdir(parent);
	}while(dir);
	parent = opendir("./");
	dir = readdir(parent);
	printf("___________________\n");
	do
	{
		if(own_name(dir->d_name) && isFile(dir->d_name))
			printf("-- File %d: %s\n", crt_file++, dir->d_name);
		dir = readdir(parent);
	}while(dir);
	printf("___________________\n");

	if (no_args(command) == 1)
		chdir(wd);

	return 1;
}

int cp (char *command)
{
	char c;
	char *fsrc = get_args(command)[0];
	char *fdst = get_args(command)[1];
	if (no_args(command) != 2)
		return -2;
	int rc_src = open_file(fsrc);
	if (rc_src == -1)
		return -1;
	FILE *rc_source = fopen(fsrc, "r");
	FILE *rc_dst = fopen(fdst, "wb+");
	while ((c = fgetc(rc_source)) != -1)
	{
		fputc(c, rc_dst);
	}
	printf("\n\n___________________________________\n");
	printf("________ = '%s' copiat cu succes catre '%s'!___________________________\n", fsrc, fdst);
	printf("___________________________________\n");
	return 1;
}

int touch (char *command)
{
	char *f = get_args(command)[0];
	if (no_args(command) > 1)
		return -2;
	int rc = open_file(f);
	if (rc >= 0)
		return -1;
	FILE *fp = fopen(f, "wb+");
	printf("\n\n___________________________________\n");
	printf("________ = Fisierul %s creat cu succes!___________________________\n", f);
	printf("___________________________________\n");
	return 1;
}

int mkd (char *command)
{
	if (no_args(command) > 1)
		return -2;
	return mkdir(get_args(command)[0], 0777);
}

int rmd (char *command)
{
	if (no_args(command) > 1)
		return -2;
	printf("\n\n___________________________________\n");
	printf("________ = Directorul %s s-a sters cu succes!___________________________\n", get_args(command)[0]);
	printf("___________________________________\n");
	return rmdir(get_args(command)[0]);
}

int rmv (char *command)
{
	char *f = get_args(command)[0];
	if (no_args(command) > 1)
		return -2;
	int rc = open_file(f);
	if (rc < 0)
		return -1;
	remove(f);
	printf("\n\n___________________________________\n");
	printf("________ = Fisierul %s sters cu succes!___________________________\n", f);
	printf("___________________________________\n");
	return 1;
}

int cat (char *command)
{
	int c;
	char *f = get_args(command)[0];
	if (no_args(command) > 1)
		return -2;
	int rc = open_file(f);
	if (rc < 0)
		return rc;
	printf("___________________________________\n");
	printf("\tNe aflam in: %s\n", pwd());
	printf("\tContinutul fisierului: %s\n", f);
	printf("___________________________________\n\n\n");
	FILE *fp = fopen(f, "rw");
	while ((c = fgetc(fp)) != -1)
	{
		fputc(c, stdout);
	}
	printf("\n\n___________________________________\n");
	return 1;
}

int date_time()
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	printf("-- Data: %d %s %d || Ora: %d:%d\n", tm->tm_mday, 
		get_month(tm->tm_mon + 1), tm->tm_year + 1900,
		tm->tm_hour, tm->tm_min);
	return 1;
}

int quit()
{
	printf("Programul s-a terminat!\n");
	exit(0);
	return 0;
}

int main ()
{
	int n;
	char *command = malloc(200 * sizeof(char));

	while(true)
	{
		fgets(command, 200, stdin);
		command[strlen(command) - 1] = '\0';
		if(!strcmp(get_command(command), "cd"))
		{
			int rc = cd(command);
			if (rc == -2)
				printf("ERROR: Format incorect. Sintaxa:\t cd <[director/cale]>\n");
			else if (rc == -1)
				printf("ERROR: Nu exista calea\t '%s'\n", get_args(command)[0]);
				
		}
		else if(!strcmp(get_command(command), "ls"))
		{
			int rc = ls(command);
			if (rc == -2)
				printf("ERROR: Format incorect. Sintaxa:\t ls <[director/cale]>\n");
			else if (rc == -1)
				printf("ERROR: Nu exista calea\t '%s'\n", get_args(command)[0]);
		}
		else if(!strcmp(get_command(command), "pwd"))
		{
			char* wd = pwd();
			printf("-- Directorul curent:\t%s\n", current_dir(wd));
			printf("-- Calea intreaga:\t%s\n", wd);
		}
		else if(!strcmp(get_command(command), "cat"))
		{
			int rc = cat(command);
			if (rc == -2)
				printf("ERROR: Format incorect. Sintaxa:\t cat <fisier>\n");
			else if (rc == -1)
				printf("ERROR: Nu exista fisierul\t '%s'\n", get_args(command)[0]);
		}
		else if(!strcmp(get_command(command), "touch"))
		{
			int rc = touch(command);
			if (rc == -2)
				printf("ERROR: Format incorect. Sintaxa:\t touch <fisier/cale>\n");
			else if (rc == -1)
				printf("ERROR: Fisierul  '%s'  este deja existent\n", get_args(command)[0]);
		}
		else if(!strcmp(get_command(command), "rm"))
		{
			int rc = rmv(command);
			if (rc == -2)
				printf("ERROR: Format incorect. Sintaxa:\t rm <fisier/cale>\n");
			else if (rc == -1)
				printf("ERROR: Fisierul '%s' nu exista\n", get_args(command)[0]);
		}
		else if(!strcmp(get_command(command), "cp"))
		{
			int rc = cp(command);
			if (rc == -2)
				printf("ERROR: Format incorect. Sintaxa:\t cp <fisier/cale sursa> <fisier/cale destinatie>\n");
			else if (rc == -1)
				printf("ERROR: Fisierul '%s' nu exista\n", get_args(command)[0]);	
		}
		else if(!strcmp(get_command(command), "mv"))
		{

		}
		else if(!strcmp(get_command(command), "mkdir"))
		{
			int rc = mkd(command);
			if (rc == -2)
				printf("ERROR: Format incorect. Sintaxa:\t mkdir <cale>\n");
			else if (rc == -1)
				printf("ERROR: Directorul '%s' deja exista\n", get_args(command)[0]);
		}
		else if(!strcmp(get_command(command), "rmdir"))
		{
			int rc = rmd(command);
			if (rc == -2)
				printf("ERROR: Format incorect. Sintaxa:\t mkdir <cale>\n");
			else if (rc == -1)
				printf("ERROR: Directorul '%s' nu exista\n", get_args(command)[0]);
		}
		else if(!strcmp(get_command(command), "date"))
		{
			date_time();
		}
		else if(!strcmp(get_command(command), "quit"))
		{
			quit();
		}
		else
		{
			system(command);
		}
	}

	return 0;
}
