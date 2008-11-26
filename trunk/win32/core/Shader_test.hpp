#ifndef __INCLUDE_SHADER_TEST_HPP__
#define __INCLUDE_SHADER_TEST_HPP__

#include "Vector.hpp"

#include <boost/pointee.hpp>

#include "MAFragmentProcessor.hpp"

namespace ma_test{
	using namespace ma;

	float vertex_data[] = {
		0.707107f, 0.707107f, 0.000000f, 0.886394f, 0.237509f, 0.397360f,
		0.000000f, 0.000000f, 2.000000f, 0.886394f, 0.237509f, 0.397360f,
		0.965926f, -0.258819f, 0.000000f, 0.886394f, 0.237509f, 0.397360f,
		0.965926f, -0.258819f, 0.000000f, 0.648886f, -0.648885f, 0.397360f,
		0.000000f, 0.000000f, 2.000000f, 0.648886f, -0.648885f, 0.397360f,
		0.258819f, -0.965926f, 0.000000f, 0.648886f, -0.648885f, 0.397360f,
		0.258819f, -0.965926f, 0.000000f, -0.237508f, -0.886395f, 0.397360f,
		0.000000f, 0.000000f, 2.000000f, -0.237508f, -0.886395f, 0.397360f,
		-0.707106f, -0.707107f, 0.000000f, -0.237508f, -0.886395f, 0.397360f,
		-0.707106f, -0.707107f, 0.000000f, -0.886394f, -0.237509f, 0.397360f,
		0.000000f, 0.000000f, 2.000000f, -0.886394f, -0.237509f, 0.397360f,
		-0.965926f, 0.258818f, 0.000000f, -0.886394f, -0.237509f, 0.397360f,
		-0.965926f, 0.258818f, 0.000000f, -0.648886f, 0.648885f, 0.397360f,
		0.000000f, 0.000000f, 2.000000f, -0.648886f, 0.648885f, 0.397360f,
		-0.258820f, 0.965926f, 0.000000f, -0.648886f, 0.648885f, 0.397360f,
		0.000000f, 0.000000f, 2.000000f, 0.237508f, 0.886395f, 0.397360f,
		0.707107f, 0.707107f, 0.000000f, 0.237508f, 0.886395f, 0.397360f,
		-0.258820f, 0.965926f, 0.000000f, 0.237508f, 0.886395f, 0.397360f,
		0.000000f, 0.000000f, -2.000000f, 0.886394f, 0.237509f, -0.397360f,
		0.707107f, 0.707107f, 0.000000f, 0.886394f, 0.237509f, -0.397360f,
		0.965926f, -0.258819f, 0.000000f, 0.886394f, 0.237509f, -0.397360f,
		0.258819f, -0.965926f, 0.000000f, 0.648886f, -0.648885f, -0.397360f,
		0.000000f, 0.000000f, -2.000000f, 0.648886f, -0.648885f, -0.397360f,
		0.965926f, -0.258819f, 0.000000f, 0.648886f, -0.648885f, -0.397360f,
		-0.707106f, -0.707107f, 0.000000f, -0.237508f, -0.886395f, -0.397360f,
		0.000000f, 0.000000f, -2.000000f, -0.237508f, -0.886395f, -0.397360f,
		0.258819f, -0.965926f, 0.000000f, -0.237508f, -0.886395f, -0.397360f,
		-0.965926f, 0.258818f, 0.000000f, -0.886394f, -0.237509f, -0.397360f,
		0.000000f, 0.000000f, -2.000000f, -0.886394f, -0.237509f, -0.397360f,
		-0.707106f, -0.707107f, 0.000000f, -0.886394f, -0.237509f, -0.397360f,
		-0.258820f, 0.965926f, 0.000000f, -0.648886f, 0.648885f, -0.397360f,
		0.000000f, 0.000000f, -2.000000f, -0.648886f, 0.648885f, -0.397360f,
		-0.965926f, 0.258818f, 0.000000f, -0.648886f, 0.648885f, -0.397360f,
		0.707107f, 0.707107f, 0.000000f, 0.237508f, 0.886395f, -0.397360f,
		0.000000f, 0.000000f, -2.000000f, 0.237508f, 0.886395f, -0.397360f,
		-0.258820f, 0.965926f, 0.000000f, 0.237508f, 0.886395f, -0.397360f,
	};

	unsigned index_data[] = {
		0, 
		1, 
		2, 
		3, 
		4, 
		5, 
		6, 
		7, 
		8, 
		9, 
		10, 
		11, 
		12, 
		13, 
		14, 
		15, 
		16, 
		17, 
		18, 
		19, 
		20, 
		21, 
		22, 
		23, 
		24, 
		25, 
		26, 
		27, 
		28, 
		29, 
		30, 
		31, 
		32, 
		33, 
		34, 
		35, 
	};
	
	transform3f perspective_matrix(const float fovy, const float aspect, const float zNear, const float zFar)
	{
		const float dz = zFar - zNear;
		const float rad = fovy / float(2) * float(M_PI/180);
		const float s = sin(rad);
		transform3f m ;
		m.setIdentity();
		if ( ( dz == (0) ) || ( s == (0) ) || ( aspect == (0) ) ) {
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
	transform3f rotation(float angle, int x, int y, int z){
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
	template<typename GeometryRenderer>
	class VertexShader {
		struct InputVertex {
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

		static struct Light {
			vector3f dir;
			vector3f color;
		} lights[2];

		static void shade(const typename GeometryRenderer::VertexInput in, typename GeometryRenderer::VertexOutput &out)
		{
			const InputVertex &i = *static_cast<const InputVertex*>(in[0]);

			vector3f wpos = transform_point(model_matrix, i.vertex);
			vector3f wnor = transform_vector(model_matrix, i.normal);

			vector3f color = vector_product(ambient_color , material_color);
			for (int i = 0; i < 2; ++i) {
				/*fixed16_t*/float ndotl = std::max((0.0f), wnor.dot(-lights[i].dir) );
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


	template<typename ImagePtr>
	class FragmentShader : public GenericSpanDrawer<FragmentShader<ImagePtr> > {
	public:
		static const unsigned varying_count = 3;
		static const bool interpolate_z = false;
		static ImagePtr render_target;

		// per triangle callback
		static void begin_triangle(
			const MARasterizerBase::Vertex& v1, 
			const MARasterizerBase::Vertex& v2, 
			const MARasterizerBase::Vertex& v3,
			int area2)
		{}

		static void single_fragment(int x, int y, const MARasterizerBase::FragmentData &fd)
		{
			//unsigned short color_buffer;

			// unfortunaltely at the corners of the triangle the values
			// can go outside the valid range
			int r = std::min(std::max(0, fd.varyings[0]>>16), 255) >> 3;
			int g = std::min(std::max(0, fd.varyings[1]>>16), 255) >> 2;
			int b = std::min(std::max(0, fd.varyings[2]>>16), 255) >> 3;
			unsigned short color = r << 11 | g << 5 | b;
			render_target->setPixel(x,y,color);
		}
	};
	template<typename ImagePtr>
	ImagePtr FragmentShader<ImagePtr>::render_target = 0;

	template<typename driver_ptr>
	inline void test_fun(driver_ptr d_ptr)
	{
		typedef typename boost::pointee<driver_ptr>::type Driver;
		typedef Driver::GeometryRenderer GeometryRenderer;
		typedef Driver::ImagePtr ImagePtr;
		//set up matrices
		static float t = 0.f;
		
		transform3f projection_matrix = perspective_matrix((54.0f), (4.0f/3.0f), (0.1f), (50.0f));
		VertexShader<GeometryRenderer>::model_matrix = rotation((-(t / 1000.0f) * 90), (0), (0), (1));
		VertexShader<GeometryRenderer>::viewprojection_matrix = projection_matrix * 
			lookat_matrix(
			vector3f(sin(((t * 2)/1000.0f))*3, sin(((t * 1.5f)/1000.0f))*2+(4), sin((t/1000.0f))), 
			vector3f((0.0f), (0.0f), (0.0f)), 
			vector3f((0.0f), (0.0f), (1.0f)));
		t+=1.f;

		//check matrix
		//std::cout<<"projection_matrix:  \n"<<projection_matrix.linear()<<std::endl;
		std::cout<<"model_matrix:  \n"<<VertexShader<GeometryRenderer>::model_matrix.linear()<<std::endl;
		std::cout<<"viewprojection_matrix:  \n"<<VertexShader<GeometryRenderer>::viewprojection_matrix.linear()<<std::endl;
		//scene setup
		VertexShader<GeometryRenderer>::ambient_color = vector3f((0.3f),.3f,.3f);
		VertexShader<GeometryRenderer>::material_color = vector3f(0.8f,.8f,.8f);
		VertexShader<GeometryRenderer>::lights[0].dir = (vector3f((-4.0f), (2.0f), (-2.0f))).normalized();
		VertexShader<GeometryRenderer>::lights[0].color = vector3f((0.5f),.5f,.5f);
		VertexShader<GeometryRenderer>::lights[1].dir = (vector3f((5.0f), (5.0f), (1.0f))).normalized();
		VertexShader<GeometryRenderer>::lights[1].color = vector3f((0.3f),.3f,.3f);
		//bind render target
		FragmentShader<ImagePtr>::render_target = d_ptr->getBackBuffer();
		d_ptr->template drawIndexTriangleBuffer<VertexShader<GeometryRenderer>,FragmentShader<ImagePtr> >(sizeof(vertex_data)/sizeof(float),sizeof(index_data)/sizeof(unsigned),sizeof(float) * 6,vertex_data,index_data);
	}
}

#endif