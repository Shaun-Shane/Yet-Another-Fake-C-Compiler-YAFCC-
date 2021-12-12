#include "SemanticAnalyzer.h"

SemanticAnalyzer::SemanticAnalyzer() = default;

SemanticAnalyzer::SemanticAnalyzer(std::vector<std::string>* _ptrVT, std::vector<std::string>* _ptrVN) {
    //创建全局的符号表
    tables.push_back(SymTable(SymTable::GlobalTable, "global table"));
    //当前作用域为全局作用域
    currentTableStack.push_back(0);
    //创建临时变量表
    tables.push_back(SymTable(SymTable::TempTable, "temp variable table"));

    //从标号1开始生成四元式标号；0号用于 (j, -, -, main_address)
    nextQuadsId = 1;
    //main函数的标号先置为-1
    mainIndex = -1;
    //初始回填层次为0，表示不需要回填
    backpatchLevel = 0;
    //临时变量计数器归零
    tmpVarCount = 0;
    // 设置 VN VT 指针
    ptrVT = _ptrVT, ptrVN = _ptrVN;
}

//将所有的符号信息放入symbol_list
void SemanticAnalyzer::addSymbol(const SemanticSymbol& symbol) {
    symbolList.push_back(symbol);
}

//分析过程
void SemanticAnalyzer::analyze(const std::string& gl,
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
        if (!gr[0].first && (*ptrVT)[gr[0].second] != "@")
            for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, "", -1, -1, -1, -1});
    }
}

//打印四元式表
void SemanticAnalyzer::printQuads(const std::string& dirname) {
    std::ofstream quad_out;
    quad_out.open(dirname + "Quads.txt", std::ios::out);

    for (const auto& q: quads)
        quad_out << q.index << "(" << q.ope << ", " << q.arg1 << ", " << q.arg2 << ", " << q.result << ")" << std::endl;

    std::cout << "Write Quad done!\n";
}

// Program->ExtDefList
void SemanticAnalyzer::analyzeProgram(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (mainIndex == -1) { // 未声明 main 函数
        throw(std::string("Semantic analyze failed! function main() is not defined."));
    }

    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    // 在最前面插入四元式
    quads.insert(quads.begin(), {0, "j", "-", "-", std::to_string(mainIndex)});
    symbolList.push_back({{gl, "", -1, -1}, -1, -1});
}

//ExtDef->VarSpecifier ID ; | FunSpecifier FunDec Block
void SemanticAnalyzer::analyzeExtDef(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {

    if (gr[0].first && (*ptrVN)[gr[0].second] == "FunSpecifier") { // 定义函数
        SemanticSymbol identifier = symbolList[static_cast<int>(symbolList.size()) - 2]; // FunDec
        // 推出作用域
        currentTableStack.pop_back();
        // update symbollist
        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({{gl, identifier.info.value, identifier.info.row, identifier.info.col},
                              identifier.tableOffset, identifier.offset});
    } else if (gr[0].first && (*ptrVN)[gr[0].second] == "VarSpecifier") { // 定义变量
        SemanticSymbol specifier =
            symbolList[static_cast<int>(symbolList.size()) - 3]; // VarSpecifier
        SemanticSymbol identifier =
            symbolList[static_cast<int>(symbolList.size()) - 2]; // ID

        // 获取当前层符号表
        SymTable curTable =
            tables[currentTableStack.back()];
        // 判断变量是否重复定义
        if (curTable.findSym(identifier.info.value) != -1) {
            std::string err =
                "Semantic Analyze failed! Multiple definination on row" +
                std::to_string(identifier.info.row) + "col " +
                std::to_string(identifier.info.col) + identifier.info.value;
            throw(err);
        }

        // 将变量加入 table
        Identifier variable;
        variable.identifierName = identifier.info.value;
        variable.identifierType = Identifier::Variable;
        variable.specifierType = specifier.info.value;

        tables[currentTableStack.back()].addSym(variable);

        // update symbollist
        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back(
            {{gl, identifier.info.value, identifier.info.row, identifier.info.col},
             currentTableStack.back(), static_cast<int>(tables[currentTableStack.back()].table.size()) - 1});
    } else
        throw(std::string(
            "Semantic analyze failed! In SemanticAnalyzer::analyzeExtDef(const "
            "std::string& gl, const std::vector<std::pair<bool, int>>& gr)"));
}

// VarSpecifier->int
void SemanticAnalyzer::analyzeVarSpecifier(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol specifier = symbolList.back(); // int
    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    symbolList.push_back({{gl, specifier.info.value, specifier.info.row, specifier.info.col}, -1, -1});
}

// FunSpecifier->void | int
void SemanticAnalyzer::analyzeFunSpecifier(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol specifier = symbolList.back(); // void / int
    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    symbolList.push_back({{gl, specifier.info.value, specifier.info.row, specifier.info.col}, -1, -1});
}

// FunDec->ID CreateFunTable_m ( VarList )
void SemanticAnalyzer::analyzeFunDec(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol specifier = symbolList[static_cast<int>(symbolList.size()) - 4]; // ID
    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    symbolList.push_back({{gl, specifier.info.value, specifier.info.row, specifier.info.col}, 
                            specifier.tableOffset, specifier.offset});
}

// CreateFunTable_m->@
void SemanticAnalyzer::analyzeCreateFunTable_m(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol identifier = symbolList.back(); // 函数返回值
    SemanticSymbol specifier = symbolList[static_cast<int>(symbolList.size()) - 2]; // 函数名

    // 在全局的table判断函数名是否重定义
    if (tables[0].findSym(identifier.info.value) != -1) {
        std::string err =
            "Semantic analyze failed! Multiple definition on row " +
            std::to_string(identifier.info.row) + " col " +
            std::to_string(identifier.info.col) + " " + identifier.info.value;
        throw(err);
    }

    // 新建函数表
    currentTableStack.push_back(tables.size());
    tables.push_back(SymTable(SymTable::FunctionTable, identifier.info.value));
    // 在全局符号表创建当前函数的符号项 (参数个数和入口地址会进行回填)
    tables[0].addSym({Identifier::Function, specifier.info.value,
                         identifier.info.value} /*0, 0, static_cast<int>(tables.size()) - 1}*/);
    // 返回值
    Identifier return_value;
    return_value.identifierType = Identifier::ReturnVar;
    return_value.identifierName = tables.back().tableName + "_return_value";
    return_value.specifierType = specifier.info.value;
    
    // 记录 main 函数 Index，用于判断 main 函数是否定义
    if (identifier.info.value == "main") mainIndex = nextQuadsId; 
    
    // 加入四元式
    quads.push_back({nextQuadsId++, identifier.info.value, "-", "-", "-"});
    tables[currentTableStack.back()].addSym(return_value);
    symbolList.push_back({gl, identifier.info.value, identifier.info.row,
                          identifier.info.col, 0, static_cast<int>(tables[0].table.size()) - 1});
}

// ParamDec->VarSpecifier ID
void SemanticAnalyzer::analyzeParamDec(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol identifier = symbolList.back(); // 变量名
    SemanticSymbol specifier = symbolList[static_cast<int>(symbolList.size()) - 2]; // 变量类型
    SymTable& function_table = tables[currentTableStack.back()]; // 获得当前函数表

    // 参数重复定义
    if (function_table.findSym(identifier.info.value) != -1) {
        std::string err =
            "Semantic analyze failed! Multiple param definition on row " +
            std::to_string(identifier.info.row) + " col " +
            std::to_string(identifier.info.col) + " " + identifier.info.value;
        throw(err);
    }
    
    // 加入形参变量
    int new_position = function_table.addSym({Identifier::Variable, specifier.info.value, identifier.info.value}/*, -1, -1, -1}*/);
    // 获取当前函数在全局符号表的 index 形参个数 + 1
    tables[0].table[tables[0].findSym(function_table.tableName)].functionParameterCount++;

    quads.push_back({nextQuadsId ++, "defpar", "-" , "-", identifier.info.value });

    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    symbolList.push_back({gl, identifier.info.value, identifier.info.row,identifier.info.col, currentTableStack.back(), new_position});
}

// Block->{ DefList StmtList }
void SemanticAnalyzer::analyzeBlock(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    symbolList.push_back({gl, std::to_string(nextQuadsId), -1, -1, -1,-1 });
}

// Def->VarSpecifier ID ;
void SemanticAnalyzer::analyzeDef(const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol identifier = symbolList[static_cast<int>(symbolList.size()) - 2]; // ID
    SemanticSymbol specifier = symbolList[static_cast<int>(symbolList.size()) - 3]; // VarSpecifier
    SymTable& curTable = tables[currentTableStack.back()];

    // 重复定义
    if (curTable.findSym(identifier.info.value) != -1) {
        std::string err =
            "Semantic analyze failed! Multiple variable definition on row " +
            std::to_string(identifier.info.row) + " col " +
            std::to_string(identifier.info.col) + " " + identifier.info.value;
        throw(err);
    }

    curTable.addSym({Identifier::Variable, specifier.info.value, identifier.info.value}/*, -1, -1, -1}*/);

    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    symbolList.push_back({gl, identifier.info.value, identifier.info.row, identifier.info.col,
                        currentTableStack.back(), static_cast<int>(tables[currentTableStack.back()].table.size()) - 1});
}

// AssignStmt->ID = Exp
void SemanticAnalyzer::analyzeAssignStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol identifier = symbolList[static_cast<int>(symbolList.size()) - 3]; // ID
    SemanticSymbol exp = symbolList.back(); // Exp

    // 检查 ID 是否存在
    bool existed = false;
    int tableOffset = -1, index = -1;

    // 从当前层开始向低层遍历
    for (int scope = static_cast<int>(currentTableStack.size()) - 1; scope >= 0; scope--) {
        auto curTable = tables[currentTableStack[scope]];
        if ((index = curTable.findSym(identifier.info.value)) != -1) {
            existed = true;
            tableOffset = currentTableStack[scope];
            break;
        }
    }
    if (existed == false) {
        std::string err =
            "Semantic analyze failed! Identifier is undefined on row " +
            std::to_string(identifier.info.row) + " col " +
            std::to_string(identifier.info.col) + " " + identifier.info.value;
        throw(err);
    }

    quads.push_back(
        {nextQuadsId++, "=", exp.info.value, "-", identifier.info.value});

    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    symbolList.push_back({gl, identifier.info.value, identifier.info.row,
                          identifier.info.col, tableOffset, index});
}

// Exp->AddSubExp | Exp Relop AddSubExp
void SemanticAnalyzer::analyzeExp(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 1) {
        SemanticSymbol exp = symbolList.back();
        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({ gl, exp.info.value, exp.info.row, exp.info.col, exp.tableOffset, exp.offset });
    } else if (gr.size() == 3) {
        SemanticSymbol sub_exp1 =
            symbolList[static_cast<int>(symbolList.size()) - 3];  // addSubExp
        SemanticSymbol op =
            symbolList[static_cast<int>(symbolList.size()) - 2];  // relop
        SemanticSymbol sub_exp2 =
            symbolList[static_cast<int>(symbolList.size()) - 1];  // Exp
        int next_label_num = nextQuadsId++;
        // 新建多个临时变量
        std::string new_tmp_var = "T" + std::to_string(tmpVarCount++);
        quads.push_back({next_label_num, "j" + op.info.value,
                         sub_exp1.info.value, sub_exp2.info.value,
                         std::to_string(next_label_num + 3)});
        quads.push_back({nextQuadsId++, "=", "0", "-", new_tmp_var});
        quads.push_back({nextQuadsId++, "j", "-", "-",
                         std::to_string(next_label_num + 4)});
        quads.push_back({nextQuadsId++, "=", "1", "-", new_tmp_var});

        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, new_tmp_var, -1, -1, -1, -1});
    } else
        throw(std::string(
            "Semantic analyze failed! In SemanticAnalyzer::analyzeExp(const "
            "std::string& gl, const std::vector<std::pair<bool, int>>& gr)"));
}

// AddSubExp->Item | Item + Item | Item - Item
void SemanticAnalyzer::analyzeAddSubExp(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 1) { // AddSubExp->item
        SemanticSymbol item = symbolList.back(); // Item
        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, item.info.value, item.info.row, item.info.col,
                              item.tableOffset, item.offset});
    } else if (gr.size() == 3) { // AddSubExp->Item + Item | Item - Item
        SemanticSymbol sub_item1 =
            symbolList[static_cast<int>(symbolList.size()) - 3];
        SemanticSymbol op = symbolList[static_cast<int>(symbolList.size()) - 2]; // + or -
        SemanticSymbol sub_item2 =
            symbolList[static_cast<int>(symbolList.size()) - 1];
        std::string new_tmp_var = "T" + std::to_string(tmpVarCount++);
        quads.push_back({nextQuadsId++, op.info.value, sub_item1.info.value,
                         sub_item2.info.value, new_tmp_var});

        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, new_tmp_var, -1, -1, -1, -1});
    } else
        throw(std::string(
                "Semantic analyze failed! In SemanticAnalyzer::analyzeAddSubExp(const "
                "std::string& gl, const std::vector<std::pair<bool, int>>& gr)"));
}

// Item->Factor | Factor * Factor | Factor / Factor
void SemanticAnalyzer::analyzeItem(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 1) {
        SemanticSymbol factor = symbolList.back();
        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, factor.info.value, factor.info.row, factor.info.col,
                              factor.tableOffset, factor.offset});
    } else if (gr.size() == 3) {
        SemanticSymbol sub_factor1 =
            symbolList[static_cast<int>(symbolList.size()) - 3]; // Factor
        SemanticSymbol op = symbolList[static_cast<int>(symbolList.size()) - 2]; // * or /
        SemanticSymbol sub_factor2 =
            symbolList[static_cast<int>(symbolList.size()) - 1]; // Factor
        std::string new_tmp_var = "T" + std::to_string(tmpVarCount++);
        quads.push_back({nextQuadsId++, op.info.value, sub_factor1.info.value,
                         sub_factor2.info.value, new_tmp_var});

        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, new_tmp_var, -1, -1, -1, -1});
    } else
        throw(std::string(
            "Semantic analyze failed! In SemanticAnalyzer::analyzeItem(const "
            "std::string& gl, const std::vector<std::pair<bool, int>>& gr)"));
}

// Factor->num | ( Exp ) | ID | CallStmt
void SemanticAnalyzer::analyzeFactor(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 1) {
        SemanticSymbol exp = symbolList.back();

        if (!gr[0].first && (*ptrVT)[gr[0].second] == "ID") { // Factor->ID
            bool existed = false; // 判断 ID 是否未定义过
            for (int scope = currentTableStack.size() - 1;
                 scope >= 0; scope--) {
                auto curTable = tables[currentTableStack[scope]];
                if (curTable.findSym(exp.info.value) != -1) {
                    existed = true;
                    break;
                }
            }
            if (existed == false) {
                std::string err =
                    "Semantic analyze failed! Identifier undefined on "
                    "row " + std::to_string(exp.info.row) + " col " +
                    std::to_string(exp.info.col) + " " + exp.info.value;
                throw(err);
            }
        } // 否则进行简单赋值操作

        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, exp.info.value, exp.info.row, exp.info.col,
                              exp.tableOffset, exp.offset});
    } else if (gr.size() == 3) {
        SemanticSymbol exp =
            symbolList[static_cast<int>(symbolList.size()) - 2]; // ( Exp )

        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, exp.info.value, exp.info.row, exp.info.col,
                              exp.tableOffset, exp.offset});
    } else throw(std::string(
            "Semantic analyze failed! In SemanticAnalyzer::analyzeFactor(const "
            "std::string& gl, const std::vector<std::pair<bool, int>>& gr)"));
}

// CallStmt->ID ( CallFunCheck Args )
void SemanticAnalyzer::analyzeCallStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol identifier = symbolList[static_cast<int>(symbolList.size()) - 5]; // ID
    SemanticSymbol callFuncCheck = symbolList[static_cast<int>(symbolList.size()) - 3]; // CallFunCheck
    SemanticSymbol args = symbolList[static_cast<int>(symbolList.size()) - 2]; // Args

    //检查函数是否定义（在CallFunCheck时已经检查）

    // 检查参数个数
    int para_num = tables[callFuncCheck.tableOffset].table[callFuncCheck.offset].functionParameterCount;
    if (para_num > stoi(args.info.value)) { // 参数过多
        std::string err =
            "Semantic analyze failed! Too many parameter in function call on row " +
            std::to_string(identifier.info.row) + " col " +
            std::to_string(identifier.info.col) + " " + identifier.info.value;
        throw(err);
    }
    else if (para_num < stoi(args.info.value)) { // 参数过少
        std::string err =
            "Semantic analyze failed! Too many args in function call on row " +
            std::to_string(identifier.info.row) + " col " +
            std::to_string(identifier.info.col) + " " + identifier.info.value;
        throw(err);
    }

    // 生成函数调用四元式
    std::string new_tmp_var = "T" + std::to_string(tmpVarCount++);
    quads.push_back({ nextQuadsId++, "call", identifier.info.value, "-", new_tmp_var});
    
    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    // 新的exp的value为临时变量名
    symbolList.push_back({gl, new_tmp_var, -1, -1, -1, -1});
}

// CallFunCheck->@
void SemanticAnalyzer::analyzeCallFunCheck(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol fun_id = symbolList[static_cast<int>(symbolList.size()) - 2];

    int fun_id_pos = tables[0].findSym(fun_id.info.value);

    if (-1 == fun_id_pos) { // 调用未定义
        std::string err =
            "Semantic analyze failed! Function call undefined on row " +
            std::to_string(fun_id.info.row) + " col " +
            std::to_string(fun_id.info.col) + " " + fun_id.info.value;
        throw(err);
    }
    if (tables[0].table[fun_id_pos].identifierType != Identifier::Function) { // 调用未定义
        std::string err =
            "Semantic analyze failed! Function call undefined on row " +
            std::to_string(fun_id.info.row) + " col " +
            std::to_string(fun_id.info.col) + " " + fun_id.info.value;
        throw(err);
    }
    symbolList.push_back({ gl, fun_id.info.value, fun_id.info.row, fun_id.info.col, 0, fun_id_pos});
}

// Args->Exp , Args | Exp | @
void SemanticAnalyzer::analyzeArgs(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 3) { // Args->Exp, Args
        SemanticSymbol exp =
            symbolList[static_cast<int>(symbolList.size()) - 3]; // Exp
        quads.push_back(
            {nextQuadsId++, "param", exp.info.value, "-", "-"});
        int aru_num = stoi(symbolList.back().info.value) + 1;
        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, std::to_string(aru_num), -1, -1, -1, -1});
    } else if (gr[0].first && (*ptrVN)[gr[0].second] == "Exp") { // Args->Exp
        SemanticSymbol exp = symbolList.back();
        quads.push_back(
            {nextQuadsId++, "param", exp.info.value, "-", "-"});
        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, "1", -1, -1, -1, -1});
    } else if (!gr[0].first && (*ptrVT)[gr[0].second] == "@") { // Args->@
        symbolList.push_back({gl, "0", -1, -1, -1, -1});
    } else 
        throw(std::string(
            "Semantic analyze failed! In SemanticAnalyzer::analyzeArgs(const "
            "std::string& gl, const std::vector<std::pair<bool, int>>& gr)"));
}

// ReturnStmt->return Exp | return
void SemanticAnalyzer::analyzeReturnStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 2){ // ReturnStmt->return Exp
        SemanticSymbol return_exp = symbolList.back();
        SymTable function_table = tables[currentTableStack.back()];

        quads.push_back({nextQuadsId ++, "=", return_exp.info.value, "-", function_table.table[0].identifierName});
        quads.push_back({nextQuadsId ++, "return", function_table.table[0].identifierName, "-", function_table.tableName });

        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, return_exp.info.value, -1, -1, -1, -1});
    } else if (gr.size() == 1) { // ReturnStmt->return
        SymTable function_table = tables[currentTableStack.back()];

        // 检查函数的返回值是否为 void
        if (tables[0].table[tables[0].findSym(function_table.tableName)].specifierType != "void") {
            std::string err =
                "Semantic analyze failed! No return value on "
                "row " + std::to_string(symbolList.back().info.row) + " col " +
                std::to_string(symbolList.back().info.col + sizeof("return")) + " in function " + function_table.tableName;
            throw(err);
        }
        quads.push_back({ nextQuadsId++ ,"return","-","-",function_table.tableName });
        for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
        symbolList.push_back({gl, "", -1, -1, -1, -1});
    } else 
        throw(std::string(
            "Semantic analyze failed! In SemanticAnalyzer::analyzeReturnStmt(const "
            "std::string& gl, const std::vector<std::pair<bool, int>>& gr)"));
}

// Relop-> > | < | >= | <= | == | !=
void SemanticAnalyzer::analyzeRelop(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol op = symbolList.back();
    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    symbolList.push_back({gl, op.info.value, -1, -1, -1, -1});
}

//IfStmt->if IfStmt_m1 ( Exp ) IfStmt_m2 Block IfNext
void SemanticAnalyzer::analyzeIfStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol ifstmt_m2 = symbolList[static_cast<int>(symbolList.size()) - 3];
    SemanticSymbol ifnext = symbolList[static_cast<int>(symbolList.size()) - 1];

    if (ifnext.info.value.empty()) {
        // 只有 if 没有 else
        // 真出口
        quads[backpatchList.back()].result = ifstmt_m2.info.value;
        backpatchList.pop_back();

        // 假出口
        quads[backpatchList.back()].result = std::to_string(nextQuadsId);
        backpatchList.pop_back();
    } else {
        // if块出口
        quads[backpatchList.back()].result =
            std::to_string(nextQuadsId);
        backpatchList.pop_back();
        // if真出口
        quads[backpatchList.back()].result = ifstmt_m2.info.value;
        backpatchList.pop_back();
        // if假出口
        quads[backpatchList.back()].result = ifnext.info.value;
        backpatchList.pop_back();
    }
    backpatchLevel--;

    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();
    symbolList.push_back({gl, "", -1, -1, -1, -1});
}

// IfStmt_m1->@
void SemanticAnalyzer::analyzeIfStmt_m1(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    backpatchLevel++;
    symbolList.push_back({gl, std::to_string(nextQuadsId), -1, -1, -1, -1});
}

// IfStmt_m2->@
void SemanticAnalyzer::analyzeIfStmt_m2(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol if_exp = symbolList[static_cast<int>(symbolList.size()) - 2];

    // 待回填四元式 : 假出口
    quads.push_back({nextQuadsId++, "j=", if_exp.info.value, "0", ""});
    backpatchList.push_back(static_cast<int>(quads.size()) - 1);

    // 待回填四元式 : 真出口
    quads.push_back({nextQuadsId++, "j=", "-", "-", ""});
    backpatchList.push_back(static_cast<int>(quads.size()) - 1);

    symbolList.push_back({gl, std::to_string(nextQuadsId), -1, -1, -1, -1});
}

// IfNext->IfStmt_next else Block
void SemanticAnalyzer::analyzeIfNext(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol if_stmt_next = symbolList[static_cast<int>(symbolList.size()) - 3];

    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();

    symbolList.push_back({gl, if_stmt_next.info.value, -1, -1, -1, -1});
}

// IfStmt_next ::= @
void SemanticAnalyzer::analyzeIfStmt_next(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    // if 的跳出语句(else 之前)(待回填)
    quads.push_back({nextQuadsId++, "j", "-", "-", ""});
    backpatchList.push_back(static_cast<int>(quads.size()) - 1);
    symbolList.push_back({gl, std::to_string(nextQuadsId), -1, -1, -1, -1});
}

//WhileStmt->while WhileStmt_m1 ( Exp ) WhileStmt_m2 Block
void SemanticAnalyzer::analyzeWhileStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol whilestmt_m1 = symbolList[static_cast<int>(symbolList.size()) - 6];
    SemanticSymbol whilestmt_m2 = symbolList[static_cast<int>(symbolList.size()) - 2];

    // 无条件跳转到 while 的条件判断语句处
    quads.push_back(
        {nextQuadsId++, "j", "-", "-", whilestmt_m1.info.value});

    // 回填真出口
    quads[backpatchList.back()].result = whilestmt_m2.info.value;
    backpatchList.pop_back();

    // 回填假出口
    quads[backpatchList.back()].result = std::to_string(nextQuadsId);
    backpatchList.pop_back();

    backpatchLevel--;

    for (int i = 0; i < gr.size(); i++) symbolList.pop_back();

    symbolList.push_back({gl, "", -1, -1, -1, -1});
}

// WhileStmt_m1->@
void SemanticAnalyzer::analyzeWhileStmt_m1(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    backpatchLevel++;
    symbolList.push_back({gl, std::to_string(nextQuadsId), -1, -1, -1, -1});
}

// WhileStmt_m2->@
void SemanticAnalyzer::analyzeWhileStmt_m2(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol while_exp = symbolList[static_cast<int>(symbolList.size()) - 2];

    // 假出口
    quads.push_back({nextQuadsId++, "j=", while_exp.info.value, "0", ""});
    backpatchList.push_back(static_cast<int>(quads.size()) - 1);
    // 真出口
    quads.push_back({nextQuadsId++, "j", "-", "-", ""});
    backpatchList.push_back(static_cast<int>(quads.size()) - 1);

    symbolList.push_back(
        {gl, std::to_string(nextQuadsId), -1, -1, -1, -1});
}
