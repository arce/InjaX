# ARRAY-FILTERS(7)

## NAME

array-filters - InjaX array manipulation functions for JSON data

## SYNOPSIS

Filter operations on JSON arrays: mapping, selecting, rejecting, grouping, sorting, batching, zipping, and numeric aggregation.

## FILTERS

### `array`

Creates a JSON array from the provided arguments.

**Parameters:** Any number of values, each becomes an element.

**Example:**

json

```
{{ [1, 2, 3] | array | tojson }} → [1,2,3]
{{ "a", "b", "c" | array | tojson }} → ["a","b","c"]
```



### `append`

Appends an item to an array.

**Parameters:**

- `arr`: Array to modify (if not an array, new array is created)
- `item`: Value to append

**Example:**

json

```
{{ [1,2,3] | append(4) | tojson }} → [1,2,3,4]
```



### `batch`

Splits array into batches of specified size.

**Parameters:**

- `seq`: Input array
- `size`: Batch size (positive integer)
- `fill_with`: Optional filler for incomplete last batch (default: null, no filling)

**Example:**

json

```
{{ [1,2,3,4,5] | batch(2) | tojson }} → [[1,2],[3,4],[5]]
{{ [1,2,3] | batch(4, "x") | tojson }} → [[1,2,3,"x"]]
```



### `dictsort`

Sorts object key-value pairs.

**Parameters:**

- `obj`: JSON object
- `by`: Sort criteria - "key" (default) or "value"

**Returns:** Array of objects with `key` and `value` properties.

**Example:**

json

```
{{ {"c":3,"a":1,"b":2} | dictsort("key") | tojson }}
→ [{"key":"a","value":1},{"key":"b","value":2},{"key":"c","value":3}]
```



### `items`

Converts object to array of key-value pairs.

**Parameters:**

- `obj`: JSON object

**Returns:** Array of objects with `key` and `value` properties.

**Example:**

json

```
{{ {"name":"John","age":30} | items | tojson }}
→ [{"key":"name","value":"John"},{"key":"age","value":30}]
```



### `make_list`

Converts any value to an array.

**Parameters:**

- `value`: Value to convert

**Returns:** Original array if input is array; array of object values if input is object; single-element array otherwise.

**Example:**

json

```
{{ 42 | make_list | tojson }} → [42]
{{ {"a":1,"b":2} | make_list | tojson }} → [1,2]
```



### `map`

Extracts attribute or index from each array element.

**Parameters:**

- `seq`: Input array
- `key_or_path`: String path (dot notation) or integer index

**Example:**

json

```
{{ [{"name":"Alice"},{"name":"Bob"}] | map("name") | tojson }} → ["Alice","Bob"]
{{ [[1,2,3],[4,5,6]] | map(1) | tojson }} → [2,5]
```



### `obj`

Creates a JSON object from alternating key-value arguments.

**Parameters:** Even number of arguments: key1, value1, key2, value2, ...

**Example:**

json

```
{{ "name","John","age",30 | obj | tojson }} → {"name":"John","age":30}
```



### `regroup`

Groups array elements by attribute value.

**Parameters:**

- `seq`: Input array
- `attr`: Attribute path (dot notation)

**Returns:** Array of objects with `grouper` (the grouping value) and `list` (array of matching items).

**Example:**

json

```
{{ [{"type":"A","val":1},{"type":"B","val":2},{"type":"A","val":3}] 
   | regroup("type") | tojson }}
→ [{"grouper":"A","list":[{"type":"A","val":1},{"type":"A","val":3}]},
   {"grouper":"B","list":[{"type":"B","val":2}]}]
```



### `reject`

Filters out array elements equal to a value.

**Parameters:**

- `seq`: Input array
- `value`: Value to reject

**Example:**

json

```
{{ [1,2,3,2,1] | reject(2) | tojson }} → [1,3,1]
```



### `rejectattr`

Filters out elements whose attribute passes a test.

**Parameters:**

- `seq`: Input array
- `attr`: Attribute path (dot notation)
- `test`: Test name (optional: "", "equalto", "defined", "none", "in", "match")
- `test_arg`: Test argument (optional)

**Example:**

json

```
{{ [{"name":"Alice"},{"name":""},{"name":"Bob"}] 
   | rejectattr("name", "none") | tojson }}
→ [{"name":"Alice"},{"name":"Bob"}]
```



### `select`

Selects array elements equal to a value.

**Parameters:**

- `seq`: Input array
- `value`: Value to select

**Example:**

json

```
{{ [1,2,3,2,1] | select(2) | tojson }} → [2,2]
```



### `selectattr`

Selects elements whose attribute passes a test.

**Parameters:**

- `seq`: Input array
- `attr`: Attribute path (dot notation)
- `test`: Test name (optional: "", "equalto", "defined", "none", "in", "match")
- `test_arg`: Test argument (optional)

**Example:**

json

```
{{ [{"name":"Alice","age":25},{"name":"Bob","age":30}] 
   | selectattr("age", "equalto", 30) | tojson }}
→ [{"name":"Bob","age":30}]
```



### `slice`

Extracts a slice from an array.

**Parameters:**

- `seq`: Input array
- `start`: Starting index (negative counts from end)
- `stop`: Stopping index (exclusive, negative counts from end)
- `step`: Step size (default: 1, can be negative)

**Example:**

json

```
{{ [0,1,2,3,4,5] | slice(1,4) | tojson }} → [1,2,3]
{{ [0,1,2,3,4,5] | slice(5,0,-1) | tojson }} → [5,4,3,2,1]
```



### `sort_by`

Sorts array by attribute value.

**Parameters:**

- `seq`: Input array
- `attr`: Attribute path (dot notation)

**Example:**

json

```
{{ [{"name":"Bob","age":30},{"name":"Alice","age":25}] 
   | sort_by("name") | tojson }}
→ [{"name":"Alice","age":25},{"name":"Bob","age":30}]
```



### `sum`

Calculates sum of numeric values in array.

**Parameters:**

- `arr`: Input array (numeric values or numeric strings)

**Returns:** Number (double)

**Example:**

json

```
{{ [1,2,3,4] | sum }} → 10
{{ ["1.5","2.5",3] | sum }} → 7.0
```



### `tojson`

Converts JSON value to JSON string representation.

**Parameters:**

- `value`: Any JSON value

**Returns:** String

**Example:**

json

```
{{ {"name":"John"} | tojson }} → "{\"name\":\"John\"}"
```



### `unique`

Removes duplicate values from array.

**Parameters:**

- `arr`: Input array

**Returns:** Array with unique elements (preserves first occurrence)

**Example:**

json

```
{{ [1,2,2,3,3,3,1] | unique | tojson }} → [1,2,3]
```



### `zip`

Zips multiple arrays into array of tuples (shortest length).

**Parameters:** Any number of arrays

**Example:**

json

```
{{ [1,2,3], ["a","b","c"] | zip | tojson }} → [[1,"a"],[2,"b"],[3,"c"]]
```



### `zip_array`

Zips an array of arrays into array of tuples.

**Parameters:**

- `arrays`: Array containing sub-arrays to zip

**Example:**

json

```
{{ [[1,2,3],["a","b","c"]] | zip_array | tojson }} → [[1,"a"],[2,"b"],[3,"c"]]
```



### `zip_obj`

Zips keys array with data arrays into array of objects.

**Parameters:**

- `keys`: Array of key names (strings)
- `arrays`: One or more arrays to zip (at least 1)

**Example:**

json

```
{{ ["name","age"], ["Alice","Bob"], [25,30] | zip_obj | tojson }}
→ [{"name":"Alice","age":25},{"name":"Bob","age":30}]
```



## TESTS FOR ATTRIBUTES

When using `selectattr` and `rejectattr`, the following tests are available:

| Test        | Description                                  | Test Argument |
| :---------- | :------------------------------------------- | :------------ |
| (empty)     | Truthy: non-null, non-empty, non-zero        | None          |
| `"equalto"` | Equal to test_arg                            | Required      |
| `"defined"` | Not null                                     | None          |
| `"none"`    | Is null                                      | None          |
| `"in"`      | Value exists in array or substring in string | Required      |
| `"match"`   | Regex matches (strings only)                 | Required      |

## SEE ALSO

string-filters(7), html-filters(7), number-filters(7), tests(7)