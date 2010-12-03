#include <fstream>
#include "place_model_street.h"
int main()
{
    using namespace RandomMap;   
    //read test scene
    std::ifstream i_scene_file("place_model_test.txt");
    char c;
    StringMap str_map;

	const int COL_COUNT = 20;
	const int ROW_COUNT = 20;
	int col_count = COL_COUNT;
	int row_count = ROW_COUNT;
	while (row_count --)
	{
		vector<int> row;
		col_count = COL_COUNT;
		while(col_count --)
		{
			i_scene_file >> c;

			switch(c){
            case '#':
                row.push_back(eBarrier);
                break;
            case '.':
                row.push_back(ePath);
                break;
            default:assert(false);
			}
		}
		str_map.push_back(row);
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
        m.z_pixel_length = z_length * eGridSpan + 12;
        m.x_pixel_length = x_length * eGridSpan + 20;
        while(z_length--)
        {
			x_length = m.x_grid_length;
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
        }
		v.push_back(m);
    }
    vector<ModelInfo> border_models;
    vector<ModelInfo> corner_models;
    //set model type explicitly
    //case 1
    std::vector<ModelInfo> t1;
    t1.push_back(v[0]);
    t1[t1.size()-1].model_type = FacePX;

    t1.push_back(v[0]);
    t1[t1.size()-1].model_type = FacePZ;

    t1.push_back(v[0]);
    t1[t1.size()-1].model_type = FaceNX;

    t1.push_back(v[0]);
    t1[t1.size()-1].model_type = FaceNZ;

    t1.push_back(v[0]);
    t1[t1.size()-1].model_type = CornerNXNZ1;
    t1.push_back(v[0]);
    t1[t1.size()-1].model_type = CornerNXPZ1;
    t1.push_back(v[0]);
    t1[t1.size()-1].model_type = CornerPXPZ1;
    t1.push_back(v[0]);
    t1[t1.size()-1].model_type = CornerPXNZ1;

    t1.push_back(v[1]);
    t1[t1.size()-1].model_type = FacePX;

    t1.push_back(v[1]);
    t1[t1.size()-1].model_type = FacePZ;

    t1.push_back(v[1]);
    t1[t1.size()-1].model_type = FaceNX;

    t1.push_back(v[1]);
    t1[t1.size()-1].model_type = FaceNZ;

    t1.push_back(v[2]);
    t1[t1.size()-1].model_type = CornerNXNZ0;

    t1.push_back(v[3]);
    t1[t1.size()-1].model_type = CornerPXNZ0;

    t1.push_back(v[4]);
    t1[t1.size()-1].model_type = CornerPXPZ0;

    t1.push_back(v[5]);
    t1[t1.size()-1].model_type = CornerPXPZ0;

    t1.push_back(v[6]);
    t1[t1.size()-1].model_type = CornerNXPZ0;

    t1.push_back(v[7]);
    t1[t1.size()-1].model_type = CornerNXPZ0;

    t1.push_back(v[8]);
    t1[t1.size()-1].model_type = FacePZ;

    t1.push_back(v[8]);
    t1[t1.size()-1].model_type = FaceNZ;

    t1.push_back(v[9]);
    t1[t1.size()-1].model_type = FacePX;

    t1.push_back(v[9]);
    t1[t1.size()-1].model_type = FaceNX;


    t1.push_back(v[10]);
    t1[t1.size()-1].model_type = FacePZ;
 
    t1.push_back(v[10]);
    t1[t1.size()-1].model_type = FaceNZ;
    
    for(size_t i = 0;i < t1.size(); ++i)
    {
        if(t1[i].model_type == FacePX || t1[i].model_type == FacePZ ||
                t1[i].model_type == FaceNX || t1[i].model_type == FaceNZ)
            border_models.push_back(t1[i]);
        else
            corner_models.push_back(t1[i]);
    }
    PlaceModelStreet pms;
    vector<MapModelPosition> ms = pms.PlaceModel(str_map,border_models, corner_models,border_models, corner_models);

    const StringMap& psm = pms.GetProcessedMap();
    for(size_t i = 0;i < psm.size(); ++i)
    {
        for(size_t j = 0; j < psm[i].size(); ++j)
        {
            if(psm[i][j] & (ePath ))
                printf("%5s",".");
            else if(psm[i][j] & (eBarrier))
                printf("%5s","#");
            else
            {
                int midx = psm[i][j]>>16;
                printf("%5d",midx);
            }
            
        }
        printf("\n");
    }
       //
    //
    //
    //
}

