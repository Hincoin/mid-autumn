#ifndef __INCLUDE_SHADER_TEST_HPP__
#define __INCLUDE_SHADER_TEST_HPP__

#include "Vector.hpp"

#include <boost/pointee.hpp>

#include "MAFragmentProcessor.hpp"
#include "MAMath.hpp"
#include "test_data.hpp"

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
    // #define LINEAR_Z_INTERPOLATE
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
        static const unsigned varying_count = 3;

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
            out.varyings[0] = vector_op::x(color);
            out.varyings[1] = vector_op::y(color);
            out.varyings[2] = vector_op::z(color);
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


    //gauraud shading
    template<typename DriverType>
    class FragmentShader : public GenericSpanDrawer<FragmentShader<DriverType> >
    {
    public:
        typedef typename DriverType::ImagePtr ImagePtr;
        typedef typename DriverType::DepthBufferPtr DepthBufferPtr;

        static const unsigned varying_count = 3;
        static const bool interpolate_z = false;
        static ImagePtr render_target;
        static DepthBufferPtr z_buffer;
        static const MARasterizerBase::Vertex* tri_v1;
        static const MARasterizerBase::Vertex* tri_v2;
        static const MARasterizerBase::Vertex* tri_v3;
        // per triangle callback
        static void begin_triangle(
            const MARasterizerBase::Vertex& v1,
            const MARasterizerBase::Vertex& v2,
            const MARasterizerBase::Vertex& v3)
        {
            tri_v1 = &v1;
            tri_v2 = &v2;
            tri_v3 = &v3;
            //sort by y axis
            if (tri_v1->y < tri_v2->y) //v1 < v2
            {
                if (tri_v1->y < tri_v3->y)
                {
                    if (tri_v2->y < tri_v3->y);
                    else std::swap(tri_v2,tri_v3);
                }
                else//tri_v2->y > tri_v1->y > tri_v3->y
                {
                    std::swap(tri_v1,tri_v3);//tri_v1 be smallest
                    std::swap(tri_v2,tri_v3);//tri_v3 be biggest
                }
            }
            else if (tri_v1->y < tri_v3->y) // v2 < v1 < v3
            {
                std::swap(tri_v1,tri_v2);
            }
            else//v1 > v2, v1 > v3
            {
                if (tri_v2->y > tri_v3->y)//v1> v2 > v3
                {
                    std::swap(tri_v3,tri_v1);
                }
                else //v1 > v3 > v2
                {
                    std::swap(tri_v1,tri_v2);
                    std::swap(tri_v2,tri_v3);
                }

            }
            assert( tri_v1->y <= tri_v2->y && tri_v2->y <= tri_v3->y );
        }
        //true if culled out
        static bool z_cull(int x,int y,
                           const MARasterizerBase::Vertex& v1,
                           const MARasterizerBase::Vertex& v2,
                           const MARasterizerBase::Vertex& v3)
        {
            if ( v1.z < 0 && v2.z < 0 && v3.z < 0)return true;
            float* z_b = z_buffer->buffer() + y * z_buffer->width() + x;
            float area = (v2.x - v1.x) * (v3.y - v1.y) - (v2.y - v1.y) * (v3.x - v1.x) ;

            float u = (v1.x-x) * (v2.y-y) - (v2.x - x) * (v1.y - y);
            float v = (v2.x-x) * (v3.y-y) - (v2.y-y) * (v3.x-x);
            u /= area;
            v /= area;
            u = std::abs(u);
            v = std::abs(v);
            float w  = 1- u - v;
            float interpolated_z = u * v3.z + v * v1.z + w * v2.z;

            if (interpolated_z <0)
            {
                //std::cout<<interpolated_z<<std::endl;
                return true;
            }
            if (*z_b > interpolate_z)
            {
                *z_b = interpolated_z;
                return false;
            }
            return true;
        }
        static void single_fragment(int x, int y, const MARasterizerBase::FragmentData &fd)
        {
            using namespace ma;
#ifdef LINEAR_Z_INTERPOLATE
            float* z_b = z_buffer->buffer() + y * z_buffer->width() + x;
            float interpolated_z = 0;
#endif
            //simple z interpolate method: assume that no triangle intersect each other
            //interpolated_z = (tri_v1->z+tri_v2->z+tri_v3->z)/3.f;
            //if (interpolated_z >= *(z_b) )
            //    return;

            //*(z_b ) = interpolated_z;
            //if (interpolated_z < 0 )
            //{
            //    return;
            //}
#ifndef LINEAR_Z_INTERPOLATE
            if (z_cull(x,y,*tri_v1,*tri_v2,*tri_v3))return;
#endif
            //gauraud shading
            //in y axis
            if (y < tri_v2->y) // betwenn v1,v2
            {
                float t12 = linearStep(tri_v1->y,tri_v2->y,(float)y);
                float t13 = linearStep(tri_v1->y,tri_v3->y,(float)y);
#ifdef LINEAR_Z_INTERPOLATE
                float z12 = lerp(tri_v1->z,tri_v2->z,t12);
                float z13 = lerp(tri_v1->z,tri_v3->z,t13);
#endif


                float slopeR12 = lerp(tri_v1->varyings[0],tri_v2->varyings[0],t12);
                float slopeG12 = lerp(tri_v1->varyings[1],tri_v2->varyings[1],t12);
                float slopeB12 = lerp(tri_v1->varyings[2],tri_v2->varyings[2],t12);
                float posx12 = lerp(tri_v1->x,tri_v2->x,t12);



                float slopeR13 = lerp(tri_v1->varyings[0],tri_v3->varyings[0],t13);
                float slopeG13 = lerp(tri_v1->varyings[1],tri_v3->varyings[1],t13);
                float slopeB13 = lerp(tri_v1->varyings[2],tri_v3->varyings[2],t13);
                float posx13 = lerp(tri_v1->x,tri_v3->x,t13);


                if (posx12 < posx13)
                {
                    float tx = linearStep(posx12,posx13,(float)x);
#ifdef LINEAR_Z_INTERPOLATE
                    {
                        interpolated_z = lerp(z12,z13,tx);

                        if (interpolated_z>0 && *z_b > interpolated_z)
                            *(z_b) = interpolated_z;
                        else return;
                    }
#endif
                    unsigned r = (unsigned)(lerp(slopeR12,slopeR13,tx)*255.f);
                    unsigned g = (unsigned)(lerp(slopeG12,slopeG13,tx)*255.f);
                    unsigned b = (unsigned)(lerp(slopeB12,slopeB13,tx)*255.f);
                    unsigned color = 0xFF000000 | r << 16 | g << 8 | b;
                    render_target->setPixel(x,y,color);
                }
                else
                {
                    float tx = linearStep(posx13,posx12,(float)x);
#ifdef LINEAR_Z_INTERPOLATE
                    {
                        interpolated_z =lerp(z13,z12,tx);
                        if (interpolated_z>0 && *z_b > interpolated_z)
                            *(z_b) = interpolated_z;
                        else return;
                    }
#endif

                    unsigned r = (unsigned)(lerp(slopeR13,slopeR12,tx)*255.f);
                    unsigned g = (unsigned)(lerp(slopeG13,slopeG12,tx)*255.f);
                    unsigned b = (unsigned)(lerp(slopeB13,slopeB12,tx)*255.f);
                    unsigned color = 0xFF000000 | r << 16 | g << 8 | b;
                    render_target->setPixel(x,y,color);
                }
            }
            else
            {
                float t23 = linearStep(tri_v2->y,tri_v3->y,(float)y);
                float t13 = linearStep(tri_v1->y,tri_v3->y,(float)y);
#ifdef LINEAR_Z_INTERPOLATE
                float z23 = lerp(tri_v2->z,tri_v3->z, t23);
                float z13 = lerp(tri_v1->z,tri_v3->z, t13);
#endif
                float slopeR23 = lerp(tri_v2->varyings[0],tri_v3->varyings[0],t23);
                float slopeG23 = lerp(tri_v2->varyings[1],tri_v3->varyings[1],t23);
                float slopeB23 = lerp(tri_v2->varyings[2],tri_v3->varyings[2],t23);
                float posx23 = lerp(tri_v2->x,tri_v3->x,t23);


                float slopeR13 = lerp(tri_v1->varyings[0],tri_v3->varyings[0],t13);
                float slopeG13 = lerp(tri_v1->varyings[1],tri_v3->varyings[1],t13);
                float slopeB13 = lerp(tri_v1->varyings[2],tri_v3->varyings[2],t13);
                float posx13 = lerp(tri_v1->x,tri_v3->x,t13);

                if (posx23 < posx13)
                {

                    float tx = linearStep(posx23,posx13,(float)x);
#ifdef LINEAR_Z_INTERPOLATE
                    {
                        interpolated_z = lerp(z23,z13, tx);
                        if (interpolated_z>0 && *z_b > interpolated_z)
                            *(z_b) = interpolated_z;
                        else return;
                    }
#endif
                    unsigned r = (unsigned)(lerp(slopeR23,slopeR13,tx)*255.f);
                    unsigned g = (unsigned)(lerp(slopeG23,slopeG13,tx)*255.f);
                    unsigned b = (unsigned)(lerp(slopeB23,slopeB13,tx)*255.f);
                    unsigned color = 0xFF000000 | r << 16 | g << 8 | b;
                    render_target->setPixel(x,y,color);
                }
                else
                {
                    float tx = linearStep(posx13,posx23,(float)x);
#ifdef LINEAR_Z_INTERPOLATE
                    {
                        interpolated_z = lerp(z13,z23,tx);
                        if (interpolated_z>0 && *z_b > interpolated_z)
                            *(z_b) = interpolated_z;
                        else return;
                    }
#endif
                    unsigned r = (unsigned)(lerp(slopeR13,slopeR23,tx)*255.f);
                    unsigned g = (unsigned)(lerp(slopeG13,slopeG23,tx)*255.f);
                    unsigned b = (unsigned)(lerp(slopeB13,slopeB23,tx)*255.f);
                    unsigned color = 0xFF000000 | r << 16 | g << 8 | b;
                    render_target->setPixel(x,y,color);
                }
            }
        }
    };
    template<typename DriverType>
    typename DriverType::ImagePtr FragmentShader<DriverType>::render_target = 0;
    template<typename DriverType>
    typename DriverType::DepthBufferPtr FragmentShader<DriverType>::z_buffer = 0;
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
        //bind render target
        FragmentShader<Driver>::render_target = d_ptr->getBackBuffer();
        FragmentShader<Driver>::z_buffer = d_ptr->getDepthBuffer();

        assert( FragmentShader<Driver>::render_target && FragmentShader<Driver>::z_buffer);
        //d_ptr->template drawIndexTriangleBuffer<VertexShader<GeometryRenderer>,FragmentShader<Driver> >(sizeof(vertex_data)/sizeof(float),sizeof(index_data)/sizeof(unsigned),sizeof(float) * 6,vertex_data,index_data);

        d_ptr->template drawIndexTriangleBuffer<VertexShader<GeometryRenderer>,FragmentShader<Driver> >
        (sizeof(vertex_data)/sizeof(float),sizeof(index_data)/sizeof(unsigned),sizeof(float) * 6,vertex_data,index_data);
    }
}

#endif
