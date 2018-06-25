//It's after building the framework did I know that I need to make a tag game in assignment3
//don't want to copy codes, so I wrote this codes. It's pretty ugly. 
//Should not use it other than assignment2

#include "SwapWalker.h"
#include "GameManager.h"
#include <OgreManualObject.h>
#define SWAP_TIME 10

SwapWalker::SwapWalker(bool isPlayer)
{
	m_bNowPlayer = isPlayer;
	m_bCanSwap = m_bNowPlayer;
	m_rTimeSinceLastSwap = 0;
	m_swapObject = NULL;
	GamePlayer::m_eMyType = TYPE_SWAPWALKER;
	SoilderMonster::m_eMyType = TYPE_SWAPWALKER;
	_initialize();
}

SwapWalker::~SwapWalker()
{
}


void SwapWalker::_initialize()
{
	if (!m_bNowPlayer)
	{
		GamePlayer::m_sceneNode->removeAndDestroyAllChildren();
		GamePlayer::m_sceneNode = SoilderMonster::m_sceneNode;
		m_camNode = GamePlayer::m_sceneNode->createChildSceneNode(Ogre::Vector3(0, 1 ,1));
		GamePlayer::m_entity = SoilderMonster::m_entity;
	}
	else{
		SoilderMonster::m_sceneNode = GamePlayer::m_sceneNode;
		SoilderMonster::m_entity = GamePlayer::m_entity;
		GamePlayer::m_entity->setQueryFlags(PLAYER_MASK);
	}
}

void SwapWalker::perceiveWorld()
{
	GamePlayer::perceiveWorld();
	SoilderMonster::perceiveWorld();
}

void SwapWalker::_executeSwap()
{
	if (!m_bNowPlayer)
	{
		//guarantee the orientation is right
		SoilderMonster::reset();
		GamePlayer::reset();
		//notify the game manager about the new player
		GameManager::GetSingletonPtr()->refreshPlayer(static_cast<SwapWalker*>(static_cast<SoilderMonster*>(this)));
		//camera redirection
		GameManager::GetSingletonPtr()->attachCameraToNode(GamePlayer::getCameraNode(),SoilderMonster::getDirection());
		m_bCanSwap = false;
		m_bNowPlayer = true;
		m_rTimeSinceLastSwap = 0;
		GamePlayer::m_entity->setQueryFlags(PLAYER_MASK);
		SoilderMonster::m_entity->setQueryFlags(PLAYER_MASK);

		//swap effect
		GamePlayer* me = this;
		size_t id = GameManager::GetSingletonPtr()->playAttachedEffect(EFFECT_ENERGY_CLUSTER,me);
		GameManager::GetSingletonPtr()->setEffectLivingTime(id,4);
	}
	else
	{
		m_bNowPlayer  = false;
		m_rTimeSinceLastSwap = 0;
		m_bCanSwap = true;

		//notify other
		GamePlayer::reset();
		GamePlayer::m_entity->setQueryFlags(MONSTER_MASK);
		SoilderMonster::m_entity->setQueryFlags(MONSTER_MASK);
		SoilderMonster::reset();
		SoilderMonster::sendEvent(EVENT_PUSH,STATE_WALK);
		m_swapObject->sendEvent(EVENT_CHANGE,STATE_SWAP);

		m_swapObject = NULL;
	}
}

bool SwapWalker::checkCollision(float deta)
{
	if (!m_bNowPlayer)
	{
		return SoilderMonster::checkCollision(deta);
	}

	Ogre::Vector3 worldSpeedDirection = Ogre::Vector3::ZERO;
	switch(m_eCurrentDirection){
	case FORWARD:
		worldSpeedDirection = GamePlayer::getDirection();
		break;
	case BACKFORWARD:
		worldSpeedDirection = -GamePlayer::getDirection();
		break;
	case LEFT:
		worldSpeedDirection = *m_qToLeft*GamePlayer::getDirection();
		break;
	case RIGHT:
		worldSpeedDirection = -(*m_qToLeft*GamePlayer::getDirection());
		break;
	}

	Ogre::String name = "";
	//check collision of next position

	m_boxCollider->setPositionOffset(worldSpeedDirection*GamePlayer::m_rVelocity*deta - GamePlayer::getPosition());
	m_boxCollider->checkCollision(name,GamePlayer::m_vTargetType);

	Ogre::StringVector strVec = Ogre::StringUtil::split(name,"@");
	Ogre::Vector3 MeToTargetVec = Ogre::Vector3::ZERO;

	for (int i=0;i<strVec.size();i++)
	{
		m_swapObject = GameManager::GetSingletonPtr()->getGameObjectByName(strVec[i]);
		if (NULL != m_swapObject)
		{
			MeToTargetVec = m_swapObject->getPosition() - GamePlayer::getPosition();
			if (worldSpeedDirection.angleBetween(MeToTargetVec) < m_rdFOV/2)
			{
				if (m_bCanSwap)
				{
					if (m_swapObject->getType() == TYPE_SWAPWALKER)
					{
						Ogre::LogManager::getSingleton().logMessage("SWAP!!!");
						m_bCanSwap = false;
						GamePlayer::sendEvent(EVENT_CHANGE,STATE_SWAP);
					}
				}
				return true;
			}
		}
	}

/*
#if _DEBUG
	if (m_eCurrentDirection != DIRECTION_NULL)
	{
		m_manuObj->begin("LaserMaterial", Ogre::RenderOperation::OT_LINE_LIST); 
		m_manuObj->position(getPosition()); 
		m_manuObj->position(getPosition() + getPosition()+worldSpeedDirection()*1000);  
		m_manuObj->end(); 
	}
#endif
*/

	return false;
}

bool SwapWalker::update(const Ogre::FrameEvent& evt)
{
	if (!m_bNowPlayer)
	{
		SoilderMonster::update(evt);
	}
	else
	{
		GamePlayer::update(evt);
		if (!m_bCanSwap)
		{
			m_rTimeSinceLastSwap += evt.timeSinceLastFrame;
			if (m_rTimeSinceLastSwap >= SWAP_TIME)
			{
				m_bCanSwap = true;
				m_rTimeSinceLastSwap = 0;
			}
		}
	}
	return true;
}

void SwapWalker::processEvent(EVENT event, STATE state)
{
	if (!m_bNowPlayer)
	{
		SoilderMonster::processEvent(event,state);
	}
	else
	{
		GamePlayer::processEvent(event,state);
	}

	switch(state)
	{
	case STATE_SWAP:
		_executeSwap();
		break;
	}
}

size_t SwapWalker::getID()
{
	if (!m_bNowPlayer)
	{
		return SoilderMonster::getID();
	}
	else
	{
		return GamePlayer::getID();
	}

	return 0;
}

Ogre::String SwapWalker::getName()
{
	if (!m_bNowPlayer)
	{
		return SoilderMonster::getName();
	}
	else
	{
		return GamePlayer::getName();
	}
	return NULL;
}

Ogre::Entity* SwapWalker::getEntity()
{
	if (!m_bNowPlayer)
	{
		return SoilderMonster::getEntity();
	}
	else
	{
		return GamePlayer::getEntity();
	}
	return NULL;
}