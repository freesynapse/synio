#ifndef __AUTOCOMPLETOR_H
#define __AUTOCOMPLETOR_H

#include "prefix_tree.h"

//
enum AutocompletorE
{
    SINGLE_LINE = 0,
    MULTI_LINE,
};

//
class Autocompletor
{
public:
    Autocompletor() {}
    ~Autocompletor() = default;

private:
    prefix_node_t m_ptree;

};



#endif // __AUTOCOMPLETOR_H
