#pragma once
#include <bits/stdc++.h>

#define MAX_SIZE 102400	//�������Դ���򳤶� 

//�ؼ����б� 
string keywords[6]=
{
	"int","void","if","else","while","return"
};


//��ʶ��
struct TOKEN 
{
	string token; 	//token����
	string value; 	//ֵ
	int row;		//��
	int col;		//��
};

/*
struct node
���ã����е���ʶ������ 
*/ 
class LexAnalyse
{
private:
	
	vector<TOKEN> token_stream;
	char src[MAX_SIZE];		//Դ���򱣴��ַ��� 
	int Ind;				//�����±� 
	int row;				//��ǰ����
	int col;				//��ǰ���� 
	bool isend;				//������ʶ 
	string token;			//�������� 
	string value;			//��ǰ���� 

public:	
	/*
	node()
	���캯����Ϊnode�������ʼ�� 
	*/
	LexAnalyse(char * in)
	{
		Ind=0;
		row=1;
		col=1;
		//�����ļ��������ͱ��� 
		FILE *file_in;
		
		//�����ļ����쳣 
		if ((file_in = fopen(in, "r")) == NULL)
		{
			printf("could not open(%s)\n", in);
			exit(-1);
		}
		
		//�ļ������쳣���� 
		int id;
		if ((id = fread(src,1,MAX_SIZE,file_in)) <= 0)
		{
			printf("read() returned %d\n", id);
			exit(-1);
		}
		//�ر��ļ����� 
		fclose(file_in);
		procedure();
	} 
	
	/*
	void renew()
	��ʼ����������ʼ���������ֵ 
	*/
	void renew()
	{
		value="";
		token="";
	}
	
	void next()
	{
		//��ǰ�ַ� 
		char now; 
		renew();
		
		//��ǰ����Ϊ�����������ѭ�� 
		while(!isend)
		{
			now=src[Ind++];
			//��ǰ����Ϊ�ո���߻��л���tab������ 
			if (now == '\n'||now=='	'||now==' ')
			{
				if(now == '\n')
					row++,col=1;
				else col++;
				continue;
			}
			
			//��ǰ����Ϊ���� 
			else if ((now >= 'a' && now <= 'z') || (now >= 'A' && now <= 'Z') || (now == '_'))// var
			{
				value+=now;
				//cout<<"?"<<value<<endl;
				while ((src[Ind] >= 'a' && src[Ind] <= 'z') || (src[Ind] >= 'A' && src[Ind] <= 'Z') || (src[Ind] >= '0' && src[Ind] <= '9') || (src[Ind] == '_'))
					value+= src[Ind++];
				//����Ƿ�Ϊ�ؼ��� 
				for (int j = 0; j < 6; j++)
				{
					//���ؼ���ƥ��ɹ���ֵ������ 
					if (value==keywords[j])
					{
						token=value;
						break;
					}
				}
				//���ؼ���ƥ��ʧ����ֵ������ 
				if(token=="")
					token="ID";
				break;
			}
			//��ǰ����Ϊ���� 
			else if (now >= '0' && now <= '9')
			{
				value+=now;
				for (; src[Ind] >= '0' && src[Ind] <= '9';)
					value+=src[Ind++];
				token= "num";
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
					token = value;
					break;
				}
				else
				{
					value="=";
					token = value;
					break;
				}
			}
			else if (now == '!')
			{
				// parse '!='
				if (src[Ind] == '=')
				{
					Ind++;
					value="!=";
					token = value;
					break;
				}
			}
			else if (now == '<')
			{
				// parse '<=', '<<' or '<'
				if (src[Ind] == '=')
				{
					Ind++;
					value="<=";
					token = value;
					break;
				}
				else
				{
					value="<";
					token = value;
					break;
				}
			}
			else if (now == '>')
			{
				// parse '>=', '>>' or '>'
				if (src[Ind] == '=')
				{
					Ind++;
					value=">=";
					token = value;
					break;
				}
				else
				{
					value=">";
					token = value;
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
				value += now;
				token = value;
				break;
			}
		}
		//cout<<token<<"  "<<value<<endl;
		token_stream.push_back(TOKEN{token,value,row,col});
		col+=value.size();
		return;
	}
	
	/*
	void procedure()
	���ã��ڽ�����־δ�����ǰһֱ����next()���������ڽ�����־����Ǻ�������� 
	*/ 
	void procedure()
	{
		next();
		while(!isend)
			next();
	}
	
	/*
	vector<TOKEN> get_stream()
	���ã���ôʷ�������� 
	*/
	vector<TOKEN> get_stream()
	{
		return token_stream;
	}
	
};
/* 
//���÷���ʵ�� 
int main()
{
	LexAnalyse* lex = new LexAnalyse("test.cpp");
	vector<TOKEN> token_stream=lex->get_stream();
	for(int i=0;i<token_stream.size();i++)
	{
		cout<<token_stream[i].token<<"  "<<token_stream[i].value<<"  "<<token_stream[i].row<<"  "<<token_stream[i].col<<endl;
	}
	return 0;
}*/ 

