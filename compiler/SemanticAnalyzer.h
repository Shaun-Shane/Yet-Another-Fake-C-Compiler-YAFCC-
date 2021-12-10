#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <string>
#include <vector>
#include <utility>

#include "LexAnalyzer.hpp"
#include "utils.hpp"

//语义分析
class SemanticAnalyzer {
   private:
    std::vector<std::string>* ptrVT = nullptr;  // 终结符 vector 指针
    std::vector<std::string>* ptrVN = nullptr;  // 非终结符 vector 指针

   public:
    std::vector<Quad> quads;        // 四元式
    int mainIndex;                      // main函数对应的四元式标号
    int backpatching_level;              // 回填层次
    std::vector<int> backpatching_list;  // 回填列表
    int next_quaternary_index;           // 下一个四元式标号
    int tmp_var_count;                   // 临时变量计数
    std::vector<SemanticSymbol> symbolList;    // 语义分析过程的符号流
    std::vector<Semantic_symbol_table> tables;  // 程序所有符号表
    std::vector<int> current_table_stack;  //当前作用域对应的符号表索引栈

    //构造函数
    SemanticAnalyzer();
    //将所有的符号信息放入symbol_list
    void Add_symbol_to_list(const SemanticSymbol symbol);
    //分析过程
    void Analysis(const std::string& gl, const std::vector<std::pair<bool, int>>& gr);
    //打印四元式表
    void Print_quaternary(const std::string file_path);

   private:
    void analyzeProgram(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // Program->ExtDefList 
    void analyzeExtDef(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); //ExtDef->VarSpecifier ID ; | FunSpecifier FunDec Block ExitFunTable_m
    void analyzeVarSpecifier(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // VarSpecifier->int
    void analyzeFunSpecifier(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); //FunSpecifier->void | int
    void analyzeFunDec(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // FunDec->ID CreateFunTable_m ( VarList )
    void analyzeCreateFunTable_m(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // CreateFunTable_m->@
    void analyzeParamDec(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // ParamDec->VarSpecifier ID
    void analyzeBlock(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // Block->{ DefList StmtList }
    void analyzeDef(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // Def->VarSpecifier ID ;
    void analyzeAssignStmt(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // AssignStmt->ID = Exp
    void analyzeExp(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // Exp->AddSubExp | Exp Relop AddSubExp
    void analyzeAddSubExp(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // AddSubExp->Item | Item + Item | Item - Item
    void analyzeItem(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // Item->Factor Factor * Factor Factor / Factor
    void analyzeFactor(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // Factor->num | ( Exp ) ID CallStmt
    void analyzeCallStmt(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // CallStmt->ID ( CallFunCheck Args )
    void analyzeCallFunCheck(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // CallFunCheck->@
    void analyzeArgs(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // Args->Exp , Args | Exp | @
    void analyzeReturnStmt(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // ReturnStmt->return Exp return
    void analyzeRelop(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // Relop-> > | < | >= | <= | == | !=
    void analyzeIfStmt(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // IfStmt->if IfStmt_m1 ( Exp ) IfStmt_m2 Block IfNext
    void analyzeIfStmt_m1(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // IfStmt_m1->@
    void analyzeIfStmt_m2(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // IfStmt_m2->@
    void analyzeIfNext(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // IfNext->@ | IfStmt_next else Block
    void analyzeIfStmt_next(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // IfStmt_next->@
    void analyzeWhileStmt(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // WhileStmt->while WhileStmt_m1 ( Exp ) WhileStmt_m2 Block
    void analyzeWhileStmt_m1(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // WhileStmt_m1->@
    void analyzeWhileStmt_m2(const std::string& gl, const std::vector<std::pair<bool, int>>& gr); // WhileStmt_m2->@
};
#endif