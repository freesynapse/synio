#ifndef __FILE_EXPLORER_WINDOW
#define __FILE_EXPLORER_WINDOW

#include <filesystem>

#include "mline_input_window.h"
#include "../types.h"
#include "../utils/prefix_tree.h"
#include "../callbacks.h"


//
class FileExplorerWindow : public MLineInputWindow
{
public:
    FileExplorerWindow(const frame_t &_frame, 
                       const std::string &_id, 
                       int _wnd_params=WPARAMS_BORDER,
                       WindowCallback _callback=nullptr,
                       const std::string &_input_prompt="Filename: ");
    ~FileExplorerWindow() = default;

    // overrides
    virtual void handleInput(int _c, CtrlKeyAction _ctrl_action) override;
    virtual void redraw() override;
    virtual void moveCursorToLineBegin() override { moveCursorColumn(-m_currentCol); }
    virtual void moveCursorToLineEnd() override { moveCursorColumn(m_ncols - 1 - m_currentCol); }
    virtual void findCompletions() override;
    
    // FileExplorerWindow-only functions
    void moveCursorColumn(int _dcol=0);
    void moveToColRow(int _col, int _row);
    void getCurrentDirContents(bool _reset_error=true);

    // accessors
    const std::string &getFilename() { return m_selectedFilename; }


private:
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

    // rendering
    int m_nrows = 0;
    int m_ncols = 0;
    int m_colWidth = 0;
    int m_currentCol = 0;

};



#endif // __FILE_EXPLORER_WINDOW
