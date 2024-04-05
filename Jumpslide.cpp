#include <iostream>
#include <Windows.h>
#include <random>
#include <thread>
#include <mutex>
#include <stdlib.h>

#pragma comment(lib, "winmm.lib")


namespace nt
{
	NTSYSAPI NTSTATUS	NTAPI NtDelayExecution(
		_In_ BOOLEAN Alertable,
		_In_opt_ PLARGE_INTEGER DelayInterval);

	inline decltype(&NtDelayExecution) pNtDelayExecution{};

	// custom sleep function to fix broken randomization
	//
	__forceinline static void sleep(std::uint64_t delay_interval)
	{
		// lambda for getting NtDelayExecution pointer
		static auto grab_nt_delay_execution = [&]() -> bool
			{
				pNtDelayExecution = reinterpret_cast<decltype(pNtDelayExecution)>(
					GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtDelayExecution"));

				// std::printf("NtDelayExecution: 0x%p\n", pNtDelayExecution);
				return true;
			};

		static auto _ = grab_nt_delay_execution();

		// set our periodic timer resolution to 1ms
		timeBeginPeriod(1);

		LARGE_INTEGER _delay_interval{};
		_delay_interval.QuadPart = -static_cast<LONGLONG>(delay_interval * static_cast<std::uint64_t>(10'000));

		pNtDelayExecution(false, &_delay_interval);

		// reset periodic timer resolution
		timeEndPeriod(1);
	}
}


void PressSpace()
{
	// Create a generic keyboard event structure
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0x39; // hardware scan code for key

	// ip.ki.wVk = VkKeyScanA(' ');
	ip.ki.dwFlags = KEYEVENTF_SCANCODE; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	nt::sleep(1);

	// Release the "H" key
	ip.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));

}

void PressC()
{
	// Create a generic keyboard event structure
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0x2E; // hardware scan code for key

	// ip.ki.wVk = 0x43;
	ip.ki.dwFlags = KEYEVENTF_SCANCODE; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	nt::sleep(1);
	// Release the "H" key
	ip.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));

}


void JumpSlide()
{
	while (true)
	{
		bool jumped = false;
		nt::sleep(1);
		while (true)
		{
			if (GetAsyncKeyState(VK_XBUTTON1))
			{
				if (jumped == false)
				{
					PressSpace();
					nt::sleep(5);
					PressC();
					jumped = true;
				}
			}
			if ((GetAsyncKeyState(VK_XBUTTON1) & 0x8000) == 0)
			{
				jumped = false;
			}
		}
	}
}

int main()
{
	JumpSlide();
	return 0;
}