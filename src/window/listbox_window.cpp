
#include "listbox_window.h"

#include <algorithm>

#include "../platform/ncurses_colors.h"

//
ListboxWindow::ListboxWindow(const frame_t &_frame, 
                             const std::string &_id, 
                             bool _border,
                             const std::string &_header, 
                             const std::vector<listbox_entry_t> &_entries) :
    MLineInputWindow(_frame, _id, _border)
{
    //
    m_header = _header;

    // set cursor offset (under current dir header)
    m_cursor.set_offset(ivec2_t(1, 1));

    // scan for longest value in pairs
    size_t max_value_len = 0;
    for (size_t i = 0; i < _entries.size(); i++)
        max_value_len = std::max(max_value_len, _entries[i].value.size());

    //
    size_t max_line_len = 0;
    for (size_t i = 0; i < _entries.size(); i++)
    {
        auto &k = _entries[i].key;
        auto &v = _entries[i].value;

        //
        m_keys.push_back(k);
        m_values.push_back(v);
        
        // format buffer line and insert into prefix tree (two flies with one stone)
        char buf[256];
        memset(buf, 0, 256);
        int n = 0;
        if (v != "")
        {
            PrefixTree::insert_string(m_ptree, v);
            n += snprintf(buf, 256, "%-*s -- ", (int)max_value_len, v.c_str());
        }
        else
            PrefixTree::insert_string(m_ptree, k);

        snprintf(buf + n, 256 - n, "%s", k.c_str());
        m_lineBuffer.push_back(buf);
        max_line_len = std::max(max_line_len, m_lineBuffer.m_tail->len);

    }

    // adapt frame to max_line_len
    if (max_line_len > m_frame.ncols - 3)
    {
        frame_t new_frame = m_frame;
        new_frame.v1.x = new_frame.v0.x + max_line_len + 3;
        new_frame.update_dims();
        resize(new_frame);
    }
    
    // set up the renderer with an adjusted frame
    m_renderFrame = m_frame;
    m_renderFrame.v0.y += 1;    // space for header
    m_renderFrame.v1.y -= 2;    // space for input field
    m_renderFrame.update_dims();
    m_formatter = BufferFormatter(&m_renderFrame);

    m_currentLine = m_lineBuffer.m_head;
    m_pageFirstLine = m_lineBuffer.m_head;

}

//---------------------------------------------------------------------------------------
void ListboxWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    switch (_c)
    {
        case KEY_UP:    clear_input_(); changeSelectedRow(-1);  break;
        case KEY_DOWN:  clear_input_(); changeSelectedRow( 1);  break;
        case 8:         clear_input_(); moveCursor(0, 0);       break;

        case KEY_BACKSPACE: 
            popCharFromInput();
            moveCursor(0, 0);
            break;

        case 9:     // <TAB>
            autocompleteInput();
            moveCursor(0, 0);
            break;

        case 10:    // <ENTER>
            if (m_inputLine->len > 0)
            {
                int idx = -1;
                for (size_t i = 0; i < m_values.size(); i++)
                {
                    if (strcmp(m_values[i].c_str(), m_inputLine->__debug_str) == 0)
                    {
                        idx = i;
                        break;
                    }
                }
                if (idx >= 0 && idx < m_keys.size())
                    m_selectedEntry = m_keys[idx];
                else
                    m_selectedEntry = "";
            }
            else
            {
                if (m_highlightedEntry >= 0 && m_highlightedEntry < m_keys.size())
                    m_selectedEntry = m_keys[m_highlightedEntry];
                else
                    m_selectedEntry = "";
            }
            break;
        
        default:
            pushCharToInput((char)_c);
            break;
    }

}

//---------------------------------------------------------------------------------------
void ListboxWindow::redraw()
{
    if (!m_isWindowVisible)
        return;
    
    // print header in bold
    api->clearSpace(m_apiWindowPtr, 0, 0, m_frame.ncols);
    api->enableAttr(m_apiWindowPtr, A_BOLD);
    api->wprint(m_apiWindowPtr, 0, 0, "%s", m_header.c_str());
    api->disableAttr(m_apiWindowPtr, A_BOLD);
    
    // print entries
    m_formatter.render(m_apiWindowPtr, m_pageFirstLine, NULL, 1, 1);
    api->horizontalDivider(m_apiWindowPtr, 0, m_frame.nrows - 2, m_frame.ncols);

    // print input
    //
    int y_offset = m_frame.nrows - 1;
    int x_offset = 1;
    api->clearBufferLine(m_apiWindowPtr, y_offset, m_frame.v1.x);
    // 1st get manual input if available
    if (m_inputLine->len > 0)
    {
        api->printBufferLine(m_apiWindowPtr, x_offset, y_offset, m_inputLine);
        api->printCursorBlock(m_apiWindowPtr, x_offset + m_inputLine->len, y_offset);

        if (m_autocompleteLine.length() > 0)
        {
            api->enableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_INACTIVE));
            int cx_offset = m_inputLine->len + 3;
            api->wprint(m_apiWindowPtr, cx_offset, y_offset, "%s", m_autocompleteLine.c_str());
            api->disableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_INACTIVE));
        }
    }
    // otherwise get input line from the selected entry
    else
    {
        api->enableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_SEL_TEXT));
        {
            const char *value = m_values[m_highlightedEntry].c_str();
            api->wprint(m_apiWindowPtr, x_offset, y_offset, "%s", value);
            api->printCursorBlock(m_apiWindowPtr, x_offset + strlen(value), y_offset);
        }
        api->disableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_SEL_TEXT));
    }


    updateCursor();

}

//---------------------------------------------------------------------------------------
void ListboxWindow::moveCursor(int _dx, int _dy)
{
    // only the y coordinate is used

    int dx = _dx;
    int dy = _dy;
    m_prevLine = m_currentLine;
    
    // cursor position
    int cx = m_cursor.cx();
    int cy = m_cursor.cy();
    
    if (!_dx && !_dy)
    {
        refresh_next_frame_();
        return;
    }

    // wrap around if on the last line
    if (dy > 0 && m_currentLine->next == NULL)
        dy = -m_lineBuffer.size();
    else if (dy < 0 && m_currentLine->prev == NULL)
        dy = m_lineBuffer.size() - 1;
        
    // move the cursor (if possible)
    m_cursor.move(dx, dy);
    ivec2_t new_pos = m_cursor.cpos();

    // update pointer into line buffer
    updateCurrentLinePtr(new_pos.y - cy);

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void ListboxWindow::changeSelectedRow(int _dy)
{
    if (_dy == 0)
        return;
    
    m_highlightedEntry = (_dy < 0 ? prev_entry_() : next_entry_());
    // m_currentLine = m_lineBuffer.ptrFromIdx(m_highlightedEntry);
    moveCursor(0, _dy);
    
    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void ListboxWindow::findCompletions()
{
    std::string input = std::string(m_inputLine->__debug_str);
    prefix_node_t *stree = PrefixTree::find_subtree(m_ptree, input);

    m_autocompletions.clear();
    PrefixTree::find_completions(stree, &m_autocompletions, input);

    //
    m_autocompleteLine = "";
    int space_left = m_frame.ncols - m_inputLine->len - 1;

    // Only one match, let's jump to that file in the listing
    if (m_autocompletions.size() == 1 && m_autocompletions[0] == input)
    {
        auto it = std::find(m_values.begin(), m_values.end(), input);

        // found match, diff between current cursor pos and idx
        if (it != m_values.end())
        {
            size_t idx = it - m_values.begin();
            int dy = idx - m_bufferCursorPos.y;
            moveCursor(0, dy);
        }
        refresh_next_frame_();
        return;
    }

    // Multiple matches; fill line with as many as possible
    int n_entered = 0;
    bool done = false;
    for (size_t i = 0; i < m_autocompletions.size() && !done; i++)
    {
        std::string ac = m_autocompletions[i];

        for (char c : ac)
        {
            m_autocompleteLine += c;
            n_entered++;
            if (space_left - n_entered <= 3)
            {
                m_autocompleteLine += "...";
                done = true;
                break;
            }
        }

        m_autocompleteLine += ' ';
        n_entered++;

    }

    refresh_next_frame_();

}
