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
    std::string generateFunction(Function *func);
    std::string generateStatement(Statement *stmt, Function *func);
    std::string generateExpression(Expression *expr); // new
    std::string escapeString(const std::string &s);
    std::vector<std::pair<bool, std::string>> parseInterpolated(const std::string &s);
};