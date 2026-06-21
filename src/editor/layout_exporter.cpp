#include "layout_exporter.hpp"
#include "../../lib/tinyxml2.h"

#include "../constants.hpp"
#include "std.hpp"

bool LayoutExporter::saveToXML(const std::vector<EditorElement>& elements, const std::string& filePath) {
    tinyxml2::XMLDocument doc;
    
    tinyxml2::XMLElement* layout = doc.NewElement("Layout");
    doc.InsertFirstChild(layout);
    
    tinyxml2::XMLElement* resources = doc.NewElement("Resources");
    layout->InsertFirstChild(resources);
    
    tinyxml2::XMLElement* font = doc.NewElement("Font");
    font->SetAttribute("path", constants::kDefaultFontPath);
    font->SetAttribute("size", "16");
    resources->InsertFirstChild(font);
    
    auto rootIndices = getRootIndices(elements);
    for (size_t rootIdx : rootIndices) {
        addElementToXML(&doc, layout, elements[rootIdx], elements);
    }
    
    tinyxml2::XMLError error = doc.SaveFile(filePath.c_str());
    return error == tinyxml2::XML_SUCCESS;
}

bool LayoutExporter::saveToJSON(const std::vector<EditorElement>& elements, const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{\n";
    file << "  \"resources\": {\n";
    file << "    \"fonts\": [{\"path\": \"" << constants::kDefaultFontPath << "\", \"size\": 16}]\n";
    file << "  },\n";
    
    auto rootIndices = getRootIndices(elements);
    if (rootIndices.empty()) {
        file << "  \"root\": null\n";
    } else if (rootIndices.size() == 1) {
        file << "  \"root\": " << elementToJSON(elements[rootIndices[0]], elements) << "\n";
    } else {
        file << "  \"root\": {\n";
        file << "    \"type\": \"Panel\",\n";
        file << "    \"id\": \"rootPanel\",\n";
        file << "    \"x\": 0, \"y\": 0,\n";
        file << "    \"width\": 800, \"height\": 600,\n";
        file << "    \"children\": [\n";
        
        for (size_t i = 0; i < rootIndices.size(); ++i) {
            if (i > 0) file << ",\n";
            file << "      " << elementToJSON(elements[rootIndices[i]], elements);
        }
        file << "\n    ]\n";
        file << "  }\n";
    }
    
    file << "}\n";
    file.close();
    
    return true;
}

void LayoutExporter::addElementToXML(void* docPtr, void* parentPtr, const EditorElement& element, const std::vector<EditorElement>& allElements) {
    auto* doc = static_cast<tinyxml2::XMLDocument*>(docPtr);
    auto* parent = static_cast<tinyxml2::XMLElement*>(parentPtr);
    
    tinyxml2::XMLElement* elemNode = doc->NewElement(element.type.c_str());
    
    elemNode->SetAttribute("id", element.id.c_str());
    elemNode->SetAttribute("x", element.x);
    elemNode->SetAttribute("y", element.y);
    elemNode->SetAttribute("width", element.width);
    elemNode->SetAttribute("height", element.height);
    
    for (const auto& [key, value] : element.properties) {
        if (isNumericAttribute(key, value)) {
            elemNode->SetAttribute(key.c_str(), std::stoi(value));
        } else if (isBoolAttribute(key)) {
            elemNode->SetAttribute(key.c_str(), value == "true");
        } else {
            elemNode->SetAttribute(key.c_str(), value.c_str());
        }
    }
    
    for (const auto& [state, style] : element.styles) {
        tinyxml2::XMLElement* styleNode = doc->NewElement("Style");
        styleNode->SetAttribute("state", elementStateToString(state).c_str());
        
        if (style.backgroundColor) {
            styleNode->SetAttribute("backgroundColor", colorToString(*style.backgroundColor).c_str());
        }
        if (style.textColor) {
            styleNode->SetAttribute("textColor", colorToString(*style.textColor).c_str());
        }
        if (style.borderColor) {
            styleNode->SetAttribute("borderColor", colorToString(*style.borderColor).c_str());
        }
        if (style.borderWidth) {
            styleNode->SetAttribute("borderWidth", *style.borderWidth);
        }
        if (style.borderRadius) {
            styleNode->SetAttribute("borderRadius", *style.borderRadius);
        }
        if (style.fontSize) {
            styleNode->SetAttribute("fontSize", *style.fontSize);
        }
        if (style.fontName) {
            styleNode->SetAttribute("fontName", style.fontName->c_str());
        }
        
        elemNode->InsertEndChild(styleNode);
    }
    
    auto childIndices = getChildIndices(element.id, allElements);
    for (size_t childIdx : childIndices) {
        addElementToXML(doc, elemNode, allElements[childIdx], allElements);
    }
    
    parent->InsertEndChild(elemNode);
}

std::string LayoutExporter::elementToJSON(const EditorElement& element, const std::vector<EditorElement>& allElements) {
    std::ostringstream oss;
    oss << "{\n";
    oss << "        \"type\": \"" << element.type << "\",\n";
    oss << "        \"id\": \"" << element.id << "\",\n";
    oss << "        \"x\": " << element.x << ", \"y\": " << element.y << ",\n";
    oss << "        \"width\": " << element.width << ", \"height\": " << element.height;
    
    for (const auto& [key, value] : element.properties) {
        oss << ",\n        \"" << key << "\": ";
        if (isNumericAttribute(key, value)) {
            oss << std::stoi(value);
        } else if (isBoolAttribute(key)) {
            oss << (value == "true" ? "true" : "false");
        } else {
            oss << "\"" << value << "\"";
        }
    }
    
    auto childIndices = getChildIndices(element.id, allElements);
    if (!childIndices.empty()) {
        oss << ",\n        \"children\": [\n";
        for (size_t i = 0; i < childIndices.size(); ++i) {
            if (i > 0) oss << ",\n";
            oss << "          " << elementToJSON(allElements[childIndices[i]], allElements);
        }
        oss << "\n        ]";
    }
    
    if (!element.styles.empty()) {
        oss << ",\n        \"styles\": [\n";
        bool firstStyle = true;
        for (const auto& [state, style] : element.styles) {
            if (!firstStyle) oss << ",\n";
            firstStyle = false;
            oss << "          {\"state\": \"" << elementStateToString(state) << "\"";
            if (style.backgroundColor) oss << ", \"backgroundColor\": \"" << colorToString(*style.backgroundColor) << "\"";
            if (style.textColor) oss << ", \"textColor\": \"" << colorToString(*style.textColor) << "\"";
            if (style.borderColor) oss << ", \"borderColor\": \"" << colorToString(*style.borderColor) << "\"";
            if (style.borderWidth) oss << ", \"borderWidth\": " << *style.borderWidth;
            if (style.borderRadius) oss << ", \"borderRadius\": " << *style.borderRadius;
            if (style.fontSize) oss << ", \"fontSize\": " << *style.fontSize;
            if (style.fontName) oss << ", \"fontName\": \"" << *style.fontName << "\"";
            oss << "}";
        }
        oss << "\n        ]";
    }
    
    oss << "\n      }";
    return oss.str();
}

std::vector<size_t> LayoutExporter::getRootIndices(const std::vector<EditorElement>& elements) {
    std::vector<size_t> indices;
    for (size_t i = 0; i < elements.size(); ++i) {
        if (elements[i].parentId.empty()) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<size_t> LayoutExporter::getChildIndices(const std::string& parentId, const std::vector<EditorElement>& elements) {
    std::vector<size_t> indices;
    for (size_t i = 0; i < elements.size(); ++i) {
        if (elements[i].parentId == parentId) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::string LayoutExporter::elementStateToString(ElementState state) {
    switch (state) {
        case ElementState::Normal: return "Normal";
        case ElementState::Hover: return "Hover";
        case ElementState::Pressed: return "Pressed";
        case ElementState::Disabled: return "Disabled";
        default: return "Normal";
    }
}

std::string LayoutExporter::colorToString(const SDL_Color& c) {
    std::ostringstream oss;
    oss << static_cast<int>(c.r) << "," 
        << static_cast<int>(c.g) << "," 
        << static_cast<int>(c.b) << "," 
        << static_cast<int>(c.a);
    return oss.str();
}

bool LayoutExporter::isNumericAttribute(const std::string& key, const std::string& value) {
    static const std::set<std::string> numericKeys = {
        "fontSize", "borderWidth", "borderRadius", "min", "max", "value",
        "rowCount", "colCount", "tabHeight", "rowHeight", "selectedIndex",
        "frames", "rows", "frameW", "frameH", "fps",
        "optionSpacing", "buttonX", "labelX", "startY", "buttonSize"
    };
    
    if (numericKeys.find(key) != numericKeys.end()) {
        try {
            std::size_t pos;
            static_cast<void>(std::stoi(value, &pos));
            return pos == value.length();
        } catch (...) {
            return false;
        }
    }
    return false;
}

bool LayoutExporter::isBoolAttribute(const std::string& key) {
    static const std::set<std::string> boolKeys = {
        "draggable", "checked", "selected", "visible", "enabled",
        "showRowHeaders", "showColumnHeaders", "editable", "locked",
        "wordWrap", "loop", "autoplay"
    };
    return boolKeys.find(key) != boolKeys.end();
}