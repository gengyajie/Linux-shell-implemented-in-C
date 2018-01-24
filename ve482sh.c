#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>

#define MAX_LINE_LEN 1024

void handler(int sig)
{
	if (sig == SIGINT) printf("Please press enter to continue:\n");
}

int quote_count (char *line, int *position) {
	int count = 0;
	int i = 0;
	while (line[i] != '\0') {
		if (line[i] == '"') {
			position[count] = i;
			count = count + 1;
		}
		i = i + 1;
	}
	if ((count % 2) == 1) {
		printf("> ");
		line[strlen(line)] = '\n';
		gets(line + strlen(line));
		count = quote_count(line, position);
	}
	else {
		int j = 0;
		while (line[j] != '\0') {
			if (line[j] == '"') line[j] = ' ';
			j = j + 1;
		}
		return count;
	}
}

void parse (char *line, char **arg) {
	int k = 1;
	while (line[strlen(line)-k] == ' ') {
		if (line[strlen(line)-k-1] != ' ') {
			line[strlen(line)-k] = '\0';
			break;
		}
		k = k + 1;
	}
	k = 0;
	while (line[k] == ' ') {
		if (line[k+1] != ' ') {
			line = line + k + 1;
			break;
		}
		k = k + 1;
	}
	//printf(line);
	int *position;
	char *temp = line;
	position = (int *)malloc(MAX_LINE_LEN*sizeof(int));
	int i = 0;
	int j = 0;
	int space_no = 1;
	int quote_no;
	quote_no = quote_count(line, position);
	char *space = strchr(line, ' ');
	while (space != NULL) {
		space[0] = '\0';
		arg[i] = (char *)malloc(MAX_LINE_LEN*sizeof(char));
		arg[i] = line;
		i = i + 1;
		while (j < (quote_no - 1)) {
			if (space > (temp + position[j]) && space < (temp + position[j+1])) break;
			j = j + 2;
		}
		if (j >= quote_no ) {
			k = 0;
			while (space[space_no] == ' ') {
				while (k < (quote_no - 1)) {
					if ((space + space_no) >= (temp + position[k])) break;
					k = k + 2;
				}
				if (k <= (quote_no -1)) {
					space_no = space_no + 1;
					break;
				}
				space_no = space_no + 1;
			}
		}
		j = 0;
		line = space + space_no;
		space_no = 1;
		space = strchr(line, ' ');
	}
	//printf(line);
	arg[i] = (char *)malloc(MAX_LINE_LEN*sizeof(char));
	arg[i] = line;
	i = i + 1;
	arg[i] = (char *)malloc(MAX_LINE_LEN*sizeof(char));
	arg[i] = NULL;
	free(position);
}

int split (char *line, char ***arg, char *option) {
	char *cmd;
	char *p = strchr(line, '|');
	int i = 0;
	if (p != NULL) {
		option[0] = 'p';
		p[0] = '\0';
		cmd = line;
		
		arg[i] = (char **)malloc(MAX_LINE_LEN*sizeof(char *));
		parse(cmd, arg[i]);
		i = i + 1;
		line = p + 1;
		cmd = line;
		arg[i] = (char **)malloc(MAX_LINE_LEN*sizeof(char *));
		parse(cmd, arg[i]);
		i = i + 1;
	}
	else {
		option[0] = '0';
		if ((p = strchr(line, '<')) != NULL) {
			option[1] = 'i';
			p[0] = '\0';
			cmd = line;
			arg[i] = (char **)malloc(MAX_LINE_LEN*sizeof(char *));
			parse(cmd, arg[i]);
			i = i + 1;
			line = p + 1;
		}
		else option[1] = '0';
		if ((p = strchr(line, '>')) != NULL) {
			p[0] = '\0';
			cmd = line;
			arg[i] = (char **)malloc(MAX_LINE_LEN*sizeof(char *));
			parse(cmd, arg[i]);
			i = i + 1;
			if (p[1] != '>') {
				option[2] = 'c';
				line = p + 1;
			}
			else {
				option[2] = 'a';
				line = p + 2;
			}
		}
		else option[2] = '0';
		cmd = line;
		arg[i] = (char **)malloc(MAX_LINE_LEN*sizeof(char *));
		parse(cmd, arg[i]);
		i = i + 1;
	}
	return i;
}

void runpipe (char ***arg) {
	int fd[2];
    pid_t childpid;
    int childstatus;
    pipe(fd);
    if((childpid = fork()) < 0 ) exit(1);
    else if(childpid == 0) {
		dup2(fd[1], 1);
		close(fd[0]);
		execvp(*arg[0], arg[0]);
		exit(0);
    }
    else {
		dup2(fd[0], 0);
		close(fd[1]);
		execvp(*arg[1], arg[1]);
		exit(0);
		while (wait(&childstatus) != childpid);
    }
}

void redirection (char ***arg, char *option) {
	int in, out;
	if (option[1] == 'i') {
		in = open(arg[1][0], O_RDONLY);
		if (option[2] != '0' ) {
			if (option[2] == 'c') out = open(arg[2][0], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			else if (option[2] == 'a') out = open(arg[2][0], O_WRONLY| O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR);
			dup2(in, 0);
			dup2(out, 1);
			close(in);
			close(out);
		}
		else {
			dup2(in, 0);
			close(in);
		}
	}
	else {
		if (option[2] == 'c') out = open(arg[1][0], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
		else if (option[2] == 'a') out = open(arg[1][0], O_WRONLY| O_APPEND, S_IRUSR | S_IWUSR | S_IXUSR);
		dup2(out, 1);
		close(out);
	}
	execvp(*arg[0], arg[0]);
}

void execute (char ***arg, int cmd_count, char *option) {
	pid_t pid;
	int status;
	pid = fork();
	if (pid < 0) exit (1);
	else if (pid == 0) {
		if (cmd_count == 1) {
			execvp(arg[0][0], arg[0]);
		}
		else if (cmd_count >= 2) {
			if (option[0] == 'p') runpipe(arg);
			else redirection(arg, option);
		}
	}
	else {
		while (wait(&status) != pid);
	}
}

int chg_dir (char *line) {
	char address[MAX_LINE_LEN];
	int lastcd = 0;
	int success = 0;
	if (line == NULL || (strcmp(line,"~") == 0)) {
		char *user = getlogin();
		char home[1024];
		strcat(home,"/home/");
		strcat(home,user);
		chdir(home);
		strcpy(address,home);
		lastcd = 1;
	}
	else if (strcmp(line,"-") == 0) {
		if (lastcd == 0) {
			printf("Error occured!\n");
			return 0;
		}
		else {
			chdir(address);
			lastcd = 1;
		}
	}
	success = chdir(line);
	if (success == -1) {
		printf("Error occured!\n");
		return 0;
	}
	else {
		lastcd = 1;
		getcwd(address, sizeof(address));
	}
	return 1;
}

int main () {
	char *line;
	char ***arg;
	char *option;
	line = (char *)malloc(MAX_LINE_LEN*sizeof(char));
	option = (char *)malloc(3*sizeof(char));
	arg = (char ***)malloc(MAX_LINE_LEN*sizeof(char **));
	char *tline = line;
	char ***targ = arg;
	int cmd_no;
	int error_flag = 0;
	signal (SIGINT, handler);
	while (1) {
		printf("ve482sh $ ");
		//if (fgets(line, MAX_LINE_LEN, stdin) != NULL) {
			gets(line);
			int k = 1;
			while (line[strlen(line)-k] == ' ') {
				if (line[strlen(line)-k-1] != ' ') {
					line[strlen(line)-k] = '\0';
					break;
				}
				k = k + 1;
			}
			k = 0;
			while (line[k] == ' ') {
				if (line[k+1] != ' ') {
					line = line + k + 1;
					break;
				}
				k = k + 1;
			}
			while (1) {
				if (line[strlen(line)-1] == '|' || line[strlen(line)-1] == '>' || line[strlen(line)-1] == '\n') {
					printf("> ");
					gets(line + strlen(line));
				}
				else if (line[strlen(line)-1] == '<') {
					printf("Error near <!\n");
					error_flag = 1;
					break;
				}
				else break;
			}
			if (!strcmp(line, "exit")) break;
			cmd_no = split(line, arg, option);
			if (!strcmp(arg[0][0], "cd")) error_flag = chg_dir(arg[0][1]);
			if (!error_flag) execute(arg, cmd_no, option);
		//}
		//else break;
	}
	free(tline);
	free(targ);
	free(option);
	return 0;
}
