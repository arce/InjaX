// SPDX-License-Identifier: MIT
// Project: InjaX
#include <filesystem>
#include <fstream>
#include <inja/inja.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <set>
#include "filters.hpp"
#include "tests.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

#if defined(_WIN32)
#include <windows.h>
const std::string LIB_EXTENSION = ".dll";
using lib_handle_t = HMODULE;
#else
#include <dlfcn.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
const std::string LIB_EXTENSION = ".dylib";
#else
const std::string LIB_EXTENSION = ".so";
#endif
using lib_handle_t = void*;
#endif

using parse_fn_t = nlohmann::json (*)(const std::string &);

struct DynamicLibrary {
  lib_handle_t handle = nullptr;

  DynamicLibrary(const std::string& path) {
#if defined(_WIN32)
    handle = LoadLibraryA(path.c_str());
    if (!handle) {
      throw std::runtime_error("LoadLibrary error (" + path + "): " + std::to_string(GetLastError()));
    }
#else
    handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
      throw std::runtime_error("dlopen error (" + path + "): " + std::string(dlerror()));
    }
#endif
  }

  ~DynamicLibrary() {
    if (handle) {
#if defined(_WIN32)
      FreeLibrary(handle);
#else
      dlclose(handle);
#endif
    }
  }

  DynamicLibrary(const DynamicLibrary&) = delete;
  DynamicLibrary& operator=(const DynamicLibrary&) = delete;

  template <typename FuncType>
  FuncType get_symbol(const std::string& symbol_name) const {
#if defined(_WIN32)
    auto fn = (FuncType)GetProcAddress(handle, symbol_name.c_str());
#else
    auto fn = (FuncType)dlsym(handle, symbol_name.c_str());
#endif
    if (!fn) {
      throw std::runtime_error("Symbol '" + symbol_name + "' not found");
    }
    return fn;
  }
};

fs::path get_executable_directory(const char* argv0) {
#if defined(_WIN32)
  char buffer[MAX_PATH];
  GetModuleFileNameA(NULL, buffer, MAX_PATH);
  return fs::path(buffer).parent_path();
#elif defined(__linux__)
  return fs::read_symlink("/proc/self/exe").parent_path();
#elif defined(__APPLE__)
  char buffer[PATH_MAX];
  uint32_t size = sizeof(buffer);
  if (_NSGetExecutablePath(buffer, &size) == 0) {
    return fs::path(buffer).parent_path();
  }
  return fs::absolute(fs::path(argv0)).parent_path();
#else
  return fs::absolute(fs::path(argv0)).parent_path(); 
#endif
}

std::string get_safe_extension(const fs::path& filepath) {
  std::string ext = filepath.extension().string();
  if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
  std::transform(ext.begin(), ext.end(), ext.begin(),
                 [](unsigned char c){ return std::tolower(c); });
  return ext;
}

std::string read_file(const std::string &filename) {
  std::ifstream file(filename, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file for reading: " + filename);
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void write_file(const std::string &filename, const std::string &content) {
  std::ofstream file(filename, std::ios::out | std::ios::trunc);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot write to file: " + filename);
  }
  file << content;
}

json load_data_file(const std::string &data_file, const fs::path &exe_dir, 
                    std::vector<std::unique_ptr<DynamicLibrary>> &loaded_modules) {
  std::string extension = fs::path(data_file).extension().string();
  if (!extension.empty() && extension[0] == '.') {
    extension = extension.substr(1);
  }
  json input_dataset;
  if (extension == "json") {
    std::ifstream infile(data_file);
    if (!infile.is_open()) {
      throw std::runtime_error("Target dataset file does not exist: " + data_file);
    }
    infile >> input_dataset;
  } else {
    std::string lib_name = "readers/lib" + extension + LIB_EXTENSION;
    fs::path parser_path = exe_dir / lib_name;
    if (!fs::exists(parser_path)) {
      throw std::runtime_error("Unsupported file format: ." + extension + 
                               " (plugin not found: " + parser_path.string() + ")");
    }
    std::cout << "Loading dynamic external format parser: " << lib_name << std::endl;
    loaded_modules.push_back(std::make_unique<DynamicLibrary>(parser_path.string()));
    auto parser = loaded_modules.back()->get_symbol<parse_fn_t>("parse_data");
    input_dataset = parser(data_file);
  }
  
  return input_dataset;
}

bool is_valid_data_file(const fs::path &filepath) {
  std::string ext = filepath.extension().string();
  if (ext.empty()) return false;
  static const std::vector<std::string> valid_extensions = {
    ".json", ".yaml", ".yml", ".toml", ".xml", ".csv"
  };
  return std::find(valid_extensions.begin(), valid_extensions.end(), ext) != valid_extensions.end();
}

bool is_valid_template_file(const fs::path &filepath) {
  if (filepath.extension() != ".inja") return false;
  std::string stem = filepath.stem().string();
  auto pos = stem.rfind('-');
  return (pos != std::string::npos && pos < stem.length() - 1);
}

std::string get_format_from_template(const fs::path &template_file) {
  if (template_file.extension() != ".inja") return "";
  std::string stem = template_file.stem().string();
  auto pos = stem.rfind('-');
  if (pos == std::string::npos) return "";
  std::string fmt = stem.substr(pos + 1);
  for (char c : fmt) {
    if (!std::isalnum(static_cast<unsigned char>(c))) return "";
  }
  return fmt;
}

std::string get_output_filename(const fs::path &data_file, const fs::path &template_file) {
  std::string format = get_format_from_template(template_file);
  if (format.empty()) {
    return data_file.stem().string() + ".html";
  }

  std::string template_stem = template_file.stem().string();
  auto pos = template_stem.rfind('-');
  std::string template_base = template_stem.substr(0, pos);
  std::string data_stem = data_file.stem().string();

  if (template_base == "index") {
    return data_stem + "/index." + format;
  }
  return data_stem + "." + format;
}

void process_single_file(const std::string &data_file, const std::string &template_file,
                         const std::string &output_file, inja::Environment &env,
                         const fs::path &exe_dir,
                         std::vector<std::unique_ptr<DynamicLibrary>> &loaded_modules) {
  std::cout << "Processing single file mode..." << std::endl;
  std::cout << "  Data: " << data_file << std::endl;
  std::cout << "  Template: " << template_file << std::endl;
  std::cout << "  Output: " << output_file << std::endl;
  
  json input_dataset = load_data_file(data_file, exe_dir, loaded_modules);
  std::string template_content = read_file(template_file);
  std::stringstream output_stream;
  env.render_to(output_stream, template_content, input_dataset);
  
  std::string derived_format = get_format_from_template(fs::path(template_file));
  fs::path out_path(output_file);
  if (!derived_format.empty()) {
    if (out_path.extension().string().empty()) {
      out_path += "." + derived_format;
    }
  }
  write_file(out_path.string(), output_stream.str());
  std::cout << "Render sequence completed successfully." << std::endl;
}

int get_template_distance(const std::string& data_sub_dir, const std::string& template_sub_dir) {
  if (data_sub_dir == template_sub_dir) return 0;
  fs::path current_dir(data_sub_dir);
  int distance = 0;
  while (!current_dir.empty() && current_dir != ".") {
    current_dir = current_dir.parent_path();
    distance++;
    if (current_dir.string() == template_sub_dir || (current_dir.empty() && template_sub_dir.empty())) return distance;
  }
  return template_sub_dir.empty() ? distance + 1 : -1;
}

fs::path find_best_template(const std::string &sub_dir, const std::string &target_base, 
                            const std::vector<fs::path> &template_files, const fs::path &template_path) {
  fs::path best_template;
  int best_distance = -1;
  for (const auto &template_file : template_files) {
    fs::path rel_path = fs::relative(template_file, template_path);
    std::string stem = template_file.stem().string();
    auto pos = stem.rfind('-');
    std::string base = (pos != std::string::npos) ? stem.substr(0, pos) : stem;

    if (base != target_base) continue;

    int distance = get_template_distance(sub_dir, rel_path.parent_path().string());
    if (distance != -1 && (best_distance == -1 || distance < best_distance)) {
      best_distance = distance;
      best_template = template_file;
    }
  }
  return best_template;
}

std::vector<fs::path> collect_files(const fs::path &dir, bool (*validator)(const fs::path&)) {
  std::vector<fs::path> files;
  for (const auto &entry : fs::recursive_directory_iterator(dir)) {
    if (entry.is_regular_file() && validator(entry.path())) files.push_back(entry.path());
  }
  return files;
}

void process_directory_mode(const std::string &data_dir, const std::string &template_dir,
                            const std::string &output_dir, inja::Environment &env,
                            const fs::path &exe_dir,
                            std::vector<std::unique_ptr<DynamicLibrary>> &loaded_modules) {
  std::cout << "Processing directory mode...\n  Data: " << data_dir << "\n  Template: " << template_dir << "\n  Output: " << output_dir << std::endl;
  
  fs::path data_path(data_dir), template_path(template_dir), output_path(output_dir);
  if (!fs::is_directory(data_path) || !fs::is_directory(template_path)) {
    throw std::runtime_error("Data or Template directory does not exist.");
  }
  
  auto data_files = collect_files(data_path, is_valid_data_file);
  auto template_files = collect_files(template_path, is_valid_template_file);
  if (data_files.empty() || template_files.empty()) {
    throw std::runtime_error("No valid data or template files found.");
  }
  
  std::cout << "Found " << data_files.size() << " data file(s) and " << template_files.size() << " template(s)" << std::endl;
            
  std::map<std::string, std::vector<json>> files_by_dir;
  std::map<std::string, json> loaded_data_cache;
  std::map<std::string, std::string> template_content_cache;
  std::set<std::string> active_subdirs;
  std::set<std::string> template_bases;

  for (const auto &template_file : template_files) {
    std::string stem = template_file.stem().string();
    auto pos = stem.rfind('-');
    std::string base = (pos != std::string::npos) ? stem.substr(0, pos) : stem;
    if (base != "index") template_bases.insert(base);
    
    try {
      template_content_cache[template_file.string()] = read_file(template_file.string());
    } catch (const std::exception &e) {
      std::cerr << "  Warning: Could not cache template " << template_file.string() << ": " << e.what() << std::endl;
    }
  }

  for (const auto &data_file : data_files) {
    std::string sub_dir = fs::relative(data_file, data_path).parent_path().string();
    json current_data = load_data_file(data_file.string(), exe_dir, loaded_modules);
    current_data["_filename"] = data_file.filename().string();
    current_data["_stem"] = data_file.stem().string();
    current_data["_path"] = sub_dir;

    loaded_data_cache[data_file.string()] = current_data;
    files_by_dir[sub_dir].push_back(current_data);
    active_subdirs.insert(sub_dir);
  }

  int processed_count = 0;
  auto render_and_write = [&](const fs::path &template_file, const json &data, const fs::path &dest_path, const std::string &log_msg) {
    if (template_content_cache.find(template_file.string()) == template_content_cache.end()) {
      std::cerr << "  Warning: Template content not available for " << template_file.string() << std::endl;
      return;
    }
    fs::create_directories(dest_path.parent_path());
    try {
      json render_data = data;
      render_data["_template"] = template_file.filename().string();
      render_data["_template_stem"] = template_file.stem().string();
      
      std::stringstream output_stream;
      env.render_to(output_stream, template_content_cache[template_file.string()], render_data);
      write_file(dest_path.string(), output_stream.str());
      
      std::cout << log_msg << dest_path.string() << std::endl;
      processed_count++;
    } catch (const std::exception &e) {
      std::cerr << "  Warning: Failed to render " << template_file.string() << ": " << e.what() << std::endl;
    }
  };

  for (const auto &data_file : data_files) {
    fs::path rel_data_path = fs::relative(data_file, data_path);
    std::string sub_dir = rel_data_path.parent_path().string();
    json current_data = loaded_data_cache[data_file.string()];
    current_data["page_list"] = json::array();

    for (const auto &base : template_bases) {
      fs::path best_template = find_best_template(sub_dir, base, template_files, template_path);
      if (best_template.empty()) continue;

      fs::path output_filepath = output_path / rel_data_path.parent_path() / get_output_filename(data_file, best_template);
      std::string log = "  Rendered: " + rel_data_path.string() + " + " + best_template.filename().string() + " -> ";
      render_and_write(best_template, current_data, output_filepath, log);
    }
  }

  for (const auto &sub_dir : active_subdirs) {
    fs::path best_index_template = find_best_template(sub_dir, "index", template_files, template_path);
    if (best_index_template.empty()) continue;

    fs::path output_filepath = output_path / sub_dir / ("index." + get_format_from_template(best_index_template));
    json index_data = files_by_dir[sub_dir].empty() ? json::object() : files_by_dir[sub_dir].front();
    index_data["page_list"] = files_by_dir[sub_dir];

    std::string log = "  Rendered Index: " + sub_dir + " + " + best_index_template.filename().string() + " -> ";
    render_and_write(best_index_template, index_data, output_filepath, log);
  }

  std::cout << "Directory mode completed. " << processed_count << " file(s) rendered." << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc < 4) {
    std::cerr << "Usage: " << argv[0] << " <data> <template> <output>" << std::endl;
    std::cerr << "   or: " << argv[0] << " <data-directory> <template-directory> <output-directory>" << std::endl;
    return 1;
  }

  std::cout << "SPDX-License-Identifier: MIT" << std::endl;
  std::cout << "Project: InjaX" << std::endl;

  const std::string source = argv[1];
  const std::string template_source = argv[2];
  const std::string output_target = argv[3];

  std::vector<std::unique_ptr<DynamicLibrary>> loaded_modules;
  inja::Environment env;

  env.set_trim_blocks(true);
  env.set_lstrip_blocks(true);

  custom_tests::register_tests(env);
  register_all_filters(env);

  try {
    const fs::path exe_dir = get_executable_directory(argv[0]);
    const fs::path modules_path = exe_dir / "modules";

    std::cout << "Searching plugins from: " << modules_path << std::endl;

    if (fs::exists(modules_path) && fs::is_directory(modules_path)) {
      for (const auto &entry : fs::directory_iterator(modules_path)) {
        std::string filename = entry.path().filename().string();
        std::string ext = entry.path().extension().string();
        if (ext == LIB_EXTENSION && filename.rfind("lib", 0) == 0) {
          try {
            auto mod = std::make_unique<DynamicLibrary>(entry.path().string());
            auto register_fn = mod->get_symbol<void (*)(inja::Environment &)>("register_module");
            register_fn(env);
            loaded_modules.push_back(std::move(mod));
            std::cout << "Successfully registered extension: " << filename << std::endl;
          } catch (const std::exception &e) {
            std::cerr << "Warning -> Skipping module " << filename << ": " << e.what() << std::endl;
          }
        }
      }
    }

    bool is_directory_mode = fs::is_directory(source) && 
                             fs::is_directory(template_source) && 
                             (fs::exists(output_target) ? fs::is_directory(output_target) : true);

    if (is_directory_mode) {
      process_directory_mode(source, template_source, output_target, env, exe_dir, loaded_modules);
    } else {
      process_single_file(source, template_source, output_target, env, exe_dir, loaded_modules);
    }

  } catch (const std::exception &e) {
    std::cerr << "Fatal Core Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}