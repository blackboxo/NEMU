#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"
#include "cpu/reg.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
	if(args==NULL){
		cpu_exec(1);
	}
	else {
		int num = atoi(args);
		cpu_exec(num);
	}
	return 0;
}

static int cmd_info(char *args){
	int i,j,k;
	if(*args=='r'){
		for(i=0;i<8;i++){
			printf("%s-%08x\n",regsl[i],reg_l(i));
		}
		for(j=0;j<8;j++){
			printf("%s-%04x\n",regsw[j],reg_w(j));
		}
		for(k=0;k<8;k++){
			printf("%s-%02x\n",regsb[k],reg_b(k));
		}
		printf("eip-%x\n",cpu.eip);		
	}
	return 0;
}

static int cmd_x(char *args){
	char *args1,*args2;
	args1 = strtok(args," ");
	args2 = strtok(NULL," ");
	int num = atoi(args1);
	int addr = strtol(args2,NULL,16);
	int i;
	for(i=0;i<num;i++)
	{
		printf("%08x\n",swaddr_read(addr+4*i,4));
	}
	return 0;	
}

static int cmd_p(char *args){
	char *arg;
	bool succ=true;
	bool* success=&succ;
	arg =strtok(args," ");
	printf("%s=%u\n",arg,expr(arg,success));
	return 0;
}



static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si","Single-step",cmd_si},
	{ "info","Print register",cmd_info},
	{ "x","Scan memory",cmd_x},
	{ "p","Experssion",cmd_p},
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}


#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
