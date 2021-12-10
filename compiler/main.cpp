#include "SyntaxAnalyzer.h"
#include <windows.h>

int main(int argc, char *argv[]) {
    std::string codePath;
    if (argc == 1) {
        std::cerr << "Please input file path (i.e. testfiles/test.cpp)";
        std::cin >> codePath;
    } else codePath = argv[1];

    LexAnalyzer* lex = nullptr;
    LR1* lr1 = nullptr;

    try {
        lex = new LexAnalyzer(codePath);
        lr1 = new LR1(codePath);
        lex->writeAnalyzeResult();
        lr1->parse(lex->get_stream());
        delete lex;
        delete lr1;
        system("pause");
    } catch (std::string err) {
        std::cerr << err << std::endl;
        if (lex != nullptr) delete lex;
        if (lr1 != nullptr) delete lr1;
        system("pause");
    }
    return 0;
}