#ifndef __INCLUDE_MA_GEOMETRY_RENDERER_HPP__
#define __INCLUDE_MA_GEOMETRY_RENDERER_HPP__

#include "MAVertexProcessor.hpp"
//#include "MARasterizer.hpp"
#include <boost/shared_ptr.hpp>

namespace ma{
	namespace details {
		template <typename T, int SIZE>
		class static_vector {
			T data_[SIZE];
			size_t size_;
		public:
			static_vector() : size_(0) {}
			static_vector(size_t s, const T& i) : size_(0)
			{ resize(s, i);	}

			size_t size() const
			{ return size_;	}

			void resize(size_t size)
			{ size_ = size; }

			void resize(size_t size, const T& i)
			{ while (size_ < size) data_[size_++] = i; }

			T& back()
			{ return data_[size_ - 1]; }

			const T& back() const
			{ const_cast<static_vector>(this)->back(); }

			void push_back(const T& a)
			{ data_[size_++] = a;}

			void clear()
			{ size_ = 0; }

			T& operator[] (size_t i)
			{ return data_[i]; }

			const T& operator[] (size_t i) const
			{ return const_cast<static_vector>(this)[i]; }
		};
	}
	//...
	//has a vertex_processor and fragment processor
	template<typename Rasterizer>
	class MAGeometryRenderer : private VertexProcessor<typename Rasterizer::Vertex,MAGeometryRenderer<Rasterizer> >
	{
		typedef VertexProcessor<typename Rasterizer::Vertex, MAGeometryRenderer> Base;
		friend class VertexProcessor<typename Rasterizer::Vertex, MAGeometryRenderer>;
	public:
		enum CullMode{
			CULL_NONE = 0,
			CULL_CCW,
			CULL_CW
		};
		// make these inherited types and constants public
		typedef typename Base::VertexInput VertexInput;
		typedef typename Base::VertexOutput VertexOutput;

		static const int MAX_ATTRIBUTES = Base::MAX_ATTRIBUTES;
		static const unsigned SKIP_FLAG = -1;

	public:
		MAGeometryRenderer(const boost::shared_ptr<Rasterizer>& rasterizer);

		boost::shared_ptr<Rasterizer>& rasterizer(){return rasterizer_;}
		// public interface

		// upper left is (0,0)
		void viewport(int x, int y, int w, int h);

		// the depth range to use. Normally from 0 to a value less than MAX_INT
		void depth_range(int n, int f);

		void vertex_attrib_pointer(int n, int stride, const void* buffer);

		// count gives the number of indices
		void draw_triangles(unsigned count,const unsigned *indices);
		void draw_lines(unsigned count,const unsigned *indices);
		void draw_points(unsigned count,const unsigned *indices);

		void cull_mode(CullMode m);

		template <typename VertexShader>
		void vertex_shader()
		{
			Base::template vertex_shader<VertexShader>();
			varying_count_ = &varying_count_template<VertexShader>;
		}
		template <typename FragShader>
		void fragment_shader()
		{
			rasterizer_->template fragment_shader<FragShader>();
		}
	private:
		template <typename VertexShader>
		static unsigned varying_count_template()
		{
			return VertexShader::varying_count;
		}
	private:
		void add_interp_vertex(float t, int out, int in);

		void pdiv_and_vt();

		void clip_triangles();
		void process_triangles();

		void clip_lines();
		void process_lines();

		void clip_points();
		void process_points();

	private:
		// interface inherited from the vertex processor
		void process_begin();
		VertexOutput* acquire_output_location();
		bool push_vertex_index(unsigned i);
		void process_end();

	private:
		// maximum number of triangles to accumulate before going on with
		// clipping and triangle setup
		static const unsigned MAX_TRIANGLES = 32;
		static const unsigned MAX_LINES = 32;
		static const unsigned MAX_POINTS = 32;

		static const unsigned MAX_VERTICES_INDICES =
			MAX_TRIANGLES * 3 +
			MAX_TRIANGLES * 12; // Clipping generates additional vertices

		enum DrawMode {
			DM_TRIANGLES,
			DM_LINES,
			DM_POINTS
		};

		DrawMode draw_mode_;

		details::static_vector<VertexOutput, MAX_VERTICES_INDICES> vertices_;
		details::static_vector<unsigned, MAX_VERTICES_INDICES> indices_;

		struct {
			int ox, oy; // origin x and y
			int px, py; // width and height divided by 2
		} viewport_;


		struct {
			//Ü³ËûÂè¸ö±ÆµÄ windef.h
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
			float near,far;
		} depth_range_;

		CullMode cull_mode_;

		// internal function pointer; returns the number of varyings to interpolate.
		unsigned (*varying_count_)();

		boost::shared_ptr<Rasterizer> rasterizer_;
	};
}
#include "MAGeometryRendererImpl.hpp"

#endif

