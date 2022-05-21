#ifndef CR_GRAPHICS_HPP
    #define CR_GRAPHICS_HPP

    #include "Types.hpp"

    namespace CR {

        namespace Gfx {

            struct Settings {
                int width;
                int height;
                bool resizable;
                bool fullscreen;
                void setParams(const std::vector<std::string> &params);
                void readSettings(const std::string &path);
            };

            void loadSettings(const std::vector<std::string> &params, const std::string &path);
            bool init();
            void end();
            void onEnd();
            void render();
            bool isRunning();

        }
        

    }


#endif