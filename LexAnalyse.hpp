#pragma once
#include <bits/stdc++.h>

#define MAX_SIZE 102400	//最大输入源程序长度 

//关键字列表 
string keywords[6]=
{
	"int","void","if","else","while","return"
};


//标识符
struct TOKEN 
{
	string token; 	//token类型
	string value; 	//值
	int row;		//行
	int col;		//列
};

/*
struct node
作用：进行单词识别和输出 
*/ 
class LexAnalyse
{
private:
	
	vector<TOKEN> token_stream;
	char src[MAX_SIZE];		//源程序保存字符串 
	int Ind;				//访问下标 
	int row;				//当前行数
	int col;				//当前列数 
	bool isend;				//结束标识 
	string token;			//单词种类 
	string value;			//当前单词 

public:	
	/*
	node()
	构造函数，为node类变量初始化 
	*/
	LexAnalyse(char * in)
	{
		Ind=0;
		row=1;
		col=1;
		//定义文件数据类型变量 
		FILE *file_in;
		
		//输入文件打开异常 
		if ((file_in = fopen(in, "r")) == NULL)
		{
			printf("could not open(%s)\n", in);
			exit(-1);
		}
		
		//文件读入异常跳出 
		int id;
		if ((id = fread(src,1,MAX_SIZE,file_in)) <= 0)
		{
			printf("read() returned %d\n", id);
			exit(-1);
		}
		//关闭文件管理 
		fclose(file_in);
		procedure();
	} 
	
	/*
	void renew()
	初始化函数，初始化所需变量值 
	*/
	void renew()
	{
		value="";
		token="";
	}
	
	void next()
	{
		//当前字符 
		char now; 
		renew();
		
		//当前若不为结束符则持续循环 
		while(!isend)
		{
			now=src[Ind++];
			//当前符号为空格或者换行或者tab则跳过 
			if (now == '\n'||now=='	'||now==' ')
			{
				if(now == '\n')
					row++,col=1;
				else col++;
				continue;
			}
			
			//当前符号为变量 
			else if ((now >= 'a' && now <= 'z') || (now >= 'A' && now <= 'Z') || (now == '_'))// var
			{
				value+=now;
				//cout<<"?"<<value<<endl;
				while ((src[Ind] >= 'a' && src[Ind] <= 'z') || (src[Ind] >= 'A' && src[Ind] <= 'Z') || (src[Ind] >= '0' && src[Ind] <= '9') || (src[Ind] == '_'))
					value+= src[Ind++];
				//检查是否为关键字 
				for (int j = 0; j < 6; j++)
				{
					//若关键字匹配成功则赋值并返回 
					if (value==keywords[j])
					{
						token=value;
						break;
					}
				}
				//若关键字匹配失败则赋值并返回 
				if(token=="")
					token="ID";
				break;
			}
			//当前符号为数字 
			else if (now >= '0' && now <= '9')
			{
				value+=now;
				for (; src[Ind] >= '0' && src[Ind] <= '9';)
					value+=src[Ind++];
				token= "num";
				break;
			}
			//其余符号
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
			//当前符号为#结束符，程序结束 
			else if(now=='#')
			{
				isend=1;
				break;
			}
			//若当前符号不在上述任何一个种类中，则不合法，输出不合法信息 
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
	作用：在结束标志未被标记前一直调用next()函数，并在结束标志被标记后结束调用 
	*/ 
	void procedure()
	{
		next();
		while(!isend)
			next();
	}
	
	/*
	vector<TOKEN> get_stream()
	作用：获得词法分析结果 
	*/
	vector<TOKEN> get_stream()
	{
		return token_stream;
	}
	
};
/* 
//调用方法实例 
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

