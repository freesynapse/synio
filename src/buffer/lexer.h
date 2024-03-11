#ifndef __LEXER_H
#define __LEXER_H

#include <stddef.h>
#include <unordered_map>
#include <string.h>
#include <stdio.h>

//
enum TokenKind
{
    TOKEN_END = 0,
    TOKEN_EOL,
    TOKEN_INVALID,
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_MSTRING,  // multi-line string (although syntactically invalid, mostly)
    TOKEN_COMMENT,
    TOKEN_MCOMMENT, // multi-line comment
    TOKEN_PREPROCESSOR, // for now, everything following # will be the same

    // structural literals
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_DQUOTE,
    TOKEN_SQUOTE,

    // arithmetic literals 
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_TIMES,
    TOKEN_DIV,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_EQUAL,

    // separation literals
    TOKEN_DOT,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,


};


//
typedef struct token_t
{
    int start = 0;
    int end = 0;
    TokenKind kind = TOKEN_END;
    
    int start_line = 0;
    int end_line = 0;   // for multi-line comments and strings

    //
    token_t() {}
    // NOTE : length will have to be book-kept outside, if needed at all
    token_t(int _start, int _start_line, int _end, int _end_line, TokenKind _kind) :
        start(_start), start_line(_start_line), end(_end), end_line(_end_line), kind(_kind)
    {}

} token_t;

//
typedef struct line_t
{
    line_t *next = NULL;
    char *content = NULL;
    size_t len = 0;
    //
    line_t() {}
    line_t(const char *_content, size_t _len) :
        len(_len)
    {
        content = new char[len + 1];
        memset(content, 0, len + 1);
        memcpy(content, _content, len);
    }
    ~line_t() { delete content; }

} line_t;

//
class Lexer
{
public:
    Lexer() {}
    ~Lexer() = default;

    void set_start_line(line_t *_line, int _line_no=0);
    token_t next_token(bool _single_line=true);

    bool EOS() { return m_EOS; }

// private:
    void declare_eos_() { printf("%s: reached end of stream.\n", __PRETTY_FUNCTION__); m_EOS = true; }
    void trim_whitespace_left_();
    __always_inline bool is_literal_(char _c) 
    { return m_literals.find(_c) != m_literals.end(); }


// private:
    line_t *m_line;
    int m_line_no = 0;
    int m_cursor = 0;
    bool m_EOS = false; // end of stream

    std::unordered_map<char, TokenKind> m_literals = {
        { '(', TOKEN_LEFT_PAREN, },
        { ')', TOKEN_RIGHT_PAREN, },
        { '{', TOKEN_LEFT_BRACE, },
        { '}', TOKEN_RIGHT_BRACE, },
        { '[', TOKEN_LEFT_BRACKET, },
        { ']', TOKEN_RIGHT_BRACKET, },
        { '\"', TOKEN_DQUOTE, },
        { '\'', TOKEN_SQUOTE, },

        { '+', TOKEN_PLUS, },
        { '-', TOKEN_MINUS, },
        { '*', TOKEN_TIMES, },
        { '/', TOKEN_DIV, },
        { '<', TOKEN_LT, },
        { '>', TOKEN_GT, },
        { '=', TOKEN_EQUAL, },

        { '.', TOKEN_DOT, },
        { ';', TOKEN_SEMICOLON, },
        { ',', TOKEN_COMMA, },

    };

};

//
extern const char *token2str(TokenKind _kind);

/*
//
enum TokenLiteralKind
{
    TOKEN_LITERAL_STRUCTURAL = 0,
    TOKEN_LITERAL_ARITHMETIC,
    TOKEN_LITERAL_SEPARATION,

};

//
typedef struct literal_t
{
    TokenKind kind;
    TokenLiteralKind literal_kind;

    literal_t() {}
    literal_t(TokenKind _kind, TokenLiteralKind _literal_kind) : 
        kind(_kind), literal_kind(_literal_kind)
    {}

} literal_t;

std::unordered_map<char, TokenKind> m_literals = {
        { '(', literal_t(TOKEN_LEFT_PAREN, TOKEN_LITERAL_STRUCTURAL) },
        { ')', literal_t(TOKEN_RIGHT_PAREN, TOKEN_LITERAL_STRUCTURAL) },
        { '{', literal_t(TOKEN_LEFT_BRACE, TOKEN_LITERAL_STRUCTURAL) },
        { '}', literal_t(TOKEN_RIGHT_BRACE, TOKEN_LITERAL_STRUCTURAL) },
        { '\"', literal_t(TOKEN_DQUOTE, TOKEN_LITERAL_STRUCTURAL) },
        { '\'', literal_t(TOKEN_SQUOTE, TOKEN_LITERAL_STRUCTURAL) },

        { '+', literal_t(TOKEN_PLUS, TOKEN_LITERAL_ARITHMETIC) },
        { '-', literal_t(TOKEN_MINUS, TOKEN_LITERAL_ARITHMETIC) },
        { '*', literal_t(TOKEN_TIMES, TOKEN_LITERAL_ARITHMETIC) },
        { '/', literal_t(TOKEN_DIV, TOKEN_LITERAL_ARITHMETIC) },
        { '<', literal_t(TOKEN_LT, TOKEN_LITERAL_ARITHMETIC) },
        { '>', literal_t(TOKEN_GT, TOKEN_LITERAL_ARITHMETIC) },
        { '=', literal_t(TOKEN_EQUAL, TOKEN_LITERAL_ARITHMETIC) },

        { '.', literal_t(TOKEN_DOT, TOKEN_LITERAL_SEPARATION) },
        { ';', literal_t(TOKEN_SEMICOLON, TOKEN_LITERAL_SEPARATION) },
        { ',', literal_t(TOKEN_COMMA, TOKEN_LITERAL_SEPARATION) },
*/



#endif // __LEXER_H
