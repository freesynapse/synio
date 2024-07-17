
#include <assert.h>
#include <string.h>

#include "str_utils.h"


// CHTYPE_STR constructors
CHTYPE_STR::CHTYPE_STR(char *_str, size_t _len)
{ 
    str = char_to_chtype_ptr(_str, _len);
    len = _len;
}

//---------------------------------------------------------------------------------------
CHTYPE_STR::CHTYPE_STR(const char *_str)
{
    str = char_to_chtype_ptr(_str);
    len = strlen(_str);
}

//---------------------------------------------------------------------------------------
CHTYPE_STR::CHTYPE_STR(const std::string &_str)
{
    str = char_to_chtype_ptr(_str.c_str());
    len = _str.size();
}

//---------------------------------------------------------------------------------------
void CHTYPE_STR::append(char *_str, size_t _len)
{
    assert(_str != NULL && _len > 0);

    size_t new_len = len + _len;
    str = (CHTYPE_PTR)realloc(str, (new_len + 1) * CHTYPE_SIZE);
    for (size_t i = 0; i < _len; i++)
        str[len + i] = _str[i];
    len = new_len;

}

//---------------------------------------------------------------------------------------
void CHTYPE_STR::append(const char *_str)
{
    append((char *)_str, strlen(_str));

}

//---------------------------------------------------------------------------------------
void CHTYPE_STR::append(CHTYPE_STR_PTR _str)
{
    // N.B.! free:s argument str

    assert(_str->str != NULL && _str->len > 0);

    size_t new_len = len + _str->len;
    str = (CHTYPE_PTR)realloc(str, (new_len + 1) * CHTYPE_SIZE);
    memcpy(str + len, _str->str, _str->len * CHTYPE_SIZE);

    free(_str);
    _str = NULL;

}

// Generic manipulations ----------------------------------------------------------------
CHTYPE_STR_PTR char_to_chtype_str(char *_str, size_t _len)
{
    CHTYPE_STR_PTR s = (CHTYPE_STR_PTR )malloc(CHTYPE_STR_SIZE);
    s->str = (CHTYPE_PTR)malloc(CHTYPE_SIZE * _len + 1);
    for (size_t i = 0; i < _len; i++)
        s->str[i] = _str[i];
    s->str[_len] = 0;
    s->len = _len;
    return s;

}

//---------------------------------------------------------------------------------------
CHTYPE_STR_PTR char_to_chtype_str(const char *_str)
{
    return char_to_chtype_str((char *)_str, strlen(_str));
}

//---------------------------------------------------------------------------------------
CHTYPE_STR_PTR string_to_chtype_str(const std::string &_str)
{
    return char_to_chtype_str(_str.c_str());
}

//---------------------------------------------------------------------------------------
CHTYPE_STR_PTR concat_chtype_str(CHTYPE_STR_PTR _s0, CHTYPE_STR_PTR _s1, char *_sep, size_t _sep_len)
{
    CHTYPE_STR_PTR s = (CHTYPE_STR_PTR )malloc(CHTYPE_STR_SIZE);
    s->str = (CHTYPE_PTR)malloc((_s0->len + _s1->len + _sep_len + 1) * CHTYPE_SIZE);
    // copy first
    memcpy(s->str, _s0, _s0->len * CHTYPE_SIZE);
    
    // copy separator
    if (_sep != NULL && _sep_len > 0)
    {
        for (size_t i = 0; i < _sep_len; i++)
            s->str[_s0->len + i] = _sep[i];
    }
    // copy second
    memcpy(s->str + _s0->len + _sep_len, _s1->str, _s1->len);

    s->len = _s0->len + _s1->len + _sep_len;
    s->str[s->len] = 0;

    free(_s0);
    free(_s1);
    _s0 = _s1 = NULL;

    return s;

}

//---------------------------------------------------------------------------------------
CHTYPE_PTR char_to_chtype_ptr(char *_str, size_t _len)
{
    CHTYPE_PTR s = (CHTYPE_PTR)malloc(CHTYPE_SIZE * _len);
    memset(s, 0, CHTYPE_SIZE * _len);
    for (size_t i = 0; i < _len; i++)
        s[i] = _str[i];

    return s;
}

// (overload) ---------------------------------------------------------------------------
CHTYPE_PTR char_to_chtype_ptr(const char *_str)
{
    return char_to_chtype_ptr((char *)_str, strlen(_str));

}

//---------------------------------------------------------------------------------------
CHTYPE_PTR concat_chtype_ptrs(CHTYPE_PTR _s0, size_t _len0, CHTYPE_PTR _s1, size_t _len1,
                              char *_sep, size_t _sep_len)
{
    CHTYPE_PTR s = (CHTYPE_PTR)malloc((_len0 + _len1 + _sep_len) * CHTYPE_SIZE);
    // copy first string
    memcpy(s, _s0, _len0 * CHTYPE_SIZE);
    // copy spacing
    if (_sep != NULL && _sep_len > 0)
    {
        for (size_t i = 0; i < _sep_len; i++)
            s[_len0 + i] = _sep[i];
    }
    // copy second string
    memcpy(s + _len0 + _sep_len, _s1, _len1 * CHTYPE_SIZE);

    free(_s0);
    free(_s1);
    _s0 = _s1 = NULL;

    return s;

}

//---------------------------------------------------------------------------------------
void string_to_ml_string(const std::string &_str, std::vector<std::string> *_out_v, 
                         size_t _line_len)
{
    // first split on ' ' into words
    std::string w;
    std::vector<std::string> words;
    std::stringstream ss(_str);
    while(std::getline(ss, w, ' '))
        words.push_back(w);

    // next add words per line so it will fit the window frame
    int chars_left = (int)_str.size();
    int curr_col = 0;
    std::string line = "";
    int curr_word = 0;
    while (chars_left > 0)
    {
        int n = words[curr_word].length() + 1;
        if (curr_col + n <= _line_len)
        {
            line += words[curr_word] + " ";
            chars_left -= n;
            curr_col += n;
        }
        else
        {
            _out_v->push_back(line);
            line = words[curr_word] + " ";
            chars_left -= n;
            curr_col = n;
        }

        curr_word++;
    }

    if (line != "")
        _out_v->push_back(line);
        
}