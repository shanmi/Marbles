#ifndef __CHARACTER_VIEW_H__
#define __CHARACTER_VIEW_H__

#include "cocos2d.h"

class UiLayout;
class CharacterView 
	:public cocos2d::CCNode
{
public:
	CREATE_FUNC(CharacterView);
	~CharacterView(){}
	
	virtual bool init();
	/*virtual void onEnter();
	virtual void onExit();*/

public:
	void initCharacterLayout();
	void initCharacterLayout2();
	cocos2d::CCNode *getBodyById(int id);
	void checkPosition();
	void touchCallback(float degree);
	void characterMove(float offsetX);
	void checkShooterPos(cocos2d::CCPoint position);
	void initShooterPos();

private:
	CharacterView();

private:
	UiLayout *m_characterLayout;
	cocos2d::CCSprite *m_arrow;
};
#endif