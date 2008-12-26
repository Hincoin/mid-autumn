#ifndef STENCIL_BUFFER_TEST_HPP_INCLUDED
#define STENCIL_BUFFER_TEST_HPP_INCLUDED

#include "MAStencilFunction.hpp"
#include "Vector.hpp"

#include <boost/pointee.hpp>

#include "MAFragmentProcessor.hpp"
#include "MAMath.hpp"

#define NO_TEXTURE_BIG
#include "test_data.hpp"
namespace ma_test{

//blend , stencil, fog operations
//render state using finite state machine object or 4 kinds of shader operations
    //*
    //  Rasterizer
    //*
    //  Depth Stencil
    //*
    //  Blend
    //*
    //  Sampler

enum eRenderState
{
	/// <summary> Back buffer clear color </summary>
	STATE_FRAMECLEAR,

	/// <summary> Depth buffer clear color </summary>
	STATE_DEPTHCLEAR,

	/// <summary> Stencil buffer clear color </summary>
	STATE_STENCILCLEAR,

	/// <summary> Ambient color </summary>
	STATE_AMBIENT,

	/// <summary> Polygon fill mode </summary>
	STATE_FILLMODE,

	/// <summary> Enabled/Disable clipping stage </summary>
	STATE_CLIPPING,

	/// <summary> backface culling mode </summary>
	STATE_CULLMODE,

	/// <summary> Enable/Disable scissor test </summary>
	STATE_SCISSORTEST,

	/// <summary> Alpha test reference value </summary>
	STATE_ALPHAREF,

	/// <summary> Stencil test reference value </summary>
	STATE_STENCILREF,

	/// <summary> Stencil test read mask </summary>
	STATE_STENCILMASK,

	/// <summary> Stencil test write mask </summary>
	STATE_STENCILWRITEMASK,

	/// <summary> Fog color </summary>
	STATE_FOGCOLOR,

	/// <summary> Fog start range </summary>
	STATE_FOGSTART,

	/// <summary> Fog end range </summary>
	STATE_FOGEND,

	/// <summary> Fog density </summary>
	STATE_FOGDENSITY,

	/// <summary> Texture factor </summary>
	STATE_TEXTUREFACTOR,

	_RENDERSTATE_SIZE,
	_RENDERSTATE_FORCEDWORD = 0x7FFFFFFF
};


///----------------------------------------------------------------------------
/// <summary> Pixel blend factors </summary>
///----------------------------------------------------------------------------
enum eBlendFactor
{
	/// <summary> Zero </summary>
	BLEND_ZERO,

	/// <summary> One </summary>
	BLEND_ONE,

	/// <summary> Source color </summary>
	BLEND_SRCCOLOR,

	/// <summary> Source color inverse </summary>
	BLEND_INVSRCCOLOR,

	/// <summary> Destination color </summary>
	BLEND_DESTCOLOR,

	/// <summary> Destination color inverse </summary>
	BLEND_INVDESTCOLOR,

	/// <summary> Source color alpha </summary>
	BLEND_SRCALPHA,

	/// <summary> Source color alpha inverse </summary>
	BLEND_INVSRCALPHA,

	/// <summary> Destination color alpha </summary>
	BLEND_DESTALPHA,

	/// <summary> Destination color alpha inverse </summary>
	BLEND_INVDESTALPHA,

	/// <summary> Saturated source color alpha </summary>
	BLEND_SRCALPHASAT,

    _BLENDFACTOR_FORCEDWORD = 0x7FFFFFFF,
};


///----------------------------------------------------------------------------
/// <summary> Pixel blend operations </summary>
///----------------------------------------------------------------------------
enum eBlendOp
{
	/// <summary> Returns src + dst </summary>
	BLENDOP_ADD,

	/// <summary> Returns src - dst </summary>
	BLENDOP_SUBTRACT,

	/// <summary> Returns dst - src </summary>
	BLENDOP_REVSUBTRACT,

	/// <summary> Returns min( src, dst ) </summary>
	BLENDOP_MIN,

	/// <summary> Returns max( src, dst ) </summary>
	BLENDOP_MAX,

    _BLENDOP_FORCEDWORD = 0x7FFFFFFF,
};
///----------------------------------------------------------------------------
/// <summary> Vertex fog factor modes </summary>
///----------------------------------------------------------------------------
enum eFogMode
{
	/// <summary> Fog factor = 1.0f  </summary>
	FOG_NONE,

	/// <summary> Fog factor = ( end - eye ) / ( end - start ) </summary>
	FOG_LINEAR,

	/// <summary> Fog factor = Exp( -density * eye ) </summary>
	FOG_EXP,

	/// <summary> Fog factor = Exp( -density * density * eye * eye ) </summary>
	FOG_EXP2,

	_FOGMODE_FORCEDWORD = 0x7FFFFFFF
};

///----------------------------------------------------------------------------
/// <summary> Vertex fog factor </summary>
///----------------------------------------------------------------------------
struct FogFactor
{
	/// <summary> Fog start range  </summary>
	float start;

	/// <summary> Fog end range  </summary>
	float end;

	/// <summary> Fog density  </summary>
	float density;
};

///----------------------------------------------------------------------------
/// <summary> Buffers clear flags </summary>
///----------------------------------------------------------------------------
enum eClearFlag
{
	/// <summary> No buffer selected </summary>
	CLEAR_NONE			= 0,

	/// <summary> Back buffer selected </summary>
	CLEAR_BACKBUFFER	= 1,

	/// <summary> Depth buffer selected </summary>
	CLEAR_DEPTHBUFFER	= 2,

	/// <summary> Stencil buffer selected </summary>
	CLEAR_STENCILBUFFER = 4,

	_CLEARFLAG_FORCEDWORD = 0x7FFFFFFF
};



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
    #define LINEAR_Z_INTERPOLATE
    template<typename GeometryRenderer>
    class VertexShader
    {
        struct InputVertex
        {
            vector3f vertex;
            vector3f normal;
        };

    public:
        static const unsigned attribute_count = 1;
        //static const unsigned varying_count = 3;'
        static const unsigned persp_var_cnt = 0;
        static const unsigned linear_var_cnt = 3;

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
            const InputVertex &in_vertex = *static_cast<const InputVertex*>(in[0]);

            vector3f wpos = transform_point(model_matrix, in_vertex.vertex);
            vector3f wnor = transform_vector(model_matrix, in_vertex.normal);

            vector3f color = vector_product(ambient_color , material_color);
            for (int i = 0; i < 2; ++i)
            {
                /*fixed16_t*/
                float ndotl = std::max((0.0f), wnor.dot(-lights[i].dir) );
                color += lights[i].color * ndotl;
            }
            color =vector_product(color ,material_color) ;
            for (int i = 0; i < 3; ++i)
                color[i] = std::min((1.0f), color[i]);

            vector4f tvertex = viewprojection_matrix * vector4f(wpos[0],wpos[1],wpos[2], (1.0f));

            out.x = vector_op::x(tvertex);
            out.y = vector_op::y(tvertex) ;
            out.z = vector_op::z(tvertex) ;
            out.w = vector_op::w(tvertex) ;
            out.linear_varyings[0] = vector_op::x(color);
            out.linear_varyings[1] = vector_op::y(color);
            out.linear_varyings[2] = vector_op::z(color);
            //debug code
            if (out.x/out.w > 0.0001f )
            {
                //int a = 100;
                //std::cout<<a<<std::endl;
            }
        }
    };

    template<typename GeometryRenderer>
    transform3f VertexShader<GeometryRenderer>::viewprojection_matrix;
    template<typename GeometryRenderer>
    transform3f VertexShader<GeometryRenderer>::model_matrix;
    template<typename GeometryRenderer>
    vector3f VertexShader<GeometryRenderer>::material_color;
    template<typename GeometryRenderer>
    vector3f VertexShader<GeometryRenderer>::ambient_color;

    template<typename GeometryRenderer>
    typename VertexShader<GeometryRenderer>::Light VertexShader<GeometryRenderer>::lights[2];

        //template<typename U,typename T>
        //bool operator()(U z,U* z_buffer,int ref,int mask,T* stencil)
    template <typename StencilOperation>
    inline bool stencil_test(float z,float *z_buffer,int ref,int mask,char* stencil)
    {
        return StencilOperation()(z,z_buffer,ref,mask,stencil);
    }
    //gauraud shading
    template<typename DriverType>
    class FragmentShader : public GenericSpanDrawer<FragmentShader<DriverType> >
    {
    public:
    typedef GenericSpanDrawer<FragmentShader<DriverType> > Base;
        typedef typename DriverType::ImagePtr ImagePtr;
        typedef typename DriverType::DepthBufferPtr DepthBufferPtr;
        typedef typename DriverType::StencilBufferPtr StencilBufferPtr;
        typedef bool (*StencilFun)(float z,float *z_buffer,int ref,int mask,char* stencil);

        static StencilFun stencil_func;

        static int ref,mask;//stencil operation
        static const unsigned varying_count = 3;
        static const bool interpolate_z = false;
        static ImagePtr render_target;
        static DepthBufferPtr z_buffer;
        static StencilBufferPtr stencil_buffer;

        static const MARasterizerBase::Vertex* tri_v1;
        static const MARasterizerBase::Vertex* tri_v2;
        static const MARasterizerBase::Vertex* tri_v3;
        // per triangle callback
        static void begin_triangle(
            const MARasterizerBase::Vertex& v1,
            const MARasterizerBase::Vertex& v2,
            const MARasterizerBase::Vertex& v3)
        {
            Base::prepare_gauround_triangle
            (v1,v2,v3,tri_v1,tri_v2,tri_v3);
        }
         static void single_fragment(int x, int y,const MARasterizerBase::FragmentData& frag)
        {
            using namespace ma;
            //run stencil test first
            char* s_b = stencil_buffer->buffer()+y * stencil_buffer->width() + x;
            if(stencil_func && !(*stencil_func)(0,0,ref,mask,s_b))return;

            float* z_b = z_buffer->buffer() + y * z_buffer->width() + x;
            ;
            if(stencil_func)
            {
            StencilFunctor<float,char> sf(ref,mask,stencil_func);
            if(sf(frag.z,z_b,s_b))
             *z_b = frag.z;
             else return;
            }
            else
            {
                 //sf;
                if (StencilFunctor<ma::details::only_z_tag,ma::details::only_z_tag>()(frag.z,z_b,s_b))
                *z_b = frag.z;
                else return;
                //if(!Base::gauround_shading(x,y,*tri_v1,*tri_v2,*tri_v3,z_b,s_b,
                //StencilFunctor<ma::details::only_z_tag,ma::details::only_z_tag>(),frag)) return;
            }

            unsigned r = (unsigned)(frag.linear_varyings[0]*255.f);
            unsigned g = (unsigned)(frag.linear_varyings[1]*255.f);
            unsigned b = (unsigned)(frag.linear_varyings[2]*255.f);
            unsigned color = 0xFF000000 | r << 16 | g << 8 | b;
            render_target->setPixel(x,y,color);
        }
    };
    template<typename DriverType>
    typename DriverType::ImagePtr FragmentShader<DriverType>::render_target = 0;
    template<typename DriverType>
    typename DriverType::DepthBufferPtr FragmentShader<DriverType>::z_buffer = 0;
    template<typename DriverType>
    typename DriverType::StencilBufferPtr FragmentShader<DriverType>::stencil_buffer;
    template<typename DriverType>
    typename FragmentShader<DriverType>::StencilFun FragmentShader<DriverType>::stencil_func = 0;
    template<typename DriverType>
    int FragmentShader<DriverType>::ref;
    template<typename DriverType>
    int FragmentShader<DriverType>::mask;
    template<typename DriverType>
    const MARasterizerBase::Vertex* FragmentShader<DriverType>::tri_v1 =0;
    template<typename DriverType>
    const MARasterizerBase::Vertex* FragmentShader<DriverType>::tri_v2 =0;
    template<typename DriverType>
    const MARasterizerBase::Vertex* FragmentShader<DriverType>::tri_v3 =0;

    template<typename driver_ptr>
    inline void test_fun(driver_ptr d_ptr)
    {
        typedef typename boost::pointee<driver_ptr>::type Driver;
        typedef typename Driver::GeometryRenderer GeometryRenderer;
        typedef typename Driver::ImagePtr ImagePtr;
        typedef VertexShader<GeometryRenderer> VertexShader_;
        typedef FragmentShader<Driver> FragmentShader_;

        MARasterizerBase::Vertex::linear_var_cnt = VertexShader<GeometryRenderer>::linear_var_cnt;
        MARasterizerBase::Vertex::persp_var_cnt = VertexShader<GeometryRenderer>::persp_var_cnt;;
        //set up matrices
        static float t = 1000.f;

        transform3f projection_matrix = perspective_matrix((54.0f), (4.0f/3.0f), (0.1f), (50.0f));
        VertexShader<GeometryRenderer>::model_matrix = rotation((-(t / 1000.0f) * 90), (0), (0), (1));
        VertexShader<GeometryRenderer>::viewprojection_matrix = projection_matrix *
                lookat_matrix(
                    vector3f(sin(((t * 2)/1000.0f))*3 +3, sin(((t * 1.5f)/1000.0f))*2 + 1, sin((t/1000.0f) +1)),
                    //vector3f(sin(((t * 20)/1000.0f))*3 , sin(((t * 20)/1000.0f))*3, sin(((t * 20)/1000.0f))*3),
                    vector3f((0.0f), (0.0f), (0.0f)),
                    vector3f((0.0f), (1.0f), (.0f)));
        t+=1.f;

        //check matrix
        ////std::cout<<"projection_matrix:  \n"<<projection_matrix.linear()<<std::endl;
        //std::cout<<"model_matrix:  \n"<<VertexShader<GeometryRenderer>::model_matrix.linear()<<std::endl;
        //std::cout<<"viewprojection_matrix:  \n"<<VertexShader<GeometryRenderer>::viewprojection_matrix.linear()<<std::endl;
        //scene setup
        VertexShader<GeometryRenderer>::ambient_color = vector3f((0.3f),.3f,.3f);
        VertexShader<GeometryRenderer>::material_color = vector3f(0.8f,.8f,.8f);
        VertexShader<GeometryRenderer>::lights[0].dir = (vector3f((-4.0f), (2.0f), (-2.0f))).normalized();
        VertexShader<GeometryRenderer>::lights[0].color = vector3f((0.5f),.5f,.5f);
        VertexShader<GeometryRenderer>::lights[1].dir = (vector3f((5.0f), (5.0f), (1.0f))).normalized();
        VertexShader<GeometryRenderer>::lights[1].color = vector3f((0.3f),.3f,.3f);
        d_ptr->template vertexBuffer<VertexShader_>(0,sizeof(float)*6,vertex_data);
		//bind render target
        FragmentShader<Driver>::render_target = d_ptr->getBackBuffer();
        FragmentShader<Driver>::z_buffer = d_ptr->getDepthBuffer();
        FragmentShader<Driver>::stencil_buffer = d_ptr->getStencilBuffer();
        FragmentShader<Driver>::ref = 1;
        FragmentShader<Driver>::mask = 0xFFFF;
        assert( FragmentShader<Driver>::render_target && FragmentShader<Driver>::z_buffer &&
        FragmentShader<Driver>::stencil_buffer);
        /*
        FragmentShader<Driver>::stencil_func = & stencil_test<StencilDepthTest<STENCILOP_KEEP,STENCILOP_KEEP,STENCILOP_REPLACE,CMP_ALWAYS> >;

		d_ptr->template drawIndexedTriangle<FragmentShader<Driver> >
        (sizeof(index_data)/sizeof(unsigned),index_data);

        FragmentShader<Driver>::ref = 1;
        FragmentShader<Driver>::mask = 0xFFFF;
        FragmentShader<Driver>::stencil_func = & stencil_test<StencilDepthTest<STENCILOP_KEEP,STENCILOP_KEEP,STENCILOP_REPLACE,CMP_NOTEQUAL> >;
		d_ptr->template drawIndexedTriangle<FragmentShader<Driver> >
			(sizeof(index_data)/sizeof(unsigned),index_data);
			*/
        //disable stencil test
        FragmentShader<Driver>::stencil_func = 0;
		d_ptr->template drawIndexedTriangle<FragmentShader<Driver> >
			(sizeof(index_data)/sizeof(unsigned),index_data);
    }
}

#endif // STENCIL_BUFFER_TEST_HPP_INCLUDED
