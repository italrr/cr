#include <algorithm>
#include <random>
#include <stack>

#include "Engine/Log.hpp"
#include "Engine/Tools.hpp"
#include "Map.hpp"

static std::shared_ptr<CR::Gfx::RenderLayer> game;

namespace MeshType {
    enum MeshType : unsigned {
        UNIT_CUBE = 0,
        TALL_CUBE,
        PLANE,
    };
}

namespace CellType {
    enum CellType : unsigned {
        EMPTY = 0,
        WALL,
        PATH
    };
}

struct Cell {
    int x;
    int y;
    unsigned i;
    unsigned type;
    Cell(){
        this->type = CellType::WALL;
    }
    Cell(int x, int y){
        this->x = x;
        this->y = y;
    }
};

struct CellMap {
    std::vector<Cell> cells;
    unsigned width;
    unsigned height;
    unsigned total;
};

static CR::Gfx::MeshData buildMesh(float cubeScale, const std::vector<CR::Map::AtlasSingle> &faces, unsigned type){

    auto FTS    = CR::Map::FaceType::SOUTH;    
    auto FTN    = CR::Map::FaceType::NORTH;    
    auto FTW    = CR::Map::FaceType::WEST;    
    auto FTE    = CR::Map::FaceType::EAST;    
    auto FTT    = CR::Map::FaceType::TOP;    
    auto FTB    = CR::Map::FaceType::BOTTOM;    

    auto mesh = CR::Gfx::MeshData();

    switch(type){
        case MeshType::PLANE: {
            mesh = CR::Gfx::createMesh({
                // POSITION                             // TEXTURE
                // TOP
                -cubeScale,  cubeScale, -cubeScale,
                cubeScale,  cubeScale, -cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     
                
                cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
            },{
                faces[FTB].coors.x,   faces[FTB].size.y,
                faces[FTB].size.x,    faces[FTB].size.y,
                faces[FTB].size.x,    faces[FTB].coors.y,
                faces[FTB].size.x,    faces[FTB].coors.y,
                faces[FTB].coors.x,   faces[FTB].coors.y,
                faces[FTB].coors.x,   faces[FTB].size.y                
            });
            mesh.vertn = 6;
        } break;
        case MeshType::UNIT_CUBE: {

            mesh = CR::Gfx::createMesh({ 
                // POSITION                             // TEXTURE

                // SOUTH
                -cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale, -cubeScale,     
                cubeScale,  cubeScale, -cubeScale,     

                cubeScale,  cubeScale, -cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
                -cubeScale, -cubeScale, -cubeScale,     

                //  NORTH
                -cubeScale, -cubeScale,  cubeScale,     
                cubeScale, -cubeScale,  cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     
                
                cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale, -cubeScale,  cubeScale,     

                // WEST
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
                -cubeScale, -cubeScale, -cubeScale,     

                -cubeScale, -cubeScale, -cubeScale,     
                -cubeScale, -cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     

                // EAST
                cubeScale,  cubeScale,  cubeScale,    
                cubeScale,  cubeScale, -cubeScale,     
                cubeScale, -cubeScale, -cubeScale,     

                cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale,  cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     

                // TOP
                -cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale,  cubeScale,     

                cubeScale, -cubeScale,  cubeScale,     
                -cubeScale, -cubeScale,  cubeScale,     
                -cubeScale, -cubeScale, -cubeScale,     

                // BOTTOM
                -cubeScale,  cubeScale, -cubeScale,     
                cubeScale,  cubeScale, -cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     
                
                cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
            }, {
faces[FTS].coors.x,   faces[FTS].coors.y,
faces[FTS].size.x,    faces[FTS].coors.y,
faces[FTS].size.x,    faces[FTS].size.y,
faces[FTS].size.x,    faces[FTS].size.y,
faces[FTS].coors.x,   faces[FTS].size.y,
faces[FTS].coors.x,   faces[FTS].coors.y,
faces[FTN].coors.x,   faces[FTN].coors.y,
faces[FTN].size.x,    faces[FTN].coors.y,
faces[FTN].size.x,    faces[FTN].size.y,
faces[FTN].size.x,    faces[FTN].size.y,
faces[FTN].coors.x,   faces[FTN].size.y,
faces[FTN].coors.x,   faces[FTN].coors.y,
faces[FTW].size.x,    faces[FTW].size.y,
faces[FTW].coors.x,   faces[FTW].size.y,
faces[FTW].coors.x,   faces[FTW].coors.y,
faces[FTW].coors.x,   faces[FTW].coors.y,
faces[FTW].size.x,    faces[FTW].coors.y,
faces[FTW].size.x,    faces[FTW].size.y,
faces[FTE].size.x,    faces[FTE].size.y,
faces[FTE].coors.x,   faces[FTE].size.y,
faces[FTE].coors.x,   faces[FTE].coors.y,
faces[FTE].coors.x,   faces[FTE].coors.y,
faces[FTE].size.x,    faces[FTE].coors.y,
faces[FTE].size.x,    faces[FTE].size.y,
faces[FTT].size.x,    faces[FTT].size.y,
faces[FTT].coors.x,   faces[FTT].size.y,
faces[FTT].coors.x,   faces[FTT].coors.y,
faces[FTT].coors.x,   faces[FTT].coors.y,
faces[FTT].size.x,    faces[FTT].coors.y,
faces[FTT].size.x,    faces[FTT].size.y,
faces[FTB].coors.x,   faces[FTB].size.y,
faces[FTB].size.x,    faces[FTB].size.y,
faces[FTB].size.x,    faces[FTB].coors.y,
faces[FTB].size.x,    faces[FTB].coors.y,
faces[FTB].coors.x,   faces[FTB].coors.y,
faces[FTB].coors.x,   faces[FTB].size.y
            });

        } break;        
        case MeshType::TALL_CUBE: {
            CR::Vec3<float> offset = CR::Vec3<float>(0, -cubeScale * 2, 0);

            mesh = CR::Gfx::createMesh({ 
                // POSITION                             // TEXTURE

                // SOUTH
                -cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale, -cubeScale,     
                cubeScale,  cubeScale, -cubeScale,     

                cubeScale,  cubeScale, -cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
                -cubeScale, -cubeScale, -cubeScale,     

                //  NORTH
                -cubeScale, -cubeScale,  cubeScale,     
                cubeScale, -cubeScale,  cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     
                
                cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale, -cubeScale,  cubeScale,     

                // WEST
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     
                -cubeScale, -cubeScale, -cubeScale,     

                -cubeScale, -cubeScale, -cubeScale,     
                -cubeScale, -cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     

                // EAST
                cubeScale,  cubeScale,  cubeScale,     
                cubeScale,  cubeScale, -cubeScale,     
                cubeScale, -cubeScale, -cubeScale,     

                cubeScale, -cubeScale, -cubeScale,     
                cubeScale, -cubeScale,  cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     


                // BOTTOM
                -cubeScale,  cubeScale, -cubeScale,     
                cubeScale,  cubeScale, -cubeScale,     
                cubeScale,  cubeScale,  cubeScale,     
                
                cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale,  cubeScale,     
                -cubeScale,  cubeScale, -cubeScale,     

                // second level
                // SOUTH
                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                 
                cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                  
                cubeScale + offset.x,  cubeScale + offset.y, -cubeScale + offset.z,                  

                cubeScale + offset.x,  cubeScale + offset.y, -cubeScale + offset.z,                  
                -cubeScale + offset.x,  cubeScale + offset.y, -cubeScale + offset.z,                 
                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                 


                //  NORTH
                -cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                
                cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                 
                cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                 
                
                cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                 
                -cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                
                -cubeScale + offset.x, -cubeScale + offset.y,   cubeScale + offset.z,               

                // WEST
                -cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                
                -cubeScale + offset.x,  cubeScale + offset.y, -cubeScale + offset.z,                
                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                

                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                
                -cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                
                -cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                


                // EAST
                cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                 
                cubeScale + offset.x,  cubeScale + offset.y, -cubeScale + offset.z,                 
                cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                 

                cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                 
                cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                 
                cubeScale + offset.x,  cubeScale + offset.y,  cubeScale + offset.z,                 

                // TOP
                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                
                cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                 
                cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                 

                cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                 
                -cubeScale + offset.x, -cubeScale + offset.y,  cubeScale + offset.z,                
                -cubeScale + offset.x, -cubeScale + offset.y, -cubeScale + offset.z,                

            }, {
                faces[FTS].coors.x,   faces[FTS].coors.y,
                faces[FTS].size.x,    faces[FTS].coors.y,
                faces[FTS].size.x,    faces[FTS].size.y,
                faces[FTS].size.x,    faces[FTS].size.y,
                faces[FTS].coors.x,   faces[FTS].size.y,
                faces[FTS].coors.x,   faces[FTS].coors.y,
                faces[FTN].coors.x,   faces[FTN].coors.y,
                faces[FTN].size.x,    faces[FTN].coors.y,
                faces[FTN].size.x,    faces[FTN].size.y,
                faces[FTN].size.x,    faces[FTN].size.y,
                faces[FTN].coors.x,   faces[FTN].size.y,
                faces[FTN].coors.x,   faces[FTN].coors.y,
                faces[FTW].size.x,    faces[FTW].size.y,
                faces[FTW].coors.x,   faces[FTW].size.y,
                faces[FTW].coors.x,   faces[FTW].coors.y,
                faces[FTW].coors.x,   faces[FTW].coors.y,
                faces[FTW].size.x,    faces[FTW].coors.y,
                faces[FTW].size.x,    faces[FTW].size.y,
                faces[FTE].size.x,    faces[FTE].size.y,
                faces[FTE].coors.x,   faces[FTE].size.y,
                faces[FTE].coors.x,   faces[FTE].coors.y,
                faces[FTE].coors.x,   faces[FTE].coors.y,
                faces[FTE].size.x,    faces[FTE].coors.y,
                faces[FTE].size.x,    faces[FTE].size.y,
                faces[FTB].coors.x,   faces[FTB].size.y,
                faces[FTB].size.x,    faces[FTB].size.y,
                faces[FTB].size.x,    faces[FTB].coors.y,
                faces[FTB].size.x,    faces[FTB].coors.y,
                faces[FTB].coors.x,   faces[FTB].coors.y,
                faces[FTB].coors.x,   faces[FTB].size.y,
                faces[FTS].coors.x,   faces[FTS].coors.y,
                faces[FTS].size.x,    faces[FTS].coors.y,
                faces[FTS].size.x,    faces[FTS].size.y,
                faces[FTS].size.x,    faces[FTS].size.y,
                faces[FTS].coors.x,   faces[FTS].size.y,
                faces[FTS].coors.x,   faces[FTS].coors.y,      
                faces[FTN].coors.x,   faces[FTN].coors.y,
                faces[FTN].size.x,    faces[FTN].coors.y,
                faces[FTN].size.x,    faces[FTN].size.y,
                faces[FTN].size.x,    faces[FTN].size.y,
                faces[FTN].coors.x,   faces[FTN].size.y,
                faces[FTN].coors.x,   faces[FTN].coors.y,
                faces[FTW].size.x,    faces[FTW].size.y,
                faces[FTW].coors.x,   faces[FTW].size.y,
                faces[FTW].coors.x,   faces[FTW].coors.y,
                faces[FTW].coors.x,   faces[FTW].coors.y,
                faces[FTW].size.x,    faces[FTW].coors.y,
                faces[FTW].size.x,    faces[FTW].size.y,  
                faces[FTE].size.x,    faces[FTE].size.y,   
                faces[FTE].coors.x,   faces[FTE].size.y,  
                faces[FTE].coors.x,   faces[FTE].coors.y,
                faces[FTE].coors.x,   faces[FTE].coors.y,
                faces[FTE].size.x,    faces[FTE].coors.y,
                faces[FTE].size.x,    faces[FTE].size.y,
                faces[FTT].size.x,    faces[FTT].size.y,
                faces[FTT].coors.x,   faces[FTT].size.y,
                faces[FTT].coors.x,   faces[FTT].coors.y,
                faces[FTT].coors.x,   faces[FTT].coors.y,
                faces[FTT].size.x,    faces[FTT].coors.y,
                faces[FTT].size.x,    faces[FTT].size.y,                
            });
            // mesh.vertn = 60;
        } break;
    }
    return mesh;
};

static std::shared_ptr<CR::Map::TileSource> buildTileSource(const std::string &name, float scale, const std::vector<CR::Map::AtlasSingle> &faces, unsigned type = MeshType::UNIT_CUBE){
    auto src = std::make_shared<CR::Map::TileSource>(CR::Map::TileSource());

    src->name = name;
    src->md = buildMesh(scale, faces, type);

    for(unsigned i = 0; i < faces.size(); ++i){
        src->faces[i] = faces[i];
    }

    return src;
}

static std::vector<Cell> generateEmptyMaze(unsigned w, unsigned h, unsigned defaultType = CellType::WALL){
    unsigned total = w * h;
    std::vector<Cell> maze;
    for(unsigned i = 0; i < total; ++i){
        auto cell = Cell();
        cell.type = defaultType;
        cell.i = i;
        cell.x = i % w;
        cell.y = i / h;
        maze.push_back(cell);
    }
    return maze;
}

struct Room {
    CR::Vec2<int> pos;
    CR::Vec2<int> size;
    Room(){

    }
    Room(unsigned x, unsigned y, unsigned width, unsigned height){
        this->pos.x = x;
        this->pos.y = y;
        this->size.x = width;
        this->size.y = height;
    }

};

static void generateMaze(std::vector<Cell> &src, Cell &startc, unsigned width, unsigned height){
    auto randodd = [](int from, int to){
        return CR::Math::odd(CR::Math::random(from, to));
    };    

    auto fillBorders = [&](Room &room, Cell except = Cell(-1, -1)){
        unsigned absW = room.pos.x + room.size.x;
        unsigned absH = room.pos.y + room.size.y;
        for(int x = room.pos.x; x < absW; ++x){
            for(int y = room.pos.y; y < absH; ++y){
                if(except.x != -1 && except.y != -1 && except.x == x  && except.y == y){ /// ignore door
                    continue;
                }
                if(     x == room.pos.x ||
                        y == room.pos.y ||
                        x == absW-1 ||
                        y == absH-1 ){

                    src[x  + y * width].type = CellType::WALL;
                }
            }
        }        
    };

    auto digDoor = [&](Room &room){
        std::vector<unsigned> sides;
        unsigned absW = room.pos.x + room.size.x;
        unsigned absH = room.pos.y + room.size.y;
        for(int y = room.pos.y; y < absH; ++y){
            for(int x = room.pos.x; x < absW; ++x){
                // avoid corners
                bool isCorner =     (x - room.pos.x) == 0 && (y - room.pos.y) == 0                              || // BOTTOM LEFT
                                    (x - room.pos.x) == 0 && (y - room.pos.y) == room.size.y -1                 || // TOP LEF
                                    (x - room.pos.x) == room.size.x-1 && (y - room.pos.y) == room.size.y-1      ||  // TOP RIGHT
                                    (x - room.pos.x) == room.size.x-1 && (y - room.pos.y) ==  0;                    // BOTTOM RIGHT

                if(x == 0 || y == 0 || x == width-1 || y == height-1 || isCorner){
                    continue;
                }
                if(x == room.pos.x || y == room.pos.y || x == absW-1 || y == absH-1){
                    sides.push_back(src[x  + y * width].i);
                }
            }
        }    
        unsigned index = sides[CR::Math::random(0, sides.size()-1)];
        src[index].type = CellType::PATH;
        return src[index];
    };    

    auto makeRoom = [&](int _x, int _y, int _width, int _height){
        _width = CR::Math::even(_width);
        _height = CR::Math::even(_height);
        for(int x = 0; x < _width; ++x){
            for(int y = 0; y < _height; ++y){
                src[ (x + _x) + (y + _y)* width].type = CellType::PATH;
            }
        }
        auto r = Room(_x, _y, _width, _height);
        return r;
    };   

    auto depthFirstSearch = [&](Cell dstart = Cell(-1, -1)){
        // choose a random start position
        unsigned start = 0;
        Cell spawn(dstart.x, dstart.y);    
        startc.x = spawn.x;
        startc.y = spawn.y;
        startc.i = dstart.x + dstart.y * width;
     
        start = spawn.x + spawn.y * width;
        src[spawn.x + spawn.y * width].type = CellType::PATH;
        auto randdir = [&](){
            std::vector<unsigned> dirs;
            for(int i = 0; i < 4; ++i){
                dirs.push_back(i);
            }
            std::random_shuffle(dirs.begin(), dirs.end());           
            return dirs;
        };
        std::function<void(Cell)> recursion = [&](Cell cell){
            auto dirs = randdir();
            auto index = [&](int x, int y){
                return x + y * height;
            };
            auto ptype = CellType::PATH;
            for(int i = 0; i < dirs.size(); ++i){
                switch(dirs[i]){
                    case 0: { // up
                        if(cell.y - 2 < 0) continue;
                        if(src[index(cell.x, cell.y - 2)].type != ptype){
                            src[index(cell.x, cell.y - 2)].type = ptype;
                            src[index(cell.x, cell.y - 1)].type = ptype;
                            recursion(Cell(cell.x, cell.y - 2));
                        }
                    } break;
                    case 1: { // right
                        if(cell.x + 2 >= width) continue;
                        if(src[index(cell.x + 2, cell.y)].type != ptype){
                            src[index(cell.x + 2, cell.y)].type = ptype;
                            src[index(cell.x + 1, cell.y)].type = ptype;
                            recursion(Cell(cell.x + 2, cell.y));
                        }
                    } break;
                    case 2: { // down
                        if(cell.y + 2 >= height) continue;
                        if(src[index(cell.x, cell.y + 2)].type != ptype){
                            src[index(cell.x, cell.y + 2)].type = ptype;
                            src[index(cell.x, cell.y + 1)].type = ptype;
                            recursion(Cell(cell.x, cell.y + 2));
                        }
                    } break;
                    case 3: { // left
                        if(cell.x - 2 < 0) continue;
                        if(src[index(cell.x - 2, cell.y)].type != ptype){
                            src[index(cell.x - 2, cell.y)].type = ptype;
                            src[index(cell.x - 1, cell.y)].type = ptype;
                            recursion(Cell(cell.x - 2, cell.y));
                        }
                    } break;                                                            
                }
            }
        };
        recursion(spawn);
        // for(int i = 0; i < 3; ++i){
        //     makeRoom(4, 4, 8, 8); // make a random room
        // }

    };

    // for(unsigned i = 0; i < 4; ++i){
    //     unsigned _x = CR::Math::odd(CR::Math::random(4, width - 4));
    //     unsigned _y = CR::Math::odd(CR::Math::random(4, height - 4));
    //     unsigned _w = CR::Math::random(4, 6);
    //     unsigned _h = CR::Math::random(4, 6);
    //     auto r = makeRoom(_x, _y, _w, _h);
    //     // digDoor(r);
    //     CR::log("created room (%ix%i) at %ix%i\n", _w, _h, _x, _y);
    // }
    
    // fillBorders(r, d);

    Cell cell;

    cell.x = CR::Math::odd(CR::Math::random(4, width - 4));
    cell.y = CR::Math::odd(CR::Math::random(4, height - 4));

    cell.i = cell.x + cell.y * width;

    depthFirstSearch(cell);

}


CR::Map::Map::Map(){
    this->tiles = NULL;
    this->worldShader = std::make_shared<CR::Gfx::Shader>(CR::Gfx::Shader());
    this->atlas = std::make_shared<CR::Gfx::Texture>(CR::Gfx::Texture());

}

void CR::Map::Map::build(const CR::Vec2<int> _mapSize, unsigned bpScale, float us){

    game = CR::Gfx::getRenderLayer("world", true);

    this->worldShader->load("data/shader/b_cube_texture_f.glsl", "data/shader/b_cube_texture_v.glsl");
    this->worldShader->findAttrs({"color", "model", "projection", "image", "view"});

    this->atlas->load("data/texture/32x32.bmp");

    CR::Vec2<float> atlasUnitSize(32.0f);

    // _mapSize.x = CR::Math::odd(_mapSize.x);
    // _mapSize.y = CR::Math::odd(_mapSize.y);

    float indexCoorW = atlasUnitSize.x / static_cast<float>(this->atlas->getRsc()->size.x);
    float indexCoorH = atlasUnitSize.y / static_cast<float>(this->atlas->getRsc()->size.y);

    float inAtlasSizeW = static_cast<float>(this->atlas->getRsc()->size.x) / atlasUnitSize.x;
    float inAtlasSizeH = static_cast<float>(this->atlas->getRsc()->size.y) / atlasUnitSize.y;

    this->atlasSingles.resize(inAtlasSizeW * inAtlasSizeH);

    for(unsigned x = 0; x < inAtlasSizeW; ++x){
        for(unsigned y = 0; y < inAtlasSizeH; ++y){
            unsigned index = x + inAtlasSizeW * y;
            CR::Map::AtlasSingle single;
            single.index = index;
            single.coors.x = x * indexCoorW;
            single.coors.y = y * indexCoorH;
            single.size.x = single.coors.x + indexCoorW;
            single.size.y = single.coors.y + indexCoorH;
            this->atlasSingles[index] = single;
        }
    }

    // Build some sources
    this->sources["wall"] = buildTileSource("wall", us, {
                                                            this->atlasSingles[5],  // SOUTH
                                                            this->atlasSingles[5],  // NORTH
                                                            this->atlasSingles[5],  // WEST
                                                            this->atlasSingles[5],  // EAST
                                                            this->atlasSingles[1],  // TOP
                                                            this->atlasSingles[0],  // BOTTOM
                                                        }, MeshType::TALL_CUBE);
    this->sources["floor"] = buildTileSource("floor", us, {
                                                            this->atlasSingles[3],  // SOUTH
                                                            this->atlasSingles[3],  // NORTH
                                                            this->atlasSingles[3],  // WEST
                                                            this->atlasSingles[3],  // EAST
                                                            this->atlasSingles[3],  // TOP
                                                            this->atlasSingles[3],  // BOTTOM
                                                        }, MeshType::PLANE);                                                        

    auto &scale = bpScale;
    CR::Vec2<int> mapSize (CR::Math::odd(_mapSize.x * scale), CR::Math::odd(_mapSize.y * scale));

    this->size.set(mapSize.x, mapSize.y);
    this->tsize = CR::Vec2<float>(this->size.x, this->size.y) * CR::Vec2<float>(us);
    this->usize = us;
    this->totalUnits = mapSize.x * mapSize.y;
    this->bpScale = scale;



    auto maze = generateEmptyMaze(_mapSize.x, _mapSize.y);

    Cell scell;
    generateMaze(maze, scell, _mapSize.x, _mapSize.y);

    
    
    this->tiles = new Tile[this->totalUnits];
    
    // build tiles
    for(unsigned i = 0; i < this->totalUnits; ++i){
        int mazeind = (i % mapSize.x) / scale + ((i / mapSize.x) / scale) * _mapSize.x; 
        this->tiles[i].id = i;
        this->tiles[i].index = i;
        this->tiles[i].type = maze[mazeind].type == CellType::PATH ? CR::Map::TileType::FLOOR : CR::Map::TileType::EMPTY;
        this->tiles[i].height = 0;
        this->tiles[i].position.x = i % mapSize.x;
        this->tiles[i].position.y = i / mapSize.y;
        
        this->tiles[i].rposition.x = this->tiles[i].position.x * (us*2.0f);
        this->tiles[i].rposition.y = -this->tiles[i].height * (us*2.0f);
        this->tiles[i].rposition.z = this->tiles[i].position.y * (us*2.0f);
        
        if(mazeind == scell.i){
            this->start = this->tiles[i];
        }

    }

    auto getNeighbors = [&](Tile *current, const std::vector<unsigned> &pattern){
        std::vector<unsigned> group;

        if(current->position.x-1 >= 0 && current->position.y-1 >= 0){
            auto &other = this->tiles[(current->position.x-1) + ((current->position.y-1) * mapSize.x)];
            group.push_back(other.type);
        }else{
            group.push_back(TileType::OUTBOUNDS);
        } 

        if(current->position.y-1 >= 0){
            auto &other = this->tiles[current->position.x + (current->position.y-1) * mapSize.x];
            group.push_back(other.type);
        }else{
            group.push_back(TileType::OUTBOUNDS);
        }

        if(current->position.x+1 < mapSize.x && current->position.y-1 >= 0){
            auto &other = this->tiles[(current->position.x+1) + ((current->position.y-1) * mapSize.x)];
            group.push_back(other.type);
        }else{
            group.push_back(TileType::OUTBOUNDS);
        } 

        if(current->position.x-1 >= 0){
            auto &other = this->tiles[(current->position.x-1) + current->position.y * mapSize.x];
            group.push_back(other.type);
        }else{
            group.push_back(TileType::OUTBOUNDS);
        }

        if(current->position.x >= 0 && current->position.x < mapSize.x && current->position.y >= 0 && current->position.y < mapSize.y){
            group.push_back(current->type);
        }else{
            group.push_back(TileType::OUTBOUNDS);
        }

        if(current->position.x+1 < mapSize.x){
            auto &other = this->tiles[(current->position.x+1) + current->position.y * mapSize.x];
            group.push_back(other.type);
        }else{
            group.push_back(TileType::OUTBOUNDS);
        } 

        if(current->position.x-1 >= 0 && current->position.y+1 < mapSize.x){
            auto &other = this->tiles[(current->position.x-1) + ((current->position.y+1) * mapSize.x)];
            group.push_back(other.type);
        }else{
            group.push_back(TileType::OUTBOUNDS);
        }       
        
        if(current->position.y+1 < mapSize.y){
            auto &other = this->tiles[current->position.x + (current->position.y+1) * mapSize.x];
            group.push_back(other.type);
        }else{
            group.push_back(TileType::OUTBOUNDS);
        }

        if(current->position.x+1 < mapSize.x && current->position.y+1 < mapSize.y){
            auto &other = this->tiles[(current->position.x+1) + ((current->position.y+1) * mapSize.x)];
            group.push_back(other.type);
        }else{
            group.push_back(TileType::OUTBOUNDS);
        }

        unsigned approved = false;
        for(unsigned i = 0; i < pattern.size(); ++i){
            if(pattern[i] == TileType::ANY || group[i] == pattern[i]){
                ++approved;
            }
        }

        return approved == pattern.size();

    };

    // clear unreachable
    std::vector<CR::Map::Tile> toWall;
    for(unsigned i = 0; i < this->totalUnits; ++i){
        auto &tile = this->tiles[i];

        /*
            T T T
            T T T
            T T T
        */

        #define W CR::Map::TileType::WALL
        #define O CR::Map::TileType::OUTBOUNDS
        #define A CR::Map::TileType::ANY
        #define E CR::Map::TileType::EMPTY
        #define F CR::Map::TileType::FLOOR

        static const std::vector<unsigned> WALL_FILL1 = {
            A, A, A,
            A, E, F,
            A, A, A
        };  

        static const std::vector<unsigned> WALL_FILL2 = {
            A, A, A,
            F, E, A,
            A, A, A
        };  

        static const std::vector<unsigned> WALL_FILL3 = {
            A, A, A,
            A, E, A,
            A, F, A
        };  

        static const std::vector<unsigned> WALL_FILL4 = {
            A, F, A,
            A, E, A,
            A, A, A
        };                          

        static const std::vector<unsigned> WALL_FILL5 = {
            A, A, A,
            A, E, A,
            A, A, F
        };                

        static const std::vector<unsigned> WALL_FILL6 = {
            A, A, A,
            A, E, A,
            F, A, A
        };  

        static const std::vector<unsigned> WALL_FILL7 = {
            F, A, A,
            A, E, A,
            A, A, A
        };  

        static const std::vector<unsigned> WALL_FILL8 = {
            A, A, F,
            A, E, A,
            A, A, A
        };                 

        #undef W
        #undef O
        #undef A
        #undef E
        #undef F

        if(getNeighbors(&tile, WALL_FILL1) || 
           getNeighbors(&tile, WALL_FILL2) ||
           getNeighbors(&tile, WALL_FILL3) ||
           getNeighbors(&tile, WALL_FILL4) ||
           getNeighbors(&tile, WALL_FILL5) ||
           getNeighbors(&tile, WALL_FILL6) ||
           getNeighbors(&tile, WALL_FILL7) ||
           getNeighbors(&tile, WALL_FILL8))    toWall.push_back(tile);
        if(getNeighbors(&tile, WALL_FILL2))    toWall.push_back(tile);
    }
    for(unsigned i = 0; i < toWall.size(); ++i){
        this->tiles[toWall[i].index].type = CR::Map::TileType::WALL;
    }

    // build transforms
    for(unsigned i = 0; i < this->totalUnits; ++i){       

        this->tiles[i].source = this->tiles[i].type == TileType::FLOOR ? this->sources["floor"].get() : this->sources["wall"].get();

        this->tiles[i].transform = new CR::Gfx::Transform();

        this->tiles[i].transform->shader = worldShader;
        this->tiles[i].transform->textures[CR::Gfx::TextureRole::DIFFUSE] = this->atlas->getRsc()->textureId;
        this->tiles[i].transform->shAttrsLoc = worldShader->shAttrs;
        this->tiles[i].transform->position = this->tiles[i].rposition;
        this->tiles[i].transform->scale.set(1.0f);
        this->tiles[i].transform->rotation.set(0, 0, 0, 0);

        auto position = CR::MAT4Identity
                        .translate(this->tiles[i].rposition)
                        .rotate(0, CR::Vec3<float>(1, 1, 1))
                        .scale(CR::Vec3<float>(1.0f));

        this->tiles[i].transform->shAttrsVal = {
            {"image", std::make_shared<CR::Gfx::ShaderAttrInt>(0)},
            {"model", std::make_shared<CR::Gfx::ShaderAttrMat4>(position)},
            {"view", std::make_shared<CR::Gfx::ShaderAttrMat4>(CR::MAT4Identity)},
            {"projection", std::make_shared<CR::Gfx::ShaderAttrMat4>(MAT4Identity)},
            {"color", std::make_shared<CR::Gfx::ShaderAttrColor>(CR::Color(1.0f, 1.0f, 1.0f, 1.0f))}
        };    
        this->tiles[i].transform->fixShaderAttributes({"image", "model", "view", "projection", "color"});


    }

    this->rebuildBatch();

    CR::log("Built map %ix%i(%.0fx%.0f) | BP Scale: %i | Meshes: %i\n", this->size.x, this->size.y, this->tsize.x, this->tsize.y, this->bpScale, this->batchMesh.size());
  
}

void CR::Map::Map::rebuildBatch(){
    this->batchMesh.clear();
    this->batchTrans.clear();
    for(unsigned i = 0; i < this->totalUnits; ++i){
        auto &tile = this->tiles[i];
        if(tile.type == TileType::EMPTY) {continue;}
        this->batchMesh.push_back(&this->tiles[i].source->md);
        this->batchTrans.push_back(this->tiles[i].transform);
    }


}

void CR::Map::Map::render(){
    game->renderOn([&](CR::Gfx::RenderLayer *layer){    

        // for(unsigned i = 0; i < totalUnits; ++i){
        //     if(this->tiles[i].type == TileType::EMPTY) {continue;}
        //     layer->add(CR::Gfx::Draw::Mesh(this->tiles[i].source->md, this->tiles[i].transform));
        // }

        // layer->add(CR::Gfx::Draw::MeshBatch(&this->batchMesh, &this->batchTrans, true, true, true, 1));


    });    
}

void CR::Map::Map::clear(){
    if(tiles == NULL){
        return;
    }

    delete tiles;
    this->width = 0;
    this->height = 0;
}