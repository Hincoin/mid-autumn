#include <string>
#include <iostream>
#include <cstdio>

#include "construct_by_connection.hpp"

//Ex.
//PL=a;b;c;d
//PU=b;c;e;
//PR
//PD
//CLR = a,b;c,d;
//CUD = a,c;b,d;
//static std::map<std::string,
std::vector<Connector*> build_from_string(
        const std::vector<std::string>& all_scenes,
        const std::vector<std::string>& ls,
        const std::vector<std::string>& us,
        const std::vector<std::string>& rs,
        const std::vector<std::string>& ds,
        const std::vector<std::pair<std::string,std::string> >& clrs,
        const std::vector<std::pair<std::string,std::string> >& cuds)
{
    typedef std::vector<std::string>::const_iterator iterator_t;
    typedef std::vector<std::pair<std::string,std::string> >::const_iterator iterator_p_t;
    int key = 0;
    std::vector<Connector*> ret;
    ret.resize(all_scenes.size());
    for(iterator_t it = all_scenes.begin();
            it != all_scenes.end(); ++it)
    {
        int path_type = 0;
        if (std::find(ls.begin(), ls.end(), *it) != ls.end())
            path_type |= PL;
        if (std::find(us.begin(), us.end(), *it) != us.end())
            path_type |= PU;
        if (std::find(rs.begin(), rs.end(), *it) != rs.end())
            path_type |= PR;
        if (std::find(ds.begin(), ds.end(), *it) != ds.end())
            path_type |= PD;
        ret[key] = (new Connector(key,path_type)); 
        ++key;
    }
    for(iterator_p_t it = clrs.begin();
            it != clrs.end();
            ++it)
    {
        iterator_t t0 = std::find(all_scenes.begin(),all_scenes.end(),it->first);
        iterator_t t1 = std::find(all_scenes.begin(),all_scenes.end(),it->second);
        assert(all_scenes.end() != t0);
        assert(all_scenes.end() != t1);
        ret[std::distance(all_scenes.begin(),t0)]->add_connector_1d<R>(ret[std::distance(all_scenes.begin(), t1)]);
    }
    for(iterator_p_t it = cuds.begin();
            it != cuds.end();
            ++it)
    {
        iterator_t t0 = std::find(all_scenes.begin(),all_scenes.end(),it->first);
        iterator_t t1 = std::find(all_scenes.begin(),all_scenes.end(),it->second);
        assert(all_scenes.end() != t0);
        assert(all_scenes.end() != t1);
        ret[std::distance(all_scenes.begin(),t0)]->add_connector_1d<D>(ret[std::distance(all_scenes.begin(), t1)]);
    }
    return ret;
}

//be careful path and connection are same here
std::vector<std::string> init_scene_string(
        std::vector<std::string>& ls,
        std::vector<std::string>& us,
        std::vector<std::string>& rs,
        std::vector<std::string>& ds,
        std::vector<std::pair<std::string,std::string> >& clrs,
        std::vector<std::pair<std::string,std::string> >& cuds
        )
{
    std::vector<std::string> all;
    all.push_back("DL");
    all.push_back("LR");
    all.push_back("UD");
    all.push_back("LU");
    all.push_back("UR");
    all.push_back("RD");
    all.push_back("LUR");
    all.push_back("URD");
    all.push_back("RDL");
    all.push_back("DLU");

    ls.push_back("DL");
    ls.push_back("LU");
    ls.push_back("LUR");
    ls.push_back("RDL");
    ls.push_back("DLU");
    ls.push_back("LR");

    us.push_back("LU");
    us.push_back("UD");
    us.push_back("UR");
    us.push_back("LUR");
    us.push_back("URD");
    us.push_back("DLU");

    rs.push_back("UR");
    rs.push_back("RD");
    rs.push_back("LR");
    rs.push_back("LUR");
    rs.push_back("URD");
    rs.push_back("RDL");


    ds.push_back("DL");
    ds.push_back("UD");
    ds.push_back("RD");
    ds.push_back("URD");
    ds.push_back("RDL");
    ds.push_back("DLU");

    for(size_t i = 0; i < rs.size(); ++i)
        for(size_t j = 0; j < ls.size(); ++j)
        {
            clrs.push_back(std::make_pair(rs[i],ls[j]));
        }
    for(size_t i = 0; i < ds.size(); ++i)
        for(size_t j = 0; j < us.size(); ++j)
        {
            cuds.push_back(std::make_pair(ds[i],us[j]));
        }

    return all;
}


void test_case()
{
    std::vector<std::string> ls;
    std::vector<std::string> us;
    std::vector<std::string>  rs;
    std::vector<std::string> ds;
    std::vector<std::pair<std::string,std::string> > clrs;
    std::vector<std::pair<std::string,std::string> > cuds;

    std::vector<std::string> all = init_scene_string(ls,us,rs,ds,clrs,cuds);
    std::vector<Connector*> cs = build_from_string(all,ls,us,rs,ds,clrs,cuds);
    for(int i = 2;i < 20; ++i)
        for(int j = 2;j < 20; ++j)
        {
            ConnectorMatrix m = construct_by_connection(i,j,i*20 + j,cs);
            assert(is_solution(m));
            if(is_solution(m))
            {   //output 
                for(size_t z = 0; z < m.size(); ++z)
                {
                    for(size_t x = 0; x < m[z].size(); ++x)
                    {
                        printf("%5.5s",all[m[z][x]->get_key()].c_str());
                    }
                    printf("\n");
                }
                printf("\n");
            }
        }

    for(size_t i = 0;i < cs.size(); ++i)
    {
        delete cs[i];
    }
}
int main()
{
    test_case();
    return 0;
}



