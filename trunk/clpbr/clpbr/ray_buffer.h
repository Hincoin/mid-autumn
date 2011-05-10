#ifndef _RAY_BUFFER_H_
#define _RAY_BUFFER_H_
#include <algorithm>
template<typename RayType>
class RayBuffer{
public:
	explicit RayBuffer(unsigned s):size_(0),buffer_size_(s)
	{
		rays_ = new RayType[buffer_size_];
	}
	RayBuffer(RayType* r,unsigned s,bool is_acquire_ptr)
	{
		if(is_acquire_ptr)	
		{
			rays_ = r;buffer_size_ = size_ = s;
		}
		else
		{
			buffer_size_ = size_ = s;
			rays_ = new RayType[buffer_size_];
			std::copy(r,r+s,rays_);
		}
	}
	~RayBuffer(){delete []rays_;}

	bool IsFull()const{return size_ == buffer_size_;}
	void Push(const RayType& r){rays_[size_++] = r;}
	RayType& Back(){return rays_[size_-1];}
    void DeleteWithoutOrder(unsigned idx){
        size_--;
        if(idx < size_)
            rays_[idx] = rays_[size_];
    }
    bool Empty()const{return size() == 0;}

	const RayType *rays()const{return rays_;}
	RayType *rays(){return rays_;}
	unsigned size()const{return size_;}
	
	const RayType& operator[](unsigned idx)const{return rays()[idx];};
	RayType& operator[](unsigned idx){return rays()[idx];};
protected:
	RayBuffer(const RayBuffer&){}
private:
	unsigned size_;
	unsigned buffer_size_;
	RayType *rays_;//RayType must be pod type
};
#endif
