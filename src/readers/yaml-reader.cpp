#include <fkYAML/node.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <functional>

using json = nlohmann::json;

#if defined(_WIN32) || defined(_WIN64)
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

EXPORT json parse_data(const std::string& filename) {
    if (filename.empty()) return json::object();

    try {
        std::ifstream file(filename);
        if (!file.is_open()) return json::object();        
        fkyaml::node yaml_root = fkyaml::node::deserialize(file);
        
        std::function<json(const fkyaml::node&)> convert = [&](const fkyaml::node& n) -> json {
            if (n.is_scalar()) {
                std::string val = n.get_value<std::string>();
                if (val == "true") return true;
                if (val == "false") return false;
                if (val == "null") return nullptr;
                try {
                    size_t pos;
                    double d = std::stod(val, &pos);
                    if (pos == val.length()) return d;
                } catch (...) {}
                return val;
            }
            if (n.is_sequence()) {
                json arr = json::array();
                for (const auto& child : n.as_seq()) {
                    arr.push_back(convert(child));
                }
                return arr;
            }
            if (n.is_mapping()) {
                json obj = json::object();
                for (const auto& [key_node, value_node] : n.map_items()) {
                    std::string key_str = key_node.get_value<std::string>();
                    obj[key_str] = convert(value_node);
                }
                return obj;
            }
            return nullptr;
        };
        
        return convert(yaml_root);
        
    } catch (const fkyaml::exception& e) {
        std::cerr << "YAML parsing error: " << e.what() << std::endl;
        return json::object();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return json::object();
    }
}