#include <bits/stdc++.h>

using namespace std;

#define MAX_SIZE 102400	//�������Դ���򳤶� 
#define MAX_SINGLE 1024	//��󵥴ʳ��� 

//�ؼ����б� 
string keywords[6]=
{
	"int","void","if","else","while","return"
};

//Դ���򱣴��ַ��� 
char src[MAX_SIZE];

/*
struct node
���ã����е���ʶ������ 
*/ 
struct node
{
	bool isend;				//������ʶ 
	int Ind;				//�����±� 
	int key;				//���ʱ�� 
	string type;			//�������� 
	string value;			//��ǰ���� 

	/*
	node()
	���캯����Ϊnode�������ʼ�� 
	*/
	node()
	{
		Ind=0;
	} 
	
	/*
	void renew()
	��ʼ����������ʼ���������ֵ 
	*/
	void renew()
	{
		isend=0;key=0;
		value="";
		type="";
	}
	
	node* next()
	{
		//��ǰ�ַ� 
		char now; 
		renew();
		
		//��ǰ����Ϊ�����������ѭ�� 
		while(!isend)
		{
			now=src[Ind++];
			//printf("now=%c\n",now);
			//��ǰ����Ϊ�ո���߻��л���tab������ 
			if (now == '\n'||now=='	'||now==' ')
				continue;
			
			//��ǰ����Ϊ���� 
			else if ((now >= 'a' && now <= 'z') || (now >= 'A' && now <= 'Z') || (now == '_'))// var
			{
				value+=now;
				//cout<<"?"<<value<<endl;
				while ((src[Ind] >= 'a' && src[Ind] <= 'z') || (src[Ind] >= 'A' && src[Ind] <= 'Z') || (src[Ind] >= '0' && src[Ind] <= '9') || (src[Ind] == '_'))
				{
					value+= src[Ind++];
					//printf("1");
				}
				//����Ƿ�Ϊ�ؼ��� 
				for (int j = 0; j < 6; j++)
				{
					//���ؼ���ƥ��ɹ���ֵ������ 
					if (value==keywords[j])
					{
						key = j + 1;
						type=value;
						break;
					}
				}
				//���ؼ���ƥ��ʧ����ֵ������ 
				if(key==0)
				{
					key=7;
					type="ID";
				}
				break;
			}
			//��ǰ����Ϊ���� 
			else if (now >= '0' && now <= '9')
			{
				value+=now;
				for (; src[Ind] >= '0' && src[Ind] <= '9';)
				{
					value+=src[Ind++];
				}
				key = 8;
				type= "num";
				break;
			}
			//�������
			else if (now == '=')
			{
				// parse '==' and '='
				if (src[Ind] == '=')
				{
					Ind++;
					value="==";
					type = value;
					break;
				}
				else
				{
					key=9;
					value="=";
					type = value;
					break;
				}
			}
			else if (now == '!')
			{
				// parse '!='
				if (src[Ind] == '=')
				{
					Ind++;
					key=9;
					value="!=";
					type = value;
					break;
				}
			}
			else if (now == '<')
			{
				// parse '<=', '<<' or '<'
				if (src[Ind] == '=')
				{
					Ind++;
					key=9;
					value="<=";
					type = value;
					break;
				}
				else
				{
					key=9;
					value="<";
					type = value;
					break;
				}
			}
			else if (now == '>')
			{
				// parse '>=', '>>' or '>'
				if (src[Ind] == '=')
				{
					Ind++;
					key=9;
					value=">=";
					type = value;
					break;
				}
				else
				{
					key=9;
					value=">";
					type = value;
					break;
				}
			}
			//��ǰ����Ϊ#��������������� 
			else if(now=='#')
			{
				isend=1;
				break;
			}
			//����ǰ���Ų��������κ�һ�������У��򲻺Ϸ���������Ϸ���Ϣ 
			else
			{
				key=9;
				value+=now;
				type = value;
				break;
			}
		}
		return this;
	}
	/*
	void procedure(struct node *tmp)
	���ã��ڽ�����־δ�����ǰһֱ���� ��next()���������ڽ�����־����Ǻ�������� 
	*/ 
	void procedure(struct node *tmp)
	{
		//printf("result:\n");
		//��ȡ��һ������ 
		tmp=tmp->next();
		while (!tmp->isend)
		{
			//printf("%d->%d %s->ID\n", tmp->key,tmp->type, tmp->value);
			//fprintf(file_out,"%s->%s\n", tmp->value, tmp->value);
			cout<<tmp->type<<endl;
			tmp=tmp->next();
		}
		cout << "#" << endl;
		//printf("\n\ndata being writen into output.txt\n\n");
	}
};


/*
void init()
���ã������ļ����ʣ�����node��procedure��������������޲����������ӿ���̨���������ļ���ַ������ļ�Ĭ���������ļ���ͬĿ¼����Ϊoutput.txt 
*/ 
char path[50];
void init()
{
	//�����ļ��������ͱ��� 
	FILE *file_in;
	
	struct node *head;
	head = new(nothrow)node;

	//�ļ�·�� 
	
	//�����ļ����쳣 
	if ((file_in = fopen(path, "r")) == NULL)
	{
		printf("could not open(%s)\n", path);
		exit(-1);
	}
	
	
	//�ļ������쳣���� 
	int index;
	if ((index = fread(src,1,MAX_SIZE,file_in)) <= 0)
	{
		printf("read() returned %d\n", index);
		exit(-1);
	}
	
	freopen("../SyntaxAnalyzer/input.txt","w",stdout);
	//���дʷ����� 
	head->procedure(head);
	delete head; 
	
	//�ر��ļ����� 
	fclose(file_in);
	fclose(stdout);
}

int main(int argc, char *argv[])
{
	strcpy(path, argv[1]);
	init();
	return 0;
}

