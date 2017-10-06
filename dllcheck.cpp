#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <stdio.h>
#include <psapi.h>

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1

const wchar_t *GetWC(const char *c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

int PrintModules(DWORD processID, char *searchDll)
{
	HMODULE hMods[1024];
	HANDLE hProcess;
	DWORD cbNeeded;
	unsigned int i;

	// Get a handle to the process.
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID);

	if (NULL == hProcess) {
		return 0;
	}

	// Get a list of all the modules in this process.

	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_PATH];

			// Get the full path to the module's file.

			if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
				sizeof(szModName) / sizeof(TCHAR)))
			{
				// Print the module name and handle value.
				const wchar_t *wsdll = GetWC(searchDll);
				if (wcsstr(szModName, wsdll) != NULL) {
					printf("Process ID: %u\n", processID);
					printf("Search DLL: %s\n", searchDll);
					_tprintf(TEXT("\t%s (0x%08X)\n"), szModName, hMods[i]);
					delete[] wsdll;
					return 1;
				}
				delete[] wsdll;
			}
		}
	}

	// Release the handle to the process.
	CloseHandle(hProcess);
	return 0;
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		return 1;
	}

	DWORD aProcesses[1024];
	DWORD cbNeeded;
	DWORD cProcesses;
	unsigned int i;

	// Get the list of process identifiers.
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		return 0;

	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);

	// Print the names of the modules for each process.

	for (i = 0; i < cProcesses; i++)
	{
		int ret = PrintModules(aProcesses[i], argv[1]);
		if (ret != 0) {
			return ret;
		}
	}
	return 0;
}