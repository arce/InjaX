// node_editor_parser.cpp
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

using json = nlohmann::json;

#if defined(_WIN32) || defined(_WIN64)
  #define EXPORT __declspec(dllexport)
#elif defined(__APPLE__) || defined(__linux__)
  #define EXPORT __attribute__((visibility("default")))
#else
  #define EXPORT
#endif

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

json parse_link(const std::string& line) {
    std::string content = line.substr(5);
    content = trim(content);
    
    size_t arrow_pos = content.find("->");
    if (arrow_pos == std::string::npos) return json::object();
    
    std::string source = trim(content.substr(0, arrow_pos));
    std::string target = trim(content.substr(arrow_pos + 2));
    
    json link;
    link["source"] = source;
    link["target"] = target;
    return link;
}

json parse_node(const std::string& line) {
    std::string content = line.substr(5);
    content = trim(content);
    
    std::vector<std::string> parts = split(content, ' ');
    if (parts.size() < 4) return json::object();
    
    json node;
    node["id"] = parts[0];
    node["type"] = parts[1];
    node["x"] = std::stoi(parts[2]);
    node["y"] = std::stoi(parts[3]);
    
    if (parts.size() >= 5) {
        node["name"] = parts[4];
    }
    
    return node;
}

json parse_nodetype(const std::string& line) {
    std::string content = line;
    bool is_group = false;
    
    if (content.find("NODETYPE+") == 0) {
        content = content.substr(9);
        is_group = true;
    } else if (content.find("NODETYPE") == 0) {
        content = content.substr(8);
    }
    content = trim(content);
    
    std::vector<std::string> parts = split(content, ' ');
    if (parts.size() < 2) return json::object();
    
    json nodetype;
    nodetype["name"] = parts[0];
    nodetype["category"] = parts[1];
    nodetype["is_group"] = is_group;
    
    return nodetype;
}

json parse_child(const std::string& line) {
    std::string content = line.substr(6);
    content = trim(content);
    
    std::vector<std::string> parts = split(content, ' ');
    if (parts.size() < 2) return json::object();
    
    json child;
    child["parent"] = parts[0];
    child["child"] = parts[1];
    return child;
}

extern "C" {

EXPORT json parse_node_file(const std::string& filename) {
    if (filename.empty()) return json::object();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        return json::object();
    }
    
    json result;
    json nodes = json::array();
    json links = json::array();
    json nodetypes = json::array();
    json children = json::array();
    
    std::string line;
    while (std::getline(file, line)) {
        // Eliminar espacios al inicio
        line = trim(line);
        
        // Saltar líneas vacías y comentarios
        if (line.empty() || line[0] == '#') continue;
        
        // Parsear según el tipo de línea
        if (line.find("NODETYPE+") == 0 || line.find("NODETYPE") == 0) {
            json nt = parse_nodetype(line);
            if (!nt.empty()) {
                nodetypes.push_back(nt);
            }
        }
        else if (line.find("NODE ") == 0) {
            json node = parse_node(line);
            if (!node.empty()) {
                nodes.push_back(node);
            }
        }
        else if (line.find("CHILD ") == 0) {
            json child = parse_child(line);
            if (!child.empty()) {
                children.push_back(child);
            }
        }
        else if (line.find("LINK ") == 0) {
            json link = parse_link(line);
            if (!link.empty()) {
                links.push_back(link);
            }
        }
    }
    
    file.close();
    
    result["nodes"] = nodes;
    result["links"] = links;
    result["nodetypes"] = nodetypes;
    result["children"] = children;
    
    return result;
}

}