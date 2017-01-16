#include "SoundMgr.h"
#include "SimpleAudioEngine.h"
#include "cocos2d.h"
#include "UserInfo.h"
USING_NS_CC;
using namespace CocosDenshion;

SoundMgr *SoundMgr::theMgr()
{
	static SoundMgr mgr;
	return &mgr;
}

void SoundMgr::init()
{
	m_bIsMute = UserInfo::getInstance()->isMute();

	auto audioMgr = SimpleAudioEngine::sharedEngine();
	//audioMgr->preloadBackgroundMusic(Background_Music);

	audioMgr->preloadEffect(Effect_Pop);
	audioMgr->preloadEffect(Effect_Pop1);
	audioMgr->preloadEffect(Effect_Pop2);
	audioMgr->preloadEffect(Effect_Pop3);
}

void SoundMgr::playBackground()
{
	//SimpleAudioEngine::sharedEngine()->playBackgroundMusic(Background_Music, true);
}

void SoundMgr::playEffect(const char *effect)
{
	if (!m_bIsMute)
	{
		SimpleAudioEngine::sharedEngine()->stopAllEffects();
		SimpleAudioEngine::sharedEngine()->playEffect(effect);
	}
}

void SoundMgr::setMute(bool isMute)
{
	if (isMute)
	{
		SimpleAudioEngine::sharedEngine()->pauseAllEffects();
		SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
	}
	else
	{
		SimpleAudioEngine::sharedEngine()->resumeAllEffects();
		SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
	}

	m_bIsMute = isMute;
	UserInfo::getInstance()->setMute(isMute);
}

bool SoundMgr::isMute()
{
	return m_bIsMute;
}