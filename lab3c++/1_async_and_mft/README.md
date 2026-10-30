# Раздел 1. Асинхронный ввод-вывод и устройство файловой системы NTFS (MFT)

В этом разделе содержатся решения ключевых задач по асинхронному копированию файлов через Windows API, исследованию резидентных и нерезидентных атрибутов MFT, анализу физического размера файлов на диске и эффекту синхронной блокировки потоков.

---

## Задача 1. Асинхронное копирование большого файла с `OVERLAPPED` и `hEvent`

* **Исходный файл:** [`task1_async_copy_overlapped.cpp`](task1_async_copy_overlapped.cpp)

### Условие
Написать консольное приложение на C++, копирующее файл большого объема (от 1 ГБ) блоками фиксированного размера (например, 64 КБ) с использованием `FILE_FLAG_OVERLAPPED` и событий (`hEvent`).

### Подробный построчный и блочный разбор программы

#### 1. Подключение заголовков и глобальные константы
```cpp
#include <windows.h>
#include <iostream>
#include <iomanip>

const DWORD CHUNK_SIZE = 64 * 1024; // 64 КБ на один блок передачи
```
* `#include <windows.h>` — подключает основной заголовочный файл Win32 API, содержащий объявления структур `OVERLAPPED`, функций `CreateFileW`, `ReadFile`, `WriteFile`, `WaitForSingleObject` и типов дескрипторов `HANDLE`.
* `const DWORD CHUNK_SIZE = 64 * 1024;` — определяет размер порции передачи данных в байтах (65 536 байт = 64 КБ). Размер 64 КБ является оптимальным для большинства контроллеров дисков и соответствует системной гранулярности распределения памяти Windows (`Allocation Granularity`).

#### 2. Вспомогательная функция `CreateDummyFile`
```cpp
bool CreateDummyFile(const wchar_t* path, DWORD sizeMB) {
    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER liSize;
    liSize.QuadPart = (LONGLONG)sizeMB * 1024 * 1024;
    
    if (!SetFilePointerEx(hFile, liSize, NULL, FILE_BEGIN) || !SetEndOfFile(hFile)) {
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);
    return true;
}
```
* `CreateFileW(path, GENERIC_WRITE, ...)` — создает новый файл с правом записи.
* `LARGE_INTEGER liSize; liSize.QuadPart = (LONGLONG)sizeMB * 1024 * 1024;` — 64-битное целое число для указания размера файла (1024 МБ = 1 ГБ).
* `SetFilePointerEx(hFile, liSize, NULL, FILE_BEGIN)` — мгновенно перемещает логический файловый указатель на 1 ГБ вперед от начала файла (`FILE_BEGIN`).
* `SetEndOfFile(hFile)` — устанавливает текущую позицию указателя как физический конец файла. Это позволяет за долю секунды создать тестовый файл любого размера без необходимости циклической побайтовой записи на диск.
* `CloseHandle(hFile)` — закрывает дескриптор созданного файла.

#### 3. Открытие файлов в асинхронном режиме (Overlapped I/O)
```cpp
HANDLE hSrc = CreateFileW(srcPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
HANDLE hDst = CreateFileW(dstPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);
```
* `FILE_FLAG_OVERLAPPED` — **ключевой системный флаг**. Сообщает ядру Windows, что операции чтения и записи не должны блокировать вызывающий поток. Функции `ReadFile` и `WriteFile` возвращают управление немедленно, выполняя передачу данных аппаратно в фоновом режиме (через контроллер DMA).
* `OPEN_EXISTING` — открывает существующий исходный файл.
* `CREATE_ALWAYS` — создает новый файл для копии (перезаписывая старый при наличии).

#### 4. Инициализация объектов синхронизации и структуры `OVERLAPPED`
```cpp
HANDLE hReadEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
HANDLE hWriteEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

OVERLAPPED ovRead;
OVERLAPPED ovWrite;
memset(&ovRead, 0, sizeof(ovRead));
memset(&ovWrite, 0, sizeof(ovWrite));

ovRead.hEvent = hReadEvent;
ovWrite.hEvent = hWriteEvent;
```
* `CreateEventW(NULL, TRUE, FALSE, NULL)` — создает событие ядра с ручным сбросом (`Manual-Reset Event`, второй параметр `TRUE`) и начальным несигнальным состоянием (`FALSE`).
* `memset(&ovRead, 0, sizeof(ovRead))` — **критически важное обнуление полей**. Предотвращает попадание мусорных байт со стека в поля файлового смещения.
* `ovRead.hEvent = hReadEvent;` — привязывает событие к структуре ввода-вывода. Когда драйвер диска закончит чтение блока, ядро автоматически переведет `hReadEvent` в сигнальное состояние.

#### 5. Основной цикл асинхронного копирования
```cpp
while (currentOffset < (ULONGLONG)fileSize.QuadPart) {
    ...
    ovRead.Offset = (DWORD)(currentOffset & 0xFFFFFFFF);
    ovRead.OffsetHigh = (DWORD)(currentOffset >> 32);
    ResetEvent(hReadEvent);

    BOOL readRet = ReadFile(hSrc, buffer, bytesToRead, NULL, &ovRead);
    if (!readRet && GetLastError() != ERROR_IO_PENDING) { ... break; }

    WaitForSingleObject(hReadEvent, INFINITE);
    GetOverlappedResult(hSrc, &ovRead, &bytesTransferred, FALSE);
    ...
    ovWrite.Offset = ovRead.Offset;
    ovWrite.OffsetHigh = ovRead.OffsetHigh;
    ResetEvent(hWriteEvent);

    BOOL writeRet = WriteFile(hDst, buffer, bytesTransferred, NULL, &ovWrite);
    if (!writeRet && GetLastError() != ERROR_IO_PENDING) { ... break; }

    WaitForSingleObject(hWriteEvent, INFINITE);
    GetOverlappedResult(hDst, &ovWrite, &bytesTransferred, FALSE);

    currentOffset += bytesTransferred;
}
```
* `ovRead.Offset` и `ovRead.OffsetHigh` — задают точное 64-битное байтовое смещение в файле. Поскольку в асинхронном режиме скрытый указатель файла не используется, смещение обязано передаваться явно для каждого пакета.
* `ResetEvent(hReadEvent)` — сбрасывает событие в несигнальное состояние перед новым системным вызовом.
* `ReadFile(..., &ovRead)` — запускает асинхронное аппаратное чтение. Возвращает `FALSE`, а `GetLastError()` возвращает `ERROR_IO_PENDING` (код 997 — операция отложена и выполняется драйвером).
* `WaitForSingleObject(hReadEvent, INFINITE)` — переводит поток в ожидание аппаратного прерывания от диска. При этом процессор не нагружается пустыми циклами (CPU = 0%).
* `GetOverlappedResult(...)` — запрашивает у ядра фактический статус завершенной операции и точное количество прочитанных байт (`bytesTransferred`).
* Блок записи `WriteFile` с `ovWrite` работает по полностью аналогичному алгоритму.
* `currentOffset += bytesTransferred;` — сдвигает текущую позицию для чтения следующего 64 КБ блока до достижения конца файла.

---

## Задача 2. Умение читать MFT файл (резидентные и нерезидентные данные)

* **Исходный файл:** [`task2_mft_resident_theory.cpp`](task2_mft_resident_theory.cpp)

### Условие
Умение читать MFT файл при условии, что на диске размер одного и того же файла в одном случае строго меньше 500 КБ, в другом больше 2000.

### Подробный разбор структуры MFT и кода программы

#### 1. Внутреннее устройство таблицы MFT в NTFS
1. Таблица **MFT (Master File Table)** состоит из фиксированных записей размером **1024 байта (1 КБ)**. Каждому файлу на диске соответствует минимум одна такая запись.
2. Внутри записи размещаются атрибуты файла:
   * `$STANDARD_INFORMATION` — метки времени (создание, изменение), атрибуты (Read-Only, Hidden).
   * `$FILE_NAME` — имя файла в формате UTF-16 Unicode.
   * `$DATA` — фактическое содержимое файла.
3. **Резидентный атрибут (`Resident Data`):**
   * Если размер содержимого файла меньше примерно **700–800 байт** (например, 200 байт), данные помещаются прямо внутрь 1024-байтной записи MFT!
   * В этом случае на томе **не выделяется ни одного кластера** диска. Физический размер файла вне MFT равен **0 байт**.
4. **Нерезидентный атрибут (`Non-Resident Data`):**
   * Если размер файла превышает свободное место в записи MFT (например, 2000 байт или 500 КБ), данные не помещаются в запись.
   * NTFS выделяет на дисковом разделе отдельные блоки — **кластеры** (обычно по 4096 байт = 4 КБ).
   * Внутри записи MFT вместо самих данных сохраняется так называемый `Data Run` (список цепочек экстентов), отображающий виртуальные кластеры файла (`VCN` — Virtual Cluster Number) на физические кластеры диска (`LCN` — Logical Cluster Number).

#### 2. Разбор системного вызова `FSCTL_GET_RETRIEVAL_POINTERS`
```cpp
STARTING_VCN_INPUT_BUFFER inputVcn;
inputVcn.StartingVcn.QuadPart = 0;

BYTE outBuf[1024];
DWORD bytesReturned = 0;

BOOL fsctlSuccess = DeviceIoControl(
    hFile,
    FSCTL_GET_RETRIEVAL_POINTERS,
    &inputVcn,
    sizeof(inputVcn),
    outBuf,
    sizeof(outBuf),
    &bytesReturned,
    NULL
);
```
* `FSCTL_GET_RETRIEVAL_POINTERS` — специальный управляющий код драйвера NTFS (`File System Control`). Запрашивает физическую карту размещения кластеров файла на накопителе.
* `inputVcn.StartingVcn.QuadPart = 0;` — запрашивает цепочки кластеров с самого начала файла (с VCN 0).
* **Поведение при резидентном файле:** вызов возвращает `FALSE`, а код ошибки `GetLastError()` равен `ERROR_HANDLE_EOF` (38). Драйвер NTFS сообщает: "У файла нет экстентов на диске, данные целиком лежат внутри записи MFT".
* **Поведение при нерезидентном файле:** вызов возвращает `TRUE`. Структура `RETRIEVAL_POINTERS_BUFFER` содержит:
  * `rp->ExtentCount` — количество непрерывных фрагментов (экстентов) на диске;
  * `rp->Extents[i].Lcn` — начальный физический номер кластера (Logical Cluster Number) на дисковом разделе;
  * `rp->Extents[i].NextVcn` — виртуальный кластер, на котором заканчивается данный фрагмент.

---

## Задача 3. Физический размер файла вне таблицы MFT (`GetCompressedFileSize`)

* **Исходный файл:** [`task3_mft_physical_size.cpp`](task3_mft_physical_size.cpp)

### Условие
Написать небольшую утилиту, которая создает два файла с разным объемом текстовых данных, а затем опрашивает файловую систему о том, сколько реальных физических байт эти файлы заняли на жестком диске вне таблицы MFT:
* **Кейс 1 (Маленький файл):** Размер содержимого — строго 200 байт.
* **Кейс 2 (Большой файл):** Размер содержимого — строго 2000 байт (чуть меньше 2 КБ).

### Построчный разбор функции `investigateFile`
```cpp
void investigateFile(const wchar_t* fileName, DWORD bytesToWrite) {
    // 1. Создаем файл на запись
    HANDLE hFile = CreateFileW(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    ...
    // 2. Выделяем буфер в куче и заполняем символом 'A'
    char* buffer = new char[bytesToWrite];
    memset(buffer, 'A', bytesToWrite);
    
    // 3. Записываем данные в файл
    DWORD bytesWritten = 0;
    WriteFile(hFile, buffer, bytesToWrite, &bytesWritten, NULL);
    
    // 4. Обязательно закрываем дескриптор, чтобы драйвер NTFS сбросил метаданные на диск
    CloseHandle(hFile);
    delete[] buffer;
    
    // 5. Системный опрос реального физического дискового пространства
    DWORD highSize = 0;
    DWORD physicalSize = GetCompressedFileSizeW(fileName, &highSize);
    ...
}
```
* `GetCompressedFileSizeW(fileName, &highSize)` — возвращает **реальное количество физических байт**, выделенных файловой системой на диске под хранение файла.
* **Результат работы Кейса 1 (200 байт):**
  * Логический размер: `200` байт.
  * Физический размер: **`0` байт**.
  * *Причина:* 200 байт полностью поместились в структуру `MFT RECORD` (размером 1024 байта). Дисковые кластеры вне таблицы MFT не расходовались.
* **Результат работы Кейса 2 (2000 байт):**
  * Логический размер: `2000` байт.
  * Физический размер: **`4096` байт** (в файловой системе с кластером 4 КБ).
  * *Причина:* 2000 байт превышают размер свободной части MFT-записи. Файл стал нерезидентным, и файловая система выделила под него один целый кластер диска (4096 байт).

---

## Задача 4. Исследование эффекта блокировки потока при синхронном I/O

* **Исходный файл:** [`task4_sync_blocking_demo.cpp`](task4_sync_blocking_demo.cpp)

### Условие
Написать консольную программу, которая эмулирует работу интерфейса (выводит бегущие тики в цикле) и по команде начинает тяжелое синхронное чтение файла с диска, демонстрируя эффект замирания потока.

### Построчный разбор эффекта блокировки
```cpp
for (int i = 1; i <= 10; i++) {
    std::wcout << L"Интерфейс: Тик " << i << L" [Поток активен, State: Running]" << std::endl;
    Sleep(500); // Эмуляция обработки событий интерфейса

    if (i == 5) {
        // Синхронный системный вызов ReadFile:
        ReadFile(hFile, buffer, bufferSize, &bytesRead, NULL);
    }
}
```
1. **Состояния потока в ОС Windows:**
   * `Running` — поток выполняется на физическом ядре CPU (выводит тики в цикле).
   * `Ready` — поток готов к исполнению и ждет своей очереди в диспетчере планировщика Windows.
   * `Waiting (Blocked)` — поток добровольно или вынужденно уступил процессор и ожидает внешнего события.
2. **Что происходит в момент `ReadFile` без флага `FILE_FLAG_OVERLAPPED`:**
   * Поток выполняет программное прерывание (`syscall / sysenter`) и переходит в ядро Windows.
   * Ядро формирует пакет `IRP` и передает его дисковому стеку драйверов (`storport.sys`, `nvme.sys`).
   * Планировщик Windows видит, что операция синхронная, и немедленно переводит вызывающий поток из состояния `Running` в состояние **`Waiting`**.
   * Поток полностью исключается из очередей процессора.
   * Цикл `for` замирает: консоль перестает выводить тики, а если бы это было GUI-приложение с окном, Windows пометила бы окно как **«Не отвечает» (Not Responding)**.
3. **Пробуждение потока:**
   * Когда контроллер накопителя завершает передачу 64 МБ через DMA в память, он генерирует аппаратное прерывание (Hardware Interrupt).
   * Драйвер диска сигнализирует ядру о завершении IRP.
   * Планировщик переводит поток из состояния `Waiting` в `Ready`, а затем запускает его на свободном ядре CPU.
   * Функция `ReadFile` возвращает управление, и интерфейс "оживает".
