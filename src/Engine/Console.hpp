#ifndef CR_CONSOLE_HPP
    #define CR_CONSOLE_HPP

    #include "Types.hpp"
    #include "Graphics.hpp"
    #include "Font.hpp"

    namespace CR {
        
        namespace Console {

            namespace UserLevel {
                enum UserLevel : unsigned {
                    PLAYER = 1,
                    MOD,
                    ADMIN
                };
            }

            namespace Source {
                enum UserLevel : unsigned {
                    LOCAL = 1,
                    REMOTE
                };
            }

            namespace VarType {
                enum VarType : unsigned {
                    Int = 0,
                    Float,
                    Bool,
                    Literal, // String
                    Unknown
                };
            }            


            std::shared_ptr<CR::Result> interpret(const std::string &input, int source, int userLevel);
        }

    }

#endif