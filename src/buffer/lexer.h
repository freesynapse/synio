#ifndef __LEXER_H
#define __LEXER_H

#include <stddef.h>
#include <unordered_map>
#include <unordered_set>
#include <string.h>

#include "line_buffer.h"
#include "../types.h"
#include "../platform/ncurses_colors.h"


//
enum TokenKind
{
    TOKEN_INVALID = -1,
    TOKEN_EOS = 0,
    TOKEN_EOL,
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
    TOKEN_AMPERSAND,
    TOKEN_QUESTIONMARK,
    TOKEN_EXCLAMATION,

    // separation literals
    TOKEN_DOT,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_COMMA,

    //
    TOKEN_LITERAL_STRUCT,
    TOKEN_LITERAL_OP,
    TOKEN_LITERAL_DELIM,

};

//
typedef struct token_t
{
    int start = 0;
    int end = 0;
    TokenKind kind = TOKEN_EOS;
    
    //
    token_t() {}
    token_t(int _start, int _end, TokenKind _kind) :
        start(_start), end(_end), kind(_kind)
    {}

} token_t;

//
class Lexer
{
public:
    Lexer() {}
    ~Lexer() = default;

    // void set_start_line(line_t *_line, int _line_no);
    // void parse_buffer();
    // void parse_line(line_t *_line);
    // token_t next_token(bool _single_line=true);

    void parseBuffer(LineBuffer *_buffer);
    void parseBufferFromLine(line_t *_line, LineBuffer *_buffer);
    void parseLine(line_t *_line);
    token_t nextLineToken(line_t *_line);

    void colorToken(line_t *_line, token_t *_t);
    __always_inline TokenKind tokenFromColor(int16_t _color)
    {
        if (m_colorTokenMapping.find(_color) != m_colorTokenMapping.end())
            return m_colorTokenMapping[_color];
        return TOKEN_INVALID;
    }


    void setInMComment(bool _b) { m_inMComment = _b; }
    void setInMString(bool _b) { m_inMString = _b; }
    //
    void __debug_print_parsing(token_t _t, int _fill=64);


// private:
    void trim_whitespace_left_();
    __always_inline bool is_literal_(char _c) { return m_literals.find(_c) != m_literals.end(); }
    __always_inline bool is_keyword_(const char *_s) { return m_keywords.find(_s) != m_keywords.end(); }

// private:
    line_t *m_line; // internal reference to current line ptr
    int m_cursor = 0;

    bool m_inMComment = false;
    bool m_inMString = false;
    char m_startMStringChar;

    // keywords for c/c++
    // https://en.cppreference.com/w/cpp/keyword
    #define KEYWORD_MAX_SIZE 128
    char m_keyword_buffer[KEYWORD_MAX_SIZE];
    // custom compare and hash functions needed to prevent a const char * unordered set 
    // to compare pointers and instead use pointer contents (ie the strings).
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
        { '(',  TOKEN_LEFT_PAREN    },
        { ')',  TOKEN_RIGHT_PAREN   },
        { '{',  TOKEN_LEFT_BRACE    },
        { '}',  TOKEN_RIGHT_BRACE   },
        { '[',  TOKEN_LEFT_BRACKET  },
        { ']',  TOKEN_RIGHT_BRACKET },
        { '\"', TOKEN_DQUOTE        },
        { '\'', TOKEN_SQUOTE        },

        { '+',  TOKEN_PLUS          },
        { '-',  TOKEN_MINUS         },
        { '*',  TOKEN_TIMES         },
        { '/',  TOKEN_DIV           },
        { '<',  TOKEN_LT            },
        { '>',  TOKEN_GT            },
        { '=',  TOKEN_EQUAL         },
        { '&',  TOKEN_AMPERSAND     },
        { '?',  TOKEN_QUESTIONMARK  },
        { '!',  TOKEN_EXCLAMATION   },

        { '.',  TOKEN_DOT           },
        { ';',  TOKEN_SEMICOLON     },
        { ':',  TOKEN_COLON         },
        { ',',  TOKEN_COMMA         },

    };

    // mappings between tokens and their colors
    std::unordered_map<int16_t, TokenKind> m_colorTokenMapping = {
        { SYN_COLOR_TEXT,           TOKEN_IDENTIFIER        },
        { SYN_COLOR_KEYWORD,        TOKEN_KEYWORD           },
        { SYN_COLOR_STRING,         TOKEN_STRING            },
        { SYN_COLOR_MSTRING,        TOKEN_MSTRING           },
        { SYN_COLOR_NUMBER,         TOKEN_NUMBER            },
        { SYN_COLOR_LITERAL_STRUCT, TOKEN_LITERAL_STRUCT    },
        { SYN_COLOR_LITERAL_OP,     TOKEN_LITERAL_OP        },
        { SYN_COLOR_LITERAL_DELIM,  TOKEN_LITERAL_DELIM     },
        { SYN_COLOR_COMMENT,        TOKEN_COMMENT           },
        { SYN_COLOR_MCOMMENT,       TOKEN_MCOMMENT          },
        { SYN_COLOR_PREPROC,        TOKEN_PREPROCESSOR      },
    };

};

//
extern const char *token2str(TokenKind _kind);

//
extern const char *token2str(TokenKind _kind);


#endif // __LEXER_H
