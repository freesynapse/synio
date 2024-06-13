#ifndef __FILE_EXPLORER_WINDOW
#define __FILE_EXPLORER_WINDOW

#include "file_buffer_window.h"
#include "../types.h"


//
class FileExplorerWindow : public FileBufferWindow
{
public:
    FileExplorerWindow(const frame_t &_frame, 
                       const std::string &_id, 
                       bool _border=false,
                       const std::string &_input_prompt="Filename: ");
    ~FileExplorerWindow() = default;

    // overrides
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void redraw() override;

    // FileExplorerWindow-only functions
    void getCurrentDirContents();
    const std::string &getFilename() { return m_selectedFilename; }

private:
    __always_inline void toggle_mode_()
    {
        m_isBrowsing = !m_isBrowsing;
        ivec2_t pcpos = m_prevModeCPos;
        m_prevModeCPos = m_cursor.cpos();

        if (m_isBrowsing)   m_cursor.set_offset({ 0 });
        else                m_cursor.set_offset(m_inputFieldOffset);

        m_cursor.set_cpos(pcpos);
    }

private:
    // files etc
    std::string m_currentDir = "/bin/";
    // std::string m_currentDir = ".";
    std::string m_selectedFilename = "";
    std::vector<std::string> m_currentDirListing;
    
    // rendering
    line_t *m_currentListingPtr; // current line in dir listing
    frame_t m_renderFrame;
    size_t m_renderYPos = 0;     // scroll position

    // different modes; input and browsing files
    bool m_isBrowsing;
    ivec2_t m_prevModeCPos;
    
    // input field 
    std::string m_inputPrompt;
    ivec2_t m_inputFieldOffset;             // cursor offset when in this field
    

};



#endif // __FILE_EXPLORER_WINDOW
