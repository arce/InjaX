# TESTS(7)

## NAME

tests - InjaX type-checking and condition functions for JSON data

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

**Example:**

json

```
{% if [1,2,3] is contains(2) %}yes{% endif %} → "yes"
{% if "hello" is contains("ell") %}yes{% endif %} → "yes"
{% if {"a":1} is contains("a") %}yes{% endif %} → "yes"
```



### `endsWith`

Checks if string ends with suffix.

**Parameters:**

- `value`: String to check
- `suffix`: Suffix to test

**Example:**

json

```
{% if "hello.txt" is endsWith(".txt") %}yes{% endif %} → "yes"
```



### `hasKey`

Checks if object contains specific key.

**Parameters:**

- `obj`: Object to check
- `key`: Key name (string)

**Example:**

json

```
{% if {"name":"John","age":30} is hasKey("age") %}yes{% endif %} → "yes"
```



### `isArray`

Checks if value is a JSON array.

**Parameters:** Single value

**Example:**

json

```
{% if [1,2,3] is isArray %}yes{% endif %} → "yes"
```



### `isBoolean`

Checks if value is boolean (true/false).

**Parameters:** Single value

**Example:**

json

```
{% if true is isBoolean %}yes{% endif %} → "yes"
```



### `isContained`

Checks if item exists in array (reverse of contains).

**Parameters:**

- `item`: Value to find
- `collection`: Array to search

**Example:**

json

```
{% if 2 is isContained([1,2,3]) %}yes{% endif %} → "yes"
```



### `isDefined`

Checks if value is not null.

**Parameters:** Single value

**Example:**

json

```
{% if null is isDefined %}no{% else %}yes{% endif %} → "yes"
```



### `isDivisibleBy`

Checks if integer is divisible by another integer.

**Parameters:**

- `dividend`: Value to test
- `divisor`: Divisor (non-zero)

**Example:**

json

```
{% if 10 is isDivisibleBy(2) %}yes{% endif %} → "yes"
```



### `isEmpty`

Checks if string, array, or object is empty.

**Parameters:** Single value

**Example:**

json

```
{% if "" is isEmpty %}yes{% endif %} → "yes"
{% if [] is isEmpty %}yes{% endif %} → "yes"
```



### `isEscaped`

Checks if string contains HTML escape sequences.

**Parameters:** Single value

**Example:**

json

```
{% if "&lt;tag&gt;" is isEscaped %}yes{% endif %} → "yes"
```



### `isEven`

Checks if integer is even.

**Parameters:** Single value

**Example:**

json

```
{% if 42 is isEven %}yes{% endif %} → "yes"
```



### `isFloat`

Checks if value is floating-point number.

**Parameters:** Single value

**Example:**

json

```
{% if 3.14 is isFloat %}yes{% endif %} → "yes"
```



### `isInteger`

Checks if value is integer number.

**Parameters:** Single value

**Example:**

json

```
{% if 42 is isInteger %}yes{% endif %} → "yes"
```



### `isIterable`

Checks if value is iterable (array or object).

**Parameters:** Single value

**Example:**

json

```
{% if [1,2,3] is isIterable %}yes{% endif %} → "yes"
```



### `isLower`

Checks if string contains no uppercase letters (non-letters ignored).

**Parameters:** Single value

**Example:**

json

```
{% if "hello 123" is isLower %}yes{% endif %} → "yes"
```



### `isMapping`

Checks if value is object (dictionary/map).

**Parameters:** Single value

**Example:**

json

```
{% if {"a":1} is isMapping %}yes{% endif %} → "yes"
```



### `isNone`

Checks if value is null.

**Parameters:** Single value

**Example:**

json

```
{% if null is isNone %}yes{% endif %} → "yes"
```



### `isNull`

Alias for `isNone`. Checks if value is null.

**Example:**

json

```
{% if null is isNull %}yes{% endif %} → "yes"
```



### `isNumber`

Checks if value is number (integer or float).

**Parameters:** Single value

**Example:**

json

```
{% if 42 is isNumber %}yes{% endif %} → "yes"
```



### `isObject`

Checks if value is object.

**Parameters:** Single value

**Example:**

json

```
{% if {"a":1} is isObject %}yes{% endif %} → "yes"
```



### `isOdd`

Checks if integer is odd.

**Parameters:** Single value

**Example:**

json

```
{% if 41 is isOdd %}yes{% endif %} → "yes"
```



### `isSequence`

Checks if value is array.

**Parameters:** Single value

**Example:**

json

```
{% if [1,2,3] is isSequence %}yes{% endif %} → "yes"
```



### `isString`

Checks if value is string.

**Parameters:** Single value

**Example:**

json

```
{% if "hello" is isString %}yes{% endif %} → "yes"
```



### `isUndefined`

Checks if value is null (alias for isNone).

**Parameters:** Single value

**Example:**

json

```
{% if null is isUndefined %}yes{% endif %} → "yes"
```



### `isUpper`

Checks if string contains no lowercase letters (non-letters ignored).

**Parameters:** Single value

**Example:**

json

```
{% if "HELLO 123" is isUpper %}yes{% endif %} → "yes"
```



### `matches`

Checks if string matches regular expression.

**Parameters:**

- `value`: String to test
- `pattern`: Regular expression pattern

**Note:** Uses `std::regex` (ECMAScript syntax).

**Example:**

json

```
{% if "user@example.com" is matches("^[^@]+@[^@]+\.[^@]+$") %}
  valid email
{% endif %}
```



### `startsWith`

Checks if string starts with prefix.

**Parameters:**

- `value`: String to check
- `prefix`: Prefix to test

**Example:**

json

```
{% if "hello.txt" is startsWith("hello") %}yes{% endif %} → "yes"
```



## SEE ALSO

array-filters(7), string-filters(7), html-filters(7), number-filters(7)