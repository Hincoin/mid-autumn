#include <math.h>   // smallppm, Progressive Photon Mapping by T. Hachisuka
#include <stdlib.h> // originally smallpt, a path tracer by Kevin Beason, 2008
#include <stdio.h>  // Usage: ./smallppm 100000 && xv image.ppm
#include <vector>
#define PI ((double)3.14159265358979) // ^^^^^^:number of photons emitted
#define ALPHA ((double)0.7) // the alpha parameter of PPM
int primes[61]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,
83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,
191,193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283};
inline int rev(const int i,const int p) {if (i==0) return i; else return p-i;}
double hal(const int b, int j) { 
// Halton sequence with reverse permutation
    if(b >= sizeof(primes)/sizeof(primes[0]) ) return ((double)rand()) / RAND_MAX; 
	const int p = primes[b]; 
//    if(j/p > 10) return ((double)rand()) / RAND_MAX;
    double h = 0.0, f = 1.0 / (double)p, fct = f;
	while (j > 0) {h += rev(j % p, p) * fct; j /= p; fct *= f;} return h;
}
struct Vec {double x, y, z; // vector: position, also color (r,g,b)
Vec(double x_ = 0, double y_ = 0, double z_ = 0) {x = x_; y = y_; z = z_;}
inline Vec operator+(const Vec &b) const {return Vec(x+b.x, y+b.y, z+b.z);}
inline Vec operator-(const Vec &b) const {return Vec(x-b.x, y-b.y, z-b.z);}
inline Vec operator+(double b) const {return Vec(x + b, y + b, z + b);}
inline Vec operator-(double b) const {return Vec(x - b, y - b, z - b);}
inline Vec operator*(double b) const {return Vec(x * b, y * b, z * b);}
inline Vec mul(const Vec &b) const {return Vec(x * b.x, y * b.y , z * b.z);}
inline Vec norm() {return (*this) * (1.0 / sqrt(x*x+y*y+z*z));}
inline double dot(const Vec &b) const {return x * b.x + y * b.y + z * b.z;}
Vec operator%(Vec& b) {return Vec(y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x);}//cross
};
#define MAX(x, y) ((x > y) ? x : y)

struct AABB {Vec min, max; // axis aligned bounding box
inline void fit(const Vec &p) {
	if (p.x<min.x)min.x=p.x; if (p.y<min.y)min.y=p.y; if (p.z<min.z)min.z=p.z;
	max.x=MAX(p.x, max.x); max.y=MAX(p.y, max.y); max.z=MAX(p.z, max.z);}

inline void reset() {min=Vec(1e20,1e20,1e20); max=Vec(-1e20,-1e20,-1e20);}
};
struct HPoint {
    HPoint()
    {
        r2 = -1.;//not init
    }
    Vec f,pos,nrm,flux,accum_flux; double r2; 
    unsigned int photon_count,accum_photon_count; 
    int pix;
    unsigned hit_count,accum_hit_count;
};
struct List {HPoint *id; List *next;};
List* ListAdd(HPoint *i,List* h){List* p=new List;p->id=i;p->next=h;return p;}

unsigned int num_hash, pixel_index, num_photon;
double hash_s; List **hash_grid; List *hitpoints = NULL; AABB hpbbox;
double max_photon_r2=0;
std::vector<HPoint*> hit_points_array;

inline unsigned int hash(const int ix, const int iy, const int iz) {
	return (unsigned int)((ix*73856093)^(iy*19349663)^(iz*83492791))%num_hash;
}

void clear_hash_grid()
{
	for (unsigned int i=0; i<num_hash;i++){
        List* lst = hash_grid[i];
        while(lst){List* elm = lst;lst=lst->next;delete elm;}
    }
	delete []hash_grid;
    hash_grid = NULL;
}
void build_hash_grid(const int w, const int h) {
	hpbbox.reset(); List *lst = hitpoints; while (lst != NULL) {
		HPoint *hp=lst->id; lst=lst->next; hpbbox.fit(hp->pos);}
	Vec ssize = hpbbox.max - hpbbox.min; // compute initial radius

    double irad = sqrt(max_photon_r2);
    if (irad == 0)
    	irad = ((ssize.x + ssize.y + ssize.z) / 3.0) / ((w + h) / 2.0) * 2.0;

	hpbbox.reset(); lst = hitpoints; int vphoton = 0; // determine hash size

	while (lst != NULL) {HPoint *hp = lst->id; lst = lst->next;
	hp->r2 = (max_photon_r2 == 0 || hp->r2 < 0) ? (irad * irad):hp->r2;
    hp->accum_photon_count = 0; hp->accum_flux = Vec();
	vphoton++; hpbbox.fit(hp->pos-irad); hpbbox.fit(hp->pos+irad);}
	hash_s=1.0/(irad*2.0); num_hash = vphoton; 

    hash_grid=new List*[num_hash];
	for (unsigned int i=0; i<num_hash;i++){
        hash_grid[i] = NULL;
    }
	lst = hitpoints; while (lst != NULL) { // store hitpoints in the hashed grid
		HPoint *hp = lst->id; lst = lst->next;
        irad = sqrt(hp->r2);
		Vec BMin = ((hp->pos - irad) - hpbbox.min) * hash_s;
		Vec BMax = ((hp->pos + irad) - hpbbox.min) * hash_s;
		for (int iz = abs(int(BMin.z)); iz <= abs(int(BMax.z)); iz++)
			for (int iy = abs(int(BMin.y)); iy <= abs(int(BMax.y)); iy++)
				for (int ix = abs(int(BMin.x)); ix <= abs(int(BMax.x)); ix++)
				{int hv=hash(ix,iy,iz); hash_grid[hv]=ListAdd(hp,hash_grid[hv]);}
		}
}
struct Ray {Vec o, d; Ray(){}; Ray(Vec o_, Vec d_) : o(o_), d(d_) {}};
enum Refl_t {DIFF, SPEC, REFR};  // material types, used in radiance()

struct Sphere {double rad; Vec p, c; Refl_t refl;
Sphere(double r_,Vec p_,Vec c_,Refl_t re_) : rad(r_),p(p_),c(c_),refl(re_){}
inline double intersect(const Ray &r) const { // returns distance
	Vec op=p-r.o; double t, b=op.dot(r.d), det=b*b-op.dot(op)+rad*rad;
	if (det < 0) return 1e20; else det = sqrt(det);
	return (t=b-det) > 1e-4 ? t : ((t=b+det)>1e-4 ? t : 1e20);}
};

	Sphere sph[] = { // Scene: radius, position, color, material
		Sphere(1e5, Vec( 1e5+1,40.8,81.6), Vec(.75,.25,.25),DIFF),//Left
		Sphere(1e5, Vec(-1e5+99,40.8,81.6),Vec(.25,.25,.75),DIFF),//Rght
		Sphere(1e5, Vec(50,40.8, 1e5),     Vec(.75,.75,.75),DIFF),//Back
		Sphere(1e5, Vec(50,40.8,-1e5+170), Vec(),           DIFF),//Frnt
		Sphere(1e5, Vec(50, 1e5, 81.6),    Vec(.75,.75,.75),DIFF),//Botm
		Sphere(1e5, Vec(50,-1e5+81.6,81.6),Vec(.75,.75,.75),DIFF),//Top
		Sphere(16.5,Vec(27,16.5,47),       Vec(1,1,1)*.999, SPEC),//Mirr
		Sphere(16.5,Vec(73,16.5,88),       Vec(1,1,1)*.999, REFR),//Glas
		Sphere(8.5, Vec(50,8.5,60),        Vec(1,1,1)*.999, DIFF)};//Mid

int toInt(double x){return int(pow(1-exp(-x),1/2.2)*255+.5);} //tone mapping

inline bool intersect(const Ray &r,double &t,int &id){//ray-sphere intrsect.
			int n = sizeof(sph) / sizeof(Sphere); double d, inf = 1e20; t = inf;
			for(int i=0;i<n;i++){d=sph[i].intersect(r);if(d<t){t=d;id=i;}}return t<inf;}

void genp(Ray* pr, Vec* f, int i) {
		*f = Vec(2500,2500,2500)*(PI*4.0);
		double p=2.*PI*hal(0,i),t=2.*acos(sqrt(1.-hal(1,i)));
		double st=sin(t);
		pr->d=Vec(cos(p)*st,cos(t),sin(p)*st);
		pr->o=Vec(50,60,85);
		}

        // m is true for ray,false for photon tracing
        // do path tracing when do photon tracing
        // trace both specular transmission and specular reflection but not diffuse surface for ray tracing
		void trace(const Ray &r,int dpt,bool m,const Vec &fl,const Vec &adj,int i) 
		{
			double t;int id; dpt++;
            if(!intersect(r,t,id)||(dpt>=20))return;

            int d3=dpt*3;

			const Sphere &obj = sph[id]; Vec x=r.o+r.d*t, n=(x-obj.p).norm(), f=obj.c;
			Vec nl=n.dot(r.d)<0?n:n*-1; double p=f.x>f.y&&f.x>f.z?f.x:f.y>f.z?f.y:f.z;
			if (obj.refl == DIFF) {
			
			double r1=2.*PI*hal(d3-1,i),r2=hal(d3+0,i);
			double r2s=sqrt(r2);Vec w=nl,u=((fabs(w.x)>.1?Vec(0,1):Vec(1))%w).norm();
			Vec v=w%u, d = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1-r2)).norm();
			if (m) {
                //very very slow !!!!!!!!!!!!!!!
                HPoint* hp= hit_points_array[pixel_index];
                bool found = hp != NULL;
                hp= found?hp:(new HPoint); hp->f=f.mul(adj); hp->pos=x;
			hp->nrm=n; hp->pix = pixel_index; 
            if(!found)
            {
                hitpoints = ListAdd(hp, hitpoints);
                hit_points_array[pixel_index] = hp;
            }
			} else {Vec hh = (x-hpbbox.min) * hash_s;
			int ix = abs(int(hh.x)), iy = abs(int(hh.y)), iz = abs(int(hh.z));
			// strictly speaking, we should use #pragma omp critical here
			//#pragma omp critical
			{
				List* hp = hash_grid[hash(ix, iy, iz)]; 
				while (hp != NULL) {
					HPoint *hitpoint = hp->id; hp = hp->next; Vec v = hitpoint->pos - x;
					if ((hitpoint->nrm.dot(n) > 1e-3) && (v.dot(v) <= hitpoint->r2)) {
						//double g = (hitpoint->n*ALPHA+ALPHA) / (hitpoint->n*ALPHA+1.0);
						//hitpoint->r2=hitpoint->r2*g; 
                        hitpoint->accum_photon_count++;
						hitpoint->accum_flux=(hitpoint->accum_flux+hitpoint->f.mul(fl)*(1./PI));// *(1./PI))*g;
                    }
				}
			}

			if (!m && hal(d3+1,i)<p)
                 trace(Ray(x,d),dpt,m,f.mul(fl)*(1./p),adj,i);
            }
			} else if (obj.refl == SPEC) {
				trace(Ray(x, r.d-n*2.0*n.dot(r.d)), dpt, m, f.mul(fl), f.mul(adj),i);
			} else {
			
			Ray lr(x,r.d-n*2.0*n.dot(r.d)); bool into = (n.dot(nl)>0.0);
			double nc = 1.0, nt=1.5, nnt = into?nc/nt:nt/nc, ddn = r.d.dot(nl), cos2t;
			if ((cos2t=1-nnt*nnt*(1-ddn*ddn))<0) return trace(lr,dpt,m,fl,adj,i);
			Vec td = (r.d*nnt - n*((into?1:-1)*(ddn*nnt+sqrt(cos2t)))).norm();
			double a=nt-nc, b=nt+nc, R0=a*a/(b*b), c = 1-(into?-ddn:td.dot(n));
			double Re=R0+(1-R0)*c*c*c*c*c,P=Re;Ray rr(x,td);Vec fa=f.mul(adj);
			if(m)
			{
				(hal(d3-1,i) < P)? trace(lr,dpt,m,fl,fa,i):trace(rr,dpt,m,fl,fa,i);
			}
			else {(hal(d3-1,i)<P)?trace(lr,dpt,m,fl,fa,i):trace(rr,dpt,m,fl,fa,i);}}
			
			}

		int main(int argc, char *argv[]) {
			int w=1024, h=768, samps = (argc==2) ? MAX(atoi(argv[1])/1000,1) : 1;
			Ray cam(Vec(50,48,295.6), Vec(0,-0.042612,-1).norm());
			Vec cx=Vec(w*.5135/h), cy=(cx%cam.d).norm()*.5135, *c=new Vec[w*h], vw;

            hit_points_array.resize(w*h,NULL);
            unsigned photon_pass = 0;
            unsigned ray_pass = 0;
            num_photon=samps; vw=Vec(1,1,1);
            unsigned total_photon = num_photon;
           while( (max_photon_r2 == 0. || max_photon_r2 > 1e-3f) ){

//#pragma omp parallel for schedule(dynamic, 256)
			for (int y=0; y<h; y++){
				fprintf(stderr, "\rHitPointPass %5.2f%%", 100.0*y/(h-1));
				for (int x=0; x<w; x++) {
					pixel_index = x + y * w;
                    int ray_idx = pixel_index + w*h*photon_pass;
					Vec d = cx * ((x + hal(0,ray_idx)) / w - 0.5) + cy * (-(y + hal(1,ray_idx)) / h + 0.5)+cam.d;
					trace(Ray(cam.o + d * 140, d.norm()), 2, true, Vec(), Vec(1, 1, 1),ray_idx);
					}
			}
            ray_pass++;
			fprintf(stderr,"\n"); build_hash_grid(w,h); 
//#pragma omp parallel for schedule(dynamic, 256)
			for( int i=0;i<num_photon;i++) {
                double p=100.*(i+1)/num_photon;
			   int m=1000*(i+photon_pass*num_photon); Ray r; Vec f;
                fprintf(stderr,"\rPhotonPass %5.2f%%, %d",p,m); 
			    for(int j=0;j<1000;j++){
                    genp(&r,&f,m+j); trace(r,0,0>1,f,vw,m+j);}
            }
			List* lst=hitpoints; 
            max_photon_r2 = 0;
            while (lst != NULL) {
                HPoint* hp=lst->id;lst=lst->next;
                unsigned pcount = hp->photon_count + hp->accum_photon_count;
                if(hp->accum_photon_count > 0)
                {
                    float g = ALPHA*pcount / (hp->photon_count * ALPHA + hp->accum_photon_count);
                    if(g>1.f)
                        fprintf(stderr,"g>1,g:%.6f,pcount:%d,photon_count:%d,accum_photon_count:%d\n",
                                g,pcount,hp->photon_count,hp->accum_photon_count);
                    hp->photon_count = pcount;
                    hp->r2=hp->r2*g;
                    hp->flux = (hp->flux + hp->accum_flux)*(g);

                    hp->accum_photon_count = 0;
                    hp->accum_flux = Vec();
                    hp->hit_count ++;
                    
                    max_photon_r2 = MAX(max_photon_r2,hp->r2);
                }

                if(pcount>0)
                {
                    const double k = 1.0/(PI * hp->r2 * total_photon*1000.0);
                    int i=hp->pix;
                    c[i] = hp->flux * k;
                }
                hp->f = Vec();
                //c[i]=c[i]+hp->flux*(1.0/(PI*hp->r2*total_photon*1000.0));
            }
    		FILE* f = fopen("image.ppm","w"); fprintf(f,"P3\n%d %d\n%d\n",w,h,255);
			for(int i = 0; i< w * h; i++) {
				fprintf(f,"%d %d %d ", toInt(c[i].x), toInt(c[i].y), toInt(c[i].z));
                c[i].x = c[i].y=c[i].z=0.f;
            }
            fprintf(stderr,"\nprogress %d,total_photon:%d,max_photon_radius2:%.6f\n",photon_pass++,total_photon*1000,max_photon_r2); 
            total_photon += num_photon;
            clear_hash_grid();
        }

		}