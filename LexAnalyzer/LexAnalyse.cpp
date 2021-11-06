#include <bits/stdc++.h>

using namespace std;

#define MAX_SIZE 102400
#define MAX_SINGLE 1024

char keywords[6][MAX_SINGLE]=
{
	"int","void","if","else","while","return"
};

enum
{
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Char, Else, Enum, If, Int, Return, Sizeof, While,
    Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak, Brace, Pare, Colon, Scolon, Comma,
    Var, Separator, Keyw, Other
};

char src[MAX_SIZE];

int Ind=0;

struct node
{
	bool isend;
	int key;
	int type;
	char value[MAX_SINGLE];
	struct node* nxt;

	node* next()
	{
		int now;//now character
		isend=0;
		for(int i=0; i<MAX_SINGLE; i++)
			value[i]=0;
		key=0;
		type=0;
		nxt=NULL;
		while(!isend)
		{
			now=src[Ind++];
			if (now == '\n'||now=='	')
				continue;
			else if(now==' ')
			{
				if(src[Ind]==' ')
					Ind++;
			}
			else if ((now >= 'a' && now <= 'z') || (now >= 'A' && now <= 'Z') || (now == '_'))// var
			{
				int i = 0;
				value[i]=now;
				while ((src[Ind] >= 'a' && src[Ind] <= 'z') || (src[Ind] >= 'A' && src[Ind] <= 'Z') || (src[Ind] >= '0' && src[Ind] <= '9') || (src[Ind] == '_'))
				{
					value[++i] = src[Ind++];
				}
				value[i+1] = '\0';
				//check keywords
				for (int j = 0; j < 6; j++)
				{
					if (strcmp(value, keywords[j]) == 0)
					{
						key = j + 1;
						type=Keyw;
						break;
					}
				}
				if(type!=Keyw)
				{
					key=39;
					type=Var;
				}
				break;
			}
			//number
			else if (now >= '0' && now <= '9')
			{
				int i;
				value[0]=now;
				for (i=1; src[Ind] >= '0' && src[Ind] <= '9'; i++)
				{
					value[i]=src[Ind++];
				}
				key = 40;
				type = Num;
				value[i]='\0';
				break;
			}
			//comments
			else if (now == '/')
			{
				if (src[Ind] == '/')
				{
					// skip comments
					while (src[Ind] != 0 && src[Ind] != '\n')
						Ind++;
				}
				else
				{
					// divide operator
					key=44;
					type = Div;
					value[0]=now;
					break;
				}
			}
			//others
			else if (now == '+')
			{
				// parse '+' and '++'
				if (src[Ind] == '+')
				{
					Ind++;
					key=45;
					type = Inc;
					value[0]=value[1]='+';
					break;
				}
				else
				{
					key=41;
					type = Add;
					value[0]='+';
					break;
				}
			}
			else if (now == '-')
			{
				// parse '-' and '--'
				if (src[Ind] == '-')
				{
					Ind++;
					key=46;
					type = Dec;
					value[0]=value[1]='-';
					break;
				}
				else
				{
					key=42;
					type = Sub;
					value[0]='-';
					break;
				}
			}
			else if (now == '*')
			{
				key=43;
				type = Mul;
				value[0]='*';
				break;
			}
			else if (now == '=')
			{
				// parse '==' and '='
				if (src[Ind] == '=')
				{
					Ind++;
					type = Eq;
					value[0]=value[1]='=';
					break;
				}
				else
				{
					key=47;
					type = Assign;
					value[0]='=';
					break;
				}
			}
			else if (now == '!')
			{
				// parse '!='
				if (src[Ind] == '=')
				{
					Ind++;
					key=48;
					type = Ne;
					value[0]='!';
					value[1]='=';
					break;
				}
			}
			else if (now == '<')
			{
				// parse '<=', '<<' or '<'
				if (src[Ind] == '=')
				{
					Ind++;
					key=49;
					type = Le;
					value[0]='<';
					value[1]='=';
					break;
				}
				else if (src[Ind] == '<')
				{
					Ind++;
					key=50;
					type = Shl;
					value[0]=value[1]='<';
					break;
				}
				else
				{
					key=51;
					type = Lt;
					value[0]='<';
					break;
				}
			}
			else if (now == '>')
			{
				// parse '>=', '>>' or '>'
				if (src[Ind] == '=')
				{
					Ind++;
					key=52;
					type = Ge;
					value[0]='>';
					value[1]='=';
					break;
				}
				else if (src[Ind] == '>')
				{
					Ind++;
					key=53;
					type = Shr;
					value[0]=value[1]='>';
					break;
				}
				else
				{
					key=54;
					type = Gt;
					value[0]='>';
					break;
				}
			}
			else if (now == '|')
			{
				// parse '|' or '||'
				if (src[Ind] == '|')
				{
					Ind++;
					key=55;
					type = Lor;
					value[0]=value[1]='|';
					break;
				}
				else
				{
					key=56;
					type = Or;
					value[0]='|';
					break;
				}
			}
			else if (now == '&')
			{
				// parse '&' and '&&'
				if (src[Ind] == '&')
				{
					Ind++;
					key=58;
					type = Lan;
					value[0]=value[1]='&';
					break;
				}
				else
				{
					key=59;
					type = And;
					value[0]='&';
					break;
				}
			}
			else if (now == '^')
			{
				key=60;
				type = Xor;
				value[0]='^';
				break;
			}
			else if (now == '%')
			{
				key=61;
				type = Mod;
				value[0]='%';
				break;
			}
			else if (now == '['||now==']')
			{
				key=62;
				type = Brak;
				value[0]=now;
				break;
			}
			else if (now == '?')
			{
				key=63;
				type = Cond;
				value[0]='?';
				break;
			}
			else if (now == '{' || now == '}')
			{
				key=64;
				type = Brace;
				value[0]=now;
				break;
			}
			else if (now == '(' || now == ')')
			{
				key=65;
				type = Pare;
				value[0]=now;
				break;
			}
			else if (now == ':')
			{
				key=66;
				type = Colon;
				value[0]=now;
				break;
			}
			else if (now == ';')
			{
				key=67;
				type = Scolon;
				value[0]=now;
				break;
			}
			else if (now == ',')
			{
				key=68;
				type = Comma;
				value[0]=now;
				break;
			}
			else if(now == '~' || now == '"' || now =='\\')
			{
				key=69;
				type=Separator;
				value[0]=now;
				break;
			}
			else if(now=='#')
			{
				isend=1;
				break;
			}
			else
			{
				printf("Error\n");
				key = 70;
				type =Other;
				value[0]=now;
				break;
			}
		}
		node *tmpp = new(nothrow)node;
		tmpp->nxt=NULL;
		nxt = tmpp;
		return this;
	}
	void procedure(struct node *tmp, FILE *file_out)
	{
		printf("result:\n");
		tmp=tmp->next();//get next
		while (!tmp->isend)
		{
			printf("< %d , %s >\n", tmp->key, tmp->value);
			fprintf(file_out,"< %d , %s >\n", tmp->key, tmp->value);
			tmp=tmp->nxt;
			tmp=tmp->next();
		}
		printf("\n\ndata being writen into output.txt\n\n");
	}
} head;



void init()
{
	int index;
	FILE *file_in,*file_out;
	char path[50];
	struct node *head;

	head = new(nothrow)node;
	head->nxt = NULL;

	printf("path:");
	scanf("%s", path);

	if ((file_in = fopen(path, "r")) == NULL)
	{
		printf("could not open(%s)\n", path);
		exit(-1);
	}
	if ((file_out = fopen(".\\output.txt", "w")) == NULL)
	{
		printf("could not open(%s)\n", path);
		exit(-1);
	}
	//
	if ((index = fread(src,1,MAX_SIZE,file_in)) <= 0)
	{
		printf("read() returned %d\n", index);
		exit(-1);
	}
	fclose(file_in);
	head->procedure(head,file_out);
	fclose(file_out);
}

int main()
{
	init();
	return 0;
}

