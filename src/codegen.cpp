#include "codegen.h"
#include "ast.h"
#include <cstring> // for strlen
#include <cstdlib> // for malloc, free
#include <vector>
#include <string>
#include <utility>   // for std::pair
#include <stdexcept> // for std::runtime_error
#include <set>       // for std::set

std::string CodeGen::generate(Program *program)
{
    std::string code = "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\n";

    // Generate global variables
    for (auto &g : program->globals)
    {
        if (g->isString)
        {
            if (g->kind == "interp")
            {
                // Interpolated string globals need runtime init; for now just declare as NULL.
                code += "char* " + g->name + " = NULL; // interpolated global not fully implemented\n";
            }
            else
            {
                std::string escaped = escapeString(g->value);
                code += "char* " + g->name + " = \"" + escaped + "\";\n";
            }
        }
        else
        {
            code += "int " + g->name + " = " + g->value + ";\n";
        }
    }
    code += "\n";

    // Collect names of all defined functions
    std::set<std::string> definedFunctions;
    for (auto &func : program->functions)
    {
        definedFunctions.insert(func->name);
    }

    // Generate all user functions (except main, which we will replace)
    for (auto &func : program->functions)
    {
        if (func->name == "main")
            continue;
        code += generateFunction(func.get(), program);
        code += "\n";
    }

    // Generate the custom main function
    code += "int main() {\n";

    if (definedFunctions.count("init"))
        code += "    init();\n";
    if (definedFunctions.count("setup"))
        code += "    setup();\n";
    if (definedFunctions.count("start"))
        code += "    start();\n";

    code += "    while (1) {\n";
    if (definedFunctions.count("loop"))
        code += "        loop();\n";
    else if (definedFunctions.count("update"))
        code += "        update();\n";
    else if (definedFunctions.count("run"))
        code += "        run();\n";
    else
        code += "        // no loop function defined\n";
    code += "    }\n";

    if (definedFunctions.count("shutdown"))
        code += "    shutdown();\n";

    code += "    return 0;\n";
    code += "}\n";

    return code;
}

std::string CodeGen::generateFunction(Function *func, Program *program)
{
    std::string code = "int " + func->name + "(";

    for (size_t i = 0; i < func->parameters.size(); i++)
    {
        auto &param = func->parameters[i];
        if (param.first == "str")
            code += "char* " + param.second;
        else
            code += "int " + param.second;
        if (i < func->parameters.size() - 1)
            code += ", ";
    }
    code += ") {\n";

    for (auto &stmt : func->body)
    {
        code += generateStatement(stmt.get(), func, program);
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
            parts.push_back({false, s.substr(pos)});
            break;
        }
        std::string varName = s.substr(brace + 1, close - brace - 1);
        parts.push_back({true, varName});
        pos = close + 1;
    }
    return parts;
}

// Helper: look up the type of a variable (parameter, local, or global)
std::string CodeGen::getVariableType(const std::string &name, Function *func, Program *program)
{
    // Check parameters
    for (auto &param : func->parameters)
    {
        if (param.second == name)
            return param.first;
    }
    // Check local variables
    for (auto &stmt : func->body)
    {
        if (auto var = dynamic_cast<VarDeclaration *>(stmt.get()))
        {
            if (var->name == name)
                return var->type;
        }
    }
    // Check globals
    for (auto &g : program->globals)
    {
        if (g->name == name)
            return g->type;
    }
    return "str"; // fallback
}

std::string CodeGen::generateExpression(Expression *expr, Function *func, Program *program)
{
    if (auto lit = dynamic_cast<LiteralExpr *>(expr))
    {
        if (lit->isString)
            return "\"" + escapeString(lit->value) + "\"";
        else
            return lit->value;
    }
    else if (auto var = dynamic_cast<VariableExpr *>(expr))
    {
        return var->name;
    }
    else if (auto call = dynamic_cast<CallExpr *>(expr))
    {
        if (call->callee == "int")
        {
            if (call->arguments.size() != 1)
                throw std::runtime_error("int() expects exactly one argument");
            std::string arg = generateExpression(call->arguments[0].get(), func, program);
            return "(int)(" + arg + ")";
        }
        else if (call->callee == "str")
        {
            if (call->arguments.size() != 1)
                throw std::runtime_error("str() expects exactly one argument");
            std::string arg = generateExpression(call->arguments[0].get(), func, program);
            return "(char*)(" + arg + ")";
        }
        else
        {
            std::string code = call->callee + "(";
            for (size_t i = 0; i < call->arguments.size(); i++)
            {
                code += generateExpression(call->arguments[i].get(), func, program);
                if (i < call->arguments.size() - 1)
                    code += ", ";
            }
            code += ")";
            return code;
        }
    }
    else if (auto bin = dynamic_cast<BinaryExpr *>(expr))
    {
        std::string left = generateExpression(bin->left.get(), func, program);
        std::string right = generateExpression(bin->right.get(), func, program);
        std::string op = bin->op;

        if (op == "and")
            op = "&&";
        else if (op == "or")
            op = "||";
        else if (op == "xor")
            op = "^";
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

        return "(" + left + " " + op + " " + right + ")";
    }
    else if (auto un = dynamic_cast<UnaryExpr *>(expr))
    {
        std::string sub = generateExpression(un->expr.get(), func, program);
        std::string op = un->op;
        if (op == "not")
            op = "!";
        else if (op == "~")
            op = "~";
        return "(" + op + " " + sub + ")";
    }
    return "";
}

std::string CodeGen::generateStatement(Statement *stmt, Function *func, Program *program)
{
    // Print statement
    if (auto printStmt = dynamic_cast<PrintStatement *>(stmt))
    {
        if (auto lit = dynamic_cast<LiteralExpr *>(printStmt->expr.get()))
        {
            if (lit->isString)
            {
                if (lit->kind == "interp")
                {
                    auto parts = parseInterpolated(lit->value);
                    std::string format;
                    std::vector<std::pair<std::string, std::string>> varInfo;
                    for (auto &p : parts)
                    {
                        if (p.first)
                        {
                            std::string type = getVariableType(p.second, func, program);
                            varInfo.push_back({p.second, type});
                            if (type == "str")
                                format += "%s";
                            else
                                format += "%d";
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

                    std::string code = "    {\n";
                    code += "        int len = ";
                    for (size_t i = 0; i < parts.size(); i++)
                    {
                        if (i > 0)
                            code += " + ";
                        if (parts[i].first)
                        {
                            std::string varName = parts[i].second;
                            std::string type = getVariableType(varName, func, program);
                            if (type == "str")
                                code += "strlen(" + varName + ")";
                            else
                                code += "snprintf(NULL, 0, \"%d\", " + varName + ")";
                        }
                        else
                            code += std::to_string(parts[i].second.size());
                    }
                    code += " + 1;\n";
                    code += "        char* temp = malloc(len);\n";
                    code += "        snprintf(temp, len, \"" + format + "\"";
                    for (auto &vi : varInfo)
                        code += ", " + vi.first;
                    code += ");\n";
                    code += "        printf(\"%s\\n\", temp);\n";
                    code += "        free(temp);\n";
                    code += "    }\n";
                    return code;
                }
                else
                {
                    std::string escaped = escapeString(lit->value);
                    return "    printf(\"" + escaped + "\\n\");\n";
                }
            }
            else
            {
                return "    printf(\"%d\\n\", " + lit->value + ");\n";
            }
        }
        else if (auto var = dynamic_cast<VariableExpr *>(printStmt->expr.get()))
        {
            std::string type = getVariableType(var->name, func, program);
            if (type == "str")
                return "    printf(\"%s\\n\", " + var->name + ");\n";
            else
                return "    printf(\"%d\\n\", " + var->name + ");\n";
        }
        else
        {
            std::string exprCode = generateExpression(printStmt->expr.get(), func, program);
            return "    printf(\"%d\\n\", " + exprCode + ");\n";
        }
    }

    // Return statement
    if (auto retStmt = dynamic_cast<ReturnStatement *>(stmt))
    {
        std::string expr = generateExpression(retStmt->value.get(), func, program);
        return "    return " + expr + ";\n";
    }

    // Stop statement
    if (auto stopStmt = dynamic_cast<StopStatement *>(stmt))
    {
        return "    exit(0);\n";
    }

    // Variable declaration (local)
    if (auto var = dynamic_cast<VarDeclaration *>(stmt))
    {
        if (var->isString)
        {
            if (var->kind == "interp")
            {
                auto parts = parseInterpolated(var->value);

                std::string format;
                std::vector<std::pair<std::string, std::string>> varInfo;
                for (auto &p : parts)
                {
                    if (p.first)
                    {
                        std::string type = getVariableType(p.second, func, program);
                        varInfo.push_back({p.second, type});
                        if (type == "str")
                            format += "%s";
                        else
                            format += "%d";
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

                std::string code = "    int len_" + var->name + " = ";
                for (size_t i = 0; i < parts.size(); i++)
                {
                    if (i > 0)
                        code += " + ";
                    if (parts[i].first)
                    {
                        std::string varName = parts[i].second;
                        std::string type = getVariableType(varName, func, program);
                        if (type == "str")
                            code += "strlen(" + varName + ")";
                        else
                            code += "snprintf(NULL, 0, \"%d\", " + varName + ")";
                    }
                    else
                        code += std::to_string(parts[i].second.size());
                }
                code += " + 1;\n";
                code += "    char* " + var->name + " = malloc(len_" + var->name + ");\n";
                code += "    snprintf(" + var->name + ", len_" + var->name + ", \"" + format + "\"";
                for (auto &vi : varInfo)
                    code += ", " + vi.first;
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
            return "    int " + var->name + " = " + var->value + ";\n";
        }
    }

    // Assignment statement
    if (auto assign = dynamic_cast<Assignment *>(stmt))
    {
        std::string expr = generateExpression(assign->value.get(), func, program);
        return "    " + assign->name + " = " + expr + ";\n";
    }

    // If statement
    if (auto ifStmt = dynamic_cast<IfStatement *>(stmt))
    {
        std::string cond = generateExpression(ifStmt->condition.get(), func, program);
        std::string code = "    if (" + cond + ") {\n";

        for (auto &s : ifStmt->thenBlock->statements)
        {
            code += generateStatement(s.get(), func, program);
        }

        if (ifStmt->elseBlock)
        {
            code += "    } else {\n";
            for (auto &s : ifStmt->elseBlock->statements)
            {
                code += generateStatement(s.get(), func, program);
            }
        }
        code += "    }\n";
        return code;
    }

    // While loop
    if (auto whileStmt = dynamic_cast<WhileStatement *>(stmt))
    {
        std::string cond = generateExpression(whileStmt->condition.get(), func, program);
        std::string code = "    while (" + cond + ") {\n";
        for (auto &s : whileStmt->body->statements)
        {
            code += generateStatement(s.get(), func, program);
        }
        code += "    }\n";
        return code;
    }

    // Expression statement (e.g., function call)
    if (auto exprStmt = dynamic_cast<ExpressionStatement *>(stmt))
    {
        return "    " + generateExpression(exprStmt->expr.get(), func, program) + ";\n";
    }

    return "";
}