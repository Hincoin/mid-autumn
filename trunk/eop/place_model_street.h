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
	//corner 0-1 means it enclose path or barrier
	enum ModelType{FacePX,FacePZ,FaceNX,FaceNZ,CornerPXPZ0,CornerPXNZ0,CornerNXNZ0,CornerNXPZ0,CornerPXPZ1,CornerPXNZ1,CornerNXNZ1,CornerNXPZ1,UnknownModel};
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
			stack<pair<size_t,size_t> > visit_stack;
			visit_stack.push(make_pair(0,0));
            set<pair<size_t,size_t> > visited_set;
            //dfs
			while (!visit_stack.empty())
			{
				pair<size_t,size_t> cur = visit_stack.top();
				visit_stack.pop();
                if(visited_set.find(cur) != visited_set.end())
                    continue;

                size_t start_z=cur.first;
			    size_t start_x=cur.second;
				if (str_map[start_z][start_x] == kBarrier)
				{
					outmost_str_map_coords_.insert(cur);
				}
				if (start_x + 1 < str_map[start_z].size() && str_map[start_z][start_x+1] == kBarrier)
                    visit_stack.push(make_pair(start_z,start_x + 1));
                if (start_x > 1 && str_map[start_z][start_x-1] == kBarrier)
                    visit_stack.push(make_pair(start_z,start_x-1));
                if (start_z + 1 < str_map.size() && str_map[start_z + 1][start_x] == kBarrier)
                    visit_stack.push(make_pair(start_z+1,start_x));
                if (start_z > 1 && str_map[start_z -1][start_x])
                    visit_stack.push(make_pair(start_z -1,start_x));
                visited_set.insert(cur);
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
			for (size_t j = 1;j + 1 < processed_str_map_[0].size();++j)
			{
				for (size_t i = 1;i + 1 < processed_str_map_.size(); )
				{
					size_t start = i;
					while (processed_str_map_[i][j] == kBarrier && 
						(processed_str_map_[i][j+1] == kPath )
						)
					{
						i++;
					}
					size_t end = i;
					place_px_border(start,end,j,out_most_border_models,inner_barrier_border_models,processed_str_map_,ret);
					start = i;
					while (processed_str_map_[i][j] == kBarrier && 
						(processed_str_map_[i][j-1] == kPath )
						)
					{
						i++;
					}
					end = i;
					place_nx_border(start,end,j,out_most_border_models,inner_barrier_border_models,processed_str_map_,ret);
					i++;
				}
			}
		}
	private:
		bool pick_model_for_border(ModelType t,size_t z,size_t x,
			const vector<ModelInfo>& out_most_border_models,
			const vector<ModelInfo>& inner_barrier_border_models,
			size_t size_hint ,
			ModelInfo& p
			)
		{
			const vector<ModelInfo>& models = outmost_str_map_coords_.find(make_pair(z,x)) != outmost_str_map_coords_.end() ? out_most_border_models:inner_barrier_border_models;
			size_t ret_idx = size_t(-1);
			size_t diff_size = size_t (-1);
			for (size_t i = 0;i < models.size(); ++i)
			{
				const ModelInfo& picked = models[i];
				if(t == models[i].model_type)
				{
					size_t length = 0;
					if(t == FacePX)
					{
						size_t ix = picked.x_grid_length;
						size_t iz = 0;
						size_t tlength = 0;
						for (;ix > 0 && length == 0;ix--)
						{
							for (;iz < picked.z_grid_length; iz++)
							{
								if (picked.barrier_info[iz * picked.x_grid_length + ix] == kBarrier)
								{
									tlength ++;
									length = tlength;
								}
								else if (length > 0)
								{
									tlength ++;
								}
							}
						}
					}
					if(t == FacePZ)
					{
						size_t iz = picked.z_grid_length;
						size_t ix = 0;
						size_t tlength = 0;
						for (;iz > 0 && length == 0;iz--)
						{
							for (;ix < picked.x_grid_length; ix++)
							{
								if (picked.barrier_info[iz * picked.x_grid_length + ix] == kBarrier)
								{
									tlength ++;
									length = tlength;
								}
								else if (length > 0)
								{
									tlength ++;
								}
							}
						}
					}
					if(t == FaceNX)
					{
						size_t ix = 0;
						size_t iz = 0;
						size_t tlength = 0;
						for (;ix < picked.x_grid_length && length == 0;ix++)
						{
							for (;iz < picked.z_grid_length; iz++)
							{
								if (picked.barrier_info[iz*picked.x_grid_length+ix] == kBarrier)
								{
									tlength ++;
									length = tlength;
								}
								else if (length > 0)
								{
									tlength ++;
								}
							}
						}
					}
					if(t == FaceNZ)
					{
						size_t ix = 0;
						size_t iz = 0;
						size_t tlength = 0;
						for (;iz < picked.z_grid_length && length == 0;iz++)
						{
							for (;ix < picked.x_grid_length; ix++)
							{
								if (picked.barrier_info[iz*picked.x_grid_length+ix] == kBarrier)
								{
									tlength ++;
									length = tlength;
								}
								else if (length > 0)
								{
									tlength ++;
								}
							}
						}
					}

					if(length < size_hint)
					{
						if (diff_size > size_hint - length)
						{
							diff_size = size_hint - length;
							ret_idx = i;
						}
					}
				}
			}
			if(ret_idx != size_t (-1))
			{
				p = models[ret_idx];
				return true;
			}
			return false;
		}

		bool pick_model_for_corner(ModelType t,size_t z,size_t x,
			const vector<ModelInfo>& out_most_border_models,
			const vector<ModelInfo>& inner_barrier_border_models,
			ModelInfo& picked
			)
		{
			const vector<ModelInfo>& models = outmost_str_map_coords_.find(make_pair(z,x)) != outmost_str_map_coords_.end() ? out_most_border_models:inner_barrier_border_models;
            size_t barrier_count = size_t(-1);
            size_t selected_idx = 0;
            //minimize the barrier count
            for(size_t i = 0;i < models.size(); ++i)
            {
                const ModelInfo& cur = models[i];
                size_t tb_cnt = 0;
                if(cur.model_type == t)
                {
                    for(size_t mi = 0;mi < cur.barrier_info.size(); ++mi)
                    {
                        if(cur.barrier_info[mi] == kBarrier)
                        {
                           tb_cnt ++; 
                        }
                    }
                }
                if(tb_cnt < barrier_count )
                {
                    selected_idx = i;
                    barrier_count = tb_cnt;
                }
            }
            if(barrier_count == size_t(-1))
                return false;
            picked = models[selected_idx];
            return true;
		}
		void place_at(size_t z,size_t x, size_t iz,size_t ix,const ModelInfo& picked,MapModelPosition& mmp)
		{
			int pixel_offset_x = picked.x_pixel_length/2 - ix * eGridSpan ;
			int pixel_offset_z = picked.z_pixel_length/2 - iz * eGridSpan;
			int pixel_x = x * eGridSpan + pixel_offset_x + picked.x_pixel_offset;
			int pixel_z = z * eGridSpan + pixel_offset_z + picked.z_pixel_offset;
			mmp.model_idx = picked.model_idx;
			mmp.x = pixel_x / eGridSpan;
			mmp.z = pixel_z / eGridSpan;
			mmp.pixel_x = pixel_x - mmp.x * eGridSpan;
			mmp.pixel_z = pixel_z - mmp.z * eGridSpan;
		}
		//along the z direction but face positive x direction
		void place_px_border(size_t start,size_t end,size_t x,
			const vector<ModelInfo>& out_most_border_models,
			const vector<ModelInfo>& inner_barrier_border_models,
			StringMap& processed_str_map_,
			vector<MapModelPosition>& ret
			)
		{
			for (size_t z = start;z< end;)
			{
				ModelInfo picked;

				if (pick_model_for_border(FacePX,z,x,out_most_border_models,inner_barrier_border_models,end-z,picked) && picked.x_grid_length > 0)
				{
					bool find_place = false;
					size_t ix=picked.x_grid_length,iz=0;
					for (;ix > 0 && !find_place; --ix)
					{
						for (;iz < picked.z_grid_length && !find_place; ++iz)
						{
							if (processed_str_map_[iz][ix] == kBarrier)
							{
								find_place = true;
							}
						}
					}
					if (!find_place)
					{
						z++;
						continue;
					}
					MapModelPosition mmp;
					place_at(z,x,iz,ix,picked,mmp);
					ret.push_back(mmp);
					size_t px = x;
					size_t pz = z;
					for (;ix > 0;ix--)
					{
						for (; iz < picked.z_grid_length;iz++)
						{
							if (picked.barrier_info[iz*picked.x_grid_length + ix] == kBarrier)
							{
								processed_str_map_[pz][px] = - mmp.model_idx;
								if (pz == z)
								{
									z = pz + 1;	
								}
							}
							pz++;
						}
						px--;
					}
				}
				else
				{
					z++;
				}
			}
		}
		void place_nx_border(size_t start,size_t end,size_t x,
			const vector<ModelInfo>& out_most_border_models,
			const vector<ModelInfo>& inner_barrier_border_models,
			StringMap& processed_str_map_,
			vector<MapModelPosition>& ret
			)
		{
			for (size_t z = start; z < end;)
			{
				ModelInfo picked;
				if (!pick_model_for_border(FaceNX,z,x,out_most_border_models,inner_barrier_border_models,end-z,picked))
				{
					z++;
					continue;
				}
				bool find_place = false;
				size_t ix = 0;
				size_t iz = 0;
				for (;ix < picked.x_grid_length && !find_place; ++ix)
				{
					for (;iz < picked.z_grid_length && !find_place; ++iz)
					{
						if (picked.barrier_info[iz * picked.x_grid_length + ix] == kBarrier)
						{
							find_place = true;
						}
					}
				}
				if (!find_place)
				{
					z++;
					continue;
				}
				MapModelPosition mmp;
				place_at(z,x,iz,ix,picked,mmp);
				size_t px = x;
				size_t pz = z;
				for (;ix < picked.x_grid_length; ++ix)
				{
					for (;iz < picked.z_grid_length ; ++iz)
					{
						if (picked.barrier_info[iz * picked.x_grid_length + ix] == kBarrier)
						{
							processed_str_map_[pz][px] = - picked.model_idx;
							if (px == x)
							{
								z = pz+1;
							}
						}
						pz ++;
					}
					px ++;
				}
			}
		}

		void place_nz_border(size_t z,size_t start,size_t end,
			const vector<ModelInfo>& out_most_border_models,
			const vector<ModelInfo>& inner_barrier_border_models,
			StringMap& processed_str_map_,
			vector<MapModelPosition>& ret)
		{
			for (size_t x = start; x < end;)
			{
				ModelInfo picked;
				if (!pick_model_for_border(FaceNZ,z,x,out_most_border_models,inner_barrier_border_models,end-x,picked))
				{
					x++;
					continue;
				}
				size_t iz=0,ix=0;
				bool find_place = false;
				for(iz = 0;iz<picked.z_grid_length && !find_place;++iz)
				{
					for (ix = 0;ix < picked.x_grid_length && !find_place;++ix)
					{
						if(picked.barrier_info[iz * picked.x_grid_length + ix] == kBarrier)
							find_place = true;
					}
				}
				if (find_place)
				{
					MapModelPosition mmp;
					place_at(z,x,iz,ix,picked,mmp);
					size_t px = x;
					size_t pz = z;
					for (;iz < picked.z_grid_length; ++iz)
					{
						for (;ix < picked.x_grid_length ; ++ix)
						{
							if (picked.barrier_info[iz * picked.x_grid_length + ix] == kBarrier)
							{
								processed_str_map_[pz][px] = - picked.model_idx;
								if (pz == z)
								{
									x = px+1;
								}
							}
							px ++;
						}
						pz ++;
					}
				}
				else
					x++;
			}
		}
		void place_pz_border(size_t z,size_t start,size_t end,
			const vector<ModelInfo>& out_most_border_models,
			const vector<ModelInfo>& inner_barrier_border_models,
			StringMap& processed_str_map_,
			vector<MapModelPosition>& ret)
		{
			for (size_t x = start; x < end; x ++)
			{
				ModelInfo picked;
				if (!pick_model_for_border(FacePZ,z,x,out_most_border_models,inner_barrier_border_models,end-x,picked))
				{
					x++;
					continue;
				}
				size_t iz,ix;
				bool find_place=false;
				if(picked.z_grid_length>0)
				{
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
					if(!find_place)
					{
						z++;
						continue;
					}
					MapModelPosition mmp;
					place_at(z,x,iz,ix,picked,mmp);
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
				else
				{
					x++;
				}
			}
		}
		bool get_corner_type(ModelType t,const ModelInfo& picked, const StringMap& str_map,size_t& zi,size_t& xi)const
		{
			switch(t)
			{
			case CornerPXPZ1:
				{
				}
				break;
			case CornerPXPZ0:
				{
				}
				break;
			case CornerPXNZ0:
				{

				}
				break;
			case CornerPXNZ1:
				{

				}
				break;
			case CornerNXNZ0:
				{

				}
				break;
			case CornerNXNZ1:
				{

				}
				break;
			case CornerNXPZ0:
				{

				}
				break;
			case CornerNXPZ1:
				{

				}
				break;
			}
			return false;
		}

		void post_process_str_map(size_t z,size_t x,size_t zi,size_t xi,const ModelInfo& picked,StringMap& str_map)
		{
			size_t pz = z - zi;
			size_t px = x - xi;
			str_map[pz][px] = -picked.model_idx;
			for (zi = 0; zi < picked.z_grid_length; ++zi)
			{
				for(xi = 0; xi < picked.x_grid_length ; ++xi)
				{
					if (picked.barrier_info[zi*picked.x_grid_length + xi] == kBarrier)
					{
						str_map[pz+zi][px+xi] = -picked.model_idx;
					}
				}
			}
		}
		void place_model_in_cornerpxpz(ModelType t,size_t z,size_t x,const StringMap& str_map,
			const vector<ModelInfo>& out_most,
			const vector<ModelInfo>& inner,
			StringMap& processed,
			vector<MapModelPosition>& r
			)
		{
			ModelInfo picked;
			//pick model
			if (!pick_model_for_corner(t,z,x,out_most,inner,picked))
			{
				return;
			}

			//  ##
			//  @#
			bool find_place = false;
			size_t zi = 0;
			size_t xi = 0;
			for (; zi < picked.z_grid_length  && !find_place; ++zi)
			{
				for(; xi < picked.x_grid_length && !find_place ; ++xi)
					if (picked.barrier_info[zi*picked.x_grid_length + xi] == kBarrier)
					{
						find_place = true;
					}
			}
			if (!find_place)
			{
				assert(false);
				return ;
			}
			MapModelPosition mmp;
			place_at(z,x,zi,xi,picked,mmp);
			r.push_back(mmp);
			post_process_str_map(z,x,zi,xi,picked,processed);
		}
		void place_model_in_cornerpxnz(size_t z,size_t x,const StringMap& str_map,
			const vector<ModelInfo>& out_most,
			const vector<ModelInfo>& inner,
			StringMap& processed,
			vector<MapModelPosition>& r
			)
		{
			// @##
			// ##
			// #
			ModelInfo picked;
				//pick model
			if (!pick_model_for_corner(t,z,x,out_most,inner,picked))
			{
				return;
			}

			bool find_corner = false;
			size_t zi = picked.z_grid_length - 1,xi=0;
			for (; zi > 0 && !find_corner; --zi)
			{
				for(; xi < picked.x_grid_length && !find_corner; ++xi)
					if (picked.barrier_info[zi*picked.x_grid_length + xi] == kBarrier
						)
					{
						find_corner = true;
					}
			}

			if(!find_corner)
			{
				assert(false);
				return ;
			}
			MapModelPosition mmp;
			place_at(z,x,zi,xi,picked,mmp);
			r.push_back(mmp);
			post_process_str_map(z,x,zi,xi,picked,processed);
		}


		void place_model_in_cornernxnz(size_t z,size_t x,const StringMap& str_map,
			const vector<ModelInfo>& out_most,
			const vector<ModelInfo>& inner,
			StringMap& processed,
			vector<MapModelPosition>& r
			)
		{
			ModelInfo picked;
			if (!pick_model_for_corner(t,z,x,out_most,inner,picked))
			{
				return;
			}

			//##@
			// ##
			// 
			bool find_corner = false;
			size_t zi = picked.z_grid_length - 1 ;
			size_t xi = picked.x_grid_length - 1;
			for (; zi>0 && !find_corner; --zi)
			{
				for(; xi>0 && !find_corner; --xi)
					if (picked.barrier_info[zi*picked.x_grid_length + xi] == kBarrier
						)
					{
						find_corner = true;
					}
			}
			if(!find_corner)
			{
				assert(false);
				return ;
			}
			MapModelPosition mmp;
			place_at(z,x,zi,xi,picked,mmp);
			r.push_back(mmp);
			post_process_str_map(z,x,zi,xi,picked,processed_str_map_);
		}

		void place_model_in_cornernxpz(size_t z,size_t x,const StringMap& str_map,
			const vector<ModelInfo>& out_most,
			const vector<ModelInfo>& inner,
			StringMap& processed,
			vector<MapModelPosition>& r
			)
		{
			ModelInfo picked;
            if (!pick_model_for_corner(t,z,x,out_most,inner,picked))
			{
				return;
			}

			//###
			//##@
			// 
				
			bool find_corner = false;
			size_t zi = 0 ;
			size_t xi = picked.x_grid_length - 1;
			for (; zi < picked.z_grid_length && !find_corner; --zi)
			{
				for(; xi>0 && !find_corner; --xi)
					if (picked.barrier_info[zi*picked.x_grid_length + xi] == kBarrier
						)
					{
						find_corner = true;
					}
			}
			if(!find_corner)
			{
				assert(false);
				return ;
			}
			MapModelPosition mmp;
			place_at(z,x,zi,xi,picked,mmp);
			r.push_back(mmp);
			post_process_str_map(z,x,zi,xi,picked,processed_str_map_);

		}
		void place_model_in_corner(size_t z,size_t x,const StringMap& str_map,
			vector<ModelInfo>& out_most,vector<ModelInfo>& inner,StringMap& processed,vector<MapModelPosition>& r)
		{
			if (processed_str_map_[z][x] != kBarrier)
			{
				return;
			}
            ModelType mt = get_needed_model_type(z,x,str_map);
			switch(mt)
			{
			case CornerPXPZ1:
            case CornerPXPZ0:
				{
					place_model_in_cornerpxpz(mt,z,x,str_map,out_most,inner,processed,r);
				}
				break;
			case CornerNXNZ0:
            case CornerNXNZ1:
				{
                    place_model_in_cornernxnz(mt,z,x,str_map,out_most,inner,processed,r);
				}
				break;
			case CornerNXPZ0:
			case CornerNXPZ1:
				{
                    place_model_in_cornernxpz(mt,z,x,str_map,out_most,inner,processed,r);
				}
				break;
			case CornerPXNZ0:
			case CornerPXNZ1:
				{
                    place_model_in_cornerpxnz(mt,z,x,str_map,out_most,inner,processed,r);
				}
				break;
			case CornerPXPZ0:
			case CornerPXPZ1:
				{
                    place_model_in_cornerpxpz(mt,z,x,str_map,out_most,inner,processed,r);
				}
				break;
			case UnknownModel:break;
			default:break;
			}
		}

		void preprocess(const StringMap& str_map_input,StringMap& str_map)
		{
			//iterate to throw away the place that has 3 neighbors are paths
			//todo: antialias
			str_map = str_map_input;
			StringMap tmp = str_map;
			do 
			{
				for (size_t i = 1;i + 1 < str_map.size();++i)
				{
					for (size_t j = 1; j+1<str_map[i].size(); ++j)
					{
						int path_count = 0;
						if (str_map[i-1][j] == kPath)
						{
							path_count++;
						}
						if (str_map[i][j-1] == kPath)
						{
							path_count++;
						}
						if (str_map[i+1][j] == kPath)
						{
							path_count++;
						}
						if (str_map[i][j+1] == kPath)
						{
							path_count++;
						}
						if (path_count < 3)
						{
							tmp[i][j] = str_map[i][j]
						}
						else
							tmp[i][j] = kPath;
					}
				}
			} while(tmp != str_map);
			str_map.swap(tmp);
		}
		ModelType get_needed_model_type(size_t i,size_t j,const StringMap& str_map)const
		{
			//todo
            assert(i > 1 && i + 1 < str_map.size());
            assert(j > 1 && j + 1 < str_map[i].size());
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
