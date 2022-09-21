#ifndef CR_GAME_MAP_HPP
    #define CR_GAME_MAP_HPP

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
                    EVENT,
                    EMPTY,
                    ANY,
                    OUTBOUNDS
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
                CR::Vec3<float> rposition;
                CR::Map::TileSource *source;
                CR::Gfx::Transform *transform;
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


                std::vector<CR::Gfx::Transform*> batchTrans;
                std::vector<CR::Gfx::MeshData*> batchMesh;

                Tile start;

                void rebuildBatch();


                CR::Vec3<float> origin;
                float usize;
                CR::Vec2<float> tsize;  // units * unitsize
                CR::Vec2<unsigned> size;
                unsigned bpScale;
                unsigned totalUnits;

                unsigned width;
                unsigned height;

                Map();
                void build(const CR::Vec2<int> mapSize, unsigned bpScale, float us);
                void render();
                void clear();
            };


        }

    }

#endif