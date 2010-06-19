
/*
    pbrt source code Copyright(c) 1998-2010 Matt Pharr and Greg Humphreys.

    This file is part of pbrt.

    pbrt is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.  Note that the text contents of
    the book "Physically Based Rendering" are *not* licensed under the
    GNU GPL.

    pbrt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

// perspective.cpp*
#include "camera.h"
#include "film.h"
#include "paramset.h"
// PerspectiveCamera Declarations
class PerspectiveCamera : public ProjectiveCamera {
public:
	// PerspectiveCamera Public Methods
	PerspectiveCamera(const Transform &world2cam,
		const float Screen[4], float hither, float yon,
		float sopen, float sclose,
		float lensr, float focald, float fov,
		Film *film);
	float GenerateRay(const Sample &sample, Ray *) const;

	virtual bool GetSamplePosition(const Point& p,const Vector& wi,
		float distance,	float* x,float *y)const;	
	virtual bool GetSample(const Point& p,Sample& s,float& factor)const
	{
		Normal normal = CameraToWorld(Normal(0,0,1));
		Vector direction(normal);
		Point pos = GetPosition();

		Vector v = p - pos;
		float distance = v.Length();
		v =	Normalize(v);
		const float cosi = Dot(v,direction);
		//if(/*cosi <= 0.f ||*/ (!isinf(distance) && (distance * cosi < ClipHither || distance * cosi > ClipYon)))
		if(cosi <=0 ||( !isinf(distance) && (distance < ClipHither || distance > ClipYon)))
			return false;
		Point pa(640,480,1);
		pa = RasterToCameraBidir(pa);
		pa = CameraToWorld(pa);
	//	fprintf(stderr,"w,h:%f,%f\t",pa.x,pa.y);
		if (LensRadius > 0.f)
		{
			/*
			   Point pFC(p + wi *(FocalDistance / cosi));
			   Vector wi0(pFC - pos);
			   Point p0(pos + wi0/Dot(wi0,direction));
			   WorldToRasterBidir(p0,&p0);
			 *x = p0.x;
			 *y = p0.y;
			 */
		}
		else
		{
			Point p0 = WorldToRasterBidir(p /*pos + wi / cosi*/);
			if(normal.z * p0.z<0.f)return false;
			s.imageX = p0.x;
			s.imageY = p0.y;	
			factor =(640*480)/fabs(pa.x * pa.y) * 1.f/(cosi*cosi*cosi*cosi);//1.f/(cosi*cosi*cosi * tanf(Radians(fov_)/2.f) * tanf(Radians(fov_)/2.f));
		}
		return true;

		
	}
	
private:
	Transform RasterToCameraBidir,WorldToRasterBidir;
	float fov_;
};
// PerspectiveCamera Method Definitions
PerspectiveCamera::
    PerspectiveCamera(const Transform &world2cam,
		const float Screen[4], float hither, float yon,
		float sopen, float sclose,
		float lensr, float focald,
		float fov, Film *f)
	: ProjectiveCamera(world2cam,
	    Perspective(fov, hither, yon),
		Screen, hither, yon, sopen, sclose,
		lensr, focald, f) {

		fov_=fov;
		RasterToCameraBidir = Perspective(fov,1.f,2.f).GetInverse() * RasterToScreen;
		WorldToRasterBidir = RasterToCameraBidir.GetInverse() * WorldToCamera;
}

float PerspectiveCamera::GenerateRay(const Sample &sample,
		Ray *ray) const {
	// Generate raster and camera samples
	Point Pras(sample.imageX, sample.imageY, 0);
	Point Pcamera;
	RasterToCamera(Pras, &Pcamera);
	ray->o = Pcamera;
	ray->d = Vector(Pcamera.x, Pcamera.y, Pcamera.z);
	// Set ray time value
	ray->time = Lerp(sample.time, ShutterOpen, ShutterClose);
	// Modify ray for depth of field
	if (LensRadius > 0.) {
		// Sample point on lens
		float lensU, lensV;
		ConcentricSampleDisk(sample.lensU, sample.lensV,
		                     &lensU, &lensV);
		lensU *= LensRadius;
		lensV *= LensRadius;
		// Compute point on plane of focus
		float ft = (FocalDistance - ClipHither) / ray->d.z;
		Point Pfocus = (*ray)(ft);
		// Update ray for effect of lens
		ray->o.x += lensU * (FocalDistance - ClipHither) / FocalDistance;
		ray->o.y += lensV * (FocalDistance - ClipHither) / FocalDistance;
		ray->d = Pfocus - ray->o;
	}
	ray->d = Normalize(ray->d);
	ray->mint = 0.;
	ray->maxt = (ClipYon - ClipHither) / ray->d.z;
	CameraToWorld(*ray, ray);
	return 1.f;
}
bool PerspectiveCamera::GetSamplePosition(const Point& p,const Vector& wi,float distance,
			float* x,float *y)const	
{
	Normal normal = CameraToWorld(Normal(0,0,1));
	Vector direction(normal);
	Point pos = GetPosition();
	const float cosi = Dot(wi,direction);
	//if(/*cosi <= 0.f ||*/ (!isinf(distance) && (distance * cosi < ClipHither || distance * cosi > ClipYon)))
	if(!isinf(distance) && (distance < ClipHither || distance > ClipYon))
		return false;
	if (LensRadius > 0.f)
	{
		/*
		Point pFC(p + wi *(FocalDistance / cosi));
		Vector wi0(pFC - pos);
		Point p0(pos + wi0/Dot(wi0,direction));
		WorldToRasterBidir(p0,&p0);
		*x = p0.x;
		*y = p0.y;
		*/
	}
	else
	{
		Point p0 = WorldToRasterBidir(p /*pos + wi / cosi*/);
		if(normal.z * p0.z<0.f)return false;
		*x = p0.x;
		*y = p0.y;	
	}
	return true;

}
extern "C" DLLEXPORT Camera *CreateCamera(const ParamSet &params,
		const Transform &world2cam, Film *film) {
	// Extract common camera parameters from _ParamSet_
	float hither = max(1e-4f, params.FindOneFloat("hither", 1e-3f));
	float yon = min(params.FindOneFloat("yon", 1e30f), 1e30f);
	float shutteropen = params.FindOneFloat("shutteropen", 0.f);
	float shutterclose = params.FindOneFloat("shutterclose", 1.f);
	float lensradius = params.FindOneFloat("lensradius", 0.f);
	float focaldistance = params.FindOneFloat("focaldistance", 1e30f);
	float frame = params.FindOneFloat("frameaspectratio",
		float(film->xResolution)/float(film->yResolution));
	float screen[4];
	if (frame > 1.f) {
		screen[0] = -frame;
		screen[1] =  frame;
		screen[2] = -1.f;
		screen[3] =  1.f;
	}
	else {
		screen[0] = -1.f;
		screen[1] =  1.f;
		screen[2] = -1.f / frame;
		screen[3] =  1.f / frame;
	}
	int swi;
	const float *sw = params.FindFloat("screenwindow", &swi);
	if (sw && swi == 4)
		memcpy(screen, sw, 4*sizeof(float));
	float fov = params.FindOneFloat("fov", 90.);
	return new PerspectiveCamera(world2cam, screen, hither, yon,
		shutteropen, shutterclose, lensradius, focaldistance,
		fov, film);
}
