#include "nemu.h"
#include "common.h"
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
	NOTYPE = 256, EQ,
	DEC,HEX,REG,
	AND,NE,OR,
	NOT,POINTER,NEG,
	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"[0-9]+",DEC},					// decimal
	{"0x[0-9a-f]+",HEX},			// heximal
	{"\\$[a-z]+",REG},				// reg
	{"\\+", '+'},					// plus
	{"-",'-'},						// subscribe
	{"\\*",'*'},					// multiply
	{"/",'/'},						// divide
	{"\\(",'('},					// left
	{"\\)",')'},					// right
	{"==", EQ},						// equal
	{"!=",NE},						// not equal
	{"&&",AND},						// and
	{"\\|\\|",OR},					// or
	{"!",NOT},						// not
	{"-",NEG},						// negative
	{"\\*",POINTER},				// pointer
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];
uint32_t eval(uint32_t p,uint32_t q);
bool check_parentheses(uint32_t p,uint32_t q);
uint32_t getOp(uint32_t p,uint32_t q);

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array ``tokens''. For certain 
				 * types of tokens, some extra actions should be performed.
				 */
					
				switch(rules[i].token_type) {
					case NOTYPE:
						break;
					case DEC:
						tokens[nr_token].type=rules[i].token_type;
						if(substr_len<32){
							memcpy(tokens[nr_token].str,substr_start,substr_len);
						}	
						else{ 
							Log("Error");
							return false;
						};
						nr_token++;
						break;
					case HEX:
						tokens[nr_token].type=rules[i].token_type;	
						memcpy(tokens[nr_token].str,substr_start,substr_len);
						nr_token++;
						break;
					case REG:
						tokens[nr_token].type=rules[i].token_type;  
						memcpy(tokens[nr_token].str,substr_start+1,substr_len-1);
						nr_token++;
						break;
					case '-':
						if(nr_token==0||((tokens[nr_token-1]).type!=DEC&&(tokens[nr_token-1]).type!=HEX&&(tokens[nr_token-1]).type!=')'))
						{tokens[nr_token].type=NEG;
						nr_token++;
						break;}
					case '*':
						if(nr_token==0||((tokens[nr_token-1]).type!=DEC&&(tokens[nr_token-1]).type!=HEX&&(tokens[nr_token-1]).type!=')'))
						{tokens[nr_token].type=POINTER;
						nr_token++;
						break;}
					case '+':
					case '/':
					case '(':
					case ')':
					case EQ:
					case NE:
					case AND:
					case OR:
					case NOT:
						tokens[nr_token].type=rules[i].token_type;
						nr_token++;
						break;
					default: panic("please implement me");
				}
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	return eval(0,nr_token-1);
}

uint32_t eval(uint32_t p,uint32_t q) {
	    if(p > q) {
			assert(0); /* Bad expression */
		}
		else if(p == q) {
			char temp[32];
			switch(tokens[p].type){
				case DEC:
					return atoi(tokens[p].str);
				case HEX:
					return strtoull(tokens[p].str,NULL,16);
				case REG:
					strcpy(temp,tokens[p].str);
					if(!strcmp(temp,"eax"))return cpu.eax;
					else if(!strcmp(temp,"ecx"))return cpu.ecx;
					else if(!strcmp(temp,"edx"))return cpu.edx;
					else if(!strcmp(temp,"ebx"))return cpu.ebx;
					else if(!strcmp(temp,"esp"))return cpu.esp;
					else if(!strcmp(temp,"ebp"))return cpu.ebp;
					else if(!strcmp(temp,"esi"))return cpu.esi;
					else if(!strcmp(temp,"edi"))return cpu.edi;
					else if(!strcmp(temp,"eip"))return cpu.eip;
					default:assert(0);
		}
		}
		else if(check_parentheses(p, q) == true) {
			return eval(p + 1, q - 1); 
		}
		else if(tokens[p].type==NEG){
			return -eval(p+1,q);	
		}
		else if(tokens[p].type==NOT){
			return !eval(p+1,q);
		}
		else if(tokens[p].type==POINTER){
			return swaddr_read(eval(p+1,q),4);
		}
		else{
			uint32_t op=getOp(p,q);
			uint32_t val1 = eval(p, op - 1);
		    uint32_t val2 = eval(op + 1, q);
			
			switch(tokens[op].type){
				case '+':return val1+val2;
				case '-':return val1-val2;
				case '*':return val1*val2;
				case '/':return val1/val2;
				case AND:return val1&&val2;
				case OR:return val1||val2;
				case EQ:return val1==val2;
				case NE:return val1!=val2;
				default:assert(0);
			}
		}
}

bool check_parentheses(uint32_t p,uint32_t q)
{
	if(tokens[p].type!='('||tokens[q].type!=')')
		return false;
	int par=0;
	for(;p<=q;p++){
		if(par<0)
			return false;
		if(tokens[p].type=='(')
			par++;
		if(tokens[p].type==')')
			par--;
		if(par==0&&p!=q)
			return false;
	}
	if(par==0)
		return true;
	return false;
}	

uint32_t getOp(uint32_t p,uint32_t q)
{
	/* /,*:3,
	 * +,-:4,
	 * ==,!=:7
	 * &&:11
	 * ||:12
	 */
	int par=0;
	int op=p;
	int pri=0;
	for(;p<=q;p++)
	{
		if(tokens[p].type==DEC||tokens[p].type==HEX||tokens[p].type==REG||tokens[p].type==NEG||tokens[p].type==POINTER||tokens[p].type==NOT)
			continue;
		else if(tokens[p].type=='(')
		{
			par++;
			p++;
			while(par!=0){
			if(tokens[p].type=='(')par++;
			else if(tokens[p].type==')')par--;
			p++;}
		p--;
		}
		else if(tokens[p].type=='/'||tokens[p].type=='*'){
			if(pri<=3){
				op=p;pri=3;
			}
		}
		else if(tokens[p].type=='+'||tokens[p].type=='-'){
			if(pri<=4){
				op=p;pri=4;
			}
		}
		else if(tokens[p].type==EQ||tokens[p].type==NE){
			if(pri<=7){
				op=p;pri=7;
			}
		}
		else if(tokens[p].type==AND){
			if(pri<=11){
				op=p;pri=11;
			}
		}
		else if(tokens[p].type==OR){
			if(pri<=12){
				op=p;pri=12;
			}
		}
				
	}
	return op;
}


