
#include "lexer.h"

#include <ctype.h>

#include "../config.h"


//
const char *token2str(TokenKind _kind)
{
    switch(_kind)
    {
        case TOKEN_EOS:             return "TOKEN_EOS";
        case TOKEN_EOL:             return "TOKEN_EOL";
        case TOKEN_INVALID:         return "TOKEN_INVALID";
        case TOKEN_IDENTIFIER:      return "TOKEN_IDENTIFIER";
        case TOKEN_KEYWORD:         return "TOKEN_KEYWORD";
        case TOKEN_NUMBER:          return "TOKEN_NUMBER";
        case TOKEN_STRING:          return "TOKEN_STRING";
        case TOKEN_MSTRING:         return "TOKEN_MSTRING";
        case TOKEN_COMMENT:         return "TOKEN_COMMENT";
        case TOKEN_MCOMMENT:        return "TOKEN_MCOMMENT";
        case TOKEN_PREPROCESSOR:    return "TOKEN_PREPROCESSOR";
        case TOKEN_LEFT_PAREN:      return "TOKEN_LEFT_PAREN";
        case TOKEN_RIGHT_PAREN:     return "TOKEN_RIGHT_PAREN";
        case TOKEN_LEFT_BRACE:      return "TOKEN_LEFT_BRACE";
        case TOKEN_RIGHT_BRACE:     return "TOKEN_RIGHT_BRACE";
        case TOKEN_LEFT_BRACKET:    return "TOKEN_LEFT_BRACKET";
        case TOKEN_RIGHT_BRACKET:   return "TOKEN_RIGHT_BRACKET";
        case TOKEN_SEMICOLON:       return "TOKEN_SEMICOLON";
        case TOKEN_COMMA:           return "TOKEN_COMMA";
        case TOKEN_DQUOTE:          return "TOKEN_DQUOTE";
        case TOKEN_SQUOTE:          return "TOKEN_SQUOTE";
        case TOKEN_DIV:             return "TOKEN_DIV";
        case TOKEN_PLUS:            return "TOKEN_PLUS";
        case TOKEN_MINUS:           return "TOKEN_MINUS";
        case TOKEN_LT:              return "TOKEN_LT";
        case TOKEN_GT:              return "TOKEN_GT";
        case TOKEN_TIMES:           return "TOKEN_TIMES";
        case TOKEN_EQUAL:           return "TOKEN_EQUAL";
        case TOKEN_AMPERSAND:       return "TOKEN_AMPERSAND";
        case TOKEN_QUESTIONMARK:    return "TOKEN_QUESTIONMARK";
        case TOKEN_EXCLAMATION:     return "TOKEN_EXCLAMATION";
        case TOKEN_LITERAL_STRUCT:  return "TOKEN_LITERAL_STRUCT";
        case TOKEN_LITERAL_OP:      return "TOKEN_LITERAL_OP";
        case TOKEN_LITERAL_DELIM:   return "TOKEN_LITERAL_DELIM";

        default: return "how did we get here?!";
    }

}
//---------------------------------------------------------------------------------------
void Lexer::colorToken(line_t *_line, token_t *_t)
{
    // color parsed tokens
    switch (_t->kind)
    {
        case TOKEN_EOS:
        case TOKEN_EOL:
        case TOKEN_INVALID:         break;
        case TOKEN_IDENTIFIER:      ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_TEXT);      break;
        case TOKEN_KEYWORD:         ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_KEYWORD);   break;
        case TOKEN_NUMBER:          ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_NUMBER);    break;
        case TOKEN_STRING:          ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_STRING);    break;
        case TOKEN_MSTRING:         ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_MSTRING);   break;
        case TOKEN_COMMENT:         ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_COMMENT);   break;
        case TOKEN_MCOMMENT:        ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_MCOMMENT);  break;
        case TOKEN_PREPROCESSOR:    ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_PREPROC);   break;

        case TOKEN_LEFT_PAREN:
        case TOKEN_RIGHT_PAREN:
        case TOKEN_LEFT_BRACE:
        case TOKEN_RIGHT_BRACE:
        case TOKEN_LEFT_BRACKET:
        case TOKEN_RIGHT_BRACKET:   ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_LITERAL_STRUCT);  break;
        case TOKEN_DQUOTE:
        case TOKEN_SQUOTE:
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_TIMES:
        case TOKEN_DIV:
        case TOKEN_LT:
        case TOKEN_GT:
        case TOKEN_EQUAL:
        case TOKEN_AMPERSAND:
        case TOKEN_QUESTIONMARK:
        case TOKEN_EXCLAMATION:     ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_LITERAL_OP);      break;
        case TOKEN_DOT:
        case TOKEN_SEMICOLON:
        case TOKEN_COLON:
        case TOKEN_COMMA:           ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_LITERAL_DELIM);   break;
        default:                    ncurses_color_substr(_line, _t->start, _t->end, SYN_COLOR_TEXT);            break;
    }

}


//---------------------------------------------------------------------------------------
void Lexer::parseBuffer(LineBuffer *_buffer)
{
    line_t *p = _buffer->m_head;
    while (p != NULL)
    {
        parseLine(p);
        p = p->next;
    }

}

//---------------------------------------------------------------------------------------
void Lexer::parseBufferFromLine(line_t *_line, LineBuffer *_buffer)
{
    line_t *p = _line;
    line_t *end_condition = _line->next;

    bool in_mcomment = m_inMComment;
    
    if (m_inMComment)
        end_condition = NULL;

    else if (m_inMString)   // since we don't know if the opening was ' or ", we need
                            // to reparse the whole buffer
    {
        end_condition = NULL;
        p = _buffer->m_head;
        m_inMString = false;
    }

    //
    while (p != end_condition)
    {
        parseLine(p);

        if (in_mcomment)
        {
            // no longer in comment
            if (!m_inMComment)
                break;
        }
        
        p = p->next;
    }

}

//---------------------------------------------------------------------------------------
void Lexer::parseLine(line_t *_line)
{
    m_cursor = 0;
    while (m_cursor < _line->len)
    {
        token_t t = nextLineToken(_line);
        colorToken(_line, &t);
    }

}

//---------------------------------------------------------------------------------------
token_t Lexer::nextLineToken(line_t *_line)
{
    m_line = _line;

    token_t token;
    trim_whitespace_left_();

    char *line = _line->__debug_str;
    size_t len = m_line->len;

    // for the debugger
    char *debug = &line[m_cursor];

    // First, check for multi-line tokens from previous line
    if (m_inMComment)
    {
        // hunt for */
        int start = m_cursor;
        while (m_cursor < len)
        {
            if (m_cursor < len - 1 && line[m_cursor] == '*' && line[m_cursor+1] == '/')
            {
                m_inMComment = false;
                m_cursor += 2;
                token = token_t(start, m_cursor, TOKEN_MCOMMENT);
                
                return token;
            }

            m_cursor++;
        }
        // not found what we're looking for, the whole line is part of the mcomment
        token = token_t(start, m_cursor, TOKEN_MCOMMENT);
        return token;

    }
    else if (m_inMString)
    {
        // hunt for m_startMStringChar
        int start = m_cursor;
        while (m_cursor < len)
        {
            if (line[m_cursor] == m_startMStringChar)
            {
                m_inMString = false;
                m_cursor++;
                token = token_t(start, m_cursor, TOKEN_MSTRING);

                return token;                
            }

            m_cursor++;
        }
        // not found what we're looking for, the whole line is part of the mstring
        token = token_t(start, len, TOKEN_MSTRING);
        return token;

    }

    // Parsing of line 
    //

    // preprocessor
    if (m_cursor < len && line[m_cursor] == '#')
    {
        token = token_t(m_cursor, len, TOKEN_PREPROCESSOR);
        m_cursor = len;
        return token;
    }

    // identifiers and keywords
    else if (m_cursor < len && isalpha(line[m_cursor]))
    {
        int start = m_cursor;
        while (isalnum(line[m_cursor]) || line[m_cursor] == '_')
            m_cursor++;
        
        // is token keyword or identifier?
        memset(m_keyword_buffer, 0, KEYWORD_MAX_SIZE);
        memcpy(m_keyword_buffer, &line[start], m_cursor - start);
        TokenKind tk = (is_keyword_(m_keyword_buffer) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER);

        return token_t(start, m_cursor, tk);
    }

    // numbers
    else if (m_cursor < len && isdigit(line[m_cursor]))
    {
        int start = m_cursor;
        int ndots = 0;
        while (isdigit(line[m_cursor]) || 
               (line[m_cursor] == '.' && m_cursor < len - 1 && isdigit(line[m_cursor+1]) && !ndots))
        {
            if (line[m_cursor] == '.') ndots++;
            m_cursor++;
        }
        
        return token_t(start, m_cursor, TOKEN_NUMBER);
    }

    // strings
    else if (m_cursor < len && (line[m_cursor] == '\"' || line[m_cursor] == '\''))
    {
        m_startMStringChar = line[m_cursor]; // ' or ", so we can match against this for closing
        int start = m_cursor;
        m_cursor++;
        
        while (m_cursor < len)
        {
            if (line[m_cursor] == m_startMStringChar)
            {
                m_cursor++;
                token = token_t(start, m_cursor, TOKEN_STRING);
                return token;
            }

            m_cursor++;

        }
        // did not find string closing char, hence multi-line string
        token = token_t(start, len, TOKEN_MSTRING);
        m_inMString = true;
        return token;

    }

    // comments
    else if (m_cursor < len && line[m_cursor] == '/')
    {
        // comments
        if (m_cursor < len - 1 && (line[m_cursor+1] == '/' || line[m_cursor+1] == '*'))
        {
            int start = m_cursor;
            m_cursor++;
            // single line comment
            if (line[m_cursor] == '/')
            {
                token = token_t(start, len, TOKEN_COMMENT);
                m_cursor = len;
                return token;

            }
            // multi-line comment
            else if (line[m_cursor] == '*')
            {
                m_cursor++;
                m_inMComment = true;    // Means that further parsing of this and subsequent
                                        // lines will be treated as mcomments until */ is
                                        // found.
                token = token_t(start, m_cursor, TOKEN_MCOMMENT);
                return token;

            }
        }
        
        // no comment, treat single '/' as a literal
        else
        {
            token = token_t(m_cursor, m_cursor + 1, m_literals[line[m_cursor]]);
            m_cursor++;
            return token;

        }

    }

    // literals
    else if (m_cursor < len && is_literal_(line[m_cursor]))
    {
        token = token_t(m_cursor, m_cursor + 1, m_literals[line[m_cursor]]);
        m_cursor++;
        return token;

    }

    token = token_t(m_cursor, m_cursor+1, TOKEN_INVALID);
    m_cursor++;
    return token;

}

//---------------------------------------------------------------------------------------
void Lexer::trim_whitespace_left_()
{
    while (m_cursor < m_line->len && isspace(m_line->__debug_str[m_cursor]))
        m_cursor++;

}


