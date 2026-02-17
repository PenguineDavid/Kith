#pragma once
#include <string>
#include <vector>

enum class TokenType
{
    FUNC,
    INT_TYPE,
    STR_TYPE,
    IF,
    ELSE,
    WHILE,
    FOR,
    FOREACH,
    TRUE,
    FALSE,
    AND,
    OR,
    XOR,
    NOT,
    RETURN,
    STOP, // <-- new
    IDENTIFIER,
    NUMBER,
    STRING,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    DOT,
    ASSIGN, // =
    EQ,     // ==
    NE,     // !=
    LT,     // <
    GT,     // >
    LE,     // <=
    GE,     // >=
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    BIT_AND, // &
    BIT_OR,  // |
    BIT_XOR, // ^
    BIT_NOT, // ~
    COMMA,   // ,
    COLON,   // :
    END_OF_FILE
};

struct Token
{
    TokenType type;
    std::string value;
};

class Lexer
{
public:
    Lexer(const std::string &src);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t position;

    char currentChar();
    void advance();
    void skipWhitespace();
    Token identifierOrKeyword();
    Token number();
    Token stringLiteral();
    Token parseStringLiteral(const std::string &kind);
};