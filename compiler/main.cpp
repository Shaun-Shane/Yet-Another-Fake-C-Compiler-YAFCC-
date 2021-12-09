#include "SyntaxAnalyzer.h"
#include <windows.h>

int main(int argc, char *argv[]) {
    std::string codePath;
    if (argc == 1) {
        std::cerr << "Please input file path (i.e. testfiles/test.cpp)";
        std::cin >> codePath;
    } else codePath = argv[1];

    LexAnalyzer* lex = new LexAnalyzer(codePath);
    LR1* lr1 = new LR1(codePath);
    lex->writeAnalyzeResult();
    lr1->parse(lex->get_stream());

    delete lex;
    delete lr1;
    system("pause");
    return 0;
}