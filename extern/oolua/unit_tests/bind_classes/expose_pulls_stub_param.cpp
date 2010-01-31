#include "expose_pulls_stub_param.h"

EXPORT_OOLUA_NO_FUNCTIONS(Stub)

#ifndef _MSC_VER
#define  EXPORT_FUNC EXPORT_OOLUA_FUNCTIONS_7_NON_CONST
#else
#define  EXPORT_FUNC EXPORT_OOLUA_FUNCTIONS_6_NON_CONST
#endif

#ifndef _MSC_VER
EXPORT_FUNC(Pulls_stub
			,ref
			,ref_const
			,ref_ptr_const
			,ref_const_ptr_const
			,ptr
			,ptr_const
			,const_ptr_const
			)
#else
EXPORT_FUNC(Pulls_stub
			,ref
			,ref_const
			,ref_ptr_const
			,ref_const_ptr_const
			,ptr
			,ptr_const
			)
#endif


EXPORT_OOLUA_FUNCTIONS_0_CONST(Pulls_stub)
