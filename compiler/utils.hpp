#ifndef UTILS_HPP
#define UTILS_HPP

#include "ext/pb_ds/assoc_container.hpp"
#include "ext/pb_ds/hash_policy.hpp"

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

//关键字列表
const std::vector<std::string> keywords = {"int",  "void",  "if",
                                           "else", "while", "return"};

struct TOKEN { // 词法分析 TOKEN
    std::string token;  // token类型
    std::string value;  // 值
    int row;            // 行
    int col;            // 列
};

struct Item {            // LR1 项目
    std::size_t pId;     // 属于哪一条产生式
    std::size_t dotPos;  // . 的位置, 0 到 size
    std::size_t right;   // 右侧终结符下标
    bool operator<(const Item& other) const {
        if (pId == other.pId) {
            if (dotPos == other.dotPos) return right < other.right;
            return dotPos < other.dotPos;
        }
        return pId < other.pId;
    }
};

struct Quad {            // 四元式结构体
    int index;           // 四元式标号
    std::string ope;     // 操作类型
    std::string arg1;    // 操作数 1
    std::string arg2;    // 操作数 2
    std::string result;  // 运算结果
};

struct SemanticSymbol {  //语义分析中的符号
    TOKEN info;
    int tableOffset;  // 符号所在 table 的 offset
    int offset;   // 符号在 table 内部的 index
};

// 标识符 函数、变量、临时变量、常量的具体信息
struct Identifier {
    //几种标识符类型，分别为函数、变量、临时变量、常量、返回值
    enum IdentifierType { Function, Variable, TempVar, ConstVar, ReturnVar };

    IdentifierType identifierType;  // 标识符的类型
    std::string specifierType;      // 变(常)量类型/函数返回类型
    std::string identifierName;     // 标识符名称/常量值
    int functionParameterCount;     // 函数参数个数
    // int functionEntry;              // 函数入口地址(四元式的标号)
    // int functionTableIndex;  // 函数的函数符号表在整个程序的符号表列表中的索引
};

// 语义分析中的符号表
struct SymTable {
    // 全局表、函数表、块级表、临时表
    enum TableType { GlobalTable, FunctionTable, BlockTable, TempTable };

    TableType tableType;                // 表的类型
    std::vector<Identifier> table;  // 符号表
    __gnu_pbds::gp_hash_table<std::string, int> hashTable; // 杂凑表
    std::string tableName;              // 表名

    SymTable() = default;

    // 构造函数
    SymTable(const TableType& _tableType,
                          const std::string& _tableName) {
        tableType = _tableType;
        tableName = _tableName;
    }

    // 寻找一个变量
    int findSym(const std::string& id_name) {
        auto it = hashTable.find(id_name);
        if (it != hashTable.end()) return it->second; // 存在该项
        return -1;
    }

    // 加入一个变量，返回加入的位置
    int addSym(const Identifier& id) {
        if (hashTable.find(id.identifierName) == hashTable.end()) { // 不存在 id
            table.push_back(id);
            return hashTable[id.identifierName] = static_cast<int>(table.size()) - 1;
        }
        return -1;
    }
};

#endif