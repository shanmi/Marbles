#ifndef __BOX2D_FACTORY_H__
#define __BOX2D_FACTORY_H__

#include "Box2D/Box2D.h"
#include "GLES-Render.h"
#include "ContactListener.h"
#include "cocos2d.h"

#define PTM_RATIO 32

class Box2dFactory
{
public:
	static Box2dFactory *getInstance();
	~Box2dFactory();

public:
	void initPhysics();
	void initBorders();
	void initBorders(float topLine, float bottomLine);
	b2World *getWorld(){ return m_world; }
	void debugDraw();

	b2Body *createCircle(cocos2d::CCNode *node);
	b2Body *createSquare(cocos2d::CCNode *node);
	b2Body *createTriangle(cocos2d::CCNode *node);

private:
	Box2dFactory();

private:
	b2World *m_world;
	GLESDebugDraw *m_debugDraw;
	ContactListener *m_contactListener;
};

#endif