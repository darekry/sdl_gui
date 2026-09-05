#pragma once

#include "../gui.hpp"
#include "../panel.hpp"
#include "../button.hpp"
#include "../label.hpp"
#include "../text_input.hpp"
#include "../string_grid.hpp"

#include "std.hpp"

/**
 * @brief FileDialog - composite file picker dialog
 *
 * Layout:
 *   +---------------------------------------------+
 *   | Title bar: current path                      |
 *   +---------------------+-----------------------+
 *   | Directories         | Files                 |
 *   | [..]                | [file1.txt]           |
 *   | [subdir/]           | [file2.cpp]           |
 *   | [...]               | [...]                 |
 *   +---------------------+-----------------------+
 *   | Filename: [________________]                 |
 *   +---------------------------------------------+
 *   |                        [Open] [Cancel]       |
 *   +---------------------------------------------+
 *
 * Directories panel on the left, files panel on the right.
 * ".." entry at the top of directories list navigates to parent directory.
 */

class FileDialog : public Panel {
public:
    enum class Mode { Open, Save };

    using Callback = std::function<void(const std::string& path)>;

    static FileDialog* createOpen(
        GUIManager& manager,
        std::string_view title,
        Callback callback,
        std::string_view startPath = {},
        std::string_view filter = "*.*"
    );

    static FileDialog* createSave(
        GUIManager& manager,
        std::string_view title,
        Callback callback,
        std::string_view startPath = {},
        std::string_view filter = "*.*"
    );

    FileDialog(
        GUIManager& manager, int x, int y, int width, int height,
        std::string_view title, Mode mode,
        Callback callback
    );

    void setCurrentPath(std::string_view path);
    [[nodiscard]] const std::string& getCurrentPath() const { return m_currentPath; }
    [[nodiscard]] const std::string& getSelectedFile() const { return m_selectedFile; }

    void close();
    [[nodiscard]] bool isOpen() const { return m_isOpen; }

    [[nodiscard]] ComponentType getComponentTypeId() const override;
    [[nodiscard]] bool isOverlay() const override { return true; }

protected:
    void draw(SDL_Renderer* renderer) override;
    bool handleEvent(const SDL_Event& e) override;

private:
    void refreshDirectories();
    void refreshFiles();
    void navigateTo(const std::string& path);
    void navigateUp();
    void onFileSelected(const std::string& filename);
    void confirmSelection();
    bool matchesFilter(const std::string& filename) const;

    // Widget pointers
    StringGrid* m_dirGrid = nullptr;
    StringGrid* m_fileGrid = nullptr;
    Label* m_pathLabel = nullptr;
    Label* m_titleLabel = nullptr;
    TextInput* m_filenameInput = nullptr;
    Button* m_openBtn = nullptr;
    Button* m_cancelBtn = nullptr;

    Mode m_mode;
    Callback m_callback;
    std::string m_currentPath;
    std::string m_selectedFile;
    std::string m_filter;
    std::string m_title;
    bool m_isOpen = true;

    int m_titleBarHeight = 30;
    int m_bottomBarHeight = 80;
};
