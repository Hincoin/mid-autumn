#ifndef _MATRIX4x4_H_
#define _MATRIX4x4_H_

//no type cast allowed: a = float(b); is not allowed

typedef float matrix4x4_t[16];

typedef struct  
{
	matrix4x4_t m,inv_m;
}transform_t;

#define matrix_init(m,_00,_01,_02,_03,_10,_11,_12,_13,_20,_21,_22,_23,_30,_31,_32,_33)\
					  {\
						m[0] = _00;m[1]=_01;m[2] = _02;m[3] = _03;\
						m[4] = _10;m[5]=_11;m[6] = _12;m[7] = _13;\
						m[8] = _20;m[9]=_21;m[10] = _22;m[11] = _23;\
					   m[12] = _30;m[13]=_31;m[14] = _32;m[15] = _33;\
					  }\

#define matrix_element(m,r,c) m[r*4+c]

#define matrix_assign(ml,mr) {for(size_t _i = 0;_i < sizeof(ml)/sizeof(ml[0]);++_i)ml[_i]=(mr)[_i]; }

//operation on self
#define matrix_identity(m) {for(size_t _i = 0;_i < sizeof(m)/sizeof(m[0]);++_i) m[_i] = 0;\
							matrix_element(m,0,0) = matrix_element(m,1,1) = matrix_element(m,2,2) = matrix_element(m,3,3) = 1.f;}

#define matrix_transpose(ml,mr)\
	matrix_init(ml,\
	matrix_element(mr,0,0),matrix_element(mr,1,0),matrix_element(mr,2,0),matrix_element(mr,3,0),\
	matrix_element(mr,0,1),matrix_element(mr,1,1),matrix_element(mr,2,1),matrix_element(mr,3,1),\
	matrix_element(mr,0,2),matrix_element(mr,1,2),matrix_element(mr,2,2),matrix_element(mr,3,2),\
	matrix_element(mr,0,3),matrix_element(mr,1,3),matrix_element(mr,2,3),matrix_element(mr,3,3))

#define matrix_inverse(minv,m)\
{\
	int indxc[4], indxr[4];\
	int ipiv[4] ;\
	ipiv[0]=ipiv[1] =ipiv[2]=ipiv[3]=0;\
	int is_singular_matrix = 0;\
	matrix_assign(minv,m);\
	for (int _i = 0; _i < 4; _i++) {\
		int irow = -1, icol = -1;\
		float big = 0.f;\
		for (int _j = 0; _j < 4 && !is_singular_matrix; _j++) {\
			if (ipiv[_j] != 1) {\
				for (int _k = 0; _k < 4 && !is_singular_matrix; _k++) {\
					if (ipiv[_k] == 0) {\
						if (fabs(matrix_element(minv,_j,_k)) >= big) {\
							big = (fabs(matrix_element(minv,_j,_k)));\
							irow = _j;\
							icol = _k;\
						}\
					}\
					else if (ipiv[_k] > 1)\
					{\
						is_singular_matrix = 1;\
						break;\
					}\
				}\
			}\
		}\
		if (is_singular_matrix)\
		{\
			break;\
		}\
		++ipiv[icol];\
		if (irow != icol) {\
			for (int _k = 0; _k < 4; ++_k)\
			{\
				float _t = matrix_element(minv,irow,_k);\
				matrix_element(minv,irow,_k) = matrix_element(minv,icol,_k);\
				matrix_element(minv,icol,_k) = _t;\
			}\
		}\
		indxr[_i] = irow;\
		indxc[_i] = icol;\
		if (matrix_element(minv,icol,icol) == 0.f)\
		{\
			is_singular_matrix = true;\
			break;\
		}\
		float pivinv = 1.f / matrix_element(minv,icol,icol);\
		matrix_element(minv,icol,icol) = 1.f;\
		for (int _j = 0; _j < 4; _j++)\
			matrix_element(minv,icol,_j) *= pivinv;\
		for (int _j = 0; _j < 4; _j++) {\
			if (_j != icol) {\
				float save = matrix_element(minv,_j,icol);\
				matrix_element(minv,_j,icol) = 0;\
				for (int _k = 0; _k < 4; _k++)\
					matrix_element(minv,_j,_k) -= matrix_element(minv,icol,_k)*save;\
			}\
		}\
	} \
	if(!is_singular_matrix)\
	for (int _j = 3; _j >= 0; _j--) {\
		if (indxr[_j] != indxc[_j]) {\
			for (int _k = 0; _k < 4; _k++)\
			{\
				float _t = matrix_element(minv,_k,indxr[_j]);\
				matrix_element(minv,_k,indxr[_j]) = matrix_element(minv,_k,indxc[_j]);\
				matrix_element(minv,_k,indxc[_j]) = _t;\
			}\
		}\
	}\
}\
//operations with result returned
#define matrix_multiply(ml,mr0,mr1)\
	for(int _i = 0;_i < 4; ++_i)\
		for(int _j = 0; _j < 4; ++_j)\
		{\
			matrix_element(ml,_i,_j) = matrix_element(mr0,_i,0) * matrix_element(mr1,0,_j)\
				+ matrix_element(mr0,_i,1) * matrix_element(mr1,1,_j) + matrix_element(mr0,_i,2) * matrix_element(mr1,2,_j)\
				+ matrix_element(mr0,_i,3) * matrix_element(mr1,3,_j);\
		}\

//operations change self

#define transform_init(t,_00,_01,_02,_03,_10,_11,_12,_13,_20,_21,_22,_23,_30,_31,_32,_33)\
	{ matrix_init(t.m,_00,_01,_02,_03,_10,_11,_12,_13,_20,_21,_22,_23,_30,_31,_32,_33);\
	  matrix_inverse(t.inv_m,t.m);\
	}\

#define transform_load_from_array(t,float_array)\
	{\
		matrix_assign((t).m,float_array);\
		matrix_assign((t).inv_m,float_array+(sizeof((t).m)/sizeof((t).m[0])))\
	}

#define transform_construct_from_matrix(t,mm) {matrix_assign(t.m,mm);matrix_inverse(t.inv_m,mm);}
#define transform_construct_from_matrix2(t,mm,invm) {matrix_assign(t.m,mm);matrix_assign(t.inv_m,invm);}
#define transform_assign(tl,tr) {matrix_assign((tl).m,(tr).m);matrix_assign((tl).inv_m,(tr).inv_m);}
#define transform_identity(t) {matrix_identity(t.m);matrix_identity(t.inv_m);}
#define transform_translate(t,tx,ty,tz ) \
	{	matrix_element(t.m,0,3) += tx;matrix_element(t.m,1,3) += ty;matrix_element(t.m,2,3) += tz;\
		matrix_element(t.inv_m,0,3) -= tx;matrix_element(t.inv_m,1,3) -= ty;matrix_element(t.inv_m,2,3) -= tz;\
	}

#define transform_scale(t,sx,sy,sz )\
	{	matrix_element(t.m,0,0) *= sx;matrix_element(t.m,1,1) *= sy;matrix_element(t.m,2,2) *= sz;\
		matrix_element(t.inv_m,0,0) /= sx;matrix_element(t.inv_m,1,1) /= sy;matrix_element(t.inv_m,2,2) /= sz;\
	}\

//todo
#define transform_rotate(t,angle,rx,ry,rz)
#define transform_rotate_x(t,angle) {}
#define transform_rotate_y(t,angle) {}
#define transform_rotate_z(t,angle) {}

//operations on concate
#define transform_concate(tl,tr0,tr1) {matrix_multiply(tl.m,tr0.m,tr1.m);matrix_multiply(tl.inv_m,tr1.inv_m,tr0.inv_m);}

#define matrix_point(pl,m,p)\
	{\
	(pl).x = matrix_element((m),0,0) * (p).x + matrix_element(m,0,1) * (p).y + matrix_element(m,0,2) * (p).z + matrix_element(m,0,3);\
	(pl).y = matrix_element((m),1,0) *  (p).x + matrix_element(m,1,1) * (p).y + matrix_element(m,1,2) * (p).z + matrix_element(m,1,3);\
	(pl).z = matrix_element((m),2,0) *  (p).x + matrix_element(m,2,1) * (p).y + matrix_element(m,2,2) * (p).z + matrix_element(m,2,3);\
	float w = matrix_element((m),3,0) * (p).x + matrix_element(m,3,1) * (p).y + matrix_element(m,3,2) * (p).z + matrix_element(m,3,3);\
	if(w != 1.) {pl.x/=w;pl.y/=w;pl.z/=w;}\
	}\



#define transform_point(pl,t,p)\
	matrix_point((pl),t.m,p)

#define inverse_transform_point(pl,t,p)\
	matrix_point((pl),t.inv_m,p)

#define matrix_vector(vl,m,vr)\
	{\
		vl.x = matrix_element(m,0,0) * vr.x + matrix_element(m,0,1) * vr.y + matrix_element(m,0,2) * vr.z ;\
		vl.y = matrix_element(m,1,0) * vr.x + matrix_element(m,1,1) * vr.y + matrix_element(m,1,2) * vr.z ;\
		vl.z = matrix_element(m,2,0) * vr.x + matrix_element(m,2,1) * vr.y + matrix_element(m,2,2) * vr.z ;\
	}\




#define transform_vector(vl,t,vr)\
	matrix_vector(vl,t.m, vr)

#define inverse_transform_vector(vl,t,vr)\
	matrix_vector(vl,t.inv_m,vr)

#define transform_normal(nl,t,nr)\
	{\
		(nl).x = matrix_element(t.inv_m,0,0) * (nr).x + matrix_element(t.inv_m,1,0) * (nr).y + matrix_element(t.inv_m,2,0) * (nr).z ;\
		(nl).y = matrix_element(t.inv_m,0,1) * (nr).x + matrix_element(t.inv_m,1,1) * (nr).y + matrix_element(t.inv_m,2,1) * (nr).z ;\
		(nl).z = matrix_element(t.inv_m,0,2) * (nr).x + matrix_element(t.inv_m,1,2) * (nr).y + matrix_element(t.inv_m,2,2) * (nr).z ;\
	}\

#define inverse_transform_normal(nl,t,nr)\
	{\
		nl.x = matrix_element(t.m,0,0) * nr.x + matrix_element(t.m,1,0) * nr.y + matrix_element(t.m,2,0) * nr.z ;\
		nl.y = matrix_element(t.m,0,1) * nr.x + matrix_element(t.m,1,1) * nr.y + matrix_element(t.m,2,1) * nr.z ;\
		nl.z = matrix_element(t.m,0,2) * nr.x + matrix_element(t.m,1,2) * nr.y + matrix_element(t.m,2,2) * nr.z ;\
	}\

#define transform_ray(rl,t,rr)\
{\
		transform_point((rl).o, t, (rr).o);\
		transform_vector((rl).d, t, (rr).d);\
		(rl).mint = (rr).mint;\
		(rl).maxt = (rr).maxt;\
}\


#define inverse_transform_ray(rl,t,rr)\
{\
		inverse_transform_point((rl).o, t, (rr).o);\
		inverse_transform_vector((rl).d, t, (rr).d);\
		(rl).mint = (rr).mint;\
		(rl).maxt = (rr).maxt;\
}\

//todo
/*
bool Transform::SwapsHandedness() const {
	float det = ((m->m[0][0] *
		(m->m[1][1] * m->m[2][2] -
		m->m[1][2] * m->m[2][1])) -
		(m->m[0][1] *
		(m->m[1][0] * m->m[2][2] -
		m->m[1][2] * m->m[2][0])) +
		(m->m[0][2] *
		(m->m[1][0] * m->m[2][1] -
		m->m[1][1] * m->m[2][0])));
	return det < 0.f;
}
*/


#endif