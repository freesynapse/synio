
#include <assert.h>
#include <cmath>

#include "file_buffer_window.h"
#include "../platform/ncurses_colors.h"
#include "../utils/timer.h"


//
FileBufferWindow::FileBufferWindow(const frame_t &_frame,
                                   const std::string &_id_filename,
                                   int _wnd_params,
                                   bool _use_line_numbers,
                                   bool _auto_open_file) :
    BufferWindowBase(_frame, _id_filename, _wnd_params)
{
    m_formatter = BufferFormatter(&m_frame);
    m_cursor = Cursor(this);
    
    // Create a line numbers subwindow. In case of reading buffer from file, the width 
    // of the LineNumbers are deduced at runtime
    m_useLineNumbers = _use_line_numbers;
    if (_use_line_numbers)
    {
        frame_t line_numbers_rect(ivec2_t(0, m_frame.v0.y), 
                                ivec2_t(m_frame.v0.x, m_frame.v1.y));
        m_lineNumbers = new LineNumbers(line_numbers_rect, _id_filename+"_line_numbers", _wnd_params);
        m_lineNumbers->setBuffer(this);

        m_lineNumbers->setWidth(Config::LINE_NUMBERS_MIN_WIDTH);
        resize(m_frame);

        if (!Config::SHOW_LINE_NUMBERS)
            m_lineNumbers->setVisibility(false);
    }

    // selection instance
    m_selection = new Selection();

    // undo buffer
    m_undoBuffer = UndoBuffer(this);

    // read filename
    if (_auto_open_file)
        readFileToBuffer(_id_filename);

}

//---------------------------------------------------------------------------------------
FileBufferWindow::~FileBufferWindow()
{
    delete m_lineNumbers;
    delete m_lexer;
}

//---------------------------------------------------------------------------------------
#ifdef DEBUG
void FileBufferWindow::__debug_fnc()
{
    // m_undoBuffer.__debug_print_stack();
    deleteToPrevColDelim();

}
#endif
//---------------------------------------------------------------------------------------
void FileBufferWindow::handleInput(int _c, CtrlKeyAction _ctrl_action)
{
    if (_ctrl_action != CtrlKeyAction::NONE)
    {
        switch (_ctrl_action)
        {
            case CtrlKeyAction::CTRL_LEFT:  deselect_(BACKWARD); findColDelim(BACKWARD);            break;
            case CtrlKeyAction::CTRL_RIGHT: deselect_(FORWARD);  findColDelim(FORWARD);             break;
            case CtrlKeyAction::CTRL_UP:    deselect_(BACKWARD); moveCursorToRowDelim(BACKWARD);    break;
            case CtrlKeyAction::CTRL_DOWN:  deselect_(FORWARD);  moveCursorToRowDelim(FORWARD);     break;
            case CtrlKeyAction::CTRL_HOME:  deselect_(BACKWARD); moveFileBegin();                   break;
            case CtrlKeyAction::CTRL_END:   deselect_(FORWARD);  moveFileEnd();                     break;
            case CtrlKeyAction::CTRL_DEL:   deleteToNextColDelim();                                 break;
            
            // selections
            case CtrlKeyAction::SHIFT_UP:           select_(); moveCursor(0, -1);                   break;
            case CtrlKeyAction::SHIFT_DOWN:         select_(); moveCursor(0,  1);                   break;
            case CtrlKeyAction::SHIFT_CTRL_LEFT:    select_(); findColDelim(BACKWARD);              break;
            case CtrlKeyAction::SHIFT_CTRL_RIGHT:   select_(); findColDelim(FORWARD);               break;
            case CtrlKeyAction::SHIFT_CTRL_UP:      select_(); moveCursorToRowDelim(BACKWARD);      break;
            case CtrlKeyAction::SHIFT_CTRL_DOWN:    select_(); moveCursorToRowDelim(FORWARD);       break;
            case CtrlKeyAction::SHIFT_CTRL_HOME:    select_(); moveFileBegin();                     break;
            case CtrlKeyAction::SHIFT_CTRL_END:     select_(); moveFileEnd();                       break;

            default: break;
            // default: LOG_INFO("ctrl keycode %d : %s", _c, ctrlActionStr(_ctrl_action)); break;

        }
    }
    else
    {
        switch (_c)
        {
            // cursor movement
            case KEY_DOWN:      deselect_(FORWARD);  moveCursor( 0,  1);            break;
            case KEY_UP:        deselect_(BACKWARD); moveCursor( 0, -1);            break;
            case KEY_LEFT:      deselect_(BACKWARD); moveCursor(-1,  0);            break;
            case KEY_RIGHT:     deselect_(FORWARD);  moveCursor( 1,  0);            break;
            case KEY_PPAGE:     deselect_(BACKWARD); movePageUp();                  break;
            case KEY_NPAGE:     deselect_(FORWARD);  movePageDown();                break;
            case KEY_HOME:      deselect_(BACKWARD); moveCursorToLineBegin();       break;
            case KEY_END:       deselect_(FORWARD);  moveCursorToLineEnd();         break;
            case KEY_DC:        deleteCharAtCursor();                               break;
            case KEY_BACKSPACE: deleteCharBeforeCursor();                           break;
            case KEY_ENTER:
            case 10:            insertNewLine();                                    break;

            // for some reason, ctrl+backspace maps to 8
            case 8:             deleteToPrevColDelim();                             break;

            // selections
            case KEY_SLEFT:     select_(); moveCursor(-1, 0);           break;
            case KEY_SRIGHT:    select_(); moveCursor( 1, 0);           break;
            case KEY_SHOME:     select_(); moveCursorToLineBegin();     break;
            case KEY_SEND:      select_(); moveCursorToLineEnd();       break;
            case KEY_SPREVIOUS: select_(); movePageUp();                break;
            case KEY_SNEXT:     select_(); movePageDown();              break;

            // cut-n-paste (and copy)
            case CTRL('c'):
                copySelection();
                deselect_(FORWARD);
                refresh_next_frame_();
                break;

            case CTRL('v'):
                paste();
                deselect_(FORWARD);
                refresh_next_frame_();
                break;

            case CTRL('w'):
                cutSelection();
                refresh_next_frame_();
                break;

            //
            case CTRL('z'):
                m_undoBuffer.undo();
                break;

            #ifdef DEBUG
            case CTRL('d'):
                __debug_fnc();
                moveCursor(0, 0);
                refresh_next_frame_();
                break;
            #endif

            // 'proper' inputs
            case 9:
                insertTab();
                break;

            case KEY_BTAB:
                removeLeadingTab();
                break;

            case '{':
            case '\'':
            case '"':
            case '[':
            case '(':
                insertStructuralLiteral(_c);
                break;
            
            // TODO : closing structural literals:
            //case '}':
            //case '"': -- duplicate!
            //case ']':
            //case ')':

            default:
                insertCharAtCursor((char)_c);
                break;

        }
    }

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::scroll_(int _axis, int _dir, int _steps, bool _update_current_line)
{
    ivec2_t scroll = { 0, 0 };
    switch (_axis)
    {
        case X_AXIS:
            scroll.x = _dir < 0 ? -_steps : _steps;
            m_cursor.set_scrolled_x();
            break;

        case Y_AXIS:
            int n = 0;
            int n_scrolled_y = 0;
            
            // scroll up
            if (_dir < 0)
            {
                while (n < _steps && m_pageFirstLine->prev != NULL)
                {
                    m_pageFirstLine = m_pageFirstLine->prev;
                    n++;
                    n_scrolled_y--;
                }

            }
            // scroll down
            else
            {
                while (n < _steps)
                {
                    m_pageFirstLine = m_pageFirstLine->next;
                    n++;
                    n_scrolled_y++;
                }

            }

            scroll.y = n_scrolled_y;
            m_cursor.set_scrolled_y();
            break;
    }
    
    if (_update_current_line)
        updateCurrentLinePtr(scroll.y);

    // update scroll position
    m_scrollPos += scroll;
    
    clear_next_frame_();
    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveCursor(int _dx, int _dy)
{
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

    // beginning of line, move to last char of prev
    if (cx == 0 && dx < 0 && m_currentLine->prev != NULL)
    {
        dx = m_currentLine->prev->len - cx;
        dy = -1;
    }
    // last char in line, move to first char of next
    else if (cx == m_currentLine->len && dx > 0 && (dy == 1 || dy == 0) && m_currentLine->next != NULL)
    {
        dx = -cx;
        dy = 1;
    }
    // last char of last line, do nothing
    else if (cx == m_currentLine->len && dx > 0 && m_currentLine->next == NULL)
    {
        dx = 0;
        dy = 0;
    }

    // prevent out of bounds
    if (dy > 0 && m_currentLine->next == NULL)
        dy = 0;
        
    // move the cursor (if possible)
    m_cursor.move(dx, dy);
    ivec2_t new_pos = m_cursor.cpos();

    // update pointer into line buffer
    updateCurrentLinePtr(new_pos.y - cy);

    refresh_next_frame_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveCursorToLineBegin()
{
    int first_char = find_first_non_empty_char_(m_currentLine);
    if (m_cursor.cx() == first_char)
        first_char = 0;
    moveCursor(-(m_cursor.cx() - first_char), 0);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveCursorToLineEnd()
{
    moveCursor(m_currentLine->len - m_cursor.cx(), 0);

}

//---------------------------------------------------------------------------------------
int FileBufferWindow::findColDelim(int _dir, bool _move_cursor)
{
    // change lines if at beginning or end of line
    if (_dir > 0 && m_cursor.cx() >= m_currentLine->len - 1)    { if (_move_cursor) moveCursor( 1, 0); return  1; }
    else if (_dir < 0 && m_cursor.cx() <= 1)                    { if (_move_cursor) moveCursor(-1, 0); return -1; }

    int cx = m_cursor.cx();
    int dx = 0;
    //
    CHTYPE_PTR p = m_currentLine->content + cx;
    bool start_on_delimiter = is_col_delimiter_(*p);

    p += _dir;
    start_on_delimiter = is_col_delimiter_(*p);
    dx += _dir;
    char c = (*p & CHTYPE_CHAR_MASK);
    // starting on a delimiter -> skip all delimiters and then find the next
    if (start_on_delimiter)
    {
        while (is_col_delimiter_((*p & CHTYPE_CHAR_MASK)) && cx + dx < m_currentLine->len && cx + dx >= 0)
        {
            p += _dir;
            c = (*p & CHTYPE_CHAR_MASK);
            dx += _dir;
        }
    }
    // just jump to next delimiter
    else
    {
        while (!is_col_delimiter_((*p & CHTYPE_CHAR_MASK)) && cx + dx < m_currentLine->len && cx + dx >= 0)
        {
            p += _dir;
            c = (*p & CHTYPE_CHAR_MASK);
            dx += _dir;
        }

    }

    // special case where the first char is preceeded by only whitespace (i.e. tabs),
    // then move to the first actual character
    // if (_dir < 0 && find_first_non_empty_char_(m_currentLine) > cx + dx)
    // Actually, we want to jump to the non-delimieter when going backwards, always
    if (_dir < 0)
        dx++;

    if (_move_cursor)
        moveCursor(dx, 0);

    return dx;

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveCursorToRowDelim(int _dir)
{
    line_t *line = m_currentLine;
    
    int y = (_dir < 0 ? m_cursor.cy() : 0);
    bool do_continue = (_dir < 0 ? line->prev != NULL : line->next != NULL);
    while (do_continue)
    {
        line = (_dir < 0 ? line->prev : line->next);
        y += _dir;

        if (line->len == 0 || is_row_empty_(line))
            break;

        do_continue = (_dir < 0 ? line->prev != NULL : line->next != NULL);

    }

    int dy = (_dir < 0 ? -(m_cursor.cy() - y) : y);
    moveCursor(0, dy);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::movePageUp()
{
    moveCursor(0, -Config::PAGE_SIZE);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::movePageDown()
{
    int steps = Config::PAGE_SIZE;
    if (m_bufferCursorPos.y + Config::PAGE_SIZE > m_lineBuffer.lineCount() - 1)
        steps = m_lineBuffer.lineCount() - 1 - m_bufferCursorPos.y;
    moveCursor(0, steps);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveFileBegin()
{
    moveCursor(-m_cursor.cx(), -m_bufferCursorPos.y);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::moveFileEnd()
{
    int steps = m_lineBuffer.lineCount() - 1 - m_bufferCursorPos.y;
    // two moves are needed since the x move causes the cursor to use 'last char in line'
    // behaviour and set dy to 1... well, well..
    // another hack is needed to prevent the x move to overwrite the prev line ptr
    line_t *p = m_currentLine;
    moveCursor(0, steps);
    moveCursor(m_lineBuffer.m_tail->len - m_cursor.cx(), 0);
    m_prevLine = p;
}

//---------------------------------------------------------------------------------------
void FileBufferWindow::insertCharAtCursor(char _c)
{
    // only allowed characters (for now)
    if (Config::ALLOWED_CHAR_SET.find(_c) == Config::ALLOWED_CHAR_SET.end())
    {
        moveCursor(0, 0);
        return;
    }

    // check if we delete a selection as part of this insert
    int deleted = delete_selection_();
    if (deleted)
        updateBufferCursorPos();

    ivec2_t start_pos = m_bufferCursorPos;

    insertCharAtPos(_c, m_cursor.cx());

    //
    if (m_storeActions)
    {
        // really not interested in content since undo is delete, but let's copy anyways
        line_chars_t chars(start_pos, m_currentLine->__debug_str+start_pos.x, 1);
        undo_item_t undo(UndoAction::CHAR_ADD, chars);
        if (deleted)
            undo.deleted_selection = true;
        m_undoBuffer.push(undo);
    }

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::insertCharAtPos(char _c, size_t _pos, bool _update_cursor)
{
    m_currentLine->insert_char(_c, _pos);
    if (_update_cursor)
        moveCursor(1, 0);

    refresh_next_frame_();
    buffer_changed_();
    syntax_highlight_buffer_();
}

//---------------------------------------------------------------------------------------
void FileBufferWindow::insertStrAtCursor(char *_str, size_t _len, bool _update_cursor)
{
    CHTYPE_PTR str_ = (CHTYPE_PTR)malloc(CHTYPE_SIZE * _len);
    for (size_t i = 0; i < _len; i++)
        str_[i] = _str[i];

    insertStrAtCursor(str_, _len, _update_cursor);

    free(str_);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::insertStrAtCursor(CHTYPE_PTR _str, size_t _len, bool _update_cursor)
{
    ivec2_t start_pos = m_bufferCursorPos;

    m_currentLine->insert_str(_str, _len, m_cursor.cx());
    
    if (_update_cursor)
        moveCursor(_len, 0);

    // if tabs or spaces needs update -- rendering-related only!
    for (size_t i = 0; i < _len; i++)
    {
        if (_str[i] == '\t' || _str[i] == ' ')
        {
            m_windowLinesUpdateList.insert(m_cursor.cy());
            break;
        }
    }

    if (m_storeActions)
    {
        // really not interested in content since undo is delete, but let's copy anyways
        line_chars_t chars(start_pos, m_currentLine->__debug_str+start_pos.x, _len);
        m_undoBuffer.push(undo_item_t(UndoAction::STRING_ADD, chars));
    }

    refresh_next_frame_();
    buffer_changed_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::insertNewLine(bool _auto_align)
{
    if (delete_selection_())
        return;

    ivec2_t start_pos = m_bufferCursorPos;

    // undo item -- mline_block_t.start_pos stores the cursor position at newline nad
    // mline_block_t.end_pos stores the indent level. mline_block_t.copy_lines stores 
    // the content of the newly formed lines so they may be concatenated upon undo
    mline_block_t undo_block;
    undo_block.setStart(start_pos);

    // need to check if this is a newline between two structural literals
    int cx = m_cursor.cx();
    int cy = m_cursor.cy();

    bool new_empty_line = false;
    if (cx > 0 && is_structural_literal_(m_currentLine->content[cx]) > 0)
        new_empty_line = true;

    // split line at cursor pos.x
    line_t *new_line = m_currentLine->split_at_pos(m_cursor.cx());
    m_lineBuffer.insertAtPtr(m_currentLine, INSERT_AFTER, new_line);

    // store the current line (all inserted spaces will be removed upon undo concat)
    undo_block.copy_lines.push_back(copy_line_t(m_currentLine, false, false));

    // find correct indentation -- use spaces, not tabs
    // TODO :   1.  Use TabsOrSpaces Config::USE_TABS_OR_SPACES to determine tabs or
    //              spaces, for now, use spaces.
    //
    if (_auto_align)
    {
        int white_spaces = find_indentation_level_(m_currentLine);
        int next_tab = 0;
        undo_block.setEnd(ivec2_t(white_spaces, 0));

        for (int i = 0; i < white_spaces; i++)
            new_line->insert_char(' ', 0);
        if (new_empty_line)
        {
            m_lineBuffer.insertAtPtr(m_currentLine, INSERT_AFTER, "");
            white_spaces = find_indentation_level_(m_currentLine);
            next_tab = find_next_tab_stop_(white_spaces);
            white_spaces = next_tab;
            for (int i = 0; i < next_tab; i++)
                m_currentLine->next->insert_char(' ', 0);
            
            // store this new line
            undo_block.copy_lines.push_back(copy_line_t(NULL, false, false));
        }

        int dy = m_frame.nrows - cy;
        if (new_empty_line && 3 > dy)
        {
            scroll_(Y_AXIS, 1, 3 - dy, true); // 2 or 1
            moveCursor(0, -(2 - dy));
        }
        else
        {
            moveCursor(0, 1);

        }
        moveCursor(-(m_cursor.cx() - white_spaces), 0);
        
    }
    else
        moveCursor(-cx, 1);

    // store last line    
    undo_block.copy_lines.push_back(copy_line_t(new_line, false, false));

    //
    if (m_storeActions)
        m_undoBuffer.push(undo_item_t(UndoAction::LINE_NEW, undo_block));

    //
    update_lines_after_y_(cy - 1);
    refresh_next_frame_();
    buffer_changed_();
    syntax_highlight_buffer_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::insertTab()
{
    ivec2_t start_pos = m_bufferCursorPos;

    if (m_selection->lineCount(m_bufferCursorPos))
    {
        int cy_start = (m_bufferCursorPos.y <  m_selection->startingBufferPos().y ? 
                        m_bufferCursorPos.y : m_selection->startingBufferPos().y);
        int sel_start_y = cy_start;
        cy_start -= m_scrollPos.y;
        // store start position in mline_block_t.start_pos, using the 
        // std::vector<copy_lint_t> for the size of the selection. mline_block_t.end_pos
        // is used to mark the selection start (i.e. the first line)
        mline_block_t undo_block;
        undo_block.setStart(start_pos);
        undo_block.setEnd(ivec2_t(m_selection->lineCount(m_bufferCursorPos), sel_start_y));

        line_t *line = m_lineBuffer.ptrFromIdx(sel_start_y);
        size_t i = 0;
        size_t sel_count = m_selection->lineCount(m_bufferCursorPos);
        while (i < sel_count && line != NULL)
        {
            if (line->sel_end - line->sel_start == 0)
            {
                undo_block.copy_lines.push_back(copy_line_t(NULL, false, false));
                line = line->next;
                i++;
                continue;
            }
            
            undo_block.copy_lines.push_back(copy_line_t(line, false, false));
            insert_leading_tab_(line);
            m_selection->expandSelection(line, 0, Config::TAB_SIZE);

            line = line->next;
            i++;
        }

        update_lines_after_y_(cy_start);

        //
        if (m_storeActions)
            m_undoBuffer.push(undo_item_t(UndoAction::MTABS_ADD, undo_block));

    }
    else
    {
        int cx = m_cursor.cx();

        char buffer[Config::TAB_SIZE];
        memset(buffer, ' ', Config::TAB_SIZE);

        int next_stop = find_next_tab_stop_(cx);
        int n = next_stop - cx;

        m_currentLine->insert_str(buffer, n, cx);
        moveCursor(n, 0);

        m_windowLinesUpdateList.insert(m_cursor.cy());

        if (m_storeActions)
        {
            // really not interested in content since undo is delete, but let's copy anyways
            line_chars_t chars(start_pos, m_currentLine->__debug_str+start_pos.x, n);
            m_undoBuffer.push(undo_item_t(UndoAction::STRING_ADD, chars));

        }
    }

    refresh_next_frame_();
    buffer_changed_();
    syntax_highlight_buffer_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::removeLeadingTab()
{
    ivec2_t start_pos = m_bufferCursorPos;

    if (m_selection->lineCount(m_bufferCursorPos))
    {
        int cy_start = (m_bufferCursorPos.y <  m_selection->startingBufferPos().y ? 
                        m_bufferCursorPos.y : m_selection->startingBufferPos().y);
        int sel_start_y = cy_start;
        cy_start -= m_scrollPos.y;
        // store cursor position in mline_block_t.start_pos, and .end_pos.x to store the
        // line count and .end_pos.y to store the index of the first line in selection
        // the std::vector<copy_lint_t> stores the lines
        mline_block_t undo_block;
        auto &clines = undo_block.copy_lines;
        undo_block.setStart(start_pos);

        line_t *line = m_lineBuffer.ptrFromIdx(sel_start_y);
        size_t i = 0;
        size_t sel_count = m_selection->lineCount(m_bufferCursorPos);
        while (i < sel_count && line != NULL)
        {
            if (line->sel_end - line->sel_start == 0)
            {
                clines.push_back(copy_line_t(NULL, false, false));
                line = line->next;
                i++;
                continue;
            }

            int first_char = find_first_non_empty_char_(line);
            if (!first_char)
            {
                clines.push_back(copy_line_t(NULL, false, false));
                line = line->next;
                i++;
                continue;
            }

            // find previous tab stop before first char
            int steps = first_char - find_prev_tab_stop_(first_char);
            line->delete_n_at(0, steps);

            if (steps == 0)
                clines.push_back(copy_line_t(NULL, false, false));
            else
                clines.push_back(copy_line_t(line, false, false));

            // update selection
            line->sel_end -= steps;

            //
            line = line->next;
            i++;

        }
        update_lines_after_y_(cy_start);

        //
        if (m_storeActions)
        {
            // int sel_start_y = (m_selection->startingBufferPos().y < m_bufferCursorPos.y ? 
            //                    m_selection->startingBufferPos().y : m_bufferCursorPos.y);
            undo_block.setEnd(ivec2_t(m_selection->lineCount(m_bufferCursorPos), sel_start_y));
            m_undoBuffer.push(undo_item_t(UndoAction::MTABS_DEL, undo_block));
        }

    }
    else
    {
        // if tabbed before line chars, removes one tabstop
        int first_char = find_first_non_empty_char_(m_currentLine);
        if (!first_char)
        {
            moveCursor(0, 0);
            return;
        }
        
        // find previous tab stop before first char
        int steps = first_char - find_prev_tab_stop_(first_char);
        for (int i = 0; i < steps; i++)
            m_currentLine->delete_at(0);
        if (m_cursor.cx() >= steps)
            moveCursor(-steps, 0);

        m_windowLinesUpdateList.insert(m_cursor.cy());
    }

    refresh_next_frame_();
    buffer_changed_();
    syntax_highlight_buffer_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::insertStructuralLiteral(char _c)
{
    char next_char = m_currentLine->__debug_str[m_cursor.cx() + 1];
    if (next_char == ' ' || next_char == 0 || !m_currentLine->len || m_selection->lineCount(m_bufferCursorPos))
    {
        char b[2];
        switch (_c)
        {
            case '{':   b[0] = '{';  b[1] = '}';    break;
            case '\'':  b[0] = '\''; b[1] = '\'';   break;
            case '"':   b[0] = '"';  b[1] = '"';    break;
            case '[':   b[0] = '[';  b[1] = ']';    break;
            case '(':   b[0] = '(';  b[1] = ')';    break;
        }
        if (m_selection->lineCount(m_bufferCursorPos))
        {
            ivec2_t bpos = m_bufferCursorPos;
            ivec2_t sspos = m_selection->startingBufferPos();

            mline_block_t undo_block;
            undo_block.setStart(bpos);
            undo_block.setEnd(sspos);

            if (bpos.y != sspos.y)
            {                             
                if (sspos.y > bpos.y)
                    std::swap(sspos, bpos);

                line_t *ssline = m_lineBuffer.ptrFromIdx(sspos.y);
                ssline->insert_char(b[0], sspos.x);
                line_t *bline = m_lineBuffer.ptrFromIdx(bpos.y);
                bline->insert_char(b[1], bpos.x);
                moveCursor(1, 0);

                deselect_(FORWARD);
            }
            else
            {
                if (sspos.x > m_bufferCursorPos.x)
                    std::swap(sspos, bpos);

                insertCharAtPos(b[0], sspos.x, false);
                insertCharAtPos(b[1], bpos.x + 1);
                
                deselect_(FORWARD);
            }

            //
            if (m_storeActions)
                m_undoBuffer.push(undo_item_t(UndoAction::LITERAL_MADD, undo_block));

        }
        else
        {
            delete_selection_();
            insertStrAtCursor(b, 2);
            moveCursor(-1, 0);
        }

    }
    else
    {
        delete_selection_();
        insertCharAtCursor(_c);
    }
    
    refresh_next_frame_();
    buffer_changed_();
    syntax_highlight_buffer_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::deleteCharAtCursor()
{
    // <DEL>

    if (delete_selection_())
        return;

    ivec2_t start_pos = m_bufferCursorPos;

    int cx = m_cursor.cx();

    //
    if ((cx == 0 || cx == m_currentLine->len) &&
        m_currentLine->len == 0 &&
        m_currentLine->next == NULL)
    {
        moveCursor(0, 0);
        return;
    }
    // in line
    else if (cx < m_currentLine->len)
    {
        line_chars_t ch(start_pos, &m_currentLine->__debug_str[cx], 1);

        m_currentLine->delete_at(cx);
        m_windowLinesUpdateList.insert(m_cursor.cy());

        if (m_storeActions)
            m_undoBuffer.push(undo_item_t(UndoAction::CHAR_DEL, ch));

    }
    // end of line (with len > 0)
    else
    {
        mline_block_t undo_block;
        undo_block.setStart(start_pos);
        // using mline_block_t.end_pos.x to store len of current line
        undo_block.setEnd(ivec2_t(m_currentLine->len, 0));
        
        m_lineBuffer.appendNextToThis(m_currentLine);

        if (m_storeActions)
            m_undoBuffer.push(undo_item_t(UndoAction::LINE_COLLAPSE_NEXT, undo_block));

        update_lines_after_y_(m_cursor.cy());
    }

    refresh_next_frame_();
    buffer_changed_();
    syntax_highlight_buffer_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::deleteToNextColDelim()
{
    // <CTRL> + <DEL>

    if (delete_selection_())
        return;

    ivec2_t start_pos = m_bufferCursorPos;

    int cx = m_cursor.cx();

    int nspaces = find_empty_chars_from_pos_(cx, FORWARD);
    int next_delim_pos_dx = findColDelim(FORWARD, false);
    
    int ndels = next_delim_pos_dx;
    if (nspaces < next_delim_pos_dx && nspaces > 0)
        ndels = nspaces;
    
    //
    if (cx + ndels <= m_currentLine->len)
    {
        if (m_storeActions)
        {
            line_chars_t str(start_pos, &m_currentLine->__debug_str[cx], ndels);
            m_undoBuffer.push(undo_item_t(UndoAction::STRING_DEL, str));
        }

        m_currentLine->delete_n_at(cx, ndels);

    }
    // if at end of line, collapse lines
    else
        deleteCharAtCursor();        

    m_windowLinesUpdateList.insert(m_cursor.cy());

    refresh_next_frame_();
    buffer_changed_();
    syntax_highlight_buffer_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::deleteCharBeforeCursor()
{
    // <BACKSPACE>
    
    if (delete_selection_())
        return;

    ivec2_t start_pos = m_bufferCursorPos;

    int cx = m_cursor.cx();

    // beggining of first line
    if (cx == 0 && m_currentLine->prev == NULL)
    {
        m_cursor.move(0, 0);
        refresh_next_frame_();
        return;
    }
    // at x 0 and not first line
    else if (cx == 0 && m_currentLine->prev != NULL)
    {
        if (m_cursor.cy() == 0)
            scroll_(Y_AXIS, -1, 1, false);

        mline_block_t undo_block;
        undo_block.setStart(start_pos);

        //
        size_t prev_len = m_currentLine->prev->len;

        // store prev len for undo in mline_block_t.end_pos.x
        undo_block.setEnd(ivec2_t(prev_len, 0));

        // coalesce lines
        m_currentLine = m_lineBuffer.appendThisToPrev(m_currentLine);
        m_cursor.set_cpos(prev_len, m_cursor.cy() - 1);

        //
        // undo_block.copy_lines.push_back(copy_line_t(m_currentLine, false, false));
        if (m_storeActions)
            m_undoBuffer.push(undo_item_t(UndoAction::LINE_COLLAPSE_PREV, undo_block));

        // redraw lines
        update_lines_after_y_(m_cursor.cy());

    }
    // inside line
    else if (cx > 0)
    {
        // if before printed chars remove spaces one tabstop at the time
        if (find_empty_chars_from_pos_(cx, BACKWARD) > 1)
        {
            int dx = 0;
            int steps = 0;
            // find previous tab stop
            dx = find_prev_tab_stop_(cx);
            steps = cx - dx;

            if (m_storeActions)
            {
               start_pos.x -= steps;
               line_chars_t str(start_pos, &m_currentLine->__debug_str[cx - steps], steps);
               undo_item_t undo(UndoAction::STRING_ERASE, str);
               undo.move_cursor = false;
               m_undoBuffer.push(undo);
            }

            // delete the characters
            for (int i = 0; i < steps; i++)
                m_currentLine->delete_at(cx - steps);
            m_cursor.move(-steps, 0);

        }
        else
        {
            if (m_storeActions)
            {
                start_pos.x -= 1;
                line_chars_t ch(start_pos, &m_currentLine->__debug_str[cx - 1], 1);
                m_undoBuffer.push(undo_item_t(UndoAction::CHAR_ERASE, ch));
            }

            m_currentLine->delete_at(cx - 1);
            m_cursor.move(-1, 0);

        }
        m_windowLinesUpdateList.insert(m_cursor.cy());

    }

    refresh_next_frame_();
    buffer_changed_();
    syntax_highlight_buffer_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::deleteToPrevColDelim()
{
    // <CTRL> + <BACKSPACE>

    if (delete_selection_())
        return;

    if (m_cursor.cx() == 0)
    {
        deleteCharBeforeCursor();
        return;
    }

    ivec2_t start_pos = m_bufferCursorPos;
    
    int prev_delim_pos_dx = findColDelim(BACKWARD, false);
    int cx = m_cursor.cx();
    int nspaces = find_empty_chars_from_pos_(cx, BACKWARD);

    if (abs(prev_delim_pos_dx) > m_currentLine->len)
        prev_delim_pos_dx++;
    
    int ndels = abs(prev_delim_pos_dx);
    if (nspaces < abs(prev_delim_pos_dx) && nspaces > 0)
        ndels = nspaces;
    
    if (m_storeActions)
    {
        start_pos.x -= ndels;
        line_chars_t str(start_pos, &m_currentLine->__debug_str[cx - ndels], ndels);
        undo_item_t undo(UndoAction::STRING_DEL, str);
        undo.move_cursor = true;
        m_undoBuffer.push(undo);
    }

    m_currentLine->delete_n_at(cx - ndels, ndels);

    moveCursor(-ndels, 0);
    m_windowLinesUpdateList.insert(m_cursor.cy());

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::updateCursor()
{
    static ivec2_t prev_pos = m_cursor.cpos();

    // actually move the cursor
    m_cursor.update();

    //
    updateBufferCursorPos();

    //
    if (prev_pos != m_cursor.cpos())
    {
        refresh_next_frame_();
        prev_pos = m_cursor.cpos();
    }

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::copySelection(std::vector<copy_line_t> *_store_buffer)
{
    if (!m_selection->lineCount(m_bufferCursorPos))
        return;

    if (_store_buffer == NULL)
        _store_buffer = &m_copyBuffer;

    _store_buffer->clear();

    //
    ivec2_t spos = m_selection->startingBufferPos();
    ivec2_t bpos = m_bufferCursorPos;

    // single line
    if (bpos.y == spos.y && bpos.x < spos.x)
        std::swap(bpos, spos);
    // multi line
    else if (bpos.y < spos.y)
        std::swap(bpos, spos);

    //
    line_t *line_ptr = m_lineBuffer.ptrFromIdx(spos.y);
    size_t y = spos.y;
    while (y <= bpos.y && line_ptr != NULL)
    {
        if (m_selection->isLineEntry(line_ptr))
        {
            _store_buffer->push_back(copy_line_t(line_ptr, y != bpos.y));
        }

        y++;
        line_ptr = line_ptr->next;

    }

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::deleteSelection()
{
    if (!m_selection->lineCount(m_bufferCursorPos))
        return;
    
    ivec2_t start = m_selection->startingBufferPos();
    ivec2_t end = m_bufferCursorPos;

    if ((end.y == start.y && end.x < start.x) ||
        (end.y < start.y))
        std::swap(end, start);

    // handle future undo
    std::vector<copy_line_t> undo_lines;
    copySelection(&undo_lines);
    m_undoBuffer.push(undo_item_t(UndoAction::LINES_DEL,
                                  start,
                                  end,
                                  undo_lines));

    // 'manual' deselect_()
    m_selection->clear();
    m_isSelecting = false;

    //
    gotoBufferCursorPos(start);

    // remove selected text
    line_t *line_ptr = m_lineBuffer.ptrFromIdx(start.y);
    int nlines = end.y - start.y;
    bool merge_after = (line_ptr->sel_start != 0 && nlines != 0);

    //
    int y = 0;
    while (y <= nlines && line_ptr != NULL)
    {
        size_t sel_len = line_ptr->sel_end - line_ptr->sel_start;
        line_t *lnext = line_ptr->next;
        // delete entire line
        if (sel_len == line_ptr->len && y != nlines)
            m_lineBuffer.deleteAtPtr(line_ptr);
        // delete part of line
        else
            line_ptr->delete_n_at(line_ptr->sel_start, sel_len);
        
        y++;
        line_ptr = lnext;

    }
    // update line ptrs
    m_currentLine = m_lineBuffer.ptrFromIdx(start.y);
    m_pageFirstLine = m_lineBuffer.ptrFromIdx(m_scrollPos.y);

    // if first line of selection was not a complete line
    if (merge_after)
        m_lineBuffer.appendNextToThis(m_currentLine);

    //
    update_lines_after_y_(m_cursor.cy());
    refresh_next_frame_();
    buffer_changed_();
    syntax_highlight_buffer_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::cutSelection()
{
    copySelection();
    deleteSelection();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::paste()
{
    if (m_copyBuffer.size() == 0)
        return;

    if (m_selection->lineCount(m_bufferCursorPos))
        deleteSelection();

    bool was_storing_actions = m_storeActions;
    m_storeActions = false;

    //
    ivec2_t pre_paste = m_bufferCursorPos;
    int cy = m_cursor.cy();

    // first entry
    copy_line_t cline = m_copyBuffer[0];
    insertStrAtCursor(cline.line_chars, cline.len);
    if (cline.newline)
        insertNewLine(false);

    if (m_copyBuffer.size() > 1)
    {
        for (size_t i = 1; i < m_copyBuffer.size() - 1; i++)
        {
            copy_line_t cline1 = m_copyBuffer[i];
            // paste lines as a new lines (omitting last line)
            insertLineAtCursor(cline1.line_chars, cline1.len);
        }

        // last line
        cline = m_copyBuffer[m_copyBuffer.size() - 1];
        insertStrAtCursor(cline.line_chars, cline.len);
        if (cline.newline)
            insertNewLine(false);
    }

    //
    m_currentLine = m_lineBuffer.ptrFromIdx(m_cursor.cy() + m_scrollPos.y);
    m_pageFirstLine = m_lineBuffer.ptrFromIdx(m_scrollPos.y);

    updateCursor();
    
    m_storeActions = was_storing_actions;

    if (m_storeActions)
    {
        ivec2_t post_paste = m_bufferCursorPos;
        auto copy_buffer = m_copyBuffer;
        copy_buffer[0].offset0 = pre_paste.x;
        m_undoBuffer.push(undo_item_t(UndoAction::LINES_ADD,
                                      pre_paste,
                                      post_paste,
                                      copy_buffer));
    }

    //
    update_lines_after_y_(cy);
    refresh_next_frame_();
    buffer_changed_();
    syntax_highlight_buffer_();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::insertLineAtCursor(char *_content, size_t _len)
{
    insertStrAtCursor(_content, _len);
    insertNewLine(false);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::gotoBufferCursorPos(const ivec2_t &_pos)
{
    int dx = _pos.x - m_bufferCursorPos.x;
    int dy = _pos.y - m_bufferCursorPos.y;
    moveCursor(dx, dy);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::updateBufferCursorPos()
{
    //
    m_prevBufferCursorPos = m_bufferCursorPos;
    m_bufferCursorPos = m_scrollPos + m_cursor.cpos();
    
    // cursor moved in buffer, and in selection mode, add selection
    if (m_prevBufferCursorPos != m_bufferCursorPos && m_isSelecting)
    {
        ivec2_t prev = m_prevBufferCursorPos;
        ivec2_t curr = m_bufferCursorPos;

        m_dirOfSelection = FORWARD;

        line_t *prev_ptr = m_prevLine;
        line_t *curr_ptr = m_currentLine;

        // same line
        if (prev.y == curr.y)
        {
            if (curr.x < prev.x)
            {
                std::swap(prev, curr);
                m_dirOfSelection = BACKWARD;
            }

            if (curr.x > 0)
                m_selection->selectChars(prev_ptr, prev.x, curr.x);
        }

        // different lines -- here we need the offset into the first and last selected
        // line and the starting and ending line pointers
        else
        {
            if (prev.y > curr.y)
            {
                std::swap(prev, curr);
                std::swap(prev_ptr, curr_ptr);
                m_dirOfSelection = BACKWARD;
            }

            m_selection->selectLines(prev_ptr, prev.x, curr_ptr, curr.x);
        }

        //
        refresh_next_frame_();
        refresh();
        redraw();

    }
}

//---------------------------------------------------------------------------------------
void FileBufferWindow::updateCurrentLinePtr(int _dy)
{
    // if the cursor moved, update current line in buffer
    if (_dy > 0)
    {
        for (int i = 0; i < abs(_dy); i++)
            if (m_currentLine->next != NULL) m_currentLine = m_currentLine->next; 
    }
    else if (_dy < 0)
    {
        for (int i = 0; i < abs(_dy); i++)
            if (m_currentLine->prev != NULL) m_currentLine = m_currentLine->prev; 
    }

}

//---------------------------------------------------------------------------------------
int FileBufferWindow::readFileToBuffer(const std::string &_filename)
{
    int ret = FileIO::read_file_to_buffer(_filename, &m_lineBuffer);
    if (!m_lineBuffer.lineCount())
        m_lineBuffer.push_front("");
    
    // apply syntax highlighting
    // TODO :   inherit lexer base class and intialize here depending on filetype
    //          i.e. LexerC_CPP, LexerSH, LexerPY etc.
    switch (FileIO::s_lastFileType)
    {
        case C_CPP:     m_lexer = new Lexer_C_CPP;  m_filetype = "C/C++";       break;
        case TXT:       m_lexer = new Lexer_TXT;    m_filetype = "TXT";         break;
        case PY:        m_lexer = new Lexer_TXT;    m_filetype = "Python";      break;
        case JS:        m_lexer = new Lexer_TXT;    m_filetype = "JavaScript";  break;
        default:        m_lexer = new Lexer_TXT;    m_filetype = "TXT";         break;
    }

    //    
    m_lexer->parseBuffer(&m_lineBuffer);
    m_syntaxHLNextFrame = true;
    
    // line pointers
    m_currentLine = m_lineBuffer.m_head;
    m_pageFirstLine = m_lineBuffer.m_head;

    m_filename = std::string(_filename);

    // initialize line numbers window
    if (m_filename != "" && Config::SHOW_LINE_NUMBERS && m_useLineNumbers)
    {
        int width = (int)std::round(std::log10((float)m_lineBuffer.m_lineCount) + 1) + 1;
        // also leave 2 blank spaces left and 1 before the divider
        width += 4;
        m_lineNumbers->setWidth(width);
        resize(m_frame);

    }

    m_isDirty = false;

    return ret;

}

//---------------------------------------------------------------------------------------
int FileBufferWindow::writeBufferToFile(const std::string &_filename)
{
    if (!m_isDirty)
        return 0;
        
    std::string fn = (_filename == "" ? m_filename : std::string(_filename));

    int ret = FileIO::write_buffer_to_file(fn, &m_lineBuffer);
    m_isDirty = false;

    //
    moveCursor(0, 0);

    return ret;

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::resize(frame_t _new_frame)
{
    // the _left_reserved argument is used to reserve space of the LineNumbers window
    // (eg when called from FileBufferWindow::readFileToBuffer when we know the max 
    // number of lines in the file).
    //

    m_frame = _new_frame;
    m_frame.update_dims();

    // if (m_frame.v0.x != m_lineNumbers->getWidth())
    if (m_useLineNumbers)
        m_frame.v0.x = m_lineNumbers->getWidth();

    //if (_left_reserved != -1 && m_frame.v0.x != _left_reserved)
    //{
    //    m_frame.v0.x = _left_reserved;
    //    m_frame.update_dims();
    //}

    api->clearWindow(m_apiWindowPtr);
    api->deleteWindow(m_apiWindowPtr);
    m_apiWindowPtr = api->newWindow(&m_frame);  // resfresh called by api->newWindow()
    
    if (m_apiBorderWindowPtr)
    {
        api->clearWindow(m_apiBorderWindowPtr);
        api->deleteWindow(m_apiBorderWindowPtr);
        m_apiBorderWindowPtr = api->newBorderWindow(&m_frame);
    }

    if (m_useLineNumbers)
        m_lineNumbers->resize(m_frame);

    m_cursor.set_frame(m_frame);

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::redraw()
{
    // order matters here
    m_lineNumbers->redraw();

    #if (defined DEBUG) & (defined NCURSES_IMPL) & 0
    updateCursor();
    int x = 110;
    int y = 0;
    __debug_print(x, y++, "bpos  = (%d, %d)", m_bufferCursorPos.x, m_bufferCursorPos.y);
    __debug_print(x, y++, "pbpos = (%d, %d)", m_prevBufferCursorPos.x, m_prevBufferCursorPos.y);
    __debug_print(x, y++, "cpos  = (%d, %d)", m_cursor.cx(), m_cursor.cy());
    __debug_print(x, y++, "rpos  = (%d, %d)", m_cursor.rx(), m_cursor.ry());
    __debug_print(x, y++, "spos  = (%d, %d)", m_scrollPos.x , m_scrollPos.y);
    y++;
    if (m_currentLine != NULL)
    {
        CHTYPE c = m_currentLine->content[m_cursor.cx()];
        __debug_print(x, y++, "is_delim: %s", is_col_delimiter_(c) ? "true" : "false");
        // int16_t color = ncurses_get_CHTYPE_color(c);
        // TokenKind tk = dynamic_cast<Lexer_C_CPP *>(m_lexer)->tokenFromColor(color);
        // __debug_print(x, y++, "cpos HL:  %s", token2str(tk));
    }
    y++;
    if (m_currentLine != NULL)
    {
        __debug_print(x, y++, "this line: '%s'", m_currentLine->__debug_str);
        __debug_print(x, y++, "this len:  %zu", m_currentLine->len);
    }
    //if (m_prevLine != NULL)
    //{
    //    __debug_print(x, y++, "prev line: '%s'", m_prevLine->__debug_str);
    //    __debug_print(x, y++, "prev len:  %zu", m_prevLine->len);
    //}
    y++;
    if (m_pageFirstLine != NULL)    __debug_print(x, y++, "page[ 0]: '%s'", m_pageFirstLine->__debug_str);
    else                            __debug_print(x, y++, "page[ 0]: NULL");

    __debug_print(x, y++, "m_lineBuffer.size() = %zu", m_lineBuffer.lineCount());
    if (m_lineBuffer.m_head != NULL) __debug_print(x, y++, "buffer head: '%s'", m_lineBuffer.m_head->__debug_str);
    if (m_lineBuffer.m_tail != NULL) __debug_print(x, y++, "buffer tail: '%s'", m_lineBuffer.m_tail->__debug_str);


    y++;
    __debug_print(x, y++, "selecting: %s", m_isSelecting ? "true" : "false");
    if (m_selection != NULL)
       __debug_print(x, y++, "selected line count = %zu", m_selection->lineCount(m_bufferCursorPos));

    #endif

    if (m_isWindowVisible)
    {
        if (m_syntaxHLNextFrame)
        {
            m_lexer->parseBuffer(&m_lineBuffer);
            m_syntaxHLNextFrame = false;
        }
        
        // check for lines that have been changed
        if (m_windowLinesUpdateList.size() > 0)
        {
            for (auto &line_y : m_windowLinesUpdateList)
                api->clearBufferLine(m_apiWindowPtr, line_y, m_frame.ncols);

            m_windowLinesUpdateList.clear();
        }

        // render buffer
        m_formatter.render(m_apiWindowPtr, m_pageFirstLine, m_currentLine);
    }

    updateCursor();

}

//---------------------------------------------------------------------------------------
void FileBufferWindow::clear()
{
    if (m_useLineNumbers)
        m_lineNumbers->clear();

    if (m_clearNextFrame)
    {
        api->clearWindow(m_apiWindowPtr);
        m_clearNextFrame = false;
    }

}
//---------------------------------------------------------------------------------------
void FileBufferWindow::refresh()
{
    if (m_useLineNumbers)
        m_lineNumbers->refresh();   // called first to move the cursor to the buffer window
                                    // on updateCursor()

    if (m_refreshNextFrame)
    {
        // This causes screen flickering!!!
        //if (m_apiBorderWindowPtr)
        //    api->refreshBorder(m_apiBorderWindowPtr);

        api->refreshWindow(m_apiWindowPtr);

        m_refreshNextFrame = false;
    }

}

