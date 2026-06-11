// geo_module.hpp
#ifndef GEO_MODULE_H
#define GEO_MODULE_H

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <stdexcept>

using json = nlohmann::json;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Point {
  double x = 0.0;
  double y = 0.0;

  Point() = default;
  Point(double x_val, double y_val) : x(x_val), y(y_val) {}

  Point operator-(const Point& other) const { return {x - other.x, y - other.y}; }
  Point operator+(const Point& other) const { return {x + other.x, y + other.y}; }
  
  double lengthSq() const { return x * x + y * y; }
  double length() const { return std::sqrt(lengthSq()); }
};

struct Vertex {
  Point p;
  int id = -1;
  bool isIntersection = false;
  bool entry = false;
  int neighbor = -1;

  Vertex() = default;
  Vertex(Point pt, int idx) : p(pt), id(idx) {}
};

using Polygon = std::vector<Point>;
using VertexList = std::vector<Vertex>;

class GreinerHormann {
private:
  VertexList vertices1, vertices2;
  static constexpr double EPSILON = 1e-9;
  
  static double orientation(const Point& a, const Point& b, const Point& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  }
  
  static bool onSegment(const Point& p, const Point& q, const Point& r) {
    return q.x <= std::max(p.x, r.x) + EPSILON && q.x >= std::min(p.x, r.x) - EPSILON &&
           q.y <= std::max(p.y, r.y) + EPSILON && q.y >= std::min(p.y, r.y) - EPSILON;
  }
  
  static bool segmentsIntersect(const Point& p1, const Point& p2, 
                                const Point& q1, const Point& q2, 
                                Point& intersection) {
    double o1 = orientation(p1, p2, q1);
    double o2 = orientation(p1, p2, q2);
    double o3 = orientation(q1, q2, p1);
    double o4 = orientation(q1, q2, p2);
    
    if (std::abs(o1) < EPSILON && onSegment(p1, q1, p2)) { intersection = q1; return true; }
    if (std::abs(o2) < EPSILON && onSegment(p1, q2, p2)) { intersection = q2; return true; }
    if (std::abs(o3) < EPSILON && onSegment(q1, p1, q2)) { intersection = p1; return true; }
    if (std::abs(o4) < EPSILON && onSegment(q1, p2, q2)) { intersection = p2; return true; }
    
    if (((o1 > EPSILON && o2 < -EPSILON) || (o1 < -EPSILON && o2 > EPSILON)) && 
        ((o3 > EPSILON && o4 < -EPSILON) || (o3 < -EPSILON && o4 > EPSILON))) {
      
      double A1 = p2.y - p1.y, B1 = p1.x - p2.x, C1 = A1 * p1.x + B1 * p1.y;
      double A2 = q2.y - q1.y, B2 = q1.x - q2.x, C2 = A2 * q1.x + B2 * q1.y;
      
      double det = A1 * B2 - A2 * B1;
      if (std::abs(det) < EPSILON) return false;
      
      intersection.x = (B2 * C1 - B1 * C2) / det;
      intersection.y = (A1 * C2 - A2 * C1) / det;
      return true;
    }
    return false;
  }
  
  static bool pointInPolygon(const Point& point, const VertexList& polygon) {
    bool inside = false;
    int n = polygon.size();
    for (int i = 0, j = n - 1; i < n; j = i++) {
      const Point& p1 = polygon[i].p;
      const Point& p2 = polygon[j].p;
      
      if (((p1.y > point.y) != (p2.y > point.y))) {
        double intersectX = (p2.x - p1.x) * (point.y - p1.y) / (p2.y - p1.y) + p1.x;
        if (point.x < intersectX) {
          inside = !inside;
        }
      }
    }
    return inside;
  }

  struct IntersectNode {
    Point pt;
    double t1;
    double t2;
    size_t edge1;
    size_t edge2;
  };

  void insertIntersections() {
    std::vector<IntersectNode> inters;
    size_t n1 = vertices1.size();
    size_t n2 = vertices2.size();

    for (size_t i = 0; i < n1; i++) {
      for (size_t j = 0; j < n2; j++) {
        Point inter;
        if (segmentsIntersect(vertices1[i].p, vertices1[(i + 1) % n1].p,
                              vertices2[j].p, vertices2[(j + 1) % n2].p, inter)) {
          
          double len1_sq = (vertices1[(i + 1) % n1].p - vertices1[i].p).lengthSq();
          double len2_sq = (vertices2[(j + 1) % n2].p - vertices2[j].p).lengthSq();
          
          double t1 = (inter - vertices1[i].p).lengthSq() / (len1_sq < EPSILON ? 1.0 : len1_sq);
          double t2 = (inter - vertices2[j].p).lengthSq() / (len2_sq < EPSILON ? 1.0 : len2_sq);
          t1 = std::sqrt(t1);
          t2 = std::sqrt(t2);

          if (t1 > EPSILON && t1 < 1.0 - EPSILON && t2 > EPSILON && t2 < 1.0 - EPSILON) {
            inters.push_back({inter, t1, t2, i, j});
          }
        }
      }
    }

    VertexList newVertices1;
    for (size_t i = 0; i < n1; i++) {
      newVertices1.push_back(vertices1[i]);
      std::vector<IntersectNode> localInters;
      for (const auto& in : inters) {
        if (in.edge1 == i) localInters.push_back(in);
      }
      std::sort(localInters.begin(), localInters.end(), [](const IntersectNode& a, const IntersectNode& b) {
        return a.t1 < b.t1;
      });
      for (const auto& in : localInters) {
        Vertex v(in.pt, -1);
        v.isIntersection = true;
        newVertices1.push_back(v);
      }
    }

    VertexList newVertices2;
    for (size_t j = 0; j < n2; j++) {
      newVertices2.push_back(vertices2[j]);
      std::vector<IntersectNode> localInters;
      for (const auto& in : inters) {
        if (in.edge2 == j) localInters.push_back(in);
      }
      std::sort(localInters.begin(), localInters.end(), [](const IntersectNode& a, const IntersectNode& b) {
        return a.t2 < b.t2;
      });
      for (const auto& in : localInters) {
        Vertex v(in.pt, -1);
        v.isIntersection = true;
        newVertices2.push_back(v);
      }
    }

    vertices1 = newVertices1;
    vertices2 = newVertices2;

    for (size_t i = 0; i < vertices1.size(); i++) {
      if (vertices1[i].isIntersection) {
        for (size_t j = 0; j < vertices2.size(); j++) {
          if (vertices2[j].isIntersection && 
              (vertices1[i].p - vertices2[j].p).lengthSq() < EPSILON) {
            vertices1[i].neighbor = static_cast<int>(j);
            vertices2[j].neighbor = static_cast<int>(i);
            break;
          }
        }
      }
    }
  }
  
  void markEntryExit(bool unionOp) {
    if (vertices1.empty() || vertices2.empty()) return;
    
    bool inside = pointInPolygon(vertices1[0].p, vertices2);
    for (size_t i = 0; i < vertices1.size(); i++) {
      if (vertices1[i].isIntersection) {
        vertices1[i].entry = unionOp ? inside : !inside;
        inside = !inside;
      }
    }
    
    inside = pointInPolygon(vertices2[0].p, vertices1);
    for (size_t i = 0; i < vertices2.size(); i++) {
      if (vertices2[i].isIntersection) {
        vertices2[i].entry = unionOp ? inside : !inside;
        inside = !inside;
      }
    }
  }
  
  std::vector<Polygon> extractPolygons(bool unionOp) {
    std::vector<Polygon> result;
    std::vector<bool> visited1(vertices1.size(), false);
    
    for (size_t i = 0; i < vertices1.size(); i++) {
      if (vertices1[i].isIntersection && !visited1[i]) {
        Polygon poly;
        size_t idx = i;
        int currentPoly = 0; 
        
        while (true) {
          if (currentPoly == 0) {
            visited1[idx] = true;
            poly.push_back(vertices1[idx].p);
            
            if (vertices1[idx].isIntersection) {
              int n2_idx = vertices1[idx].neighbor;
              idx = static_cast<size_t>(n2_idx);
              currentPoly = 1; 
              bool forward = unionOp ? !vertices2[idx].entry : vertices2[idx].entry;
              idx = forward ? (idx + 1) % vertices2.size() : (idx + vertices2.size() - 1) % vertices2.size();
            } else {
              idx = (idx + 1) % vertices1.size();
            }
          } else {
            int n1_idx = vertices2[idx].neighbor;
            if (n1_idx != -1) { 
              visited1[static_cast<size_t>(n1_idx)] = true;
              poly.push_back(vertices2[idx].p);
              idx = static_cast<size_t>(n1_idx);
              currentPoly = 0; 
              bool forward = unionOp ? vertices1[idx].entry : !vertices1[idx].entry;
              idx = forward ? (idx + 1) % vertices1.size() : (idx + vertices1.size() - 1) % vertices1.size();
            } else {
              poly.push_back(vertices2[idx].p);
              idx = (idx + 1) % vertices2.size();
            }
          }
          if (currentPoly == 0 && idx == i) break;
        }
        
        if (poly.size() >= 3) {
          result.push_back(poly);
        }
      }
    }
    return result;
  }
  
public:
  GreinerHormann(const Polygon& poly1, const Polygon& poly2) {
    for (size_t i = 0; i < poly1.size(); i++) vertices1.emplace_back(poly1[i], static_cast<int>(i));
    for (size_t i = 0; i < poly2.size(); i++) vertices2.emplace_back(poly2[i], static_cast<int>(i));
  }
  
  std::vector<Polygon> intersection() {
    insertIntersections();
    markEntryExit(false); 
    return extractPolygons(false);
  }
  
  std::vector<Polygon> unionOp() {
    insertIntersections();
    markEntryExit(true); 
    return extractPolygons(true);
  }
  
  std::vector<Polygon> difference() {
    insertIntersections();
    markEntryExit(false);
    for (auto& v : vertices2) {
      if (v.isIntersection) v.entry = !v.entry;
    }
    return extractPolygons(false);
  }
};

class SpatialPredicates {
private:
  static constexpr double BASE_EPS = 1e-9;

  inline double relEps(const std::vector<Point>& pts) const {
    double maxCoord = 0.0;
    for (const auto& p : pts) {
      maxCoord = std::max(maxCoord, std::abs(p.x));
      maxCoord = std::max(maxCoord, std::abs(p.y));
    }
    return BASE_EPS * std::max(1.0, maxCoord);
  }

  inline Polygon normalizePolygon(const Polygon& poly) const {
    Polygon out;
    int n = static_cast<int>(poly.size());
    if (n == 0) return out;
    for (int i = 0; i < n; ++i) {
      const Point& p = poly[i];
      if (out.empty() || (std::abs(out.back().x - p.x) > BASE_EPS || std::abs(out.back().y - p.y) > BASE_EPS)) {
        out.push_back(p);
      }
    }
    if (out.size() > 1) {
      const Point& f = out.front();
      const Point& l = out.back();
      if (std::abs(f.x - l.x) < BASE_EPS && std::abs(f.y - l.y) < BASE_EPS) {
        out.pop_back();
      }
    }
    Polygon res;
    int m = static_cast<int>(out.size());
    if (m == 0) return res;
    for (int i = 0; i < m; ++i) {
      const Point& a = out[(i + m - 1) % m];
      const Point& b = out[i];
      const Point& c = out[(i + 1) % m];
      double cross = (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);
      if (std::abs(cross) > BASE_EPS) {
        res.push_back(b);
      }
    }
    if (res.size() < 3) return Polygon{};
    return res;
  }

  struct BBox {
    double minX, minY, maxX, maxY;
    BBox() : minX(0), minY(0), maxX(0), maxY(0) {}
    BBox(const Polygon& poly) {
      minX = minY = std::numeric_limits<double>::infinity();
      maxX = maxY = -std::numeric_limits<double>::infinity();
      for (const auto& p : poly) {
        minX = std::min(minX, p.x);
        minY = std::min(minY, p.y);
        maxX = std::max(maxX, p.x);
        maxY = std::max(maxY, p.y);
      }
    }
    inline bool intersects(const BBox& other, double eps) const {
      return !(maxX < other.minX - eps ||
               minX > other.maxX + eps ||
               maxY < other.minY - eps ||
               minY > other.maxY + eps);
    }
    inline bool contains(const BBox& other, double eps) const {
      return minX <= other.minX + eps &&
             maxX >= other.maxX - eps &&
             minY <= other.minY + eps &&
             maxY >= other.maxY - eps;
    }
  };

  inline double distance(const Point& a, const Point& b) const {
    double dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
  }

  inline int orientation(const Point& a, const Point& b, const Point& c, double eps) const {
    double val = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
    if (std::abs(val) <= eps) return 0;
    return (val > 0) ? 1 : -1;
  }

  inline bool pointOnSegment(const Point& p, const Point& a, const Point& b, double eps) const {
    double cross = (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
    if (std::abs(cross) > eps) return false;
    double dot = (p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y);
    if (dot < -eps) return false;
    double squaredLen = (b.x - a.x)*(b.x - a.x) + (b.y - a.y)*(b.y - a.y);
    return dot <= squaredLen + eps;
  }

  int pointInPolygonExtended(const Point& p, const Polygon& poly, double eps) const {
    if (pointOnBoundary(p, poly, eps)) return 0;
    bool inside = false;
    int n = static_cast<int>(poly.size());
    for (int i = 0, j = n - 1; i < n; j = i++) {
      const Point& pi = poly[i];
      const Point& pj = poly[j];
      bool intersect = ((pi.y > p.y) != (pj.y > p.y));
      if (intersect) {
        double xint = pj.x + (pi.x - pj.x) * (p.y - pj.y) / (pi.y - pj.y);
        if (p.x < xint - eps) inside = !inside;
      }
    }
    return inside ? 1 : -1;
  }

  bool pointOnBoundary(const Point& p, const Polygon& poly, double eps) const {
    int n = static_cast<int>(poly.size());
    for (int i = 0; i < n; ++i) {
      const Point& a = poly[i];
      const Point& b = poly[(i + 1) % n];
      if (pointOnSegment(p, a, b, eps)) return true;
    }
    return false;
  }

  bool segmentsIntersect(const Point& p1, const Point& p2,
                         const Point& q1, const Point& q2,
                         Point& inter, double eps) const {
    int o1 = orientation(p1, p2, q1, eps);
    int o2 = orientation(p1, p2, q2, eps);
    int o3 = orientation(q1, q2, p1, eps);
    int o4 = orientation(q1, q2, p2, eps);

    if (o1 == 0 && pointOnSegment(q1, p1, p2, eps)) { inter = q1; return true; }
    if (o2 == 0 && pointOnSegment(q2, p1, p2, eps)) { inter = q2; return true; }
    if (o3 == 0 && pointOnSegment(p1, q1, q2, eps)) { inter = p1; return true; }
    if (o4 == 0 && pointOnSegment(p2, q1, q2, eps)) { inter = p2; return true; }

    if (o1 != o2 && o3 != o4) {
      double A1 = p2.y - p1.y;
      double B1 = p1.x - p2.x;
      double C1 = A1 * p1.x + B1 * p1.y;

      double A2 = q2.y - q1.y;
      double B2 = q1.x - q2.x;
      double C2 = A2 * q1.x + B2 * q1.y;

      double det = A1 * B2 - A2 * B1;
      if (std::abs(det) <= eps) return true;
      inter.x = (B2 * C1 - B1 * C2) / det;
      inter.y = (A1 * C2 - A2 * C1) / det;
      return true;
    }
    return false;
  }

public:
  bool disjoint(const Polygon& a_in, const Polygon& b_in) const {
    Polygon a = normalizePolygon(a_in);
    Polygon b = normalizePolygon(b_in);
    if (a.empty() || b.empty()) return true;
    double eps = relEps(a_in.size() ? a_in : b_in);
    BBox A(a), B(b);
    if (!A.intersects(B, eps)) return true;
    return !intersects(a, b);
  }

  bool touches(const Polygon& a_in, const Polygon& b_in) const {
    Polygon a = normalizePolygon(a_in);
    Polygon b = normalizePolygon(b_in);
    if (a.empty() || b.empty()) return false;
    double eps = relEps(a_in.size() ? a_in : b_in);
    BBox A(a), B(b);
    if (!A.intersects(B, eps)) return false;

    bool boundaryContact = false;
    int n = static_cast<int>(a.size()), m = static_cast<int>(b.size());
    for (int i = 0; i < n && !boundaryContact; ++i) {
      for (int j = 0; j < m; ++j) {
        Point inter;
        if (segmentsIntersect(a[i], a[(i+1)%n], b[j], b[(j+1)%m], inter, eps)) {
          boundaryContact = true;
          break;
        }
      }
    }
    if (!boundaryContact) return false;

    for (const auto& p : a) {
      if (pointInPolygonExtended(p, b, eps) == 1) return false;
    }
    for (const auto& q : b) {
      if (pointInPolygonExtended(q, a, eps) == 1) return false;
    }
    return true;
  }

  bool crosses(const Polygon& a_in, const Polygon& b_in) const {
    Polygon a = normalizePolygon(a_in);
    Polygon b = normalizePolygon(b_in);
    if (a.empty() || b.empty()) return false;
    double eps = relEps(a_in.size() ? a_in : b_in);
    BBox A(a), B(b);
    if (!A.intersects(B, eps)) return false;

    int intersectionPoints = 0;
    int n = static_cast<int>(a.size()), m = static_cast<int>(b.size());
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < m; ++j) {
        Point inter;
        if (segmentsIntersect(a[i], a[(i+1)%n], b[j], b[(j+1)%m], inter, eps)) {
          bool aAtEndpoint = (distance(inter, a[i]) <= eps || distance(inter, a[(i+1)%n]) <= eps);
          bool bAtEndpoint = (distance(inter, b[j]) <= eps || distance(inter, b[(j+1)%m]) <= eps);
          if (!aAtEndpoint && !bAtEndpoint) intersectionPoints++;
        }
      }
    }
    if (intersectionPoints == 0) return false;
    return !within(a, b) && !within(b, a);
  }

  bool within(const Polygon& a_in, const Polygon& b_in) const {
    Polygon a = normalizePolygon(a_in);
    Polygon b = normalizePolygon(b_in);
    if (a.empty() || b.empty()) return false;
    double eps = relEps(a_in.size() ? a_in : b_in);
    BBox A(a), B(b);
    if (!B.contains(A, eps)) return false;

    for (const auto& p : a) {
      int res = pointInPolygonExtended(p, b, eps);
      if (res == -1) return false;
    }

    int n = static_cast<int>(a.size()), m = static_cast<int>(b.size());
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < m; ++j) {
        Point inter;
        if (segmentsIntersect(a[i], a[(i+1)%n], b[j], b[(j+1)%m], inter, eps)) {
          if (distance(inter, a[i]) > eps && distance(inter, a[(i+1)%n]) > eps) {
            return false;
          }
        }
      }
    }
    return true;
  }

  bool contains(const Polygon& a_in, const Polygon& b_in) const {
    return within(b_in, a_in);
  }

  bool overlaps(const Polygon& a_in, const Polygon& b_in) const {
    Polygon a = normalizePolygon(a_in);
    Polygon b = normalizePolygon(b_in);
    if (a.empty() || b.empty()) return false;
    double eps = relEps(a_in.size() ? a_in : b_in);
    BBox A(a), B(b);
    if (!A.intersects(B, eps)) return false;

    GreinerHormann gh(a, b);
    auto intersection = gh.intersection();
    if (intersection.empty()) return false;

    double overlapArea = 0.0;
    for (const auto& poly : intersection) {
      overlapArea += polygonArea(poly);
    }
    double areaA = polygonArea(a);
    double areaB = polygonArea(b);

    return (overlapArea > eps) && (overlapArea < areaA - eps) && (overlapArea < areaB - eps);
  }

  bool equals(const Polygon& a_in, const Polygon& b_in) const {
    Polygon a = normalizePolygon(a_in);
    Polygon b = normalizePolygon(b_in);
    if (a.size() != b.size()) return false;
    double eps = relEps(a_in.size() ? a_in : b_in);
    int n = static_cast<int>(a.size());

    for (int offset = 0; offset < n; ++offset) {
      bool match = true;
      for (int i = 0; i < n; ++i) {
        if (distance(a[i], b[(i + offset) % n]) > eps) { match = false; break; }
      }
      if (match) return true;
    }
    for (int offset = 0; offset < n; ++offset) {
      bool match = true;
      for (int i = 0; i < n; ++i) {
        int idx = (n + offset - i) % n;
        if (distance(a[i], b[idx]) > eps) { match = false; break; }
      }
      if (match) return true;
    }
    return false;
  }

  bool intersects(const Polygon& a_in, const Polygon& b_in) const {
    Polygon a = normalizePolygon(a_in);
    Polygon b = normalizePolygon(b_in);
    if (a.empty() || b.empty()) return false;
    double eps = relEps(a_in.size() ? a_in : b_in);
    BBox A(a), B(b);
    if (!A.intersects(B, eps)) return false;

    int n = static_cast<int>(a.size()), m = static_cast<int>(b.size());
    for (int i = 0; i < n; ++i) {
      for (int j = 0; j < m; ++j) {
        Point inter;
        if (segmentsIntersect(a[i], a[(i+1)%n], b[j], b[(j+1)%m], inter, eps)) return true;
      }
    }
    for (const auto& p : a) {
      if (pointInPolygonExtended(p, b, eps) != -1) return true;
    }
    for (const auto& p : b) {
      if (pointInPolygonExtended(p, a, eps) != -1) return true;
    }
    return false;
  }

  double polygonArea(const Polygon& poly) const {
    double area = 0.0;
    int n = static_cast<int>(poly.size());
    if (n < 3) return 0.0;
    for (int i = 0; i < n; ++i) {
      const Point& p = poly[i];
      const Point& q = poly[(i + 1) % n];
      area += p.x * q.y - q.x * p.y;
    }
    return std::abs(area) * 0.5;
  }
};

namespace geo_utils {
  
  inline Polygon json_to_polygon(const json& j) {
    if (!j.is_array()) throw std::runtime_error("Polygon must be an array of points");
    Polygon poly;
    for (const auto& pt : j) {
      if (pt.is_array() && pt.size() >= 2) {
        poly.emplace_back(pt[0].get<double>(), pt[1].get<double>());
      } else if (pt.is_object() && pt.contains("x") && pt.contains("y")) {
        poly.emplace_back(pt["x"].get<double>(), pt["y"].get<double>());
      }
    }
    return poly;
  }

  inline json polygons_to_json(const std::vector<Polygon>& polys) {
    json res = json::array();
    for (const auto& poly : polys) {
      json curr_poly = json::array();
      for (const auto& pt : poly) {
        curr_poly.push_back({{"x", pt.x}, {"y", pt.y}});
      }
      res.push_back(curr_poly);
    }
    return res;
  }

  inline std::pair<Polygon, Polygon> extract_two_polygons(const inja::Arguments& args) {
    if (args.size() < 2 || args[0]->is_null() || args[1]->is_null()) {
      throw std::runtime_error("Two valid polygons are required as arguments");
    }
    return {json_to_polygon(*args[0]), json_to_polygon(*args[1])};
  }
}

namespace geo_algo {
  
  inline double area(const Polygon& poly) {
    double a = 0.0;
    int n = static_cast<int>(poly.size());
    if (n < 3) return 0.0;
    for (int i = 0, j = n - 1; i < n; j = i++) {
      a += (poly[j].x + poly[i].x) * (poly[j].y - poly[i].y);
    }
    return std::abs(a * 0.5);
  }

  inline double perimeter(const Polygon& poly) {
    double p = 0.0;
    int n = static_cast<int>(poly.size());
    if (n < 2) return 0.0;
    for (int i = 0; i < n; i++) {
      p += (poly[(i + 1) % n] - poly[i]).length();
    }
    return p;
  }
}

inline void register_geo_module(inja::Environment& env) {
  
  env.add_callback("geo_intersection", [](inja::Arguments& args) -> json {
    auto [p1, p2] = geo_utils::extract_two_polygons(args);
    GreinerHormann gh(p1, p2);
    return geo_utils::polygons_to_json(gh.intersection());
  });

  env.add_callback("geo_union", [](inja::Arguments& args) -> json {
    auto [p1, p2] = geo_utils::extract_two_polygons(args);
    GreinerHormann gh(p1, p2);
    return geo_utils::polygons_to_json(gh.unionOp());
  });

  env.add_callback("geo_difference", [](inja::Arguments& args) -> json {
    auto [p1, p2] = geo_utils::extract_two_polygons(args);
    GreinerHormann gh(p1, p2);
    return geo_utils::polygons_to_json(gh.difference());
  });

  env.add_callback("geo_area", [](inja::Arguments& args) -> json {
    if (args.empty() || args[0]->is_null()) return 0.0;
    return geo_algo::area(geo_utils::json_to_polygon(*args[0]));
  });

  env.add_callback("geo_perimeter", [](inja::Arguments& args) -> json {
    if (args.empty() || args[0]->is_null()) return 0.0;
    return geo_algo::perimeter(geo_utils::json_to_polygon(*args[0]));
  });

  SpatialPredicates sp;

  env.add_callback("geo_disjoint", [&sp](inja::Arguments& args) -> json {
    auto [p1, p2] = geo_utils::extract_two_polygons(args);
    return sp.disjoint(p1, p2);
  });

  env.add_callback("geo_touches", [&sp](inja::Arguments& args) -> json {
    auto [p1, p2] = geo_utils::extract_two_polygons(args);
    return sp.touches(p1, p2);
  });

  env.add_callback("geo_crosses", [&sp](inja::Arguments& args) -> json {
    auto [p1, p2] = geo_utils::extract_two_polygons(args);
    return sp.crosses(p1, p2);
  });

  env.add_callback("geo_within", [&sp](inja::Arguments& args) -> json {
    auto [p1, p2] = geo_utils::extract_two_polygons(args);
    return sp.within(p1, p2);
  });

  env.add_callback("geo_contains", [&sp](inja::Arguments& args) -> json {
    auto [p1, p2] = geo_utils::extract_two_polygons(args);
    return sp.contains(p1, p2);
  });

  env.add_callback("geo_overlaps", [&sp](inja::Arguments& args) -> json {
    auto [p1, p2] = geo_utils::extract_two_polygons(args);
    return sp.overlaps(p1, p2);
  });

  env.add_callback("geo_equals", [&sp](inja::Arguments& args) -> json {
    auto [p1, p2] = geo_utils::extract_two_polygons(args);
    return sp.equals(p1, p2);
  });

  env.add_callback("geo_intersects", [&sp](inja::Arguments& args) -> json {
    auto [p1, p2] = geo_utils::extract_two_polygons(args);
    return sp.intersects(p1, p2);
  });
}

#if defined(_WIN32) || defined(_WIN64)
  #define EXPORT extern "C" __declspec(dllexport)
#else
  #define EXPORT extern "C"
#endif

EXPORT void register_module(inja::Environment& env) {
    register_geo_module(env);
}

#endif