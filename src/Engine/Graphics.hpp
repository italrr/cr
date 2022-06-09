#ifndef CR_GRAPHICS_HPP
    #define CR_GRAPHICS_HPP

    #include "Types.hpp"
    #include "Resource.hpp"

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
                unsigned width;
                unsigned height;
                FramebufferObj(unsigned framebufferId, unsigned textureId, unsigned width, unsigned height){
                    this->framebufferId = framebufferId;
                    this->textureId = textureId;
                    this->width = width;
                    this->height = height;
                }
            };

            namespace ImageFormat {
                enum ImageFormat : unsigned {
                    RED, // 8 bits
                    GREEN, // 8 bits
                    BLUE, // 8 bits
                    RGB, // 24 bits
                    RGBA // 32 bits
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
            }

            struct BindTexture {
                unsigned textureId;
                int role;
				BindTexture(unsigned textureId, int role){
					this->textureId = textureId;
					this->role = role;
				}
                BindTexture(){
                    this->role = TextureRole::NONE;
                }
            };

			namespace MaterialMode {
				enum MaterialMode : int {
					DIFFUSE = 1,
					SPECULAR = 2,
					NORMAL = 4,
					HEIGHT = 8,
				};
			}

            namespace MeshType {
                enum MeshType : unsigned {
                    PRIMITIVE, // verts only
                    COMPLEX // verts & indices
                };
            }

			struct Material {
				int diffuse;
				int specular;
				float shininess;
				Material(){

				}
				Material(int diffuse, int specular, float shininess){
					this->diffuse = diffuse;
					this->specular = specular;
					this->shininess = shininess;
				}
			};

            struct MeshData {
                unsigned vbo;
                unsigned vao;
                unsigned ebo;
                MeshData(){
                    vbo = 0;
                    vao = 0;
                    ebo = 0;
                }
            };
            

            struct Transform {
				int matMode;
				CR::Gfx::Material material;
				CR::Mat<4, 4, float> model;
				CR::Vec3<float> position;
				CR::Color color;
				Transform(){
					this->model = CR::MAT4Identity;
					this->matMode = 0;
					enMatMode(Gfx::MaterialMode::DIFFUSE);
					enMatMode(Gfx::MaterialMode::SPECULAR);
					enMatMode(Gfx::MaterialMode::NORMAL);
					enMatMode(Gfx::MaterialMode::HEIGHT);
				}
				void enMatMode(int mode){
					this->matMode = this->matMode | (1 << mode);
				}
				void disMatMode(int mode){
					this->matMode = this->matMode & (~mode);
				}
				void resetMatMode(){
					this->matMode = 0;
				}
				bool hasMatMode(int mode){
					return this->matMode & (1 << mode); 
				}
            };

            struct Vertex {
                CR::Vec3<float> position;
                CR::Vec3<float> normal;
                CR::Vec2<float> texCoords;
				CR::Vec3<float> tangent;
				CR::Vec3<float> bitangent;
				// bone data
				unsigned int id[4];
				float weight[4];
				Vertex();
				void setBoneData(unsigned int bId, float weight){
                    for(unsigned int i = 0; i < 4; ++i){ 
                        if(this->weight[i] == 0.0f){
                            this->id[i] = bId;
                            this->weight[i] = weight;
                            return;
                        }
                    }                    
                }           
            };

            struct Mesh {
                std::vector<CR::Gfx::Vertex> vertices;
                const std::vector<unsigned int> indices;
                unsigned nverts;
                unsigned strides;
                unsigned vbo;
                unsigned vao;
                unsigned ebo;
            };

            struct RenderLayer;
            struct Renderable {
                Transform transform;
                unsigned type;
                void *self;
                std::function<void(Renderable*, RenderLayer *rl)> render;
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

            struct Renderable3D : Renderable {
                // transformation
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
				CR::Vec3<float> front;
				CR::Vec3<float> up;
				CR::Vec3<float> right;
				CR::Vec3<float> worldUp;
				float yaw;
				float pitch;
				void setPosition(const CR::Vec3<float> &pos);
				void setFront(const CR::Vec3<float> &front);
				void update();
				void setUp(const CR::Vec3<float> &up);
				void init();
				CR::Mat<4, 4, float> getView();
            };

            struct RenderLayer {
                unsigned id;
                unsigned type;
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
                bool init(int width, int height);
                bool init();
                void setDepth(int n);
                void renderOn(const std::function<void(RenderLayer *layer)> &what, bool flush, bool clear);
                void renderOn(const std::function<void(RenderLayer *layer)> &whatr);
                void clear(); 
                void flush();
            };
            
            std::shared_ptr<RenderLayer> createRenderLayer(const CR::Vec2<int> &size, int type);
            std::shared_ptr<RenderLayer> createRenderLayer(const CR::Vec2<int> &size, int type, const std::string &tag, bool systemLayer, int order = -1); // -1 = auto
            std::shared_ptr<RenderLayer> getRenderLayer(int id, bool isSystemLayer);
            std::shared_ptr<RenderLayer> getRenderLayer(const std::string &tag, bool isSystemLayer);

            // 2D default render methods (mainly for debugging, or specific internal usage)
            struct Texture;
            namespace Draw {
                CR::Gfx::Renderable *RenderLayer(const std::shared_ptr<CR::Gfx::RenderLayer> &rl, const CR::Vec2<float> &pos, const CR::Vec2<int> &size, const CR::Vec2<float> &origin, float angle);        
                CR::Gfx::Renderable *Texture(const std::shared_ptr<CR::Gfx::Texture> &tex, const CR::Vec2<float> &pos, const CR::Vec2<int> &size, const CR::Vec2<float> &origin, float angle); 
            } 

            bool init();
            void end();
            void onEnd();
            void render();
            bool isRunning();
            int getWidth();
            int getHeight();
            CR::Vec2<int> getSize();

            unsigned createTexture2D(unsigned char *data, unsigned w, unsigned h, unsigned format);
            bool deleteTexture2D(unsigned id);
            
            std::shared_ptr<FramebufferObj> createFramebuffer(unsigned w, unsigned h);
            bool deleteFramebuffer(unsigned id);
            
            CR::Gfx::MeshData createPrimMesh(const std::vector<float> &vertices);
            CR::Gfx::MeshData createMesh(const std::vector<CR::Gfx::Vertex> &vertices, const std::vector<unsigned int> &indices);
            bool deleteMesh(MeshData &md);

            unsigned createShader(const std::string &vert, const std::string &frag);
            unsigned findShaderAttr(unsigned shaderId, const std::string &name);
            struct ShaderAttr;
            bool applyShader(unsigned shaderId, const std::unordered_map<std::string, unsigned> &loc, const std::unordered_map<std::string, std::shared_ptr<CR::Gfx::ShaderAttr>> &attrs);
            bool deleteShader(unsigned id);

        }
        

    }


#endif