#include "parser.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>



static void
execute_command(const struct command cmd, int input_fd, int output_fd)
{
	// printf("--- execute command  input_fd: %d, output_fd: %d ---\n", input_fd, output_fd);
	int pid;
	if ((pid = fork()) < 0) {
		printf("*** Couldn't Fork: %d ***\n", pid);
		assert(false);
	}
	if (pid == 0) {
		if (input_fd != STDIN_FILENO){
			dup2(input_fd, STDIN_FILENO);
			close(input_fd);
		}

		if (output_fd != STDOUT_FILENO){
			dup2(output_fd, STDOUT_FILENO);
			close(output_fd);
		}
		int exit_code;

		char **args = (char **)malloc(sizeof(cmd.args) * (cmd.arg_count + 2));
		args[0] = cmd.exe;
		for(int i = 0; i < cmd.arg_count; i ++ ){
			args[i+1] = cmd.args[i];
		}
		args[cmd.arg_count+1] = NULL;
		if ((exit_code = execvp(cmd.exe, args)) == -1) {
			printf("*** Exitted With Error: %d ***\n", exit_code);
			assert(false);
		}
		free(args);
	} else {
		wait(NULL);
	}
}

static void
execute_command_line(const struct command_line *line)
{	
	int fds[2], input_fd = STDIN_FILENO;
	
	const struct expr *e = line->head;
	while (e != NULL) {
		if (e->type == EXPR_TYPE_COMMAND) {
			pipe(fds);
			// printf("--- fds: %d %d ---\n", fds[0], fds[1]);
			if (e->next != NULL){
				execute_command(e->cmd, input_fd, fds[1]);
			}else{
				
				execute_command(e->cmd, input_fd, STDOUT_FILENO);
			}
			close(fds[1]);
			if( input_fd != STDIN_FILENO){ 
				close(input_fd);
			}
			input_fd = fds[0];
		} 
		else if (e->type == EXPR_TYPE_PIPE) {
			// printf("--- PIPE ---\n");
		}else if (e->type == EXPR_TYPE_AND) {
			// printf("--- AND ---\n");
		} else if (e->type == EXPR_TYPE_OR) {
			// printf("--- OR ---\n");
		} else {
			assert(false);
		}
		e = e->next;
	}
	if(input_fd != STDIN_FILENO){
		close(input_fd);
	}

}

static void
execute_command_line_default(const struct command_line *line)
{
	/* REPLACE THIS CODE WITH ACTUAL COMMAND EXECUTION */

	assert(line != NULL);
	printf("================================\n");
	printf("Command line:\n");
	printf("Is background: %d\n", (int)line->is_background);
	printf("Output: ");
	if (line->out_type == OUTPUT_TYPE_STDOUT) {
		printf("stdout\n");
	} else if (line->out_type == OUTPUT_TYPE_FILE_NEW) {
		printf("new file - \"%s\"\n", line->out_file);
	} else if (line->out_type == OUTPUT_TYPE_FILE_APPEND) {
		printf("append file - \"%s\"\n", line->out_file);
	} else {
		assert(false);
	}
	printf("Expressions:\n");
	const struct expr *e = line->head;
	while (e != NULL) {
		if (e->type == EXPR_TYPE_COMMAND) {
			printf("\tCommand: %s, size: %d\n", e->cmd.exe, e->cmd.arg_count);
			for (uint32_t i = 0; i < e->cmd.arg_count; ++i)
				printf(" %s,", e->cmd.args[i]);
			printf("\n");
		} else if (e->type == EXPR_TYPE_PIPE) {
			printf("\tPIPE\n");
		} else if (e->type == EXPR_TYPE_AND) {
			printf("\tAND\n");
		} else if (e->type == EXPR_TYPE_OR) {
			printf("\tOR\n");
		} else {
			assert(false);
		}
		e = e->next;
	}
}

int
main(void)
{
	const size_t buf_size = 1024;
	char buf[buf_size];
	int rc;
	struct parser *p = parser_new();
	while ((rc = read(STDIN_FILENO, buf, buf_size)) > 0) {
		parser_feed(p, buf, rc);
		struct command_line *line = NULL;
		while (true) {
			enum parser_error err = parser_pop_next(p, &line);
			if (err == PARSER_ERR_NONE && line == NULL)
				break;
			if (err != PARSER_ERR_NONE) {
				printf("Error: %d\n", (int)err);
				continue;
			}

			execute_command_line(line);
			// execute_command_line_default(line);
			command_line_delete(line);
		}
	}
	parser_delete(p);
	return 0;
}
