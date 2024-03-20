#ifndef __LEXER_H
#define __LEXER_H

#include <stddef.h>
#include <unordered_map>
#include <unordered_set>
#include <string.h>

#include "../types.h"
#include "../platform/ncurses_colors.h"

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
    TOKEN_COLON,
    TOKEN_COMMA,

};


//
typedef struct token_t
{
    int start = 0;
    int end = 0;
    TokenKind kind = TOKEN_END;
    
    line_t *start_line_ptr;
    line_t *end_line_ptr;   // for multi-line comments and strings

    //
    token_t() {}
    // NOTE : length will have to be book-kept outside, if needed at all
    // TODO : Don't need line numbers for anything, right? Remove.
    token_t(int _start, line_t *_start_line_ptr, int _end, line_t *_end_line_ptr, TokenKind _kind) :
        start(_start), start_line_ptr(_start_line_ptr), end(_end), end_line_ptr(_end_line_ptr), kind(_kind)
    {}

} token_t;

//
class Lexer
{
public:
    Lexer() {}
    ~Lexer() = default;

    void set_start_line(line_t *_line, int _line_no=0);
    void color_token(line_t *_line, token_t *_t);
    void parse_buffer();
    void parse_line(line_t *_line);
    token_t next_token(bool _single_line=true);

    bool EOS() { return m_EOS; }

// private:
    __always_inline void declare_eos_() { m_EOS = true; }
    __always_inline bool is_literal_(char _c) { return m_literals.find(_c) != m_literals.end(); }
    void trim_whitespace_left_();
    __always_inline bool is_keyword_(const char *_token) { return (m_keywords.find((char *)_token) != m_keywords.end()); }


// private:
    line_t *m_line;
    int m_line_no = 0;
    int m_cursor = 0;
    bool m_EOS = false; // end of stream

    // keywords for c/c++
    // https://en.cppreference.com/w/cpp/keyword
    #define KEYWORD_MAX_SIZE 128
    char m_keyword_buffer[KEYWORD_MAX_SIZE];
    struct unordered_cmp    { bool operator()(const char *_a, const char *_b) const noexcept { return strcmp(_a, _b) == 0; } };
    struct unordered_deref  { size_t operator()(const char *_s) const noexcept { return std::hash<std::string>()(_s); } };
    std::unordered_set<const char *, unordered_deref, unordered_cmp> m_keywords = {
        "alignas", "alignof", "and", "and_eq", "asm", "atomic_cancel", "atomic_commit", 
        "atomic_noexcept", "auto", "bitand", "bitor", "bool", "break", "case", "catch", 
        "char", "char8_t", "char16_t", "char32_t", "class", "compl", "concept", "const", 
        "consteval", "constexpr", "constinit", "const_cast", "continue", "co_await", 
        "co_return", "co_yield", "decltype", "default", "delete", "do", "double", 
        "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false", "float", 
        "for", "friend", "goto", "if", "inline", "int", "long", "mutable", "namespace", 
        "new", "noexcept", "not", "not_eq", "nullptr", "operator", "or", "or_eq", 
        "private", "protected", "public", "reflexpr", "register", "reinterpret_cast", 
        "requires", "return", "short", "signed", "sizeof", "static", "static_assert", 
        "static_cast", "struct", "switch", "synchronized", "template", "this", 
        "thread_local", "throw", "true", "try", "typedef", "typeid", "typename", "union", 
        "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "xor", 
        "xor_eq", "NULL",
    };

    // literals
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
        { ':', TOKEN_COLON, },
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
