#include <windows.h>
#include <iostream>

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    std::cout << "=== ЗАДАНИЕ 1: ЖИЗНЕННЫЙ ЦИКЛ ПРОЦЕССА И ОЧИСТКА РЕСУРСОВ ЯДРА ===" << std::endl;
    std::cout << "Запускаем Блокнот (notepad.exe) через CreateProcess..." << std::endl;

    // 1. Структуры, описывающие запуск нового процесса.
    //    STARTUPINFO — как должен выглядеть новый процесс (окно, дескрипторы ввода/вывода).
    //    PROCESS_INFORMATION — что вернёт нам система о запущенном процессе.
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // 2. Обязательно обнуляем обе структуры: в них не должно остаться мусора.
    //    Неинициализированные поля STARTUPINFO — классическая причина
    //    непонятных сбоев CreateProcess.
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));

    // 3. Поле cb — размер структуры в байтах. CreateProcess проверяет его,
    //    чтобы понять, какую версию STARTUPINFO мы используем.
    si.cb = sizeof(si);

    // 4. Создаём дочерний процесс — Блокнот.
    //    Первый параметр (lpApplicationName) — NULL: имя программы берётся
    //    из командной строки. Второй параметр — командная строка.
    //    Остальные параметры по умолчанию: наследование дескрипторов выключено,
    //    флаги создания — 0, окружение и текущий каталог — как у родителя.
    //    Важно: буфер командной строки должен быть изменяемым (wchar_t[]),
    //    потому что CreateProcess имеет право его менять.
    wchar_t cmdLine[] = L"notepad.exe";

    BOOL ok = CreateProcess(
        NULL,           // lpApplicationName: NULL — имя берём из командной строки
        cmdLine,        // lpCommandLine: изменяемая строка "notepad.exe"
        NULL,           // lpProcessAttributes: дескриптор процесса не наследуется
        NULL,           // lpThreadAttributes: дескриптор потока не наследуется
        FALSE,          // bInheritHandles: дочерний процесс НЕ наследует наши дескрипторы
        0,              // dwCreationFlags: без особых флагов
        NULL,           // lpEnvironment: NULL — копия блока окружения родителя
        NULL,           // lpCurrentDirectory: NULL — текущий каталог родителя
        &si,            // lpStartupInfo: как запускать (заполнена выше)
        &pi             // lpProcessInformation: сюда система запишет дескрипторы и ID
    );

    // 5. Проверяем результат. FALSE — процесс не создан, деталей нет:
    //    ни дескрипторов, ни PID — закрывать нечего, просто выходим с кодом 1.
    if (!ok) {
        std::cout << "Ошибка CreateProcess: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Блокнот запущен. PID процесса: " << pi.dwProcessId
              << ", PID главного потока: " << pi.dwThreadId << std::endl;
    std::cout << "Ждём, пока вы закроете Блокнот..." << std::endl;

    // 6. Ждём завершения дочернего процесса. WaitForSingleObject блокирует
    //    наш поток, пока объект-дескриптор процесса pi.hProcess не перейдёт
    //    в сигнальное состояние — это происходит ровно в момент завершения
    //    процесса. INFINITE — ждать без ограничения по времени.
    //    Возврат WAIT_OBJECT_0 означает: процесс завершился.
    DWORD waitResult = WaitForSingleObject(pi.hProcess, INFINITE);

    if (waitResult == WAIT_OBJECT_0) {
        std::cout << "Блокнот закрыт пользователем." << std::endl;
    } else {
        std::cout << "WaitForSingleObject вернул неожиданный код: " << waitResult
                  << " (GetLastError: " << GetLastError() << ")" << std::endl;
    }

    // 7. КРИТИЧЕСКОЕ УСЛОВИЕ ЗАДАНИЯ: закрываем дескрипторы процесса и потока.
    //    Пока дескриптор открыт, ядро хранит объект процесса (и его код
    //    завершения) в памяти, даже если сам процесс уже завершён.
    //    Незакрытый дескриптор = утечка ресурса ядра + занятая запись
    //    в таблице дескрипторов процесса. Порядок закрытия не важен,
    //    но закрыть нужно ОБА дескриптора.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::cout << "Дескрипторы процесса и потока закрыты (CloseHandle). Ресурсы ядра освобождены." << std::endl;

    return 0;
}
