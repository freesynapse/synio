
#include "status_window.h"

#include "file_buffer_window.h"
#include "../utils/str_utils.h"


//
void StatusWindow::update(FileBufferWindow *_buffer_window)
{
    // update from current buffer window
    if (_buffer_window != NULL)
    {
        m_filename = _buffer_window->fileName();
        m_filetype = _buffer_window->fileType();
        m_lineCount = _buffer_window->lineCount();
        _buffer_window->updateBufferCursorPos();
        m_bpos = _buffer_window->bufferCursorPos();
    
        // left-aligned status
        bool dirty = _buffer_window->bufferChanged();
        std::string filename = (dirty ? "*" : "") + m_filename + (dirty ? "*" : "");
        m_statusStr = " " + filename;

        // right-aligned status
        std::string right = "(" + m_filetype + ")  [" + std::to_string(m_bpos.x+1) + ", " + \
                            std::to_string(m_bpos.y+1) + "]  (" + \
                            to_string_n(((float)(m_bpos.y + 1) / m_lineCount) * 100.0f, 0) + "%) ";
        
        size_t w = (size_t)m_frame.ncols;
        std::string spacing = std::string(w - m_statusStr.size() - right.size(), ' ');

        m_statusStr += spacing;
        m_statusStr += right;

        m_wasUpdated = true;
    }
    
}

//---------------------------------------------------------------------------------------
void StatusWindow::resize(frame_t _new_frame)
{
    m_frame = _new_frame;
    api->clearWindow(m_apiWindowPtr);
    api->deleteWindow(m_apiWindowPtr);
    m_apiWindowPtr = api->newWindow(&m_frame);  // resfresh called by api->newWindow()

}

//---------------------------------------------------------------------------------------
void StatusWindow::redraw()
{
    //
    CHTYPE_STR cstr(m_statusStr);

    api->enableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_STATUS));
    api->printString(m_apiWindowPtr, 0, 0, cstr.str, cstr.len);
    api->disableAttr(m_apiWindowPtr, COLOR_PAIR(SYN_COLOR_STATUS));
    
    // reset for next frame
    m_wasUpdated = false;

}

