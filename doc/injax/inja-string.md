# STRING-FILTERS(7)

## NAME

string-filters - InjaX string manipulation functions for JSON data

## SYNOPSIS

Filter operations on JSON strings: centering, reversing, trimming, truncating, titling, word counting, wrapping, and conversion.

## FILTERS

### `center`

Centers a string within a field of specified width.

**Parameters:**

- `input`: String to center
- `width`: Field width (if ≤ input length, returns input)
- `fill`: Fill character(s) (default: space)

**Example:**

json

```
{{ "hello" | center(11, "-") }} → "---hello---"
{{ "hi" | center(5) }} → "  hi "
```



### `reverse`

Reverses the characters of a string.

**Parameters:**

- `input`: String to reverse

**Example:**

json

```
{{ "hello" | reverse }} → "olleh"
```



### `string`

Converts any JSON value to its string representation.

**Parameters:**

- `value`: Any JSON value

**Example:**

json

```
{{ 42 | string }} → "42"
{{ true | string }} → "true"
{{ null | string }} → ""
{{ {"a":1} | string }} → "{\"a\":1}"
```



### `title`

Converts string to title case (first letter of each word capitalized, rest lowercase).

**Parameters:**

- `input`: String to convert

**Example:**

json

```
{{ "hello WORLD" | title }} → "Hello World"
{{ "the-quick_brown fox" | title }} → "The-Quick_Brown Fox"
```



### `trim`

Removes leading and trailing whitespace.

**Parameters:**

- `input`: String to trim

**Whitespace:** space, tab, newline, carriage return, form feed, vertical tab

**Example:**

json

```
{{ "  hello  " | trim }} → "hello"
{{ "\n\t test \r\n" | trim }} → "test"
```



### `truncate`

Truncates string to specified length with custom ending.

**Parameters:**

- `input`: String to truncate
- `length`: Maximum length (must be ≥ 1)
- `end`: Suffix to append when truncated (default: "...")

**Example:**

json

```
{{ "Hello world" | truncate(8) }} → "Hello..."
{{ "Hello world" | truncate(10, "!") }} → "Hello wor!"
{{ "Hello" | truncate(10) }} → "Hello"
```



### `wordcount`

Counts number of words in string.

**Parameters:**

- `input`: String to analyze

**Returns:** Integer

**Example:**

json

```
{{ "Hello world from InjaX" | wordcount }} → 4
{{ "One. Two? Three!" | wordcount }} → 3
```



### `wordwrap`

Wraps text to specified line width.

**Parameters:**

- `input`: Text to wrap
- `width`: Maximum line width
- `break_str`: Line break string (default: "\n")
- `break_long_words`: Whether to break words longer than width (default: false)

**Example:**

json

```
{{ "This is a long sentence" | wordwrap(10) }}
→ "This is a\nlong\nsentence"

{{ "Supercalifragilistic" | wordwrap(5, "\n", true) }}
→ "Super\ncalif\nragil\nistic"
```



## SEE ALSO

array-filters(7), html-filters(7), number-filters(7), tests(7)