
#include "listbox_window.h"
#include "../platform/ncurses_colors.h"

//
ListboxWindow::ListboxWindow(const frame_t &_frame, 
                             const std::string &_id, 
                             bool _border) :
    LineBufferWindow(_frame, _id, _border)
{
    for (int i = 0; i < m_frame.nrows; i++)
        m_entries.push_back("line " + std::to_string(i));
}

//---------------------------------------------------------------------------------------
void ListboxWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    switch (_c)
    {
        case KEY_UP:        m_currentEntryIdx = prev_entry_(); refresh_next_frame_(); break;
        case KEY_DOWN:      m_currentEntryIdx = next_entry_(); refresh_next_frame_(); break;
        case KEY_BACKSPACE: deleteCharBeforeCursor(); break;
        
        case 10:
            m_selectedEntryIdx = m_currentEntryIdx;
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
    
    //
    for (int i = 0; i < m_frame.nrows; i++)
    {
        api->enableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_INACTIVE));
    
        if (i == m_currentEntryIdx)
            api->disableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_INACTIVE));
    
        api->wprint(m_apiWindowPtr, 5, i, "%s", m_entries[i].c_str());
        
    }
    
    updateCursor();

}

//---------------------------------------------------------------------------------------
std::string ListboxWindow::getSelectedEntry()
{
    if (m_selectedEntryIdx != -1)
    {
        return m_entries[m_selectedEntryIdx];
    }
    else
        return "";

}
