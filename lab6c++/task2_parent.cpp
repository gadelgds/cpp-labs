#include <windows.h>
#include <iostream>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== ЗАДАНИЕ 2: PARENT (РОДИТЕЛЬ) ===" << std::endl;

    // 1. Динамически формируем переменную окружения STUDENT_ID=19045
    //    в СОБСТВЕННОМ блоке окружения этого процесса.
    //    SetEnvironmentVariable меняет переменную только в памяти текущего
    //    процесса — ни в системе, ни в других процессах она не появляется.
    const wchar_t* varName = L"STUDENT_ID";
    const wchar_t* valueBefore = L"19045";

    if (!SetEnvironmentVariable(varName, valueBefore)) {
        std::cout << "Ошибка SetEnvironmentVariable: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Parent: установлена переменная STUDENT_ID=";
    std::wcout << valueBefore << std::endl;

    // 2. Готовим структуры для запуска дочернего процесса — Child.exe.
    //    Обнуление обязательно: мусор в STARTUPINFO ломает CreateProcess.
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    // 3. Запускаем Child.exe. Имя — относительное: L"Child.exe".
    //    Поэтому Child.exe ОБЯЗАТЕЛЬНО должен лежать в той же папке,
    //    что и Parent.exe, иначе CreateProcess вернёт FALSE.
    //    lpEnvironment = NULL — это ключевой момент задания: потомок
    //    получает КОПИЮ блока окружения родителя на момент запуска.
    //    Копию — значит, потомок живёт со своим снимком, а не с общей памятью.
    wchar_t cmdLine[] = L"Child.exe";

    BOOL ok = CreateProcess(
        NULL,           // lpApplicationName: NULL — имя берём из командной строки
        cmdLine,        // lpCommandLine: "Child.exe" (должен лежать рядом с Parent.exe)
        NULL,           // lpProcessAttributes: дескриптор процесса не наследуется
        NULL,           // lpThreadAttributes: дескриптор потока не наследуется
        FALSE,          // bInheritHandles: дочерний процесс НЕ наследует наши дескрипторы
        0,              // dwCreationFlags: без особых флагов
        NULL,           // lpEnvironment: NULL — Child получает КОПИЮ нашего блока окружения
        NULL,           // lpCurrentDirectory: NULL — текущий каталог родителя
        &si,            // lpStartupInfo
        &pi             // lpProcessInformation: дескрипторы и ID потомка
    );

    if (!ok) {
        std::cout << "Ошибка CreateProcess (не найден Child.exe рядом с Parent.exe?): "
                  << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Parent: Child.exe запущен. PID потомка: " << pi.dwProcessId << std::endl;

    // 4. СРАЗУ ПОСЛЕ ЗАПУСКА перезаписываем переменную У СЕБЯ на другое значение.
    //    Это происходит ПОСЛЕ CreateProcess, поэтому копия блока окружения,
    //    уже переданная потомку, НЕ меняется: у потомка остался снимок
    //    со значением 19045, а у нас теперь 99999.
    const wchar_t* valueAfter = L"99999";
    if (!SetEnvironmentVariable(varName, valueAfter)) {
        std::cout << "Ошибка повторного SetEnvironmentVariable: " << GetLastError() << std::endl;
    } else {
        std::cout << "Parent: переменная у СЕБЯ перезаписана на STUDENT_ID=";
        std::wcout << valueAfter << std::endl;
        std::cout << "Parent: у потомка осталась КОПИЯ со значением 19045 — она не изменилась." << std::endl;
    }

    // 5. Родитель НЕ ждёт потомка (в отличие от задания 1 здесь нет
    //    WaitForSingleObject) — он мгновенно завершается. Потомок при этом
    //    продолжает жить сам по себе: завершение родителя его не убивает.
    //    Но перед выходом всё равно закрываем дескрипторы потомка,
    //    иначе они утекут в ядро (см. задание 1).
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::cout << "Parent: дескрипторы потомка закрыты, родитель завершает работу." << std::endl;

    return 0;
}
