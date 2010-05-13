#ifndef MASTENCILFUNCTION_HPP_INCLUDED
#define MASTENCILFUNCTION_HPP_INCLUDED

namespace ma{
///----------------------------------------------------------------------------
/// <summary> Comparision functions </summary>
///----------------------------------------------------------------------------
enum eCompFunc
{
	/// <summary> Returns ( false ) </summary>
	CMP_NEVER,

	/// <summary> Returns ( arg1 &lt; arg2 ) </summary>
	CMP_LESS,

	/// <summary> Returns ( arg1 == arg2 ) </summary>
	CMP_EQUAL,

	/// <summary> Returns ( arg1 &lt;=  arg2 ) </summary>
	CMP_LESSEQUAL,

	/// <summary> Returns ( arg1 &gt;  arg2 ) </summary>
	CMP_GREATER,

	/// <summary> Returns ( arg1 !=  arg2 ) </summary>
    CMP_NOTEQUAL,

	/// <summary> Returns ( arg1 &gt;=  arg2 ) </summary>
    CMP_GREATEREQUAL,

	/// <summary> Returns ( true ) </summary>
    CMP_ALWAYS,

	_COMPFUNC_FORCEDWORD = 0x7FFFFFFF
};

template <int CMP>
struct MACompare;//{};
template <>
struct MACompare<CMP_NEVER>{
    template <typename T>
    bool operator()(T a,T b)const{return false;}
    };

    template <>
struct MACompare<CMP_LESS>{
    template <typename T>
    bool operator()(T a,T b)const{return a < b;}
    };

    template <>
struct MACompare<CMP_EQUAL>{
    template <typename T>
    bool operator()(T a,T b)const{return a == b;}
    };

    template <>
struct MACompare<CMP_LESSEQUAL>{
    template <typename T>
    bool operator()(T a,T b)const{return a <= b;}
    };

    template <>
struct MACompare<CMP_GREATER>{
    template <typename T>
    bool operator()(T a,T b)const{return a > b;}
    };
        template <>
struct MACompare<CMP_NOTEQUAL>{
    template <typename T>
    bool operator()(T a,T b)const{return a != b;}
    };
        template <>
struct MACompare<CMP_GREATEREQUAL>{
    template <typename T>
    bool operator()(T a,T b)const{return a >= b;}
    };
        template <>
struct MACompare<CMP_ALWAYS>{
    template <typename T>
    bool operator()(T a,T b)const{return true;}
    };
///----------------------------------------------------------------------------
/// <summary> Stencil operations </summary>
///----------------------------------------------------------------------------
enum eStencilOp
{
	/// <summary> Keeps the current value </summary>
    STENCILOP_KEEP,

	/// <summary> Resets the value to zero </summary>
    STENCILOP_ZERO,

	/// <summary> Replaces with reference value </summary>
    STENCILOP_REPLACE,

	/// <summary> Increments current value </summary>
    STENCILOP_INCR,

	/// <summary> Decrements current value </summary>
    STENCILOP_DECR,

	/// <summary> Inverse current value </summary>
    STENCILOP_INVERT,

	_STENCILOP_FORCEDWORD = 0x7FFFFFFF,
};
namespace details{

    template <int Op>
    struct StencilOp;
        template <>
    struct StencilOp<STENCILOP_KEEP>{
                template <typename U>
        void operator()(int ref,int mask,U* stencil)const{}
        };    template <>
    struct StencilOp<STENCILOP_ZERO>{
                template <typename U>
        void operator()(int ref,int mask,U* stencil)const{*stencil = U(0);}
        };
            template <>
    struct StencilOp<STENCILOP_REPLACE>{
                template <typename U>
        void operator()(int ref,int mask,U* stencil)const{*stencil = ref;}

        };
            template <>
    struct StencilOp<STENCILOP_INCR>{
                template <typename U>
        void operator()(int ref,int mask,U* stencil)const{ ++ *stencil;}
        };
            template<>
    struct StencilOp<STENCILOP_DECR>{
                template <typename U>
        void operator()(int ref,int mask,U* stencil)const{ -- *stencil;}
        };

            template<>
    struct StencilOp<STENCILOP_INVERT>{
                template <typename U>
        void operator()(int ref,int mask,U* stencil)const{ *stencil = ~ *stencil;}
        };
}
    template<int SF_Fun,int SPZF_Fun,int SPZP_Fun,int Cmp>
    struct StencilDepthTest // true if the pixel should be processed
    {
        template<typename U,typename T>
        bool operator()(U z,U* z_buffer,int ref,int mask,T* stencil)const
        {
            using namespace details;
            if (z_buffer)//stencil pass
            {// z test only
                if(z < *z_buffer) // z pass
                {
                StencilOp<SPZP_Fun>()(ref,mask,stencil);return true;}
                else // z fail
                {StencilOp<SPZF_Fun>()(ref,mask,stencil);return false;}
            }// stencil test only
            else
            {
                if (MACompare<Cmp>()(*stencil & mask, ref& mask)) //stencil pass
            {
                return true;
            }
            else // stencil fail
            {
                StencilOp<SF_Fun>()(ref,mask,stencil);
                return false;
            }
            }
        }

    };

}

#endif // MASTENCILFUNCTION_HPP_INCLUDED
