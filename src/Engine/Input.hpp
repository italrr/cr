#ifndef CR_INPUT_HPP
    #define CR_INPUT_HPP

    #include "Types.hpp"

    namespace CR {

        namespace Input {
            
            namespace Key {
                enum Key : int {
                    A,B,C,D,E,F,G,H,I,J,K,L,
                    M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,
                    ZERO,ONE,TWO,THREE,FOUR,FIVE,SIX,SEVEN,EIGHT,NINE,
                    ESCAPE,LCONTROL,LSHIFT,LALT,LSYSTEM,RCONTROL,RSHIFT,
                    RALT,RSYSTEM,MENU,LBRACKET,RBRACKET,SEMICOLON,COMMA,
                    PERIOD,QUOTE,SLASH,BACKSLASH,TILDE,EQUAL,DASH,SPACE,
                    ENTER,BACK,TAB,PAGEUP,PAGEDOWN,END,HOME,INSERT,DELETE,
                    ADD,SUBTRACT,MULTIPLY,DIVIDE,LEFT,RIGHT,UP,DOWN,PAUSE,
                    NUMPAD0,NUMPAD1,NUMPAD2,NUMPAD3,NUMPAD4,NUMPAD5,NUMPAD6,NUMPAD7,
                    NUMPAD8,NUMPAD9,
                    F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,F13,F14,F15
                };
            }

            namespace Button {
                enum Button : int {
                    LEFT = 201,
                    MIDDLE,
                    RIGHT
                };
            }
            
            namespace Trackpad {

            }

            namespace Touchscreen {
                
            }

            bool mouseCheck(int button);
            bool mousePressed(int button);
            bool mouseReleased(int button);

            bool keyboardCheck(int key);
            bool keyboardPressed(int key);
            bool keyboardReleased(int key);

            CR::Vec2<int> mousePosition();
        }        

    }


#endif