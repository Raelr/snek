#pragma once

#include "../Utils/Math.h"
#include "../Window/Window.h"
#include <unordered_set>

#include <map>

#define KEY_W GLFW_KEY_W
#define KEY_A GLFW_KEY_A
#define KEY_S GLFW_KEY_S
#define KEY_D GLFW_KEY_D
#define KEY_LEFT_SUPER GLFW_KEY_LEFT_SUPER
#define KEY_Q GLFW_KEY_Q

#define MOUSE_KEY_RIGHT GLFW_MOUSE_BUTTON_RIGHT

/** TODO: This could be made a lot more efficient.
 * The ideal scenario here would be to read key events from GLFW as the events occur.
 * We create an object which stores our keyboard state each frame. At the start of each frame we compare the previous
 * to the current frame adjust state accordingly.
 *
 * We could then store multiple unordered sets which store the state for the frame. I.e: a map for storing release or
 * justPressed events. We could even have an object per key which holds the requisite state.
 */

class Input 
{
    public:
        struct MouseCoordinates
        {
            double x;
            double y;
        };
        static void SetWindowPointer(SnekVk::Window* window);
        static bool IsKeyDown(int key);
        static bool IsKeyJustPressed(int key);
        static bool IsKeyJustReleased(int key);
        static bool IsMouseButtonDown(int button);
        static bool IsMouseButtonJustPressed(int button);
        static bool IsMouseButtonJustReleased(int button);
        static void FlushKeys();

        static const MouseCoordinates& GetCursorPosition();
        static MouseCoordinates GetNormalisedMousePosition();
    private:
        static SnekVk::Window* windowPtr;
        static bool movedLastFrame;
        static MouseCoordinates currentMouseCoordinates;
        static std::map<int, int> keyMap;
        static std::unordered_set<int> clearedKeys;

        static void GetCursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

};
