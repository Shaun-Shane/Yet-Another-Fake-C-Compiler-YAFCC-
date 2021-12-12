#ifndef LEX_ANALYZE_HPP
#define LEX_ANALYZE_HPP

#include <vector>
#include <string>
#include <cstdio>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "utils.hpp"

class LexAnalyzer {
   private:
    std::vector<TOKEN> token_stream;
    std::string src;  //源程序保存字符串
    int Ind;             //访问下标
    int row;             //当前行数
    int col;             //当前列数
    bool isend;          //结束标识
    std::string token;        //单词种类
    std::string value;        //当前单词
    std::string dirname;

   public:
    /*
    node()
    构造函数，为node类变量初始化
    */
    LexAnalyzer (const std::string& file) {
        Ind = 0;
        row = 1;
        col = 1;
        isend = false;
        // 通过 ifstream 读入
        std::ifstream in;
        in.open(file);

        if (!in.is_open()) throw(std::string("Open source file failed!"));

        char tmp;
        while (in >> std::noskipws >> tmp) src.push_back(tmp);
        src.push_back('#'); // 插入终结符
        src.shrink_to_fit();

        dirname = file; // 处理储源文件所在文件夹
        while (!dirname.empty() && dirname.back() != '/') dirname.pop_back();
        procedure();
    }

    /*
    void renew()
    初始化函数，初始化所需变量值
    */
    void renew() {
        value = "";
        token = "";
    }

    void next() {
        //当前字符
        char now;
        renew();

        //当前若不为结束符则持续循环
        while (!isend) {
            now = src[Ind++];
            //当前符号为空格或者换行或者tab则跳过
            if (now == '\n' || now == '	' || now == ' ') {
                if (now == '\n') row++, col = 1;
                else if (now == ' ') col++;
                else col += 4;
                continue;
            }

            //当前符号为变量
            else if ((now >= 'a' && now <= 'z') || (now >= 'A' && now <= 'Z') ||
                     (now == '_'))  // var
            {
                value += now;
                // cout<<"?"<<value<<endl;
                while ((src[Ind] >= 'a' && src[Ind] <= 'z') ||
                       (src[Ind] >= 'A' && src[Ind] <= 'Z') ||
                       (src[Ind] >= '0' && src[Ind] <= '9') ||
                       (src[Ind] == '_'))
                    value += src[Ind++];
                //检查是否为关键字
                for (int j = 0; j < 6; j++) {
                    //若关键字匹配成功则赋值并返回
                    if (value == keywords[j]) {
                        token = value;
                        break;
                    }
                }
                //若关键字匹配失败则赋值并返回
                if (token == "") token = "ID";
                break;
            }
            //当前符号为数字
            else if (now >= '0' && now <= '9') {
                value += now;
                for (; src[Ind] >= '0' && src[Ind] <= '9';) value += src[Ind++];
                token = "num";
                break;
            }
            //其余符号
            else if (now == '=') {
                // parse '==' and '='
                if (src[Ind] == '=') {
                    Ind++;
                    value = "==";
                    token = value;
                    break;
                } else {
                    value = "=";
                    token = value;
                    break;
                }
            } else if (now == '!') {
                // parse '!='
                if (src[Ind] == '=') {
                    Ind++;
                    value = "!=";
                    token = value;
                    break;
                }
            } else if (now == '<') {
                // parse '<=', '<<' or '<'
                if (src[Ind] == '=') {
                    Ind++;
                    value = "<=";
                    token = value;
                    break;
                } else {
                    value = "<";
                    token = value;
                    break;
                }
            } else if (now == '>') {
                // parse '>=', '>>' or '>'
                if (src[Ind] == '=') {
                    Ind++;
                    value = ">=";
                    token = value;
                    break;
                } else {
                    value = ">";
                    token = value;
                    break;
                }
            }
            //当前符号为#结束符，程序结束
            else if (now == '#') {
                isend = 1;
                break;
            }
            //若当前符号不在上述任何一个种类中，则不合法，输出不合法信息
            else {
                value += now;
                token = value;
                break;
            }
        }
        if (!isend) token_stream.push_back(TOKEN{token, value, row, col});
        col += value.size();
        return;
    }

    /*
    void procedure()
    作用：在结束标志未被标记前一直调用next()函数，并在结束标志被标记后结束调用
    */
    void procedure() {
        next();
        while (!isend) next();
    }

    /*
    vector<TOKEN> get_stream()
    作用：获得词法分析结果
    */
    std::vector<TOKEN> get_stream() { return token_stream; }

    void writeAnalyzeResult() {
        std::ofstream lexAnalyzeResult;
        lexAnalyzeResult.open(dirname + "lexAnalyzeResult.txt");
        lexAnalyzeResult << setiosflags(std::ios::left) << std::setw(20) << "Token";
        lexAnalyzeResult << setiosflags(std::ios::left) << std::setw(20) << "Value";
        lexAnalyzeResult << setiosflags(std::ios::left) << std::setw(20) << "Row";
        lexAnalyzeResult << setiosflags(std::ios::left) << std::setw(20) << "Col";
        lexAnalyzeResult << std::endl;
        for (const auto& i : token_stream) {
            lexAnalyzeResult << setiosflags(std::ios::left) << std::setw(20) << i.token;
            lexAnalyzeResult << setiosflags(std::ios::left) << std::setw(20) << i.value;
            lexAnalyzeResult << setiosflags(std::ios::left) << std::setw(20) << i.row;
            lexAnalyzeResult << setiosflags(std::ios::left) << std::setw(20) << i.col;
            lexAnalyzeResult << std::endl;
        }
        lexAnalyzeResult.close();
    }
};


#endif
