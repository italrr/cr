# Caribbean Raster [CR]

Caribbean Raster is the evolution of two different engines:
- https://github.com/italrr/nite
- https://github.com/italrr/astro

It borrows concepts, and code from both projects to build a simple but robust enough 3D game engine

Features
- Models are basic .obj
- Animations are interpolations of .obj(s)
- Font rendering on 2D spaces through FreeType
- 

# Cavern Rush / Rush In Dungeons

It's a 3D, randomly generated, multiplayer dungeon crawler.




Bank can only withdraw without penalty after time
If you didn't puck you get charged a penalty





            // for(unsigned i = 0; i < totalUnits; ++i){
                // layer->add(CR::Gfx::Draw::Mesh(this->tiles[i].source->md, this->tiles[i].transform));
            // }

            add += 45 * CR::getDelta();

            // auto position = CR::Vec3<float>(450.0f, -500, 0.0f);

            auto diff = (game->camera.getCenter()) - position;

            auto normal = diff.normalize();

            CR::Vec3<float> front(0, 0, 1);

            if(CR::Input::keyboardPressed(CR::Input::Key::SPACE)){
                position = game->camera.getCenter();
            }

            

            // auto theta = CR::Math::asin(normal.dot(front));
            auto theta = CR::Math::degs(CR::Math::atan(normal.z, normal.x));



            // if(CR::Input::keyboardPressed()){

            // }

            // if(normal.x * normal.z < 0){
            //     theta = 360.0f - theta;
            // }

            // if (theta >= 292.5f && theta < 337.5f)
            //     this->lookAt = 0;
            // else if (theta >= 22.5f && theta < 67.5f)
            //     this->lookAt = 1;
            // else if (theta >= 67.5f && theta < 112.5f)
            //     this->lookAt = 2;
            // else if (theta >= 112.5f && theta < 157.5f)
            //     this->lookAt = 3;
            // else if (theta >= 157.5f && theta < 202.5f)
            //     this->lookAt = 4;


            this->lookAt = CharFace::faceFromAngle(theta);
            // this->lookAt = CharFace::RIGHT;
            CR::log("%s\n", CharFace::str(this->lookAt).c_str());


            // CR::Vec3<float> offPos = game->camera.position + CR::Vec3<float>(CR::Gfx::getWidth(), CR::Gfx::getHeight(), -CR::Gfx::getHeight()) * CR::Vec3<float>(0.5f);

            // CR::log("%s\n", offPos.str().c_str());








    auto depthFirstSearch = [&](Cell dstart){
        
        src[dstart.i].type = CellType::PATH;
        std::stack<Cell> track;
        track.push(dstart);
        unsigned visited = 1;

        auto getNeighbours = [&](Cell &from, std::vector<Cell> &src){
            std::vector<Cell> neighbours;
            // EAST
            if(from.x + 1 < width){
                auto &current = src[(from.x+1) + from.y * width];
                if(current.type == CellType::WALL){
                    neighbours.push_back(current);
                }
            }            
            // NORTH
            if(from.y - 1 >= 0){
                auto &current = src[from.x + (from.y-1) * width];
                if(current.type == CellType::WALL){
                    neighbours.push_back(current);
                }
            }
            // WEST
            if(from.x - 1 >= 0){
                auto &current = src[(from.x-1) + from.y * width];
                if(current.type == CellType::WALL){
                    neighbours.push_back(current);
                }
            }               
            // SOUTH
            if(from.y + 1 < height){
                auto &current = src[from.x + (from.y+1) * width];
                if(current.type == CellType::WALL){
                    neighbours.push_back(current);
                }
            }     
            return neighbours;
        };

        while(visited < src.size() || track.size() == 0){

            auto &top = track.top();
            auto neighbours = getNeighbours(top, src);

            if(neighbours.empty()){
                track.pop();
            }else{
                auto &next = neighbours[CR::Math::random(0, neighbours.size()-1)];
                src[next.i].type = CellType::PATH;
                track.push(next);
                ++visited;
            }
        }
        

    };



// LockStep Implementation Schematic

[AUDIT] -> ["FRAME"] -> [SIMULATION] -> [Animation_Integration]

A "Audit" Is a single state change, ranging from anything like an object being created to an entity moving

Audits are also called frames

A tick is a conjuction of frames that happened together in an order