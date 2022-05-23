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
                enum ImageFormat : int {
                    RED, // 8 bits
                    GREEN, // 8 bits
                    BLUE, // 8 bits
                    RGB, // 24 bits
                    RGBA // 32 bits
                };
            }

            namespace RenderObjectType {
                enum RenderObjectType : int {
                    // FLAT
                    SPRITE, // aka texture
                    PRIMITIVE_2D,
                    TEXT,
                    // 3D
                    MESH,
                    PRIMITIVE_3D,
                    MODEL
                };
                static std::string name(int type){
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
                int type;
                int depth;
                CR::Vec3<float> position;
            };            

            namespace RenderLayerType {
                enum RenderLayerType {
                    T_2D,
                    T_3D
                };
            }

            namespace RenderLayer {
                enum RenderLayer {
                    ZERO = 0,
                    GAME,
                    MIDDLE,
                    UI,
                    POST_1,
                    POST_2,
                    POST_2
                };
            }

            struct RenderLayer {
                int type;
                int order;
                std::string tag;
                std::vector<std::shared_ptr<RenderObject>> objects;
                void begin();
                void end();
                void flush();
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

            int generateTexture2D(unsigned char *data, int w, int h, int format);
            int compileShader(const std::string &vert, const std::string &frag);
            void deleteShader(int id);

        }
        

    }


#endif