#include <map>
#include <GLFW/glfw3.h>
#include "Input.hpp"

enum _STATE : int {
    RELEASE,
    PRESS
};

static const int MAX_KEYS = 101;
static const int keyTranslateTable[MAX_KEYS] = {
    65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 
    77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57,
    256, 341, 340, 342, 343, 345, 344,
    346, 347, 348, 91, 93, 59, 44,
    46, 44, 47, 92, 96, 336, 45, 32,
    257, 259, 258, 266, 267, 269, 268, 260, 261,
    334, 333, 332, 331, 263, 262, 265, 264, 284,
    320, 321, 322, 323, 324, 325, 326, 327, 328, 329,
    290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300,
    301, 302, 303, 304
};
static std::map<int, int> keyRawState;
static std::map<int, int> KeyStatePressed;
static std::map<int, int> KeyStateReleased;

static CR::Vec2<int> cursorPosition;
static const int MAX_BUTTONS = 3;
static const int buttonTranslateTable[MAX_BUTTONS] = {
    0, 2, 1
};
static std::map<int, int> buttonRawState;
static std::map<int, int> buttonStatePressed;
static std::map<int, int> buttonStateReleased;

static void keycallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    auto _k = keyRawState.find(key);
    if(_k == keyRawState.end()){
        return;
    }
    switch(action){
        case GLFW_PRESS: {
            keyRawState[key] = _STATE::PRESS;
        } break;
        case GLFW_RELEASE: {
            keyRawState[key] = _STATE::RELEASE;
        } break;        
    }
    if(KeyStatePressed[key] == _STATE::PRESS && action == GLFW_RELEASE){
        KeyStatePressed[key] =_STATE::RELEASE;
    }
    if(KeyStateReleased[key] == _STATE::RELEASE && action == GLFW_PRESS){
        KeyStateReleased[key] =_STATE::PRESS;
    }    
}

static void buttonCallback(GLFWwindow* window, int button, int action, int mods){
    auto _k = buttonRawState.find(button);
    if(_k == buttonRawState.end()){
        return;
    }
    switch(action){
        case GLFW_PRESS: {
            buttonRawState[button] = _STATE::PRESS;
        } break;
        case GLFW_RELEASE: {
            buttonRawState[button] = _STATE::RELEASE;
        } break;        
    }
    if(buttonStatePressed[button] == _STATE::PRESS && action == GLFW_RELEASE){
        buttonStatePressed[button] =_STATE::RELEASE;
    }
    if(buttonStateReleased[button] == _STATE::RELEASE && action == GLFW_PRESS){
        buttonStateReleased[button] =_STATE::PRESS;
    }  
}

static void cursorPositionCallBack(GLFWwindow* window, double xpos, double ypos){
    cursorPosition.set(xpos, ypos);
}

bool CR::Input::mouseCheck(int button){
    return button >= 0 && button <= MAX_BUTTONS-1 && buttonRawState[buttonTranslateTable[button]] == _STATE::PRESS;
}

bool CR::Input::mousePressed(int button){
	if (button < 0 && button > MAX_BUTTONS-1){
		return false;
	}
    int tbutton = buttonTranslateTable[button];
    if(buttonRawState[tbutton] == _STATE::PRESS && buttonStatePressed[tbutton] == _STATE::RELEASE){
        buttonStatePressed[tbutton] = _STATE::PRESS;
        return true;
    }
	return false;
}

bool CR::Input::mouseReleased(int button){
	if (button < 0 && button > MAX_BUTTONS-1){
		return false;
	}
    int tbutton = buttonTranslateTable[button];
    if(buttonRawState[tbutton] == _STATE::RELEASE && buttonStateReleased[tbutton] == _STATE::PRESS){
        buttonStateReleased[tbutton] = _STATE::RELEASE;
        return true;
    }
	return false;
}

bool CR::Input::keyboardCheck(int key){
    return key >= 0 && key <= MAX_KEYS-1 && keyRawState[keyTranslateTable[key]] == _STATE::PRESS;
}

bool CR::Input::keyboardPressed(int key){
	if (key < 0 && key > MAX_KEYS-1){
		return false;
	}
    int tkey = keyTranslateTable[key];
    if(keyRawState[tkey] == _STATE::PRESS && KeyStatePressed[tkey] == _STATE::RELEASE){
        KeyStatePressed[tkey] = _STATE::PRESS;
        return true;
    }
	return false;
}

bool CR::Input::keyboardReleased(int key){
	if (key < 0 && key > MAX_KEYS-1){
		return false;
	}
    int tkey = keyTranslateTable[key];
    if(keyRawState[tkey] == _STATE::RELEASE && KeyStateReleased[tkey] == _STATE::PRESS){
        KeyStateReleased[tkey] = _STATE::RELEASE;
        return true;
    }
	return false;
}

CR::Vec2<int> CR::Input::mousePosition(){
    return cursorPosition;
}

void __CR_init_input(GLFWwindow* win){
    auto *window = static_cast<GLFWwindow*>(win);
    for(int i = 0; i < MAX_KEYS; ++i){
        keyRawState[keyTranslateTable[i]] = _STATE::RELEASE;
        KeyStatePressed[keyTranslateTable[i]] = _STATE::RELEASE;
        KeyStateReleased[keyTranslateTable[i]] = _STATE::RELEASE;        
    }
    for(int i = 0; i < MAX_BUTTONS; ++i){
        buttonRawState[buttonTranslateTable[i]] = _STATE::RELEASE;
        buttonStatePressed[buttonTranslateTable[i]] = _STATE::RELEASE;
        buttonStateReleased[buttonTranslateTable[i]] = _STATE::RELEASE;        
    }    
    glfwSetKeyCallback(window, keycallback);
    glfwSetMouseButtonCallback(window, buttonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallBack);
}

void __CR_end_input(){

}