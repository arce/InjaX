# InjaX

[![GitHub stars](https://img.shields.io/github/stars/arce/injax)](https://github.com/arce/injax/stargazers)
[![GitHub license](https://img.shields.io/github/license/arce/injax)](https://github.com/arce/injax/blob/main/LICENSE)
[![GitHub releases](https://img.shields.io/github/v/release/arce/injax)](https://github.com/arce/injax/releases)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)](https://isocpp.org/)

**InjaX** is a modern, high-performance template engine for C++ that brings the elegance of Jinja2 syntax to the C++ ecosystem. Use it as a header-only library or a standalone CLI tool.

## Quick Start

```bash
# Download a binary for your platform
curl -L https://github.com/arce/injax/releases/download/v2.0.1/InjaX-linux-x64.tar.gz | tar xz

# Render a template
injax template.txt data.json output.html
```

## Why InjaX?

- **Familiar syntax** — Uses Jinja2-style delimiters (`{{ }}`, `{% %}`, `{# #}`)
- **Fast** — Compiled to efficient static binaries, no interpreter overhead
- **Zero heavy deps** — Header-only with inja and nlohmann/json
- **Multi-format data** — JSON natively, plus plugins for CSV, YAML, XML, and INI
- **Extensible** — Plugin system for custom data readers

## Installation

### Download pre-built binaries

| Platform | Architecture | Download |
|----------|--------------|----------|
| macOS | Apple Silicon | [InjaX-darwin-arm64.tar.gz](https://github.com/arce/injax/releases/download/v2.0.1/InjaX-darwin-arm64.tar.gz) (4.2 MB) |
| macOS | Intel | [InjaX-darwin-x64.tar.gz](https://github.com/arce/injax/releases/download/v2.0.1/InjaX-darwin-x64.tar.gz) (4.5 MB) |
| Windows | ARM64 | [InjaX-windows-arm64.zip](https://github.com/arce/injax/releases/download/v2.0.1/InjaX-windows-arm64.zip) (3.8 MB) |
| Windows | x64 | [InjaX-windows-x64.zip](https://github.com/arce/injax/releases/download/v2.0.1/InjaX-windows-x64.zip) (4.1 MB) |
| Linux | ARM64 | [InjaX-linux-arm64.tar.gz](https://github.com/arce/injax/releases/download/v2.0.1/InjaX-linux-arm64.tar.gz) (3.5 MB) |
| Linux | x64 | [InjaX-linux-x64.tar.gz](https://github.com/arce/injax/releases/download/v2.0.1/InjaX-linux-x64.tar.gz) (3.9 MB) |

## Example

**Template (`template.txt`):**
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
injax template.txt data.json output.html
```

**Output (`output.html`):**
```html
Hello <strong>Alice</strong>!
You have 2 new messages.
```

## Built-in Filters

| Category | Filters |
|----------|---------|
| Strings | `center`, `reverse`, `trim`, `truncate`, `title`, `wordcount`, `wordwrap` |
| Arrays | `map`, `select`, `reject`, `unique`, `slice`, `batch`, `sort_by`, `append` |
| Objects | `items`, `dictsort`, `regroup` |
| HTML | `escape`, `striptags`, `urlencode`, `urlize`, `xmlattr` |
| Numbers | `abs`, `random`, `filesizeformat`, `indent` |

## Conditional Tests

```jinja
{% if value is string %}String!{% endif %}
{% if value is number %}Number!{% endif %}
{% if value is defined %}Exists!{% endif %}
{% if value is empty %}Nothing here{% endif %}
{% if "hello" in value %}Contains hello{% endif %}
```

Available tests: `isString`, `isNumber`, `isArray`, `isObject`, `isDefined`, `isEmpty`, `contains`, `matches`, `startsWith`, `endsWith`, `isEven`, `isOdd`

## Supported Data Formats

| Format | Extension | Built-in | Requires |
|--------|-----------|----------|----------|
| JSON | `.json` | ✅ Yes | None |
| CSV | `.csv` | ❌ No | `libcsv.so` plugin |
| YAML | `.yaml`, `.yml` | ❌ No | `libyaml.so` plugin |
| XML | `.xml` | ❌ No | `libxml.so` plugin |
| INI | `.ini` | ❌ No | `libini.so` plugin |

## Use Cases

- **Web development** — Generate HTML pages, dashboards, portals
- **Email campaigns** — Personalized newsletters and notifications
- **Code generation** — Boilerplate, SQL queries, Terraform configs
- **DevOps** — Dynamic configs for Ansible, Kubernetes, Nginx
- **Document generation** — Reports, invoices, Markdown docs
- **API responses** — Transform JSON data on the fly

## License

InjaX is open source software released under the **MIT License**.

## Acknowledgments

- [Inja](https://github.com/pantor/inja) — The core template engine
- [nlohmann/json](https://github.com/nlohmann/json) — JSON for modern C++
- [Jinja2](https://github.com/pallets/jinja/) — The inspiration

