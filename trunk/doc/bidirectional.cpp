
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

// bidirectional.cpp*
#include "pbrt.h"
#include "transport.h"
#include "scene.h"

#include "camera.h"
#include "film.h"

#include "mc.h"

// Bidirectional Local Declarations
struct BidirVertex;
class BidirIntegrator : public SurfaceIntegrator {
public:
	// BidirIntegrator Public Methods
	Spectrum Li(const Scene *scene, const RayDifferential &ray, const Sample *sample, float *alpha) const;
	void RequestSamples(Sample *sample, const Scene *scene);
private:
	// BidirIntegrator Private Methods
	int generatePath(const Scene *scene, const Ray &r, const Sample *sample,
		const int *bsdfOffset, const int *bsdfCompOffset,
		BidirVertex *vertices, int maxVerts) const;
	float weightPath(BidirVertex *eye,int iEye, int nEye, BidirVertex *light,int iLight, int nLight) const;
	Spectrum evalPath(const Scene *scene, BidirVertex *eye, int nEye,
	BidirVertex *light, int nLight) const;
	static float G(const BidirVertex &v0, const BidirVertex &v1);
	static bool visible(const Scene *scene, const Point &P0, const Point &P1);

	void computeWeight(BidirVertex *eye, int nEye,
	BidirVertex *light, int nLight)const;
	// BidirIntegrator Data
	#define MAX_VERTS 4
	#define MAX_WEIGHTS (MAX_VERTS+1)
	int eyeBSDFOffset[MAX_VERTS], eyeBSDFCompOffset[MAX_VERTS];
	int lightBSDFOffset[MAX_VERTS], lightBSDFCompOffset[MAX_VERTS];
	int directLightOffset[MAX_VERTS], directLightNumOffset[MAX_VERTS];
	int directBSDFOffset[MAX_VERTS], directBSDFCompOffset[MAX_VERTS];
	int lightNumOffset, lightPosOffset, lightDirOffset;
	mutable float weight[MAX_WEIGHTS][MAX_WEIGHTS];
};
struct BidirVertex {
	BidirVertex() { add_cumulative = cumulative=0.f;bsdfWeight = dAWeight = 0.; rrWeight = 1.;
		flags = BxDFType(0); bsdf = NULL; }
	BSDF *bsdf;
	Point p;
	Normal ng, ns;
	Vector wi, wo;
	float bsdfWeight, dAWeight, rrWeight;
	BxDFType flags;
	Spectrum cumulative;
	Spectrum add_cumulative;
};
// Bidirectional Method Definitions
void BidirIntegrator::RequestSamples(Sample *sample, const Scene *scene) {
	for (int i = 0; i < MAX_VERTS; ++i) {
		eyeBSDFOffset[i] = sample->Add2D(1);
		eyeBSDFCompOffset[i] = sample->Add1D(1);
		lightBSDFOffset[i] = sample->Add2D(1);
		lightBSDFCompOffset[i] = sample->Add1D(1);
		directLightOffset[i] = sample->Add2D(1);
		directLightNumOffset[i] = sample->Add1D(1);
		directBSDFOffset[i] = sample->Add2D(1);
		directBSDFCompOffset[i]  = sample->Add1D(1);
	}
	lightNumOffset = sample->Add1D(1);
	lightPosOffset = sample->Add2D(1);
	lightDirOffset = sample->Add2D(1);
}
static bool debug_pixel = false;
static FILE* fptr=NULL;
Spectrum BidirIntegrator::Li(const Scene *scene,
		const RayDifferential &ray,
		const Sample *sample, float *alpha) const {
	int pixelx=int(sample->imageX);
	int pixely=int(sample->imageY);
	if(abs(int(sample->imageX)-317) == 0 && abs(int(sample->imageY)- 73) == 0)
		debug_pixel = true;
	else
		debug_pixel = false;

	fptr = fopen(debug_pixel? "debug.txt":"no_debug.txt","a");
	Spectrum L(0.);
	// Generate eye and light sub-paths
	BidirVertex eyePath[MAX_VERTS], lightPath[MAX_VERTS];
	int nEye = generatePath(scene, ray, sample, eyeBSDFOffset,
		eyeBSDFCompOffset, eyePath, MAX_VERTS);
	if(debug_pixel)fprintf(fptr,"eye path length:%d\n",nEye);
	if (nEye == 0) {
		*alpha = 0.;

	//	fclose(fptr);
	//	return L;
	}
	else
		*alpha = 1;
	// Choose light for bidirectional path
	int lightNum = Floor2Int(sample->oneD[lightNumOffset][0] *
		scene->lights.size());
	lightNum = min(lightNum, (int)scene->lights.size() - 1);
	Light *light = scene->lights[lightNum];
	float lightWeight = float(scene->lights.size());
	// Sample ray from light source to start light path
	Ray lightRay;
	float lightPdf;
	float u[4];
	u[0] = sample->twoD[lightPosOffset][0];
	u[1] = sample->twoD[lightPosOffset][1];
	u[2] = sample->twoD[lightDirOffset][0];
	u[3] = sample->twoD[lightDirOffset][1];
	Spectrum Le = light->Sample_L(scene, u[0], u[1], u[2], u[3],
		&lightRay, &lightPdf);
	if (lightPdf == 0.) return 0.f;
	Le *= lightWeight / lightPdf;
	int nLight = generatePath(scene, lightRay, sample, lightBSDFOffset,
		lightBSDFCompOffset, lightPath, MAX_VERTS);
	// Connect bidirectional path prefixes and evaluate throughput

//	computeWeight(eyePath,nEye,lightPath,nLight);	
	Spectrum directWt(1.0);
	for (int i = 1; i <= nEye; ++i) {
		// Handle direct lighting for bidirectional integrator
		directWt /= eyePath[i-1].rrWeight; 
	Spectrum localLe = directWt *
		 UniformSampleOneLight(scene, eyePath[i-1].p, eyePath[i-1].ng, eyePath[i-1].wi,
			eyePath[i-1].bsdf, sample, directLightOffset[i-1], directLightNumOffset[i-1],
			directBSDFOffset[i-1], directBSDFCompOffset[i-1]);
		if(i>1)
			localLe *= eyePath[i-2].cumulative;
	/*	if(eyePath[i-1].bsdfWeight != 0.f)
			localLe *=eyePath[i-1].bsdf->f(eyePath[i-1].wi, eyePath[i-1].wo) *
				AbsDot(eyePath[i-1].wo, eyePath[i-1].ng)/eyePath[i-1].bsdfWeight;
				*/
		L +=	weightPath(eyePath, i,nEye, lightPath, 0,nLight)* (localLe + eyePath[i-1].add_cumulative);
		/*L += localLe   * eyePath[i-1].bsdf->f(eyePath[i-1].wi, eyePath[i-1].wo) *
			AbsDot(eyePath[i-1].wo, eyePath[i-1].ng) /
			eyePath[i-1].bsdfWeight*/;;

			if(debug_pixel)fprintf(fptr,"n EyePath:%d eye position :%d,illuminance:%f,cumulative:%f,localLe:%f,eyePath.bsdf->f:%f\n",nEye,i,L.y(),eyePath[i-1].cumulative.y(),localLe.y(),eyePath[i-1].bsdf->f(eyePath[i-1].wi, eyePath[i-1].wo).y() );
			
		/*directWt *= eyePath[i-1].bsdf->f(eyePath[i-1].wi, eyePath[i-1].wo) *
			AbsDot(eyePath[i-1].wo, eyePath[i-1].ng) /
			eyePath[i-1].bsdfWeight;*/
		for (int j = 1; j <= nLight; ++j)
		{

			if (debug_pixel)fprintf(fptr,"eye visible %d,%d,%d\n",i,j,visible(scene, eyePath[i-1].p, lightPath[j-1].p));
			L += Le * evalPath(scene, eyePath, i, lightPath, j) *
				weightPath(eyePath, i,nEye, lightPath, j,nLight);
		}
	}
	if(debug_pixel)
		fprintf(fptr,"add_cumulative : %f L: %f\n",eyePath[0].add_cumulative.y(),L.y());
	directWt=1.f;
	//do light tracing
	for (int i = 1; i <= nLight; ++i)
	{
		const BidirVertex& lv = lightPath[i-1];
		directWt /= lv.rrWeight;
		if(visible(scene,lv.p,scene->camera->GetPosition()))
		{
			Spectrum localLe = Le;	
			Point p = scene->camera->GetPosition();
			Vector wo = scene->camera->GetPosition() - lv.p;
			float lengthSquared = wo.LengthSquared();
			float distance = sqrtf(lengthSquared);
			wo /= distance;
			
			if (i > 1)
				localLe *= lightPath[i-2].cumulative;
			float x,y;
			if (!localLe.Black() && scene->camera->GetSamplePosition(lv.p,lv.wi,distance,&x,&y))
			{
				Sample s;
				//camera sample
				s.imageX=x;
				s.imageY=y;
				//lensU,lensV etc
				localLe*=weightPath(eyePath,0,nEye,lightPath,i,nLight);
				localLe *= directWt * lv.bsdf->f(lv.wi,wo)* AbsDot(wo,lv.ng)/ (lengthSquared );
				float a = 1.f;
				scene->camera->film->AddSample(s, ray,localLe , a);
			}
		}
	}
	fflush(fptr);
	fclose(fptr);
	fptr = NULL;
	return L;
}
void BidirIntegrator::computeWeight(BidirVertex *eye, int nEye,
	BidirVertex *light, int nLight)const
{
	for (int i = 0;i < MAX_WEIGHTS;++i)
		for (int j = 0;j < MAX_WEIGHTS;++j)
		{
			weight[i][j]=0.f;
		}
	for (int i = 1;i < nEye;++i)
		weight[i][0] = 1.f/nEye;
	for (int i = 1;i < nLight;++i)
		weight[0][i] = 1.f/nLight;
	for(int i = 1;i < nEye;++i)
	{
		for(int j = 1;j < nLight;++j)
		{
			int s = i;
			int t = j;
			float sum = 0.f;
			float ps=eye[s].bsdfWeight;
			bool has_specular = (eye[s].flags & BSDF_SPECULAR) != 0;
			for (int k = 1;k < s+t && !has_specular;++k)
			{
				float pk;
				if(k<=s) 
				{
					pk = eye[k].bsdfWeight;
					has_specular = (eye[k].flags & BSDF_SPECULAR) != 0;
				}
				else if(k == s )
				{
					if(k+1<nEye)
					{
						pk=eye[k+1].bsdfWeight;
						has_specular = (eye[k+1].flags & BSDF_SPECULAR) != 0;
					}
					else
					{
						pk = eye[k].bsdfWeight * light[t].bsdfWeight;
						has_specular = (eye[k].flags & BSDF_SPECULAR) != 0 ||
							(light[t].flags & BSDF_SPECULAR )!= 0;
					}
				}
				else
					pk = light[t].bsdfWeight;
				sum += (pk/ps)*(pk/ps);
			}
			if (sum <= 0.000001f && !has_specular)
				fprintf(stderr,"i,j: %d,%d",i,j);
			if(!has_specular)
				weight[i][j]=1/sum;
		}
	}
}
	
int BidirIntegrator::generatePath(const Scene *scene, const Ray &r,
		const Sample *sample, const int *bsdfOffset,
		const int *bsdfCompOffset,
		BidirVertex *vertices, int maxVerts) const {
	int nVerts = 0;
	RayDifferential ray(r.o, r.d);
	Spectrum cumulative(1.0f);// = scene->Transmittance(ray);
	//for eye
	bool specularBounce = false;
	Spectrum add_cumulative(0.f);
	while (nVerts < maxVerts) {
		// Find next vertex in path and initialize _vertices_
		Intersection isect;
		if (!scene->Intersect(ray, &isect))
			break;
		BidirVertex &v = vertices[nVerts];
		v.bsdf = isect.GetBSDF(ray); // do before Ns is set!
		v.p = isect.dg.p;
		v.ng = isect.dg.nn;
		v.ns = v.bsdf->dgShading.nn;
		v.wi = -ray.d;
		//eye path
		if(nVerts == 0 || specularBounce)
		{
			add_cumulative += cumulative * isect.Le(-ray.d);
			if(debug_pixel)
				fprintf(fptr,"generatePath:add_cumulative %f,wi:%f,%f,%f",add_cumulative.y(),v.wi.x,v.wi.y,v.wi.z);
		}
		if(nVerts > 0)
			cumulative *= scene->Transmittance(ray);
		v.add_cumulative = add_cumulative;
		//////////////////////////////
	
		// Possibly terminate bidirectional path sampling
		if (nVerts > 2) {
			float rrProb = .8f;
			if (RandomFloat() > rrProb)
				break;
			v.rrWeight =rrProb;// 1.f / rrProb;
		}
		++nVerts;
		// Initialize _ray_ for next segment of path
		float u1 = sample->twoD[bsdfOffset[nVerts-1]][0];
		float u2 = sample->twoD[bsdfOffset[nVerts-1]][1];
		float u3 = sample->oneD[bsdfCompOffset[nVerts-1]][0];
	
		cumulative *= v.bsdf->Sample_f(v.wi, &v.wo, u1, u2, u3,
			 &v.bsdfWeight, BSDF_ALL, &v.flags);

		if (cumulative.Black() && v.bsdfWeight == 0.f)
			break;
		cumulative *= AbsDot(v.wo,v.ng) / (v.bsdfWeight);
		v.cumulative = cumulative;
		specularBounce = (v.flags & BSDF_SPECULAR) != 0;
		ray = RayDifferential(v.p, v.wo);
	}
	// Initialize additional values in _vertices_
	for (int i = 0; i < nVerts-1; ++i)
		vertices[i].dAWeight = vertices[i].bsdfWeight *
			AbsDot(-vertices[i].wo, vertices[i+1].ng) /
			DistanceSquared(vertices[i].p, vertices[i+1].p);
	return nVerts;
}

float BidirIntegrator::weightPath(BidirVertex *eye, int iEye,int nEye,
		BidirVertex *light,int iLight, int nLight) const {
//	if(nEye > 0)return 0.f;
//	else return 1.f;
//	if(nEye > 0 && nLight > 0 )return 1.f;
//	else return 0.f;
//	return weight[iEye][iLight];
	return 1.f/float(iEye + iLight );
}
Spectrum BidirIntegrator::evalPath(const Scene *scene, BidirVertex *eye, int nEye,
		BidirVertex *light, int nLight) const {
	BidirVertex& ev=eye[nEye-1];
	BidirVertex& lv=light[nLight-1];

	if (!visible(scene, ev.p, lv.p))
		return 0.;
	
	Spectrum L(1.);
	if (nEye > 1)
		L *= eye[nEye-2].cumulative;
	if(L.Black())return L;
	if (nLight> 1)
		L *= light[nLight-2].cumulative;
	if(L.Black())return L;
	Vector w = lv.p - ev.p;
	if(debug_pixel && fptr)
	{
		fprintf(fptr,"evalPath luminance: %f\n",L.y());
	}
	Vector we = w;
	Vector wl = w;
	Spectrum ef = ev.bsdf->f(ev.wi,we);
	if(ef.Black())return 0.f;
	Spectrum lf = lv.bsdf->f(-wl,lv.wi);
	if(lf.Black())return 0.f;

	L *= G(ev,lv) * ef * lf/(ev.rrWeight * lv.rrWeight);
	if(debug_pixel && fptr)
	{
		fprintf(fptr,"evalPath luminance=evbsdf * g * lvbsdf: %f =%f * %f * %f \n",L.y(),ev.bsdf->f(ev.wi,w).y(),G(ev,lv),(lv.bsdf->f(-w,lv.wi)).y());
	}
	return L;
	for (int i = 0; i < nEye-1; ++i)
		L *= eye[i].bsdf->f(eye[i].wi, eye[i].wo) *
			AbsDot(eye[i].wo, eye[i].ng) /
			(eye[i].bsdfWeight * eye[i].rrWeight);
	//Vector w = light[nLight-1].p - eye[nEye-1].p;
	/*L *= eye[nEye-1].bsdf->f(eye[nEye-1].wi, w) *
		G(eye[nEye-1], light[nLight-1]) *
		light[nLight-1].bsdf->f(-w, light[nLight-1].wi) /
		(eye[nEye-1].rrWeight * light[nLight-1].rrWeight);
		*/
	for (int i = nLight-2; i >= 0; --i)
		L *= light[i].bsdf->f(light[i].wi, light[i].wo) *
			AbsDot(light[i].wo, light[i].ng) /
			(light[i].bsdfWeight * light[i].rrWeight);
			
	if (L.Black())
		return L;
/*	if (!visible(scene, eye[nEye-1].p, light[nLight-1].p))
		return 0.;
		*/
	return L;
	
}
float BidirIntegrator::G(const BidirVertex &v0, const BidirVertex &v1) {
	Vector w = Normalize(v1.p - v0.p);
	float dot2 = AbsDot(v0.ng, w) * AbsDot(v1.ng, -w);
	if (dot2 < FLT_EPSILON)return 0.f;
	float dist2 = DistanceSquared(v0.p, v1.p);
	if (dist2 < FLT_EPSILON) return 0.f;
	return dot2 / dist2;
}
bool BidirIntegrator::visible(const Scene *scene, const Point &P0,
		const Point &P1) {
	Ray ray(P0, P1-P0, RAY_EPSILON, 1.f - RAY_EPSILON);
	return !scene->IntersectP(ray);
}
extern "C" DLLEXPORT SurfaceIntegrator *CreateSurfaceIntegrator(const ParamSet &params) {
	return new BidirIntegrator;
}
