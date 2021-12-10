#include "SemanticAnalyzer.h"

SemanticAnalyzer::SemanticAnalyzer() {}

//将所有的符号信息放入symbolList
void SemanticAnalyzer::Add_symbol_to_list(const SemanticSymbol symbol) {}

//分析过程
void SemanticAnalyzer::Analysis(const std::string& gl,
              const std::vector<std::pair<bool, int>>& gr) {
    // Program->ExtDefList 
    if (gl == "Program") analyzeProgram(gl, gr);
    //ExtDef->VarSpecifier ID ; | FunSpecifier FunDec Block ExitFunTable_m
    else if (gl == "ExtDef") analyzeExtDef(gl, gr);
    // VarSpecifier->int
    else if (gl == "VarSpecifier") analyzeVarSpecifier(gl, gr);
    //FunSpecifier->void | int
    else if (gl == "FunSpecifier") analyzeFunSpecifier(gl, gr);
    // FunDec->ID CreateFunTable_m ( VarList )
    else if (gl == "FunDec") analyzeFunDec(gl, gr);
    // CreateFunTable_m->@
    else if (gl == "CreateFunTable_m") analyzeCreateFunTable_m(gl, gr);
    // ParamDec->VarSpecifier ID
    else if (gl == "ParamDec") analyzeParamDec(gl, gr);
    // Block->{ DefList StmtList }
    else if (gl == "Block") analyzeBlock(gl, gr);
    // Def->VarSpecifier ID ;
    else if (gl == "Def") analyzeDef(gl, gr);
    // AssignStmt->ID = Exp
    else if (gl == "AssignStmt") analyzeAssignStmt(gl, gr);
    // Exp->AddSubExp | Exp Relop AddSubExp
    else if (gl == "Exp") analyzeExp(gl, gr);
    // AddSubExp->Item | Item + Item | Item - Item
    else if (gl == "AddSubExp") analyzeAddSubExp(gl, gr);
    // Item->Factor Factor * Factor Factor / Factor
    else if (gl == "Item") analyzeItem(gl, gr);
    // Factor->num | ( Exp ) ID CallStmt
    else if (gl == "Factor") analyzeFactor(gl, gr);
    // CallStmt->ID ( CallFunCheck Args )
    else if (gl == "CallStmt") analyzeCallStmt(gl, gr);
    // CallFunCheck->@
    else if (gl == "CallFunCheck") analyzeCallFunCheck(gl, gr);
    // Args->Exp , Args | Exp | @
    else if (gl == "Args") analyzeArgs(gl, gr);
    // ReturnStmt->return Exp return
    else if (gl == "ReturnStmt") analyzeReturnStmt(gl, gr);
    // Relop-> > | < | >= | <= | == | !=
    else if (gl == "Relop") analyzeRelop(gl, gr);
    // IfStmt->if IfStmt_m1 ( Exp ) IfStmt_m2 Block IfNext
    else if (gl == "IfStmt") analyzeIfStmt(gl, gr);
    // IfStmt_m1->@
    else if (gl == "IfStmt_m1") analyzeIfStmt_m1(gl, gr);
    // IfStmt_m2->@
    else if (gl == "IfStmt_m2") analyzeIfStmt_m2(gl, gr);
    // IfNext->@ | IfStmt_next else Block
    else if (gl == "IfNext") analyzeIfNext(gl, gr);
    // IfStmt_next->@
    else if (gl == "IfStmt_next") analyzeIfStmt_next(gl, gr);
    // WhileStmt->while WhileStmt_m1 ( Exp ) WhileStmt_m2 Block
    else if (gl == "WhileStmt") analyzeWhileStmt(gl, gr);
    // WhileStmt_m1->@
    else if (gl == "WhileStmt_m1") analyzeWhileStmt_m1(gl, gr);
    // WhileStmt_m2->@
    else if (gl == "WhileStmt_m2") analyzeWhileStmt_m2(gl, gr);
    else {
        /*if (gr[0] != "@") {
            int count = gr.size();
            while (count--)
                symbolList.pop_back();
        }*/
        symbolList.push_back({gl, "", -1, -1, -1, -1});
    }
}

//打印四元式表
void SemanticAnalyzer::Print_quaternary(const std::string file_path) {}

// Program->ExtDefList 
void SemanticAnalyzer::analyzeProgram(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (mainIndex == -1) throw(std::string("Semantic analyze failed! Function main() is undefined!")); // 未定义 main 函数
    
    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    // 在最前面加入四元式
    quads.insert(quads.begin(), { 0, "j", "-", "-" , std::to_string(mainIndex) });
    symbolList.push_back({ gl, "", -1, -1, -1, -1});
}

//ExtDef->VarSpecifier ID ; | FunSpecifier FunDec Block ExitFunTable_m
void SemanticAnalyzer::analyzeExtDef(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 2) { // 函数
        SemanticSymbol identifier =
            symbolList[static_cast<int>(symbolList.size()) - 2];
        // 退出作用域
        current_table_stack.pop_back();
        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, identifier.info.value, identifier.info.row,
                              identifier.info.col, identifier.table_index,
                              identifier.innerIndex});
    } else if (gr.size() == 3) { // 变量
        // VarSpecifier ID ; -2 得到 ID, -3 得到 int
        SemanticSymbol specifier = symbolList[symbolList.size() - 3];  // int
        SemanticSymbol identifier = symbolList[symbolList.size() - 2];  //变量名

        //用于判断该变量是否在当前层已经定义
        bool existed = false;
        Semantic_symbol_table current_table =
            tables[current_table_stack.back()];
        if (current_table.findSymbol(identifier.info.value) != -1) {
            std::string err;
            err += "Semantic analyze failed! Multiple defination on row " + std::to_string(identifier.info.row)
            + " col " + std::to_string(identifier.info.col) + " variable " + identifier.info.value;
            throw(err);
        }

        // 将这一变量加入table
        IdentifierInfo variable;
        variable.identifierName = identifier.info.value;  //变量名
        variable.identifierType = IdentifierInfo::Variable;
        variable.specifierType = specifier.info.value;  // int

        //加入table
        tables[current_table_stack.back()].addSymbol(variable);

        // symbolList更新
        
        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back(
            {gl, identifier.info.value, identifier.info.row,
             identifier.info.col, current_table_stack.back(),
             int(tables[current_table_stack.back()].table.size() - 1)});
    } else throw(std::string("Semantic analyze error! In analyzeExtDef" + __LINE__));
}

void SemanticAnalyzer::analyzeVarSpecifier(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeFunSpecifier(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}
void SemanticAnalyzer::analyzeFunDec(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeCreateFunTable_m(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeParamDec(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeBlock(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeDef(const std::string& gl,
                                  const std::vector<std::pair<bool, int>>& gr) {
}

void SemanticAnalyzer::analyzeAssignStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeExp(const std::string& gl,
                                  const std::vector<std::pair<bool, int>>& gr) {
}

void SemanticAnalyzer::analyzeAddSubExp(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeItem(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeFactor(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeCallStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeCallFunCheck(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeArgs(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeReturnStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeRelop(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeIfStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeIfStmt_m1(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeIfStmt_m2(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeIfNext(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeIfStmt_next(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeWhileStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeWhileStmt_m1(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}

void SemanticAnalyzer::analyzeWhileStmt_m2(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {}