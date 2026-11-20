# Лабораторная работа №8: Потоки, CRT и перехват через контекст потока

В данной лабораторной работе исследуются два механизма Windows: почему краткоживущие потоки, созданные через `CreateThread`, текут памятью (CRT-состояние потока не освобождается), а через `_beginthreadex` — нет; и как перехватить старт потока, подменив указатель инструкций (RIP) в его контексте до `ResumeThread`.

> **Код программ**: [`programs.html`](programs.html) — обе программы лабы: код байт-в-байт как в .cpp-файлах, кнопки копирования и блоки «Как запустить» у каждой задачи.
> **Объяснения**: [`explanations.html`](explanations.html) — построчный разбор каждой программы, теория (CRT и TLS потока, Private Bytes, `CREATE_SUSPENDED`, контекст потока, RIP, хук) и словарь терминов (16 терминов).
> **STE-версия** (стиль ASD-STE100): отдельный раздел в [`explanations.html`](explanations.html) — STE-разбор, STE-теория и STE-словарь. Код не тронут.

---

## Структура файлов лабораторной работы №8

| Файл | Назначение |
| :--- | :--- |
| [`programs.html`](programs.html) | Код обеих программ + кнопки копирования + «Как запустить» у каждой задачи |
| [`explanations.html`](explanations.html) | Построчный разбор, теория по каждой задаче, словарь (16 терминов) + раздел «STE-версия» |
| [`task1_createthread_vs_beginthreadex.cpp`](task1_createthread_vs_beginthreadex.cpp) | Задача 1: 10 000 краткоживущих потоков, внутри потока — `strtok` по тестовой строке; вариант `CreateThread` или `_beginthreadex` — аргументом командной строки |
| [`task2_suspended_hook.cpp`](task2_suspended_hook.cpp) | Задача 2: поток `CREATE_SUSPENDED` для Fib(40); подмена RIP на функцию-перехватчик через `GetThreadContext` / `SetThreadContext`, затем `ResumeThread` |

---

## Задачи

### Задача 1. CreateThread против _beginthreadex (утечка CRT-памяти)

- Программа в цикле 10 000 раз создаёт краткоживущий поток; внутри потока 100 раз вызывается `strtok` для тестовой строки.
- Без аргумента или с аргументом `create` — вариант `CreateThread`; с аргументом `begin` — вариант `_beginthreadex`.
- Программа печатает вариант, счётчик потоков и память процесса (`GetProcessMemoryInfo`: WorkingSet и PrivateUsage) на старте, каждые 1000 потоков и на финише.
- Суть: `CreateThread` не инициализирует CRT-состояние потока; `strtok` использует CRT TLS, и при выходе потока это состояние не освобождается — утечка на каждый поток, Private Bytes растёт. `_beginthreadex` инициализирует и освобождает CRT-данные потока — потребление стабилизируется.
- Рост Private Bytes можно смотреть и в диспетчере задач (столбец «Выделенная память» / Private Bytes).

### Задача 2. CREATE_SUSPENDED + перехват через RIP

- Поток создаётся с флагом `CREATE_SUSPENDED` для вычисления Fib(40) рекурсивно (около 100 млн вызовов — настоящая работа, несколько секунд).
- До `ResumeThread` — `GetThreadContext` с `ContextFlags = CONTEXT_FULL`, замена RIP (x64) на функцию-перехватчик, `SetThreadContext`, затем `ResumeThread`.
- Хук печатает сообщение о перехвате, сам считает Fib и записывает результат в общую переменную; исходная функция потока при этом не запускается (её сообщение «ОРИГИНАЛ» не печатается — это и есть доказательство перехвата).
- Ожидаемый результат: Fib(40) = 102334155.

---

## Условия запуска

- Только Windows (x64): программы используют `windows.h`, `process.h` (`_beginthreadex`), `psapi.h` (`GetProcessMemoryInfo`).
- Компиляция. MSVC: `cl /EHsc task1_createthread_vs_beginthreadex.cpp` (линковка с `psapi.lib`: `cl /EHsc task1_createthread_vs_beginthreadex.cpp psapi.lib`). MinGW: `g++ -o task1.exe task1_createthread_vs_beginthreadex.cpp -lpsapi`. Аналогично для задачи 2 (без psapi).
- Задача 1: `task1.exe` (или `task1.exe create`) — вариант CreateThread; `task1.exe begin` — вариант _beginthreadex. Для сравнения запустите оба варианта по очереди и сравните PrivateUsage на финише.
- Честная оговорка: код в песочнице не компилировался (компилятора Windows там нет), ожидаемый вывод выведен из логики кода — прогон на домашнем ПК.
