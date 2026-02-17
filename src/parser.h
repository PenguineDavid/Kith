#pragma once
#include "lexer.h"
#include "ast.h"
#include <vector>
#include <memory>

class Parser
{
public:
    Parser(const std::vector<Token> &tokens);
    std::unique_ptr<Program> parse();

private:
    std::vector<Token> tokens;
    size_t position;

    Token &current();
    void advance();
    bool match(TokenType type);
    bool check(TokenType type);
    Token previous();

    std::unique_ptr<Function> parseFunction();
    std::unique_ptr<GlobalVarDeclaration> parseGlobalDeclaration(); // <-- new
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Block> parseBlock();

    // Expression parsing
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseLogicalOr();
    std::unique_ptr<Expression> parseLogicalXor();
    std::unique_ptr<Expression> parseLogicalAnd();
    std::unique_ptr<Expression> parseEquality();
    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parseAdditive();
    std::unique_ptr<Expression> parseMultiplicative();
    std::unique_ptr<Expression> parseUnary();
    std::unique_ptr<Expression> parsePrimary();
};