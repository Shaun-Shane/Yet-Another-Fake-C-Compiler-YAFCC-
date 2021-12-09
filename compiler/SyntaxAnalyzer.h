#ifndef SYNTAX_ANALYZER_H
#define SYNTAX_ANALYZER_H

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


#include "LexAnalyzer.hpp"

class LR1 {
   private:
    std::string codePath;
    std::string S;  // 文法起始符号 拓展文法起始符应为 S'
    std::vector<std::string> VT;  // 终结符
    std::vector<std::string> VN;  // 非终结符

    std::vector<std::pair<std::string, std::vector<std::pair<bool, int>>>>
        P;  // S -> X1X2X3...
    std::map<std::string, std::vector<std::size_t>>
        G;  // 产生式 如 Si->branch1 | branch2 ...

    std::map<std::string, std::set<int>> first;   // first 集
    std::map<std::tuple<int, int, int>, int> GO;  // GO(I, type, idx) = j;

    std::vector<std::map<std::string, std::pair<int, int>>> ACTION;
    // fir == -1:err   0:acc   1:sj   2:rj
    std::vector<std::map<std::string, int>> GOTO;
    std::vector<std::vector<std::pair<int, std::string>>> SyntaxTree;
    std::vector<std::set<Item>> I;  // 项目集 I0, I1, I2, ...

    // output functions
   private:
    void printVN_VT_S(); // for debug 输出变元 终结符
    void printG(); // for debug 输出所有产生式
    void writeFirst(); // 输出 first 集和
    void writeC_GO(); // 输出文法的项目集族 C 和 GO(I, X)
    void writeLR1Table(); // 输出 LR1 分析
    void writeSyntaxTree(); // 输出语法树信息

    // lr1 private functions
   private:
    void readGrammar(); // 读取文法
    void getFirstSet(); // 生成 first 集和
    std::set<int> getFirstVT(const std::vector<std::pair<bool, int>>& l, std::size_t right, std::size_t dotPos); // 获得 first(βa) 的终结符;
    std::string P_ToString(const std::string& gl, const std::vector<std::pair<bool, int>>& vec); // 返回产生式对应字符串
    void genC(); // 生成 LR(1) 项目集族 C
    void genClosure(std::set<Item>& Ii); // 生成项目集 Ii 的闭包
    int foundI(const std::set<Item>& Itmp); // 判断项目集 Itmp 是否存在，返回项目集编号 brute force
    void genLR1Table();  // 生成LR(1)分析表

   public:
    LR1() = default;

    LR1(const std::string& file);

    void parse(const std::vector<TOKEN>& _token_stream);
};

#endif