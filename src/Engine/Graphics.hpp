#ifndef CR_GRAPHICS_HPP
    #define CR_GRAPHICS_HPP

    #include "Types.hpp"
    #include "Indexer.hpp"
    #include "Resource.hpp"

    namespace CR {

        CR::Indexing::Indexer *getIndexer();
        CR::Resource::ResourceManager *getResourceMngr();

        double getDelta();

        namespace Gfx {

            namespace ImageFormat {
                enum ImageFormat : unsigned {
                    RED, // 8 bits
                    GREEN, // 8 bits
                    BLUE, // 8 bits
                    RGB, // 24 bits
                    RGBA // 32 bits
                };
            }

            namespace RenderObjectType {
                enum RenderObjectType : unsigned {
                    // FLAT
                    SPRITE, // aka texture
                    PRIMITIVE_2D,
                    TEXT,
                    // 3D
                    MESH,
                    PRIMITIVE_3D,
                    MODEL
                };
                static std::string name(unsigned type){
                    switch(type){
                        // FLAT
                        case SPRITE:
                            return "SPRITE";
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

            struct RenderObject {
                unsigned type;
                int depth; // this ignored if the renderlayer type is T_3D
                CR::Vec3<float> position;
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

            struct RenderLayer {
                unsigned id;
                unsigned type;
                int order;
                std::string tag;
                std::vector<RenderObject*> objects;
                void begin();
                void end();
                void flush(); // render to texture
            };

            struct Settings {
                int width;
                int height;
                bool resizable;
                bool fullscreen;
                void setParams(const std::vector<std::string> &params);
                void readSettings(const std::string &path);
            };

            void loadSettings(const std::vector<std::string> &params, const std::string &path);
            
            int addRenderLayer(int type, const std::string &tag, int order);

            void setRenderLayer(int id);


            bool init();
            void end();
            void onEnd();
            void render();
            bool isRunning();

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

            unsigned createTexture2D(unsigned char *data, unsigned w, unsigned h, unsigned format);
            bool deleteTexture2D(unsigned id);
            
            // Note: only use framebuffers directly if it's absolutely necessary, 
            // for everything else, use renderlayer directly
            std::shared_ptr<FramebufferObj> createFramebuffer(unsigned w, unsigned h);
            bool deleteFramebuffer(unsigned id);
            
            unsigned createShader(const std::string &vert, const std::string &frag);
            bool deleteShader(unsigned id);

        }
        

    }


#endif