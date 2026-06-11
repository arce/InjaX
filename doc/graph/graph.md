# GRAPH-MODULE(3)

## NAME

graph-module - Graph algorithms and layout utilities for Inja templates

## SYNOPSIS

```inja
{{ layout_circle(graph, radius) }}
{{ layout_random(graph, range, seed) }}
{{ layout_force(graph, iterations, temperature) }}
{{ layout_hierarchical(graph, root, dx, dy) }}
{{ metric_pagerank(graph, damping, iterations) }}
{{ metric_betweenness(graph, normalized, mode) }}
{{ metric_degree(graph, mode) }}
{{ metric_kcore(graph, mode) }}
{{ metric_components(graph, mode) }}
{{ metric_clustering(graph, mode) }}
{{ graph_density(graph) }}
```

## DESCRIPTION

The graph-module provides functions for graph analysis and node positioning. Layout functions compute 2D coordinates for visualization. Metric functions calculate centrality measures, decomposition values, and structural properties of graphs. All functions return JSON arrays except `graph_density` which returns a single numeric value.

## LAYOUT FUNCTIONS

### layout_circle

Places nodes evenly on a circle. Nodes are ordered by their index around the circumference.

**Parameters:**
- `graph` - Input graph object
- `radius` - Circle radius in pixels (default: 150)

**Returns:** JSON array of `{id, x, y}` coordinates

**Example:**
```inja
{{ layout_circle(graph, 200) }}
```

**Input graph:**
```json
{
  "nodes": ["A", "B", "C", "D"],
  "edges": [{"source": 0, "target": 1}]
}
```

**Output:**
```json
[
  {"id": 0, "x": 200.0, "y": 0.0},
  {"id": 1, "x": 0.0, "y": 200.0},
  {"id": 2, "x": -200.0, "y": 0.0},
  {"id": 3, "x": 0.0, "y": -200.0}
]
```

### layout_random

Places nodes at random positions within a square range.

**Parameters:**
- `graph` - Input graph object
- `range` - Coordinate range from -range to +range (default: 100)
- `seed` - Random seed for reproducibility (default: 42)

**Returns:** JSON array of `{id, x, y}` coordinates

**Example:**
```inja
{{ layout_random(graph, 50, 1234) }}
```

**Input graph:**
```json
{"nodes": ["P", "Q", "R"], "edges": []}
```

**Output:**
```json
[
  {"id": 0, "x": 23.45, "y": -12.78},
  {"id": 1, "x": -45.12, "y": 34.56},
  {"id": 2, "x": 12.89, "y": -23.45}
]
```

### layout_force

Force-directed layout using Fruchterman-Reingold algorithm. Nodes repel each other while connected edges create attraction. Optimized with Barnes-Hut for O(n log n) performance. Supports weighted edges.

**Parameters:**
- `graph` - Input graph object
- `iterations` - Simulation steps (default: 100)
- `temperature` - Initial movement amount (default: 15)

**Returns:** JSON array of `{id, x, y}` coordinates

**Example:**
```inja
{{ layout_force(graph, 150, 20) }}
```

**Input graph:**
```json
{
  "nodes": ["Center", "Left", "Right"],
  "edges": [
    {"source": 0, "target": 1, "weight": 2.0},
    {"source": 0, "target": 2, "weight": 2.0}
  ]
}
```

**Output:**
```json
[
  {"id": 0, "x": 2.34, "y": -1.56},
  {"id": 1, "x": -45.23, "y": 3.12},
  {"id": 2, "x": 48.91, "y": -2.78}
]
```

### layout_hierarchical

Tree-style layout based on BFS traversal from a root node. Nodes at same depth share the same Y coordinate.

**Parameters:**
- `graph` - Input graph object
- `root` - Starting node index (default: 0)
- `dx` - Horizontal spacing between siblings (default: 60)
- `dy` - Vertical spacing between levels (default: 100)

**Returns:** JSON array of `{id, x, y}` coordinates

**Example:**
```inja
{{ layout_hierarchical(graph, 0, 80, 120) }}
```

**Input graph:**
```json
{
  "nodes": ["Root", "Child", "Grandchild"],
  "edges": [
    {"source": 0, "target": 1},
    {"source": 1, "target": 2}
  ],
  "directed": true
}
```

**Output:**
```json
[
  {"id": 0, "x": 0.0, "y": 0.0},
  {"id": 1, "x": 80.0, "y": 120.0},
  {"id": 2, "x": 80.0, "y": 240.0}
]
```

## METRIC FUNCTIONS

### metric_pagerank

Computes PageRank centrality. Measures node importance based on incoming links from other important nodes. Higher values indicate more influential nodes.

**Parameters:**
- `graph` - Input graph object
- `damping` - Probability of following links (default: 0.85)
- `iterations` - Number of iterations (default: 20)

**Returns:** JSON array of PageRank scores (sum = 1.0)

**Example:**
```inja
{{ metric_pagerank(graph, 0.85, 30) }}
```

**Input graph:**
```json
{
  "nodes": ["Home", "About", "Contact", "Blog"],
  "edges": [
    {"source": 0, "target": 1},
    {"source": 0, "target": 2},
    {"source": 0, "target": 3},
    {"source": 3, "target": 0}
  ],
  "directed": true
}
```

**Output:**
```json
[0.405, 0.198, 0.198, 0.199]
```

### metric_betweenness

Computes betweenness centrality. Measures how often a node lies on shortest paths between other nodes. Useful for identifying bridges or bottlenecks.

**Parameters:**
- `graph` - Input graph object
- `normalized` - Scale scores to [0,1] (default: false)
- `mode` - `"total"`, `"in"`, `"out"`, or `"directed"` (default: "total")

**Returns:** JSON array of betweenness scores

**Example:**
```inja
{{ metric_betweenness(graph, true, "total") }}
```

**Input graph:**
```json
{
  "nodes": ["A", "Bridge", "C", "D"],
  "edges": [
    {"source": 0, "target": 1},
    {"source": 1, "target": 2},
    {"source": 1, "target": 3}
  ]
}
```

**Output:**
```json
[0.0, 0.5, 0.0, 0.0]
```

### metric_degree

Computes degree centrality. Counts connections per node. For directed graphs, can measure in-degree, out-degree, or total.

**Parameters:**
- `graph` - Input graph object
- `mode` - `"total"`, `"in"`, or `"out"` (default: "total")

**Returns:** JSON array of degree values

**Example:**
```inja
{{ metric_degree(graph, "in") }}
```

**Input graph:**
```json
{
  "nodes": ["X", "Y", "Z"],
  "edges": [
    {"source": 0, "target": 2},
    {"source": 1, "target": 2},
    {"source": 2, "target": 0}
  ],
  "directed": true
}
```

**Output:**
```json
[1, 1, 1]
```

### metric_kcore

Computes k-core decomposition. Each node gets the maximum k such that it belongs to a subgraph where all nodes have degree at least k. Higher core numbers indicate more central, well-connected nodes.

**Parameters:**
- `graph` - Input graph object
- `mode` - `"all"`, `"in"`, or `"out"` (default: "all")

**Returns:** JSON array of core numbers

**Example:**
```inja
{{ metric_kcore(graph, "all") }}
```

**Input graph:**
```json
{
  "nodes": ["A", "B", "C", "D", "E"],
  "edges": [
    {"source": 0, "target": 1}, {"source": 1, "target": 2},
    {"source": 2, "target": 0}, {"source": 1, "target": 3},
    {"source": 3, "target": 4}
  ]
}
```

**Output:**
```json
[2, 2, 2, 1, 1]
```

### metric_components

Finds connected components. Nodes in the same component are reachable from each other. For directed graphs, "weak" mode treats edges as undirected.

**Parameters:**
- `graph` - Input graph object
- `mode` - `"weak"` or `"strong"` (default: "weak")

**Returns:** JSON array of component IDs per node

**Example:**
```inja
{{ metric_components(graph, "weak") }}
```

**Input graph:**
```json
{
  "nodes": ["A", "B", "C", "D", "E"],
  "edges": [
    {"source": 0, "target": 1},
    {"source": 1, "target": 2},
    {"source": 3, "target": 4}
  ]
}
```

**Output:**
```json
[0, 0, 0, 1, 1]
```

### metric_clustering

Computes clustering coefficient. Measures how well connected a node's neighbors are to each other. Local mode returns per-node values (0 = no connections between neighbors, 1 = all neighbors connected). Global mode returns the average.

**Parameters:**
- `graph` - Input graph object
- `mode` - `"local"` or `"global"` (default: "local")

**Returns:** JSON array of coefficients

**Example (local mode):**
```inja
{{ metric_clustering(graph, "local") }}
```

**Input graph:**
```json
{
  "nodes": ["Triangle", "Line", "Star"],
  "edges": [
    {"source": 0, "target": 1}, {"source": 1, "target": 2}, {"source": 2, "target": 0},
    {"source": 3, "target": 4},
    {"source": 5, "target": 6}, {"source": 5, "target": 7}, {"source": 5, "target": 8}
  ]
}
```

**Output:**
```json
[1.0, 0.0, 0.0]
```

**Example (global mode):**
```inja
{{ metric_clustering(graph, "global") }}
```

**Output:**
```json
[0.33]
```

### graph_density

Computes graph density. Ratio of actual edges to maximum possible edges. Dense graphs have many connections; sparse graphs have few.

**Parameters:**
- `graph` - Input graph object

**Returns:** Single numeric value

**Example:**
```inja
{{ graph_density(graph) }}
```

**Input graph:**
```json
{
  "nodes": ["A", "B", "C"],
  "edges": [
    {"source": 0, "target": 1},
    {"source": 0, "target": 2},
    {"source": 1, "target": 2}
  ]
}
```

**Output:**
```
1.0
```