#ifndef __FILE_EXPLORER_WINDOW
#define __FILE_EXPLORER_WINDOW

#include <filesystem>

#include "file_buffer_window.h"
#include "../types.h"
#include "../utils/prefix_tree.h"


//
class FileExplorerWindow : public FileBufferWindow
{
public:
    FileExplorerWindow(const frame_t &_frame, 
                       const std::string &_id, 
                       bool _border=false,
                       const std::string &_input_prompt="Filename: ");
    ~FileExplorerWindow();

    // overrides
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void redraw() override;
    virtual void moveCursorToLineBegin() override { moveCursorColumn(-m_currentCol); }
    virtual void moveCursorToLineEnd() override { moveCursorColumn(m_ncols - 1 - m_currentCol); }
    // virtual void insertCharAtCursor(char _c) override;
    
    // FileExplorerWindow-only functions
    void pushCharToInput(char _c);
    void popCharFromInput();
    void moveCursorColumn(int _dcol=0);
    void moveToColRow(int _col, int _row);
    void autocompleteInput();
    void findCompletions();

    void getCurrentDirContents(bool _reset_error=true);

    // accessors
    const std::string &getFilename() { return m_selectedFilename; }


private:
    //
    __always_inline void clear_input_()
    {
        if (m_inputLine->len == 0)
            return;
        
        delete m_inputLine;
        m_inputLine = create_line("");
    }
    //
    __always_inline int get_listing_offset_from_x_(int _x)
    {
        updateBufferCursorPos();

        int col = get_column_from_x_(_x);
        int offset = col * m_nrows + m_bufferCursorPos.y;

        return offset;
    }

    //
    __always_inline int get_column_from_x_(int _x) { return _x / m_colWidth; }


private:
    // files etc
    std::filesystem::path m_currentPath;
    std::string m_selectedFilename = "";
    std::vector<FileEntry> m_currentDirListing;
    std::string m_errorMsg = "";
    
    // autocomplete for file names
    prefix_node_t *m_dirPTree = NULL;
    std::string m_autocompleteLine = "";
    std::vector<std::string> m_autocompletions;
    
    // rendering
    frame_t m_renderFrame;          // accounting for input line at the bottom
    int m_nrows;
    int m_ncols;
    int m_colWidth;
    int m_currentCol = 0;

    // state-dependent variables 
    std::string m_inputPrompt;
    ivec2_t m_inputFieldOffset;     // cursor offset in input field
    line_t *m_inputLine;            // m_currentLine is pointing into the line buffer,
                                    // this is for keyboard input (e.g. filename)
    

};



#endif // __FILE_EXPLORER_WINDOW
