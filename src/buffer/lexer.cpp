
#include "lexer.h"

#include <ctype.h>
#include <stdio.h>


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

//
void Lexer::set_start_line(line_t *_line, int _line_no)
{
    m_line = _line;
    m_line_no = _line_no;
    m_cursor = 0;

}

//
token_t Lexer::next_token(bool _single_line)
{
    token_t token;
    trim_whitespace_left_();

    char *line = m_line->content;
    size_t len = m_line->len;

    // for the debugger
    char *debug = &line[m_cursor];

    // end or next line?
    if (m_cursor >= len && (m_line->next == NULL || _single_line == true))
    {
        token = token_t(m_cursor, m_line_no, m_cursor, m_line_no, TOKEN_END);
        declare_eos_();
        return token;

    }
    else if (m_cursor >= len && m_line->next != NULL)
    {
        token = token_t(m_cursor, m_line_no, m_cursor, m_line_no, TOKEN_EOL);
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
        token = token_t(m_cursor, m_line_no, len - m_cursor, m_line_no, TOKEN_PREPROCESSOR);
        m_cursor = len;
        return token;
    }

    // symbols
    else if (m_cursor < len && isalpha(line[m_cursor]))
    {
        int start = m_cursor;
        while (isalnum(line[m_cursor]) || line[m_cursor] == '_')
            m_cursor++;
        
        return token_t(start, m_line_no, m_cursor, m_line_no, TOKEN_IDENTIFIER);
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
        
        return token_t(start, m_line_no, m_cursor, m_line_no, TOKEN_NUMBER);
    }

    // strings
    else if (m_cursor < len && (line[m_cursor] == '\"' || line[m_cursor] == '\''))
    {
        char c = line[m_cursor]; // ' or ", so we can match against this for closing
        // if (line[m_cursor] == '\"')
        // {
            int start = m_cursor;
            int start_line = m_line_no;
            int curr_line = start_line;
            m_cursor++;
            
            // everything is a string until hitting the next '"'
            while (1)
            {
                if (m_cursor < m_line->len && m_line->content[m_cursor] == c)
                {
                    m_cursor++;
                    token = token_t(start, start_line, m_cursor, m_line_no, TOKEN_MSTRING);
                    return token;
                }
                m_cursor++;

                // goto next line
                if (m_cursor >= len)
                {
                    m_line = m_line->next;
                    
                    // the rest of the stream (!) was a comment
                    if (!m_line)
                    {
                        declare_eos_();
                        return token_t(start, start_line, m_cursor, m_line_no, TOKEN_MSTRING);
                    }
                    
                    m_line_no++;
                    m_cursor = 0;
                    len = m_line->len;

                    trim_whitespace_left_();

                }
            }
                
            /*
            while (m_cursor < len && line[m_cursor] != '\"')
                m_cursor++;
            m_cursor++; // advance past the 2nd "   (not safe)
            
            token = token_t(start, start_line, m_cursor, curr_line, TOKEN_STRING);
            */

        // }

        // single quoted chars
        // else if (line[m_cursor] == '\'')
        // {
        //     int start = m_cursor;
        //     int start_line = m_line_no;
        //     int curr_line = start_line;
        //     m_cursor++; // advance past the 1st '
        //     while (m_cursor < len && line[m_cursor] != '\'')
        //         m_cursor++;
        //     m_cursor++; // advance past the 2nd '   (not safe)
            
        //     token = token_t(start, start_line, m_cursor, curr_line, TOKEN_STRING);
        
        // }

        // return token;

    }

    // comments
    else if (m_cursor < len && line[m_cursor] == '/')
    {
        // comments
        if (m_cursor < len - 1 && (line[m_cursor+1] == '/' || line[m_cursor+1] == '*'))
        {
            int start = m_cursor;
            int start_line = m_line_no;
            m_cursor++;
            // single line comment
            if (line[m_cursor] == '/')
            {
                token = token_t(start, m_line_no, len, m_line_no, TOKEN_COMMENT);
                m_cursor = len;
                return token;

            }
            // multi-line comment
            else if (line[m_cursor] == '*')
            {
                m_cursor++;
                // everything is a comment until hitting '*/'
                while (1)
                {
                    if (m_cursor < m_line->len - 1 && m_line->content[m_cursor] == '*' && m_line->content[m_cursor+1] == '/')
                    {
                        m_cursor += 2;
                        token = token_t(start, start_line, m_cursor, m_line_no, TOKEN_MCOMMENT);
                        return token;
                    }
                    m_cursor++;

                    // goto next line
                    if (m_cursor >= len)
                    {
                        m_line = m_line->next;
                        
                        // the rest of the stream (!) was a comment
                        if (!m_line)
                        {
                            declare_eos_();
                            return token_t(start, start_line, m_cursor, m_line_no, TOKEN_MCOMMENT);
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
            token = token_t(m_cursor, m_line_no, m_cursor + 1, m_line_no, m_literals[line[m_cursor]]);
            m_cursor++;
            return token;

        }

    }

    // literals
    else if (m_cursor < len && is_literal_(line[m_cursor]))
    {
        token = token_t(m_cursor, m_line_no, m_cursor + 1, m_line_no, m_literals[line[m_cursor]]);
        m_cursor++;
        return token;

    }

    token = token_t(m_cursor, m_line_no, m_cursor+1, m_line_no, TOKEN_INVALID);
    m_cursor++;
    return token;

}

//
void Lexer::trim_whitespace_left_()
{
    while (m_cursor < m_line->len && isspace(m_line->content[m_cursor]))
        m_cursor++;

}


