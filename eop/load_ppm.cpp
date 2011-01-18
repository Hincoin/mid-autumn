#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string.h>
#include <malloc.h>

using namespace std;
typedef vector<vector<char> > barrier_map_t;
vector<vector<char> > read_pgm_map(const char* file_name)
{
   FILE* fptr = ::fopen(file_name,"r"); 
   if(fptr == NULL) return barrier_map_t();
#define PGMREADBUFLEN 256
   char buf[PGMREADBUFLEN]={0};
   unsigned int ncol,nrow,nmax;
   char* t = fgets(buf,PGMREADBUFLEN,fptr);
   if( (t == NULL) || (strncmp(buf,"P5\n",3)!=0))
       return barrier_map_t();
    do
    {
        t = fgets(buf,PGMREADBUFLEN,fptr);
        if (t == NULL) return barrier_map_t();
    }while(strncmp(buf,"#",1) == 0);
    
    int r = sscanf(buf,"%u %u",&ncol,&nrow);
    if (r < 2) return barrier_map_t();
    r = fscanf(fptr,"%u\n",&nmax);
    if(r<1)return barrier_map_t();
    barrier_map_t::value_type v;
    v.resize(ncol);
    barrier_map_t ret;
    ret.resize(nrow,v);
    char* image_buf = (char*)calloc(ncol*nrow,sizeof(char));
    if (image_buf)
    {
        size_t rd = fread(image_buf,sizeof(char),ncol*nrow,fptr);
        if(rd < ncol*nrow)
        {
            free(image_buf);
            return barrier_map_t();
        }
        for(size_t i = 0;i < nrow;i++)
            for(size_t j = 0;j < ncol;j++)
            {
                ret[i][j] = image_buf[i*ncol+j];
            }
        free(image_buf);
        return ret;
    }
    return barrier_map_t();
}


int main(int argc,char**argv)
{
    if(argc > 1)
    {
        barrier_map_t str_map = read_pgm_map(argv[1]);
        for(size_t i = 0;i < str_map.size(); ++i)
        {
            for(size_t j = 0;j < str_map[i].size(); ++j)
            {    printf("%3d",str_map[i][j]);}
            printf("\n");
        }
    }
}
