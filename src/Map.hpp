#ifndef CR_MAP_HPP
    #define CR_MAP_HPP

    #include "Engine/Types.hpp"

    namespace CR {

        namespace Map {

            static const unsigned WALL_NUMBER = 4;
            static const unsigned CEIL_NUMBER = 1;
            
            namespace TileType {
                enum TileType : unsigned {
                    FLOOR = 0,
                    WALL,
                    DOOR,
                    EVENT
                };
            }

            namespace WallType {
                enum WallType : unsigned {
                    NORTH = 0,
                    SOUTH,
                    EAST,
                    WEST
                };
            }

            struct Tile {
                unsigned id;
                unsigned index;
                unsigned type;
                CR::Vec2<int> position;
                
                // Texture
                unsigned texWalls[4];
                unsigned texCeil;
            };

            struct UnitSize {
                unsigned width;
                unsigned height;
            };

            struct Map {
                std::string title;
                std::string author;
                uint64 createdAt;
                
                CR::Map::UnitSize usize;
                CR::Vec2<int> size; // units * unitsize
                unsigned width;
                unsigned height;
            };


        }

    }

#endif