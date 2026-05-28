# NUMBER-FILTERS(7)

## NAME

number-filters - InjaX numeric operations and formatting functions for JSON data

## SYNOPSIS

Filter operations for numbers: absolute value, random selection, file size formatting, and text indentation.

## FILTERS

### `abs`

Returns absolute value of a number or each number in an array.

**Parameters:**

- `value`: Number, array of numbers, or convertible string

**Returns:** Number or array of numbers

**Special handling:** For integer `LLONG_MIN`, returns `LLONG_MAX`.

**Example:**

json

```
{{ -42 | abs }} → 42
{{ [-5, 3, -2, 0] | abs | tojson }} → [5, 3, 2, 0]
{{ "-123.45" | abs }} → 123.45
```



### `filesizeformat`

Formats byte count as human-readable file size.

**Parameters:**

- `value`: Number of bytes (integer or numeric string)
- `binary`: Use binary units (KiB/MiB) if true, decimal (KB/MB) if false (default: false)

**Units:**

| Decimal | Binary |
| :------ | :----- |
| B       | B      |
| KB      | KiB    |
| MB      | MiB    |
| GB      | GiB    |
| TB      | TiB    |
| PB      | PiB    |

**Example:**

json

```
{{ 0 | filesizeformat }} → "0 B"
{{ 1024 | filesizeformat }} → "1.0 KB"
{{ 1024 | filesizeformat(true) }} → "1.0 KiB"
{{ 1536000 | filesizeformat }} → "1.5 MB"
{{ 500 | filesizeformat }} → "500 B"
```



### `indent`

Indents each line of text with spaces.

**Parameters:**

- `value`: String to indent (non-strings are JSON-dumped)
- `width`: Number of spaces per indent (default: 4)
- `indent_first`: Whether to indent first line (default: false)
- `blank`: Whether to indent blank lines (default: false)

**Example:**

json

```
{{ "line1\nline2\n\nline4" | indent(2, true) }}
→ "  line1\n  line2\n\n  line4"

{{ "first\nsecond" | indent(4, true, true) }}
→ "    first\n    second"
```



### `random`

Selects random element from array or random character from string.

**Parameters:**

- `source`: Array, string, or other value

**Returns:** Random element (if array/string) or original value (if not iterable)

**Note:** Uses thread-local Mersenne Twister with high-resolution clock seeding.

**Example:**

json

```
{{ [1,2,3,4,5] | random }} → 3 (random)
{{ "abcdef" | random }} → "c" (random)
{{ 42 | random }} → 42
```



## SEE ALSO

array-filters(7), string-filters(7), html-filters(7), tests(7)