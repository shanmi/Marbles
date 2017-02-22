#ifndef __LUCKY_LAYER_H__
#define __LUCKY_LAYER_H__

#include "cocos2d.h"
#include "GameController.h"
class UiLayout;
class LuckyLayer
	:public cocos2d::CCLayer
	, public INotifyView
{
public:
	virtual bool init();
	static LuckyLayer *create();

	void onEnter();
	void onExit();
	bool ccTouchBegan(cocos2d::CCTouch *pTouch, cocos2d::CCEvent *pEvent);

	virtual void updateCoins();

public:
	void initLayout();
	void closePanel(cocos2d::CCObject *pSender);
	void startDraw(cocos2d::CCObject *pSender);

private:
	LuckyLayer();

private:
	UiLayout *m_mainLayout;
	UiLayout *m_itemLayout;
};

#endif