#ifndef CR_BITMAP_HPP
    #define CR_BITMAP_HPP

    #include "Types.hpp"


    namespace CR {
        namespace Gfx {

            struct Bitmap {
                std::vector<CR::Color> pixels;
                unsigned width;
                unsigned height;
                unsigned format;
                unsigned channels;
                
                Bitmap sub(const CR::Rect<unsigned> &box);
                Bitmap sub(unsigned x, unsigned y, unsigned width, unsigned height);
                
                void paste(const Bitmap &src, const CR::Vec2<unsigned> &vec);
                void paste(const Bitmap &src, unsigned x, unsigned y);
                    
                std::vector<uint8> getFlatArray() const;
                Bitmap copy();
                CR::Rect<unsigned> autocrop();
                Bitmap &build(const CR::Color &p, unsigned format, unsigned width, unsigned height);
                std::vector<std::vector<CR::Rect<unsigned>>> findBoxes();        
                bool load(const std::string &path);
                bool write(const std::string &path);
            };

        }
    }

#endif