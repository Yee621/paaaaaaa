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

  {"^ +$", TK_NOTYPE},       // spaces
  {"^\\+$", '+'},            // plus
  {"^-$", '-'},              // subtract or negtive
  {"^\\*$", '*'},            // multiply or dereference
  {"^\\/$", '/'},            // devide
  {"^%$", '%'},              // mod
  {"^==$", TK_EQ},           // equal
  {"^!=$", TK_UNEQ},         // unequal
  {"^[0-9]+$", TK_DECIMAL},  // 10
  {"^0[xX][0-9|a-f|A-F]+$", TK_HEXADECIMAL},  //16
  {"^\\$e?[abcd]x|\\$e?[bs]p|\\$e?[sd]i|\\$[abcd][hl]$", TK_REGISTER_NAME},                               // register
  {"^\\($", '('},             // (
  {"^\\)$", ')'},             // )
  {"^&&$", TK_LOGIC_AND},     // &&
  {"^\\|\\|$", TK_LOGIC_OR},  // ||
  {"^!$",  '!'},              // !
  {"^>=$", TK_HIGHEREQ},      // >=
  {"^>$", '>'},               // >
  {"^<=$", TK_LOWEREQ},       // <=
  {"^<$", '<'},               // <
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

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
