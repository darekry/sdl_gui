# FileDialog - Design Document

## Wizja

FileDialog w stylu Windows 3.11:
- **Lewa strona**: Drzewo katalogów (TreeView) - hierarchiczna nawigacja
- **Prawa strona**: Lista plików (ListView) - wybór plików w wybranym katalogu
- **Bottom**: TextInput (ścieżka/pattern) + Buttons (OK/Cancel)

![Layout](layout示意图)

```
┌─────────────────────────────────────────────────────────────────────┐
│ File Dialog                                              [_][□][X] │
├─────────────────────┬───────────────────────────────────────────────┤
│ Directories         │ Files                                         │
│ ┌───────────────────┐ ┌─────────────────────────────────────────────┐│
│ │ ▼ C:              │ │ ┌─────────────────────────────────────────┐ ││
│ │   ▼ Documents     │ │ │ document.txt                12 KB   TXT │ ││
│ │     → Work        │ │ │ image.png                   45 KB   PNG │ ││
│ │       Project     │ │ │ readme.md                    3 KB   MD  │ ││
│ │     Personal      │ │ └─────────────────────────────────────────┘ ││
│ │   ▼ Downloads     │ │                                              ││
│ │     Archive       │ │                                              ││
│ │   Programs        │ │                                              ││
│ └───────────────────┘ └─────────────────────────────────────────────┘│
│                     │                                               ││
├─────────────────────┴───────────────────────────────────────────────┤
│ Path: C:\Documents\Work\                    Filter: *.txt            │
│                     [ Open ]               [ Cancel ]                 │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Wymagane nowe widgety

### 1. TreeView (NEW) - hierarchiczny widok

Najważniejszy nowy widget dla FileDialog.

#### Struktura danych:

```cpp
struct TreeNode {
    std::string text;           // Display text
    std::string path;           // Full path (filesystem)
    std::vector<TreeNode> children;
    bool expanded = false;      // Is node expanded?
    bool hasChildren = false;   // Has potential children (lazy load)
    bool loaded = false;        // Children already loaded?
    int depth = 0;              // Indentation level
    void* userData = nullptr;   // Optional user data
};
```

#### API:

```cpp
class TreeView : public GUIElement {
public:
    TreeView(GUIManager& manager, int x, int y, int width, int height);
    
    // Root nodes
    void setRootNodes(std::vector<TreeNode> nodes);
    void addRootNode(TreeNode node);
    void clearNodes();
    
    // Node operations
    void expandNode(size_t index);
    void collapseNode(size_t index);
    void toggleNode(size_t index);
    void selectNode(size_t index);
    [[nodiscard]] std::optional<size_t> getSelectedNode() const;
    
    // Lazy loading
    void setLazyLoadCallback(std::function<std::vector<TreeNode>(const TreeNode&)> callback);
    
    // Callbacks
    void setOnNodeSelect(std::function<void(const TreeNode&)> callback);
    void setOnNodeExpand(std::function<void(const TreeNode&)> callback);
    void setOnNodeDoubleClick(std::function<void(const TreeNode&)> callback);
    
    // Visual
    void setShowIcons(bool show);
    void setIndentWidth(int pixels);
    void setNodeHeight(int pixels);
    
    // Existing overrides
    bool handleEvent(const SDL_Event& e) override;
    const char* getComponentType() const override;
    
protected:
    void draw(SDL_Renderer* renderer) override;
    bool wantsDirectRender() const override { return true; }
    void drawDirect(SDL_Renderer* renderer) override;
    
private:
    std::vector<TreeNode> m_nodes;
    std::vector<size_t> m_visibleNodes;  // Flattened visible nodes (expanded ones included)
    size_t m_selectedIndex = SIZE_MAX;
    int m_vScrollOffset = 0;
    Slider* m_vSlider = nullptr;
    int m_indentWidth = 20;
    int m_nodeHeight = 24;
    
    void flattenVisibleNodes();
    void loadNodeChildren(TreeNode& node);
    int getTotalContentHeight() const;
    void updateSliderRange();
    TreeNode* getNodeByIndex(size_t index);
};
```

#### Renderowanie:

```
▶ Folder        (collapsed, has children)
▼ Folder        (expanded, children visible)
  ▶ Subfolder   (indented child, collapsed)
  ■ File        (leaf node)
→ Selected      (selected node marker)
```

---

## Skład FileDialog

### Component Composition:

```cpp
class FileDialog : public Panel {
public:
    // Main factory methods
    static std::unique_ptr<FileDialog> createOpen(
        GUIManager& manager,
        std::string_view title = "Open File",
        std::string_view initialPath = "",
        std::string_view filter = "*.*",
        std::function<void(std::string)> onSelect = nullptr
    );
    
    static std::unique_ptr<FileDialog> createSave(
        GUIManager& manager,
        std::string_view title = "Save File",
        std::string_view initialPath = "",
        std::string_view defaultName = "",
        std::string_view filter = "*.*",
        std::function<void(std::string)> onSelect = nullptr
    );
    
private:
    // Layout regions
    Panel* m_leftPanel = nullptr;      // Directory tree container
    Panel* m_rightPanel = nullptr;     // File list container
    TreeView* m_dirTree = nullptr;     // Directory tree widget
    ListView* m_fileList = nullptr;    // File list widget
    TextInput* m_pathInput = nullptr;  // Current path
    TextInput* m_filterInput = nullptr;// Filter pattern
    Label* m_pathLabel = nullptr;      // "Path:" label
    Label* m_filterLabel = nullptr;    // "Filter:" label
    Button* m_openBtn = nullptr;       // Open/Save button
    Button* m_cancelBtn = nullptr;     // Cancel button
    
    // State
    std::filesystem::path m_currentPath;
    std::string m_filterPattern;
    std::string m_selectedFile;
    bool m_isSaveDialog = false;
    
    // Callback
    std::function<void(std::string)> m_onSelect;
};
```

### Layout dimensions:

```
Dialog width:  600px
Dialog height: 450px

Left panel:    200px width
Right panel:   380px width (600 - 200 - padding)
Splitter:      20px (optional draggable)

Bottom bar:    80px height
   - Path input:    300px
   - Filter input:  150px
   - Buttons:       100px each, 40px height
```

---

## Implementacja - Fazy

### Phase 1: TreeView Widget

Najważniejszy - nowy widget dla hierarchicznej nawigacji.

#### Pliki:
- `src/tree_view.hpp`
- `src/tree_view.cpp`
- `examples/example_tree_view.cpp`
- `tests/test_tree_view.cpp`

#### Trudności:
- **Indentacja**: Każdy node ma depth level
- **Expand/Collapse**: Dynamiczne aktualizowanie visible nodes
- **Lazy loading**: Filesystem directories → load on expand
- **Scrollbar**: Dynamiczne range changes

#### Key algorithms:

```cpp
void TreeView::flattenVisibleNodes() {
    m_visibleNodes.clear();
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        flattenNodeRecursive(m_nodes[i], i);
    }
    updateSliderRange();
}

void TreeView::flattenNodeRecursive(const TreeNode& node, size_t index) {
    m_visibleNodes.push_back(index);
    if (node.expanded && node.loaded) {
        for (const auto& child : node.children) {
            flattenNodeRecursive(child, index);  // Or use absolute indices
        }
    }
}
```

---

### Phase 2: FileDialog

Integracja TreeView + ListView + filesystem.

#### Pliki:
- `src/composite/file_dialog.hpp`
- `src/composite/file_dialog.cpp`
- `examples/example_file_dialog.cpp`

#### Filesystem integration:

```cpp
// Load directories for TreeView
std::vector<TreeNode> loadDirectoryTree(const std::filesystem::path& root) {
    std::vector<TreeNode> nodes;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(root)) {
            if (entry.is_directory()) {
                TreeNode node;
                node.text = entry.path().filename().string();
                node.path = entry.path().string();
                node.hasChildren = true;  // Assume has children (lazy load)
                node.loaded = false;
                node.depth = 0;
                nodes.push_back(std::move(node));
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // Handle permission errors, etc.
    }
    return nodes;
}

// Load files for ListView (on directory select)
void FileDialog::loadFiles(const std::filesystem::path& dir) {
    m_fileList->clearItems();
    try {
        std::string pattern = m_filterPattern;
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (matchesPattern(filename, pattern)) {
                    m_fileList->addItem(filename);
                }
            }
        }
    } catch (...) {
        // Handle error
    }
}
```

#### Pattern matching:

```cpp
bool matchesPattern(std::string_view filename, std::string_view pattern) {
    if (pattern == "*.*" || pattern.empty()) return true;
    
    // Simple glob: *.txt, *.png, etc.
    if (pattern.starts_with("*.")) {
        std::string_view ext = pattern.substr(2);
        return filename.ends_with(ext);
    }
    
    // Exact match
    return filename == pattern;
}
```

---

### Phase 3: Drives/Root paths

Na Windows/Linux różne root paths:

```cpp
// Windows: C:\, D:\, etc.
// Linux: /, /home, /mnt, etc.

std::vector<TreeNode> getRootPaths() {
    std::vector<TreeNode> roots;
    
#ifdef _WIN32
    // Get drive letters
    DWORD drives = GetLogicalDrives();
    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if (drives & (1 << (letter - 'A'))) {
            TreeNode node;
            node.text = std::string(1, letter) + ":";
            node.path = node.text + "\\";
            node.hasChildren = true;
            roots.push_back(std::move(node));
        }
    }
#else
    // Linux: start with /
    TreeNode root;
    root.text = "/";
    root.path = "/";
    root.hasChildren = true;
    roots.push_back(root);
    
    // Common paths
    TreeNode home;
    home.text = "Home";
    home.path = std::filesystem::path(getenv("HOME")).string();
    home.hasChildren = true;
    roots.push_back(home);
#endif
    
    return roots;
}
```

---

## Opcjonalne funkcje

### Draggable Splitter

User może resize left/right panels:

```cpp
class Splitter : public GUIElement {
    // Vertical/horizontal divider
    // Drag to resize adjacent panels
};
```

### File Details Columns

StringGrid zamiast ListView dla więcej info:

```
┌─────────────────────────────────────────────────┐
│ Name          │ Size     │ Type │ Modified      │
├───────────────┼──────────┼──────┼───────────────│
│ document.txt  │ 12 KB    │ TXT  │ 2026-05-20    │
│ image.png     │ 45 KB    │ PNG  │ 2026-05-19    │
└─────────────────────────────────────────────────┘
```

### File Icons

Show icons based on extension:

```cpp
TextureManager::loadTexture("assets/icons/folder.png");
TextureManager::loadTexture("assets/icons/file_txt.png");
TextureManager::loadTexture("assets/icons/file_png.png");
```

### Recent/Favorite Paths

Quick access panel:

```cpp
void FileDialog::addFavorite(const std::string& path);
std::vector<std::string> getFavorites();
```

---

## Integration z Makefile

TreeView jako główny widget (nie composite):

```makefile
HPP_SOURCES := \
    ...
    $(SRC)/tree_view.hpp \
    ...
    $(SRC)/composite/file_dialog.hpp \
    ...
```

---

## Estymacja złożoności

| Komponent | Trudność | Czas est. | Dependencies |
|-----------|----------|-----------|--------------|
| **TreeView** | Hard | 4-6h | None |
| TreeView tests | Medium | 2h | TreeView |
| FileDialog | Hard | 3-4h | TreeView, ListView, filesystem |
| Filesystem integration | Medium | 2h | C++17 std::filesystem |
| Pattern matching | Easy | 30min | None |
| FileDialog example | Easy | 30min | FileDialog |
| **Total** | | **~12h** | |

---

## Next Steps

1. **Implement TreeView first** - independent widget, reusable
2. **Create example_tree_view** - test basic functionality
3. **Implement FileDialog** - use TreeView + ListView
4. **Add filesystem integration** - std::filesystem
5. **Create example_file_dialog** - full demonstration

---

## Minimal viable FileDialog (MVP)

Jeśli TreeView jest zbyt complex na początek, można użyć ListView dla directories (flat list, navigate by double-click):

```cpp
// MVP approach - ListView for directories
class FileDialogMVP : public Panel {
    ListView* m_dirList;   // Flat directory list
    ListView* m_fileList;  // File list
    // Navigate up/down by double-click
};
```

To pozwala na functional FileDialog bez TreeView, ale UX jest gorszy (nie widoczna hierarchia).