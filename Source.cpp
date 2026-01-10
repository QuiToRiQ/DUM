#include <Windows.h>
#include <iostream>
#include <atomic>
#include <thread>

class MyMouse
{
public:
    static void MoveMouse(int dx, int dy)
    {
        if (dx == 0 && dy == 0) return;

        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dx = dx;
        input.mi.dy = dy;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;

        SendInput(1, &input, sizeof(INPUT));
    }

    static void LeftDown() {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));
    }

    static void LeftUp() {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    static void RightDown() {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        SendInput(1, &input, sizeof(INPUT));
    }

    static void RightUp() {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        SendInput(1, &input, sizeof(INPUT));
    }
};

// Key states
std::atomic<bool> E_pressed(false); // left
std::atomic<bool> S_pressed(false); // down
std::atomic<bool> D_pressed(false); // right
std::atomic<bool> F_pressed(false); // left
std::atomic<bool> W_pressed(false); // left click
std::atomic<bool> R_pressed(false); // right click
std::atomic<bool> space_pressed(false); // hold for active

HHOOK keyboardHook;

// Hook callback
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        bool keyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        bool keyUp = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

        // Spacebar activates the functionality
        if (kb->vkCode == VK_SPACE)
        {
            space_pressed = keyDown;
            // Suppress space for other apps while holding
            if (keyDown) return 1;
            if (keyUp) return 1;
        }

        // Only handle keys when space is held
        if (space_pressed)
        {
            switch (kb->vkCode)
            {
            case 'E': E_pressed = keyDown; return 1; // left
            case 'S': S_pressed = keyDown; return 1; // down
            case 'D': D_pressed = keyDown; return 1; // right
            case 'F': F_pressed = keyDown; return 1; // up

            case 'W': W_pressed = keyDown; return 1; // left click
            case 'R': R_pressed = keyDown; return 1; // right click
            }
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

// Input loop
void InputLoop()
{
    const int speed = 5; // pixels per iteration
    bool leftHeld = false;
    bool rightHeld = false;

    while (true)
    {
        if (space_pressed)
        {
            int dx = 0, dy = 0;
            if (E_pressed) dy -= speed; // up
            if (D_pressed) dy += speed; // down
            if (F_pressed) dx += speed; // right
            if (S_pressed) dx -= speed; // left

            MyMouse::MoveMouse(dx, dy);

            // Handle left click hold
            if (W_pressed && !leftHeld) { MyMouse::LeftDown(); leftHeld = true; }
            if (!W_pressed && leftHeld) { MyMouse::LeftUp(); leftHeld = false; }

            // Handle right click hold
            if (R_pressed && !rightHeld) { MyMouse::RightDown(); rightHeld = true; }
            if (!R_pressed && rightHeld) { MyMouse::RightUp(); rightHeld = false; }
        }
        else
        {
            // Reset held states when space is released
            if (leftHeld) { MyMouse::LeftUp(); leftHeld = false; }
            if (rightHeld) { MyMouse::RightUp(); rightHeld = false; }
        }

        Sleep(10); // smooth loop
    }
}

int main()
{
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (!keyboardHook)
    {
        std::cout << "Failed to install hook!\n";
        return 1;
    }

    std::cout << "Hold Spacebar to activate mouse keys.\n";
    std::cout << "E/S/D/F = Move, W = Left Click, R = Right Click\n";

    std::thread loopThread(InputLoop);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    loopThread.join();
    UnhookWindowsHookEx(keyboardHook);
    return 0;
}
