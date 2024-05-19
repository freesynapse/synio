#ifndef __STR_UTILS_H
#define __STR_UTILS_H

#include <string>

#include "../types.h"

//
#define CHTYPE_STR_PTR CHTYPE_STR *
#define CHTYPE_STR_SIZE sizeof(CHTYPE_STR)

//
struct CHTYPE_STR
{
    CHTYPE_PTR str = NULL;
    size_t len = 0;

    void append(char *_str, size_t _len);
    void append(const char *_str);
    void append(CHTYPE_STR_PTR _str);

    CHTYPE_STR(char *_str, size_t _len);
    CHTYPE_STR(const char *_str);
    CHTYPE_STR(const std::string &_str);
    ~CHTYPE_STR() { free(str); }

    #ifdef DEBUG
    char __debug_str[1024];
    void __debug_content_to_str_()
    {
        for (size_t i = 0; i < len; i++)
            __debug_str[i] = (char)(str[i] & CHTYPE_CHAR_MASK);
        __debug_str[len] = '\0';
    }
    char *__debug_get_str()
    {
        __debug_content_to_str_();
        return __debug_str;
    }

    #endif

};

// basic manipulations, both of chtype string and chtype pointers
extern CHTYPE_STR_PTR char_to_chtype_str(char *_str, size_t _len);
extern CHTYPE_STR_PTR char_to_chtype_str(const char *_str);
extern CHTYPE_STR_PTR string_to_chtype_str(const std::string &_str);
extern CHTYPE_STR_PTR concat_chtype_str(CHTYPE_STR_PTR _s0, CHTYPE_STR_PTR _s1, char *_sep=NULL, size_t _sep_len=0);

extern CHTYPE_PTR char_to_chtype_ptr(char *_str, size_t _len);
extern CHTYPE_PTR char_to_chtype_ptr(const char *_str);
extern CHTYPE_PTR concat_chtype_ptrs(CHTYPE_PTR _s0, size_t _len0, CHTYPE_PTR _s1, size_t _len1,
                              char *_space=NULL, size_t _space_len=0);

#endif // __STR_UTILS_H
