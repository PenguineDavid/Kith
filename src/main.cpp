#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: glyph <input.glyph> [output_exe]\n";
        std::cout << "If output_exe is omitted, 'output' is used.\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open())
    {
        std::cout << "Error opening file: " << argv[1] << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto ast = parser.parse();

    CodeGen generator;
    std::string c_code = generator.generate(ast.get());

    // Always write intermediate C code to "output.c"
    std::ofstream out("output.c");
    out << c_code;
    out.close();

    // Determine output executable name
    std::string output_exe = (argc > 2) ? argv[2] : "output";

    // Compile with gcc
    std::string cmd = "gcc output.c -o " + output_exe;
    int result = system(cmd.c_str());
    if (result != 0)
    {
        std::cout << "Compilation failed.\n";
        return 1;
    }

    std::cout << "Compiled successfully: ./" << output_exe << "\n";
    return 0;
}