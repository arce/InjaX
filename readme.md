# InjaX

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)](https://isocpp.org/)

**InjaX** is a modern, high-performance template engine for C++ that brings the elegance of Jinja2 syntax to the C++ ecosystem. Use it as a standalone CLI tool.

## Quick Start

```bash
# Single file mode: render one template with one data file
injax data.json template.inja output.html

# Directory mode: batch render entire sites (Hugo-like)
injax data/ templates/ output/

```

## Why InjaX?

* **Familiar syntax** — Uses Jinja2-style delimiters (`{{ }}`, `{% %}`, `{# #}`)
* **Fast** — Compiled to efficient static binaries, no interpreter overhead
* **Multi-format data** — JSON natively, plus plugins for CSV, YAML, XML, MKD, and INI
* **Extensible** — Plugin system for custom modules
* **Batch processing** — Render entire websites from directories of content and templates
* **Smart output formatting** — Template filenames determine output format

## Usage Modes

### Mode 1: Single File Processing

Process one data file with one template to produce a single output file.

```bash
injax <data_file> <template_file> <output_file>

```

**Example:**

```bash
injax data.json template.inja output.html

```

### Mode 2: Directory Mode

Process all data files in a directory with all templates found in the template hierarchy, preserving the directory structure in the output.

```bash
injax <data_directory> <template_directory> <output_directory>

```

## Template Naming Convention

InjaX uses a smart naming convention for templates that determines the output file format in directory mode.

### Basic Templates

All template files must have the **`.inja` extension** and contain a **hyphen (`-`)** in the filename before the extension.

**Syntax:** `{basename}-{output_format}.inja`

| Template Name | Output File | Description |
| --- | --- | --- |
| `index-html.inja` | `index.html` | Creates HTML index file |
| `product-html.inja` | `{data_stem}.html` | Creates individual HTML files |
| `list-md.inja` | `list.md` | Creates Markdown list file |
| `post-xml.inja` | `post.xml` | Creates XML file |

### How It Works

The template filename is parsed as follows:

* Everything before the last hyphen (`-`) is the base name
* Everything between the last hyphen and the dot (`.inja`) is the output format

```
index-html.inja
├── base: "index"
└── output_format: "html"

product-xml.inja
├── base: "product"
└── output_format: "xml"

```

## Directory Structure & Inheritance

InjaX features an advanced hierarchical template lookup mechanism. Instead of requiring a strict, identical 1:1 matching folder structure between content and templates, the engine resolves the optimal template via **upstream inheritance**.

### Complete Working Example

```
data/                            templates/                        output/
│                                │                                 │
├── posts/                       ├── posts/                        ├── posts/
│   ├── post1.json               │   ├── index-html.inja           │   ├── index.html
│   ├── post2.json               │   └── publish-html.inja         │   ├── post1.html
│   └── post3.json               │                                 │   └── post2.html
│                                │                                 │   └── post3.html
├── news/                        ├── news /                        │
│   ├── news1.json               │   ├── index-html.inja           ├── news/
│   └── news2.json               │   └── broadcast-html.inja       │   ├── index.html
│                                │                                 │   ├── news1.html
│                                └── index-html.json               │   └── news2.html
└── main.json                                                      │
                                                                   └── index.html

```

### How Directory Mode Works (Upstream Resolution)

1. **Exact Directory Matching**: For any data file located in a subdirectory (e.g., `data/posts/post1.json`), InjaX first looks for matching templates inside the corresponding folder (`templates/posts/`).
2. **Upstream Inheritance**: If a target template base is not found in the immediate subdirectory, the engine traverses **upward** through parent directories until it finds the closest matching template.

3. **`index-{format}.inja` templates** generate a single aggregated listing file named `index.{format}` directly within the current output subdirectory level (`output/{sub_dir}/index.{format}`).
4. **Other templates** (e.g., `publish-html.inja`, `broadcast-html.inja`) automatically map to individual data files inside that folder, creating one output file per dataset named `{data_stem}.{format}`.

## Examples

### Single File Mode

**Template (`template.inja`):**

```jinja
{% if user.is_active %}
  Hello <strong>{{ user.name | escape }}</strong>!
  You have {{ messages | length }} new messages.
{% else %}
  Please activate your account.
{% endif %}

```

**Data (`data.json`):**

```json
{
  "user": {
    "is_active": true,
    "name": "Alice"
  },
  "messages": ["Hello!", "Meeting at 3pm"]
}

```

**Command:**

```bash
injax data.json template.inja output.html

```

**Output (`output.html`):**

```html
Hello <strong>Alice</strong>!
You have 2 new messages.

```

### Directory Mode with Multiple Templates

**Data file (`data/posts/first-post.json`):**

```json
{
  "title": "My First Post",
  "date": "2024-01-15",
  "content": "This is my first blog post!",
  "tags": ["introduction", "hello-world"]
}

```

**Template 1 (`templates/posts/index-html.inja`):**

```jinja
<!DOCTYPE html>
<html>
<head>
  <title>Blog Posts</title>
</head>
<body>
  <h1>All Posts</h1>
  <ul>
  {% for post in page_list %}
    <li><a href="{{ post._stem }}.html">{{ post.title }}</a></li>
  {% endfor %}
  </ul>
</body>
</html>

```

**Template 2 (`templates/posts/publish-html.inja`):**

```jinja
<!DOCTYPE html>
<html>
<head>
  <title>{{ title }} | My Blog</title>
</head>
<body>
  <article>
    <h1>{{ title }}</h1>
    <div class="date">{{ date }}</div>
    <div class="content">{{ content }}</div>
    <div class="tags">
      Tags: {% for tag in tags %}
        <span class="tag">{{ tag }}</span>
      {% endfor %}
    </div>
  </article>
</body>
</html>

```

**Command:**

```bash
injax data/ templates/ output/

```

**Output 1 (`output/posts/index.html`):**

```html
<!DOCTYPE html>
<html>
<head>
  <title>Blog Posts</title>
</head>
<body>
  <h1>All Posts</h1>
  <ul>
    <li><a href="first-post.html">My First Post</a></li>
    <li><a href="second-post.html">My Second Post</a></li>
  </ul>
</body>
</html>

```

**Output 2 (`output/posts/first-post.html`):**

```html
<!DOCTYPE html>
<html>
<head>
  <title>My First Post | My Blog</title>
</head>
<body>
  <article>
    <h1>My First Post</h1>
    <div class="date">2024-01-15</div>
    <div class="content">This is my first blog post!</div>
    <div class="tags">
      Tags: 
        <span class="tag">introduction</span>
        <span class="tag">hello-world</span>
    </div>
  </article>
</body>
</html>

```

### Root Data and Root Template Example

**Data (`data/main.json`):**

```json
{
  "site_title": "My Awesome Site",
  "description": "Welcome to my website"
}

```

**Template (`templates/index-html.inja`):**

```jinja
<!DOCTYPE html>
<html>
<head>
  <title>{{ site_title }}</title>
  <meta name="description" content="{{ description }}">
</head>
<body>
  <h1>{{ site_title }}</h1>
  <p>{{ description }}</p>
</body>
</html>

```

**Output (`output/index.html`):**

```html
<!DOCTYPE html>
<html>
<head>
  <title>My Awesome Site</title>
  <meta name="description" content="Welcome to my website">
</head>
<body>
  <h1>My Awesome Site</h1>
  <p>Welcome to my website</p>
</body>
</html>

```

### Multi-Format Output Example

**Directory structure:**

```
data/                          templates/                        output/
├── posts/                     ├── posts/                        ├── posts/
│   └── article.json           │   ├── index-html.inja           │   ├── index.html
└── main.json                  │   ├── index-md.inja             │   ├── index.md
                               │   ├── publish-html.inja         │   ├── article.html
                               │   └── publish-md.inja           │   └── article.md
                               └── index-html.inja               └── index.html

```

**Command:**

```bash
injax data/ templates/ output/

```

This generates both HTML and Markdown outputs targeting the matching destination directory natively from the same underlying dataset.

## Available Metadata in Directory Mode

When running in directory mode, InjaX automatically injects these internal context variables into your dataset:

| Variable | Description | Example |
| --- | --- | --- |
| `_filename` | Original data filename with extension | `"first-post.json"` |
| `_stem` | Filename without extension | `"first-post"` |
| `_path` | Relative subdirectory from data root | `"posts"` (or `""` if root) |
| `_template` | Resolved template filename with ext. | `"publish-html.inja"` |
| `_template_stem` | Template filename without extension | `"publish-html"` |
| `page_list` | Array of all data items in current dir | `[{...}, {...}]` |

**Safe usage of metadata in templates:**

```jinja
<nav>
  You are here: Root 
  {% if _path %} > {{ _path }}{% endif %} > {{ _stem }}
</nav>

{% for page in page_list %}
  <li><a href="{{ page._stem }}.html">{{ page.title }}</a></li>
{% endfor %}

```

## Built-in Filters

| Category | Filters |
| --- | --- |
| Strings | `center`, `reverse`, `trim`, `truncate`, `title`, `wordcount`, `wordwrap` |
| Arrays | `map`, `select`, `reject`, `unique`, `slice`, `batch`, `sort_by`, `append` |
| Objects | `items`, `dictsort`, `regroup` |
| HTML | `escape`, `striptags`, `urlencode`, `urlize`, `xmlattr` |
| Numbers | `abs`, `random`, `filesizeformat`, `indent` |

## Conditional Tests

```jinja
{% if isString(value) %}String!{% endif %}
{% if isNumber(value) %}Number!{% endif %}
{% if isDefined(value) %}Exists!{% endif %}
{% if isEmpty(value) %}Nothing here{% endif %}
{% if contains(value,"hello") %}Contains hello{% endif %}

```

Available tests: `isString`, `isNumber`, `isArray`, `isObject`, `isDefined`, `isEmpty`, `contains`, `matches`, `startsWith`, `endsWith`, `isEven`, `isOdd`

## Best Practices

### For Single File Mode

* Use for one-off rendering tasks.
* Great for documentation generation or config file processing.
* Ideal when you have a single distinct data source.

### For Directory Mode

* Build scalable static sites, portfolios, and blogs.
* Generate multi-page websites and documentation from clean collections of raw structured content (JSON, YAML, CSV).
* Generate multiple parallel output formats (e.g. HTML + XML + Markdown RSS feeds) simultaneously from the same exact data.
* Leverage **Upstream Inheritance** by putting common layout configurations inside the root `templates/` folder to avoid duplicating templates across every subfolder.
* Use `index-{format}.inja` for building listings pages within an active directory level.

## License

InjaX is open source software released under the **MIT License**.

## Acknowledgments

* [Inja](https://github.com/pantor/inja) — The core template engine
* [nlohmann/json](https://github.com/nlohmann/json) — JSON for modern C++
* [Jinja2](https://github.com/pallets/jinja/) — Extensible templating engine
* [Hugo](https://github.com/gohugoio/hugo) — Framework for building websites