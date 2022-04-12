# Язык Lala

Lala — процедурно-ориентированный, статически типизированный, байткод-компилируемый язык программирования с автоматической сборкой мусора.

- [Комментарии](#comments)
- [Переменные](#variables)
- [Система типов](#type-system)
  - [Базовые типы](#base-types)
  - [Массивы](#arrays)
  - [Словари](#maps)
  - [Структуры](#structures)
- [Операторы](#operators)
- [Поток управления](#control-flow)
  - [Условный оператор](#if)
  - [Циклы](#loops)
    - [While](#while)
    - [Do-while](#do-while)
    - [For-in](#for-in)
    - [Continue, break](#continue-break)
  - [Функции](#functions)

<a name="comments"/>

## Комментарии

Однострочный комментарий начинается с `|`.

```
| Single-line comment
```

Многострочный комменатрий начинается с `/-` и заканчивается `-/`. 

```
/-
Multiline comment
-/
```

Такой синтаксис позволяет удобно создавать комментарии в рамках

```
/----------------\
| Framed comment |
\----------------/
```

<a name="variables"/>

## Переменные

Переменные определяются при помощи следующего синтаксиса:
```
(const|var) <name>: <type> (= <expression>)?
```

Доступны следующие операции-присваивания: `=`, `+=`, `-=`, `*=`, `/=`, `%=`

Например:
```
const i: int
var   f: float  = 0.76
const s: string = 'Hello, world!'

f = 0.65          | OK
i = 1             | Error (i is const)

f += 1.0
```


<a name="type-system"/>

## Система типов

<a name="base-types"/>

### Базовые типы

| Тип            | Пример               | Значение по умолчанию |
| -------------- | -------------------- | --------------------- |
| `bool`         | `true`, `false`      | `false`               |
| `int`          | `12`, `0`, `-7`      | `0`                   |
| `float`        | `9.834`, `-0.76`     | `0.0`                 |
| `string`       | `'Hello, world!'`    | `''`                  |

<a name="arrays"/>

### Массивы

Массив позволяет хранить последовательность элементов одного типа.

```
var array: [string] = [ 'Hello,', 'world!' ]

print(array[0] + ' ' + array[1])
| 'Hello, world!'

append(array, 'My name is Sonia')
print(array)
| ['Hello,', 'world!', 'My name is Sonia']
```

<a name="maps"/>

### Словари

Словари позволяют хранить набор пар ключ-значение и предоставляет доступ к значениям по ключам.

```
var map: {string: int} = {
  'apples':  10,
  'pears':    2,
  'tomatos':  8,
}
map['cucumbers'] = 1

print(map['pears'])
| 2

print(map)
| {'apples': 10, 'pears': 2, 'tomatos': 8, 'cucumbers': 1}
```

<a name="structures"/>

### Структуры

Пользователь может определять другие типы на основе базовых при помощи структур.

```
structure Person {
  age: int,
  name: string,
}

const person: Person(age=20, name='Sonia')

print(person.name)
| 'Sonia'

print(person)
| Person(age=20, name='Sonia')
```

<a name="operators"/>

## Операторы

| Приоритет     | Оператор  | Название         | Типы операндов                      |
| ------------- | --------- | ---------------- | ----------------------------------- |
| 1. postfix    | `a.`      | member access    | object                              | 
|               | `a()`     | function call    | function                            |
|               | `a[]`     | subscript        | array, map                          |
|               | `a: b`    | type cast        | int-float, float-int, any-string    |
| 2. prefix     | `-a`      | unary minus      | int, float                          |
|               | `!a`      | negation         | bool                                |
| 3. factor     | `a * b`   | multiplication   | int-int, float-float                |
|               | `a / b`   | division         | int-int, float-float                |
|               | `a % b`   | modulo           | int-int, float-int                  |
| 4. term       | `a + b`   | addition         | int-int, float-float                |
|               | `a + b`   | concatenation    | string-string                       |
|               | `a - b`   | subtraction      | int-int, float-float                |
| 5. comparison | `a == b`  | equality         | int-int, float-float, string-string |
|               | `a != b`  | inequality       | int-int, float-float, string-string |
|               | `a >= b`  | greater or equal | int-int, float-float, string-string |
|               | `a <= b`  | less or equal    | int-int, float-float, string-string |
|               | `a > b`   | greater          | int-int, float-float, string-string |
|               | `a < b`   | less             | int-int, float-float, string-string |
| 6. and        | `a and b` | logical and      | bool-bool                           |
| 7. or         | `a or b`  | logical or       | bool-bool                           |

Не поддерживаются неявные приведения типов, применение операндов возможно только на типах, указанных в таблице.

<a name="control-flow"/>

## Поток управления

<a name="if"/>

### Условный оператор

```
const a: int = -3
if (a > 0) {
  print('greater)
} else if (a < 0) {
  print('less')
} else {
  print('equal')
}
| less
```

<a name="loops"/>

### Циклы

<a name="while"/>

#### While

```
var i: 0
while (i < 5) {
  i += 1
  print(i)
}
| 1 2 3 4 5
```

<a name="do-while"/>

#### Do-while

```
var i: 0
do {
  i += 1
  print(i)
} while (i < 5)
| 1 2 3 4 5
```

<a name="for-in"/>

#### For-in

```
for (str in ['a', 'b', 'c']) {
  print(str)
}
| a b c
```

<a name="continue-break"/>

#### continue, break

`continue` переходит к следующей итерации цикла, `break` выходит из цикла

<a name="functions"/>

## Функции

```
function sum(const a:int, const b: int): int {
  return a + b
}

print(sum(12, 3))
| 15
```
