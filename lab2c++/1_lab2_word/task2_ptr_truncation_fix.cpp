#include <windows.h>
#include <iostream>

// Исправлено: замена DWORD на DWORD_PTR для предотвращения усечения адреса в x64
void ProcessData(DWORD_PTR dataStylePointer) {
    int* pNumber = reinterpret_cast<int*>(dataStylePointer);
    if (pNumber != nullptr) {
        std::cout << "Value: " << *pNumber << std::endl;
    }
}

int main() {
    #if defined(_WIN32)
    system("chcp 65001 > nul");
    #endif

    int criticalValue = 2026;
    int* lpContext = &criticalValue;

    // Исправлено: каст указателя в 64-битный тип DWORD_PTR вместо 32-битного DWORD
    DWORD_PTR SAFE_PTR = (DWORD_PTR)lpContext;

    ProcessData(SAFE_PTR);

    return 0;
}
