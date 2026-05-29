# TESTS(7)

## NAME

tests - InjaX type-checking and condition functions

## SYNOPSIS

Test functions for use in conditional expressions: type checking, value testing, and pattern matching.

## TESTS

### `contains`

Checks if value exists in array, string, or object.

**Parameters:**

- `haystack`: Array, string, or object to search
- `needle`: Value to find

**Returns:** Boolean

**Behavior:**

- Array: element equality
- String: substring presence
- Object: key existence (needle must be string)

**Examples with literal values:**

```
{% if [1,2,3] is contains(2) %}yes{% endif %} → "yes"
{% if "hello" is contains("ell") %}yes{% endif %} → "yes"
```

**Examples with external JSON data:**

Given an external JSON file `data.json`:
```json
{
  "numbers": [1, 2, 3],
  "text": "hello",
  "obj": {"a": 1}
}
```

Loaded as variable `data`:
```
{% if data.numbers is contains(2) %}yes{% endif %} → "yes"
{% if data.text is contains("ell") %}yes{% endif %} → "yes"
{% if data.obj is contains("a") %}yes{% endif %} → "yes"
```

### `endsWith`

Checks if string ends with suffix.

**Parameters:**

- `value`: String to check
- `suffix`: Suffix to test

**Example with literal value:**

```
{% if "hello.txt" is endsWith(".txt") %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `files.json`:
```json
{"filename": "hello.txt"}
```

Loaded as variable `file`:
```
{% if file.filename is endsWith(".txt") %}yes{% endif %} → "yes"
```

### `hasKey`

Checks if object contains specific key.

**Parameters:**

- `obj`: Object to check
- `key`: Key name (string)

**Example with external JSON data:**

Given an external JSON file `person.json`:
```json
{"name": "John", "age": 30}
```

Loaded as variable `person`:
```
{% if person is hasKey("age") %}yes{% endif %} → "yes"
```

### `isArray`

Checks if value is an array.

**Parameters:** Single value

**Example with literal value:**

```
{% if [1,2,3] is isArray %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `data.json`:
```json
{"items": [1, 2, 3]}
```

Loaded as variable `data`:
```
{% if data.items is isArray %}yes{% endif %} → "yes"
```

### `isBoolean`

Checks if value is boolean (true/false).

**Parameters:** Single value

**Example with literal value:**

```
{% if true is isBoolean %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `config.json`:
```json
{"enabled": true}
```

Loaded as variable `config`:
```
{% if config.enabled is isBoolean %}yes{% endif %} → "yes"
```

### `isContained`

Checks if item exists in array (reverse of contains).

**Parameters:**

- `item`: Value to find
- `collection`: Array to search

**Example with literal value:**

```
{% if 2 is isContained([1,2,3]) %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `dataset.json`:
```json
{
  "value": 2,
  "list": [1, 2, 3]
}
```

Loaded as variable `data`:
```
{% if data.value is isContained(data.list) %}yes{% endif %} → "yes"
```

### `isDefined`

Checks if value is not null.

**Parameters:** Single value

**Example with literal value:**

```
{% if null is isDefined %}no{% else %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `data.json`:
```json
{"value": null}
```

Loaded as variable `data`:
```
{% if data.value is isDefined %}no{% else %}yes{% endif %} → "yes"
```

### `isDivisibleBy`

Checks if integer is divisible by another integer.

**Parameters:**

- `dividend`: Value to test
- `divisor`: Divisor (non-zero)

**Example with literal value:**

```
{% if 10 is isDivisibleBy(2) %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `numbers.json`:
```json
{"number": 10, "divisor": 2}
```

Loaded as variable `nums`:
```
{% if nums.number is isDivisibleBy(nums.divisor) %}yes{% endif %} → "yes"
```

### `isEmpty`

Checks if string, array, or object is empty.

**Parameters:** Single value

**Examples with literal values:**

```
{% if "" is isEmpty %}yes{% endif %} → "yes"
{% if [] is isEmpty %}yes{% endif %} → "yes"
```

**Examples with external JSON data:**

Given an external JSON file `data.json`:
```json
{
  "empty_string": "",
  "empty_array": [],
  "non_empty": "hello"
}
```

Loaded as variable `data`:
```
{% if data.empty_string is isEmpty %}yes{% endif %} → "yes"
{% if data.empty_array is isEmpty %}yes{% endif %} → "yes"
{% if data.non_empty is isEmpty %}no{% endif %} → "no"
```

### `isEscaped`

Checks if string contains HTML escape sequences.

**Parameters:** Single value

**Example with literal value:**

```
{% if "&lt;tag&gt;" is isEscaped %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `html.json`:
```json
{"escaped": "&lt;tag&gt;", "raw": "<tag>"}
```

Loaded as variable `html`:
```
{% if html.escaped is isEscaped %}yes{% endif %} → "yes"
{% if html.raw is isEscaped %}no{% endif %} → "no"
```

### `isEven`

Checks if integer is even.

**Parameters:** Single value

**Example with literal value:**

```
{% if 42 is isEven %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `numbers.json`:
```json
{"value": 42}
```

Loaded as variable `num`:
```
{% if num.value is isEven %}yes{% endif %} → "yes"
```

### `isFloat`

Checks if value is floating-point number.

**Parameters:** Single value

**Example with literal value:**

```
{% if 3.14 is isFloat %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `values.json`:
```json
{"pi": 3.14, "answer": 42}
```

Loaded as variable `vals`:
```
{% if vals.pi is isFloat %}yes{% endif %} → "yes"
{% if vals.answer is isFloat %}no{% endif %} → "no"
```

### `isInteger`

Checks if value is integer number.

**Parameters:** Single value

**Example with literal value:**

```
{% if 42 is isInteger %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `values.json`:
```json
{"count": 42, "pi": 3.14}
```

Loaded as variable `vals`:
```
{% if vals.count is isInteger %}yes{% endif %} → "yes"
{% if vals.pi is isInteger %}no{% endif %} → "no"
```

### `isIterable`

Checks if value is iterable (array or object).

**Parameters:** Single value

**Examples with literal values:**

```
{% if [1,2,3] is isIterable %}yes{% endif %} → "yes"
```

**Examples with external JSON data:**

Given an external JSON file `data.json`:
```json
{
  "list": [1, 2, 3],
  "dict": {"a": 1},
  "string": "hello"
}
```

Loaded as variable `data`:
```
{% if data.list is isIterable %}yes{% endif %} → "yes"
{% if data.dict is isIterable %}yes{% endif %} → "yes"
{% if data.string is isIterable %}no{% endif %} → "no"
```

### `isLower`

Checks if string contains no uppercase letters (non-letters ignored).

**Parameters:** Single value

**Example with literal value:**

```
{% if "hello 123" is isLower %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `strings.json`:
```json
{"lower": "hello 123", "upper": "HELLO 123"}
```

Loaded as variable `str`:
```
{% if str.lower is isLower %}yes{% endif %} → "yes"
{% if str.upper is isLower %}no{% endif %} → "no"
```

### `isMapping`

Checks if value is object (dictionary/map).

**Parameters:** Single value

**Example with external JSON data:**

Given an external JSON file `data.json`:
```json
{"obj": {"a": 1}, "arr": [1, 2, 3]}
```

Loaded as variable `data`:
```
{% if data.obj is isMapping %}yes{% endif %} → "yes"
{% if data.arr is isMapping %}no{% endif %} → "no"
```

### `isNone`

Checks if value is null.

**Parameters:** Single value

**Example with literal value:**

```
{% if null is isNone %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `data.json`:
```json
{"value": null}
```

Loaded as variable `data`:
```
{% if data.value is isNone %}yes{% endif %} → "yes"
```

### `isNull`

Alias for `isNone`. Checks if value is null.

**Example with external JSON data:**

Given an external JSON file `data.json`:
```json
{"value": null}
```

Loaded as variable `data`:
```
{% if data.value is isNull %}yes{% endif %} → "yes"
```

### `isNumber`

Checks if value is number (integer or float).

**Parameters:** Single value

**Example with literal value:**

```
{% if 42 is isNumber %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `data.json`:
```json
{"count": 42, "pi": 3.14, "text": "hello"}
```

Loaded as variable `data`:
```
{% if data.count is isNumber %}yes{% endif %} → "yes"
{% if data.pi is isNumber %}yes{% endif %} → "yes"
{% if data.text is isNumber %}no{% endif %} → "no"
```

### `isObject`

Checks if value is object.

**Parameters:** Single value

**Example with external JSON data:**

Given an external JSON file `data.json`:
```json
{"obj": {"a": 1}, "arr": [1, 2, 3]}
```

Loaded as variable `data`:
```
{% if data.obj is isObject %}yes{% endif %} → "yes"
{% if data.arr is isObject %}no{% endif %} → "no"
```

### `isOdd`

Checks if integer is odd.

**Parameters:** Single value

**Example with literal value:**

```
{% if 41 is isOdd %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `numbers.json`:
```json
{"value": 41}
```

Loaded as variable `num`:
```
{% if num.value is isOdd %}yes{% endif %} → "yes"
```

### `isSequence`

Checks if value is array.

**Parameters:** Single value

**Example with literal value:**

```
{% if [1,2,3] is isSequence %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `data.json`:
```json
{"items": [1, 2, 3], "obj": {"a": 1}}
```

Loaded as variable `data`:
```
{% if data.items is isSequence %}yes{% endif %} → "yes"
{% if data.obj is isSequence %}no{% endif %} → "no"
```

### `isString`

Checks if value is string.

**Parameters:** Single value

**Example with literal value:**

```
{% if "hello" is isString %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `data.json`:
```json
{"text": "hello", "number": 42}
```

Loaded as variable `data`:
```
{% if data.text is isString %}yes{% endif %} → "yes"
{% if data.number is isString %}no{% endif %} → "no"
```

### `isUndefined`

Checks if value is null (alias for isNone).

**Parameters:** Single value

**Example with external JSON data:**

Given an external JSON file `data.json`:
```json
{"value": null}
```

Loaded as variable `data`:
```
{% if data.value is isUndefined %}yes{% endif %} → "yes"
```

### `isUpper`

Checks if string contains no lowercase letters (non-letters ignored).

**Parameters:** Single value

**Example with literal value:**

```
{% if "HELLO 123" is isUpper %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `strings.json`:
```json
{"upper": "HELLO 123", "lower": "hello 123"}
```

Loaded as variable `str`:
```
{% if str.upper is isUpper %}yes{% endif %} → "yes"
{% if str.lower is isUpper %}no{% endif %} → "no"
```

### `matches`

Checks if string matches regular expression.

**Parameters:**

- `value`: String to test
- `pattern`: Regular expression pattern

**Note:** Uses `std::regex` (ECMAScript syntax).

**Example with literal value:**

```
{% if "user@example.com" is matches("^[^@]+@[^@]+\.[^@]+$") %}
  valid email
{% endif %}
```

**Example with external JSON data:**

Given an external JSON file `emails.json`:
```json
{"email": "user@example.com"}
```

Loaded as variable `contact`:
```
{% if contact.email is matches("^[^@]+@[^@]+\.[^@]+$") %}
  valid email
{% endif %}
```

### `startsWith`

Checks if string starts with prefix.

**Parameters:**

- `value`: String to check
- `prefix`: Prefix to test

**Example with literal value:**

```
{% if "hello.txt" is startsWith("hello") %}yes{% endif %} → "yes"
```

**Example with external JSON data:**

Given an external JSON file `files.json`:
```json
{"filename": "hello.txt"}
```

Loaded as variable `file`:
```
{% if file.filename is startsWith("hello") %}yes{% endif %} → "yes"
```

## Complete example 

*File: userdata.json*
```json
{
  "name": "John Doe",
  "email": "john@example.com",
  "age": 25,
  "tags": ["admin", "user", "verified"],
  "active": true,
  "metadata": null,
  "config": {"theme": "dark"}
}
```

*File: template.inja*
```
{% if user.name is isString %}Name is valid{% endif %}
{% if user.email is matches("^[^@]+@[^@]+\.[^@]+$") %}Email format OK{% endif %}
{% if user.age is isNumber and user.age >= 18 %}Adult{% endif %}
{% if "admin" is isContained(user.tags) %}Admin user{% endif %}
{% if user.active is isBoolean and user.active %}Account active{% endif %}
{% if user.metadata is isNone %}No metadata{% endif %}
{% if user.config is isMapping %}Configuration present{% endif %}
```

## SEE ALSO

array-filters(7), string-filters(7), html-filters(7), number-filters(7)
