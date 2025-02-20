#include <windows.h>
#include <shobjidl.h>
#include <TlHelp32.h>

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>

bool OpenFileDialog(std::vector<std::wstring>& paths, bool selectFolder = false, bool multiSelect = false, const WCHAR* title = L"Select Folder", const WCHAR* defaultFolderPath = L"C:\\")
{
    IFileOpenDialog* p_file_open = nullptr;
    bool are_all_operation_success = false;
    while (!are_all_operation_success)
    {
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
            IID_IFileOpenDialog, reinterpret_cast<void**>(&p_file_open));
        if (FAILED(hr))
            break;

        if (selectFolder || multiSelect)
        {
            FILEOPENDIALOGOPTIONS options = 0;
            hr = p_file_open->GetOptions(&options);
            if (FAILED(hr))
                break;

            if (selectFolder)
                options |= FOS_PICKFOLDERS;
            if (multiSelect)
                options |= FOS_ALLOWMULTISELECT;

            hr = p_file_open->SetOptions(options);
            if (FAILED(hr))
                break;
        }

        p_file_open->SetTitle(title);
        p_file_open->SetOkButtonLabel(L"Select");

        IShellItem* p_default_folder = nullptr;
        hr = SHCreateItemFromParsingName(defaultFolderPath, NULL, IID_PPV_ARGS(&p_default_folder));
        if (SUCCEEDED(hr))
        {
            p_file_open->SetDefaultFolder(p_default_folder);
            p_default_folder->Release();
        }

        hr = p_file_open->Show(NULL);
        if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) // No items were selected.
        {
            are_all_operation_success = true;
            break;
        }
        else if (FAILED(hr))
            break;

        IShellItemArray* p_items;
        hr = p_file_open->GetResults(&p_items);
        if (FAILED(hr))
            break;
        DWORD total_items = 0;
        hr = p_items->GetCount(&total_items);
        if (FAILED(hr))
            break;

        for (int i = 0; i < static_cast<int>(total_items); ++i)
        {
            IShellItem* p_item;
            p_items->GetItemAt(i, &p_item);
            if (SUCCEEDED(hr))
            {
                PWSTR path;
                hr = p_item->GetDisplayName(SIGDN_FILESYSPATH, &path);
                if (SUCCEEDED(hr))
                {
                    paths.push_back(path);
                    CoTaskMemFree(path);
                }
                p_item->Release();
            }
        }

        p_items->Release();
        are_all_operation_success = true;
    }

    if (p_file_open)
        p_file_open->Release();
    return are_all_operation_success;
}

DWORD ProcessID(const char* lpProcessName)
{
    char lpCurrentProcessName[255];

    PROCESSENTRY32 ProcList{};
    ProcList.dwSize = sizeof(ProcList);

    const HANDLE hProcList = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcList == INVALID_HANDLE_VALUE)
        return 0;

    if (!Process32First(hProcList, &ProcList))
        return 0;

    wcstombs_s(nullptr, lpCurrentProcessName, ProcList.szExeFile, 255);

    if (lstrcmpA(lpCurrentProcessName, lpProcessName) == 0)
        return ProcList.th32ProcessID;

    while (Process32Next(hProcList, &ProcList))
    {
        wcstombs_s(nullptr, lpCurrentProcessName, ProcList.szExeFile, 255);

        if (lstrcmpA(lpCurrentProcessName, lpProcessName) == 0)
            return ProcList.th32ProcessID;
    }

    return 0;
}

VOID StartApp(const wchar_t* lpCommandLine)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    CreateProcess(NULL,
        const_cast<LPWSTR>(lpCommandLine),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main()
{
	// Display welcome message
	std::cout << "Welcome to MaplePS!" << std::endl;
	std::cout << "MaplePS is a release of Maple Lite for osu!" << std::endl;
	std::cout << "WARNING: DO NOT USE THIS ON BANCHO. YOU WILL GET BANNED!" << std::endl;
	std::cout << "\nA few things are going to happen:" << std::endl;
	std::cout << "1. You will be asked to locate your osu! installation directory." << std::endl;
	std::cout << "2. MaplePS will then create a \"_staging\" file and update your \"osu!auth.dll\"." << std::endl;
	std::cout << "   - This will prevent osu! from updating and allow MaplePS to work correctly." << std::endl;
	std::cout << "   - Don't worry. You can undo this later." << std::endl;
	std::cout << "3. MaplePS will ask you to select a private server to connect to." << std::endl;
	std::cout << "4. MaplePS will then open osu! on the selected private server." << std::endl;
	std::cout << "5. Once osu! is open, press ENTER to inject MaplePS." << std::endl;

    // Begin prompt
	std::cout << "\nAre you ready to begin? (Y/N): ";
	char ready;
	std::cin >> ready;
	if (ready != 'Y' && ready != 'y') {
		std::cout << "Exiting..." << std::endl;
		return 0;
	}

	// Clear console
    system("cls");

    // Initialize COM library
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        std::cout << "Failed to initialize COM library.\n";
        return -1;
    }

    // Select osu! folder; default is %localappdata%\osu!
    std::cout << "Select your osu! folder." << std::endl;
    std::vector<std::wstring> paths;
    if (OpenFileDialog(paths, true, false, L"Select osu! Folder", L"%localappdata%\\osu!"))
    {
        if (paths.empty())
        {
            std::cout << "No folder was selected.\n";
            return -1;
        }
	}
	else
	{
		std::cout << "Failed to open file dialog.\n";
		return -1;
	}

	// Uninitialize COM library
    CoUninitialize();

	// Clear console
    system("cls");

    // Paths (osu!)
    std::wstring osuPath = paths[0];
    std::wstring osuExePath = osuPath + L"\\osu!.exe";
    std::wstring osuAuthPath = osuPath + L"\\osu!auth.dll";

	// Paths (MaplePS)
    TCHAR psPath[MAX_PATH + 1] = L"";
    GetModuleFileName(NULL, psPath, MAX_PATH);
    std::wstring psPathStr = psPath;
    psPathStr = psPathStr.substr(0, psPathStr.find_last_of(L"\\"));
    wcscpy_s(psPath, psPathStr.c_str());
    std::wstring psAuthPath = std::wstring(psPath) + L"\\osu!auth.dll";
    std::wstring dllPath = std::wstring(psPath) + L"\\Maple.dll";

	// Check if osu!.exe and osu!auth.dll exist
	std::cout << "Checking for files...";
    if (GetFileAttributes(osuExePath.c_str()) == INVALID_FILE_ATTRIBUTES || GetFileAttributes(osuAuthPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        std::cout << "FAILED" << std::endl;
        return -1;
    }
	std::cout << "DONE" << std::endl;

	// Create _staging file (must be read only)
	std::cout << "Creating _staging file...";
	std::ofstream(osuPath + L"\\_staging").close();
	SetFileAttributes((osuPath + L"\\_staging").c_str(), FILE_ATTRIBUTE_READONLY);
	std::cout << "DONE" << std::endl;

	// Replace osu!auth.dll with MaplePS version
	std::cout << "Replacing osu!auth.dll...";
	if (GetFileAttributes(psAuthPath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		std::cout << "FAILED (missing)" << std::endl;
		return -1;
	}

	DeleteFile(osuAuthPath.c_str());

    if (CopyFile(psAuthPath.c_str(), osuAuthPath.c_str(), FALSE) == NULL)
    {
		std::cout << "FAILED (move)" << std::endl;     
        return -1;
    }
	std::cout << "DONE" << std::endl;

    // Give user a selection of osu! private servers to open osu! on:
	// 1. Ripple (osu!.exe -devserver ripple.moe)
	// 2. Akatsuki (osu!.exe -devserver akatsuki.gg)
	// 3. Gatari (osu!.exe -devserver gatari.pw)
    // 4. Mamestagram (osu!.exe -devserver mamesosu.net)
    // 5. NoLimits (osu!.exe -devserver osunolimits.dev)
    // 6. Custom (prompt user for server url)
	std::cout << "\nSelect a private server to connect to:" << std::endl;
	std::cout << "1. Ripple (ripple.moe)" << std::endl;
	std::cout << "2. Akatsuki (akatsuki.gg)" << std::endl;
	std::cout << "3. Gatari (gatari.pw)" << std::endl;
	std::cout << "4. Mamestagram (mamesosu.net)" << std::endl;
	std::cout << "5. NoLimits (osunolimits.dev)" << std::endl;
	std::cout << "6. Kawata (kawata.pw)" << std::endl;
	std::cout << "7. Custom" << std::endl;
	std::cout << "Select a private server: ";
	int server;
	std::cin >> server;

	// Open osu! on selected private server
	std::wstring serverUrl;
	switch (server)
	{
	case 1:
		serverUrl = L"ripple.moe";
		break;
	case 2:
		serverUrl = L"akatsuki.gg";
		break;
	case 3:
		serverUrl = L"gatari.pw";
		break;
	case 4:
		serverUrl = L"mamesosu.net";
		break;
	case 5:
		serverUrl = L"osunolimits.dev";
		break;

    case 6:
		serverUrl = L"kawata.pw";
		break;
	case 7:
		std::wcout << "Enter a custom private server URL: ";
		std::wcin >> serverUrl;
		break;
	default:
		std::cout << "Invalid selection." << std::endl;
		return -1;
	}

	std::wcout << "Selected private server: " << serverUrl << std::endl;

	// Open osu! on selected private server
	std::wstring osuDevExePath = osuPath + L"\\osu!.exe -devserver " + serverUrl;
	StartApp(osuDevExePath.c_str());

	std::cout << "\nPlease wait for osu! to finish opening!" << std::endl;
	std::cout << "Press ENTER and MaplePS will inject." << std::endl;
	std::cin.ignore();
	std::cin.get();

    // Inject Maple.dll into osu!
	std::cout << "Injecting...";
    const DWORD dwProcessId = ProcessID("osu!.exe");
	if (!dwProcessId)
	{
		std::cout << "FAILED (osu! is not running)" << std::endl;
		return -1;
	}

	const HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (hProcess == NULL)
	{
		std::cout << "FAILED (open process)" << std::endl;
		return -1;
	}

    const LPVOID lpAddress = VirtualAllocEx(hProcess, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (lpAddress == NULL)
	{
		std::cout << "FAILED (allocate memory)" << std::endl;
		return -1;
	}

    const DWORD dwWriteResult = WriteProcessMemory(hProcess, lpAddress, dllPath.c_str(), MAX_PATH, nullptr);
	if (dwWriteResult == NULL)
	{
		std::cout << "FAILED (write memory)" << std::endl;
		return -1;
	}

	const HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, LPTHREAD_START_ROUTINE(LoadLibraryW), lpAddress, 0, nullptr);
	if (hThread == NULL)
	{
		std::cout << "FAILED (create thread)" << std::endl;
		return -1;
	}

	// Cleanup
    CloseHandle(hProcess);
    VirtualFreeEx(hProcess, LPVOID(lpAddress), 0, MEM_RELEASE);

	// Success
	std::cout << "DONE" << std::endl;

	// Keep the console open
    system("pause");
    return 0;
}