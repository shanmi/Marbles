#ifndef __COMMONMACROS_H__
#define __COMMONMACROS_H__

#define FONT_WHITE "numbers/number1.png"
#define FONT_YELLOW "numbers/number2.png"

enum GameType
{
	kGame_Normal,
	kGame_Shoot,
};

enum TouchPriority
{
	kPriority_Main = -1,
	kPriority_Game = -10,
	kPriority_Guide = -20,
	kPriority_LuckyLayer = -30,
	kPriority_Pause = -40,
	kPriority_Shop = -50,
	kPriority_Login = -60,
	kPriority_Libao = -70,
	kPriority_Reward = -80,
	kPriority_Clipping = -90,
};

enum GameZOrder
{
	kZOrder_Main,
	kZOrder_EliEffect,
	kZOrder_Square,
	kZOrder_Marble,
	kZOrder_Boss,
	kZOrder_Darkness,
	kZOrder_Character,
	kZOrder_Layout,
	kZOrder_Effect,
	KZOrder_GuideLayer,
	KZOrder_GameOver,
	kZOrder_Shop,
	KZOrder_LoginLayer,
	KZOrder_LuckyLayer,
	KZOrder_LibaoLayer,
	KZOrder_PauseLayer,
};

enum SquareType
{
	kType_Empty,
	kType_Square,
	kType_Triangle,
	kType_AddMarble,
	kType_AddCoin,
	kType_Rebound,
	kType_EliminateRow,
	kType_EliminateCol,
	kType_EliminateCross,
	kType_BossEatMarble,
	kType_Iron,
	kType_Crystal,
	kType_Shotguns,
	kType_Bomb,
};

enum MarbleType
{
	kMarble_Normal,
	kMarble_Faster,
	kMarble_Bigger,
	kMarble_Dispersed,
	kMarble_Bomb,
	kMarble_Across,
};

enum LuckyItem
{
	kLucky_Faster,
	kLucky_Hammer,
	kLucky_Bigger,
	kLucky_Freezing,
	kLucky_Bomb,
	kLucky_Dispersed,
	kLucky_DoubleAttact,
	kLucky_MaxCount,
};

enum ViewTag
{
	kTag_Streak = 100,
	kTag_Wall = 200,
	kTag_Wall_Top = 201,
	kTag_Boss = 210,
	kTag_Marble = 300,
	kTag_Pause,
	kTag_Shop,
	kTag_Libao,
	kTag_LoginLayer,
	kTag_GuideLayer,
	kTag_GameOver,
};

enum PropType
{
	kProp_Clear,
	kProp_DoubleAttact,
	kProp_Freezing
};

enum ToastType
{
	kToast_No_Coin,
	kToast_No_Prop
};

enum BossType
{
	kBoss_Ghost,
	kBoss_Spider,
	kBoss_Moth,
	kBoss_Max
};
#define NOTIFY_VIEWS(_FUNC_ ,...)						\
for (size_t index = 0; index < m_views.size(); ++index) \
{														\
	m_views[index]->_FUNC_(__VA_ARGS__);				\
}

#endif