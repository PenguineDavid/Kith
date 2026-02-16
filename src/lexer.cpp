#include "lexer.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(const std::string &src) : source(src), position(0) {}

char Lexer::currentChar()
{
    if (position >= source.length())
        return '\0';
    return source[position];
}

void Lexer::advance() { position++; }

void Lexer::skipWhitespace()
{
    while (std::isspace(currentChar()))
        advance();
}

Token Lexer::identifierOrKeyword()
{
    std::string value;
    while (std::isalnum(currentChar()) || currentChar() == '_')
    {
        value += currentChar();
        advance();
    }

    // Keywords
    if (value == "func")
        return {TokenType::FUNC, value};
    if (value == "int")
        return {TokenType::INT_TYPE, value};
    if (value == "str")
        return {TokenType::STR_TYPE, value};
    if (value == "if")
        return {TokenType::IF, value};
    if (value == "else")
        return {TokenType::ELSE, value};
    if (value == "while")
        return {TokenType::WHILE, value};
    if (value == "for")
        return {TokenType::FOR, value};
    if (value == "foreach")
        return {TokenType::FOREACH, value};
    if (value == "true")
        return {TokenType::TRUE, value};
    if (value == "false")
        return {TokenType::FALSE, value};
    if (value == "and")
        return {TokenType::AND, value};
    if (value == "or")
        return {TokenType::OR, value};
    if (value == "xor")
        return {TokenType::XOR, value};
    if (value == "not")
        return {TokenType::NOT, value};

    return {TokenType::IDENTIFIER, value};
}

Token Lexer::number()
{
    std::string value;
    while (isdigit(currentChar()))
    {
        value += currentChar();
        advance();
    }
    return {TokenType::NUMBER, value};
}

Token Lexer::parseStringLiteral(const std::string &kind)
{
    std::string content;
    std::string finalKind = kind;

    if (currentChar() == '"' && source[position + 1] == '"' && source[position + 2] == '"')
    {
        if (finalKind == "normal")
            finalKind = "m";
        advance();
        advance();
        advance(); // consume """
        while (!(currentChar() == '"' && source[position + 1] == '"' && source[position + 2] == '"') && currentChar() != '\0')
        {
            content += currentChar();
            advance();
        }
        advance();
        advance();
        advance(); // consume """
    }
    else
    {
        advance(); // consume opening "
        while (currentChar() != '"' && currentChar() != '\0')
        {
            content += currentChar();
            advance();
        }
        advance(); // consume closing "
    }

    std::string value = (finalKind == "normal") ? content : finalKind + ":" + content;
    return {TokenType::STRING, value};
}

Token Lexer::stringLiteral()
{
    return parseStringLiteral("normal");
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;
    while (currentChar() != '\0')
    {
        skipWhitespace();
        char c = currentChar();

        // Raw string: r"
        if (c == 'r' && position + 1 < source.size() && source[position + 1] == '"')
        {
            advance();
            tokens.push_back(parseStringLiteral("r"));
        }
        // Interpolated string: $"
        else if (c == '$' && position + 1 < source.size() && source[position + 1] == '"')
        {
            advance();
            tokens.push_back(parseStringLiteral("$"));
        }
        // Single-character operators
        else if (c == '(')
        {
            tokens.push_back({TokenType::LPAREN, "("});
            advance();
        }
        else if (c == ')')
        {
            tokens.push_back({TokenType::RPAREN, ")"});
            advance();
        }
        else if (c == '{')
        {
            tokens.push_back({TokenType::LBRACE, "{"});
            advance();
        }
        else if (c == '}')
        {
            tokens.push_back({TokenType::RBRACE, "}"});
            advance();
        }
        else if (c == '=')
        {
            if (position + 1 < source.size() && source[position + 1] == '=')
            {
                tokens.push_back({TokenType::EQ, "=="});
                advance();
                advance();
            }
            else
            {
                tokens.push_back({TokenType::ASSIGN, "="});
                advance();
            }
        }
        else if (c == '!')
        {
            if (position + 1 < source.size() && source[position + 1] == '=')
            {
                tokens.push_back({TokenType::NE, "!="});
                advance();
                advance();
            }
            else
            {
                // standalone '!' is not part of the language (use "not" instead)
                advance();
            }
        }
        else if (c == '<')
        {
            if (position + 1 < source.size() && source[position + 1] == '=')
            {
                tokens.push_back({TokenType::LE, "<="});
                advance();
                advance();
            }
            else
            {
                tokens.push_back({TokenType::LT, "<"});
                advance();
            }
        }
        else if (c == '>')
        {
            if (position + 1 < source.size() && source[position + 1] == '=')
            {
                tokens.push_back({TokenType::GE, ">="});
                advance();
                advance();
            }
            else
            {
                tokens.push_back({TokenType::GT, ">"});
                advance();
            }
        }
        else if (c == '+')
        {
            tokens.push_back({TokenType::PLUS, "+"});
            advance();
        }
        else if (c == '-')
        {
            tokens.push_back({TokenType::MINUS, "-"});
            advance();
        }
        else if (c == '*')
        {
            tokens.push_back({TokenType::STAR, "*"});
            advance();
        }
        else if (c == '/')
        {
            tokens.push_back({TokenType::SLASH, "/"});
            advance();
        }
        else if (c == '%')
        {
            tokens.push_back({TokenType::PERCENT, "%"});
            advance();
        }
        else if (c == '&')
        {
            tokens.push_back({TokenType::BIT_AND, "&"});
            advance();
        }
        else if (c == '|')
        {
            tokens.push_back({TokenType::BIT_OR, "|"});
            advance();
        }
        else if (c == '^')
        {
            tokens.push_back({TokenType::BIT_XOR, "^"});
            advance();
        }
        else if (c == '~')
        {
            tokens.push_back({TokenType::BIT_NOT, "~"});
            advance();
        }
        else if (c == '"')
        {
            tokens.push_back(stringLiteral());
        }
        else if (std::isalpha(c) || c == '_')
        {
            tokens.push_back(identifierOrKeyword());
        }
        else if (isdigit(c))
        {
            tokens.push_back(number());
        }
        else
        {
            advance(); // skip unknown
        }
    }
    tokens.push_back({TokenType::END_OF_FILE, ""});
    return tokens;
}