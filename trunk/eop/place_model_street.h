#ifndef PLACE_MODEL_STREET_H
#define PLACE_MODEL_STREET_H

#include <vector>
#include <set>
#include <stack>
#include <cassert>
namespace RandomMap
{
	enum EGridType{ePath = 0x01,eBarrier = ePath << 1};

	const int eGridSpan = 64;

	using std::vector;
    using std::set;
    using std::pair;
    using std::stack;
    using std::make_pair;
	//P for positive
	//N for negative
	//corner 0-1 means it enclose path or barrier
	enum ModelType{FacePX,FacePZ,FaceNX,FaceNZ,CornerPXPZ0,CornerPXNZ0,CornerNXNZ0,CornerNXPZ0,CornerPXPZ1,CornerPXNZ1,CornerNXNZ1,CornerNXPZ1,UnknownModel};
	struct ModelInfo{
        ModelInfo():model_idx(-1),
        x_grid_length(0),
        z_grid_length(0),
        x_pixel_length(0),
        z_pixel_length(0),
        x_pixel_offset(0),
        z_pixel_offset(0),
        model_type(UnknownModel){}
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
        const StringMap& GetProcessedMap()const
        {
            return processed_str_map_;
        }
		vector<MapModelPosition> PlaceModel(const StringMap& str_map_input,
			const vector<ModelInfo>& out_most_border_models,
			const vector<ModelInfo>& out_most_corner_models,
			const vector<ModelInfo>& inner_barrier_border_models,
			const vector<ModelInfo>& inner_barrier_corner_models)
		{
			vector<MapModelPosition> ret;
			if (str_map_input.empty())
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
				if (str_map[start_z][start_x] & eBarrier)
				{
					outmost_str_map_coords_.insert(cur);
				}
				if (start_x + 1 < str_map[start_z].size() &&( str_map[start_z][start_x+1] & eBarrier))
                    visit_stack.push(make_pair(start_z,start_x + 1));
                if (start_x > 1 &&(  str_map[start_z][start_x-1] & eBarrier))
                    visit_stack.push(make_pair(start_z,start_x-1));
                if (start_z + 1 < str_map.size() && (str_map[start_z + 1][start_x] & eBarrier))
                    visit_stack.push(make_pair(start_z+1,start_x));
                if (start_z > 1 && (str_map[start_z -1][start_x] & eBarrier))
                    visit_stack.push(make_pair(start_z -1,start_x));
                visited_set.insert(cur);
			}
			//search for corners
			for (size_t i = 1;i + 1 < str_map.size(); ++i)
			{
				for (size_t j = 1; j + 1 < str_map[i].size(); ++j)
				{
					place_model_in_corner(i,j,str_map,out_most_corner_models,inner_barrier_corner_models,processed_str_map_,ret);
				}
			}
			//scan row lines
			for (size_t i = 1; i+1< processed_str_map_.size(); ++i)
			{
				for (size_t j = 1; j+1 < processed_str_map_[i].size(); )
				{
					size_t start = j;
					while ((processed_str_map_[i][j] & eBarrier) && 
						(processed_str_map_[i+1][j] & ePath )
						)
					{
						j++;
					}
					size_t end = j;
					place_pz_border(i,start,end,out_most_border_models,inner_barrier_border_models,processed_str_map_,ret);
					start = j;
					while ((processed_str_map_[i][j] & eBarrier) && 
						( (processed_str_map_[i-1][j] & ePath) )
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
					while ((processed_str_map_[i][j] & eBarrier) && 
						(processed_str_map_[i][j+1] & ePath )
						)
					{
						i++;
					}
					size_t end = i;
					place_px_border(start,end,j,out_most_border_models,inner_barrier_border_models,processed_str_map_,ret);
					start = i;
					while ((processed_str_map_[i][j] & eBarrier) && 
						(processed_str_map_[i][j-1] & ePath )
						)
					{
						i++;
					}
					end = i;
					place_nx_border(start,end,j,out_most_border_models,inner_barrier_border_models,processed_str_map_,ret);
					i++;
				}
			}
			return ret;
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
						size_t ix = picked.x_grid_length-1;
						size_t iz = 0;
						size_t tlength = 0;
						for (;ix > 0 && length == 0;ix--)
						{
							for (iz = 0;iz < picked.z_grid_length; iz++)
							{
								if (picked.barrier_info[iz * picked.x_grid_length + ix] & eBarrier)
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
						size_t iz = picked.z_grid_length-1;
						size_t ix = 0;
						size_t tlength = 0;
						for (;iz > 0 && length == 0;iz--)
						{
							for (ix = 0;ix < picked.x_grid_length; ix++)
							{
								if (picked.barrier_info[iz * picked.x_grid_length + ix] & eBarrier)
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
							for (iz = 0;iz < picked.z_grid_length; iz++)
							{
								if (picked.barrier_info[iz*picked.x_grid_length+ix] & eBarrier)
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
							for (ix = 0;ix < picked.x_grid_length; ix++)
							{
								if (picked.barrier_info[iz*picked.x_grid_length+ix] & eBarrier)
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

					if(length <= size_hint)
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
                        if(cur.barrier_info[mi] & eBarrier)
                        {
                           tb_cnt ++; 
                        }
                    }
					if(tb_cnt < barrier_count )
					{
						selected_idx = i;
						barrier_count = tb_cnt;
					}
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
					size_t ix=picked.x_grid_length-1,iz=0;
					for (;ix != -1 ; --ix)
					{
						for (iz = 0;iz < picked.z_grid_length; ++iz)
						{
							if (picked.barrier_info[iz*picked.x_grid_length + ix] & eBarrier)
							{
								find_place = true;
								break;
							}
						}
						if (find_place)
							break;
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
					size_t pz = z - iz;
					z = pz + 1;	
					for (;ix != -1;ix--)
					{
						for (iz = 0; iz < picked.z_grid_length;iz++)
						{
							if (picked.barrier_info[iz*picked.x_grid_length + ix] & eBarrier)
							{
								processed_str_map_[pz][px] = mmp.model_idx << 16;
								if (px == x && z < pz + 1)
								{
									z = pz + 1;	
								}
							}
							pz++;
						}
						pz -= iz;
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
				for (;ix < picked.x_grid_length; ++ix)
				{
					for (iz = 0;iz < picked.z_grid_length; ++iz)
					{
						if (picked.barrier_info[iz * picked.x_grid_length + ix] & eBarrier)
						{
							find_place = true;
							break;
						}
					}
					if(find_place) break;
				}
				if (!find_place)
				{
					z++;
					continue;
				}
				MapModelPosition mmp;
				place_at(z,x,iz,ix,picked,mmp);
				size_t px = x;
				size_t pz = z - iz;
				z = pz + 1;
				for (;ix < picked.x_grid_length; ++ix)
				{
					for (iz = 0;iz < picked.z_grid_length ; ++iz)
					{
						if (picked.barrier_info[iz * picked.x_grid_length + ix] & eBarrier)
						{
							processed_str_map_[pz][px] = picked.model_idx << 16;
							if (px == x && z < pz + 1)
							{
								z = pz+1;
							}
						}
						pz ++;
					}
					pz -= iz;
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
				for(iz = 0;iz<picked.z_grid_length ;++iz)
				{
					for (ix = 0;ix < picked.x_grid_length ;++ix)
					{
						if(picked.barrier_info[iz * picked.x_grid_length + ix] & eBarrier)
						{
							find_place = true;
							break;
						}
					}
					if(find_place) break;
				}
				if (find_place)
				{
					MapModelPosition mmp;
					place_at(z,x,iz,ix,picked,mmp);
					size_t px = x - ix;
					size_t pz = z;
					x = px + 1;
					for (;iz < picked.z_grid_length; ++iz)
					{
						for (ix = 0;ix < picked.x_grid_length ; ++ix)
						{
							if (picked.barrier_info[iz * picked.x_grid_length + ix] & eBarrier)
							{
								processed_str_map_[pz][px] = picked.model_idx << 16;
								if (pz == z && x < px + 1)
								{
									x = px+1;
								}
							}
							px ++;
						}
						px -= ix;
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
			for (size_t x = start; x < end; )
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
					for (iz=picked.z_grid_length-1;iz != -1 ;iz--)
					{
						for (ix = 0; ix < picked.x_grid_length ;ix++)
						{
							if (picked.barrier_info[iz*picked.x_grid_length + ix] & eBarrier)
							{
								find_place = true;
								break;
							}
						}
						if(find_place) break;
					}
					if(!find_place)
					{
						z++;
						continue;
					}
					MapModelPosition mmp;
					place_at(z,x,iz,ix,picked,mmp);
					ret.push_back(mmp);
					size_t px = x - ix;
					size_t pz = z;
					x = px + 1;
					for (;iz != -1;iz--)
					{
						for (ix = 0; ix < picked.x_grid_length;ix++)
						{
							if (picked.barrier_info[iz*picked.x_grid_length + ix] & eBarrier)
							{
								processed_str_map_[pz][px] = mmp.model_idx << 16;
								if (pz == z && x < px + 1)
								{
									x = px + 1;	
								}
							}
							px++;
						}
						px -= ix;
						pz--;
					}
				}
				else
				{
					x++;
				}
			}
		}
		void post_process_str_map(size_t z,size_t x,size_t zi,size_t xi,const ModelInfo& picked,StringMap& str_map)
		{
			size_t pz = z - zi;
			size_t px = x - xi;
			for (zi = 0; zi < picked.z_grid_length; ++zi)
			{
				for(xi = 0; xi < picked.x_grid_length ; ++xi)
				{
					if (picked.barrier_info[zi*picked.x_grid_length + xi] & eBarrier)
					{
						str_map[pz+zi][px+xi] = picked.model_idx << 16;
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
			for (; zi < picked.z_grid_length ; ++zi)
			{
				for(xi=0; xi < picked.x_grid_length  ; ++xi)
					if (picked.barrier_info[zi*picked.x_grid_length + xi] & eBarrier)
					{
						find_place = true;
						break;
					}
				if(find_place)
					break;
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
		void place_model_in_cornerpxnz(ModelType t,size_t z,size_t x,const StringMap& str_map,
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
			for (; zi != -1 ; --zi)
			{
				for(; xi < picked.x_grid_length ; ++xi)
					if (picked.barrier_info[zi*picked.x_grid_length + xi] & eBarrier
						)
					{
						find_corner = true;
						break;
					}
				if (find_corner)
				{
					break;
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


		void place_model_in_cornernxnz(ModelType t,size_t z,size_t x,const StringMap& str_map,
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
			for (; zi != -1 ; --zi)
			{
				for(; xi != -1 ; --xi)
					if (picked.barrier_info[zi*picked.x_grid_length + xi] & eBarrier
						)
					{
						find_corner = true;
						break;
					}
				if (find_corner)
				{
					break;
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

		void place_model_in_cornernxpz(ModelType t,size_t z,size_t x,const StringMap& str_map,
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
			for (; zi < picked.z_grid_length; --zi)
			{
				for(; xi != -1 ; --xi)
					if (picked.barrier_info[zi*picked.x_grid_length + xi] & eBarrier
						)
					{
						find_corner = true;
						break;
					}
				if (find_corner)
				{
					break;
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
			const vector<ModelInfo>& out_most,const vector<ModelInfo>& inner,StringMap& processed,vector<MapModelPosition>& r)
		{
			if ( (str_map[z][x] & eBarrier) == 0)
			{
				return;
			}
            ModelType mt = get_needed_corner_model_type(z,x,str_map);
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
						if (str_map[i-1][j] & ePath)
						{
							path_count++;
						}
						if (str_map[i][j-1] == ePath)
						{
							path_count++;
						}
						if (str_map[i+1][j] == ePath)
						{
							path_count++;
						}
						if (str_map[i][j+1] == ePath)
						{
							path_count++;
						}
						if (path_count < 3)
						{
							tmp[i][j] = str_map[i][j];
						}
						else
							tmp[i][j] = ePath;
					}
				}
			} while(tmp != str_map);
			str_map.swap(tmp);
		}
		bool is_sat_3x3(size_t z,size_t x, const StringMap& str_map,
		int _00,int _01,int _02,
		int _10,int _11,int _12,
		int _20,int _21,int _22
		)
            const
		{
			return (str_map[z+1][x-1] & _00 )
					&&
					(str_map[z+1][x] & _01 )
					&&
					(str_map[z+1][x+1] & _02)
					&&
					(str_map[z][x-1] & _10 )
					&&
					(str_map[z][x] & _11 )
					&&
					(str_map[z][x+1] & _12)
					&&
					(str_map[z-1][x-1] & _20)
					&&
					(str_map[z-1][x] & _21 )
					&&
					(str_map[z-1][x+1] & _22);
		}
		//strict
		ModelType get_needed_corner_model_type(size_t i,size_t j,const StringMap& str_map)const
		{
            assert(i > 0 && i + 1 < str_map.size());
            assert(j > 0 && j + 1 < str_map[i].size());
			if(str_map[i][j] & eBarrier)
			{
				if (
				is_sat_3x3(
				i,j,str_map,
				ePath,         eBarrier,      eBarrier|ePath,
				eBarrier,      eBarrier,      eBarrier|ePath,
				eBarrier|ePath,eBarrier|ePath,eBarrier|ePath
				)
				)
					return CornerNXPZ0;
				if(
				is_sat_3x3(
				i,j,str_map,
				eBarrier|ePath,eBarrier|ePath,eBarrier|ePath,
				eBarrier,      eBarrier,      eBarrier|ePath,
				ePath,         eBarrier,      eBarrier|ePath
				)
				)
					return CornerNXNZ0;
				if(
				is_sat_3x3(
				i,j,str_map,
				eBarrier|ePath,eBarrier|ePath,eBarrier|ePath,
				eBarrier|ePath,eBarrier,      eBarrier,
				eBarrier|ePath,eBarrier,      ePath
				)
				)
					return CornerPXNZ0;
				if(
				is_sat_3x3(i,j,str_map,
				eBarrier|ePath,eBarrier,      ePath,
				eBarrier|ePath,eBarrier,      eBarrier,
				eBarrier|ePath,eBarrier|ePath,eBarrier|ePath
				)
				)
					return CornerPXPZ0;

				if(
				is_sat_3x3(
				i,j,str_map,
				eBarrier,      eBarrier,      ePath,
				eBarrier,      eBarrier,      ePath,
				ePath,         ePath,         ePath
				)
				)
					return CornerNXPZ1;
				if(
				is_sat_3x3(
				i,j,str_map,
				ePath,         ePath,         ePath,
				eBarrier,      eBarrier,      ePath,
				eBarrier,      eBarrier,      ePath
				)
				)
					return CornerNXNZ1;
				if(
				is_sat_3x3
				(i,j,str_map,
				ePath,ePath,         ePath,
				ePath,eBarrier,      eBarrier,
				ePath,eBarrier,      eBarrier
				)
				)
					return CornerPXNZ1;
				if(
				is_sat_3x3(i,j,str_map,
				ePath,eBarrier,      eBarrier,
				ePath,eBarrier,      eBarrier,
				ePath,ePath,         ePath
				)
				)
					return CornerPXPZ1;
			}
			return UnknownModel;
		}
	};

}

#endif
