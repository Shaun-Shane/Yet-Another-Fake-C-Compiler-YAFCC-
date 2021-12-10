#include "SemanticAnalyzer.h"

SemanticAnalyzer::SemanticAnalyzer() {}

//将所有的符号信息放入symbol_list
void SemanticAnalyzer::Add_symbol_to_list(const SemanticSymbol symbol) {
    symbolList.push_back(symbol);
}

//分析过程
void SemanticAnalyzer::Analysis(const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    
}

//打印四元式表
void SemanticAnalyzer::Print_quads(const std::string file_path) {
    std::ofstream quad_out;
    quad_out.open(file_path, std::ios::out);

    for (auto& q: quads)
        quad_out << q.index << "(" << q.ope << ", " << q.arg1 << ", " << q.arg2 << ", " << q.result << ")" << std::endl;
}

void SemanticAnalyzer::analyzeProgram(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (mainIndex == -1) {
        throw(6); //SEMANTIC_ERROR_NO_MAIN
    }
    int cnt = gr.size();
    while(cnt --)
        symbolList.pop_back();

    quads.insert(quads.begin(), { 0, "j", "-", "-", std::to_string(mainIndex) });
    symbolList.push_back({{gl, "", -1, -1}, -1, -1});
}

void SemanticAnalyzer::analyzeExtDef(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {

    if (gr.size() == 3) {
        SemanticSymbol specifier = symbolList[symbolList.size() - 3];
        SemanticSymbol identifier = symbolList[symbolList.size() - 2];
        
        Semantic_symbol_table current_table = tables[current_table_stack.back()];
        if (current_table.findSymbol(identifier.info.value) != -1) {
            // cout << "" << identifier.row << "" << identifier.col << "" << identifier.value << "" << endl;
            throw(7); // SEMANTIC_ERROR_REDEFINED
        }

        IdentifierInfo variable;
        variable.identifierName = identifier.info.value;
        variable.identifierType = IdentifierInfo::Variable;
        variable.specifierType = specifier.info.value;

        tables[current_table_stack.back()].addSymbol(variable);

        int cnt = gr.size();
        while (cnt --)
            symbolList.pop_back();
        symbolList.push_back({{gl, identifier.info.value, identifier.info.row, identifier.info.col}, current_table_stack.back(), int(tables[current_table_stack.back()].table.size() - 1)});
    }
    else {
        SemanticSymbol identifier = symbolList[symbolList.size() - 2];

        current_table_stack.pop_back();
        
        int cnt = gr.size();
        while (cnt --)
            symbolList.pop_back();
        symbolList.push_back({{gl, identifier.info.value, identifier.info.row, identifier.info.col}, identifier.table_index, identifier.innerIndex});
    }
}

void SemanticAnalyzer::analyzeVarSpecifier(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol specifier = symbolList.back();
    int cnt = gr.size();
    while (cnt --)
        symbolList.pop_back();
    symbolList.push_back({{gl, specifier.info.value, specifier.info.row, specifier.info.col}, -1, -1});
}

void SemanticAnalyzer::analyzeFunSpecifier(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol specifier = symbolList.back();
    int cnt = gr.size();
    while (cnt --)
        symbolList.pop_back();
    symbolList.push_back({{gl, specifier.info.value, specifier.info.row, specifier.info.col}, -1, -1});
}

void SemanticAnalyzer::analyzeFunDec(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol specifier = symbolList[symbolList.size() - 4];
    int cnt = gr.size();
    while (cnt --)
        symbolList.pop_back();
    symbolList.push_back({{gl, specifier.info.value, specifier.info.row, specifier.info.col}, specifier.table_index, specifier.innerIndex});
}

void SemanticAnalyzer::analyzeCreateFunTable_m(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol identifier = symbolList.back();
    SemanticSymbol specifier = symbolList[symbolList.size() - 2];

    if (tables[0].findSymbol(identifier.info.value) != -1) {
        //cout << "" << identifier.row << "" << identifier.col << "" << identifier.value << "" << endl;
        throw(7); //SEMANTIC_ERROR_REDEFINED
    }

    tables.push_back(Semantic_symbol_table(Semantic_symbol_table::FunctionTable, identifier.info.value));
    
    tables[0].addSymbol({IdentifierInfo::Function, specifier.info.value, identifier.info.value, 0, 0, int(tables.size() - 1)});

    current_table_stack.push_back(tables.size() - 1);
   
    IdentifierInfo return_value;
    return_value.identifierType = IdentifierInfo::ReturnVar;
    return_value.identifierName = tables.back().tableName + "_return_value";
    return_value.specifierType = specifier.info.value;
    
    if (identifier.info.value == "main")
        mainIndex = next_quads_index;
    
    quads.push_back({ next_quads_index++ , identifier.info.value,"-","-" ,"-" });
    tables[current_table_stack.back()].addSymbol(return_value);
    symbolList.push_back({gl, identifier.info.value, identifier.info.row, identifier.info.col, 0, int(tables[0].table.size() - 1)});
}

void SemanticAnalyzer::analyzeParamDec(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol identifier = symbolList.back();
    SemanticSymbol specifier = symbolList[symbolList.size() - 2];
    Semantic_symbol_table& function_table = tables[current_table_stack.back()];

    if (function_table.findSymbol(identifier.info.value) != -1) {
        //cout << "" << identifier.info.row << "" << identifier.info.col << "" << identifier.info.value << "" << endl;
        throw(7);//SEMANTIC_ERROR_REDEFINED
    }
    
    int new_position = function_table.addSymbol({IdentifierInfo::Variable, specifier.info.value, identifier.info.value, -1, -1, -1});
    int table_position = tables[0].findSymbol(function_table.tableName);

    tables[0].table[table_position].functionParameterCount++;

    quads.push_back({next_quads_index ++, "defpar", "-" , "-", identifier.info.value });

    int cnt = gr.size();
    while (cnt--)
        symbolList.pop_back();
    symbolList.push_back({gl, identifier.info.value, identifier.info.row,identifier.info.col, current_table_stack.back(), new_position});
}

void SemanticAnalyzer::analyzeBlock(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    int count = gr.size();
    while (count--)
        symbolList.pop_back();
    symbolList.push_back({gl, std::to_string(next_quads_index), -1, -1, -1,-1 });
}

void SemanticAnalyzer::analyzeDef(const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol identifier = symbolList[symbolList.size() - 2];
    SemanticSymbol specifier = symbolList[symbolList.size() - 3];
    Semantic_symbol_table& current_table = tables[current_table_stack.back()];

    //�ض����򱨴�
    if (current_table.findSymbol(identifier.info.value) != -1)
    {
        //cout << "��������з������󣺣�" << identifier.row << "�У�" << identifier.col << "�У�����" << identifier.value << "�ض���" << endl;
        throw(7); // SEMANTIC_ERROR_REDEFINED
    }

    current_table.addSymbol({IdentifierInfo::Variable, specifier.info.value, identifier.info.value, -1, -1, -1});

    int cnt = gr.size();
    while (cnt --)
        symbolList.pop_back();
    symbolList.push_back({gl, identifier.info.value, identifier.info.row, identifier.info.col, current_table_stack.back(), int(tables[current_table_stack.back()].table.size() - 1)});
}

void SemanticAnalyzer::analyzeAssignStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol identifier = symbolList[symbolList.size() - 3];
    SemanticSymbol exp = symbolList.back();

    bool existed = false;
    int table_index = -1, index = -1;
    
    for (int scope_layer = current_table_stack.size() - 1; scope_layer >= 0; scope_layer--) {
        auto current_table = tables[current_table_stack[scope_layer]];
        if ((index=current_table.findSymbol(identifier.info.value)) != -1) {
            existed = true;
            table_index = current_table_stack[scope_layer];
            break;
        }
    }
    if (existed == false) {
        //cout << "��������з������󣺣�" << identifier.row << "�У�" << identifier.col << "�У�����" << identifier.value << "δ����" << endl;
        throw(8);//SEMANTIC_ERROR_UNDEFINED
    }

    quads.push_back({next_quads_index++, "=", exp.info.value, "-", identifier.info.value });

    int cnt = gr.size();
    while (cnt --)
        symbolList.pop_back();
    symbolList.push_back({gl, identifier.info.value ,identifier.info.row,identifier.info.col, table_index, index});
}

void SemanticAnalyzer::analyzeExp(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 1) {
        SemanticSymbol exp = symbolList.back();
        //����symbol_list
        int cnt = gr.size();
        while (cnt --)
            symbolList.pop_back();
        symbolList.push_back({ gl, exp.info.value, exp.info.row, exp.info.col, exp.table_index, exp.innerIndex });
    }
    else {
        SemanticSymbol sub_exp1 = symbolList[symbolList.size() - 3];
        SemanticSymbol op = symbolList[symbolList.size() - 2];
        SemanticSymbol sub_exp2 = symbolList[symbolList.size() - 1];
        int next_label_num = next_quads_index++;
        std::string new_tmp_var = "T" + std::to_string(tmp_var_count++);
        quads.push_back({ next_label_num, "j" + op.info.value, sub_exp1.info.value, sub_exp2.info.value, std::to_string(next_label_num + 3) });
        quads.push_back({ next_quads_index++, "=", "0", "-", new_tmp_var });
        quads.push_back({ next_quads_index++, "j", "-", "-", std::to_string(next_label_num + 4) });
        quads.push_back({ next_quads_index++, "=", "1", "-", new_tmp_var });

        int cnt = gr.size();
        while (cnt--)
            symbolList.pop_back();
        symbolList.push_back({gl, new_tmp_var, -1, -1, -1, -1});
    }
}

void SemanticAnalyzer::analyzeAddSubExp(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 1) {
        SemanticSymbol exp = symbolList.back();
        
        int cnt = gr.size();
        while (cnt--)
            symbolList.pop_back();
        symbolList.push_back({ gl, exp.info.value, exp.info.row, exp.info.col, exp.table_index, exp.innerIndex });
    }
    else {
        SemanticSymbol sub_exp1 = symbolList[symbolList.size() - 3];
        SemanticSymbol op = symbolList[symbolList.size() - 2];
        SemanticSymbol sub_exp2 = symbolList[symbolList.size() - 1];
        std::string new_tmp_var = "T" + std::to_string(tmp_var_count++);
        quads.push_back({next_quads_index++, op.info.value, sub_exp1.info.value, sub_exp2.info.value, new_tmp_var});

        int cnt = gr.size();
        while (cnt --)
            symbolList.pop_back();
        symbolList.push_back({gl, new_tmp_var, -1, -1, -1, -1});
    }
}

void SemanticAnalyzer::analyzeItem(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 1) {
        SemanticSymbol exp = symbolList.back();
        //����symbol_list
        int count = gr.size();
        while (count--)
            symbolList.pop_back();
        symbolList.push_back({ gl, exp.info.value, exp.info.row, exp.info.col, exp.table_index,exp.innerIndex});
    }
    else {
        SemanticSymbol sub_exp1 = symbolList[symbolList.size() - 3];
        SemanticSymbol op = symbolList[symbolList.size() - 2];
        SemanticSymbol sub_exp2 = symbolList[symbolList.size() - 1];
        std::string new_tmp_var = "T" + std::to_string(tmp_var_count++);
        quads.push_back({ next_quads_index++, op.info.value, sub_exp1.info.value, sub_exp2.info.value, new_tmp_var });

        int count = gr.size();
        while (count--)
            symbolList.pop_back();
        symbolList.push_back({ gl, new_tmp_var, -1, -1, -1, -1});
    }
}

void SemanticAnalyzer::analyzeFactor(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 1) {
        SemanticSymbol exp = symbolList.back();
        
        if (!gr[0].first && (*ptrVT)[gr[0].second] == "ID") {
            bool existed = false;
            for (int scope_layer = current_table_stack.size() - 1; scope_layer >= 0; scope_layer--) {
                auto current_table = tables[current_table_stack[scope_layer]];
                if (current_table.findSymbol(exp.info.value) != -1) {
                    existed = true;
                    break;
                }
            }
            if (existed == false) {
                //cout << "��������з������󣺣�" << exp.row << "�У�" << exp.col << "�У�����" << exp.value << "δ����" << endl;
                throw(8);//SEMANTIC_ERROR_UNDEFINED
            }
        }

        //����symbol_list
        int cnt = gr.size();
        while (cnt--)
            symbolList.pop_back();
        symbolList.push_back({ gl, exp.info.value, exp.info.row, exp.info.col, exp.table_index, exp.innerIndex});
    }
    else {
        SemanticSymbol exp = symbolList[symbolList.size() - 2];

        //����symbol_list
        int cnt = gr.size();
        while (cnt--)
            symbolList.pop_back();
        symbolList.push_back({ gl, exp.info.value, exp.info.row, exp.info.col, exp.table_index, exp.innerIndex });
    }
}

void SemanticAnalyzer::analyzeCallStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol identifier = symbolList[symbolList.size() - 5];
    SemanticSymbol check = symbolList[symbolList.size() - 3];
    SemanticSymbol args = symbolList[symbolList.size() - 2];

    int para_num = tables[check.table_index].table[check.innerIndex].functionParameterCount;
    if (para_num > stoi(args.info.value)) {
        //cout << "��������з������󣺣�" << identifier.row << "�У�" << identifier.col << "�У�����" << identifier.value << "����ʱ��������" << endl;
        throw(9); //SEMANTIC_ERROR_PARAMETER_NUM
    }
    else if (para_num < stoi(args.info.value)) {
        //cout << "��������з������󣺣�" << identifier.row << "�У�" << identifier.col << "�У�����" << identifier.value << "����ʱ��������" << endl;
        throw(9);//SEMANTIC_ERROR_PARAMETER_NUM
    }

    // ���ɺ���������Ԫʽ 
    std::string new_tmp_var = "T" + std::to_string(tmp_var_count++);
    quads.push_back({ next_quads_index++, "call", identifier.info.value, "-", new_tmp_var});

    int cnt = gr.size();
    while (cnt--)
        symbolList.pop_back();
    // �µ�exp��valueΪ��ʱ������
    symbolList.push_back({gl, new_tmp_var, -1, -1, -1, -1});
}

void SemanticAnalyzer::analyzeCallFunCheck(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol fun_id = symbolList[symbolList.size() - 2];

    int fun_id_pos = tables[0].findSymbol(fun_id.info.value);

    if (-1 == fun_id_pos) {
        //cout << "��������з������󣺣�" << fun_id.row << "�У�" << fun_id.col << "�У�����" << fun_id.value << "����δ����" << endl;
        throw(8);//SEMANTIC_ERROR_UNDEFINED
    }
    if (tables[0].table[fun_id_pos].identifierType != IdentifierInfo::Function) {
        //cout << "��������з������󣺣�" << fun_id.row << "�У�" << fun_id.col << "�У�����" << fun_id.value << "����δ����" << endl;
        throw(8);//SEMANTIC_ERROR_UNDEFINED
    }
    symbolList.push_back({ gl, fun_id.info.value, fun_id.info.row, fun_id.info.col, 0, fun_id_pos});
}

void SemanticAnalyzer::analyzeArgs(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 3) {
        SemanticSymbol exp = symbolList[symbolList.size() - 3];
        quads.push_back({ next_quads_index++, "param", exp.info.value, "-", "-" });
        int aru_num = stoi(symbolList.back().info.value) + 1;
        int count = gr.size();
        while (count--)
            symbolList.pop_back();
        symbolList.push_back({ gl, std::to_string(aru_num),-1,-1,-1,-1 });
    }
    else if(gr[0].first && (*ptrVN)[gr[0].second] == "Exp"){
        SemanticSymbol exp = symbolList.back();
        quads.push_back({ next_quads_index++, "param", exp.info.value, "-", "-" });
        int count = gr.size();
        while (count--)
            symbolList.pop_back();
        symbolList.push_back({ gl, "1", -1, -1, -1, -1});
    }
    else if (!gr[0].first && (*ptrVT)[gr[0].second] == "@") {
        symbolList.push_back({ gl, "0", -1, -1, -1, -1});
    }
}

void SemanticAnalyzer::analyzeReturnStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    if (gr.size() == 2){
        SemanticSymbol return_exp = symbolList.back();
        Semantic_symbol_table function_table = tables[current_table_stack.back()];

        quads.push_back({next_quads_index ++, "=", return_exp.info.value, "-", function_table.table[0].identifierName});
        quads.push_back({next_quads_index ++ ,"return", function_table.table[0].identifierName, "-", function_table.tableName });

        int cnt = gr.size();
        while (cnt--)
            symbolList.pop_back();
        symbolList.push_back({ gl, return_exp.info.value, -1, -1, -1, -1});
    }
    else {
        Semantic_symbol_table function_table = tables[current_table_stack.back()];

        if (tables[0].table[tables[0].findSymbol(function_table.tableName)].specifierType != "void") {
            //cout << "��������з������󣺣�" << symbolList.back().row << "�У�" << symbolList.back().col+sizeof("return") << "�У�����"<< function_table.table_name <<"�����з���ֵ" << endl;
            throw(11);//SEMANTIC_ERROR_NO_RETURN
        }

        //������Ԫʽ
        quads.push_back({ next_quads_index++ ,"return","-","-",function_table.tableName });

        //����symbol_list
        int count = gr.size();
        while (count--)
            symbolList.pop_back();
        symbolList.push_back({ gl, "", -1, -1, -1, -1 });
    }
}

void SemanticAnalyzer::analyzeRelop(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol op = symbolList.back();

    int cnt = gr.size();
    while (cnt--) {
        symbolList.pop_back();
    }
    symbolList.push_back({ gl, op.info.value ,-1,-1,-1,-1 });
}

void SemanticAnalyzer::analyzeIfStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol ifstmt_m2 = symbolList[symbolList.size() - 3];
    SemanticSymbol ifnext = symbolList[symbolList.size() - 1];

    if (ifnext.info.value.empty()) {
        //ֻ��ifû��else
        //�����
        quads[backpatching_list.back()].result = ifstmt_m2.info.value;
        backpatching_list.pop_back();

        //�ٳ���
        quads[backpatching_list.back()].result = std::to_string(next_quads_index);
        backpatching_list.pop_back();
    }
    else {
        //if�����
        quads[backpatching_list.back()].result = std::to_string(next_quads_index);
        backpatching_list.pop_back();
        //if�����
        quads[backpatching_list.back()].result = ifstmt_m2.info.value;
        backpatching_list.pop_back();
        //if�ٳ���
        quads[backpatching_list.back()].result = ifnext.info.value;
        backpatching_list.pop_back();
    }
    backpatching_level--;

    //popback
    int count = gr.size();
    while (count--)
        symbolList.pop_back();
    symbolList.push_back({ gl,"",-1,-1,-1,-1 });
}

void SemanticAnalyzer::analyzeIfStmt_m1(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    backpatching_level++;
    symbolList.push_back({ gl, std::to_string(next_quads_index), -1, -1, -1, -1 });
}

void SemanticAnalyzer::analyzeIfStmt_m2(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol if_exp = symbolList[symbolList.size() - 2];

    //��������Ԫʽ : �ٳ���
    quads.push_back({ next_quads_index++,"j=",if_exp.info.value,"0","" });
    backpatching_list.push_back(quads.size() - 1);

    //��������Ԫʽ : �����
    quads.push_back({ next_quads_index++,"j=","-","-","" });
    backpatching_list.push_back(quads.size() - 1);

    symbolList.push_back({ gl, std::to_string(next_quads_index),-1,-1,-1,-1 });
}

void SemanticAnalyzer::analyzeIfNext(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol if_stmt_next = symbolList[symbolList.size() - 3];

    int count = gr.size();
    while (count--)
        symbolList.pop_back();

    symbolList.push_back({ gl,if_stmt_next.info.value,-1,-1,-1,-1 });
}

void SemanticAnalyzer::analyzeIfStmt_next(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    quads.push_back({ next_quads_index++,"j","-","-","" });
    backpatching_list.push_back(quads.size() - 1);
    symbolList.push_back({ gl,std::to_string(next_quads_index),-1,-1,-1,-1 });
}

void SemanticAnalyzer::analyzeWhileStmt(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol whilestmt_m1 = symbolList[symbolList.size() - 6];
    SemanticSymbol whilestmt_m2 = symbolList[symbolList.size() - 2];

    // ��������ת�� while �������ж���䴦
    quads.push_back({ next_quads_index++,"j", "-", "-" , whilestmt_m1.info.value });

    //���������
    quads[backpatching_list.back()].result = whilestmt_m2.info.value;
    backpatching_list.pop_back();

    //����ٳ���
    quads[backpatching_list.back()].result = std::to_string(next_quads_index);
    backpatching_list.pop_back();

    backpatching_level--;

    int count = gr.size();
    while (count--)
        symbolList.pop_back();

    symbolList.push_back({ gl,"",-1,-1,-1,-1 });
}

void SemanticAnalyzer::analyzeWhileStmt_m1(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    backpatching_level++;
    symbolList.push_back({ gl, std::to_string(next_quads_index),-1,-1,-1,-1 });
}

void SemanticAnalyzer::analyzeWhileStmt_m2(
    const std::string& gl, const std::vector<std::pair<bool, int>>& gr) {
    SemanticSymbol while_exp = symbolList[symbolList.size() - 2];

    //�ٳ���
    quads.push_back({ next_quads_index++,"j=", while_exp.info.value,"0","" });
    backpatching_list.push_back(quads.size() - 1);
    //�����
    quads.push_back({ next_quads_index++ ,"j","-","-" ,"" });
    backpatching_list.push_back(quads.size() - 1);

    symbolList.push_back({ gl, std::to_string(next_quads_index),-1,-1,-1,-1 });
}
