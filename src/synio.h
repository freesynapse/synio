#ifndef __SYNIO_H
#define __SYNIO_H

#include "utils/utils.h"
#include "window/file_buffer_window.h"
#include "window/command_window.h"
#include "window/status_window.h"
#include "window/listbox_window.h"

//
class Synio
{
public:
    Synio(const std::string &_filename);
    ~Synio();

    // some functions
    void initialize();
    CommandWindow *newCommandWindow();
    void deleteCommandWindow(Event *_e);
    
    FileBufferWindow *newFileBufferWindow(const std::string &_filename);
    void deleteFileBufferWindow(Event *_e);

    void adjustBufferWindowFrameY(int _dy);

    void refreshBufferWindow() { clear_redraw_refresh_window_ptr_(m_currentBufferWindow); }

    // event callbacks
    void onExitEvent(Event *_e);
    void onAdjustBufferWindowEvent(Event *_e);

    // accessor
    FileBufferWindow *currentBufferWindow() { return m_currentBufferWindow; }
    void setCurrentBufferWindow(FileBufferWindow *_w) { m_currentBufferWindow = _w; }
    const std::unordered_map<std::string, FileBufferWindow *> &openBufferWindows() { return m_bufferWindows; }
    const frame_t &bufferWndFrame() { return m_bufferWndFrame; }

    //
    void mainLoop();

private:
    void clear_redraw_refresh_window_ptr_(BufferWindowBase *_w)
    {
        if (m_focusedWindow == NULL) 
            return;
            
        _w->clear_next_frame_();
        _w->clear();
        _w->redraw();
        _w->updateCursor();
        _w->refresh_next_frame_();
        _w->refresh();
        api->redrawScreen();
    }

    //
    void adjustBufferWindowFrame(BufferWindowBase *_w, frame_t *_w_frame, int _dx0, 
                                 int _dy0, int _dx1, int _dy1)
    {
        _w_frame->v0.x = MAX(_w_frame->v0.x + _dx0, 0);
        _w_frame->v0.y = MAX(_w_frame->v0.y + _dy0, 0);
        _w_frame->v1.x = MAX(_w_frame->v1.x + _dx1, 1);
        _w_frame->v1.y = MAX(_w_frame->v1.y + _dy1, 1);
        _w_frame->update_dims();

        _w->resize(*_w_frame);
        clear_redraw_refresh_window_ptr_(_w);
    }

private:
    // flags
    bool m_shouldClose = false;
    bool m_commandMode = false;
    
    // windows
    //

    // map of all open buffers
    std::unordered_map<std::string, FileBufferWindow *> m_bufferWindows;
    std::vector<std::string> m_bufferQueue;

    FileBufferWindow *m_currentBufferWindow = NULL; // current displayed file window
    std::string m_currentFilename = "";
    StatusWindow *m_statusWindow = NULL;            // updated by the current file buffer
    CommandWindow *m_commandWindow = NULL;          // created on demand

    BufferWindowBase *m_focusedWindow = NULL;       // status, command, dialog, buffer, etc
    
    // window sizes
    frame_t m_bufferWndFrame;
    frame_t m_commandWndFrame;
    frame_t m_statusWndFrame;

    //
    ivec2_t m_screenSize;
    
};


#endif // __SYNIO_H
