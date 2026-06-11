#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <functional>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using json = nlohmann::json;

struct TreeAlgoModule {
  static std::string to_string_id(const json& j) {
    if (j.is_string()) return j.get_ref<const std::string&>();
    if (j.is_number()) return std::to_string(j.get<double>());
    if (j.is_boolean()) return j.get<bool>() ? "true" : "false";
    return j.dump();
  }

  static bool has_children(const json& node) {
    return node.contains("children") && node["children"].is_array() && !node["children"].empty();
  }

  template<typename F>
  void for_each_child(const json& node, F&& f) const {
    if (!node.contains("children") || !node["children"].is_array()) return;
    for (const auto& c : node["children"]) f(c);
  }

  void build_maps(const json& root,
                  std::map<std::string, std::vector<std::string>>& children,
                  std::map<std::string, std::string>& parent,
                  const std::string& parent_id = "") const {
    if (!root.contains("id")) throw std::runtime_error("node missing id");
    std::string id = to_string_id(root["id"]);
    if (!parent_id.empty()) {
      parent[id] = parent_id;
      children[parent_id].push_back(id);
    }
    for_each_child(root, [&](const json& c){ build_maps(c, children, parent, id); });
  }

  void collect_all_nodes(const json& root, std::vector<std::string>& out) const {
    if (!root.contains("id")) throw std::runtime_error("node missing id");
    out.push_back(to_string_id(root["id"]));
    for_each_child(root, [&](const json& c){ collect_all_nodes(c, out); });
  }

  void compute_depths(const json& root, std::map<std::string,int>& depth_map, int depth = 0) const {
    if (!root.contains("id")) throw std::runtime_error("node missing id");
    std::string id = to_string_id(root["id"]);
    depth_map[id] = depth;
    for_each_child(root, [&](const json& c){ compute_depths(c, depth_map, depth + 1); });
  }

  std::vector<std::string> collect_ancestors(const std::string& node_id,
                                             const std::map<std::string,std::string>& parent) const {
    std::vector<std::string> anc;
    auto it = parent.find(node_id);
    while (it != parent.end() && !it->second.empty()) {
      anc.push_back(it->second);
      it = parent.find(it->second);
    }
    return anc;
  }

  int height(const json& root) const {
    if (!has_children(root)) return 0;
    int maxh = 0;
    for_each_child(root, [&](const json& c){ maxh = std::max(maxh, height(c)); });
    return maxh + 1;
  }

  json levels(const json& root) const {
    std::map<std::string,int> depth_map;
    compute_depths(root, depth_map);
    json out = json::object();
    for (const auto& [id,d] : depth_map) out[id] = d;
    return out;
  }

  json ancestors(const json& root, const std::string& node_id) const {
    std::map<std::string,std::vector<std::string>> children;
    std::map<std::string,std::string> parent;
    build_maps(root, children, parent);
    auto anc = collect_ancestors(node_id, parent);
    json out = json::array();
    for (const auto& a : anc) out.push_back(a);
    return out;
  }

  json descendants(const json& root, const std::string& node_id) const {
    std::function<void(const json&, json&)> collect;
    std::function<void(const json&, const std::string&, json&)> find_node;
    collect = [&](const json& node, json& list){
      for_each_child(node, [&](const json& c){
        list.push_back(c["id"]);
        collect(c, list);
      });
    };
    find_node = [&](const json& node, const std::string& target, json& list){
      if (!node.contains("id")) return;
      if (to_string_id(node["id"]) == target) { collect(node, list); return; }
      for_each_child(node, [&](const json& c){ find_node(c, target, list); });
    };
    json out = json::array();
    find_node(root, node_id, out);
    return out;
  }

  json path_between(const json& root, const std::string& a, const std::string& b) const {
    std::map<std::string,std::vector<std::string>> children;
    std::map<std::string,std::string> parent;
    build_maps(root, children, parent);
    auto ancA = collect_ancestors(a, parent);
    auto ancB = collect_ancestors(b, parent);
    std::set<std::string> setA(ancA.begin(), ancA.end());
    std::string lca;
    for (const auto& x : ancB) if (setA.count(x)) { lca = x; break; }
    std::vector<std::string> path;
    path.push_back(a);
    for (const auto& x : ancA) { path.push_back(x); if (x == lca) break; }
    std::reverse(path.begin(), path.end());
    if (path.empty() || path.back() != lca) path.push_back(lca);
    std::vector<std::string> toB;
    for (const auto& x : ancB) { if (x == lca) break; toB.push_back(x); }
    std::reverse(toB.begin(), toB.end());
    path.insert(path.end(), toB.begin(), toB.end());
    path.push_back(b);
    json out = json::array();
    for (const auto& n : path) out.push_back(n);
    return out;
  }

  json lowest_common_ancestor(const json& root, const std::string& a, const std::string& b) const {
    std::map<std::string,std::vector<std::string>> children;
    std::map<std::string,std::string> parent;
    build_maps(root, children, parent);
    auto ancA = collect_ancestors(a, parent);
    auto ancB = collect_ancestors(b, parent);
    std::set<std::string> setA(ancA.begin(), ancA.end());
    for (const auto& x : ancB) if (setA.count(x)) return json(x);
    return json(nullptr);
  }

  double balance(const json& root) const {
    if (!has_children(root)) return 1.0;
    std::vector<int> heights;
    for_each_child(root, [&](const json& c){ heights.push_back(height(c)); });
    int mn = *std::min_element(heights.begin(), heights.end());
    int mx = *std::max_element(heights.begin(), heights.end());
    if (mx == 0) return 1.0;
    return static_cast<double>(mn + 1) / (mx + 1);
  }

  int leaf_count(const json& root) const {
    if (!has_children(root)) return 1;
    int cnt = 0;
    for_each_child(root, [&](const json& c){ cnt += leaf_count(c); });
    return cnt;
  }

  int max_width(const json& root) const {
    std::map<std::string,int> depth_map;
    compute_depths(root, depth_map);
    std::map<int,int> per_level;
    for (const auto& [id,d] : depth_map) per_level[d]++;
    int mx = 0;
    for (const auto& [lvl,w] : per_level) mx = std::max(mx, w);
    return mx;
  }

  json layout_layered(const json& root, double x_spacing = 100.0, double y_spacing = 100.0) const {
    std::map<std::string,int> depth_map;
    compute_depths(root, depth_map);
    std::map<int,int> counters;
    json out = json::array();
    std::function<void(const json&, int)> place = [&](const json& node, int level){
      std::string id = to_string_id(node["id"]);
      double x = counters[level] * x_spacing;
      double y = level * y_spacing;
      json n;
      n["id"] = node["id"];
      n["x"] = x;
      n["y"] = y;
      n["level"] = level;
      out.push_back(n);
      counters[level]++;
      for_each_child(node, [&](const json& c){ place(c, level + 1); });
    };
    place(root, 0);
    return out;
  }

  json layout_radial(const json& root, double radius_step = 150.0) const {
    std::map<std::string,int> depth_map;
    compute_depths(root, depth_map);
    json out = json::array();
    std::function<void(const json&, int, double, double)> place =
      [&](const json& node, int level, double start, double end){
        std::string id = to_string_id(node["id"]);
        double radius = level * radius_step;
        double angle = (level == 0) ? 0.0 : (start + end) / 2.0;
        double x = radius * cos(angle);
        double y = radius * sin(angle);
        json n;
        n["id"] = node["id"];
        n["x"] = x;
        n["y"] = y;
        n["radius"] = radius;
        n["angle"] = angle;
        n["level"] = level;
        out.push_back(n);
        if (has_children(node)) {
          int m = node["children"].size();
          double range = (end - start) * 0.8;
          double s = angle - range / 2.0;
          double step = range / m;
          for (int i = 0; i < m; ++i) {
            place(node["children"][i], level + 1, s + i * step, s + (i + 1) * step);
          }
        }
      };
    place(root, 0, -M_PI, M_PI);
    return out;
  }

  json layout_star(const json& root, double radius = 200.0) const {
    json out = json::array();
    std::function<void(const json&, double, double)> place =
      [&](const json& node, double cx, double cy){
        json n;
        n["id"] = node["id"];
        n["x"] = cx;
        n["y"] = cy;
        out.push_back(n);
        if (has_children(node)) {
          int m = node["children"].size();
          for (int i = 0; i < m; ++i) {
            double angle = (2.0 * M_PI * i) / m;
            double x = cx + radius * cos(angle);
            double y = cy + radius * sin(angle);
            place(node["children"][i], x, y);
          }
        }
      };
    place(root, 0.0, 0.0);
    return out;
  }

  json layout_spiral(const json& root, double a = 20.0, double b = 15.0) const {
    std::map<std::string,int> depth_map;
    compute_depths(root, depth_map);
    json out = json::array();
    int index = 0;
    std::function<void(const json&)> place = [&](const json& node){
      std::string id = to_string_id(node["id"]);
      int depth = depth_map[id];
      double theta = index * 0.5;
      double r = a * exp(b * theta / 20.0);
      double x = r * cos(theta);
      double y = r * sin(theta);
      json n;
      n["id"] = node["id"];
      n["x"] = x;
      n["y"] = y;
      n["theta"] = theta;
      n["radius"] = r;
      n["level"] = depth;
      out.push_back(n);
      ++index;
      for_each_child(node, [&](const json& c){ place(c); });
    };
    place(root);
    return out;
  }

  json layout_grid(const json& root, double cell_w = 120.0, double cell_h = 80.0) const {
    std::map<std::string,int> depth_map;
    compute_depths(root, depth_map);
    std::map<int,int> counters;
    json out = json::array();
    std::function<void(const json&, int)> place = [&](const json& node, int level){
      std::string id = to_string_id(node["id"]);
      double x = counters[level] * cell_w;
      double y = level * cell_h;
      json n;
      n["id"] = node["id"];
      n["x"] = x;
      n["y"] = y;
      n["row"] = level;
      n["col"] = counters[level];
      out.push_back(n);
      counters[level]++;
      for_each_child(node, [&](const json& c){ place(c, level + 1); });
    };
    place(root, 0);
    return out;
  }

  json layout_binary(const json& root, double x_spacing = 100.0, double y_spacing = 80.0) const {
    json out = json::array();
    std::map<std::string,int> pos;
    std::function<int(const json&, int, int)> compute = [&](const json& node, int x, int level)->int{
      std::string id = to_string_id(node["id"]);
      if (has_children(node)) {
        int n = node["children"].size();
        int curx = x;
        if (n >= 1) curx = compute(node["children"][0], x, level + 1);
        pos[id] = curx;
        json nobj;
        nobj["id"] = node["id"];
        nobj["x"] = curx;
        nobj["y"] = level * y_spacing;
        out.push_back(nobj);
        if (n >= 2) return compute(node["children"][1], curx + x_spacing, level + 1);
        return curx;
      } else {
        pos[id] = x;
        json nobj;
        nobj["id"] = node["id"];
        nobj["x"] = x;
        nobj["y"] = level * y_spacing;
        out.push_back(nobj);
        return x + static_cast<int>(x_spacing);
      }
    };
    compute(root, 0, 0);
    return out;
  }
};

void register_tree_algo_module(inja::Environment& env) {
  static TreeAlgoModule instance;
  TreeAlgoModule* ptr = &instance;

  env.add_callback("tree_height", 1, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    return ptr->height(tree);
  });

  env.add_callback("node_levels", 1, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    return ptr->levels(tree);
  });

  env.add_callback("get_ancestors", 2, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    std::string id = TreeAlgoModule::to_string_id(*args[1]);
    return ptr->ancestors(tree, id);
  });

  env.add_callback("get_descendants", 2, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    std::string id = TreeAlgoModule::to_string_id(*args[1]);
    return ptr->descendants(tree, id);
  });

  env.add_callback("path_between", 3, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    std::string a = TreeAlgoModule::to_string_id(*args[1]);
    std::string b = TreeAlgoModule::to_string_id(*args[2]);
    return ptr->path_between(tree, a, b);
  });

  env.add_callback("lca", 3, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    std::string a = TreeAlgoModule::to_string_id(*args[1]);
    std::string b = TreeAlgoModule::to_string_id(*args[2]);
    return ptr->lowest_common_ancestor(tree, a, b);
  });

  env.add_callback("tree_balance", 1, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    return ptr->balance(tree);
  });

  env.add_callback("leaf_count", 1, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    return ptr->leaf_count(tree);
  });

  env.add_callback("max_width", 1, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    return ptr->max_width(tree);
  });

  env.add_callback("layout_layered", 1, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    return ptr->layout_layered(tree);
  });

  env.add_callback("layout_layered", 3, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    double xs = args[1]->get<double>();
    double ys = args[2]->get<double>();
    return ptr->layout_layered(tree, xs, ys);
  });

  env.add_callback("layout_radial", 1, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    return ptr->layout_radial(tree);
  });

  env.add_callback("layout_radial", 2, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    double rs = args[1]->get<double>();
    return ptr->layout_radial(tree, rs);
  });

  env.add_callback("layout_star", 1, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    return ptr->layout_star(tree);
  });

  env.add_callback("layout_star", 2, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    double r = args[1]->get<double>();
    return ptr->layout_star(tree, r);
  });

  env.add_callback("layout_spiral", 1, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    return ptr->layout_spiral(tree);
  });

  env.add_callback("layout_grid", 1, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    return ptr->layout_grid(tree);
  });

  env.add_callback("layout_binary_tree", 1, [ptr](inja::Arguments& args)->json{
    const json& tree = *args[0];
    return ptr->layout_binary(tree);
  });
}

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif

EXPORT void register_module(inja::Environment& env) {
  register_tree_algo_module(env);
}