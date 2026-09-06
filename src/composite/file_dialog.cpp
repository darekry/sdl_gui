#include "file_dialog.hpp"
#include "../gui_manager.hpp"
#include "../gui.hpp"
#include "../layout.hpp"

#include "std.hpp"

namespace fs = std::filesystem;

// ============================================================================
// Static factory methods
// ============================================================================

FileDialog* FileDialog::createOpen(
    GUIManager& manager,
    std::string_view title,
    Callback callback,
    std::string_view startPath,
    std::string_view filter
) {
    int width = 640;
    int height = 450;
    int screenW = 0, screenH = 0;
    manager.getWindowSize(screenW, screenH);
    auto [x, y] = CenterRect(screenW, screenH, width, height);

    auto dialog = std::unique_ptr<FileDialog>(new FileDialog(
        manager, x, y, width, height,
        title, Mode::Open, callback
    ));

    auto* raw = dialog.get();
    manager.addElement(std::move(dialog));

    if (!startPath.empty()) {
        raw->setCurrentPath(startPath);
    } else {
        raw->setCurrentPath(fs::current_path().string());
    }

    raw->m_filter = filter;
    raw->refreshDirectories();
    raw->refreshFiles();

    return raw;
}

FileDialog* FileDialog::createSave(
    GUIManager& manager,
    std::string_view title,
    Callback callback,
    std::string_view startPath,
    std::string_view filter
) {
    int width = 640;
    int height = 450;
    int screenW = 0, screenH = 0;
    manager.getWindowSize(screenW, screenH);
    auto [x, y] = CenterRect(screenW, screenH, width, height);

    auto dialog = std::unique_ptr<FileDialog>(new FileDialog(
        manager, x, y, width, height,
        title, Mode::Save, callback
    ));

    auto* raw = dialog.get();
    manager.addElement(std::move(dialog));

    if (!startPath.empty()) {
        raw->setCurrentPath(startPath);
    } else {
        raw->setCurrentPath(fs::current_path().string());
    }

    raw->m_filter = filter;
    raw->refreshDirectories();
    raw->refreshFiles();

    return raw;
}

// ============================================================================
// Constructor
// ============================================================================

FileDialog::FileDialog(
    GUIManager& manager, int x, int y, int width, int height,
    std::string_view title, Mode mode,
    Callback callback
)
    : Panel(manager, x, y, width, height)
    , m_mode(mode)
    , m_callback(callback)
    , m_title(title)
{
    setClipChildren(false);
    setDraggable(true);

    // Style
    Style dialogStyle;
    dialogStyle.backgroundColor = {240, 240, 240, 255};
    dialogStyle.borderColor = {100, 100, 100, 255};
    dialogStyle.borderWidth = 2;
    dialogStyle.borderRadius = 0;
    setStyle(ElementState::Normal, dialogStyle);

    // --- Title label ---
    auto titleLabel = std::make_unique<Label>(manager, 10, 5, title);
    m_titleLabel = titleLabel.get();
    addChild(std::move(titleLabel));

    // --- Path label ---
    int pathY = m_titleBarHeight - 2;
    auto pathLabel = std::make_unique<Label>(manager, 10, pathY, "", 12);
    pathLabel->setTextColor(ElementState::Normal, {80, 80, 80, 255});
    m_pathLabel = pathLabel.get();
    addChild(std::move(pathLabel));

    // --- Directories grid ---
    int contentY = m_titleBarHeight + 10;
    int gridHeight = height - m_titleBarHeight - m_bottomBarHeight - 5;

    int dirLabelY = contentY - 5;
    auto dirLabel = std::make_unique<Label>(manager, 12, dirLabelY, "Directories", 13);
    dirLabel->setID("fd_dir_label");
    dirLabel->setTextColor(ElementState::Normal, {60, 60, 60, 255});
    addChild(std::move(dirLabel));

    int dirGridWidth = 200;
    int dirGridX = 10;
    int dirGridY = dirLabelY + 18;

    auto dirGrid = std::make_unique<StringGrid>(manager, dirGridX, dirGridY, dirGridWidth, gridHeight - 18, 1, 1);
    dirGrid->setShowColumnHeaders(false);
    dirGrid->setShowRowHeaders(false);
    dirGrid->setRowHeight(22);
    dirGrid->setEditable(false);
    dirGrid->setColumnWidth(0, dirGridWidth - 18);

    // Directory double-click navigates into that directory
    dirGrid->setOnCellDoubleClick([this](StringGrid*, CellCoord cell) {
        if (!cell.isValid()) return;
        std::string_view dirName = m_dirGrid->getCellText(cell.row, 0);
        if (dirName == "..") {
            navigateUp();
        } else {
            std::string newPath = m_currentPath + "/" + std::string(dirName);
            navigateTo(newPath);
        }
    });

    // Directory single click selects it but doesn't navigate
    dirGrid->setOnCellClick([this](StringGrid*, CellCoord cell) {
        if (!cell.isValid()) return;
        std::string_view dirName = m_dirGrid->getCellText(cell.row, 0);
        if (dirName != "..") {
            std::string dirPath = m_currentPath + "/" + std::string(dirName);
            m_selectedFile.clear();
            m_filenameInput->setText(std::string{});
            if (m_mode == Mode::Save) {
                // For save dialog, suggest the directory name
            }
        }
    });

    m_dirGrid = dirGrid.get();
    addChild(std::move(dirGrid));

    // --- Files grid ---
    int fileLabelY = contentY - 5;
    int fileGridX = dirGridX + dirGridWidth + 15;
    auto fileLabel = std::make_unique<Label>(manager, fileGridX, fileLabelY, "Files", 13);
    fileLabel->setID("fd_file_label");
    fileLabel->setTextColor(ElementState::Normal, {60, 60, 60, 255});
    addChild(std::move(fileLabel));

    int fileGridWidth = width - fileGridX - 15;
    int fileGridY = dirGridY;

    auto fileGrid = std::make_unique<StringGrid>(manager, fileGridX, fileGridY, fileGridWidth, gridHeight - 18, 1, 1);
    fileGrid->setShowColumnHeaders(false);
    fileGrid->setShowRowHeaders(false);
    fileGrid->setRowHeight(22);
    fileGrid->setEditable(false);
    fileGrid->setColumnWidth(0, fileGridWidth - 18);

    // File single click fills filename input
    fileGrid->setOnCellClick([this](StringGrid*, CellCoord cell) {
        if (!cell.isValid()) return;
        std::string_view filename = m_fileGrid->getCellText(cell.row, 0);
        onFileSelected(std::string(filename));
    });

    // File double click confirms selection (Open) or fills filename (Save)
    fileGrid->setOnCellDoubleClick([this](StringGrid*, CellCoord cell) {
        if (!cell.isValid()) return;
        std::string_view filename = m_fileGrid->getCellText(cell.row, 0);
        m_selectedFile = filename;
        m_filenameInput->setText(filename);
        if (m_mode == Mode::Open) {
            confirmSelection();
        }
    });

    m_fileGrid = fileGrid.get();
    addChild(std::move(fileGrid));

    // --- Filename input ---
    int bottomY = height - m_bottomBarHeight + 5;
    auto filenameLabel = std::make_unique<Label>(manager, 12, bottomY, "Filename:", 13);
    filenameLabel->setID("fd_filename_label");
    filenameLabel->setTextColor(ElementState::Normal, {60, 60, 60, 255});
    addChild(std::move(filenameLabel));

    int inputX = 80;
    int inputWidth = width - inputX - 140;
    auto filenameInput = std::make_unique<TextInput>(manager, inputX, bottomY - 2, inputWidth, 26);
    m_filenameInput = filenameInput.get();
    addChild(std::move(filenameInput));

    // --- Buttons ---
    int buttonWidth = 60;
    int buttonHeight = 28;
    int buttonY = bottomY + 35;

    auto cancelBtn = std::make_unique<Button>(manager,
        width - buttonWidth - 20, buttonY,
        buttonWidth, buttonHeight, "Cancel");
    cancelBtn->setOnClickCallback([this](GUIElement*) {
        close();
    });
    m_cancelBtn = cancelBtn.get();
    addChild(std::move(cancelBtn));

    const char* openLabel = (mode == Mode::Save) ? "Save" : "Open";
    auto openBtn = std::make_unique<Button>(manager,
        width - 2 * buttonWidth - 30, buttonY,
        buttonWidth, buttonHeight, openLabel);
    openBtn->setOnClickCallback([this](GUIElement*) {
        confirmSelection();
    });
    m_openBtn = openBtn.get();
    addChild(std::move(openBtn));

    layoutChildren();
}

// ============================================================================
// Navigation
// ============================================================================

void FileDialog::setCurrentPath(std::string_view path) {
    m_currentPath = path;
    m_pathLabel->setText(m_currentPath);
    m_titleLabel->setText(m_title + " - " + m_currentPath);
}

void FileDialog::navigateTo(const std::string& path) {
    std::error_code ec;
    if (!fs::is_directory(path, ec)) return;

    m_currentPath = fs::canonical(path, ec).string();
    if (ec) m_currentPath = path;

    setCurrentPath(m_currentPath);
    refreshDirectories();
    refreshFiles();
    m_selectedFile.clear();
    m_filenameInput->setText(std::string{});
}

void FileDialog::navigateUp() {
    if (m_currentPath.empty()) return;
    std::string parentPath = fs::path(m_currentPath).parent_path().string();
    if (parentPath.empty()) parentPath = "/";
    navigateTo(parentPath);
}

void FileDialog::refreshDirectories() {
    m_dirGrid->clear();
    m_dirGrid->setColumnCount(1);

    std::error_code ec;
    std::vector<std::string> dirs;
    for (const auto& entry : fs::directory_iterator(m_currentPath, ec)) {
        if (entry.is_directory(ec)) {
            dirs.push_back(entry.path().filename().string());
        }
    }

    // Sort case-insensitive
    std::sort(dirs.begin(), dirs.end(), [](const std::string& a, const std::string& b) {
        std::string la = a, lb = b;
        std::transform(la.begin(), la.end(), la.begin(), ::tolower);
        std::transform(lb.begin(), lb.end(), lb.begin(), ::tolower);
        return la < lb;
    });

    size_t totalRows = 1 + dirs.size();  // ".." + dirs
    m_dirGrid->setRowCount(totalRows);

    m_dirGrid->setCellText(0, 0, "..");

    for (size_t i = 0; i < dirs.size(); ++i) {
        m_dirGrid->setCellText(i + 1, 0, dirs[i]);
    }
}

void FileDialog::refreshFiles() {
    m_fileGrid->clear();
    m_fileGrid->setColumnCount(1);

    std::error_code ec;
    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(m_currentPath, ec)) {
        if (entry.is_regular_file(ec)) {
            std::string filename = entry.path().filename().string();
            if (matchesFilter(filename)) {
                files.push_back(filename);
            }
        }
    }

    std::sort(files.begin(), files.end(), [](const std::string& a, const std::string& b) {
        std::string la = a, lb = b;
        std::transform(la.begin(), la.end(), la.begin(), ::tolower);
        std::transform(lb.begin(), lb.end(), lb.begin(), ::tolower);
        return la < lb;
    });

    size_t totalRows = files.size();
    if (totalRows == 0) totalRows = 1;  // empty grid looks bad with 0 rows

    m_fileGrid->setRowCount(totalRows);

    for (size_t i = 0; i < files.size(); ++i) {
        m_fileGrid->setCellText(i, 0, files[i]);
    }
}

void FileDialog::onFileSelected(const std::string& filename) {
    m_selectedFile = filename;
    m_filenameInput->setText(filename);
}

void FileDialog::confirmSelection() {
    std::string filename = m_filenameInput->getText();

    if (filename.empty()) {
        filename = m_selectedFile;
    }

    if (filename.empty()) return;

    // Build full path
    std::string fullPath;
    if (fs::path(filename).is_absolute()) {
        fullPath = filename;
    } else {
        if (m_currentPath == "/") {
            fullPath = "/" + filename;
        } else {
            fullPath = m_currentPath + "/" + filename;
        }
    }

    if (m_mode == Mode::Save) {
        // For save dialog, just pass the path
        if (m_callback) m_callback(fullPath);
        close();
    } else {
        // For open dialog, verify file exists
        std::error_code ec;
        if (fs::exists(fullPath, ec) && fs::is_regular_file(fullPath, ec)) {
            if (m_callback) m_callback(fullPath);
            close();
        }
    }
}

bool FileDialog::matchesFilter(const std::string& filename) const {
    if (m_filter == "*" || m_filter == "*.*") return true;

    // Simple wildcard matching for *.ext patterns
    if (m_filter.size() > 2 && m_filter[0] == '*' && m_filter[1] == '.') {
        std::string ext = m_filter.substr(1);  // ".ext"
        if (filename.size() >= ext.size()) {
            std::string fileExt = filename.substr(filename.size() - ext.size());
            std::string lowerFileExt = fileExt, lowerExt = ext;
            std::transform(lowerFileExt.begin(), lowerFileExt.end(), lowerFileExt.begin(), ::tolower);
            std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
            return lowerFileExt == lowerExt;
        }
        return false;
    }

    return true;
}

void FileDialog::close() {
    m_isOpen = false;
    markForDeletion();
}

void FileDialog::layoutChildren() {
    // Geometria proporcjonalna do bieżącego rozmiaru (LayoutPass — dawniej
    // tylko matematyka w ctorze, łamana przy każdym resize).
    const int contentY = m_titleBarHeight + 10;
    const int gridHeight = m_height - m_titleBarHeight - m_bottomBarHeight - 5;
    const int labelY = contentY - 5;
    const int gridY = labelY + 18;
    const int gridH = gridHeight - 18;

    constexpr int dirGridWidth = 200;
    constexpr int dirGridX = 10;
    const int fileGridX = dirGridX + dirGridWidth + 15;
    const int fileGridWidth = m_width - fileGridX - 15;

    for (const auto& child : getChildren()) {
        const std::string_view id = child->getID();
        if (id == "fd_dir_label") child->setPosition(12, labelY);
        else if (id == "fd_file_label") child->setPosition(fileGridX, labelY);
        else if (id == "fd_filename_label") child->setPosition(12, m_height - m_bottomBarHeight + 5);
    }

    if (m_dirGrid) {
        m_dirGrid->setPosition(dirGridX, gridY);
        m_dirGrid->setSize(dirGridWidth, gridH);
        m_dirGrid->setColumnWidth(0, dirGridWidth - 18);
    }
    if (m_fileGrid) {
        m_fileGrid->setPosition(fileGridX, gridY);
        m_fileGrid->setSize(fileGridWidth, gridH);
        m_fileGrid->setColumnWidth(0, fileGridWidth - 18);
    }
    if (m_filenameInput) {
        const int bottomY = m_height - m_bottomBarHeight + 5;
        constexpr int inputX = 80;
        m_filenameInput->setPosition(inputX, bottomY - 2);
        m_filenameInput->setSize(m_width - inputX - 140, 26);
    }

    // Pasek przycisków: StackLayout wyrównany do prawej.
    if (m_openBtn && m_cancelBtn) {
        const int buttonY = m_height - m_bottomBarHeight + 40;
        const StackLayout strip(StackLayout::Direction::Horizontal, 10,
                                0, 0, 20, 0, StackLayout::Align::End);
        strip.arrangeStrip(std::vector<GUIElement*>{m_openBtn, m_cancelBtn}, m_width, buttonY);
    }
}

void FileDialog::draw(SDL_Renderer* renderer) {
    drawTitleBar(renderer, m_x, m_y, m_width, m_titleBarHeight);

    // Panel background and border
    Panel::draw(renderer);
}

bool FileDialog::handleEvent(const SDL_Event& e) {
    if (!m_isOpen || !m_visible) return false;

    if (Panel::handleEvent(e)) return true;

    // Enter key in filename input confirms selection
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_RETURN) {
        confirmSelection();
        return true;
    }

    // ESC closes dialog
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) {
        close();
        return true;
    }

    return false;
}

ComponentType FileDialog::getComponentTypeId() const {
    return ComponentType::FileDialog;
}
