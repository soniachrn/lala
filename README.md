# Язык Lala

Lala — процедурно-ориентированный, статически типизированный, байткод-компилируемый язык программирования с автоматической сборкой мусора.

- [Установка](#installation)
- [Использование](#usage)
  - [Компиляция](#compilation)
  - [Исполнение](#execution)
- [Язык](#language)
  - [Комментарии](#comments)
  - [Переменные](#variables)
  - [Система типов](#type-system)
    - [Базовые типы](#base-types)
    - [Массивы](#arrays)
    - [Структуры](#structures)
  - [Операторы](#operators)
  - [Поток управления](#control-flow)
    - [Условный оператор](#if)
    - [Циклы](#loops)
      - [While](#while)
      - [Do-while](#do-while)
    - [Функции](#functions)
  - [Примеры](#examples)

<a name="installation"/>

## Установка

1. Скачать репозиторий
```
git clone https://github.com/soniachrn/lala.git lala
```

2. Собрать lala
```
cmake -S lala -B lala/build
make -C lala/build lala
```

3. Создать алиас в `.zshrc` или в `.bashrc`
```
echo "alias lala='<cwd>/lala/build/lala'" >> <~/.zshrc или ~/.bashrc>
```

<a name="usage"/>

## Использование

<a name="compilation"/>

### Компиляция

```
lala compile <файл исходного кода lala> <результирующий файл байткода lalaby>
```

<a name="execution"/>

### Исполнение

```
lala execute <файл байткода lalaby>
```

<a name="language"/>

## Язык

<a name="comments"/>

### Комментарии

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

### Переменные

Переменные определяются при помощи следующего синтаксиса:
```
var <name>: <type> (= <expression>)
```

Например:
```
var i: int    = 9
var   f: float  = 0.76
var s: string = 'Hello, world!'

f = 0.65
i = 1

f = f + 1.0
```


<a name="type-system"/>

### Система типов

<a name="base-types"/>

#### Базовые типы

| Тип            | Пример               |
| -------------- | -------------------- |
| `bool`         | `true`, `false`      |
| `int`          | `12`, `0`, `-7`      |
| `float`        | `9.834`, `-0.76`     |
| `string`       | `'Hello, world!'`    |

<a name="arrays"/>

#### Массивы

Массив позволяет хранить последовательность элементов одного типа.

```
var array: [string] = [ 'Hello,', 'world!' ]

print(array[0] + ' ' + array[1])
| 'Hello, world!'
```

<a name="structures"/>

#### Структуры

Пользователь может определять другие типы на основе базовых при помощи структур.

```
structure Person {
  age: int
  name: string
}

const person: Person(20, 'Sonia')

print(person.name)
| 'Sonia'
```

<a name="operators"/>

### Операторы

| Приоритет     | Оператор  | Название         | Типы операндов                      |
| ------------- | --------- | ---------------- | ----------------------------------- |
| 1. postfix    | `a.b`     | member access    | object                              | 
|               | `a()`     | function call    | function                            |
|               | `a[]`     | subscript        | array, map                          |
|               | `a: b`    | type cast        | int-float, float-int, any-string    |
| 2. prefix     | `-a`      | unary minus      | int, float                          |
|               | `!a`      | negation         | bool                                |
| 3. factor     | `a * b`   | multiplication   | int-int, float-float                |
|               | `a / b`   | division         | int-int, float-float                |
|               | `a % b`   | modulo           | int-int                             |
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

### Поток управления

<a name="if"/>

#### Условный оператор

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

#### Циклы

<a name="while"/>

##### While

```
var i: int = 0
while (i < 5) {
  i = i + 1
  print(i)
}
| 1 2 3 4 5
```

<a name="do-while"/>

##### Do-while

```
var i: int = 0
do {
  i = i + 1
  print(i)
} while (i < 5)
| 1 2 3 4 5
```

<a name="functions"/>

### Функции

```
function sum(var a: int, var b: int): int {
  return a + b
}

print(sum(12, 3))
| 15
```

<a name="examples"/>

### Примеры

В папке [example](example)
