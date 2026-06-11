#include <cctype>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

#if defined(_WIN32) || defined(_WIN64)
#define EXPORT __declspec(dllexport)
#elif defined(__APPLE__) || defined(__linux__)
#define EXPORT __attribute__((visibility("default")))
#else
#define EXPORT
#endif

extern "C" {
	
static std::string trim(const std::string &s) {
  size_t b = s.find_first_not_of(" \t\r\n");
  if (b == std::string::npos)
    return "";
  size_t e = s.find_last_not_of(" \t\r\n");
  return s.substr(b, e - b + 1);
}

static std::string parse_inline(const std::string &t) {
  std::string r;
  size_t i = 0, n = t.size();
  r.reserve(n);

  while (i < n) {
    if (t[i] == '\\' && i + 1 < n) {
      r += t[i + 1];
      i += 2;
      continue;
    }
    if (t[i] == '`') {
      size_t e = t.find('`', i + 1);
      if (e != std::string::npos) {
        r += "<code>" + t.substr(i + 1, e - i - 1) + "</code>";
        i = e + 1;
        continue;
      }
    }
    if (t[i] == '!' && i + 1 < n && t[i + 1] == '[') {
      size_t cb = t.find(']', i + 2);
      if (cb != std::string::npos && cb + 1 < n && t[cb + 1] == '(') {
        size_t cp = t.find(')', cb + 2);
        if (cp != std::string::npos) {
          r += "<img src=\"" + t.substr(cb + 2, cp - cb - 2) + "\" alt=\"" +
               t.substr(i + 2, cb - i - 2) + "\"/>";
          i = cp + 1;
          continue;
        }
      }
    }
    if (t[i] == '[') {
      size_t cb = t.find(']', i + 1);
      if (cb != std::string::npos && cb + 1 < n && t[cb + 1] == '(') {
        size_t cp = t.find(')', cb + 2);
        if (cp != std::string::npos) {
          r += "<a href=\"" + t.substr(cb + 2, cp - cb - 2) + "\">" +
               t.substr(i + 1, cb - i - 1) + "</a>";
          i = cp + 1;
          continue;
        }
      }
    }
    if (t[i] == '<' && i + 5 < n && t.compare(i + 1, 4, "http") == 0) {
      size_t cp = t.find('>', i + 1);
      if (cp != std::string::npos) {
        std::string url = t.substr(i + 1, cp - i - 1);
        r += "<a href=\"" + url + "\">" + url + "</a>";
        i = cp + 1;
        continue;
      }
    }
    if (i + 1 < n && ((t[i] == '*' && t[i + 1] == '*') || (t[i] == '_' && t[i + 1] == '_'))) {
      std::string m = t.substr(i, 2);
      size_t e = t.find(m, i + 2);
      if (e != std::string::npos) {
        r += "<strong>" + t.substr(i + 2, e - i - 2) + "</strong>";
        i = e + 2;
        continue;
      }
    }
    if (t[i] == '*' || t[i] == '_') {
      char m = t[i];
      size_t e = t.find(m, i + 1);
      if (e != std::string::npos) {
        r += "<em>" + t.substr(i + 1, e - i - 1) + "</em>";
        i = e + 1;
        continue;
      }
    }
    r += t[i++];
  }
  return r;
}

EXPORT json parse_markdown(const std::string &filename) {
  if (filename.empty())
    return {{"error", "Filename is empty"}};
  std::ifstream f(filename);
  if (!f.is_open())
    return {{"error", "Could not open file"}};

  json elements = json::array();
  std::string line;
  bool in_code = false;
  std::string code, lang;
  bool in_para = false;
  std::string para;
  std::vector<json> list;
  bool in_list = false, ordered = false;
  std::vector<std::string> quotes;
  bool in_quote = false;

  auto flush_para = [&] {
    if (in_para) {
      elements.push_back({{"type", "paragraph"}, {"content", parse_inline(trim(para))}});
      para.clear();
      in_para = false;
    }
  };
  auto flush_list = [&] {
    if (in_list) {
      elements.push_back({{"type", ordered ? "ordered_list" : "unordered_list"}, {"items", list}});
      list.clear();
      in_list = false;
    }
  };
  auto flush_quote = [&] {
    if (in_quote) {
      std::string q;
      for (const auto &s : quotes)
        q += s + "\n";
      elements.push_back({{"type", "blockquote"}, {"content", parse_inline(trim(q))}});
      quotes.clear();
      in_quote = false;
    }
  };

  while (std::getline(f, line)) {
    size_t indent = 0;
    while (indent < line.size() && (line[indent] == ' ' || line[indent] == '\t'))
      indent++;

    std::string t = trim(line);

    if (t.rfind("```", 0) == 0) {
      flush_para();
      flush_list();
      flush_quote();
      if (in_code) {
        elements.push_back({{"type", "code_block"}, {"language", lang}, {"content", code}});
        code.clear(); lang.clear(); in_code = false;
      } else {
        in_code = true;
        lang = t.size() > 3 ? trim(t.substr(3)) : "";
      }
      continue;
    }

    if (in_code) {
      code += line + "\n";
      continue;
    }

    if (t.empty()) {
      flush_para();
      flush_list();
      flush_quote();
      continue;
    }

    if (t[0] == '#') {
      size_t lvl = 0;
      while (lvl < t.size() && lvl < 6 && t[lvl] == '#')
        lvl++;
      
      if (lvl > 0 && lvl < t.size() && t[lvl] == ' ') {
        flush_para(); flush_list(); flush_quote();
        elements.push_back({
            {"type", "heading"},
            {"level", (int)lvl},
            {"content", parse_inline(trim(t.substr(lvl)))}
        });
        continue;
      }
    }

    if (t[0] == '>') {
      flush_para();
      flush_list();
      quotes.push_back(t.size() > 1 ? trim(t.substr(1)) : "");
      in_quote = true;
      continue;
    }

    bool is_item = false, ord = false;
    std::string lc;
    
    char m = t[0];
    if ((m == '-' || m == '*' || m == '+') && t.size() > 1 && t[1] == ' ') {
      is_item = true;
      lc = trim(t.substr(2));
    } else if (std::isdigit(static_cast<unsigned char>(m))) {
      size_t p = t.find('.');
      if (p != std::string::npos && p + 1 < t.size() && t[p + 1] == ' ') {
        is_item = true;
        ord = true;
        lc = trim(t.substr(p + 2));
      }
    }

    if (is_item) {
      flush_para();
      flush_quote();
      if (!in_list || ordered != ord) {
        flush_list();
        in_list = true;
        ordered = ord;
      }
      json it = {{"content", parse_inline(lc)}};
      if (indent > 0) {
		  it["indent"] = indent; 
      }
      list.push_back(it);
      continue;
    }

    if (in_quote) flush_quote();
    if (in_list)  flush_list();

    if (!in_para) {
      in_para = true;
      para = t;
    } else {
      para += " " + t;
    }
  }

  flush_para();
  flush_list();
  flush_quote();
  if (in_code) {
    elements.push_back({{"type", "code_block"}, {"language", lang}, {"content", code}});
  }

  return {{"document", elements}, {"elements_count", elements.size()}};
}
}