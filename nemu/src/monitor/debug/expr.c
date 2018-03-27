#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_UNEQ,
  TK_DECIMAL,       // 10
  TK_HEXADECIMAL,   // 16
  TK_REGISTER_NAME, // register
  TK_NEGTIVE,       // -
  TK_DEREFERENCE,   // *
  TK_LOGIC_AND,     // &&
  TK_LOGIC_OR,      // ||
  TK_HIGHEREQ,      // >=
  TK_LOWEREQ        // <=
  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},       // spaces
  {"\\+", '+'},            // plus
  {"-", '-'},              // subtract or negtive
  {"\\*", '*'},            // multiply or dereference
  {"\\/", '/'},            // devide
  {"%", '%'},              // mod
  {"==", TK_EQ},           // equal
  {"!=", TK_UNEQ},         // unequal
  {"0[xX][0-9|a-f|A-F]+", TK_HEXADECIMAL},  //16
  {"[0-9]+", TK_DECIMAL},  // 10
  {"\\$e?[abcd]x|\\$e?[bs]p|\\$e?[sd]i|\\$[abcd][hl]", TK_REGISTER_NAME},                               // register
  {"\\(", '('},             // (
  {"\\)", ')'},             // )
  {"&&", TK_LOGIC_AND},     // &&
  {"&", '&'},               // &
  {"\\|\\|", TK_LOGIC_OR},  // ||
  {"\\|", '|'},             // |
  {"!",  '!'},              // !
  {">=", TK_HIGHEREQ},      // >=
  {">", '>'},               // >
  {"<=", TK_LOWEREQ},       // <=
  {"<", '<'},               // <
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
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

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
			case TK_NOTYPE:
				continue;
			case '+':
			case '/':
			case '%':
			case TK_EQ:
			case TK_UNEQ:
			case '(':
			case ')':
			case TK_LOGIC_AND:
			case TK_LOGIC_OR:
			case '!':
			case '&':
			case '|':
			case TK_HIGHEREQ:
			case '>':
			case TK_LOWEREQ:
			case '<':
				tokens[nr_token].type=rules[i].token_type;
				nr_token++;
				break;
			case '-':
				if(tokens[nr_token-1].type==TK_DECIMAL||tokens[nr_token-1].type==TK_HEXADECIMAL||tokens[nr_token-1].type==TK_REGISTER_NAME||tokens[nr_token-1].type==')')
					tokens[nr_token].type=rules[i].token_type;
				else
					tokens[nr_token].type=TK_NEGTIVE;
				nr_token++;
				break;
			case '*':
				if(tokens[nr_token-1].type==TK_DECIMAL||tokens[nr_token-1].type==TK_HEXADECIMAL||tokens[nr_token-1].type==TK_REGISTER_NAME||tokens[nr_token-1].type==')')
					tokens[nr_token].type=rules[i].token_type;
				else
					tokens[nr_token].type=TK_DEREFERENCE;
				nr_token++;
				break;
			case TK_DECIMAL:
			case TK_HEXADECIMAL:
			case TK_REGISTER_NAME:
				if(substr_len>31)
				{
					Log("The description of the token is too long!");
					return false;
				}
				else
				{
					strncpy(tokens[nr_token].str,substr_start,substr_len);
					tokens[nr_token].str[substr_len]='\0';
					tokens[nr_token].type=rules[i].token_type;
					nr_token++;
					break;
				}
			default:
				Log("The type can't be matched!");
				return false;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p,int q)
{
	if((tokens[p].type!='(')||(tokens[q].type!=')'))
		return false;
	int i,num_1,num_2;
	num_1=0;
	num_2=0;
	for(i=p+1;i<p;i++)
	{
		if(tokens[i].type=='(')
			num_1++;
		else if(tokens[i].type==')')
		{
			num_2++;
			if(num_2>num_1)
			return false;
		}
	}
	if(num_1!=num_2)
		return false;
	else
		return true;
}

int search_dominant(int p,int q)
{
	int location,t,min_level,level;
	location=-1;
	min_level=0;
	for(t=p;t<=q;t++)
	{
		if(tokens[t].type=='(')
		{
			t++;
			while(tokens[t].type!=')')
				t++;
			if(t==q)
				break;
			t++;
		}
		if(tokens[t].type!=TK_DECIMAL&&tokens[t].type!=TK_HEXADECIMAL&&tokens[t].type!=TK_REGISTER_NAME)
		{
			switch(tokens[t].type)
			{
				case TK_NEGTIVE:
				case TK_DEREFERENCE:
				case '!':
					level=2;
					break;
				case '/':
				case '*':
				case '%':
					level=3;
					break;
				case '+':
				case '-':
					level=4;
					break;
				case '>':
				case TK_HIGHEREQ:
				case '<':
				case TK_LOWEREQ:
					level=6;
					break;
				case TK_EQ:
				case TK_UNEQ:
					level=7;
					break;
				case '&':
					level=8;
					break;
				case '|':
					level=10;
					break;
				case TK_LOGIC_AND:
					level=11;
					break;
				case TK_LOGIC_OR:
					level=12;
					break;
				default:
					panic("The token is wrong!");
			}
			if(level>min_level)
			{
				min_level=level;
				location=t;
			}
		}
	}
	if(location==-1)
		panic("The dominant can't be found!");
	return location;
}

uint32_t eval(int p,int q)
{
	if(p>q)
		panic("Bad expression!");
	else if(p==q)
	{
		uint32_t num;
		if(tokens[p].type==TK_DECIMAL)
		{
			sscanf(tokens[p].str,"%d",&num);
			return num;
		}
		else if(tokens[p].type==TK_HEXADECIMAL)
		{
			sscanf(tokens[p].str,"%x",&num);
			return num;
		}
		else if(tokens[p].type==TK_REGISTER_NAME)
		{
			if(strcmp(tokens[p].str,"$eax")==0)
				return reg_l(R_EAX);
			else if(strcmp(tokens[p].str,"$ebx")==0)
				return reg_l(R_EBX);
			else if(strcmp(tokens[p].str,"$ecx")==0)
				return reg_l(R_ECX);
			else if(strcmp(tokens[p].str,"$edx")==0)
				return reg_l(R_EDX);
			else if(strcmp(tokens[p].str,"$esp")==0)
				return reg_l(R_ESP);
			else if(strcmp(tokens[p].str,"$ebp")==0)
				return reg_l(R_EBP);
			else if(strcmp(tokens[p].str,"$esi")==0)
				return reg_l(R_ESI);
			else if(strcmp(tokens[p].str,"$edi")==0)
				return reg_l(R_EDI);
			else if(strcmp(tokens[p].str,"$ax")==0)
				return reg_w(R_AX);
			else if(strcmp(tokens[p].str,"$bx")==0)
				return reg_w(R_BX);
			else if(strcmp(tokens[p].str,"$cx")==0)
				return reg_w(R_CX);
			else if(strcmp(tokens[p].str,"$dx")==0)
				return reg_w(R_DX);
			else if(strcmp(tokens[p].str,"$sp")==0)
				return reg_w(R_SP);
			else if(strcmp(tokens[p].str,"$bp")==0)
				return reg_w(R_BP);
			else if(strcmp(tokens[p].str,"$si")==0)
				return reg_w(R_SI);
			else if(strcmp(tokens[p].str,"$di")==0)
				return reg_w(R_DI);
			else if(strcmp(tokens[p].str,"$al")==0)
				return reg_b(R_AL);
			else if(strcmp(tokens[p].str,"$bl")==0)
				return reg_b(R_BL);
			else if(strcmp(tokens[p].str,"$cl")==0)
				return reg_b(R_CL);
			else if(strcmp(tokens[p].str,"$dl")==0)
				return reg_b(R_DL);
			else if(strcmp(tokens[p].str,"$ah")==0)
				return reg_b(R_AH);
			else if(strcmp(tokens[p].str,"$bh")==0)
				return reg_b(R_BH);
			else if(strcmp(tokens[p].str,"$ch")==0)
				return reg_b(R_CH);
			else if(strcmp(tokens[p].str,"$dh")==0)
				return reg_b(R_DH);
		}
		else
			panic("The token is wrong!");
	}
	else if(check_parentheses(p,q)==true)
		return eval(p+1,q-1);
	else
	{
		int op,val1,val2;
		op=search_dominant(p,q);
		if(tokens[op].type==TK_NEGTIVE)
			return -1*eval(op+1,q);
		else if(tokens[op].type==TK_DEREFERENCE)
			return vaddr_read(eval(op+1,q),4);
		else if(tokens[op].type=='!')
			return !(eval(op+1,q));
		val1=eval(p,op-1);
		val2=eval(op+1,q);
		switch(tokens[op].type)
		{
			case '+':
				return val1+val2;
			case '-':
				return val1-val2;
			case '*':
				return val1*val2;
			case '/':
				if(val2!=0)
					return val1/val2;
				else
					panic("The expression is illegal!");
			case '%':
				return val1%val2;
			case TK_EQ:
				return val1==val2;
			case TK_UNEQ:
				return val1!=val2;
			case TK_LOGIC_AND:
				return val1&&val2;
			case '&':
				return val1&val2;
			case TK_LOGIC_OR:
				return val1||val2;
			case '|':
				return val1|val2;
			case TK_HIGHEREQ:
				return val1>=val2;
			case '>':
				return val1>val2;
			case TK_LOWEREQ:
				return val1<=val2;
			case '<':
				return val1<val2;
			default:
				panic("The token is wrong!");
		}
	}
	return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */

  return eval(0,nr_token-1);
}
