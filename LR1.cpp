#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <set>
#include <map>
#include <algorithm>

#define debug(x) std::cerr << #x << " = " << x << std::endl

std::ifstream grammarFile;
std::ifstream inputFile;
std::ofstream firstSetFile;
std::ofstream CFile;

std::string S; // 文法起始符号 拓展文法起始符应为 S'
std::vector<std::string> VT; // 终结符
std::vector<std::string> VN; // 非终结符
std::map<std::string, int> P; // key 产生式 value 第 i 个产生式
std::map<std::string, std::vector<std::vector<std::pair<bool, int>>>> G; // 产生式 如 Si->X1X2X3...
std::map<std::string, std::set<int>> first; // first 集
std::map<std::tuple<int, int, int>, int> GO; // GO(I, type, idx) = j;
std::vector<std::map<std::string, std::pair<int, int>>> ACTION;
//fir == -1:err   0:acc   1:sj   2:rj
std::vector<std::map<std::string, int>> GOTO;

struct Item {
    decltype(G.begin()) itrG;
    std::size_t branchId;
    std::size_t dotPos; // . 的位置, 0 到 size
    std::size_t right; // 右侧终结符下标
};
std::vector<std::vector<Item>> I; // 项目集 I0, I1, I2, ...

void openFile() {
    grammarFile.open("Grammar.txt");
    CFile.open("CFile.txt");
    firstSetFile.open("First.txt");
}

void closeFile() {
    grammarFile.close();
    CFile.close();
    firstSetFile.close();
}

void writeFirst() { // 输出 first 集和
    for (const auto& i : first) {
        firstSetFile << "First(" << i.first << ") = { ";
        for (auto it = i.second.begin(); it != i.second.end();) {
            firstSetFile << VT[*it];
            if ((++it) != i.second.end()) firstSetFile << " , ";
        }
       firstSetFile << " }\n";
    }
}

void writeC() { // 输出文法的项目集族 C
    for (int i = 0; i < I.size(); i++) {
        CFile << "I" << i << ":\n";
        for (auto& item : I[i]) {
            CFile << "[ ";
            auto& vec = item.itrG->second[item.branchId];
            CFile << item.itrG->first << "->";
            for (int j = 0; j < vec.size(); j++) {
                if (j == item.dotPos) CFile << ".";
                if (vec[j].first) CFile << VN[vec[j].second];
                else CFile << VT[vec[j].second];
            }
            if (item.dotPos == vec.size()) CFile << ".";
            CFile << " , " << VT[item.right];
            CFile << " ]\n";
        }
        CFile << "\n";
    }

    std::cout << "Write C done!" << "\n";
}

void writeGO() {
    for (auto& i : GO) {
        int I, type, idx, J;
        std::tie(I, type, idx) = i.first, J = i.second;
        CFile << "GO(I" << I << ", ";
        CFile << (type ? VN[idx] : VT[idx]);
        CFile << ") = " << J << "\n";
    }

    std::cout << "Write GO done!" << "\n";
}

void printItem(const Item& item) { // for debug 输出项目
    std::cout << "Debug item: ";
    auto& vec = item.itrG->second[item.branchId];
    std::cout << item.itrG->first << "->";
    for (int j = 0; j < vec.size(); j++) {
        if (j == item.dotPos) std::cout << ".";
        if (vec[j].first) // is VN
            std::cout << VN[vec[j].second];
        else // is VT
            std::cout << VT[vec[j].second];
    }
    if (item.dotPos == vec.size()) std::cout << ".";
    std::cout << "\n";
}

void printVN_VT_S() { // for debug 输出变元 终结符
    std::cout << "VT: ";
    for (auto& i : VT) std::cout << i << " ";
    std::cout << "\n";
    std::cout << "VN: ";
    for (auto& i : VN) std::cout << i << " ";
    std::cout << "\n";
    std::cout << "S: " << S << "\n";
}

void printG() { // for debug 输出所有产生式
    for (auto& i : G) {
        std::cout << i.first << "->";
        const auto& tmp = i.second;
        for (int i = 0; i < tmp.size(); i++) {
            for (const auto& j : tmp[i]) {
                // std::cout << j.first << " " << j.second << std::endl;
                if (j.first) std::cout << VN[j.second];
                else std::cout << VT[j.second];
            }
            if (i + 1 != tmp.size()) std::cout << "|";
        }
        std::cout << "\n";
    }
    /* output format   @ means epsilon
        B->S
        S->AS|EXPR|b|@
    */
}

// 返回产生式对应字符串
std::string P_ToString(const std::string& gl, const std::vector<std::pair<bool, int>>& vec) {
    std::string ret;
    ret += gl + "->";
    for (auto& i : vec) {
        if (i.first) ret += VN[i.second]; // VN
        else ret += VT[i.second];// VT
    }
    return ret;
}

// 输入文法
void inputGrammar() {
    std::stringstream ss;
    std::string line, vt, vn;
    getline(grammarFile, line); // 读入终结符
    ss.clear(), ss << line;
    while (getline(ss, vt, ' ')) if (vt.size()) VT.push_back(vt);
    
    sort(VT.begin(), VT.end()); // 按字典序 可以删除

    VT.push_back("#"); // # 也做为终结符
    VT.push_back("@"); // epsilon 这里额外将 epsilon 也算作终结符

    getline(grammarFile, line); // 读入非终结符
    ss.clear(), ss << line;
    while (getline(ss, vn, ' ')) if (vn.size()) VN.push_back(vn);

    sort(VN.begin(), VN.end()); // 按字典序 可删除

    // 读入文法起始符号
    getline(grammarFile, line);
    ss.clear(), ss << line;
    getline(ss, S, ' ');

    printVN_VT_S(); // for debug

    int cntP(0); // 产生式个数
    while (getline(grammarFile, line)) { // 读入产生式
        ss.clear(), ss << line; // 读入文法 
        std::string gl, gr; char ch;
        
        bool has_ = false; // 判断文法有无->
        for (auto& c : line) if (c == '-') has_ = true;

        getline(ss, gl, '-'); // 读入产生式左部
        ss >> ch;
        if (ch != '>' || !has_) { // 判断文法有无->
            std::cerr << "Error grammar input! No '->' sign!: " << line << std::endl;
            closeFile();
            exit(0);
        }
        getline(ss, gr, '-'); // 读入右部

        if (std::find(VN.begin(), VN.end(), gl) == VN.end()) { // 左部不能含有多余空格
            for (auto& c : gl) if (c == ' ') c = '^';
            std::cerr << "Error grammar input! Left side VN is invalid: " << gl << std::endl;
            closeFile();
            exit(0);
        }

        ss.clear(), ss << gr; // 开始处理产生式右部
        std::string tmp;
        std::vector<std::pair<bool, int>> vec; // 用于获得 S->XiXj... 的 i, j  pair.first 是否为终结符 pair.second 是下标
        while (getline(ss, tmp, ' ')) {
            if (!tmp.size()) continue;
            auto idx = std::find(VN.begin(), VN.end(), tmp) - VN.begin();
            if (idx != VN.size()) { // Xi 为变元
                vec.emplace_back(true, idx);
            } else {
                idx = std::find(VT.begin(), VT.end(), tmp) - VT.begin();
                if (idx != VT.size()) { // Xi 为终结符
                    vec.emplace_back(false, idx);
                    tmp.clear();
                } else { // 既不是终结符也不是变元
                    std::cerr << "Error grammar input!: " << line << " " << "\nNo such VT or VN: " << tmp << std::endl;
                    closeFile();
                    exit(0);
                }
            }
        }
        std::string stmp = P_ToString(gl, vec);
        if (P.count(stmp)) { // 判断是否重复输入产生式
            std::cerr << "Multiple P input!: " << line << std::endl;
            closeFile();
            exit(0);
        }
        cntP++; // 产生式个数 + 1
        P[stmp] = cntP;
        G[gl].push_back(vec); // push_back 完整的右部信息
    }
    ss.clear();
}

// 求 first 集
void getFirstSet() {
    bool end = false;
    for (int i = 0; i < VT.size(); i++) if (VT[i] != "@") first[VT[i]].insert(i);
    while (!end) {
        end = true;
        for (const auto& g : G) {
            auto gl = g.first; // gl -> branch1 | branch2 and gl is in VN
            for (const auto& branch : g.second) {
                bool type = branch.front().first;
                int idx = branch.front().second;

                if (type == false) { // X->a...., a in VT
                    std::size_t preSize = first[gl].size();
                    for (auto& i : first[VT[idx]]) first[gl].insert(i);
                    if (VT[idx] == "@") first[gl].insert(idx);
                    if (first[gl].size() != preSize) end = false;
                } else { // X->Y..., Y in VN
                    std::size_t preSize = first[gl].size();

                    int epsPos = -1; // 找到起始连续的 eps
                    for (int i = 0; i < branch.size(); i++) {
                        type = branch[i].first, idx = branch[i].second;
                        // eps in First(Yj) and Yj in VN
                        if (type && first[VN[idx]].count(VT.size() - 1)) epsPos = i;
                        else break; // VT[VT.size() - 1] is @ --- eps
                    }
                    for (int i = 0; i <= epsPos; i++) {
                        for (auto& j : first[VN[branch[i].second]])
                            if (j + 1 != VT.size())  // First[Yi] - {eps}
                                first[gl].insert(j);
                    }
                    if (epsPos + 1 == branch.size()) {
                        first[gl].insert(VT.size() - 1); // add {eps}
                    } else {
                        if (branch[epsPos + 1].first == true) { // VN
                            for (auto& j : first[VN[branch[epsPos + 1].second]])
                                first[gl].insert(j);
                        } else { // VT
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

// 判断是否找到一个项目
bool foundItem(const std::vector<Item>& I, const Item& item) {
    for (auto& i : I) {
        if (i.itrG == item.itrG && i.branchId == item.branchId &&
            i.dotPos == item.dotPos && i.right == item.right)
            return true;
    }
    return false;
}

// 获得 first(βa) 的终结符;
std::set<int> getFirstVT(const std::vector<std::pair<bool, int>>& l,
                         std::size_t right, std::size_t dotPos) {
    std::set<int> ret;
    int epsPos = dotPos;
    for (int i = dotPos + 1; i < l.size(); i++) { // 找到连续的含有 eps 的 V
        bool type= l[i].first;
        int idx = l[i].second;
        if (type && first[VN[idx]].count(VT.size() - 1)) epsPos = i;
        else break; // VT[VT.size() - 1] is @ --- eps
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
            for (auto& j : first[VN[l[epsPos + 1].second]])
                ret.insert(j);
        } else {  // VT
            for (auto& j : first[VT[l[epsPos + 1].second]])
                ret.insert(j);
        }
    }
    return ret;
}

// 生成项目集 Ii 的闭包
void genClosure(std::vector<Item>& Ii) {
    for (int i = 0; i < Ii.size(); i++) {
        const auto& item = Ii[i];
        const auto& vec = item.itrG->second[item.branchId];
        if (item.dotPos == vec.size()) continue;  // 规约状态
        bool type = vec[item.dotPos].first;
        int idx = vec[item.dotPos].second;
        if (type == false) continue;  // VT 终结符跳过

        for (auto& iVT : getFirstVT(vec, item.right, item.dotPos)) {  // 对于每个 First(βa) 中的终结符
            Item newItem;
            auto itr = G.find(VN[idx]); // 获取 .Bβ 的 B -> 产生式迭代器
            
            if (itr == G.end()) {
                std::cerr << "GenClosure error! line: " << __LINE__
                          << std::endl;
                return;
            }
            for (int j = 0; j < itr->second.size(); j++) {
                newItem.itrG = itr, newItem.branchId = j;
                newItem.right = iVT, newItem.dotPos = 0;
                if (!foundItem(Ii, newItem)) Ii.push_back(newItem);
            }
        }
    }
}

// 判断项目集 Itmp 是否存在，返回项目集编号 brute force
int foundI(const std::vector<Item>& Itmp) { 
    for (int i = 0; i < I.size(); i++) {
        if (I[i].size() == Itmp.size()) { // 项目集大小相同才有可能相等
            bool same = true;
            for (auto& itemOfItmp : Itmp) { // 每一个项目
                bool foundItem = false; // 都应在 I[i] 中能找到
                for (auto& itemOfIi : I[i]) {
                    if (itemOfIi.itrG == itemOfItmp.itrG && itemOfIi.branchId == itemOfItmp.branchId
                    && itemOfIi.dotPos == itemOfItmp.dotPos && itemOfIi.right == itemOfItmp.right) {
                        foundItem = true;
                        break;
                    }
                }
                if (!foundItem) same = false;
            }
            if (same) return i;
        }
    }
    return -1;
}

// 生成 LR(1) 项目集族 C
void genC() {
    // 初始化 I0: [S'->.S, #]
    std::vector<Item> I0;
    I0.push_back((Item) {G.find(S), 0, 0, VT.size() - 2}); // VT[VT.size() - 2] is #
    genClosure(I0);
    I.push_back(I0);

    for (int i = 0; i < I.size(); i++) { // 对于每个项目 Ii
        for (int idx = 0; idx < VN.size(); idx++) { // 对于每个非终结符
            std::vector<Item> newI; // 新的项目集 vector
            for (const auto& item : I[i]) {
                const auto& vec = item.itrG->second[item.branchId];
                if (item.dotPos == vec.size()) continue;  // 规约状态
                auto nxtItem = item;
                nxtItem.dotPos++;  // . 右移一位

                bool Xtype = vec[item.dotPos].first;  // X of GO(I, X)
                int Xidx = vec[item.dotPos].second;
                // Xtype = 0 表示终结符 1 表示变元
                if (Xtype == 1 && idx == Xidx) newI.push_back(nxtItem);  // 加入.右移一位的项目
            }
            if (newI.empty()) continue;
            genClosure(newI);  // 生成 closure

            int to = foundI(newI);  // 计算 GO(I, X)
            if (to == -1) {        // 新的项目集
                to = I.size();
                I.push_back(newI);
            }
            GO[{i, 1, idx}] = to;  // GO(I, X) = j; X in VT
        }

        for (int idx = 0; idx < VT.size() - 2; idx++) { // 对于每个终结符 @ # 除外
            std::vector<Item> newI; // 新的项目集 vector
            for (const auto& item : I[i]) {
                const auto& vec = item.itrG->second[item.branchId];
                if (item.dotPos == vec.size()) continue;  // 规约状态
                auto nxtItem = item;
                nxtItem.dotPos++;  // . 右移一位

                bool Xtype = vec[item.dotPos].first;  // X of GO(I, X)
                int Xidx = vec[item.dotPos].second;
                // Xtype = 0 表示终结符 1 表示变元
                if (Xtype == 0 && idx == Xidx) newI.push_back(nxtItem);  // 加入.右移一位的项目
            }
            if (newI.empty()) continue;
            genClosure(newI);  // 生成 closure

            int to = foundI(newI);  // 计算 GO(I, X)
            if (to == -1) {        // 新的项目集
                to = I.size();
                I.push_back(newI);
            }
            GO[{i, 0, idx}] = to;  // GO(I, X) = j; X in VT
        }
    }
}

// pp's work
void writeLR1Table() {
    CFile << "\nLR(1) Table:\n";
    int sz = I.size();
    CFile << "ACTION:\n";
    for(int i = 0; i < sz; i ++)
    {
        CFile << "I" << i << ": ";
        for(auto str: VT)
        {
            CFile << str << "->";
            if(ACTION[i][str].first == -1)
                CFile << "err  ";
            else if(ACTION[i][str].first == 0)
                CFile << "acc  ";
            else if(ACTION[i][str].first == 1)
                CFile << "s" << ACTION[i][str].second << "  ";
            else if(ACTION[i][str].first == 2)
                CFile << "r" << ACTION[i][str].second << "  ";
        }
        CFile << "\n";
    }

    CFile << "GOTO:\n";
    for(int i = 0; i < sz; i ++)
    {
        CFile << "I" << i << ": ";
        for(auto str: VN)
        {
            CFile << str << "->";
            if(GOTO[i][str] == -1)
                CFile << "err  ";
            else
                CFile << GOTO[i][str] << "  ";
        }
        CFile << "\n";
    }
    std::cout << "Write LR1_Table done!" << "\n";
}

// pp's work
void genLR1Table() {
    //init
    int sz = I.size();
    std::map<std::string, std::pair<int, int>> temp1;
    std::map<std::string, int> temp2;

    for(auto str: VT)
        temp1[str] = {-1, 0};
    for(auto str: VN)
        temp2[str] = -1;

    for(int i = 0; i < sz; i ++)
    {
        ACTION.push_back(temp1);
        GOTO.push_back(temp2);

        for(auto &item: I[i])
        {
            auto &vec = item.itrG->second[item.branchId];
            int pos = item.dotPos;
            //condition 1
            if(pos < vec.size() && !vec[pos].first)
            {
                std::string tmp_str = VT[vec[pos].second];
                ACTION[i][tmp_str].first = 1;
                ACTION[i][tmp_str].second = GO[std::make_tuple(i, 0, vec[pos].second)];
            }
            //condition 2
            else if(pos == vec.size() && item.itrG->first != S)
            {
                std::string tmp_str = VT[item.right];
                ACTION[i][tmp_str].first = 2;
                
                std::string stmp = P_ToString(item.itrG->first, vec);
                ACTION[i][tmp_str].second = P[stmp] - 1;
                /*
                if (P.count(stmp)) { // 判断是否重复输入产生式
                    std::cerr << "Multiple P input!: " << line << std::endl;
                    closeFile();
                    exit(0);
                }
                cntP++; // 产生式个数 + 1
                P[stmp] = cntP;
                G[gl].push_back(vec); // push_back 完整的右部信息
                */
            }
            //condition 3
            else if(pos == vec.size() && item.itrG->first == S)
            {
                std::string tmp_str = VT[item.right];
                ACTION[i][tmp_str].first = 0;
            }
        }
    }

    for (auto &i: GO) 
    {
        int I, type, idx, J;
        std::tie(I, type, idx) = i.first, J = i.second;
        if(!type) continue;
        GOTO[I][VN[idx]] = J;
    }
}

void LR1() { // LR1分析法 入口
    inputGrammar(); // 输入文法
    printG();

    getFirstSet(); // 生成 first 集和
    writeFirst();

    genC(); // 生成 LR(1) 项目集族 C

    genLR1Table();//生成LR(1)分析表
    writeC();
    writeGO();
    writeLR1Table();
}

int main() {
    openFile();
    LR1();
    closeFile();
    return 0;
}