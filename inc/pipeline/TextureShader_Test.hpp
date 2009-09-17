#ifndef TEXTURESHADER_TEST_HPP_INCLUDED
#define TEXTURESHADER_TEST_HPP_INCLUDED

#ifndef __INCLUDE_SHADER_TEST_HPP__
#define __INCLUDE_SHADER_TEST_HPP__

#include "Vector.hpp"

#include <boost/pointee.hpp>

#include "MAFragmentProcessor.hpp"
#include "MAMath.hpp"
#include "Timer.hpp"
#define HAS_TEXTURE
#include "test_data.hpp"



#define LINEAR_Z_INTERPOLATE
namespace ma_test
{
    using namespace ma;
    transform3f perspective_matrix(const float fovy, const float aspect, const float zNear, const float zFar)
    {
        const float dz = zFar - zNear;
        const float rad = fovy / float(2) * float(M_PI/180);
        const float s = sin(rad);
        transform3f m ;
        m.setIdentity();
        if ( ( dz == (0) ) || ( s == (0) ) || ( aspect == (0) ) )
        {
            return m;
        }

        const float cot = cos(rad) / s;


        m(0,0) = cot/aspect;
        m(1,1) = cot;
        m(2,2) = -(zFar + zNear)/dz;
        m(3,2) = -1;
        m(2,3) =  -2 * zNear * zFar / dz;
        m(3,3) = 0;

        //m[0]  = cot / aspect;
        //m[5]  = cot;
        //m[10] = -(zFar + zNear) / dz;
        //m[14] = (float)(-1);
        //m[11] = -2 * zNear * zFar / dz;
        //m[15] = (0);

        return m;
    }
    transform3f rotation(float angle, int x, int y, int z)
    {
        transform3f t;
        t.setIdentity();
        if (x == 1)
        {
            t.rotate(AngleAxis3f(angle,vector3f::UnitX()));
        }
        else if (y==1)
        {
            t.rotate(AngleAxis3f(angle,vector3f::UnitY()));
        }
        else if (z==1)
        {
            t.rotate(AngleAxis3f(angle,vector3f::UnitZ()));
        }
        return t;

    }
    transform3f lookat_matrix(
        const vector3f& eye_pos,
        const vector3f& center_pos,
        const vector3f& up_dir)
    {
        const vector3f forward = ((center_pos - eye_pos).normalized());//(center - eye);
        const vector3f side = ((forward.cross(up_dir)).normalized());//normalize(cross(forward, up));

        const vector3f up2 = side.cross(forward);//cross(side, forward);

        transform3f m ;//= transform3f::Identity();
        m.setIdentity();

        m(0,0) = side[0];
        m(0,1) = side[1];
        m(0,2) = side[2];

        m(1,0) = up2[0];
        m(1,1) = up2[1];
        m(1,2) = up2[2];

        m(2,0) = -forward[0];
        m(2,1) = -forward[1];
        m(2,2) = -forward[2];
        //m.elem[0][0] = side[0];
        //m.elem[0][1] = side[1];
        //m.elem[0][2] = side[2];

        //m.elem[1][0] = up2[0];
        //m.elem[1][1] = up2[1];
        //m.elem[1][2] = up2[2];

        //m.elem[2][0] = -forward[0];
        //m.elem[2][1] = -forward[1];
        //m.elem[2][2] = -forward[2];

        return m * translation3f(-eye_pos);
    }

    vector3f transform_point(const transform3f& m,const vector3f& pos)
    {
        return m*pos;
    }
    vector3f transform_vector(const transform3f& m,const vector3f& v)
    {
        return m.linear()*v;
    }

    vector3f vector_product(const vector3f& lhs,const vector3f& rhs)
    {
        return vector3f(lhs[0]*rhs[0],lhs[1]*rhs[1],lhs[2]*rhs[2]);
    }

    class Texture
    {
        char *tex;
        int width,height;
        const static int block_size = 16;
        void setup()
        {
            for (int j = 0;j < height;j++)
                for (int i=0;i<width ;i++)
                {
                    char* t = tex + 3*(j*width + i);
                    //black or white
                    //int bow = j*width + i;
                    if ( (j/block_size) & 1 && (i/block_size)&1)
                    {
                        *t =*(t+1) = *(t+2)=196;
                    }
                    else{
                        *t =*(t+1) = *(t+2)=0;
                    }
                }
        }
    public:
        Texture():width(256),height(256)
        {
            tex = new char[width * height * 3];
            setup();
        }

        ~Texture()
        {
            delete []tex;
        }

        unsigned int sample(float u, float v)
        {
			//u = clamp(u,0.f,1.f);
			//v = clamp(v,0.f,1.f);
            int tx = (int)( u* width );
            int ty = (int) (v * height);
            if (!(tx <= width && ty<= height && tx>=0 && ty>= 0))
                std::cerr<<tx<<" "<<ty<<std::endl;
			//assert(tx <= width && ty<= height && tx>=0 && ty>= 0);
			tx = clamp(tx,0,width);
			ty = clamp(ty,0,height);
            char* r = tex+ (tx + ty*width)*3 ;
            unsigned color = 0xff;
            color = color << 8;
            color |= *r;
            color = color << 8;
            color |= *(r+1);
            color = color << 8;
            color |= *(r+2);
            return color;
        }
    };

    template<typename GeometryRenderer>
    struct VertexShaderTex
    {
public:
        static const unsigned attribute_count = 1;
        static const unsigned persp_var_cnt = 2;

public:
        static transform3f viewprojection_matrix;
        static transform3f model_matrix;

        static vector3f material_color;
        static vector3f ambient_color;

        static struct Light
        {
            vector3f dir;
            vector3f color;
        } lights[2];

        static void shade(const typename GeometryRenderer::VertexInput in, typename GeometryRenderer::VertexOutput &out)
        {
            const TexVertex &in_vertex = *static_cast<const TexVertex*>(in[0]);

            vector3f wpos = transform_point(model_matrix, in_vertex.vertex);
            vector4f tvertex = viewprojection_matrix * vector4f(wpos[0],wpos[1],wpos[2], (1.0f));

            out.x = vector_op::x(tvertex);
            out.y = vector_op::y(tvertex) ;
            out.z = vector_op::z(tvertex) ;
            out.w = vector_op::w(tvertex) ;
            out.perspective_varyings[0] = in_vertex.tx;
            out.perspective_varyings[1] = in_vertex.ty;

            //perspective correction for attributes u and v
            //out.varyings[0]/=out.w;
            //out.varyings[1]/= out.w;

        }
    };
    template<typename GeometryRenderer>
    transform3f VertexShaderTex<GeometryRenderer>::viewprojection_matrix;
    template<typename GeometryRenderer>
    transform3f VertexShaderTex<GeometryRenderer>::model_matrix;
    template<typename GeometryRenderer>
    vector3f VertexShaderTex<GeometryRenderer>::material_color;
    template<typename GeometryRenderer>
    vector3f VertexShaderTex<GeometryRenderer>::ambient_color;

    template<typename GeometryRenderer>
    typename VertexShaderTex<GeometryRenderer>::Light VertexShaderTex<GeometryRenderer>::lights[2];



    //gauraud shading
    template<typename DriverType>
    class FragmentShaderTex : public GenericSpanDrawer<FragmentShaderTex<DriverType> >
    {
    public:
        typedef typename DriverType::ImagePtr ImagePtr;
        typedef typename DriverType::DepthBufferPtr DepthBufferPtr;
        typedef GenericSpanDrawer<FragmentShaderTex<DriverType> > Base;

        static const unsigned varying_count = 3;
        static const bool interpolate_z = false;
        static ImagePtr render_target;
        static DepthBufferPtr z_buffer;
        static  MARasterizerBase::Vertex* tri_v1;
        static  MARasterizerBase::Vertex* tri_v2;
        static  MARasterizerBase::Vertex* tri_v3;

        static Texture* tex;
        static float uv0[2],uv1[2],uv2[2];
        // per triangle callback
        static void begin_triangle(
            const MARasterizerBase::Vertex& v1,
            const MARasterizerBase::Vertex& v2,
            const MARasterizerBase::Vertex& v3)
        {
        }
        static void single_fragment(int x, int y,const MARasterizerBase::FragmentData& frag)
        {
            using namespace ma;
            using namespace details;
#ifdef LINEAR_Z_INTERPOLATE
            float* z_b = z_buffer->buffer() + y * z_buffer->width() + x;
#endif
            if (*z_b < frag.z)return;
            *z_b = frag.z;
             render_target->setPixel(x,y,tex->sample(frag.perspective_varyings[0],frag.perspective_varyings[1]));
             return;

        }
    };
    template<typename DriverType>
    typename DriverType::ImagePtr FragmentShaderTex<DriverType>::render_target = 0;
    template<typename DriverType>
    typename DriverType::DepthBufferPtr FragmentShaderTex<DriverType>::z_buffer = 0;
    template<typename DriverType>
     MARasterizerBase::Vertex* FragmentShaderTex<DriverType>::tri_v1 =0;
    template<typename DriverType>
     MARasterizerBase::Vertex* FragmentShaderTex<DriverType>::tri_v2 =0;
    template<typename DriverType>
     MARasterizerBase::Vertex* FragmentShaderTex<DriverType>::tri_v3 =0;
 template<typename DriverType>
    Texture* FragmentShaderTex<DriverType>::tex = 0;

     template<typename DriverType>
    float FragmentShaderTex<DriverType>::uv0[2];
     template<typename DriverType>
    float FragmentShaderTex<DriverType>::uv1[2];
         template<typename DriverType>
    float FragmentShaderTex<DriverType>::uv2[2];

    template<typename driver_ptr>
    inline void test_fun(driver_ptr d_ptr)
    {
        static perf::Timer timer;
        static int cnt=0;
        typedef typename boost::pointee<driver_ptr>::type Driver;
        typedef typename Driver::GeometryRenderer GeometryRenderer;
        typedef typename Driver::ImagePtr ImagePtr;
        typedef VertexShaderTex<GeometryRenderer> VertexShader;
        typedef FragmentShaderTex<Driver> FragmentShader;
        //set up matrices
        static float t = 5000.f;

        if (cnt == 0)
        timer.start();
        ++cnt;
        if (cnt == 30)
        { timer.end();timer.stop();cnt = 0;}


        transform3f projection_matrix = perspective_matrix((54.0f), (4.0f/3.0f), (0.1f), (50.0f));
        VertexShaderTex<GeometryRenderer>::model_matrix = rotation((-(t / 1000.0f) * 90), (0), (0), (1));
        VertexShaderTex<GeometryRenderer>::viewprojection_matrix = projection_matrix *
                lookat_matrix(
                    //vector3f(sin(((t * 2)/1000.0f))*3 +5, sin(((t * 1.5f)/1000.0f))*2 + 1, sin((t/1000.0f) +5)),
                    vector3f(0, sin(((t * 1.5f)/1000.0f))*3 + 4, sin(((t * 20)/1000.0f))*3 + 10),
                    vector3f((0.0f), (0.0f), (0.0f)),
                    vector3f((0.0f), (1.0f), (.0f)));
        t+=1.f;

        //check matrix
        ////std::cout<<"projection_matrix:  \n"<<projection_matrix.linear()<<std::endl;
        //std::cout<<"model_matrix:  \n"<<VertexShader<GeometryRenderer>::model_matrix.linear()<<std::endl;
        //std::cout<<"viewprojection_matrix:  \n"<<VertexShader<GeometryRenderer>::viewprojection_matrix.linear()<<std::endl;
        //scene setup
        VertexShaderTex<GeometryRenderer>::ambient_color = vector3f((0.3f),.3f,.3f);
        VertexShaderTex<GeometryRenderer>::material_color = vector3f(0.8f,.8f,.8f);
        VertexShaderTex<GeometryRenderer>::lights[0].dir = (vector3f((-4.0f), (2.0f), (-2.0f))).normalized();
        VertexShaderTex<GeometryRenderer>::lights[0].color = vector3f((0.5f),.5f,.5f);
        VertexShaderTex<GeometryRenderer>::lights[1].dir = (vector3f((5.0f), (5.0f), (1.0f))).normalized();
        VertexShaderTex<GeometryRenderer>::lights[1].color = vector3f((0.3f),.3f,.3f);
        //bind render target
        Texture tex;
        FragmentShaderTex<Driver>::render_target = d_ptr->getBackBuffer();
        FragmentShaderTex<Driver>::z_buffer = d_ptr->getDepthBuffer();
        FragmentShaderTex<Driver>::tex = &tex;
        assert( FragmentShaderTex<Driver>::render_target && FragmentShaderTex<Driver>::z_buffer);
        MARasterizerBase::Vertex::linear_var_cnt = 0;
        MARasterizerBase::Vertex::persp_var_cnt = 2;
        //d_ptr->template drawIndexTriangleBuffer<VertexShader<GeometryRenderer>,FragmentShader<Driver> >(sizeof(vertex_data)/sizeof(float),sizeof(index_data)/sizeof(unsigned),sizeof(float) * 6,vertex_data,index_data);
        d_ptr->template vertexBuffer<VertexShader>(0,sizeof(TexVertex),vertex_data);
        d_ptr->template drawIndexedTriangle<FragmentShader>(sizeof (index_data) / sizeof( unsigned ),index_data);
        //d_ptr->template drawIndexTriangleBuffer<VertexShaderTex<GeometryRenderer>,FragmentShaderTex<Driver> >
        //(sizeof(vertex_data)/sizeof(float),sizeof(index_data)/sizeof(unsigned),sizeof(TexVertex) ,vertex_data,index_data);
    }
}

#endif


#endif // TEXTURESHADER_TEST_HPP_INCLUDED
