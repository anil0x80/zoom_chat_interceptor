#include <Windows.h>
#include <detours.h>
#include <thread>

#include "helper.h"

const std::wstring search_message = L"Hello!";

static bool alarm_called = false;
void alarm_thread()
{
    helper::unmute_windows();
	
    for (int i = 0; i < 4 * 5; i++) // beep for 5 seconds straight, 20 beeps lasting 250ms each. 
        Beep(1000, 250);

    alarm_called = false; // allow another alarm to be called
}

using function_type = bool(__fastcall*)(void* ecx, void* edx, unsigned char* p_cstring);
function_type original_function = nullptr;

bool __fastcall hooked_function(void* ecx, void* edx, unsigned char* p_cstring)
{
    auto* raw_text_message = *(wchar_t**)(p_cstring + 4);

	if (raw_text_message && !alarm_called) // only process messages if alarm is not called, so prevent multiple alarms from triggering
	{
        auto text_message = std::wstring(raw_text_message); // copy string so we can safely do operations on it without modifying original string
		
        /* process text message */
        auto alarm_condition = text_message.find(search_message) != std::wstring::npos;
		
        if (alarm_condition)
        {
            alarm_called = true;
            std::thread t(&alarm_thread);
            t.detach();
        }
	}

	/* exit hook */
    return original_function(ecx, edx, p_cstring);
}

/*
 * this project hooks a magic function on zVideoUI.dll on Zoom.exe(meeting instance),
 * which is called when a new chat message is sent. 
 * I have yet to test the stability of this method, but first tests are looking good.
 * definitive stable method would be hooking network functions.
 */

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hinstDLL);
    	
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        original_function = (function_type)helper::find_pattern("zVideoUI.dll", "55 8B EC 56 57 8B F9 FF 15 ? ? ? ? 85 C0 74 3A");
        DetourAttach(&(PVOID&)original_function, &hooked_function);
        if (DetourTransactionCommit() == NO_ERROR)
            OutputDebugStringA("[ChatInterceptor] detour successful!");
        else
            OutputDebugStringA("[ChatInterceptor] detour failed!");

        break;

    case DLL_PROCESS_DETACH:
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)original_function, &hooked_function);

        break;
    }
	
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}