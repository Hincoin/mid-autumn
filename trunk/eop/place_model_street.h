#ifndef PLACE_MODEL_STREET_H
#define PLACE_MODEL_STREET_H

namespace RandomMap
{
	const int kBarrier = 3;
	const int kPath = 0;
	const int eGridSpan = 64;

	using std::vector;
	//P for positive
	//N for negative
	enum ModelType{FacePX,FacePZ,FaceNX,FaceNZ,CornerPXPZ,CornerPXNZ,CornerNXNZ,CornerNXPZ,UnknownModel};
	struct ModelInfo{
		int model_idx;
		int x_grid_length,z_grid_length;
		int x_pixel_length,z_pixel_length;
		int x_pixel_offset,z_pixel_offset;//when barrier is not exactly fit to model shape
		vector<int> barrier_info;
		ModelType model_type;
	};
	struct MapModelPosition{
		//
		int model_idx;
		size_t x,z;
		int pixel_x,pixel_z;
	};
	typedef vector<vector<int> > StringMap;

	//request it has straight lines
	class PlaceModelStreet{
		set<pair<size_t,size_t> > outmost_str_map_coords_;
		StringMap processed_str_map_;
	public:
		vector<MapModelPosition> PlaceModel(const StringMap& str_map_input,
			const vector<ModelInfo>& out_most_border_models,
			const vector<ModelInfo>& out_most_corner_models,
			const vector<ModelInfo>& inner_barrier_border_models,
			const vector<ModelInfo>& inner_barrier_corner_models)
		{
			vector<MapModelPosition> ret;
			if (str_map.empty())
			{
				return ret;
			}
			StringMap str_map;
			//ensure straight border
			preprocess(str_map_input,str_map);
			processed_str_map_ = str_map;
			//build outmost barrier str map
			size_t start_x=0,start_z=0;
			stack<pair<size_t,size_t> > visit_stack;
			visit_stack.push(make_pair(start_x,start_z));
			while (!visit_stack.empty())
			{
				//todo
				pair<size_t,size_t> cur = visit_stack.top();
				visit_stack.pop();
				if (str_map[start_z][start_x] == kBarrier)
				{
					outmost_str_map_coords_.insert(make_pair(start_z,start_x));
				}
				if (start_x < str_map[start_z].size())
				{
				}
			}
			//search for corners
			for (size_t i = 1;i + 1 < str_map.size(); ++i)
			{
				for (size_t j = 1; j + 1 < str_map[i].size(); ++j)
				{
					place_model_in_corner(processed_str_map_,i,j,str_map,ret,out_most_corner_models,inner_barrier_corner_models);
				}
			}
			//scan row lines
			for (size_t i = 1; i+1< processed_str_map_.size(); ++i)
			{
				for (size_t j = 1; j+1 < processed_str_map_[i].size(); )
				{
					size_t start = j;
					while (processed_str_map_[i][j] == kBarrier && 
						(processed_str_map_[i+1][j] == kPath )
						)
					{
						j++;
					}
					size_t end = j;
					place_pz_border(i,start,end,out_most_border_models,inner_barrier_border_models,processed_str_map_,ret);
					start = j;
					while (processed_str_map_[i][j] == kBarrier && 
						(processed_str_map_[i-1][j] == kPath )
						)
					{
						j++;
					}
					end = j;
					place_nz_border(i,start,end,out_most_border_models,inner_barrier_border_models,processed_str_map_,ret);
					j++;
				}
			}
			//scan col lines
			//todo
		}
	private:
		
		void place_nz_border(size_t z,size_t start,size_t end,
			const vector<ModelInfo>& out_most_border_models,
			const vector<ModelInfo>& inner_barrier_border_models,
			StringMap& processed_str_map_,
			vector<MapModelPosition>& ret)
		{}
		void place_pz_border(size_t z,size_t start,size_t end,
			const vector<ModelInfo>& out_most_border_models,
			const vector<ModelInfo>& inner_barrier_border_models,
			StringMap& processed_str_map_,
			vector<MapModelPosition>& ret)
		{
			for (size_t x = start; x < end; x ++)
			{
				ModelInfo picked;
				//todo
				if(outmost_str_map_coords_.find(make_pair(z,x)) != outmost_str_map_coords_.end())
				{
				}
				else{
					//
				}
				//get_picked(picked,end-x);
				size_t iz,ix;
				bool find_place=false;
				if(picked.z_grid_length>0)
					for (iz=picked.z_grid_length;iz > 0 && !find_place;iz--)
					{
						for (ix = 0; ix < picked.x_grid_length && !find_place;ix++)
						{
							if (picked.barrier_info[iz*picked.x_grid_length + ix] == kBarrier)
							{
								find_place = true;
							}
						}
					}
					int pixel_offset_x = picked.x_pixel_length/2 - ix * eGridSpan ;
					int pixel_offset_z = picked.z_pixel_offset/2 - iz * eGridSpan;
					int pixel_x = x * eGridSpan + pixel_offset_x + picked.x_pixel_offset;
					int pixel_z = z * eGridSpan + pixel_offset_z + picked.z_pixel_offset;
					MapModelPosition mmp;
					mmp.model_idx = picked.model_idx;
					mmp.x = pixel_x / eGridSpan;
					mmp.z = pixel_z / eGridSpan;
					mmp.pixel_x = pixel_x - mmp.x * eGridSpan;
					mmp.pixel_z = pixel_z - mmp.z * eGridSpan;
					ret.push_back(mmp);
					size_t px = x;
					size_t pz = z;
					for (;iz > 0;iz--)
					{
						for (; ix < picked.x_grid_length;ix++)
						{
							if (picked.barrier_info[iz*picked.x_grid_length + ix] == kBarrier)
							{
								processed_str_map_[pz][px] = - mmp.model_idx;
								if (pz == z)
								{
									x = px + 1;	
								}
							}
							px++;
						}
						pz--;
					}
			}
		}
		void place_model_in_corner(size_t z,size_t x,const StringMap& str_map,
			vector<ModelInfo>& out_most,vector<ModelInfo>& inner,StringMap& processed,vector<MapModelPosition>& r)
		{
			if (processed_str_map_[z][x] != kBarrier)
			{
				return;
			}
			switch(get_needed_model_type(z,x,str_map))
			{
			case CornerNXNZ:
				{
					ModelInfo picked;
					if(outmost_str_map_coords_.find(make_pair(z,x)) != outmost_str_map_coords_.end())
					{
						//pick from out most
					}
					else
					{
						//pick from inner
					}
					bool find_corner = false;
					size_t zi = 0,xi=0;
					for (; zi < picked.z_grid_length && !find_corner; ++zi)
					{
						for(; xi < picked.x_grid_length && !find_corner; ++xi)
							if (picked.barrier_info[zi*picked.x_grid_length + xi] == kBarrier)
							{
								find_corner = true;
							}
					}
					int pixel_offset_z = int( picked.z_pixel_length/2 - zi * eGridSpan );
					int pixel_offset_x = int( picked.x_pixel_length/2 - xi * eGridSpan  );
					int z_pixel = z * eGridSpan + pixel_offset_z + picked.z_pixel_offset;
					int x_pixel = x * eGridSpan + pixel_offset_x + picked.x_pixel_offset;
					MapModelPosition mmp;
					mmp.model_idx = picked.model_idx;
					mmp.x = x_pixel / eGridSpan;
					mmp.z = z_pixel / eGridSpan;
					mmp.pixel_x = x_pixel - mmp.x * eGridSpan;
					mmp.pixel_z = z_pixel - mmp.z * eGridSpan;
					r.push_back(mmp);
					processed_str_map_[mmp.z][mmp.x] = -mmp.model_idx;
					int px = x;
					int pz = z;
					for (;zi<picked.z_grid_length;++zi)
					{
						for(;xi < picked.x_grid_length; ++xi)
						{
							if (picked.barrier_info[zi*picked.x_grid_length + xi] == kBarrier)
							{
								processed_str_map_[pz][px] = -mmp.model_idx;	
							}
							px++;
						}
						pz++;
					}
				}
				break;
				//todo
			case UnknownModel:break;
			}
		}

		void preprocess(const StringMap& str_map_input,StringMap& str_map)
		{
			//iterate to throw away the place that has 3 neighbors are paths
		}
		ModelType get_needed_model_type(size_t i,size_t j,const StringMap& str_map)const
		{
			//todo
			if(str_map[i][j] == kBarrier)
			{
				if (str_map[i][j] str_map[i-1][j-1] == kBarrier &&
				str_map[i][j])
			{
				return CornerNXNZ;
			}

			return CornerNXPZ;
			return CornerPXNZ;
			return CornerPXPZ;
			}
			return UnknownModel;

		}
	};

}

#endif