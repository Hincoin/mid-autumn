#ifndef _C_ALGORITHM_H_
#define _C_ALGORITHM_H_

#define _PUSH_HEAP(_First, _Hole, _Top, _Val, _Pred)\
{ \
for (int _Push_heap_Idx = ((_Hole) - 1) / 2;\
(_Top) < (_Hole) && (_Pred( *((_First) + _Push_heap_Idx), (_Val)) );\
_Push_heap_Idx = ((_Hole) - 1) / 2)\
{ \
*((_First) + (_Hole)) = *((_First) + _Push_heap_Idx);\
(_Hole) = _Push_heap_Idx;\
}\
*((_First) + (_Hole)) = (_Val);\
}\


#define _PUSH_HEAP_0(_Type,_First, _Last, _Pred)\
{ \
int _Push_heap_0_Count = (_Last) - (_First);\
if (0 < _Push_heap_0_Count)\
    {\
        _Type _Push_heap_0_Val_t = (*(_Last));\
        int _Push_heap_Top = 0;\
        _PUSH_HEAP((_First), _Push_heap_0_Count, (_Push_heap_Top), (_Push_heap_0_Val_t), _Pred);\
    }\
}\


#define c_push_heap(_Type,_First, _Last, _Pred)\
{ \
if ((_First) != (_Last))\
    {--(_Last);_PUSH_HEAP_0(_Type,(_First), (_Last), _Pred);}\
}\

#define _ADJUST_HEAP(_First, _Hole, _Bottom,_Val, _Pred)\
{ \
int _Adjust_heap_Top = (_Hole);\
int _Adjust_heap_Idx = 2 * (_Hole) + 2;\
\
for (; _Adjust_heap_Idx < (_Bottom); _Adjust_heap_Idx = 2 * _Adjust_heap_Idx + 2)\
{ \
if ((_Pred ( *((_First) + _Adjust_heap_Idx), *((_First) + (_Adjust_heap_Idx - 1)))))\
--_Adjust_heap_Idx;\
*((_First) + (_Hole)) = *((_First) + _Adjust_heap_Idx), (_Hole) = _Adjust_heap_Idx;\
}\
\
if (_Adjust_heap_Idx == (_Bottom))\
{ \
*((_First) + (_Hole)) = *((_First) + ((_Bottom) - 1));\
(_Hole) = (_Bottom) - 1;\
}\
_PUSH_HEAP((_First), (_Hole), _Adjust_heap_Top, (_Val), _Pred);\
}\

#define _POP_HEAP(_First, _Last, _Dest,_Val, _Pred)\
{ \
 *(_Dest) = *(_First);\
    int _Pop_heap_Count = (_Last) - (_First);\
    int _Pop_heap_Hole = 0;\
_ADJUST_HEAP((_First), (_Pop_heap_Hole), (_Pop_heap_Count), (_Val), _Pred);\
}\

#define _POP_HEAP_0(_Type,_First, _Last, _Pred)\
{\
    _Type _Pop_heap_0_Val_t=(*((_Last) - 1));\
_POP_HEAP((_First), ((_Last) - 1), ((_Last) - 1),\
(_Pop_heap_0_Val_t), _Pred);\
}\

#define c_pop_heap(_Type,_First, _Last, _Pred)\
{ \
if (1 < (_Last) - (_First))\
_POP_HEAP_0(_Type,(_First), (_Last), _Pred);\
}\


#define _MAKE_HEAP(_Type,_First, _Last, _Pred)\
{\
int _Make_heap_Bottom = (_Last) - (_First);\
for (int _Make_heap_Hole = _Make_heap_Bottom / 2; 0 < _Make_heap_Hole; )\
{ \
--_Make_heap_Hole;\
    _Type _Make_heap_Val_t=(*((_First) + _Make_heap_Hole));\
_ADJUST_HEAP((_First), _Make_heap_Hole, _Make_heap_Bottom,(_Make_heap_Val_t), _Pred);\
}\
}\

#define c_make_heap(_Type, _First, _Last, _Pred)\
{\
if (1 < (_Last) - (_First))\
_MAKE_HEAP(_Type,(_First), (_Last), _Pred);\
}\




#define c_lower_bound(_F,_L,_V)\
{\
	int _Count = (_L) - (_F);\
	for(;0<_Count;)\
	{\
	int _Count2 = _Count / 2;\
	if(*((_F)+_Count2) < (_V))\
	(_F) = (_F)+_Count2+1,_Count -= _Count2 + 1;\
		else\
		_Count = _Count2;\
	}\
}\

#endif