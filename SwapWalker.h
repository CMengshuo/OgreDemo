#ifndef _SwapWalker_h_
#define _SwapWalker_h_

#include "GamePlayer.h"
#include "MonsterManager.h"

class SwapWalker: public GamePlayer, public SoilderMonster
{
public:
	SwapWalker(bool isPlayer = false);
	virtual ~SwapWalker();
	virtual bool update(const Ogre::FrameEvent& evt);
	virtual void processEvent(EVENT event, STATE state);
	virtual size_t getID();
	virtual Ogre::String getName();
	virtual Ogre::Entity* getEntity();
	virtual void perceiveWorld();

protected:
	bool m_bNowPlayer;
	Ogre::Real m_rTimeSinceLastSwap;
	bool m_bCanSwap;
	GameObject* m_swapObject;

protected:
	virtual bool checkCollision(float deta);
	virtual void _executeSwap();
	virtual void _initialize();
};

#endif