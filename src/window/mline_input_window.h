#ifndef __MLINE_INPUT_WINDOW
#define __MLINE_INPUT_WINDOW

#include "file_buffer_window.h"
#include "../utils/prefix_tree.h"
#include "../callbacks.h"

//
class MLineInputWindow : public FileBufferWindow
{
public:
    MLineInputWindow(const frame_t &_frame, const std::string &_id, int _wnd_params);
    ~MLineInputWindow();

    // MLineInputWindow-specific functions
    virtual void pushCharToInput(char _c);
    virtual void popCharFromInput();
    virtual void autocompleteInput();
    virtual void findCompletions() = 0;


protected:
    //
    __always_inline void clear_input_()
    {
        if (m_inputLine->len == 0)
            return;
        
        delete m_inputLine;
        m_inputLine = create_line("");
    }

protected:
    std::string m_header = "";
    std::string m_errorMsg = "";
    
    prefix_node_t *m_ptree = NULL;
    std::string m_autocompleteLine = "";
    std::vector<std::string> m_autocompletions;

    frame_t m_renderFrame = frame_t(0);
    ivec2_t m_listingOffset = ivec2_t(0);

    std::string m_inputPrompt = "";
    ivec2_t m_inputFieldOffset = ivec2_t(0);    // cursor offset in input field
    line_t *m_inputLine;            // m_currentLine is pointing into the line buffer,
                                    // this is for keyboard input (e.g. filename)

    std::vector<std::string> m_keys;    // internal representation, different from the listing
    std::vector<std::string> m_values;  // searchable entries, same ordering as m_keys

    // callback on <ENTER> and selection is valid
    WindowCallback m_callback = nullptr;
    
};

#endif // __MLINE_INPUT_WINDOW
