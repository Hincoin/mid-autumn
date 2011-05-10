#ifndef _RENDERER_H_
#define _RENDERER_H_

class Scene;

class Renderer{
public:
	virtual ~Renderer(){}
	virtual void Render(const Scene *scene) = 0;
};

#endif