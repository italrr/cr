#ifndef CR_FLEXIE_HPP
    #define CR_FLEXIE_HPP

    #include "../Types.hpp"
    #include "../Graphics.hpp"
    #include "../Font.hpp"

    namespace CR {
        namespace UI {

            namespace ElementType {
                enum ElementType : unsigned {
                    WINDOW,
                    LABEL,
                    BUTTON,
                    TEXTBOX,
                    SINGLE_SELECTOR,
                    MULTIPLE_SELECTOR,
                    RANGE,
                    IMAGE,
                    PANEL
                };
            }
            
            struct Base;
            struct Layout {
                bool xorient;
                bool yorient;
                CR::Vec2<unsigned> size;
                Layout(){
                    yorient = true;
                    xorient = true;
                }
                virtual void recalculate(Base &head){

                }
            };

            namespace LayoutType {
                struct VBox : public Layout {
                    VBox(){
                        yorient = true;
                        xorient = false;
                    }
                    void recalculate(Base *head);
                };
                struct HBox : public Layout {
                    HBox(){
                        yorient = false;
                        xorient = true;                
                    }
                    void recalculate(Base *head);
                };       
                struct Inline : public Layout {
                    Inline(){
                        yorient = true;
                        xorient = true;                  
                    }
                    void recalculate(Base *head);
                };
            }


            struct Dimension {
                std::string wLogical;
                std::string hLogical;
                bool fillup;
                CR::Vec2<unsigned> size;
                CR::Rect<unsigned> padding;
                CR::Rect<unsigned> margin;
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
                virtual void rerender(){};
                virtual void render(){};
                virtual void accodomate(unsigned w, unsigned h){
                    
                };
                virtual CR::Vec2<unsigned> computeSize(){

                }                
                void clear(); // removes all children

            };

            struct Panel : Base {
                  
            };

            struct Window : Panel {
                std::string title;
                Window();
                void move(const CR::Vec2<unsigned> &nPos);
                void close();
                void setSize(const CR::Vec2<unsigned> &nSize);
                void setTitle(const std::string &title);
                void render(CR::Gfx::RenderLayer *layer);
            };


            std::shared_ptr<CR::UI::Base> build(const std::string &path);

        }
    }


#endif