#include "Engine/Log.hpp"
#include "Engine/Tools.hpp"
#include "Map.hpp"

static std::shared_ptr<CR::Gfx::RenderLayer> game;

static CR::Gfx::MeshData buildCube(float cubeScale, const std::vector<CR::Map::AtlasSingle> &faces){

    auto FTS    = CR::Map::FaceType::SOUTH;    
    auto FTN    = CR::Map::FaceType::NORTH;    
    auto FTW    = CR::Map::FaceType::WEST;    
    auto FTE    = CR::Map::FaceType::EAST;    
    auto FTT    = CR::Map::FaceType::TOP;    
    auto FTB    = CR::Map::FaceType::BOTTOM;    

    auto mesh = CR::Gfx::createMesh({ 
        // POSITION                             // TEXTURE

        // SOUTH
        -cubeScale, -cubeScale, -cubeScale,     faces[FTS].coors.x,   faces[FTS].coors.y,
         cubeScale, -cubeScale, -cubeScale,     faces[FTS].size.x,    faces[FTS].coors.y,
         cubeScale,  cubeScale, -cubeScale,     faces[FTS].size.x,    faces[FTS].size.y,

         cubeScale,  cubeScale, -cubeScale,     faces[FTS].size.x,    faces[FTS].size.y,
        -cubeScale,  cubeScale, -cubeScale,     faces[FTS].coors.x,   faces[FTS].size.y,
        -cubeScale, -cubeScale, -cubeScale,     faces[FTS].coors.x,   faces[FTS].coors.y,

        //  NORTH
        -cubeScale, -cubeScale,  cubeScale,     faces[FTN].coors.x,   faces[FTN].coors.y,
         cubeScale, -cubeScale,  cubeScale,     faces[FTN].size.x,    faces[FTN].coors.y,
         cubeScale,  cubeScale,  cubeScale,     faces[FTN].size.x,    faces[FTN].size.y,
         
         cubeScale,  cubeScale,  cubeScale,     faces[FTN].size.x,    faces[FTN].size.y,
        -cubeScale,  cubeScale,  cubeScale,     faces[FTN].coors.x,   faces[FTN].size.y,
        -cubeScale, -cubeScale,  cubeScale,     faces[FTN].coors.x,   faces[FTN].coors.y,

        // WEST
        -cubeScale,  cubeScale,  cubeScale,     faces[FTW].size.x,    faces[FTW].size.y,
        -cubeScale,  cubeScale, -cubeScale,     faces[FTW].coors.x,   faces[FTW].size.y,
        -cubeScale, -cubeScale, -cubeScale,     faces[FTW].coors.x,   faces[FTW].coors.y,

        -cubeScale, -cubeScale, -cubeScale,     faces[FTW].coors.x,   faces[FTW].coors.y,
        -cubeScale, -cubeScale,  cubeScale,     faces[FTW].size.x,    faces[FTW].coors.y,
        -cubeScale,  cubeScale,  cubeScale,     faces[FTW].size.x,    faces[FTW].size.y,

        // EAST
         cubeScale,  cubeScale,  cubeScale,     faces[FTE].size.x,    faces[FTE].size.y,
         cubeScale,  cubeScale, -cubeScale,     faces[FTE].coors.x,   faces[FTE].size.y,
         cubeScale, -cubeScale, -cubeScale,     faces[FTE].coors.x,   faces[FTE].coors.y,

         cubeScale, -cubeScale, -cubeScale,     faces[FTE].coors.x,   faces[FTE].coors.y,
         cubeScale, -cubeScale,  cubeScale,     faces[FTE].size.x,    faces[FTE].coors.y,
         cubeScale,  cubeScale,  cubeScale,     faces[FTE].size.x,    faces[FTE].size.y,

        // TOP
        -cubeScale, -cubeScale, -cubeScale,     faces[FTT].size.x,    faces[FTT].size.y,
         cubeScale, -cubeScale, -cubeScale,     faces[FTT].coors.x,   faces[FTT].size.y,
         cubeScale, -cubeScale,  cubeScale,     faces[FTT].coors.x,   faces[FTT].coors.y,

         cubeScale, -cubeScale,  cubeScale,     faces[FTT].coors.x,   faces[FTT].coors.y,
        -cubeScale, -cubeScale,  cubeScale,     faces[FTT].size.x,    faces[FTT].coors.y,
        -cubeScale, -cubeScale, -cubeScale,     faces[FTT].size.x,    faces[FTT].size.y,

        // BOTTOM
        -cubeScale,  cubeScale, -cubeScale,     faces[FTB].coors.x,   faces[FTB].size.y,
         cubeScale,  cubeScale, -cubeScale,     faces[FTB].size.x,    faces[FTB].size.y,
         cubeScale,  cubeScale,  cubeScale,     faces[FTB].size.x,    faces[FTB].coors.y,
         
         cubeScale,  cubeScale,  cubeScale,     faces[FTB].size.x,    faces[FTB].coors.y,
        -cubeScale,  cubeScale,  cubeScale,     faces[FTB].coors.x,   faces[FTB].coors.y,
        -cubeScale,  cubeScale, -cubeScale,     faces[FTB].coors.x,   faces[FTB].size.y
    });
    mesh.vertn = 36;
    return mesh;
};

static std::shared_ptr<CR::Map::TileSource> buildTileSource(const std::string &name, float scale, const std::vector<CR::Map::AtlasSingle> &faces){
    auto src = std::make_shared<CR::Map::TileSource>(CR::Map::TileSource());

    src->name = name;
    src->md = buildCube(scale, faces);

    for(unsigned i = 0; i < faces.size(); ++i){
        src->faces[i] = faces[i];
    }

    return src;
}

CR::Map::Map::Map(){
    this->tiles = NULL;
    this->worldShader = std::make_shared<CR::Gfx::Shader>(CR::Gfx::Shader());
    this->atlas = std::make_shared<CR::Gfx::Texture>(CR::Gfx::Texture());

}


void CR::Map::Map::build(const CR::Vec2<int> mapSize, const CR::Map::UnitSize &us){

    game = CR::Gfx::getRenderLayer("world", true);

    this->worldShader->load("data/shader/b_cube_texture_f.glsl", "data/shader/b_cube_texture_v.glsl");
    this->worldShader->findAttrs({"color", "model", "projection", "image", "view"});

    this->totalUnits = mapSize.x * mapSize.y;
    this->atlas->load("data/texture/32x32.bmp");

    CR::Vec2<float> atlasUnitSize(32.0f);

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
    this->sources["wall"] = buildTileSource("wall", 50, {
                                                            this->atlasSingles[4],  // SOUTH
                                                            this->atlasSingles[4],  // NORTH
                                                            this->atlasSingles[4],  // WEST
                                                            this->atlasSingles[4],  // EAST
                                                            this->atlasSingles[1],  // TOP
                                                            this->atlasSingles[0],  // BOTTOM
                                                        });


    this->tiles = new Tile[this->totalUnits];

    for(unsigned i = 0; i < this->totalUnits; ++i){
        this->tiles[i].id = i;
        this->tiles[i].index = i;
        this->tiles[i].type = CR::Map::TileType::WALL;
        this->tiles[i].height = 0;
        this->tiles[i].position.x = i % mapSize.x;
        this->tiles[i].position.y = i / mapSize.y;
        this->tiles[i].source = this->sources["wall"].get();
        this->tiles[i].transform.shader = worldShader;
        this->tiles[i].transform.textures[CR::Gfx::TextureRole::DIFFUSE] = this->atlas->getRsc()->textureId;
        this->tiles[i].transform.shAttrsLoc = worldShader->shAttrs;
        auto position = CR::MAT4Identity.translate(CR::Vec3<float>(0)).rotate(0, CR::Vec3<float>(1, 1, 1)).scale(CR::Vec3<float>(1.0f));
        this->tiles[i].transform.shAttrsVal = {
            {"image", std::make_shared<CR::Gfx::ShaderAttrInt>(0)},
            {"model", std::make_shared<CR::Gfx::ShaderAttrMat4>(position)},
            {"view", std::make_shared<CR::Gfx::ShaderAttrMat4>(CR::MAT4Identity)},
            {"projection", std::make_shared<CR::Gfx::ShaderAttrMat4>(MAT4Identity)},
            {"color", std::make_shared<CR::Gfx::ShaderAttrColor>(CR::Color(1.0f, 1.0f, 1.0f, 1.0f))}
        };    
        this->tiles[i].transform.fixShaderAttributes({"image", "model", "projection", "color"});

    }

}

void CR::Map::Map::render(){
    game->renderOn([&](CR::Gfx::RenderLayer *layer){    

        for(unsigned i = 0; i < totalUnits; ++i){
            layer->add(CR::Gfx::Draw::Mesh(this->tiles[i].source->md, &this->tiles[i].transform));
        }


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