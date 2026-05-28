# InjaX

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue)](https://isocpp.org/)

**InjaX** is a modern, high-performance template engine for C++ that brings the elegance of Jinja2 syntax to the C++ ecosystem. Use it as a standalone CLI tool.

## Quick Start

```bash
# Render a template
injax template.txt data.json output.html
```

## Why InjaX?

- **Familiar syntax** — Uses Jinja2-style delimiters (`{{ }}`, `{% %}`, `{# #}`)
- **Fast** — Compiled to efficient static binaries, no interpreter overhead
- **Zero heavy deps** — Header-only with inja and nlohmann/json
- **Multi-format data** — JSON natively, plus plugins for CSV, YAML, XML, and INI
- **Extensible** — Plugin system for custom modules

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

## License

InjaX is open source software released under the **MIT License**.

## Acknowledgments

- [Inja](https://github.com/pantor/inja) — The core template engine
- [nlohmann/json](https://github.com/nlohmann/json) — JSON for modern C++
- [Jinja2](https://github.com/pallets/jinja/) — The inspiration

