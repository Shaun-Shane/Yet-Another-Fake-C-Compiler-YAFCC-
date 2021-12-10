#include "SyntaxAnalyzer.h"
//============================================================//

void LR1::printVN_VT_S() {  // for debug 输出变元 终结符
    std::cout << "VT: ";
    for (auto& i : VT) std::cout << i << " ";
    std::cout << "\n";
    std::cout << "VN: ";
    for (auto& i : VN) std::cout << i << " ";
    std::cout << "\n";
    std::cout << "S: " << S << "\n";
}

void LR1::printG() {  // for debug 输出所有产生式
    for (auto& i : G) {
        std::cout << i.first << "->";
        const auto& tmp = i.second;
        for (int i = 0; i < tmp.size(); i++) {
            for (const auto& j : P[tmp[i]].second) {
                // std::cout << j.first << " " << j.second << std::endl;
                if (j.first)
                    std::cout << VN[j.second];
                else
                    std::cout << VT[j.second];
            }
            if (i + 1 != tmp.size()) std::cout << "|";
        }
        std::cout << "\n";
    }
    // output format   @ means epsilon
    // B->S
    // S->AS|EXPR|b|@
}

void LR1::writeFirst() {  // 输出 first 集和
    std::ofstream firstSetFile;
    firstSetFile.open(codePath + "First.txt");
    for (const auto& i : first) {
        firstSetFile << "First(" << i.first << ") = { ";
        for (auto it = i.second.begin(); it != i.second.end();) {
            firstSetFile << VT[*it];
            if ((++it) != i.second.end()) firstSetFile << " , ";
        }
        firstSetFile << " }\n";
    }
    firstSetFile.close();
}

void LR1::writeC_GO() {  // 输出文法的项目集族 C
    std::ofstream clusterFile;
    clusterFile.open(codePath + "ClusterFile.txt");
    for (int i = 0; i < I.size(); i++) {
        clusterFile << "I" << i << ":\n";
        for (auto& item : I[i]) {
            clusterFile << "[ ";
            const auto& vec = P[item.pId].second;
            clusterFile << P[item.pId].first << "->";
            for (int j = 0; j < vec.size(); j++) {
                if (j == item.dotPos) clusterFile << ".";
                if (vec[j].first)
                    clusterFile << VN[vec[j].second];
                else clusterFile << VT[vec[j].second];
            }
            if (item.dotPos == vec.size()) clusterFile << ".";
            clusterFile << " , " << VT[item.right];
            clusterFile << " ]\n";
        }
        clusterFile << "\n";
    }

    std::cout << "Write C done!"
              << "\n";

    for (auto& i : GO) {
        int I, type, idx, J;
        std::tie(I, type, idx) = i.first, J = i.second;
        clusterFile << "GO(I" << I << ", ";
        clusterFile << (type ? VN[idx] : VT[idx]);
        clusterFile << ") = " << J << "\n";
    }

    std::cout << "Write GO done!"
              << "\n";
}

void LR1::writeLR1Table() { // 输出 LR1 分析表
    std::ofstream LR1TableFile;
    LR1TableFile.open(codePath + "LR1Table.txt");
    LR1TableFile << "LR(1) Table:\n";
    int sz = I.size();
    LR1TableFile << "ACTION:\n";
    for (int i = 0; i < sz; i++) {
        std::string output;
        output = "I" + std::to_string(i) + ": ";
        LR1TableFile << setiosflags(std::ios::left) << std::setw(8) << output;
        for (auto str : VT) {
            output.clear();
            output = str + "->";
            if (ACTION[i][str].first == -1)
                output += "err  ";
            else if (ACTION[i][str].first == 0)
                output += "acc  ";
            else if (ACTION[i][str].first == 1)
                output += "s" + std::to_string(ACTION[i][str].second) + "  ";
            else if (ACTION[i][str].first == 2)
                output += "r" + std::to_string(ACTION[i][str].second) + "  ";
            LR1TableFile << setiosflags(std::ios::left) << std::setw(16) << output;
        }
        LR1TableFile << "\n";
    }

    LR1TableFile << "GOTO:\n";
    for (int i = 0; i < sz; i++) {
        std::string output;
        output = "I" + std::to_string(i) + ": ";
        LR1TableFile << setiosflags(std::ios::left) << std::setw(8) << output;
        for (auto& str : VN) {
            output.clear();
            output = str + "->";
            if (GOTO[i][str] == -1)
                output += "err  ";
            else
                output += std::to_string(GOTO[i][str]) + "  ";
            LR1TableFile << setiosflags(std::ios::left) << std::setw(16) << output;
        }
        LR1TableFile << "\n";
    }
    std::cout << "Write LR1_Table done!"
              << "\n";
}

void LR1::writeSyntaxTree() { // 输出语法树信息
    std::ofstream SyntaxTreeFile;
    SyntaxTreeFile.open("../syntax_tree_builder/SyntaxTree.txt");
    int root = SyntaxTree.size() - 1;
    SyntaxTreeFile << root << "\n";
    for (const auto& itr : SyntaxTree) {
        for (const auto& son : itr)
            SyntaxTreeFile << son.first << " " << son.second << " ";
        SyntaxTreeFile << "\n";
    }

    SyntaxTreeFile << S;
    //输出起始子（如果需要）
    SyntaxTreeFile.close();
//#ifdef DISPLAY_SYNTAX_TREE
    system("start ../syntax_tree_builder/build/win-unpacked/syntax_tree.exe");
//#endif
}
//==============================================================================//

LR1::LR1(const std::string& file) {
    codePath = file;
    while (!codePath.empty() && codePath.back() != '/') codePath.pop_back();

    readGrammar();  // 输入文法
    printG();

    getFirstSet();  // 生成 first 集和
    writeFirst();

    genC();  // 生成 LR(1) 项目集族 C
    writeC_GO();

    genLR1Table();  //生成LR(1)分析表
    writeLR1Table();
}

// 返回产生式对应字符串
std::string LR1::P_ToString(const std::string& gl,
                       const std::vector<std::pair<bool, int>>& vec) {
    std::string ret;
    ret += gl + "->";
    for (auto& i : vec) {
        if (i.first)
            ret += VN[i.second];  // VN
        else
            ret += VT[i.second];  // VT
    }
    return ret;
}

// 读取文法
void LR1::readGrammar() {
    std::ifstream grammarFile;
    grammarFile.open(codePath + "Grammar.txt");
    if (!grammarFile.is_open()) throw(std::string("Open grammar file failed!"));
    std::stringstream ss;
    std::string line, vt, vn;
    while (true) {
        getline(grammarFile, line);  // 读入终结符
        if (line.size() && line[0] != '$') break;
    }
    ss.clear(), ss << line;
    while (getline(ss, vt, ' '))
        if (vt.size() && std::find(VT.begin(), VT.end(), vt) == VT.end()) VT.push_back(vt);

    sort(VT.begin(), VT.end());  // 按字典序 可以删除

    VT.push_back("#");  // # 也做为终结符
    VT.push_back("@");  // epsilon 这里额外将 epsilon 也算作终结符

    while (true) {
        getline(grammarFile, line);  // 读入非终结符
        if (line.size() && line[0] != '$') break;
    }
    ss.clear(), ss << line;
    while (getline(ss, vn, ' '))
        if (vn.size() && std::find(VN.begin(), VN.end(), vn) == VN.end()) VN.push_back(vn);

    sort(VN.begin(), VN.end());  // 按字典序 可删除

    // 读入文法起始符号
    while (true) {
        getline(grammarFile, line);  // 读入非终结符
        if (line.size() && line[0] != '$') break;
    }
    ss.clear(), ss << line;
    getline(ss, S, ' ');

    printVN_VT_S();  // for debug

    while (getline(grammarFile, line)) {  // 读入产生式
        if (line.size() == 0 || line[0] == '$') continue;
        if (line == "#") break;
        ss.clear(), ss << line;           // 读入文法
        std::string gl, gr;
        char ch;

        bool has_ = false;  // 判断文法有无->
        for (auto& c : line)
            if (c == '-') has_ = true;

        getline(ss, gl, '-');  // 读入产生式左部
        ss >> ch;
        if (ch != '>' || !has_) {  // 判断文法有无->
            grammarFile.close();
            throw(std::string("Error grammar input! No '->' sign!: " + line));
        }
        getline(ss, gr);  // 读入右部

        if (std::find(VN.begin(), VN.end(), gl) ==
            VN.end()) {  // 左部不能含有多余空格
            for (auto& c : gl)
                if (c == ' ') c = '^';
            grammarFile.close();
            throw(std::string("Error grammar input! Left side VN is invalid: " + gl));
        }

        ss.clear(), ss << gr;  // 开始处理产生式右部
        std::string tmp;
        std::vector<std::pair<bool, int>>
            vec;  // 用于获得 S->XiXj... 的 i, j  pair.first 是否为终结符
                  // pair.second 是下标
        while (getline(ss, tmp, ' ')) {
            if (!tmp.size()) continue;
            auto idx = std::find(VN.begin(), VN.end(), tmp) - VN.begin();
            if (idx != VN.size()) {  // Xi 为变元
                vec.emplace_back(true, idx);
            } else {
                idx = std::find(VT.begin(), VT.end(), tmp) - VT.begin();
                if (idx != VT.size()) {  // Xi 为终结符
                    vec.emplace_back(false, idx);
                    tmp.clear();
                } else {  // 既不是终结符也不是变元
                    grammarFile.close();
                    throw(std::string("Error grammar input!: " + line + "\nNo such VT or VN: " + tmp));
                }
            }
        }
        std::string stmp = P_ToString(gl, vec);
        std::cout << stmp << std::endl;
        for (auto& i : P) {  // 判断有无重复产生式
            if (stmp == P_ToString(i.first, i.second)) {
                grammarFile.close();
                throw(std::string("Multiple P input!: " + line));
            }
        }
        if (!G[S].empty() && gl == S) {  // 判断拓展文法开始符号是否有多个产生式
            grammarFile.close();
            throw(std::string("S has more than 1 Production!: " + line));
        }
        P.emplace_back(gl, vec);  // 存储一条产生式，产生式数目 + 1
        G[gl].push_back(P.size() - 1);
    }
    ss.clear();
    grammarFile.close();
}

// 生成 first 集和
void LR1::getFirstSet() {
    bool end = false;
    for (int i = 0; i < VT.size(); i++) // 终结符 a 的 first 集为 a，epsilon 除外
        if (VT[i] != "@") first[VT[i]].insert(i);
    while (!end) {
        end = true;
        for (const auto& g : G) {
            auto gl = g.first;  // gl -> branch1 | branch2 and gl is in VN
            for (const auto& p : g.second) {
                const auto& branch = P[p].second;
                bool type = branch.front().first;
                int idx = branch.front().second;

                if (type == false) {  // X->a...., a in VT
                    std::size_t preSize = first[gl].size();
                    if (VT[idx] == "@") first[gl].insert(idx);
                    else for (auto& i : first[VT[idx]]) first[gl].insert(i);
                    if (first[gl].size() != preSize) end = false;
                } else {  // X->Y..., Y in VN
                    std::size_t preSize = first[gl].size();

                    int epsPos = -1;  // 找到起始连续的 eps
                    for (int i = 0; i < branch.size(); i++) {
                        type = branch[i].first, idx = branch[i].second;
                        // eps in First(Yj) and Yj in VN
                        if (type && first[VN[idx]].count(VT.size() - 1))
                            epsPos = i;
                        else
                            break;  // VT[VT.size() - 1] is @ --- eps
                    }
                    for (int i = 0; i <= epsPos; i++) {
                        for (auto& j : first[VN[branch[i].second]])
                            if (j + 1 != VT.size())  // First[Yi] - {eps}
                                first[gl].insert(j);
                    }
                    if (epsPos + 1 == branch.size()) {
                        first[gl].insert(VT.size() - 1);  // add {eps}
                    } else {
                        if (branch[epsPos + 1].first == true) {  // VN
                            for (auto& j : first[VN[branch[epsPos + 1].second]])
                                first[gl].insert(j);
                        } else {  // VT
                            for (auto& j : first[VT[branch[epsPos + 1].second]])
                                first[gl].insert(j);
                        }
                    }

                    if (first[gl].size() != preSize) end = false;
                }
            }
        }
    }
}

// 获得 first(βa) 的终结符;
std::set<int> LR1::getFirstVT(const std::vector<std::pair<bool, int>>& l,
                         std::size_t right, std::size_t dotPos) {
    std::set<int> ret;
    int epsPos = dotPos;
    for (int i = dotPos + 1; i < l.size(); i++) {  // 找到连续的含有 eps 的 V
        bool type = l[i].first;
        int idx = l[i].second;
        if (type && first[VN[idx]].count(VT.size() - 1))
            epsPos = i;
        else
            break;  // VT[VT.size() - 1] is @ --- eps
    }
    for (int i = dotPos + 1; i <= epsPos; i++) {
        for (auto& j : first[VN[l[i].second]])
            if (j + 1 != VT.size())  // First[Yi] - {eps}
                ret.insert(j);
    }
    if (epsPos + 1 == l.size()) {
        ret.insert(right);  // add {a}
    } else {
        if (l[epsPos + 1].first == true) {  // VN
            for (auto& j : first[VN[l[epsPos + 1].second]]) ret.insert(j);
        } else {  // VT
            for (auto& j : first[VT[l[epsPos + 1].second]]) ret.insert(j);
        }
    }
    return ret;
}

// 生成项目集 Ii 的闭包
void LR1::genClosure(std::set<Item>& Ii) {
    while (true) {
        auto preSize = Ii.size();
        for (const auto& item : Ii) {
            const auto& vec = P[item.pId].second;
            if (item.dotPos == vec.size()) continue;  // 规约状态
            bool type = vec[item.dotPos].first;
            int idx = vec[item.dotPos].second;
            if (type == false) continue;  // VT 终结符跳过

            for (auto& iVT :
                 getFirstVT(vec, item.right,
                            item.dotPos)) {  // 对于每个 First(βa) 中的终结符
                Item newItem;
                auto itr = G.find(VN[idx]);  // 获取 .Bβ 的 B -> 产生式迭代器

                if (itr == G.end() || iVT == VT.size() - 1) {
                    std::cerr << "GenClosure error, checkP! line: " << __LINE__
                              << std::endl;
                    return;
                }
                for (auto& nxtPId : itr->second) {
                    newItem.pId = nxtPId, newItem.right = iVT,
                    newItem.dotPos = 0;
                    if (P[newItem.pId].second[0].first == false && P[newItem.pId].second[0].second == VT.size() - 1) {
                        newItem.dotPos = 1;
                    }
                    Ii.insert(newItem);
                }
            }
        }
        if (Ii.size() == preSize) break;
    }
}

// 判断项目集 Itmp 是否存在，返回项目集编号 brute force
int LR1::foundI(const std::set<Item>& Itmp) {
    for (int i = 0; i < I.size(); i++) {
        if (I[i].size() == Itmp.size()) {  // 项目集大小相同才有可能相等
            bool same = true;
            // set 已排序好 只需按迭代器访问判断是否有不同
            for (auto itr1 = I[i].begin(), itr2 = Itmp.begin();
                 itr1 != I[i].end(); ++itr1, ++itr2) {
                auto &itemOfIi = *itr1, &itemOfItmp = *itr2;
                if (!(itemOfIi.pId == itemOfItmp.pId &&
                      itemOfIi.dotPos == itemOfItmp.dotPos &&
                      itemOfIi.right == itemOfItmp.right)) {
                    same = false;
                    break;
                }
            }
            if (same) return i;
        }
    }
    return -1;
}

// 生成 LR(1) 项目集族 C
void LR1::genC() {
    // 初始化 I0: [S'->.S, #]
    std::set<Item> I0;
    I0.insert((Item){G[S][0], (std::size_t)0,
                     VT.size() - 2});  // VT[VT.size() - 2] is #
    genClosure(I0);
    I.push_back(I0);

    for (int i = 0; i < I.size(); i++) {             // 对于每个项目 Ii
        for (int idx = 0; idx < VN.size(); idx++) {  // 对于每个非终结符
            std::set<Item> newI;                     // 新的项目集 vector
            for (const auto& item : I[i]) {
                const auto& vec = P[item.pId].second;
                if (item.dotPos == vec.size()) continue;  // 规约状态
                auto nxtItem = item;
                nxtItem.dotPos++;  // . 右移一位

                bool Xtype = vec[item.dotPos].first;  // X of GO(I, X)
                int Xidx = vec[item.dotPos].second;
                // Xtype = 0 表示终结符 1 表示变元
                if (Xtype == 1 && idx == Xidx)
                    newI.insert(nxtItem);  // 加入.右移一位的项目
            }
            if (newI.empty()) continue;
            genClosure(newI);  // 生成 closure

            int to = foundI(newI);  // 计算 GO(I, X)
            if (to == -1) {         // 新的项目集
                to = I.size();
                I.push_back(newI);
            }
            GO[{i, 1, idx}] = to;  // GO(I, X) = j; X in VN
        }

        for (int idx = 0; idx < static_cast<int>(VT.size()) - 2; idx++) { // 对于每个终结符 # @除外
            std::set<Item> newI;  // 新的项目集 vector
            for (const auto& item : I[i]) {
                const auto& vec = P[item.pId].second;
                if (item.dotPos == vec.size()) continue;  // 规约状态
                auto nxtItem = item;
                nxtItem.dotPos++;  // . 右移一位

                bool Xtype = vec[item.dotPos].first;  // X of GO(I, X)
                int Xidx = vec[item.dotPos].second;
                // Xtype = 0 表示终结符 1 表示变元
                if (Xtype == 0 && idx == Xidx)
                    newI.insert(nxtItem);  // 加入.右移一位的项目
            }
            if (newI.empty()) continue;
            genClosure(newI);  // 生成 closure

            int to = foundI(newI);  // 计算 GO(I, X)
            if (to == -1) {         // 新的项目集
                to = I.size();
                I.push_back(newI);
            }
            GO[{i, 0, idx}] = to;  // GO(I, X) = j; X in VT
        }
    }
}

// 生成LR(1)分析表
void LR1::genLR1Table() {
    // init
    int sz = I.size();
    std::map<std::string, std::pair<int, int>> temp1;
    std::map<std::string, int> temp2;

    for (auto& str : VT) 
        temp1[str] = {-1, 0};
    for (auto& str : VN) temp2[str] = -1;
    for (int i = 0; i < sz; i++) {
        ACTION.push_back(temp1);
        GOTO.push_back(temp2);
        for (auto& item : I[i]) {
            auto& vec = P[item.pId].second;
            int pos = item.dotPos;
            // condition 1
            if (pos < vec.size() && !vec[pos].first) {
                if(GO.find(std::make_tuple(i, 0, vec[pos].second)) != GO.end()) {
                    std::string tmp_str = VT[vec[pos].second];
                    ACTION[i][tmp_str].first = 1;
                    ACTION[i][tmp_str].second =
                    GO[std::make_tuple(i, 0, vec[pos].second)];
                }
                
            }
            // condition 2
            else if (pos == vec.size() && P[item.pId].first != S) {
                std::string tmp_str = VT[item.right];
                ACTION[i][tmp_str].first = 2;
                ACTION[i][tmp_str].second = item.pId;
            }
            // condition 3
            else if (pos == vec.size() && P[item.pId].first == S) {
                std::string tmp_str = VT[item.right];
                ACTION[i][tmp_str].first = 0;
            }
        }
    }
    for (auto& i : GO) {
        int I, type, idx, J;
        std::tie(I, type, idx) = i.first, J = i.second;
        if (!type) continue;
        GOTO[I][VN[idx]] = J;
    }
}

void LR1::parse(const std::vector<TOKEN>& _token_stream) {
    SemanticAnalyzer semantic_analyzer = SemanticAnalyzer(&VT, &VN);

    std::vector<TOKEN> token_stream = _token_stream;
    token_stream.push_back({"#", "#", -1, -1});

    std::ofstream analyzeFile;
    analyzeFile.open(codePath + "LR1AnalyzeFile.txt");
    
    std::vector<int> status;
    std::vector<std::pair<int, std::string>> sign;
    status.push_back(0);
    sign.push_back({-1, "#"});

    bool end = false;  // sign of end
    bool err = false;  // sign of err
    int p = 0;
    analyzeFile << "\n";
    analyzeFile << setiosflags(std::ios::left) << std::setw(60) << "status";
    analyzeFile << setiosflags(std::ios::left) << std::setw(60) << "sign";
    analyzeFile << "input"
          << "\n";

    semantic_analyzer.Add_symbol_to_list({ {"#", "", -1, -1}, -1, -1 });

    while (!end) {
        std::string output;
        for (auto itr : status) output += std::to_string(itr) + ' ';
        analyzeFile << setiosflags(std::ios::left) << std::setw(60) << output;
        output.clear();
        for (auto itr : sign) output += itr.second + ' ';
        analyzeFile << setiosflags(std::ios::left) << std::setw(60) << output;
        for (int i = p; i < token_stream.size(); i++) analyzeFile << token_stream[i].token << " ";
        analyzeFile << "\n";
        int now = status.back();
        std::pair<int, int> act;
        act = ACTION[now][token_stream[p].token];
        if (act.first == -1) {
            end = true;
            err = true;
        } else if (act.first == 0) {
            std::vector<std::pair<int, std::string>> son;
            while (sign.back().second != "#") {
                status.pop_back();
                son.push_back(sign.back());
                sign.pop_back();
            }
            SyntaxTree.push_back(son);
            end = true;
        } else if (act.first == 1) {
            status.push_back(act.second);
            sign.push_back({-1, token_stream[p].token});

            semantic_analyzer.Add_symbol_to_list({token_stream[p].token , token_stream[p].value , token_stream[p].row , token_stream[p].col ,-1, -1 });
            ++p;
        } else {
            int Pnum = act.second;
            std::vector<std::pair<bool, int>> vec;
            vec = P[Pnum].second;
            int sz = vec.size();
            std::vector<std::pair<int, std::string>> son;

            if (vec.size() == 1 && vec[0].first == false && vec[0].second == VT.size() - 1) {
                // status.pop_back(); // A->@ 归约特殊处理
                son.push_back({-1, "@"});
            } else {
                for (int i = 0; i < sz; i++) {
                    status.pop_back();
                    son.push_back(sign.back());
                    sign.pop_back();
                }
            }
            
            SyntaxTree.push_back(son);
            sign.push_back({SyntaxTree.size() - 1, P[Pnum].first});
            status.push_back(GOTO[status.back()][P[Pnum].first]);

            semantic_analyzer.Analysis(P[Pnum].first, P[Pnum].second);
        }
    }
    if (!err) {
        analyzeFile << "acc\n";
        writeSyntaxTree();
    } else {
        analyzeFile << "err\n";
        throw(std::string("fail to pass syntax analyze!"));
    }

    semantic_analyzer.Print_quads("testfiles/Quads.txt");
}