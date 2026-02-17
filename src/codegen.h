#pragma once
#include "ast.h"
#include <string>
#include <vector>
#include <utility>

class CodeGen
{
public:
    std::string generate(Program *program);

private:
    // All generator functions now need Program* to access globals
    std::string generateFunction(Function *func, Program *program);
    std::string generateStatement(Statement *stmt, Function *func, Program *program);
    std::string generateExpression(Expression *expr, Function *func, Program *program);
    std::string escapeString(const std::string &s);
    std::vector<std::pair<bool, std::string>> parseInterpolated(const std::string &s);
    std::string getVariableType(const std::string &name, Function *func, Program *program);
};