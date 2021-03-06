#include "GameShooterMode.h"
#include "UiLayout.h"
#include "GameUtil.h"
#include "Config.h"
#include "SquareModel.h"
#include "BallHintModel.h"
#include "Box2dFactory.h"
#include "MarbleNode.h"
#include "MarbleModel.h"
#include "ActionSequence.h"
#include "CCFunctionAction.h"
#include "UserInfo.h"
#include "MainMenu.h"
#include "PauseLayer.h"
#include "HelpLayer.h"
#include "MyPurchase.h"
#include "GameConfig.h"
#include "LibaoDialog.h"
#include "ShopSkinLayer.h"
#include "CharacterView.h"
#include "DataHelper.h"
#include "FuhuoLibao.h"
#include "BossView.h"
#include "SoundMgr.h"
#include "CrystalView.h"

USING_NS_CC;

void GameShooterMode::onEnter()
{
	CCLayer::onEnter();
	CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -1, true);
	GameController::getInstance()->addView(this);
}

void GameShooterMode::onExit()
{
	CCLayer::onExit();
	CCDirector::sharedDirector()->getTouchDispatcher()->removeDelegate(this);
	GameController::getInstance()->removeView(this);
}

void GameShooterMode::draw()
{
	CCLayer::draw();
	Box2dFactory::getInstance()->debugDraw();
}

void GameShooterMode::keyBackClicked()
{
	onPauseGame(NULL);
}

GameShooterMode::GameShooterMode()
: m_moveCounter(0)
, m_shootDegree(90)
, m_doubleAttactTime(0)
, m_freezingTime(0)
, m_shotgunsTime(0)
, m_addMarbleTime(0)
, m_bIsGameOver(false)
, m_bossView(NULL)
{

}

GameShooterMode::~GameShooterMode()
{

}

GameShooterMode *GameShooterMode::create()
{
	GameShooterMode *layer = new GameShooterMode;
	layer->init();
	layer->autorelease();
	return layer;
}

CCScene *GameShooterMode::scene()
{
	CCScene *scene = CCScene::create();
	GameShooterMode *layer = GameShooterMode::create();
	scene->addChild(layer);
	return scene;
}

bool GameShooterMode::init()
{
	if (!CCLayer::init())
	{
		return false;
	}
	GameController::getInstance()->setGameType(kGame_Shoot);

	setKeypadEnabled(true);
	setAccelerometerEnabled(true);
	Box2dFactory::getInstance()->initPhysics(true);
	DataHelper::getInstance()->loadShootGameInfo();

	auto winSize = CCDirector::sharedDirector()->getWinSize();
	m_mainLayout = UiLayout::create("layout/game_scene2.xml");
	m_mainLayout->setAnchorPoint(ccp(0.5f, 0.5f));
	m_mainLayout->setPosition(ccpMult(winSize, 0.5f));
	m_mainLayout->setMenuTouchPriority(kPriority_Game - 1);
	addChild(m_mainLayout, kZOrder_Main);
	initMainLayout();

	m_characterView = CharacterView::create();
	m_characterView->setScale(0.8f);
	addChild(m_characterView, kZOrder_Character);
	m_arrow = dynamic_cast<CCSprite *>(m_characterView->getBodyById(11));
	auto arrow = dynamic_cast<CCSprite *>(m_mainLayout->getChildById(14));
	arrow->setVisible(false);
	auto worldPos = m_mainLayout->convertToWorldSpace(arrow->getPosition());
	m_characterView->setPosition(winSize.width / 2, worldPos.y);

	m_topLayout = UiLayout::create("layout/game_top2.xml");
	m_topLayout->setPosition(ccp(0, winSize.height - m_topLayout->getContentSize().height));
	m_topLayout->setMenuTouchPriority(kPriority_Game - 1);
	addChild(m_topLayout, kZOrder_Layout);
	initTopLayout();

	m_bottomLayout = UiLayout::create("layout/game_bottom2.xml");
	m_bottomLayout->setPosition(ccp(0, 0));
	m_bottomLayout->setMenuTouchPriority(kPriority_Game - 1);
	addChild(m_bottomLayout, kZOrder_Layout);
	initBottomLayout();
	initGameLayout();

	initPhysicBorder();

	initMarbles();
	initSquares();
	addBoss();
	updateScore();

	scheduleUpdate();
	return true;
}

void GameShooterMode::initMainLayout()
{
	CCSprite *line_top = dynamic_cast<CCSprite*>(m_mainLayout->getChildById(8));
	line_top->setTag(kTag_Wall_Top);
	line_top->setVisible(false);
	auto worldPos = m_mainLayout->convertToWorldSpace(line_top->getPosition());
	m_topLinePos = worldPos.y;
	Box2dFactory::getInstance()->createSquare(line_top, true);

	CCSprite *line_bottom = dynamic_cast<CCSprite*>(m_mainLayout->getChildById(7));
	line_bottom->setTag(kTag_Wall);
	worldPos = line_bottom->convertToWorldSpace(CCPointZero);
	m_bottomLinePos = worldPos.y;
	Box2dFactory::getInstance()->createSquare(line_bottom, true);

	CCSprite *railway = dynamic_cast<CCSprite*>(m_mainLayout->getChildById(13));
	railway->setZOrder(railway->getZOrder() + 1);
	for (int i = 0; i < 3; i++)
	{
		CCSprite *crystal = dynamic_cast<CCSprite*>(m_mainLayout->getChildById(10 + i));
		CrystalView *view = CrystalView::create(i);
		view->setPosition(crystal->getPosition());
		m_mainLayout->addChild(view, crystal->getZOrder(), crystal->getTag());
		crystal->removeFromParent();
	}

}

void GameShooterMode::onPauseGame(CCObject *pSender)
{
	if (getChildByTag(kTag_Pause))
	{
		removeChildByTag(kTag_Pause);
		return;
	}
	if (getChildByTag(kTag_GameOver))
	{
		return;
	}
	saveGameData();
	PauseLayer *pauseLayer = PauseLayer::create();
	addChild(pauseLayer, KZOrder_PauseLayer, kTag_Pause);
	CCLog("m_world->GetBodyCount()======================%d", m_world->GetBodyCount());
}

void GameShooterMode::initTopLayout()
{
	CCMenuItem *pauseItem = dynamic_cast<CCMenuItem*>(m_topLayout->getChildById(6));
	pauseItem->setTarget(this, menu_selector(GameShooterMode::onPauseGame));

	CCMenuItem *helpBtn = dynamic_cast<CCMenuItem*>(m_topLayout->getChildById(9));
	helpBtn->setTarget(this, menu_selector(GameShooterMode::onHelpPanel));

	CCSprite *progress_bg = dynamic_cast<CCSprite*>(m_topLayout->getChildById(14));
	CCSprite *logo = dynamic_cast<CCSprite*>(m_topLayout->getChildById(15));
	CCSprite *target = dynamic_cast<CCSprite*>(m_topLayout->getChildById(16));
	CCSprite *arrow = dynamic_cast<CCSprite*>(m_topLayout->getChildById(18));
	CCLabelAtlas *targetLabel = dynamic_cast<CCLabelAtlas*>(m_topLayout->getChildById(17));
	CCSprite *bossLabel = dynamic_cast<CCSprite*>(m_topLayout->getChildById(19));

	UiLayout *layout = UiLayout::create("layout/game_top2.xml");
	CCSprite *start = dynamic_cast<CCSprite*>(layout->getChildById(15));
	float startPos = start->getPositionX();

	arrow->setZOrder(arrow->getZOrder() + 1);
	auto moveBy = CCMoveBy::create(1.1f, ccp(0, 10));
	auto sequence = CCSequence::create(moveBy, moveBy->reverse(), NULL);
	auto repeat = CCRepeatForever::create(sequence);
	arrow->runAction(repeat);

	int curLevel = SquareModel::theModel()->getCurrentScore();
	int targetLevel = GameController::getInstance()->getTargetLevel();
	int lastLevel = targetLevel <= 10 ? 0 : targetLevel / 2;
	float rate = (target->getPositionX() - startPos) / (targetLevel - lastLevel);
	float gotoPos = startPos + rate * (curLevel - lastLevel);
	logo->setPositionX(gotoPos);
	arrow->setPositionX(gotoPos);
	targetLabel->setString(GameUtil::intToString(targetLevel).c_str());

	float bossPos = startPos + rate * ((targetLevel - lastLevel) / 2);
	bossLabel->setPositionX(bossPos);
	updateBoss();

	float progressPos = 100 * gotoPos / layout->getContentSize().width;
	m_progressTimer = CCProgressTimer::create(CCSprite::create("game/youxijiemian_jindutiao_jindu.png"));
	m_progressTimer->setType(kCCProgressTimerTypeBar);
	m_progressTimer->setMidpoint(ccp(0, 1));
	m_progressTimer->setBarChangeRate(ccp(1, 0));
	m_progressTimer->setPercentage(progressPos);
	m_progressTimer->setPosition(ccp(progress_bg->getContentSize().width / 2, progress_bg->getContentSize().height / 2));
	progress_bg->addChild(m_progressTimer);

	auto topPanel = CCSprite::create("game/youxijiemian_jindutiao_biankuang.png");
	topPanel->setPosition(ccp(progress_bg->getContentSize().width / 2, progress_bg->getContentSize().height / 2));
	progress_bg->addChild(topPanel);

}

void GameShooterMode::initBottomLayout()
{
	CCMenuItem *doubleAttactBtn = dynamic_cast<CCMenuItem*>(m_bottomLayout->getChildById(5));
	doubleAttactBtn->setTarget(this, menu_selector(GameShooterMode::onDoubleAttact));

	CCMenuItem *clearScreenBtn = dynamic_cast<CCMenuItem*>(m_bottomLayout->getChildById(4));
	clearScreenBtn->setTarget(this, menu_selector(GameShooterMode::onClearScreen));

	CCMenuItem *freezingBtn = dynamic_cast<CCMenuItem*>(m_bottomLayout->getChildById(3));
	freezingBtn->setTarget(this, menu_selector(GameShooterMode::onFreezing));

	for (int i = kMarble_Faster; i <= kMarble_Bomb; i++)
	{
		CCMenuItem *ballBtn = dynamic_cast<CCMenuItem*>(m_bottomLayout->getChildById(i + 9));
		ballBtn->setTag(i);
		ballBtn->setTarget(this, menu_selector(GameShooterMode::onMarbleChange));
		CCSprite *lock = dynamic_cast<CCSprite*>(m_bottomLayout->getChildById(i + 13));
		lock->setZOrder(ballBtn->getZOrder() + 1);
		bool isUnlock = UserInfo::getInstance()->isUnlock(i);
		if (!isUnlock)
		{
			lock->setVisible(true);
			ballBtn->setColor(ccc3(60, 60, 60));
		}
		else
		{
			lock->setVisible(false);
			ballBtn->setColor(ccc3(255, 255, 255));
		}
	}

}

void GameShooterMode::onDoubleAttact(CCObject *pSender)
{
	int count = UserInfo::getInstance()->getPropsCount(kProp_DoubleAttact);
	int coinCount = UserInfo::getInstance()->getCoins();
	int doubleAttactCost = GameConfig::getInstance()->m_doubleAttactCost;
	if (coinCount < doubleAttactCost  && count <= 0)
	{
		// show pay point
		showLibaoDiaolg();
		return;
	}
	if (m_doubleAttactTime <= 0)
	{
		if (count > 0)
		{
			UserInfo::getInstance()->addPropsCount(kProp_DoubleAttact, -1);
		}
		else
		{
			UserInfo::getInstance()->addCoins(-doubleAttactCost);
		}
		updateCoins();
		m_doubleAttactTime = DOUBLE_ATTACT_TIME;

		GameController::getInstance()->setDoubleAttact();
		CCMenuItem *doubleAttactBtn = dynamic_cast<CCMenuItem*>(m_bottomLayout->getChildById(5));
		auto scaleby = CCScaleBy::create(0.5f, 1.2f);
		auto reverse = scaleby->reverse();
		auto sequence = CCSequence::create(scaleby, reverse, NULL);
		auto action = CCRepeatForever::create(sequence);
		doubleAttactBtn->runAction(action);
	}
	else
	{
		// show pay point
	}
}

void GameShooterMode::onClearScreen(CCObject *pSender)
{
	int count = UserInfo::getInstance()->getPropsCount(kProp_Clear);
	int coinCount = UserInfo::getInstance()->getCoins();
	int hammerCost = GameConfig::getInstance()->m_hammerCost;
	if (coinCount < hammerCost && count <= 0)
	{
		// show pay point
		showLibaoDiaolg();
		return;
	}
	if (count > 0)
	{
		UserInfo::getInstance()->addPropsCount(kProp_Clear, -1);
	}
	else
	{
		UserInfo::getInstance()->addCoins(-hammerCost);
	}
	updateCoins();
	SquareModel::theModel()->removeAllSquares();
}

void GameShooterMode::onFreezing(CCObject *pSender)
{
	bool isFreezing = SquareModel::theModel()->isFreezing();
	int count = UserInfo::getInstance()->getPropsCount(kProp_Freezing);
	int coinCount = UserInfo::getInstance()->getCoins();
	int freezingCost = GameConfig::getInstance()->m_freezingCost;
	if (coinCount < freezingCost  && count <= 0)
	{
		// show pay point
		showLibaoDiaolg();
		return;
	}
	if (!isFreezing)
	{
		if (count > 0)
		{
			UserInfo::getInstance()->addPropsCount(kProp_Freezing, -1);
		}
		else
		{
			UserInfo::getInstance()->addCoins(-freezingCost);
		}
		updateCoins();
		SquareModel::theModel()->setSquareFreezing(true);
	}
}

void GameShooterMode::onHelpPanel(CCObject *pSender)
{
	if (getChildByTag(kTag_Pause))
	{
		removeChildByTag(kTag_Pause);
		return;
	}
	HelpLayer *helpLayer = HelpLayer::create();
	addChild(helpLayer, KZOrder_PauseLayer, kTag_Pause);
}

void GameShooterMode::showLibaoDiaolg()
{
	int random = rand() % 2;
	int libaoType = PAY_TYPE_TIME_LIBAO;
	if (random == 0)
	{
		libaoType = PAY_TYPE_COIN_LIBAO;
	}
	LibaoDialog *dialog = LibaoDialog::create(libaoType);
	addChild(dialog, KZOrder_LibaoLayer);
}

void GameShooterMode::checkLibaoShow()
{
	int score = SquareModel::theModel()->getCurrentScore();
	int m_showLibaoLevel = GameConfig::getInstance()->m_showLibaoLevel;
	if (score % m_showLibaoLevel == 0)
	{
		showLibaoDiaolg();
	}
}

void GameShooterMode::initGameLayout()
{
	/*auto ballHints = BallHintModel::theModel()->createBallHints();
	for (auto iter = ballHints.begin(); iter != ballHints.end(); ++iter)
	{
	addChild(*iter, kZOrder_Layout);
	}*/
}

void GameShooterMode::initPhysicBorder()
{
	m_world = Box2dFactory::getInstance()->getWorld();
	Box2dFactory::getInstance()->initBorders(m_topLinePos, m_bottomLinePos);
}

void GameShooterMode::initMarbles()
{
	//addMarble(0);
}

void GameShooterMode::addMarble(float dt)
{
	auto attr = MarbleModel::theModel()->getMarbleAttr();
	m_addMarbleTime += dt;
	if (m_addMarbleTime < BALL_SPEED / attr.speed)
	{
		return;
	}
	m_addMarbleTime = 0;
	int shootCount = 1;
	if (m_shotgunsTime > 0)
	{
		shootCount = 3;
	}
	if (attr.skin == kMarble_Dispersed)
	{
		shootCount += 2;
	}
	for (int i = 0; i < shootCount; i++)
	{
		auto ball = MarbleModel::theModel()->createMarble();
		ball->setBody();
		ball->setPosition(ccp(m_characterView->getPositionX() - 5, m_bottomLinePos + ball->getContentSize().height / 2 + 4));
		addChild(ball);
		ball->setMovingState(true);

		if (shootCount == 1)
		{
			ball->shooterShoot(90);
		}
		else
		{
			ball->shooterShoot(75 + 15 * i);
		}
		ball->setVisible(true);
	}
	//auto actions = ActionSequence::create(this);
	//auto action1 = CCDelayTime::create(0.1f / attr.speed);
	////actions->addAction(action1);
	//auto callback = CCFunctionAction::create([=]()
	//{
	//	addMarble(0);
	//});
	////actions->addAction(callback);
	//actions->runActions();
}

void GameShooterMode::initSquares()
{
	auto squares = SquareModel::theModel()->loadSquareList();
	for (size_t i = 0; i < squares.size(); i++)
	{
		auto node = squares[i];
		if (node->getPositionY() <= 0)
		{
			Index index = node->getIndex();
			node->setPosition(ccp((node->getContentSize().width / 2 + SQUARE_SPACING) + index.x * (node->getContentSize().width + SQUARE_SPACING),
				m_bottomLinePos + (node->getContentSize().height + SQUARE_SPACING) * (8.5 - index.y)));
		}
		addChild(node, kZOrder_Square);
	}
	bool isGameOver = GameController::getInstance()->checkGameOver();
	if (isGameOver)
	{
		showGameOver();
	}
}

void GameShooterMode::addSquares()
{
	auto allsquares = SquareModel::theModel()->getSquares();
	for (auto iter = allsquares.begin(); iter != allsquares.end(); ++iter)
	{
		auto square = *iter;
		Index index = square->getIndex();
		square->setIndex(index.x, index.y + 1);
	}

	auto squares = SquareModel::theModel()->createSquareList();
	for (size_t i = 0; i < squares.size(); i++)
	{
		auto node = squares[i];
		Index index = node->getIndex();
		node->setIndex(index.x, -1);
		node->setPosition(ccp(node->getContentSize().width / 2 + SQUARE_SPACING + index.x * (node->getContentSize().width + SQUARE_SPACING),
			m_bottomLinePos + (node->getContentSize().height + SQUARE_SPACING) * 8.5));
		addChild(node, kZOrder_Square);
	}
	saveGameData();
	updateScore();
	updateProgress();
}

void GameShooterMode::update(float dt)
{
	bool isGamePause = GameController::getInstance()->isGamePause();
	if (m_bIsGameOver || isGamePause)
	{
		return;
	}
	int32 velocityIterations = 10;
	int32 positionIterations = 10;
	m_world->Step(dt, velocityIterations, positionIterations);
	m_world->ClearForces();

	addMarble(dt);
	MarbleModel::theModel()->updateMarbles();
	if (m_shotgunsTime > 0)
	{
		m_shotgunsTime -= dt;
	}

	if (m_doubleAttactTime > 0)
	{
		m_doubleAttactTime -= dt;
	}
	else if (m_doubleAttactTime < 0)
	{
		m_doubleAttactTime = 0;
		GameController::getInstance()->resetAttactRate();
		CCMenuItem *doubleAttactBtn = dynamic_cast<CCMenuItem*>(m_bottomLayout->getChildById(5));
		doubleAttactBtn->stopAllActions();
		doubleAttactBtn->setScale(0.7f);
	}

	bool isFreezing = SquareModel::theModel()->isFreezing();
	if (isFreezing)
	{
		m_freezingTime += dt;
		if (m_freezingTime > FREEZING_TIME)
		{
			m_freezingTime = 0;
			SquareModel::theModel()->setSquareFreezing(false);
		}
	}
	else
	{
		bool isCanMove = GameController::getInstance()->isCanMove();
		if (!isCanMove)
		{
			return;
		}
		auto squares = SquareModel::theModel()->getSquares();
		for (auto iter = squares.begin(); iter != squares.end(); ++iter)
		{
			auto square = *iter;
			square->setPosition(ccp(square->getPositionX(), square->getPositionY() - square->getSpeed()));
			if (square->getPositionY() - square->getContentSize().height / 2 < m_bottomLinePos)
			{
				if (square->canRemoveByProps())
				{
					defenseCrash(square);
				}
				else
				{
					square->setScore(0);
				}
			}
		}
		m_moveCounter++;
		if (m_moveCounter > BLOCK_DISTANCE)
		{
			m_moveCounter = 0;

			int curLevel = SquareModel::theModel()->getCurrentScore();
			int targetLevel = GameController::getInstance()->getTargetLevel();
			if (curLevel >= targetLevel)
			{
				auto remainSquares = SquareModel::theModel()->getRemainSqaure().size();
				if (remainSquares <= 0)
				{
					GameController::getInstance()->setTargetLevel(targetLevel*2);
					SquareModel::theModel()->setCurrentScore(curLevel);
					GameController::getInstance()->setBossType(kBoss_Max);
					//GameController::getInstance()->resetCrystalBloods();
					updateBoss();
				}
			}
			else
			{
				addSquares();
			}
		}
	}

	//check squares by not check tool
	GameController::getInstance()->checkSquares(true);
}

bool GameShooterMode::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent)
{
	if (m_bIsGameOver)
	{
		return false;
	}
	auto location = pTouch->getLocation();
	auto winSize = CCDirector::sharedDirector()->getWinSize();
	auto characterPos = m_characterView->getPosition();
	auto size = m_characterView->getContentSize();
	auto rect = CCRect(characterPos.x - size.width / 2, characterPos.y, size.width, size.height);
	if (!rect.containsPoint(location))
	{
		return false;
	}
	if (location.x < 32)
	{
		location.x = 32;
	}
	else if (location.x > winSize.width - 32)
	{
		location.x = winSize.width - 32;
	}
	m_characterView->setScale(0.82f);
	m_characterView->setPositionX(location.x);
	m_bIsTouching = true;
	return true;
}

void GameShooterMode::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent)
{
	if (m_bIsGameOver)
	{
		return;
	}
	auto location = pTouch->getLocation();
	auto previous = pTouch->getPreviousLocation();
	for (int i = 0; i < 2; i++)
	{
		auto wheelBig = dynamic_cast<CCSprite *>(m_characterView->getBodyById(7 + i));
		auto wheelSmall = dynamic_cast<CCSprite *>(m_characterView->getBodyById(10 + i));
		auto rotate1 = CCRotateBy::create(0.6f, (location.x - previous.x) * 60);
		auto rotate2 = CCRotateBy::create(0.6f, (previous.x - location.x) * 60);

		wheelBig->stopAllActions();
		wheelBig->runAction(rotate1);

		wheelSmall->stopAllActions();
		wheelSmall->runAction(rotate2);
	}

	auto winSize = CCDirector::sharedDirector()->getWinSize();
	auto size = m_characterView->getContentSize();
	if (location.x < 32)
	{
		location.x = 32;
	}
	else if (location.x > winSize.width - 32)
	{
		location.x = winSize.width - 32;
	}
	m_characterView->setPositionX(location.x);
	GameController::getInstance()->setShooterPos(location);
}

void GameShooterMode::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent)
{
	m_characterView->setScale(0.8f);
	m_bIsTouching = false;
}

void GameShooterMode::updateMarbles()
{
	// just show adding marble action
}

void GameShooterMode::updateCoins()
{
	int coinCount = UserInfo::getInstance()->getCoins();
	std::string countStr = GameUtil::intToString(coinCount);
	CCLabelAtlas *coinLabel = dynamic_cast<CCLabelAtlas*>(m_topLayout->getChildById(5));
	coinLabel->setString(countStr.c_str());
}

void GameShooterMode::addSquareNode(SquareNode *node)
{
	addChild(node);
}

void GameShooterMode::showGameOver()
{
	CCLog("is game over");
	SquareModel::theModel()->removeBelowSquares();
}

void GameShooterMode::useShotGunsEffect()
{
	m_shotgunsTime = SHOTGUNS_TIME;
}

void GameShooterMode::onMarbleChange(cocos2d::CCObject *pSender)
{
	bool isGamePause = GameController::getInstance()->isGamePause();
	if (m_bIsGameOver || isGamePause)
	{
		return;
	}
	int tag = ((CCMenuItem*)pSender)->getTag();

	bool isUnlock = UserInfo::getInstance()->isUnlock(tag);
	if (isUnlock)
	{
		updateMarbleType(tag);
	}
	else
	{
		ShopSkinLayer *skinLayer = ShopSkinLayer::create(tag);
		addChild(skinLayer, kZOrder_Shop, kTag_Shop);
	}
}

void GameShooterMode::notifyViews()
{
	updateCoins();
	saveGameData();
	m_bIsGameOver = false;
}

void GameShooterMode::defenseCrash(SquareNode *node)
{
	Index index = node->getIndex();
	int col = 0;
	if (index.x < 2)
	{
		col = 0;
	}
	else if (index.x < 5)
	{
		col = 1;
	}
	else
	{
		col = 2;
	}
	CrystalView *crystal = dynamic_cast<CrystalView*>(m_mainLayout->getChildById(10 + col));
	if (crystal->getBloodCount() <= 0)
	{
		if (!m_bIsGameOver)
		{
			//crystal->runDieEffect();
			FuhuoLibao *fuhuoLibao = FuhuoLibao::create();
			addChild(fuhuoLibao, KZOrder_GameOver, kTag_GameOver);
			m_bIsGameOver = true;
		}
	}
	else
	{
		node->setScore(0);
	}
	crystal->addBloodCount(-1);
}

void GameShooterMode::updateScore()
{
	int score = SquareModel::theModel()->getCurrentScore();
	std::string countStr = GameUtil::intToString(score);
	CCLabelAtlas *scoreLabel = dynamic_cast<CCLabelAtlas*>(m_topLayout->getChildById(3));
	scoreLabel->setString(countStr.c_str());
	auto action1 = GameUtil::getOnceScaleAction();
	scoreLabel->runAction(action1);

	bool showEffect = false;
	int bestScore = UserInfo::getInstance()->getShootBestScore();
	if (score > bestScore)
	{
		bestScore = score;
		UserInfo::getInstance()->setShootBestScore(score);
		showEffect = true;
		//MyPurchase::sharedPurchase()->successStage(curLevel);
	}
	std::string bestStr = GameUtil::intToString(bestScore);
	CCLabelAtlas *bestScoreLabel = dynamic_cast<CCLabelAtlas*>(m_topLayout->getChildById(13));
	bestScoreLabel->setString(bestStr.c_str());
	if (showEffect)
	{
		auto action2 = GameUtil::getOnceScaleAction();
		bestScoreLabel->runAction(action2);
	}
}

void GameShooterMode::updateProgress()
{
	UiLayout *layout = UiLayout::create("layout/game_top2.xml");
	CCSprite *start = dynamic_cast<CCSprite*>(layout->getChildById(15));
	float startPos = start->getPositionX();

	CCSprite *progress_bg = dynamic_cast<CCSprite*>(m_topLayout->getChildById(14));
	CCSprite *logo = dynamic_cast<CCSprite*>(m_topLayout->getChildById(15));
	CCSprite *target = dynamic_cast<CCSprite*>(m_topLayout->getChildById(16));
	CCLabelAtlas *targetLabel = dynamic_cast<CCLabelAtlas*>(m_topLayout->getChildById(17));
	CCSprite *arrow = dynamic_cast<CCSprite*>(m_topLayout->getChildById(18));

	int curLevel = SquareModel::theModel()->getCurrentScore();
	int targetLevel = GameController::getInstance()->getTargetLevel();
	int lastLevel = targetLevel <= 10 ? 0 : targetLevel / 2;
	int bossLevel = lastLevel + (targetLevel - lastLevel) / 2;
	float gotoPos;
	if (curLevel == bossLevel)
	{
		int bloodCount = targetLevel > 60 ? targetLevel : 60;
		GameController::getInstance()->setBossBloodCount(bloodCount);
		addBoss();
	}
	if (targetLevel == lastLevel)
	{
		gotoPos = target->getPositionX();
		/*LuckyLayer *luckyLayer = LuckyLayer::create();
		addChild(luckyLayer, KZOrder_LuckyLayer);*/
	}
	else
	{
		float rate = (target->getPositionX() - startPos) / (targetLevel - lastLevel);
		gotoPos = startPos + rate * (curLevel - lastLevel);
	}

	auto moveTo = CCMoveTo::create(0.2f, ccp(gotoPos, logo->getPositionY()));
	logo->runAction(moveTo);
	moveTo = CCMoveTo::create(0.2f, ccp(gotoPos, arrow->getPositionY()));
	arrow->runAction(moveTo);

	float percentage = 100 * gotoPos / layout->getContentSize().width;
	float curPercentage = m_progressTimer->getPercentage();
	m_progressTimer->runAction(CCProgressFromTo::create(0.2f, curPercentage, percentage));

	targetLabel->setString(GameUtil::intToString(targetLevel).c_str());
}

void GameShooterMode::updateMarbleType(int type)
{
	auto attr = MarbleModel::theModel()->getMarbleAttrByType(type);
	MarbleModel::theModel()->setMarbleAttr(attr);
	for (int i = kMarble_Faster; i <= kMarble_Bomb; i++)
	{
		CCMenuItem *ballBtn = dynamic_cast<CCMenuItem*>(m_bottomLayout->getChildById(i + 9));
		CCSprite *lock = dynamic_cast<CCSprite*>(m_bottomLayout->getChildById(i + 13));
		ballBtn->stopAllActions();
		ballBtn->setScale(1.0f);
		if (type == i)
		{
			lock->setVisible(false);
			ballBtn->setColor(ccc3(255, 255, 255));
			auto action = GameUtil::getRepeatScaleAction();
			ballBtn->runAction(action);
		}
	}
}

void GameShooterMode::bossAttactEffect(int type)
{
	auto remainSquares = SquareModel::theModel()->getRemainSqaure();
	int remainCount = remainSquares.size();
	CCPoint targetPos = m_characterView->getPosition();
	if (remainCount <= 0)
	{
		int col = rand() % BALL_COL_SIZE;
		auto node = SquareModel::theModel()->createSquareNode(kBlock_Square);
		node->setIndex(col, 0);
		node->setPosition(ccp(node->getContentSize().width / 2 + SQUARE_SPACING + col * (node->getContentSize().width + SQUARE_SPACING),
			m_bottomLinePos + (node->getContentSize().height + SQUARE_SPACING) * 8.5));
		targetPos = node->getPosition();
	}
	else
	{
		SquareNode *node = nullptr;
		switch (type)
		{
		case kBoss_Spider:
			node = SquareModel::theModel()->addDoubleScore();
			node->setScale(1);
			node->runAction(GameUtil::getOnceScaleAction());
			break;
		case kBoss_Moth:
			node = SquareModel::theModel()->addDoubleSpeed();
			node->setScale(1);
			node->runAction(GameUtil::getOnceScaleAction());
			break;
		case kBoss_Ghost:
			SquareModel::theModel()->exchangeSquarePosition();
			GameController::getInstance()->setCanMove(false);
			auto actions = ActionSequence::create(this);
			auto delay = CCDelayTime::create(0.5f);
			actions->addAction(delay);
			auto callback = CCFunctionAction::create([=]()
			{
				GameController::getInstance()->setCanMove(true);
			});
			actions->addAction(callback);
			actions->runActions();
			for (auto iter = remainSquares.begin(); iter != remainSquares.end(); ++iter)
			{
				auto node = *iter;
				targetPos = node->getPosition();
				if (iter < remainSquares.end() - 1)
				{
					auto bossEffect = GameUtil::getBossSkillEffect(m_bossView->getPosition(), targetPos);
					addChild(bossEffect, kZOrder_Boss);
				}
			}
			break;
		}
		if (node != nullptr)
		{
			targetPos = node->getPosition();
		}
	}
	auto bossEffect = GameUtil::getBossSkillEffect(m_bossView->getPosition(), targetPos);
	addChild(bossEffect, kZOrder_Boss);
}

void GameShooterMode::addBoss()
{
	int bloodCount = GameController::getInstance()->getBossBloodCount();
	if (bloodCount <= 0)
	{
		return;
	}
	SoundMgr::theMgr()->playEffect(Effect_Boss);

	auto winSize = CCDirector::sharedDirector()->getWinSize();
	auto colorLayer = CCLayerColor::create(ccc4(0, 0, 0, 200));
	addChild(colorLayer, kZOrder_Boss);
	colorLayer->runAction(CCFadeOut::create(2.0f));

	int bossType = GameController::getInstance()->getBossType();
	if (bossType == kBoss_Max)
	{
		bossType = rand() % kBoss_Max;
	}
	m_bossView = BossView::create(bossType);
	m_bossView->setPosition(ccp(winSize.width / 2, winSize.height * 0.8f));
	m_bossView->startMoveAction();
	addChild(m_bossView, kZOrder_Boss, kTag_Boss);
}

void GameShooterMode::updateBoss()
{
	CCSprite *bossLabel = dynamic_cast<CCSprite*>(m_topLayout->getChildById(19));
	int bossType = GameController::getInstance()->getBossType();
	if (bossType == kBoss_Max)
	{
		bossType = rand() % kBoss_Max;
	}
	char temp[100] = { 0 };
	sprintf(temp, "animation/boss/boss_icon_%d.png", bossType);
	bossLabel->initWithFile(temp);
	GameController::getInstance()->setBossType(bossType);
}

void GameShooterMode::saveGameData()
{
	DataHelper::getInstance()->saveShootGameInfo();
}

void GameShooterMode::didAccelerate(CCAcceleration* pAccelerationValue)
{
	bool isGamePause = GameController::getInstance()->isGamePause();
	if (m_bIsGameOver || isGamePause || m_bIsTouching)
	{
		return;
	}

	float offsetX = pAccelerationValue->x * 10;
	if (offsetX < 0)
	{
		offsetX = -offsetX * offsetX;
	}
	else
	{
		offsetX = offsetX * offsetX;
	}
	auto winSize = CCDirector::sharedDirector()->getWinSize();
	auto characterPos = m_characterView->getPosition();
	characterPos.x += offsetX;
	auto size = m_characterView->getContentSize();
	if (characterPos.x < 32)
	{
		characterPos.x = 32;
	}
	else if (characterPos.x > winSize.width - 32)
	{
		characterPos.x = winSize.width - 32;
	}
	m_characterView->setPosition(characterPos);
	GameController::getInstance()->setShooterPos(characterPos);

	for (int i = 0; i < 2; i++)
	{
		auto wheelBig = dynamic_cast<CCSprite *>(m_characterView->getBodyById(7 + i));
		auto wheelSmall = dynamic_cast<CCSprite *>(m_characterView->getBodyById(10 + i));
		auto rotate1 = CCRotateBy::create(0.6f, (offsetX)* 60);
		auto rotate2 = CCRotateBy::create(0.6f, (offsetX)* 60);

		wheelBig->stopAllActions();
		wheelBig->runAction(rotate1);

		wheelSmall->stopAllActions();
		wheelSmall->runAction(rotate2);
	}
}