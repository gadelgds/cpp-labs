// Лабораторная работа №8, задание 2.
// Приостановленный поток (CREATE_SUSPENDED) и перехват через RIP.
// Сборка MSVC:  cl /EHsc task2_suspended_hook.cpp
// Сборка MinGW: g++ -o task2.exe task2_suspended_hook.cpp
// Запуск: task2.exe

#include <windows.h>
#include <iostream>

// Номер Фибоначчи для расчёта. Fib(40) рекурсивно —
// около 100 миллионов вызовов, это занимает несколько секунд:
// расчёт достаточно тяжёлый, чтобы поток был «настоящей работой»,
// а не мгновенным действием.
const int FIB_N = 40;

// Сюда хук запишет результат расчёта.
long long g_result = -1;

// Рекурсивное вычисление чисел Фибоначчи.
// Медленно, но наглядно: каждый вызов порождает два новых.
long long FibRecursive(int n)
{
    if (n <= 1) return n;
    return FibRecursive(n - 1) + FibRecursive(n - 2);
}

// Оригинальная функция потока: то, с чего поток должен был стартовать.
// Если перехват сработал, эта функция НЕ выполнится ни разу —
// её сообщение на экране не появится. Это и есть доказательство.
DWORD WINAPI FibThread(LPVOID /* param */)
{
    std::cout << "ОРИГИНАЛ: поток стартовал со своей функции FibThread."
              << std::endl;
    g_result = FibRecursive(FIB_N);
    std::cout << "ОРИГИНАЛ: Fib(" << FIB_N << ") = " << g_result
              << std::endl;
    return 0;
}

// Функция-перехватчик (хук). Поток стартует с неё вместо FibThread:
// мы подменили регистр RIP в контексте потока до его запуска.
// Получает тот же параметр, что и оригинальная функция потока.
DWORD WINAPI HookThread(LPVOID /* param */)
{
    std::cout << "ПЕРЕХВАТ: поток стартовал с хука HookThread, "
              << "а не с FibThread!" << std::endl;
    std::cout << "ПЕРЕХВАТ: выполняю расчёт Fib(" << FIB_N << ")..."
              << std::endl;
    g_result = FibRecursive(FIB_N);
    std::cout << "ПЕРЕХВАТ: Fib(" << FIB_N << ") = " << g_result
              << std::endl;
    return 0;
}

int main()
{
#if defined(_WIN32)
    system("chcp 65001 > nul"); // Русский текст в консоли без кракозябр.
#endif

    std::cout << "=== ЗАДАНИЕ 2: CREATE_SUSPENDED + перехват через RIP ==="
              << std::endl;
    std::cout << "Расчёт: Fib(" << FIB_N << ") рекурсивно." << std::endl;

    // Создаём поток ПРИОСТАНОВЛЕННЫМ: он не выполнит ни одной
    // инструкции, пока мы не вызовем ResumeThread.
    HANDLE hThread = CreateThread(
        nullptr, 0, FibThread, nullptr, CREATE_SUSPENDED, nullptr);
    if (hThread == nullptr) {
        std::cout << "Ошибка CreateThread: " << GetLastError() << std::endl;
        return 1;
    }
    std::cout << "Поток создан с CREATE_SUSPENDED и пока спит." << std::endl;

    // Готовим структуру контекста. ContextFlags говорит системе,
    // какие регистры нам нужны: CONTEXT_FULL — все основные регистры,
    // включая RIP. Без этого флага GetThreadContext вернёт мусор.
    CONTEXT ctx;
    ZeroMemory(&ctx, sizeof(ctx));
    ctx.ContextFlags = CONTEXT_FULL;

    // Читаем контекст приостановленного потока.
    // На работающем потоке это делать нельзя — контекст «плывёт».
    if (!GetThreadContext(hThread, &ctx)) {
        std::cout << "Ошибка GetThreadContext: " << GetLastError()
                  << std::endl;
        CloseHandle(hThread);
        return 1;
    }

#ifdef _WIN64
    std::cout << "Исходный RIP потока: 0x" << std::hex << ctx.Rip
              << std::dec << std::endl;
    // Подмена точки входа: поток начнётся с хука, а не с FibThread.
    // На x64 регистр называется RIP (на x86 был бы EIP).
    ctx.Rip = reinterpret_cast<DWORD64>(HookThread);
    std::cout << "Новый RIP потока:    0x" << std::hex << ctx.Rip
              << std::dec << " (адрес HookThread)" << std::endl;
#else
    std::cout << "Исходный EIP потока: 0x" << std::hex << ctx.Eip
              << std::dec << std::endl;
    ctx.Eip = reinterpret_cast<DWORD>(HookThread);
    std::cout << "Новый EIP потока:    0x" << std::hex << ctx.Eip
              << std::dec << " (адрес HookThread)" << std::endl;
#endif

    // Записываем изменённый контекст обратно в поток.
    if (!SetThreadContext(hThread, &ctx)) {
        std::cout << "Ошибка SetThreadContext: " << GetLastError()
                  << std::endl;
        CloseHandle(hThread);
        return 1;
    }
    std::cout << "Контекст изменён: поток стартует с хука." << std::endl;

    // Оживляем поток: теперь он выполнит первую инструкцию
    // уже по новому адресу — из HookThread.
    DWORD prevSuspend = ResumeThread(hThread);
    std::cout << "ResumeThread вызван (счётчик приостановки был "
              << prevSuspend << "). Поток работает..." << std::endl;

    // Ждём, пока поток закончит расчёт.
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    std::cout << "Поток завершён. Итог: Fib(" << FIB_N << ") = "
              << g_result << std::endl;
    if (g_result == 102334155) {
        std::cout << "Результат верный: Fib(40) = 102334155." << std::endl;
    }
    std::cout << "Доказательство перехвата: сообщение «ПЕРЕХВАТ» есть, "
              << "а сообщение «ОРИГИНАЛ» не появилось — FibThread "
              << "так и не запустилась." << std::endl;

    return 0;
}
