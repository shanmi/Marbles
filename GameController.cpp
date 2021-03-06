#include "GameController.h"
#include "MarbleModel.h"
#include "SquareModel.h"
#include "UserInfo.h"
#include "BallHintModel.h"
#include "MainMenu.h"
#include "GameUtil.h"
USING_NS_CC;

GameController::GameController()
:m_counter(-1)
, m_bIsRoundOver(true)
, m_targetPos(ccp(0, 0))
, m_attactRate(ATTACT_RATE)
, m_bISFirstIn(true)
, m_gameType(kGame_Normal)
, m_bIsGamePause(false)
, m_bossType(kBoss_Max)
, m_bossBloodCount(0)
, m_crystalBloods({ 2, 3, 2})
, m_bCanMove(true)
, m_targetLevel(10)
{
}

void GameController::addView(INotifyView *view)
{
	auto iter = find(m_views.begin(), m_views.end(), view);
	if (iter == m_views.end())
	{
		m_views.push_back(view);
	}
}

void GameController::removeView(INotifyView *view)
{
	auto iter = find(m_views.begin(), m_views.end(), view);
	if (iter != m_views.end())
	{
		m_views.erase(iter);
	}
}
GameController *GameController::getInstance()
{
	static GameController instance;
	return &instance;
}

void GameController::startOneRound()
{
	m_counter = 0;// MarbleModel::theModel()->getMarbles().size();
}

bool GameController::isCounterFull()
{
	if (m_counter >= MarbleModel::theModel()->getMarbles().size())
	{
		return true;
	}
	return false;
}

int GameController::getAttactRate()
{
	auto marbleDamage = MarbleModel::theModel()->getMarbleAttr().damage;
	int marbleRate = MarbleModel::theModel()->getAttactRate();
	return m_attactRate * marbleRate * marbleDamage;
}

void GameController::checkSquares(bool isRoundEnd /* = false */)
{
	auto squares = SquareModel::theModel()->getSquares();
	for (auto iter = squares.begin(); iter != squares.end(); ++iter)
	{
		auto square = *iter;
		if (square->getScore() <= 0)
		{
			if (square->shouldRemoveDirectly() || isRoundEnd)
			{
				SquareModel::theModel()->removeSquareNode(square);
			}
		}
		if (square->getBody() == NULL)
		{
			square->setBody();
			GameController::getInstance()->addSquareNode(square);
		}
	}
}

void GameController::updateMarblePos()
{
	auto marbles = MarbleModel::theModel()->getMarbles();
	for (auto iter = marbles.begin(); iter != marbles.end(); ++iter)
	{
		auto square = *iter;
		square->setPosition(m_targetPos);
	}
}

bool GameController::isGameOver()
{
	auto squares = SquareModel::theModel()->getSquares();
	for (auto iter = squares.begin(); iter != squares.end(); ++iter)
	{
		auto &square = (*iter);
		if (square->getPositionY() - square->getContentSize().height < m_targetPos.y)
		{
			if (square->canRemoveByProps())
			{
				return true;
			}
		}
	}
	return false;
}

bool GameController::checkGameOver()
{
	auto squares = SquareModel::theModel()->getSquares();
	for (auto iter = squares.begin(); iter != squares.end(); ++iter)
	{
		auto &square = (*iter);
		if (square->getPositionY() - square->getContentSize().height < m_targetPos.y)
		{
			if (square->canRemoveByProps())
			{
				showGameOver();
				return true;
			}
			//SquareModel::theModel()->removeSquareNode(square);
		}
	}
	return false;
}

void GameController::createPropByMarble(MarbleNode *marble)
{
	MarbleModel::theModel()->reboundMarbles();
	auto targetPos = GameController::getInstance()->getTargetPos();
	auto node = CCSprite::create("squares/fangkuai_hengxiao.png");
	float posX = (node->getContentSize().width / 2 + SQUARE_SPACING) + 3 * (node->getContentSize().width + SQUARE_SPACING);
	float posY = targetPos.y + node->getContentSize().height / 2 - marble->getContentSize().height / 2;
	while (posY < marble->getPositionY())
	{
		posY += (node->getContentSize().height + SQUARE_SPACING);
	}
	posY = marble->getPositionY();// posY - node->getContentSize().height / 2 - SQUARE_SPACING;
	node->setPosition(ccp(posX, posY));

	auto squares = SquareModel::theModel()->getSquares();
	for (auto iter = squares.begin(); iter != squares.end(); ++iter)
	{
		auto square = (*iter);
		if (square->getSquareType() != kType_Rebound)
		{
			auto rect = square->boundingBox();
			if (rect.containsPoint(ccp(posX, posY)))
			{
				posX = targetPos.x;
				//return;
			}
		}
	}
	auto square = SquareModel::theModel()->createSquareNode(kType_Rebound);
	square->setPosition(ccp(posX, posY));
	square->addScore(-square->getScore());
}

void GameController::backToMainMenu()
{
	MarbleModel::theModel()->clearMarbles();
	BallHintModel::theModel()->clearHints();
	SquareModel::theModel()->clearSquares();
	GameController::getInstance()->setRoundState(true);
	CCDirector* pDirector = CCDirector::sharedDirector();
	CCScene *pScene = MainMenu::scene();
	pDirector->replaceScene(pScene);
}

void GameController::setCrystalBlood(int index, int bloodCount)
{
	m_crystalBloods.at(index) = bloodCount;
}

int GameController::getCrystalBlood(int index)
{
	return m_crystalBloods.at(index);
}

void GameController::setTargetLevel(int level)
{
	m_targetLevel = level;
}

int GameController::getTargetLevel()
{
	return m_targetLevel;
}
void GameController::resetTargetLevel()
{
	m_targetLevel = 10;
}