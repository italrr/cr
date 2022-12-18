#ifndef CR_GRAPHICS_HPP
    #define CR_GRAPHICS_HPP

    #include "Types.hpp"
    #include "Resource.hpp"

    #define CR_ENABLE_DEBUG_BUILD 1

    #if CR_ENABLE_DEBUG_BUILD 

        void ____CR_GFX_CHECK_OPENGL_ERROR();

    #endif

    namespace CR {

        double getDelta();

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

            struct FramebufferObj {
                unsigned framebufferId;
                unsigned textureId;
                unsigned renderbufferId;
                unsigned width;
                unsigned height;
                FramebufferObj(unsigned framebufferId, unsigned textureId, unsigned renderbufferId, unsigned width, unsigned height){
                    this->framebufferId = framebufferId;
                    this->textureId = textureId;
                    this->renderbufferId = renderbufferId;
                    this->width = width;
                    this->height = height;
                }
            };

            namespace ImageFormat {
                enum ImageFormat : unsigned {
                    RED, // 8 bits
                    GREEN, // 8 bits
                    BLUE, // 8 bits
                    RG, // 16 bit
                    RGB, // 24 bits
                    RGBA, // 32 bits
                };
            }

            namespace RenderableType {
                enum RenderableType : unsigned {
                    // FLAT
                    TEXTURE,
                    PRIMITIVE_2D,
                    TEXT,
                    // 3D
                    MESH,
                    PRIMITIVE_3D,
                    MODEL
                };
                static std::string name(unsigned type){
                    switch(type){
                        // FLAT (This won't draw on T_3D)
                        case TEXTURE:
                            return "TEXTURE";
                        case PRIMITIVE_2D:
                            return "PRIMITIVE_2D";
                        case TEXT:
                            return "TEXT";                                                        
                        //3D
                        case MESH:
                            return "MESH";
                        case PRIMITIVE_3D:
                            return "PRIMITIVE_3D";
                        case MODEL:
                            return "MODEL";
                        default:
                            return "UNDEFINED";
                    }
                }
            }

            namespace TextureRole {
                enum TextureRole : int {
                    DIFFUSE,
                    SPECULAR,
					NORMAL,
					HEIGHT,
                    NONE
                };
                static std::string str(int t){
                    switch(t){
                        case DIFFUSE: return "DIFFUSE";
                        case SPECULAR: return "SPECULAR";
                        case NORMAL: return "NORMAL";
                        case HEIGHT: return "HEIGHT";
                        default: return "NONE";
                    }
                }
            }

            namespace VertexRole {
                enum VertexRole : unsigned {
                    POSITION = 0,
                    TEXCOORD
                };
            }

            namespace VertexStoreType {
                enum VertexStoreType : unsigned {
                    STATIC = 1,
                    DYNAMIC = 2
                };

            }

            struct MeshData {
                unsigned vbo[2];
                unsigned vao;
                unsigned ebo;
                unsigned vertn;
                void copy(const CR::Gfx::MeshData &other){
                    this->vbo[0] = other.vbo[0];
                    this->vbo[1] = other.vbo[1];
                    this->vao = other.vao;
                    this->ebo = other.ebo;
                    this->vertn = other.vertn;
                }
                MeshData(){
                    vbo[0] = vbo[1] = 0;
                    vao = 0;
                    ebo = 0;
                    vertn = 0;
                }
            };
            

            struct Mesh {
                CR::Gfx::MeshData md;             
                Mesh(){
                    
                }
            };
                        
            struct ShaderAttr;
            struct Texture;
            struct Shader;
            struct FontResource;

            struct Transform {
                std::shared_ptr<CR::Gfx::Shader> shader;
				// CR::Mat<4, 4, float> model;
                CR::Vec3<float> position;
                CR::Vec3<float> scale;
                CR::Vec4<float> rotation;
				// CR::Vec3<float> position;
				// CR::Color color;
                std::vector<unsigned> textures;

                std::unordered_map<std::string, unsigned> shAttrsLoc;
                std::unordered_map<std::string, std::shared_ptr<CR::Gfx::ShaderAttr>> shAttrsVal;
				
                std::vector<CR::Gfx::ShaderAttr*> shAttrsValVec;
                std::vector<unsigned> shAttrsLocVec;

                Transform();

                void fixShaderAttributes(const std::vector<std::string> &locOrder);
            };

            struct RenderLayer;
            
            struct Renderable {
                unsigned type;
                void (*render)(Renderable*, RenderLayer *rl);
                // std::function<void(Renderable*, RenderLayer *rl)> render;
            };

            struct Renderable2D : Renderable {
                Vec2<float> position;
                Vec2<int> size;
                Vec2<float> origin;
                Vec2<float> scale;
                Vec2<int> origSize;
                Rect<float> region;  
                unsigned handleId;              
                float angle;
                Renderable2D(){
                    scale = CR::Vec2<float>(1.0f);
                }
            };

            struct RenderableText : Renderable {
                std::string text;
                Gfx::FontResource *rsc;
                Vec2<float> position;          
                float angle;
                RenderableText(){
                    type = RenderableType::TEXT;
                }
            };

            struct Renderable3D : Renderable {
                CR::Gfx::Transform *transform;
                CR::Gfx::MeshData md;
            };          
            
            struct Renderable3DBatch : Renderable3D {
                std::vector<CR::Gfx::MeshData*> *md;
                std::vector<CR::Gfx::Transform*> *transform;
                unsigned modelPos;
                bool shareTexture;
                bool shareShader;
                bool shareModelTrans;
            };                          

            namespace RenderLayerType {
                enum RenderLayerType : unsigned {
                    T_2D,
                    T_3D
                };
            }

            namespace RenderLayers {
                enum RenderLayers : unsigned {
                    BOTTOM = 0,
                    GAME,
                    MIDDLE,
                    UI,
                    TOP
                };
            }

            struct Camera {
				CR::Vec3<float> position;
				CR::Vec3<float> target;
				CR::Vec3<float> up;
                CR::Vec3<float> targetBias;
                CR::Vec3<float> positionBias;
                CR::Vec3<float> getCenter();
				void setPosition(const CR::Vec3<float> &pos);
				void setUp(const CR::Vec3<float> &up);
                void update();
                void setTarget(const CR::Vec3<float> &up);
				CR::Mat<4, 4, float> getView();
                Camera();
            };

            struct RenderLayer {
                unsigned id;
                unsigned type;
                // std::shared_ptr<CR::Gfx::Transform> transform;
                std::shared_ptr<FramebufferObj> fb;
                int order;
                int depth; // current depth (doesn't work in T_3D)
                std::string tag;
                std::vector<Renderable*> objects;
                CR::Gfx::Camera camera;
                CR::Mat<4, 4, float> projection;
                std::mutex accesMutex;
                CR::Vec2<int> size;
                void add(const std::vector<Renderable*> &objs);
                void add(Renderable* obj);
                bool init(unsigned type,  int width, int height);
                bool init(unsigned type);
                void setDepth(int n);
                void renderOn(const std::function<void(RenderLayer *layer)> &what, bool flush, bool clear);
                void renderOn(const std::function<void(RenderLayer *layer)> &whatr);
                void clear(); 
                void flush();
                void end();
            };
            
            std::shared_ptr<RenderLayer> createRenderLayer(const CR::Vec2<int> &size, int type);
            std::shared_ptr<RenderLayer> createRenderLayer(const CR::Vec2<int> &size, int type, const std::string &tag, bool systemLayer, int order = -1); // -1 = auto
            std::shared_ptr<RenderLayer> getRenderLayer(int id, bool isSystemLayer = true);
            std::shared_ptr<RenderLayer> getRenderLayer(const std::string &tag, bool isSystemLayer = true);

            namespace Draw {
                CR::Gfx::Renderable *RenderLayer(const std::shared_ptr<CR::Gfx::RenderLayer> &rl, const CR::Vec2<float> &pos, const CR::Vec2<int> &size, const CR::Vec2<float> &origin, float angle);        
                CR::Gfx::Renderable *Texture(const std::shared_ptr<CR::Gfx::Texture> &tex, const CR::Vec2<float> &pos, const CR::Vec2<int> &size, const CR::Vec2<float> &origin, float angle); 
                CR::Gfx::Renderable *Texture(unsigned tex, const CR::Vec2<float> &pos, const CR::Vec2<int> &size, const CR::Vec2<float> &origin, float angle); 
                CR::Gfx::Renderable *Mesh(CR::Gfx::MeshData &md, CR::Gfx::Transform *transform); 
                CR::Gfx::Renderable *MeshBatch(std::vector<CR::Gfx::MeshData*> *md, std::vector<CR::Gfx::Transform*> *trans, bool shareTexture, bool shareShader, bool shareModel, unsigned modelPos = 0); 
                CR::Gfx::Renderable *Text(CR::Gfx::FontResource *font, const std::string &text, const CR::Vec2<float> &pos);        
            } 

            bool init();
            void end();
            void onEnd();
            void render();
            bool isRunning();
            int getWidth();
            int getHeight();
            CR::Vec2<int> getSize();

            bool pasteSubTexture2D(unsigned id, unsigned char *data, unsigned w, unsigned h, unsigned format, unsigned x, unsigned y);
            unsigned createTexture2D(unsigned char *data, unsigned w, unsigned h, unsigned format);
            bool deleteTexture2D(unsigned id);
            
            std::shared_ptr<FramebufferObj> createFramebuffer(unsigned w, unsigned h);
            bool deleteFramebuffer(unsigned id);
            
            CR::Gfx::MeshData createMesh(const std::vector<float> &pos, const std::vector<float> &tex);
            CR::Gfx::MeshData createMesh(const std::vector<float> &pos, const std::vector<float> &tex, unsigned vPosStType, unsigned vTexStType);
            bool updateMesh(CR::Gfx::MeshData &md, unsigned vrole, const std::vector<float> &vertex);

            unsigned createShader(const std::string &vert, const std::string &frag);
            int findShaderAttr(unsigned shaderId, const std::string &name);
            struct ShaderAttr;
            void applyShader(unsigned shaderId, const std::vector<unsigned> &loc, const std::vector<CR::Gfx::ShaderAttr*> &attributes);
            void applyShaderPartial(unsigned loc, CR::Gfx::ShaderAttr* attr);
            bool deleteShader(unsigned id);

        }
        

    }


#endif