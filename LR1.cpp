#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <set>
#include <map>
#include <algorithm>

std::ifstream grammarFile;
std::ifstream inputFile;
std::ofstream firstSetFile;

std::string S;
std::vector<std::string> VT; // 终结符
std::vector<std::string> VN; // 非终结符
std::map<std::string, std::vector<std::vector<std::pair<bool, int>>>> G; // 产生式 如 Si->X1X2X3...
std::map<std::string, std::set<int>> first; // first 集

struct Item {
    decltype(G.begin()) itrG;
    std::size_t branchId;
    std::size_t dotPos; // . 的位置
    std::string right;
};
std::vector<std::vector<Item>> I; // 项目集 I0, I1, I2, ...

void openFile() {
    grammarFile.open("Grammar.txt");
}

void closeFile() {
    grammarFile.close();
    std::cout.flush();
}

void printVN_VT() { // for debug 输出变元 终结符
    for (auto& i : VT) std::cout << i << " ";
    std::cout << "\n";
    for (auto& i : VN) std::cout << i << " ";
    std::cout << "\n";
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

void printFirst() { // 输出 first 集和
    for (const auto& i : first) {
        std::cout << "First(" << i.first << ") = { ";
        for (auto it = i.second.begin(); it != i.second.end();) {
            std::cout << VT[*it];
            if ((++it) != i.second.end()) std::cout << " , ";
        }
        std::cout << " }\n";
    }
}

void inputGrammar() { // 输入文法
    std::stringstream ss;
    std::string line, vt, vn;
    getline(grammarFile, line); // 读入终结符
    ss.clear(), ss << line;
    while (getline(ss, vt, ' ')) if (vt.size()) VT.push_back(vt);
    VT.push_back("#"); // # 也做为终结符
    VT.push_back("@"); // epsilon 这里额外将 epsilon 也算作终结符

    getline(grammarFile, line); // 读入非终结符
    ss.clear(), ss << line;
    while (getline(ss, vn, ' ')) if (vn.size()) VN.push_back(vn);

    printVN_VT(); // for debug

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
        G[gl].push_back(vec); // push_back 完整的右部信息
    }
    ss.clear();
}

void getFirstSet() { // 求 first 集
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

int getFirst(const std::pair<bool, int>& l, const std::pair<bool, int>& r) { // 获得 first(βa);

}

void genClosure() {

}

void findI() {

}

void genGO() {

}

void genC() { // 生成 LR(1) 项目集族 C
    // 初始化 I0: [S'->.S, #]

    int curId = 0;
    bool end = false;
    while (!end) {
        end = true;
        
    }
}

void LR1() { // LR1分析法 入口
    inputGrammar(); // 输入文法
    printG();

    getFirstSet(); // 生成 first 集和
    printFirst();

    genC(); // 生成 LR(1) 项目集族 C
}

int main() {
    openFile();
    LR1();
    closeFile();
    return 0;
}