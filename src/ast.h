#pragma once
#include <string>
#include <vector>
#include <memory>

// Forward declarations
struct Expression;
struct Statement;

struct ASTNode
{
    virtual ~ASTNode() = default;
};

// -------------------- Expressions --------------------
struct Expression : ASTNode
{
    virtual ~Expression() = default;
};

struct LiteralExpr : Expression
{
    std::string value;
    bool isString;
    std::string kind;
    LiteralExpr(const std::string &val, bool str = false, const std::string &k = "normal")
        : value(val), isString(str), kind(k) {}
};

struct VariableExpr : Expression
{
    std::string name;
    VariableExpr(const std::string &n) : name(n) {}
};

struct CallExpr : Expression
{
    std::string callee;
    std::vector<std::unique_ptr<Expression>> arguments;
    CallExpr(const std::string &c) : callee(c) {}
};

struct BinaryExpr : Expression
{
    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;
    BinaryExpr(std::unique_ptr<Expression> l, const std::string &o, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

struct UnaryExpr : Expression
{
    std::string op;
    std::unique_ptr<Expression> expr;
    UnaryExpr(const std::string &o, std::unique_ptr<Expression> e)
        : op(o), expr(std::move(e)) {}
};

// -------------------- Statements --------------------
struct Statement : ASTNode
{
};

struct ExpressionStatement : Statement
{
    std::unique_ptr<Expression> expr;
    ExpressionStatement(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
};

struct PrintStatement : Statement
{
    std::unique_ptr<Expression> expr;
    PrintStatement(std::unique_ptr<Expression> e) : expr(std::move(e)) {}
};

struct ReturnStatement : Statement
{
    std::unique_ptr<Expression> value;
    ReturnStatement(std::unique_ptr<Expression> v) : value(std::move(v)) {}
};

struct StopStatement : Statement
{
    StopStatement() = default;
};

struct VarDeclaration : Statement
{
    std::string type;
    std::string name;
    std::string value;
    bool isString;
    std::string kind;
    VarDeclaration(const std::string &t, const std::string &n, const std::string &v, bool str, const std::string &k = "normal")
        : type(t), name(n), value(v), isString(str), kind(k) {}
};

struct Assignment : Statement
{
    std::string name;
    std::unique_ptr<Expression> value;
    Assignment(const std::string &n, std::unique_ptr<Expression> v)
        : name(n), value(std::move(v)) {}
};

struct Block : Statement
{
    std::vector<std::unique_ptr<Statement>> statements;
};

struct IfStatement : Statement
{
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> thenBlock;
    std::unique_ptr<Block> elseBlock;
    IfStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Block> thenB, std::unique_ptr<Block> elseB = nullptr)
        : condition(std::move(cond)), thenBlock(std::move(thenB)), elseBlock(std::move(elseB)) {}
};

struct WhileStatement : Statement
{
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> body;
    WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Block> b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

// -------------------- Global declarations --------------------
struct GlobalVarDeclaration : ASTNode
{
    std::string type;
    std::string name;
    std::string value;
    bool isString;
    std::string kind;
    GlobalVarDeclaration(const std::string &t, const std::string &n, const std::string &v, bool str, const std::string &k = "normal")
        : type(t), name(n), value(v), isString(str), kind(k) {}
};

// Function
struct Function : ASTNode
{
    std::string name;
    std::vector<std::pair<std::string, std::string>> parameters;
    std::vector<std::unique_ptr<Statement>> body;
    Function(const std::string &n) : name(n) {}
};

// Program
struct Program : ASTNode
{
    std::vector<std::unique_ptr<GlobalVarDeclaration>> globals;
    std::vector<std::unique_ptr<Function>> functions;
};