
#include "listbox_window.h"
#include "../platform/ncurses_colors.h"

//
ListboxWindow::ListboxWindow(const frame_t &_frame, 
                             const std::string &_id, 
                             bool _border,
                             const std::string &_header, 
                             std::vector<std::string> _entries) :
    FileBufferWindow(_frame, _id, _border, false, false)
{
    //
    m_header = _header;

    // set cursor offset (under current dir header)
    m_cursor.set_offset(ivec2_t(0, 1));

    int i = 0;
    for (auto &it : _entries)
    {
        LOG_RAW("entry %d : %s", i, it.c_str());
        m_lineBuffer.push_back(it.c_str());
        i++;
    }
    

    // set up the renderer with an adjusted frame
    m_renderFrame = m_frame;
    m_renderFrame.v0.y += 1;    // space for header
    m_renderFrame.v1.y -= 2;    // space for input field
    m_renderFrame.update_dims();
    m_formatter = BufferFormatter(&m_renderFrame);

    m_cursor.set_frame(m_renderFrame);

    m_currentLine = m_lineBuffer.m_head;
    m_pageFirstLine = m_lineBuffer.m_head;

}

//---------------------------------------------------------------------------------------
void ListboxWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    int a;
    switch (_c)
    {
        case KEY_UP:        moveCursor(0, 0); a = prev_entry_(); /*refresh_next_frame_();*/ break;
        case KEY_DOWN:      moveCursor(0, 0); a = next_entry_(); /*refresh_next_frame_();*/ break;
        case KEY_BACKSPACE: deleteCharBeforeCursor(); break;
        
        case 10:
            m_selectedEntryIdx = m_highlightedEntry;
            break;
        
        default:
            insertCharAtCursor((char)_c);
            break;
    }

}

//---------------------------------------------------------------------------------------
void ListboxWindow::redraw()
{
    if (!m_isWindowVisible)
        return;
    
    // print current directory
    api->clearSpace(m_apiWindowPtr, 0, 0, m_frame.ncols);
    api->enableAttr(m_apiWindowPtr, A_BOLD);
    api->wprint(m_apiWindowPtr, 0, 0, "%s", m_header.c_str());
    api->disableAttr(m_apiWindowPtr, A_BOLD);
    
    // print entries
    m_formatter.render(m_apiWindowPtr, m_pageFirstLine, NULL, 0, 1);
    api->horizontalDivider(m_apiWindowPtr, 0, m_frame.nrows - 2, m_frame.ncols);

    updateCursor();

}

//---------------------------------------------------------------------------------------
std::string ListboxWindow::getSelectedEntry()
{
    if (m_selectedEntryIdx != -1)
    {
        return std::string(m_lineBuffer.ptrFromIdx(m_selectedEntryIdx)->__debug_str);
    }
    else
        return "";

}
