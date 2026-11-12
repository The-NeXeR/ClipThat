#include "paste.h"
#include <stdbool.h>

void pasteSimulate(HWND window) {
	if (window == NULL || !IsWindow(window)) return;
	DWORD foreThread = GetWindowThreadProcessId(window, NULL);
	DWORD myThread = GetCurrentThreadId();

	if (foreThread == 0) { return; }
	bool attached = false;
	if (foreThread != myThread) {
		attached = AttachThreadInput(myThread, foreThread, TRUE);
		if (!attached) {
			return;
		}
	}
	SetForegroundWindow(window);
	Sleep(15);

	INPUT input[4] = { 0 };
	input[0].type = INPUT_KEYBOARD; input[0].ki.wVk = VK_CONTROL;
	input[1].type = INPUT_KEYBOARD; input[1].ki.wVk = 'V';
	input[2].type = INPUT_KEYBOARD; input[2].ki.wVk = 'V';
	input[2].ki.dwFlags = KEYEVENTF_KEYUP;
	input[3].type = INPUT_KEYBOARD; input[3].ki.wVk = VK_CONTROL;
	input[3].ki.dwFlags = KEYEVENTF_KEYUP;

	UINT sent = SendInput(4, input, sizeof(INPUT));
	if (sent != 4) {
		OutputDebugStringW(L"pasteSimulate: SendInput did not queue all events.\n");
	}

	if (attached) {
		if (!AttachThreadInput(myThread, foreThread, FALSE)) {
			OutputDebugStringW(L"pasteSimulate: failed to detach thread input.\n");
		}
	}
}