#include "parser.h"
#include <stdexcept>
#include <memory>

Parser::Parser(const std::vector<Token> &t) : tokens(t), position(0) {}

Token &Parser::current() { return tokens[position]; }
void Parser::advance()
{
    if (position < tokens.size())
        position++;
}
bool Parser::match(TokenType type)
{
    if (current().type == type)
    {
        advance();
        return true;
    }
    return false;
}
bool Parser::check(TokenType type) { return current().type == type; }
Token Parser::previous() { return tokens[position - 1]; }

std::unique_ptr<Program> Parser::parse()
{
    auto program = std::make_unique<Program>();
    while (current().type != TokenType::END_OF_FILE)
    {
        if (current().type == TokenType::FUNC)
        {
            program->functions.push_back(parseFunction());
        }
        else if (current().type == TokenType::INT_TYPE || current().type == TokenType::STR_TYPE)
        {
            program->globals.push_back(parseGlobalDeclaration());
        }
        else
        {
            throw std::runtime_error("Expected function or global variable declaration");
        }
    }
    return program;
}

std::unique_ptr<GlobalVarDeclaration> Parser::parseGlobalDeclaration()
{
    std::string type = current().value;
    bool isString = (current().type == TokenType::STR_TYPE);
    advance();

    if (current().type != TokenType::IDENTIFIER)
        throw std::runtime_error("Expected global variable name");
    std::string name = current().value;
    advance();

    if (!match(TokenType::ASSIGN))
        throw std::runtime_error("Expected '=' in global variable declaration");

    std::string value;
    std::string kind = "normal";

    if (isString)
    {
        if (current().type != TokenType::STRING)
            throw std::runtime_error("Expected string literal");
        std::string rawValue = current().value;

        if (rawValue.rfind("r:", 0) == 0)
        {
            kind = "raw";
            rawValue = rawValue.substr(2);
        }
        else if (rawValue.rfind("$:", 0) == 0)
        {
            kind = "interp";
            rawValue = rawValue.substr(2);
        }
        else if (rawValue.rfind("m:", 0) == 0)
        {
            kind = "multi";
            rawValue = rawValue.substr(2);
        }

        value = rawValue;
        advance();
    }
    else
    {
        if (current().type != TokenType::NUMBER)
            throw std::runtime_error("Expected number literal");
        value = current().value;
        advance();
    }

    return std::make_unique<GlobalVarDeclaration>(type, name, value, isString, kind);
}

std::unique_ptr<Function> Parser::parseFunction()
{
    if (!match(TokenType::FUNC))
        throw std::runtime_error("Expected 'func'");
    if (current().type != TokenType::IDENTIFIER)
        throw std::runtime_error("Expected function name");
    std::string name = current().value;
    advance();

    if (!match(TokenType::LPAREN))
        throw std::runtime_error("Expected '('");

    auto func = std::make_unique<Function>(name);

    // Parse parameters
    if (!check(TokenType::RPAREN))
    {
        do
        {
            // Parse parameter type and name
            if (current().type != TokenType::INT_TYPE && current().type != TokenType::STR_TYPE)
                throw std::runtime_error("Expected parameter type (int/str)");
            std::string paramType = current().value;
            advance();

            if (current().type != TokenType::IDENTIFIER)
                throw std::runtime_error("Expected parameter name");
            std::string paramName = current().value;
            advance();

            func->parameters.push_back({paramType, paramName});
        } while (match(TokenType::COMMA));
    }

    if (!match(TokenType::RPAREN))
        throw std::runtime_error("Expected ')' after parameters");

    if (!match(TokenType::LBRACE))
        throw std::runtime_error("Expected '{'");

    while (current().type != TokenType::RBRACE)
    {
        func->body.push_back(parseStatement());
    }
    if (!match(TokenType::RBRACE))
        throw std::runtime_error("Expected '}'");
    return func;
}

std::unique_ptr<Statement> Parser::parseStatement()
{
    // Variable declaration
    if (current().type == TokenType::INT_TYPE || current().type == TokenType::STR_TYPE)
    {
        std::string type = current().value;
        bool isString = (current().type == TokenType::STR_TYPE);
        advance();

        if (current().type != TokenType::IDENTIFIER)
            throw std::runtime_error("Expected variable name");
        std::string name = current().value;
        advance();

        if (!match(TokenType::ASSIGN))
            throw std::runtime_error("Expected '=' in variable declaration");

        std::string value;
        std::string kind = "normal";

        if (isString)
        {
            if (current().type != TokenType::STRING)
                throw std::runtime_error("Expected string literal");
            std::string rawValue = current().value;

            if (rawValue.rfind("r:", 0) == 0)
            {
                kind = "raw";
                rawValue = rawValue.substr(2);
            }
            else if (rawValue.rfind("$:", 0) == 0)
            {
                kind = "interp";
                rawValue = rawValue.substr(2);
            }
            else if (rawValue.rfind("m:", 0) == 0)
            {
                kind = "multi";
                rawValue = rawValue.substr(2);
            }

            value = rawValue;
            advance();
        }
        else
        {
            if (current().type != TokenType::NUMBER)
                throw std::runtime_error("Expected number literal");
            value = current().value;
            advance();
        }

        return std::make_unique<VarDeclaration>(type, name, value, isString, kind);
    }

    // Assignment statement (identifier followed by '=')
    if (current().type == TokenType::IDENTIFIER)
    {
        // Look ahead for '='
        if (position + 1 < tokens.size() && tokens[position + 1].type == TokenType::ASSIGN)
        {
            std::string name = current().value;
            advance();                // consume identifier
            match(TokenType::ASSIGN); // consume '='
            auto expr = parseExpression();
            return std::make_unique<Assignment>(name, std::move(expr));
        }
    }

    // Print statement
    if (current().type == TokenType::IDENTIFIER && current().value == "print")
    {
        advance();
        if (!match(TokenType::LPAREN))
            throw std::runtime_error("Expected '(' after print");

        auto expr = parseExpression();

        if (!match(TokenType::RPAREN))
            throw std::runtime_error("Expected ')'");

        return std::make_unique<PrintStatement>(std::move(expr));
    }

    // Return statement
    if (match(TokenType::RETURN))
    {
        auto expr = parseExpression();
        return std::make_unique<ReturnStatement>(std::move(expr));
    }

    // Stop statement
    if (match(TokenType::STOP))
    {
        return std::make_unique<StopStatement>();
    }

    // If statement
    if (match(TokenType::IF))
    {
        if (!match(TokenType::LPAREN))
            throw std::runtime_error("Expected '(' after if");
        auto condition = parseExpression();
        if (!match(TokenType::RPAREN))
            throw std::runtime_error("Expected ')' after condition");

        auto thenBlock = parseBlock();
        std::unique_ptr<Block> elseBlock = nullptr;
        if (match(TokenType::ELSE))
        {
            elseBlock = parseBlock();
        }
        return std::make_unique<IfStatement>(std::move(condition), std::move(thenBlock), std::move(elseBlock));
    }

    // While loop
    if (match(TokenType::WHILE))
    {
        if (!match(TokenType::LPAREN))
            throw std::runtime_error("Expected '(' after while");
        auto condition = parseExpression();
        if (!match(TokenType::RPAREN))
            throw std::runtime_error("Expected ')' after condition");
        auto body = parseBlock();
        return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
    }

    // If none of the above, try to parse an expression as a statement (e.g., function call)
    auto expr = parseExpression();
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::unique_ptr<Block> Parser::parseBlock()
{
    if (!match(TokenType::LBRACE))
        throw std::runtime_error("Expected '{'");
    auto block = std::make_unique<Block>();
    while (current().type != TokenType::RBRACE && current().type != TokenType::END_OF_FILE)
    {
        block->statements.push_back(parseStatement());
    }
    if (!match(TokenType::RBRACE))
        throw std::runtime_error("Expected '}'");
    return block;
}

// -------------------- Expression Parsing --------------------

std::unique_ptr<Expression> Parser::parseExpression()
{
    return parseLogicalOr();
}

std::unique_ptr<Expression> Parser::parseLogicalOr()
{
    auto expr = parseLogicalXor();
    while (match(TokenType::OR))
    {
        std::string op = previous().value; // "or"
        auto right = parseLogicalXor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseLogicalXor()
{
    auto expr = parseLogicalAnd();
    while (match(TokenType::XOR))
    {
        std::string op = previous().value; // "xor"
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseLogicalAnd()
{
    auto expr = parseEquality();
    while (match(TokenType::AND))
    {
        std::string op = previous().value; // "and"
        auto right = parseEquality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseEquality()
{
    auto expr = parseComparison();
    while (true)
    {
        if (match(TokenType::EQ))
        {
            std::string op = "==";
            auto right = parseComparison();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        else if (match(TokenType::NE))
        {
            std::string op = "!=";
            auto right = parseComparison();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        else
            break;
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseComparison()
{
    auto expr = parseAdditive();
    while (true)
    {
        if (match(TokenType::LT))
        {
            std::string op = "<";
            auto right = parseAdditive();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        else if (match(TokenType::GT))
        {
            std::string op = ">";
            auto right = parseAdditive();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        else if (match(TokenType::LE))
        {
            std::string op = "<=";
            auto right = parseAdditive();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        else if (match(TokenType::GE))
        {
            std::string op = ">=";
            auto right = parseAdditive();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        else
            break;
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseAdditive()
{
    auto expr = parseMultiplicative();
    while (true)
    {
        if (match(TokenType::PLUS))
        {
            std::string op = "+";
            auto right = parseMultiplicative();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        else if (match(TokenType::MINUS))
        {
            std::string op = "-";
            auto right = parseMultiplicative();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        else
            break;
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseMultiplicative()
{
    auto expr = parseUnary();
    while (true)
    {
        if (match(TokenType::STAR))
        {
            std::string op = "*";
            auto right = parseUnary();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        else if (match(TokenType::SLASH))
        {
            std::string op = "/";
            auto right = parseUnary();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        else if (match(TokenType::PERCENT))
        {
            std::string op = "%";
            auto right = parseUnary();
            expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
        }
        else
            break;
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseUnary()
{
    if (match(TokenType::NOT) || match(TokenType::MINUS) || match(TokenType::BIT_NOT))
    {
        std::string op = previous().value; // "not", "-", or "~"
        auto expr = parseUnary();
        return std::make_unique<UnaryExpr>(op, std::move(expr));
    }
    return parsePrimary();
}

std::unique_ptr<Expression> Parser::parsePrimary()
{
    // Number literal
    if (match(TokenType::NUMBER))
    {
        return std::make_unique<LiteralExpr>(previous().value, false);
    }

    // String literal
    if (match(TokenType::STRING))
    {
        std::string raw = previous().value;
        std::string kind = "normal";
        if (raw.rfind("r:", 0) == 0)
        {
            kind = "raw";
            raw = raw.substr(2);
        }
        else if (raw.rfind("$:", 0) == 0)
        {
            kind = "interp";
            raw = raw.substr(2);
        }
        else if (raw.rfind("m:", 0) == 0)
        {
            kind = "multi";
            raw = raw.substr(2);
        }
        return std::make_unique<LiteralExpr>(raw, true, kind);
    }

    // Boolean literals
    if (match(TokenType::TRUE))
        return std::make_unique<LiteralExpr>("true", false);
    if (match(TokenType::FALSE))
        return std::make_unique<LiteralExpr>("false", false);

    // Parenthesized expression
    if (match(TokenType::LPAREN))
    {
        auto expr = parseExpression();
        if (!match(TokenType::RPAREN))
            throw std::runtime_error("Expected ')' after expression");
        return expr;
    }

    // Identifier (variable or function call)
    if (match(TokenType::IDENTIFIER))
    {
        std::string name = previous().value;
        // Function call
        if (check(TokenType::LPAREN))
        {
            advance(); // consume '('
            auto call = std::make_unique<CallExpr>(name);
            if (!check(TokenType::RPAREN))
            {
                do
                {
                    call->arguments.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }
            if (!match(TokenType::RPAREN))
                throw std::runtime_error("Expected ')' after function arguments");
            return call;
        }
        else
        {
            return std::make_unique<VariableExpr>(name);
        }
    }

    throw std::runtime_error("Unexpected token in expression");
}