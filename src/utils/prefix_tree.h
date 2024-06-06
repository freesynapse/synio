#ifndef __PREFIX_TREE_H
#define __PREFIX_TREE_H

#include <unordered_map>
#include <vector>
#include <string>

#include "../config.h"

//
struct prefix_node_t
{
    prefix_node_t *children[Config::ALLOWED_CHARS_SZ];
    bool is_leaf = false;

    prefix_node_t()
    {
        for (size_t i = 0; i < Config::ALLOWED_CHARS_SZ; i++)
            children[i] = nullptr;
    }

    ~prefix_node_t()
    {
        for (size_t i = 0; i < Config::ALLOWED_CHARS_SZ; i++)
        {
            if (children[i] != nullptr)
                delete children[i];
        }
    }

};

class PrefixTree
{
public:
    // initialize the maps
    static void initialize();

    //
    static void insert_string(prefix_node_t *_root, const std::string &_str);
    static prefix_node_t *find_subtree(prefix_node_t *_root, const std::string &_search_str);
    static void find_completions(prefix_node_t *_node, std::vector<std::string> *_words, std::string _search_str="");
    static void find_longest_prefix(prefix_node_t *_node, std::string _search_str, std::string *_found_prefix);
    static void print_tree(prefix_node_t *_node, std::string _word="");

private:
    static std::unordered_map<char, size_t> s_char2idxMap;
    static std::unordered_map<size_t, char> s_idx2charMap;


};

#endif // __PREFIX_TREE_H
