#ifndef CR_MAP_HPP
    #define CR_MAP_HPP

    #include "Engine/Types.hpp"
    #include "Engine/Graphics.hpp"
    #include "Engine/Texture.hpp"
    #include "Engine/Shader.hpp"

    namespace CR {

        namespace Map {
            
            namespace TileType {
                enum TileType : unsigned {
                    FLOOR = 0,
                    WALL,
                    DOOR,
                    EVENT
                };
            }

            namespace FaceType {
                enum FaceType : unsigned {
                    SOUTH = 0,
                    NORTH,
                    WEST,                    
                    EAST,
                    TOP,
                    BOTTOM
                };
            }

            struct AtlasSingle {
                CR::Vec2<float> coors;
                CR::Vec2<float> size;
                unsigned type;
                unsigned index;
            };

            struct TileSource {
                std::string name;
                CR::Gfx::MeshData md;
                AtlasSingle faces[6];
            };

            struct Tile {
                unsigned id;
                unsigned index;
                unsigned type;
                unsigned height; // Y
                CR::Vec2<int> position;
                CR::Map::TileSource *source;
                CR::Gfx::Transform transform;
            };

            struct UnitSize { // in pixels
                unsigned width;
                unsigned height;
                UnitSize(){
                    this->width = 50;
                    this->height = 50;
                }
                UnitSize(unsigned width, unsigned height){
                    this->width = width;
                    this->height = height;                    
                }                
            };

            struct Map {
                std::string title;
                std::string author;
                uint64 createdAt;

                Tile *tiles;
                std::vector<CR::Map::AtlasSingle> atlasSingles;
                std::unordered_map<std::string, std::shared_ptr<TileSource>> sources;
                std::shared_ptr<CR::Gfx::Texture> atlas; // Texture Atlas 
                std::shared_ptr<CR::Gfx::Shader> worldShader;

                CR::Vec3<float> origin;
                CR::Map::UnitSize usize;
                CR::Vec2<int> size; // units * unitsize
                unsigned totalUnits;

                unsigned width;
                unsigned height;

                Map();
                void build(const CR::Vec2<int> mapSize, const CR::Map::UnitSize &us);
                void render();
                void clear();
            };


        }

    }

#endif