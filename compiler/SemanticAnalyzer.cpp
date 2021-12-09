#include "SemanticAnalyzer.h"

SemanticAnalyzer::SemanticAnalyzer() {}

//将所有的符号信息放入symbol_list
void Add_symbol_to_list(const SemanticSymbol symbol) {}

//分析过程
void Analysis(const std::string gl,
              const std::vector<std::string> production_right) {}

//打印四元式表
void Print_quaternary(const std::string file_path) {}

void SemanticAnalyzer::Analysis_production_Program(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_ExtDef(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_VarSpecifier(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_FunSpecifier(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}
void SemanticAnalyzer::Analysis_production_FunDec(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_CreateFunTable_m(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_ParamDec(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_Block(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_Def(
    const std::string& gl, const std::vector<std::string> gr) {}

void SemanticAnalyzer::Analysis_production_AssignStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_Exp(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_AddSubExp(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_Item(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_Factor(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_CallStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_CallFunCheck(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_Args(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_ReturnStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_Relop(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_IfStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_IfStmt_m1(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_IfStmt_m2(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_IfNext(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_IfStmt_next(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_WhileStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_WhileStmt_m1(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::Analysis_production_WhileStmt_m2(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}