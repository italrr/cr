#ifndef CR_FLEXIE_HPP
    #define CR_FLEXIE_HPP

    #include "../Types.hpp"
    #include "../Font.hpp"

    namespace CR {
        namespace UI {
            
            struct Dimension {
                unsigned width;
                unsigned height;
                std::string wLogical;
                std::string hLogical;
                bool fillup;
                CR::Rect<int> padding;
                CR::Rect<int> margin;
            };

            struct Position {
                unsigned x;
                unsigned y;
            };

            struct StyleSheet {
                CR::Gfx::Font font;

                CR::Color bgColor;
                CR::Color textColor;
                CR::Color borderColor;

            };

            struct Base {
                std::string name;
                UI::Position pos;
                UI::Dimension dims;
                unsigned id;
                unsigned display;
                unsigned type;
                unsigned depth;
                unsigned layout;
                bool solid;
                bool interactive;
                float flex;

                UI::Base *head;
                std::vector<std::shared_ptr<UI::Base>> children;

                

                virtual void create(){};
                virtual void draw(){};
                virtual void step(){};
                virtual void destroy(){};
                void clear(); // removes alll children

            };


            struct Window : Base {
                std::string title;

                void move(const CR::Vec2<unsigned> &nPos);
                void close();
            };


            std::shared_ptr<CR::UI::Base> build(const std::string &path);

        }
    }


#endif