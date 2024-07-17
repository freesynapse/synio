
#include "options_window.h"

#include <assert.h>
#include <math.h>
#include <numeric>
#include <sstream>

#include "../utils/str_utils.h"

//
OptionsWindow::OptionsWindow(const frame_t &_frame, 
                             const std::string &_id, 
                             int _wnd_params,
                             WindowCallback _callback,
                             const std::string &_header,
                             const std::string &_text,
                             const std::vector<std::string> &_options) :
    LineBufferWindow(_frame, _id, _wnd_params)
{
    // for now, later perhaps > 4 options as selectable list
    m_options = _options;
    assert(_options.size() <= 4 && "no more than 4 options allowed.");

    // reformat header and text into multiline (if needed)
    // m_mlHeader = _header;
    int line_len = m_frame.ncols - 2 * Config::OPTIONS_DIALOG_TEXT_HPAD;
    if (_header.length() >= line_len)
        string_to_ml_string(_header, &m_mlHeader, line_len);
    else
        m_mlHeader.push_back(_header);

    // reformatting of the text into multiline (if needed)
    if (_text.size() >= line_len)
        string_to_ml_string(_text, &m_mlText, line_len);
    else
        m_mlText.push_back(_text);

    // resize frame based on number of rows and vertical padding
    // padding is 1 (abover header) + 1 (below header) + 1 (below text) + 1 (below options) = 4 rows
    // content is m_mlHeader.size() + m_mlText.size() + 1 (options)
    size_t content_rows = m_mlHeader.size() + m_mlText.size() + 6;
    m_frame.__debug_print("pre resize");
    if (m_frame.nrows < content_rows)
    {
        ivec2_t ssize;
        api->getRenderSize(&ssize);
        m_frame.v0.y = (ssize.y / 2) - (content_rows / 2) - 2;
        frame_t new_frame = frame_t(m_frame.v0, ivec2_t(m_frame.v1.x, m_frame.v0.y + content_rows));
        resize(new_frame);
    }
    m_frame.__debug_print("post resize");
    
    // spacing of options
    std::vector<int> opt_sizes;
    for (auto opt : m_options)
        opt_sizes.push_back(opt.size());
    size_t total_char_size = std::accumulate(opt_sizes.begin(), opt_sizes.end(), 0);

    int n = m_options.size();
    int x0 = m_frame.v0.x + Config::OPTIONS_DIALOG_OPT_HPAD;
    int x1 = m_frame.v1.x - Config::OPTIONS_DIALOG_OPT_HPAD;
    int dx = x1 - x0;
    assert(total_char_size < dx);
    int spacing = (dx - total_char_size) / std::max(1, n - 1);
    int spacing_mod = (dx - total_char_size) % std::max(1, n - 1);
    int x = x0 - m_frame.v0.x - (spacing_mod != 0 ? 1 : 0);
    //
    for (auto opt : _options)
    {
        ivec2_t pos = ivec2_t(x, m_frame.nrows - 3);
        m_optOffsets.push_back(pos);
        x += (opt.size() + spacing);
    }

    //
    m_cursor = Cursor(this);
    m_cursor.set_cpos(m_optOffsets[m_selectedOption]);
    m_callback = _callback;

    refresh_next_frame_();
    
}

//---------------------------------------------------------------------------------------
void OptionsWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    if (_ctrl_action != CtrlKeyAction::NONE)
    {
        moveCursor(0, 0);
        return;
    }
    else
    {
        switch (_c)
        {
            // cursor movement in input field
            case KEY_LEFT:  
                m_selectedOption = prev_option_();
                move_cursor_to_selected_();
                break;

            case KEY_RIGHT:
            case 9:     // <TAB>
                m_selectedOption = next_option_();
                move_cursor_to_selected_();
                break;

            case 10:    // <ENTER>
                m_callback(m_options[m_selectedOption]);
                break;

            default:
                moveCursor(0, 0);
                break;

        }
    }
}

//---------------------------------------------------------------------------------------
void OptionsWindow::redraw()
{
    if (!m_isWindowVisible)
        return;

    // header
    api->enableAttr(m_apiWindowPtr, A_BOLD);
    // api->wprint(m_apiWindowPtr, header_pos, "%s", m_mlHeader.c_str());
    static int line_len = m_frame.ncols - 2 * Config::OPTIONS_DIALOG_TEXT_HPAD;
    for (size_t i = 0; i < m_mlHeader.size(); i++)
    {
        // center on each row
        int x = (line_len / 2) - (m_mlHeader[i].length() / 2);
        api->wprint(m_apiWindowPtr, x + Config::OPTIONS_DIALOG_TEXT_HPAD, 1 + i, "%s",
                    m_mlHeader[i].c_str());
    }
    api->disableAttr(m_apiWindowPtr, A_BOLD);

    // text
    for (size_t i = 0; i < m_mlText.size(); i++)
    {
        api->wprint(m_apiWindowPtr, Config::OPTIONS_DIALOG_TEXT_HPAD, 2 + m_mlHeader.size() + i, "%s", 
                    m_mlText[i].c_str());
    }

    // options
    for (int i = 0; i < (int)m_options.size(); i++)
    {
        api->wprintc(m_apiWindowPtr, m_optOffsets[i].x-1, m_optOffsets[i].y, '<');
        if (i == m_selectedOption)
            api->enableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_SEL_TEXT));
        api->wprint(m_apiWindowPtr, m_optOffsets[i], m_options[i].c_str());
        if (i == m_selectedOption)
            api->disableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_SEL_TEXT));
        api->wprintc(m_apiWindowPtr, m_optOffsets[i].x+m_options[i].length(), m_optOffsets[i].y, '>');
    }

    updateCursor();

}


