
#include "lexer.h"

#include <ctype.h>

#include "../config.h"


//
const char *token2str(TokenKind _kind)
{
    switch(_kind)
    {
        case TOKEN_END:             return "END OF STREAM";
        case TOKEN_EOL:             return "(end of line)";
        case TOKEN_INVALID:         return "INVALID TOKEN";
        case TOKEN_IDENTIFIER:      return "identifier";
        case TOKEN_KEYWORD:         return "keyword";
        case TOKEN_NUMBER:          return "number";
        case TOKEN_STRING:          return "string";
        case TOKEN_MSTRING:         return "mstring";
        case TOKEN_COMMENT:         return "comment";
        case TOKEN_MCOMMENT:        return "mcomment";
        case TOKEN_PREPROCESSOR:    return "preprocessor";
        case TOKEN_LEFT_PAREN:      return "left_paren";
        case TOKEN_RIGHT_PAREN:     return "right_paren";
        case TOKEN_LEFT_BRACE:      return "left_brace";
        case TOKEN_RIGHT_BRACE:     return "right_brace";
        case TOKEN_LEFT_BRACKET:    return "left_bracket";
        case TOKEN_RIGHT_BRACKET:   return "right_bracket";
        case TOKEN_SEMICOLON:       return "semicolon";
        case TOKEN_COMMA:           return "comma";
        case TOKEN_DQUOTE:          return "dquote";
        case TOKEN_SQUOTE:          return "squote";
        case TOKEN_DIV:             return "div";
        case TOKEN_PLUS:            return "plus";
        case TOKEN_MINUS:           return "minus";
        case TOKEN_LT:              return "lt";
        case TOKEN_GT:              return "gt";
        case TOKEN_TIMES:           return "times";
        case TOKEN_EQUAL:           return "equal";
        default: return "how did we get here?!";
    }

}

//---------------------------------------------------------------------------------------
void Lexer::set_start_line(line_t *_line, int _line_no)
{
    m_line = _line;
    m_line_no = _line_no;
    m_cursor = 0;

}

//---------------------------------------------------------------------------------------
void Lexer::SYN_COLOR_token(line_t *_line, token_t *_t)
{
    // color parsed tokens
    switch (_t->kind)
    {
        case TOKEN_END:
        case TOKEN_EOL:
        case TOKEN_INVALID:         break;
        case TOKEN_IDENTIFIER:      ncurses_SYN_COLOR_substr(_line, _t->start, _t->end, SYN_COLOR_TEXT);      break;
        case TOKEN_KEYWORD:         ncurses_SYN_COLOR_substr(_line, _t->start, _t->end, SYN_COLOR_KEYWORD);   break;
        case TOKEN_NUMBER:          ncurses_SYN_COLOR_substr(_line, _t->start, _t->end, SYN_COLOR_NUMBER);    break;
        case TOKEN_STRING:
        case TOKEN_MSTRING:         ncurses_SYN_COLOR_substr(_line, _t->start, _t->end, SYN_COLOR_STRING);    break;
        case TOKEN_COMMENT:
        case TOKEN_MCOMMENT:        ncurses_SYN_COLOR_substr(_line, _t->start, _t->end, SYN_COLOR_COMMENT);   break;
        case TOKEN_PREPROCESSOR:    ncurses_SYN_COLOR_substr(_line, _t->start, _t->end, SYN_COLOR_PREPROC);   break;

        case TOKEN_LEFT_PAREN:
        case TOKEN_RIGHT_PAREN:
        case TOKEN_LEFT_BRACE:
        case TOKEN_RIGHT_BRACE:
        case TOKEN_LEFT_BRACKET:
        case TOKEN_RIGHT_BRACKET:   ncurses_SYN_COLOR_substr(_line, _t->start, _t->end, SYN_COLOR_LITERAL_STRUCT);  break;
        case TOKEN_DQUOTE:
        case TOKEN_SQUOTE:
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_TIMES:
        case TOKEN_DIV:
        case TOKEN_LT:
        case TOKEN_GT:
        case TOKEN_EQUAL:           ncurses_SYN_COLOR_substr(_line, _t->start, _t->end, SYN_COLOR_LITERAL_OP);      break;
        case TOKEN_DOT:
        case TOKEN_SEMICOLON:
        case TOKEN_COLON:
        case TOKEN_COMMA:           ncurses_SYN_COLOR_substr(_line, _t->start, _t->end, SYN_COLOR_LITERAL_DELIM);   break;
        default:                    ncurses_SYN_COLOR_substr(_line, _t->start, _t->end, SYN_COLOR_TEXT);            break;
    }

}
//---------------------------------------------------------------------------------------
void Lexer::parse_buffer()
{
    token_t t;
    do
    {
        t = next_token(false);    // false flag to go through all lines from the start line
        if (t.start_line_ptr == t.end_line_ptr)
            SYN_COLOR_token(t.start_line_ptr, &t);
        else
        {
            // TODO : implement mcomments and mstrings
            return;
        }

    } while (!m_EOS);

}

//---------------------------------------------------------------------------------------
token_t Lexer::next_token(bool _single_line)
{
    token_t token;
    trim_whitespace_left_();

    char *line = m_line->__debug_str;
    size_t len = m_line->len;

    // TODO : remove when done, for debugging only
    char *debug = &line[m_cursor];

    // end or next line?
    if (m_cursor >= len && (m_line->next == NULL || _single_line == true))
    {
        // token = token_t(m_cursor, m_line_no, m_cursor, m_line_no, TOKEN_END);
        token = token_t(m_cursor, m_line, m_cursor, m_line, TOKEN_END);
        declare_eos_();
        return token;

    }
    else if (m_cursor >= len && m_line->next != NULL)
    {
        // token = token_t(m_cursor, m_line_no, m_cursor, m_line_no, TOKEN_EOL);
        token = token_t(m_cursor, m_line, m_cursor, m_line, TOKEN_EOL);

        m_cursor = 0;
        m_line = m_line->next;
        m_line_no++;
        
        return token;

    }
    
    
    // the actual parsing of the text
    //

    // preprocessor
    if (m_cursor < len && line[m_cursor] == '#')
    {
        // token = token_t(m_cursor, m_line_no, len - m_cursor, m_line_no, TOKEN_PREPROCESSOR);
        token = token_t(m_cursor, m_line, len, m_line, TOKEN_PREPROCESSOR);
        m_cursor = len;
        return token;
    }

    // identifier
    else if (m_cursor < len && isalpha(line[m_cursor]))
    {
        int start = m_cursor;
        while (isalnum(line[m_cursor]) || line[m_cursor] == '_')
            m_cursor++;
        
        // is token keyword or identifier?
        memset(m_keyword_buffer, 0, KEYWORD_MAX_SIZE);
        memcpy(m_keyword_buffer, &line[start], m_cursor - start);
        TokenKind tk = (is_keyword_(m_keyword_buffer) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER);

        // return token_t(start, m_line_no, m_cursor, m_line_no, TOKEN_IDENTIFIER);
        return token_t(start, m_line, m_cursor, m_line, tk);
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
        
        // return token_t(start, m_line_no, m_cursor, m_line_no, TOKEN_NUMBER);
        return token_t(start, m_line, m_cursor, m_line, TOKEN_NUMBER);
    }

    // strings
    else if (m_cursor < len && (line[m_cursor] == '\"' || line[m_cursor] == '\''))
    {
        char c = line[m_cursor]; // ' or ", so we can match against this for closing
        int start = m_cursor;
        int start_line = m_line_no;
        int curr_line = start_line;
        line_t *start_line_ptr = m_line;
        m_cursor++;
        
        // everything is a string until hitting the next '"'
        while (1)
        {
            if (m_cursor < m_line->len && m_line->content[m_cursor] == c)
            {
                m_cursor++;
                TokenKind tk = (m_line == start_line_ptr ? TOKEN_STRING : TOKEN_MSTRING);
                // token = token_t(start, start_line, m_cursor, m_line_no, TOKEN_MSTRING);
                return token_t(start, start_line_ptr, m_cursor, m_line, tk);
            }
            m_cursor++;

            // goto next line
            if (m_cursor >= len)
            {
                line_t *last_line = m_line;
                m_line = m_line->next;
                
                // the rest of the stream (!) was a comment
                if (!m_line)
                {
                    declare_eos_();
                    return token_t(start, start_line_ptr, m_cursor, last_line, TOKEN_MSTRING);
                }
                
                m_line_no++;
                m_cursor = 0;
                len = m_line->len;

                trim_whitespace_left_();

            }
        }
                
    }

    // comments
    else if (m_cursor < len && line[m_cursor] == '/')
    {
        // comments
        if (m_cursor < len - 1 && (line[m_cursor+1] == '/' || line[m_cursor+1] == '*'))
        {
            int start = m_cursor;
            int start_line = m_line_no;
            line_t *start_line_ptr = m_line;
            m_cursor++;
            // single line comment
            if (line[m_cursor] == '/')
            {
                m_cursor = len;
                return token_t(start, start_line_ptr, len, start_line_ptr, TOKEN_COMMENT);

            }
            // multi-line comment
            else if (line[m_cursor] == '*')
            {
                m_cursor++;
                // everything is a comment until hitting '*\/'
                while (1)
                {
                    if (m_cursor < m_line->len - 1 && m_line->content[m_cursor] == '*' && m_line->content[m_cursor+1] == '/')
                    {
                        m_cursor += 2;
                        return token_t(start, start_line_ptr, m_cursor, m_line, TOKEN_MCOMMENT);
                    }
                    m_cursor++;

                    // goto next line
                    if (m_cursor >= len)
                    {
                        line_t *last_line = m_line;
                        m_line = m_line->next;
                        
                        // the rest of the stream (!) was a comment
                        if (!m_line)
                        {
                            declare_eos_();
                            return token_t(start, start_line_ptr, m_cursor, last_line, TOKEN_MCOMMENT);
                        }
                        
                        m_line_no++;
                        m_cursor = 0;
                        len = m_line->len;

                        trim_whitespace_left_();

                    }
                }
            }
        }
        
        // no comment, treat single '/' as a literal
        else
        {
            token = token_t(m_cursor, m_line, m_cursor + 1, m_line, m_literals[line[m_cursor]]);
            m_cursor++;
            return token;

        }

    }

    // literals
    else if (m_cursor < len && is_literal_(line[m_cursor]))
    {
        // token = token_t(m_cursor, m_line_no, m_cursor + 1, m_line_no, m_literals[line[m_cursor]]);
        token = token_t(m_cursor, m_line, m_cursor + 1, m_line, m_literals[line[m_cursor]]);
        m_cursor++;
        return token;

    }

    // token = token_t(m_cursor, m_line_no, m_cursor+1, m_line_no, TOKEN_INVALID);
    token = token_t(m_cursor, m_line, m_cursor+1, m_line, TOKEN_INVALID);
    m_cursor++;
    return token;

}

//---------------------------------------------------------------------------------------
void Lexer::trim_whitespace_left_()
{
    while (m_cursor < m_line->len && isspace(m_line->content[m_cursor]))
    {
        // if (m_line->content[m_cursor] == '\t')
            // m_cursor += Config::TAB_SIZE;
        // else
            m_cursor++;
    }

}


