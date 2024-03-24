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
                CR::Vec2<unsigned> getAbsoluteSize(){
                    unsigned w = this->size.x + this->padding.x + this->padding.w + this->margin.x + this->margin.w;
                    unsigned h = this->size.y + this->padding.y + this->padding.h + this->margin.y + this->margin.h;
                    return CR::Vec2<unsigned>(w, h);
                };
                void setAccommodatedSize(const CR::Vec2<unsigned> &size){
                    this->size.x = size.x - ((this->margin.x + this->margin.w) + (this->padding.x + this->padding.w));
                    this->size.y = size.y - ((this->margin.y + this->margin.h) + (this->padding.y + this->padding.h));
                }
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
                virtual void render(CR::Gfx::RenderLayer *layer){};

                // accommodate resizes the object to fit the provided width and height
                // this shoud only be used when the type is able to `fillup` (dims.fillup == true),
                // for example, a panel or a button
                virtual bool accommodate(const CR::Vec2<unsigned> &size){
                    if(!this->dims.fillup) return false;
                    this->dims.setAccommodatedSize(size);
                    return true;
                };
                // typical size doesn't take into account margin and padding, that's why we
                // use getAbsoluteSize
                virtual CR::Vec2<unsigned> computeSize(){
                    return this->dims.getAbsoluteSize();
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
                void step();
                void setSize(const CR::Vec2<unsigned> &nSize);
                void setTitle(const std::string &title);
                void render(CR::Gfx::RenderLayer *layer);
            };


            std::shared_ptr<CR::UI::Base> build(const std::string &path);

        }
    }


#endif