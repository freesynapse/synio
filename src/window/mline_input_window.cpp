
#include "mline_input_window.h"


//
MLineInputWindow::MLineInputWindow(const frame_t &_frame, 
                                   const std::string &_id, 
                                   int _wnd_params) :
    FileBufferWindow(_frame, _id, _wnd_params, false, false)
{
    m_ptree = new prefix_node_t;

    m_renderFrame = m_frame;
    m_renderFrame.v0.y += 1;    // space for header
    m_renderFrame.v1.y -= 2;    // space for input field
    m_renderFrame.update_dims();

    m_formatter = BufferFormatter(&m_renderFrame);
    
    // m_cursor.set_frame(m_renderFrame); -- not present in ListboxWindow...

    m_inputLine = create_line("");

}

//---------------------------------------------------------------------------------------
MLineInputWindow::~MLineInputWindow()
{
    delete m_ptree;
    delete m_inputLine;
}

//---------------------------------------------------------------------------------------
void MLineInputWindow::pushCharToInput(char _c)
{
    // only allowed characters (for now)
    if (Config::ALLOWED_CHAR_SET.find(_c) == Config::ALLOWED_CHAR_SET.end())
        return;

    m_inputLine->insert_char(_c, m_inputLine->len);

    findCompletions();

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void MLineInputWindow::popCharFromInput()
{
    if (m_inputLine->len == 0)
        return;

    m_inputLine->delete_at(m_inputLine->len);

    findCompletions();

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void MLineInputWindow::autocompleteInput()
{
    if (m_inputLine->len == 0)
        return;
    
    std::string input = std::string(m_inputLine->__debug_str);
    prefix_node_t *stree = PrefixTree::find_subtree(m_ptree, input);
    std::string longest_prefix;
    PrefixTree::find_longest_prefix(stree, input, &longest_prefix);

    if (longest_prefix.length() == 0)
    {
        return;
    }
    else if (longest_prefix.length() != input.length())
    {
        delete m_inputLine;
        m_inputLine = create_line(longest_prefix.c_str());
        findCompletions();
        refresh_next_frame_();
    }

}

