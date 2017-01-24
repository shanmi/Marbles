#include "SquareModel.h"
#include "Config.h"
#include "CommonMacros.h"
#include "GameController.h"
#include "Box2dFactory.h"
#include "CircleNode.h"
#include "GameConfig.h"

SquareModel::SquareModel()
:m_curScore(1)
, m_bIsFreezing(false)
{

}

SquareModel *SquareModel::theModel()
{
	static SquareModel model;
	return &model;
}

SquareNode *SquareModel::createSquareNode(int type, int shap /* = -1 */)
{
	SquareNode *node;
	switch (type)
	{
	case kType_Square:
		node = SquareNode::create();
		break;
	case kType_Triangle:
		node = TriangleNode::create(shap);
		break;
	case kType_AddMarble:
		node = CircleAddMarbleNode::create();
		break;
	case kType_EliminateRow:
		node = CircleEliRowNode::create();
		break;
	case kType_EliminateCol:
		node = CircleEliColNode::create();
		break;
	case kType_EliminateCross:
		node = CircleEliCrossNode::create();
		break;
	case kType_Rebound:
		node = CircleReboundNode::create();
		break;
	case kType_AddCoin:
		node = CircleAddCoinNode::create();
		break;
	case kType_BossEatMarble:
		node = BossEatMarbleNode::create();
		break;
	case kType_Protect:
		node = CircleProtectNode::create();
		break;
	case kType_Shotguns:
		node = CircleShotgunsNode::create();
		break;
	case kType_Blood:
		node = CircleBloodNode::create();
		break;
	case kType_Rocket:
		node = CircleRocketNode::create();
		break;
	case kType_Empty:
		node = nullptr;
		break;
	default:
		assert(false && "not exit square type!");
		break;
	}
	if (node != nullptr)
	{
		m_squares.push_back(node);
	}
	return node;
}

void SquareModel::removeSquareNode(SquareNode *node)
{
	auto iter = find(m_squares.begin(), m_squares.end(), node);
	if (iter != m_squares.end())
	{
		m_squares.erase(iter);
	}
	Box2dFactory::getInstance()->removeBody(node->getBody());
	node->runRemoveAction();
}

void SquareModel::removeAllSquares()
{
	for (auto iter = m_squares.begin(); iter != m_squares.end(); ++iter)
	{
		auto &square = *iter;
		square->addScore(-square->getScore());
	}
}

std::vector<SquareNode*> SquareModel::loadSquareList()
{
	if (m_squares.size() == 0)
	{
		return createSquareList();
	}
	return m_squares;
}

std::vector<SquareNode*> SquareModel::createSquareList(bool autoAddScore /* = true */)
{
	srand(time(NULL));
	std::vector<SquareNode*> nodes;

	std::vector<int> ballListType;
	if (autoAddScore)
	{
		ballListType = getBallListType1();
	}
	else
	{
		ballListType = getBallListType2();
	}
	for (int i = 0; i < BALL_COL_SIZE; i++)
	{
		int type = kType_AddCoin;// ballListType.at(i);
		int random = rand() % 2;
		if (random == 1)
		{
			type = kType_Square;
		}

		type = ballListType.at(i);
		SquareNode *node = createSquareNode(type);
		if (node != nullptr)
		{
			node->setBody();
			node->setIndex(i, 0);
			nodes.push_back(node);
		}
	}
	if (autoAddScore)
	{
		m_curScore++;
	}
	return nodes;
}

std::vector<int> SquareModel::getBallListType1()
{
	auto probabilitys = GameConfig::getInstance()->m_probability1;
	int count = GameConfig::getInstance()->m_probabilityCount1;
	std::vector<int> types;
	for (int i = 0; i < BALL_COL_SIZE; i++)
	{
		int type = getBallType(probabilitys, count);
		if (type == kType_AddMarble || type == kType_AddCoin || type == kType_Rebound || type == kType_EliminateRow || type == kType_EliminateCol || type == kType_EliminateCross || type == kType_BossEatMarble)
		{
			auto iter = find(types.begin(), types.end(), type);
			while (iter != types.end())
			{
				type = getBallType(probabilitys, count);
				iter = find(types.begin(), types.end(), type);
			}
		}
		int curSocre = SquareModel::theModel()->getCurrentScore();
		if (curSocre <= 3 && (type == kType_Rebound || type == kType_EliminateRow || type == kType_EliminateCol || type == kType_EliminateCross))
		{
			auto iter = find(types.begin(), types.end(), type);
			while (type == kType_Rebound || type == kType_EliminateRow || type == kType_EliminateCol || type == kType_EliminateCross)
			{
				type = getBallType(probabilitys, count);
				iter = find(types.begin(), types.end(), type);
			}
		}
		types.push_back(type);
	}
	return types;
}

std::vector<int> SquareModel::getBallListType2()
{
	auto probabilitys = GameConfig::getInstance()->m_probability2;
	int count = GameConfig::getInstance()->m_probabilityCount2;
	std::vector<int> types;
	for (int i = 0; i < BALL_COL_SIZE; i++)
	{
		int type = getBallType(probabilitys, count);
		if (type == kType_AddCoin || type == kType_EliminateRow || type == kType_EliminateCol || type == kType_EliminateCross || type == kType_Protect)
		{
			auto iter = find(types.begin(), types.end(), type);
			while (iter != types.end())
			{
				type = getBallType(probabilitys, count);
				iter = find(types.begin(), types.end(), type);
			}
		}
		types.push_back(type);
	}
	return types;
}

int SquareModel::getBallType(int probsArr[10][3], int size)
{
	std::vector<int> probs;
	int probAll = 0;
	for (int i = 0; i < size; i++)
	{
		int prob = probsArr[i][1];
		int curScore = SquareModel::theModel()->getCurrentScore();
		int m_showLibaoLevel = GameConfig::getInstance()->m_showLibaoLevel;
		if (curScore % m_showLibaoLevel == 0)
		{
			prob += probsArr[i][2] * (curScore / m_showLibaoLevel);
		}
		probAll += prob;
		probs.push_back(prob);
	}

	int random = rand() % (probAll);
	int total = 0;
	for (auto iter = probs.begin(); iter != probs.end(); ++iter)
	{
		int prob = (*iter);
		total += prob;
		if (random < total)
		{
			int index = iter - probs.begin();
			return probsArr[index][0];
		}
	}
}

void SquareModel::squareMoveDown()
{
	for (auto iter = m_squares.begin(); iter != m_squares.end(); ++iter)
	{
		bool isLastOne = false;
		if (iter == m_squares.end() - 1)
		{
			isLastOne = true;
		}
		auto &square = *iter;
		square->moveDown(isLastOne);
		Index index = square->getIndex();
		square->setIndex(index.x, index.y + 1);
	}
}

void SquareModel::removeBelowSquares()
{
	auto bottomPos = GameController::getInstance()->getTargetPos();
	auto squares = m_squares;
	for (auto iter = squares.begin(); iter != squares.end(); ++iter)
	{
		auto &square = (*iter);
		if (square->getPositionY() - square->getContentSize().height * 5 < bottomPos.y)
		{
			removeSquareNode(square);
		}
	}
}

void SquareModel::elimateSameRowSquare(SquareNode *node)
{
	Index index = node->getIndex();
	for (auto iter = m_squares.begin(); iter != m_squares.end(); ++iter)
	{
		auto &square = (*iter);
		Index squareIndex = square->getIndex();
		if (square->canRemoveByProps() && index.y == squareIndex.y)
		{
			square->addScore(-1);
		}
	}
}

void SquareModel::elimateSameColSquare(SquareNode *node)
{
	Index index = node->getIndex();
	for (auto iter = m_squares.begin(); iter != m_squares.end(); ++iter)
	{
		auto &square = (*iter);
		Index squareIndex = square->getIndex();
		if (square->canRemoveByProps() && index.x == squareIndex.x)
		{
			square->addScore(-1);
		}
	}
}

void SquareModel::elimateAroundSquare(SquareNode *node)
{
	Index index = node->getIndex();
	for (auto iter = m_squares.begin(); iter != m_squares.end(); ++iter)
	{
		auto &square = (*iter);
		Index squareIndex = square->getIndex();
		if (square->canRemoveByProps() && abs(index.x - squareIndex.x) <= 1 && abs(index.y - squareIndex.y) <= 1)
		{
			square->addScore(-square->getScore());
		}
	}
}

void SquareModel::setSquareFreezing(bool isFreezing)
{
	m_bIsFreezing = isFreezing;
	for (auto iter = m_squares.begin(); iter != m_squares.end(); ++iter)
	{
		auto &square = (*iter);
		square->setFreezing(isFreezing);
	}
}
void SquareModel::clearSquares()
{
	m_squares.clear();
	m_curScore = 1;
}