#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: yourlang <file.glyph>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open())
    {
        std::cout << "Error opening file.\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // Lexing
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    // Parsing
    Parser parser(tokens);
    auto ast = parser.parse();

    // Code Generation
    CodeGen generator;
    std::string c_code = generator.generate(ast.get());

    // Write C file
    std::ofstream out("output.c");
    out << c_code;
    out.close();

    // Compile C file to native executable
    int result = system("gcc output.c -o output");
    if (result != 0)
    {
        std::cout << "Compilation failed.\n";
        return 1;
    }

    std::cout << "Compiled successfully: ./output\n";
    return 0;
}