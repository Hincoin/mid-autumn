#include "place_model_street.h"
#include <fstream>
int main()
{
    using namespace RandomMap;   
    //read test scene
    std::ifstream i_scene_file("place_model_test.txt");
    char c;
    StringMap str_map;
    while(i_scene_file >> c)
    {
        vector<int> row;
        switch(c){
            case '#':
                row.push_back(eBarrier);
                break;
            case '.':
                row.push_back(ePath);
                break;
            case '\n':
                str_map.push_back(row);
                row.clear();
                break; 
            default:assert(false);
        }
    }
    //read test models
    std::ifstream i_model_file("place_model_test_models.txt");
    std::vector<ModelInfo> v;
    int idx,z_length,x_length;
    while(i_model_file >> idx >> z_length >> x_length)
    {
        ModelInfo m;
        m.model_idx = idx;
        m.z_grid_length = z_length;
        m.x_grid_length = x_length;
        while(z_length--)
        {
            while(x_length--)
            {
                i_model_file >> c;
                if(c == '#')
                {
                    m.barrier_info.push_back(eBarrier);
                }
                else if(c == '.')
                {
                    m.barrier_info.push_back(ePath);
                }
            }
            i_model_file >> c;
        }
    }
    //set model type explicitly
    //case 1
    //
    //
    //
???
