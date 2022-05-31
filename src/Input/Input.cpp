#include "Input.h"
#include <iostream>
    
SnekVk::Window* Input::windowPtr = nullptr;
Input::MouseCoordinates Input::currentMouseCoordinates;
bool Input::movedLastFrame{false};

std::map<int, int> Input::keyMap;
std::unordered_set<int> Input::clearedKeys;

void Input::SetWindowPointer(SnekVk::Window* window)
{
    // TODO: Fail if inputted pointer is nullptr
    windowPtr = window;
    glfwSetCursorPosCallback(window->GetGlfwWindow(), GetCursorPositionCallback);
}
bool Input::IsKeyDown(int key)
{
    bool hasKey = (glfwGetKey(windowPtr->GetGlfwWindow(), key) == GLFW_PRESS);

    if (hasKey) keyMap[key] +=1;
    else clearedKeys.emplace(key);

    return hasKey;
}

bool Input::IsKeyJustPressed(int key)
{
    bool hasKey = glfwGetKey(windowPtr->GetGlfwWindow(), key) == GLFW_PRESS;
    bool isKeyPresent = (keyMap.find(key) != keyMap.end());

    bool isJustPressed = false;

    if (hasKey && isKeyPresent) {
        isJustPressed = (keyMap[key] == 1);
        keyMap[key] += 1;
    }
    else if (!hasKey) clearedKeys.emplace(key);

    return isJustPressed;
}

const Input::MouseCoordinates& Input::GetCursorPosition()
{
    return currentMouseCoordinates;
}

Input::MouseCoordinates Input::GetNormalisedMousePosition()
{
    return {
        glm::clamp<float>(Utils::Math::Normalise(currentMouseCoordinates.x, 0, windowPtr->GetWidth()), -1.f, 1.f),
        glm::clamp<float>(Utils::Math::Normalise(currentMouseCoordinates.y, 0, windowPtr->GetHeight()), -1.f, 1.f)
    };
}

void Input::GetCursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    currentMouseCoordinates.x = static_cast<float>(xpos);
    currentMouseCoordinates.y = static_cast<float>(ypos);
}

bool Input::IsKeyJustReleased(int key)
{
    bool hasKey = glfwGetKey(windowPtr->GetGlfwWindow(), key) == GLFW_PRESS;
    bool isReleased = false;

    if (!hasKey)
    {
        if (keyMap[key] != 0)
        {
            isReleased = true;
            clearedKeys.emplace(isReleased);
        }
    }
    return isReleased;
}

void Input::FlushKeys()
{
    for(auto& key : clearedKeys)
    {
        keyMap[key] = 0;
    }
    clearedKeys.clear();
}

bool Input::IsMouseButtonDown(int button)
{
    bool hasButton = (glfwGetMouseButton(windowPtr->GetGlfwWindow(), button) == GLFW_PRESS);

    if (hasButton) keyMap[button] +=1;
    else clearedKeys.emplace(button);

    return hasButton;
}

bool Input::IsMouseButtonJustPressed(int button)
{
    bool hasButton = glfwGetMouseButton(windowPtr->GetGlfwWindow(), button) == GLFW_PRESS;
    bool isButtonPresent = (keyMap.find(button) != keyMap.end());

    bool isJustPressed = false;

    if (hasButton && isButtonPresent) {
        isJustPressed = (keyMap[button] == 1);
        keyMap[button] += 1;
    }
    else if (!hasButton) clearedKeys.emplace(button);

    return isJustPressed;
}

bool Input::IsMouseButtonJustReleased(int button)
{
    bool hasButton = glfwGetMouseButton(windowPtr->GetGlfwWindow(), button) == GLFW_PRESS;
    bool isReleased = false;

    if (!hasButton)
    {
        if (keyMap[button] != 0)
        {
            isReleased = true;
            clearedKeys.emplace(isReleased);
        }
    }
    return isReleased;
}


        