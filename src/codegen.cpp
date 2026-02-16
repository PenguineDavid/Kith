#include "codegen.h"
#include "ast.h"
#include <cstring> // for strlen
#include <cstdlib> // for malloc, free
#include <vector>
#include <string>
#include <utility> // for std::pair

std::string CodeGen::generate(Program *program)
{
    std::string code = "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n";
    for (auto &func : program->functions)
    {
        code += generateFunction(func.get());
        code += "\n";
    }
    return code;
}

std::string CodeGen::generateFunction(Function *func)
{
    std::string code = "int " + func->name + "() {\n";

    for (auto &stmt : func->body)
    {
        code += generateStatement(stmt.get(), func);
    }

    code += "    return 0;\n";
    code += "}\n";
    return code;
}

std::string CodeGen::escapeString(const std::string &s)
{
    std::string escaped;
    for (char c : s)
    {
        switch (c)
        {
        case '"':
            escaped += "\\\"";
            break;
        case '\\':
            escaped += "\\\\";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped += c;
            break;
        }
    }
    return escaped;
}

std::vector<std::pair<bool, std::string>> CodeGen::parseInterpolated(const std::string &s)
{
    std::vector<std::pair<bool, std::string>> parts;
    size_t pos = 0;
    while (pos < s.size())
    {
        size_t brace = s.find('{', pos);
        if (brace == std::string::npos)
        {
            parts.push_back({false, s.substr(pos)});
            break;
        }
        if (brace > pos)
            parts.push_back({false, s.substr(pos, brace - pos)});

        size_t close = s.find('}', brace + 1);
        if (close == std::string::npos)
        {
            // unmatched brace – treat as literal
            parts.push_back({false, s.substr(pos)});
            break;
        }
        std::string varName = s.substr(brace + 1, close - brace - 1);
        parts.push_back({true, varName});
        pos = close + 1;
    }
    return parts;
}

std::string CodeGen::generateExpression(Expression *expr)
{
    if (auto lit = dynamic_cast<LiteralExpr *>(expr))
    {
        if (lit->isString)
        {
            return "\"" + escapeString(lit->value) + "\"";
        }
        else
        {
            // boolean or number literal
            return lit->value;
        }
    }
    else if (auto var = dynamic_cast<VariableExpr *>(expr))
    {
        return var->name;
    }
    else if (auto bin = dynamic_cast<BinaryExpr *>(expr))
    {
        std::string left = generateExpression(bin->left.get());
        std::string right = generateExpression(bin->right.get());
        std::string op = bin->op;

        // Map logical operators to C equivalents
        if (op == "and")
            op = "&&";
        else if (op == "or")
            op = "||";
        else if (op == "xor")
            op = "^"; // C lacks logical xor; use bitwise xor for bools
        else if (op == "==")
            op = "==";
        else if (op == "!=")
            op = "!=";
        else if (op == "<")
            op = "<";
        else if (op == ">")
            op = ">";
        else if (op == "<=")
            op = "<=";
        else if (op == ">=")
            op = ">=";
        else if (op == "+")
            op = "+";
        else if (op == "-")
            op = "-";
        else if (op == "*")
            op = "*";
        else if (op == "/")
            op = "/";
        else if (op == "%")
            op = "%";
        // bitwise operators &, |, ^, ~ are already the same in C

        return "(" + left + " " + op + " " + right + ")";
    }
    else if (auto un = dynamic_cast<UnaryExpr *>(expr))
    {
        std::string sub = generateExpression(un->expr.get());
        std::string op = un->op;
        if (op == "not")
            op = "!";
        else if (op == "~")
            op = "~";
        // minus is already "-"
        return "(" + op + " " + sub + ")";
    }
    return "";
}

std::string CodeGen::generateStatement(Statement *stmt, Function *func)
{
    // Print statement
    if (auto printStmt = dynamic_cast<PrintStatement *>(stmt))
    {
        std::string value = printStmt->value;
        std::string kind = printStmt->kind;

        // Check if value matches a variable name in the function
        for (auto &s : func->body)
        {
            if (auto v = dynamic_cast<VarDeclaration *>(s.get()))
            {
                if (v->name == value)
                {
                    if (v->isString)
                        return "    printf(\"%s\\n\", " + value + ");\n";
                    else
                        return "    printf(\"%d\\n\", " + value + ");\n";
                }
            }
        }

        // Otherwise treat as literal string
        if (kind == "interp")
        {
            // Interpolated literal: build a temporary string and print it
            auto parts = parseInterpolated(value);

            std::string format;
            std::vector<std::string> vars;
            for (auto &p : parts)
            {
                if (p.first) // variable
                {
                    format += "%s";
                    vars.push_back(p.second);
                }
                else // literal
                {
                    for (char c : p.second)
                    {
                        if (c == '%')
                            format += "%%"; // escape % for printf
                        else
                            format += c;
                    }
                }
            }

            std::string code = "    {\n";
            code += "        int len = ";
            for (size_t i = 0; i < parts.size(); i++)
            {
                if (i > 0)
                    code += " + ";
                if (parts[i].first)
                    code += "strlen(" + parts[i].second + ")";
                else
                    code += std::to_string(parts[i].second.size());
            }
            code += " + 1;\n";
            code += "        char* temp = malloc(len);\n";
            code += "        snprintf(temp, len, \"" + format + "\"";
            for (auto &v : vars)
                code += ", " + v;
            code += ");\n";
            code += "        printf(\"%s\\n\", temp);\n";
            code += "        free(temp);\n";
            code += "    }\n";
            return code;
        }
        else
        {
            // Normal, raw, or multi‑line string: just escape and print
            std::string escaped = escapeString(value);
            return "    printf(\"" + escaped + "\\n\");\n";
        }
    }

    // Variable declaration
    if (auto var = dynamic_cast<VarDeclaration *>(stmt))
    {
        if (var->isString)
        {
            if (var->kind == "interp")
            {
                auto parts = parseInterpolated(var->value);

                std::string format;
                std::vector<std::string> vars;
                for (auto &p : parts)
                {
                    if (p.first)
                    {
                        format += "%s";
                        vars.push_back(p.second);
                    }
                    else
                    {
                        for (char c : p.second)
                        {
                            if (c == '%')
                                format += "%%";
                            else
                                format += c;
                        }
                    }
                }

                // Generate code WITHOUT extra braces so the variable stays in scope
                std::string code = "    int len_" + var->name + " = ";
                for (size_t i = 0; i < parts.size(); i++)
                {
                    if (i > 0)
                        code += " + ";
                    if (parts[i].first)
                        code += "strlen(" + parts[i].second + ")";
                    else
                        code += std::to_string(parts[i].second.size());
                }
                code += " + 1;\n";
                code += "    char* " + var->name + " = malloc(len_" + var->name + ");\n";
                code += "    snprintf(" + var->name + ", len_" + var->name + ", \"" + format + "\"";
                for (auto &v : vars)
                    code += ", " + v;
                code += ");\n";
                return code;
            }
            else
            {
                std::string escaped = escapeString(var->value);
                return "    char* " + var->name + " = \"" + escaped + "\";\n";
            }
        }
        else
        {
            // Integer variable
            return "    int " + var->name + " = " + var->value + ";\n";
        }
    }

    // Assignment statement
    if (auto assign = dynamic_cast<Assignment *>(stmt))
    {
        std::string expr = generateExpression(assign->value.get());
        return "    " + assign->name + " = " + expr + ";\n";
    }

    // If statement
    if (auto ifStmt = dynamic_cast<IfStatement *>(stmt))
    {
        std::string cond = generateExpression(ifStmt->condition.get());
        std::string code = "    if (" + cond + ") {\n";

        for (auto &s : ifStmt->thenBlock->statements)
        {
            code += generateStatement(s.get(), func);
        }

        if (ifStmt->elseBlock)
        {
            code += "    } else {\n";
            for (auto &s : ifStmt->elseBlock->statements)
            {
                code += generateStatement(s.get(), func);
            }
        }
        code += "    }\n";
        return code;
    }

    // While loop
    if (auto whileStmt = dynamic_cast<WhileStatement *>(stmt))
    {
        std::string cond = generateExpression(whileStmt->condition.get());
        std::string code = "    while (" + cond + ") {\n";
        for (auto &s : whileStmt->body->statements)
        {
            code += generateStatement(s.get(), func);
        }
        code += "    }\n";
        return code;
    }

    return "";
}