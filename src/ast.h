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
    std::string value; // numeric, string, or boolean ("true"/"false")
    bool isString;     // true for string literals
    LiteralExpr(const std::string &val, bool str = false) : value(val), isString(str) {}
};

struct VariableExpr : Expression
{
    std::string name;
    VariableExpr(const std::string &n) : name(n) {}
};

struct BinaryExpr : Expression
{
    std::unique_ptr<Expression> left;
    std::string op; // e.g., "+", "-", "==", "and", "or"
    std::unique_ptr<Expression> right;
    BinaryExpr(std::unique_ptr<Expression> l, const std::string &o, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

struct UnaryExpr : Expression
{
    std::string op; // "not", "-", "~"
    std::unique_ptr<Expression> expr;
    UnaryExpr(const std::string &o, std::unique_ptr<Expression> e)
        : op(o), expr(std::move(e)) {}
};

// -------------------- Statements --------------------
struct Statement : ASTNode
{
};

struct PrintStatement : Statement
{
    std::string value;
    std::string kind; // "normal", "raw", "interp", "multi"
    PrintStatement(const std::string &val, const std::string &k = "normal")
        : value(val), kind(k) {}
};

struct VarDeclaration : Statement
{
    std::string type; // "int", "str", etc.
    std::string name;
    std::string value; // for simple initialisation (string or number)
    bool isString;
    std::string kind; // for strings: "normal", "raw", "interp", "multi"
    VarDeclaration(const std::string &t, const std::string &n, const std::string &v, bool str, const std::string &k = "normal")
        : type(t), name(n), value(v), isString(str), kind(k) {}
};

// Assignment statement: x = expression
struct Assignment : Statement
{
    std::string name;
    std::unique_ptr<Expression> value;
    Assignment(const std::string &n, std::unique_ptr<Expression> v)
        : name(n), value(std::move(v)) {}
};

// Block: a sequence of statements
struct Block : Statement
{
    std::vector<std::unique_ptr<Statement>> statements;
};

// If statement: condition, then block, optional else block
struct IfStatement : Statement
{
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> thenBlock;
    std::unique_ptr<Block> elseBlock; // may be null
    IfStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Block> thenB, std::unique_ptr<Block> elseB = nullptr)
        : condition(std::move(cond)), thenBlock(std::move(thenB)), elseBlock(std::move(elseB)) {}
};

// While loop
struct WhileStatement : Statement
{
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Block> body;
    WhileStatement(std::unique_ptr<Expression> cond, std::unique_ptr<Block> b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

// Function
struct Function : ASTNode
{
    std::string name;
    std::vector<std::unique_ptr<Statement>> body;
    Function(const std::string &n) : name(n) {}
};

// Program
struct Program : ASTNode
{
    std::vector<std::unique_ptr<Function>> functions;
};