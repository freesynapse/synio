
#include "prefix_tree.h"

#include "log.h"
#include "../config.h"


// static decls
std::unordered_map<char, size_t> PrefixTree::s_char2idxMap;
std::unordered_map<size_t, char> PrefixTree::s_idx2charMap;

//---------------------------------------------------------------------------------------
void PrefixTree::initialize()
{
    for (size_t i = 0; i < Config::ALLOWED_CHARS_SZ; i++)
    {
        s_char2idxMap.insert({ Config::ALLOWED_CHARS[i], i });
        s_idx2charMap.insert({ i, Config::ALLOWED_CHARS[i] });
    }

}

//---------------------------------------------------------------------------------------
void PrefixTree::insert_string(prefix_node_t *_root, const std::string &_str)
{
    prefix_node_t *current_node = _root;
    for (char c : _str)
    {
        size_t idx = s_char2idxMap[c];
        if (current_node->children[idx] == nullptr)
        {
            prefix_node_t *new_node = new prefix_node_t();
            current_node->children[idx] = new_node;
        }
        current_node = current_node->children[idx];
    }
    current_node->is_leaf = true;
}

//---------------------------------------------------------------------------------------
prefix_node_t *PrefixTree::find_subtree(prefix_node_t *_root, 
                                        const std::string &_search_str)
{
    if (_root == nullptr)
        return nullptr;

    prefix_node_t *current_node = _root;
    for (char c : _search_str)
    {
        size_t idx = s_char2idxMap[c];
        if (current_node->children[idx] != nullptr)
            current_node = current_node->children[idx];
        else
            return nullptr;
    }
    return current_node;
}

//---------------------------------------------------------------------------------------
void PrefixTree::find_completions(prefix_node_t *_node, 
                                  std::vector<std::string> *_words, 
                                  std::string _search_str)
{
    if (_node == nullptr)
        return;

    if (_node->is_leaf)
        _words->push_back(_search_str);

    for (size_t i = 0; i < Config::ALLOWED_CHARS_SZ; i++)
    {
        find_completions(_node->children[i], _words, _search_str + s_idx2charMap[i]);

    }

}

//---------------------------------------------------------------------------------------
void PrefixTree::find_longest_prefix(prefix_node_t *_node,
                                     std::string _search_str,
                                     std::string *_found_prefix)
{
    if (_node == nullptr)
        return;
    
    // is there a branching
    int n = 0;
    size_t single_path_idx;
    for (size_t i = 0; i < Config::ALLOWED_CHARS_SZ; i++)
    {
        if (_node->children[i] != nullptr)
        {
            n++;
            single_path_idx = i;
        }
    }

    if (n == 1)
    {
        find_longest_prefix(_node->children[single_path_idx],
                            _search_str + s_idx2charMap[single_path_idx],
                            _found_prefix);
    }
    else // n = 0 or > 1
    {
        *_found_prefix = _search_str;
        return;
    }

}

//---------------------------------------------------------------------------------------
void PrefixTree::print_tree(prefix_node_t *_node, std::string _word)
{
    if (_node == nullptr)
        return;

    if (_node->is_leaf)
        LOG_INFO("%s", _word.c_str());

    for (size_t i = 0; i < Config::ALLOWED_CHARS_SZ; i++)
        print_tree(_node->children[i], _word + s_idx2charMap[i]);

}

