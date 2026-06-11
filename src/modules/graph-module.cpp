#ifndef GRAPH_MODULE_H
#define GRAPH_MODULE_H

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <queue>
#include <stack>
#include <cmath>
#include <algorithm>
#include <random>
#include <string>
#include <limits>
#include <unordered_set>
#include <functional>

using json = nlohmann::json;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct graph_t {
  int num_vertices;
  int num_edges = 0;
  bool directed;
  std::vector<std::vector<int>> adj_list;
  std::vector<std::vector<double>> weights;

  graph_t(int n = 0, bool dir = false) : num_vertices(n), directed(dir) {
    adj_list.resize(n);
    weights.resize(n);
  }

  void add_edge(int from, int to, double weight = 1.0) {
    if (from >= 0 && from < num_vertices && to >= 0 && to < num_vertices) {
      adj_list[from].push_back(to);
      weights[from].push_back(weight);
      if (!directed) {
        adj_list[to].push_back(from);
        weights[to].push_back(weight);
      }
      num_edges++;
    }
  }

  double get_weight(int from, int to) const {
    for (size_t i = 0; i < adj_list[from].size(); i++) {
      if (adj_list[from][i] == to) return weights[from][i];
    }
    return 1.0;
  }
};

struct graph_matrix_t {
  std::vector<double> data;
  int nrow = 0, ncol = 0;
  void init(int r, int c) {
    nrow = r; ncol = c;
    data.assign(r * c, 0.0);
  }
  double& operator()(int i, int j) { return data[i * ncol + j]; }
  const double& operator()(int i, int j) const { return data[i * ncol + j]; }
  void reset() { std::fill(data.begin(), data.end(), 0.0); }
};

struct QuadNode {
  double x_center = 0, y_center = 0;
  double x_min, y_min, size;
  int count = 0;
  int node_idx = -1;
  QuadNode* children[4] = {nullptr, nullptr, nullptr, nullptr};

  QuadNode(double x, double y, double s) : x_min(x), y_min(y), size(s) {}

  ~QuadNode() {
    for (int i = 0; i < 4; ++i) if (children[i]) delete children[i];
  }

  void insert(int idx, double x, double y) {
    if (count == 0) {
      node_idx = idx;
      x_center = x;
      y_center = y;
    } else {
      if (node_idx != -1) {
        insert_into_child(node_idx, x_center, y_center);
        node_idx = -1;
      }
      x_center = (x_center * count + x) / (count + 1);
      y_center = (y_center * count + y) / (count + 1);
      insert_into_child(idx, x, y);
    }
    count++;
  }

  void insert_into_child(int idx, double x, double y) {
    double half = size / 2.0;
    int quad = 0;
    if (x >= x_min + half) quad += 1;
    if (y >= y_min + half) quad += 2;
    if (!children[quad]) {
      children[quad] = new QuadNode(x_min + (quad % 2) * half, y_min + (quad / 2) * half, half);
    }
    children[quad]->insert(idx, x, y);
  }
};

namespace graph_utils {
  inline std::vector<int> compute_degrees(const graph_t& g, const std::string& mode = "total") {
    std::vector<int> degrees(g.num_vertices, 0);
    if (mode == "in") {
      for (int u = 0; u < g.num_vertices; ++u)
        for (int v : g.adj_list[u]) degrees[v]++;
    } else if (mode == "out") {
      for (int u = 0; u < g.num_vertices; ++u) degrees[u] = static_cast<int>(g.adj_list[u].size());
    } else {
      for (int u = 0; u < g.num_vertices; ++u) degrees[u] = static_cast<int>(g.adj_list[u].size());
      if (g.directed) {
        for (int u = 0; u < g.num_vertices; ++u)
          for (int v : g.adj_list[u]) degrees[v]++;
      }
    }
    return degrees;
  }

  inline void calculate_repulsion_bh(int v, QuadNode* quad, double theta, double k,
                                     const graph_matrix_t& coords, double& fx, double& fy) {
    if (!quad || quad->count == 0 || quad->node_idx == v) return;
    double dx = coords(v, 0) - quad->x_center;
    double dy = coords(v, 1) - quad->y_center;
    double dist = std::sqrt(dx * dx + dy * dy) + 1e-6;
    if (quad->node_idx != -1 || (quad->size / dist) < theta) {
      double force = (k * k * quad->count) / dist;
      fx += (dx / dist) * force;
      fy += (dy / dist) * force;
    } else {
      for (int i = 0; i < 4; ++i) {
        if (quad->children[i]) calculate_repulsion_bh(v, quad->children[i], theta, k, coords, fx, fy);
      }
    }
  }

  inline std::vector<std::vector<int>> make_undirected(const graph_t& g) {
    std::vector<std::vector<int>> undirected(g.num_vertices);
    for (int u = 0; u < g.num_vertices; ++u) {
      for (int v : g.adj_list[u]) {
        undirected[u].push_back(v);
        undirected[v].push_back(u);
      }
    }
    for (int i = 0; i < g.num_vertices; ++i) {
      std::sort(undirected[i].begin(), undirected[i].end());
      undirected[i].erase(std::unique(undirected[i].begin(), undirected[i].end()), undirected[i].end());
    }
    return undirected;
  }

  inline void init_coords(graph_matrix_t& coords, int n) {
    coords.init(n, 2);
  }
}

namespace graph_algo {
  inline std::vector<double> degree(const graph_t& g, const std::string& mode = "total") {
    std::vector<int> deg_int = graph_utils::compute_degrees(g, mode);
    return std::vector<double>(deg_int.begin(), deg_int.end());
  }

  inline std::vector<double> pagerank(const graph_t& g, double d = 0.85, int iter = 20) {
    std::vector<double> pr(g.num_vertices, (g.num_vertices > 0) ? 1.0 / g.num_vertices : 0.0);
    for (int it = 0; it < iter; ++it) {
      std::vector<double> next_pr(g.num_vertices, (g.num_vertices > 0) ? (1.0 - d) / g.num_vertices : 0.0);
      for (int u = 0; u < g.num_vertices; ++u) {
        if (g.adj_list[u].empty()) {
          for (int v = 0; v < g.num_vertices; ++v) next_pr[v] += d * pr[u] / std::max(1, g.num_vertices);
        } else {
          double share = pr[u] / static_cast<double>(g.adj_list[u].size());
          for (int v : g.adj_list[u]) next_pr[v] += d * share;
        }
      }
      pr.swap(next_pr);
    }
    return pr;
  }

  inline std::vector<double> betweenness(const graph_t& g, bool normalized = false, const std::string& mode = "total") {
    std::vector<double> bw(g.num_vertices, 0.0);
    if (g.num_vertices == 0) return bw;
    for (int s = 0; s < g.num_vertices; ++s) {
      std::stack<int> S;
      std::vector<std::vector<int>> P(g.num_vertices);
      std::vector<long long> sigma(g.num_vertices, 0);
      std::vector<int> dist(g.num_vertices, -1);
      std::queue<int> Q;
      sigma[s] = 1; dist[s] = 0; Q.push(s);
      while (!Q.empty()) {
        int v = Q.front(); Q.pop(); S.push(v);
        for (int w : g.adj_list[v]) {
          if (dist[w] < 0) { dist[w] = dist[v] + 1; Q.push(w); }
          if (dist[w] == dist[v] + 1) { sigma[w] += sigma[v]; P[w].push_back(v); }
        }
      }
      std::vector<double> delta(g.num_vertices, 0.0);
      while (!S.empty()) {
        int w = S.top(); S.pop();
        for (int v : P[w]) {
          if (sigma[w] != 0) delta[v] += (static_cast<double>(sigma[v]) / sigma[w]) * (1.0 + delta[w]);
        }
        if (w != s) bw[w] += delta[w];
      }
    }
    if (mode == "in") {
      std::vector<double> in_bw(g.num_vertices, 0.0);
      for (int u = 0; u < g.num_vertices; ++u)
        for (int v : g.adj_list[u]) in_bw[v] += bw[u];
      bw.swap(in_bw);
    } else if (!g.directed && mode != "directed") {
      for (auto &val : bw) val *= 0.5;
    }
    if (normalized && g.num_vertices > 2) {
      double denom = static_cast<double>(g.num_vertices - 1) * (g.num_vertices - 2);
      for (auto &val : bw) val /= denom;
    }
    return bw;
  }

  inline std::vector<int> k_core(const graph_t& g, const std::string& mode = "all") {
    std::vector<int> degrees = graph_utils::compute_degrees(g, (mode == "all") ? "total" : mode);
    std::vector<int> core(g.num_vertices, 0);
    if (g.num_vertices == 0) return core;
    int max_deg = *std::max_element(degrees.begin(), degrees.end());
    std::vector<int> deg = degrees;
    for (int k = 1; k <= max_deg; ++k) {
      std::queue<int> q;
      for (int i = 0; i < g.num_vertices; ++i) if (deg[i] < k && deg[i] > 0) q.push(i);
      while (!q.empty()) {
        int v = q.front(); q.pop();
        if (deg[v] == 0) continue;
        deg[v] = 0;
        for (int nb : g.adj_list[v]) {
          if (deg[nb] >= k) { deg[nb]--; if (deg[nb] < k) q.push(nb); }
        }
      }
      for (int i = 0; i < g.num_vertices; ++i) if (deg[i] >= k) core[i] = k;
    }
    return core;
  }

  inline std::vector<int> connected_components(const graph_t& g, const std::string& mode = "weak") {
    std::vector<int> comp(g.num_vertices, -1);
    if (g.num_vertices == 0) return comp;
    std::vector<std::vector<int>> adj = (mode == "weak" && g.directed) ? graph_utils::make_undirected(g) : g.adj_list;
    int current = 0;
    for (int i = 0; i < g.num_vertices; ++i) {
      if (comp[i] != -1) continue;
      std::queue<int> q; q.push(i); comp[i] = current;
      while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : adj[u]) {
          if (comp[v] == -1) { comp[v] = current; q.push(v); }
        }
      }
      current++;
    }
    return comp;
  }

  inline std::vector<double> clustering_coefficient(const graph_t& g, const std::string& mode = "local") {
    std::vector<double> cc(g.num_vertices, 0.0);
    if (g.num_vertices == 0) return cc;
    std::vector<std::unordered_set<int>> neighbor_sets(g.num_vertices);
    for (int u = 0; u < g.num_vertices; ++u) {
      for (int v : g.adj_list[u]) neighbor_sets[u].insert(v);
    }
    for (int u = 0; u < g.num_vertices; ++u) {
      const auto& neighbors = g.adj_list[u];
      int k = static_cast<int>(neighbors.size());
      if (k < 2) continue;
      int links = 0;
      for (int i = 0; i < k; ++i) {
        for (int j = i + 1; j < k; ++j) {
          int a = neighbors[i], b = neighbors[j];
          if (neighbor_sets[a].count(b)) links++;
          else if (g.directed && neighbor_sets[b].count(a)) links++;
        }
      }
      cc[u] = g.directed ? static_cast<double>(links) / (k * (k - 1)) : static_cast<double>(links * 2) / (k * (k - 1));
    }
    if (mode == "global") {
      double total = 0.0; int count = 0;
      for (double c : cc) { total += c; count++; }
      return std::vector<double>(1, (count > 0) ? total / count : 0.0);
    }
    return cc;
  }
}

namespace graph_layout {
  inline void random(const graph_t& g, graph_matrix_t& coords, double range = 100.0, int seed = 42) {
    graph_utils::init_coords(coords, g.num_vertices);
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dis(-range, range);
    for (int i = 0; i < g.num_vertices; ++i) {
      coords(i, 0) = dis(gen);
      coords(i, 1) = dis(gen);
    }
  }

  inline void circle(const graph_t& g, graph_matrix_t& coords, double radius = 150.0) {
    graph_utils::init_coords(coords, g.num_vertices);
    if (g.num_vertices == 0) return;
    for (int i = 0; i < g.num_vertices; ++i) {
      double a = 2.0 * M_PI * i / g.num_vertices;
      coords(i, 0) = radius * std::cos(a);
      coords(i, 1) = radius * std::sin(a);
    }
  }

  inline void hierarchical(const graph_t& g, graph_matrix_t& coords, int root = 0, double dx = 60, double dy = 100) {
    graph_utils::init_coords(coords, g.num_vertices);
    std::vector<int> depth(g.num_vertices, -1);
    std::vector<int> count_at_depth(g.num_vertices + 1, 0);
    std::queue<int> q;
    if (root < 0 || root >= g.num_vertices) root = 0;
    depth[root] = 0; q.push(root);
    while (!q.empty()) {
      int u = q.front(); q.pop();
      int d = depth[u];
      coords(u, 0) = count_at_depth[d] * dx;
      coords(u, 1) = d * dy;
      count_at_depth[d]++;
      for (int v : g.adj_list[u]) {
        if (depth[v] == -1) { depth[v] = d + 1; q.push(v); }
      }
    }
    int max_d = 0;
    for (int d : depth) if (d > max_d) max_d = d;
    for (int i = 0; i < g.num_vertices; ++i) {
      if (depth[i] == -1) {
        int d = max_d + 1;
        coords(i, 0) = count_at_depth[d] * dx;
        coords(i, 1) = d * dy;
        count_at_depth[d]++;
      }
    }
  }

  inline void fruchterman_reingold(const graph_t& g, graph_matrix_t& coords, int iterations = 100, double temp = 15.0) {
    int n = g.num_vertices;
    if (n == 0) return;
    random(g, coords, 50.0, 42);
    double area = 10000.0 * (n / 10.0);
    double k = std::sqrt(area / n);
    double theta = 0.5;
    std::vector<double> disp_x(n, 0.0), disp_y(n, 0.0);
    for (int iter = 0; iter < iterations; ++iter) {
      std::fill(disp_x.begin(), disp_x.end(), 0.0);
      std::fill(disp_y.begin(), disp_y.end(), 0.0);
      double min_x = coords(0, 0), max_x = coords(0, 0), min_y = coords(0, 1), max_y = coords(0, 1);
      for (int i = 1; i < n; ++i) {
        min_x = std::min(min_x, coords(i, 0)); max_x = std::max(max_x, coords(i, 0));
        min_y = std::min(min_y, coords(i, 1)); max_y = std::max(max_y, coords(i, 1));
      }
      double qsize = std::max(max_x - min_x, max_y - min_y) + 1e-6;
      QuadNode root(min_x, min_y, qsize);
      for (int i = 0; i < n; ++i) root.insert(i, coords(i, 0), coords(i, 1));
      for (int i = 0; i < n; ++i) {
        graph_utils::calculate_repulsion_bh(i, &root, theta, k, coords, disp_x[i], disp_y[i]);
      }
      for (int v = 0; v < n; ++v) {
        for (size_t i = 0; i < g.adj_list[v].size(); ++i) {
          int u = g.adj_list[v][i];
          if (!g.directed && v > u) continue;
          double dx = coords(v, 0) - coords(u, 0), dy = coords(v, 1) - coords(u, 1);
          double d = std::sqrt(dx * dx + dy * dy) + 1e-6;
          double f = (d * d / k) * g.weights[v][i];
          disp_x[v] -= (dx / d) * f; disp_y[v] -= (dy / d) * f;
          disp_x[u] += (dx / d) * f; disp_y[u] += (dy / d) * f;
        }
      }
      for (int i = 0; i < n; ++i) {
        double d = std::sqrt(disp_x[i] * disp_x[i] + disp_y[i] * disp_y[i]) + 1e-6;
        double l = std::min(d, temp);
        coords(i, 0) += (disp_x[i] / d) * l; coords(i, 1) += (disp_y[i] / d) * l;
      }
      temp *= 0.95;
    }
  }
}

inline graph_t inja_to_graph(const inja::Arguments& args) {
  if (args.empty() || args[0]->is_null()) throw std::runtime_error("Graph data is missing");
  const json& j = *args[0];
  if (!j.contains("nodes") || !j["nodes"].is_array()) return graph_t(0);
  graph_t g(static_cast<int>(j["nodes"].size()), j.value("directed", false));
  if (j.contains("edges") && j["edges"].is_array()) {
    for (const auto& e : j["edges"]) {
      try { g.add_edge(e.at("source").get<int>(), e.at("target").get<int>(), e.value("weight", 1.0)); }
      catch (...) {}
    }
  }
  return g;
}

inline json coords_to_json(const graph_t& g, graph_matrix_t& coords) {
  json res = json::array();
  for (int i = 0; i < g.num_vertices; ++i) res.push_back({{"id", i}, {"x", coords(i, 0)}, {"y", coords(i, 1)}});
  return res;
}

inline void register_graph_module(inja::Environment& env) {
  env.add_callback("layout_circle", [](inja::Arguments& args) -> json {
    graph_t g = inja_to_graph(args); graph_matrix_t c;
    graph_layout::circle(g, c, (args.size() > 1) ? args[1]->get<double>() : 150.0);
    return coords_to_json(g, c);
  });
  env.add_callback("layout_random", [](inja::Arguments& args) -> json {
    graph_t g = inja_to_graph(args); graph_matrix_t c;
    graph_layout::random(g, c, (args.size() > 1) ? args[1]->get<double>() : 100.0, (args.size() > 2) ? args[2]->get<int>() : 42);
    return coords_to_json(g, c);
  });
  env.add_callback("layout_force", [](inja::Arguments& args) -> json {
    graph_t g = inja_to_graph(args); graph_matrix_t c;
    graph_layout::fruchterman_reingold(g, c, (args.size() > 1) ? args[1]->get<int>() : 100, (args.size() > 2) ? args[2]->get<double>() : 15.0);
    return coords_to_json(g, c);
  });
  env.add_callback("layout_hierarchical", [](inja::Arguments& args) -> json {
    graph_t g = inja_to_graph(args); graph_matrix_t c;
    graph_layout::hierarchical(g, c, (args.size() > 1) ? args[1]->get<int>() : 0, (args.size() > 2) ? args[2]->get<double>() : 60.0, (args.size() > 3) ? args[3]->get<double>() : 100.0);
    return coords_to_json(g, c);
  });
  env.add_callback("metric_pagerank", [](inja::Arguments& args) -> json {
    return graph_algo::pagerank(inja_to_graph(args), (args.size() > 1) ? args[1]->get<double>() : 0.85, (args.size() > 2) ? args[2]->get<int>() : 20);
  });
  env.add_callback("metric_betweenness", [](inja::Arguments& args) -> json {
    return graph_algo::betweenness(inja_to_graph(args), (args.size() > 1) ? args[1]->get<bool>() : false, (args.size() > 2) ? args[2]->get<std::string>() : "total");
  });
  env.add_callback("metric_degree", [](inja::Arguments& args) -> json {
    return graph_algo::degree(inja_to_graph(args), (args.size() > 1) ? args[1]->get<std::string>() : "total");
  });
  env.add_callback("metric_kcore", [](inja::Arguments& args) -> json {
    return graph_algo::k_core(inja_to_graph(args), (args.size() > 1) ? args[1]->get<std::string>() : "all");
  });
  env.add_callback("metric_components", [](inja::Arguments& args) -> json {
    return graph_algo::connected_components(inja_to_graph(args), (args.size() > 1) ? args[1]->get<std::string>() : "weak");
  });
  env.add_callback("metric_clustering", [](inja::Arguments& args) -> json {
    return graph_algo::clustering_coefficient(inja_to_graph(args), (args.size() > 1) ? args[1]->get<std::string>() : "local");
  });
  env.add_callback("graph_density", [](inja::Arguments& args) -> json {
    graph_t g = inja_to_graph(args);
    if (g.num_vertices < 2) return 0.0;
    double max_e = static_cast<double>(g.num_vertices) * (g.num_vertices - 1);
    if (!g.directed) max_e /= 2.0;
    return static_cast<double>(g.num_edges) / max_e;
  });
}

#if defined(_WIN32) || defined(_WIN64)
  #define EXPORT extern "C" __declspec(dllexport)
#else
  #define EXPORT extern "C"
#endif

EXPORT void register_module(inja::Environment& env) {
    register_graph_module(env);
}

#endif