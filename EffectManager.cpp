#include "EffectManager.h"
#include <OgreParticleSystem.h>
#include "GameManager.h"
#include "CollisionHandler.h"
#include "StateHandler.h"
#include "GameObject.h"

#define EFFECT_DYING_TIME 1
#define HIT_RANGE 1

//----------------------------GameEffect---------------------------------------------------------
size_t GameEffect::ms_uID = 0;

GameEffect::GameEffect(EFFECT_RESOURCE ersc,const Ogre::Vector3& startPos,Ogre::SceneManager* sceneMgr,Ogre::String tepName)
{
	m_uMyID = ms_uID++;
	m_eMyResource = ersc;
	m_eMyType = TYPE_NORMAL_EFFECT;
	m_sceneMgr = sceneMgr;
	m_strTemplateName = tepName;
	m_particles = m_sceneMgr->createParticleSystem("effect_" + Ogre::StringConverter::toString(m_uMyID),tepName);
	m_sceneNode = m_sceneMgr->getSceneNode("EffectRootNode")->createChildSceneNode("effect_" + Ogre::StringConverter::toString(m_uMyID));
	m_sceneNode->_setDerivedPosition(startPos);
	m_sphereCollider = new SphereCollisionHandler(this,MONSTER_MASK | OBSTACLE_MASK,Ogre::SceneManager::ENTITY_TYPE_MASK);
	m_sphereCollider->setRadius(HIT_RANGE);
	m_rVelocity = 0;
	m_vRoateAxis = Ogre::Vector3::ZERO;
	m_rRotateVelocity = 0;
	m_vSpeed = Ogre::Vector3::ZERO;
	m_stateHandler = new StateHandler(this);
	m_terrainCollider = NULL;
	m_bActive = true;
	m_rDyingTime = EFFECT_DYING_TIME;
	m_rTimeToLive = -1;
	m_vTargetType.clear();
}

Ogre::String GameEffect::getName()
{
	return "effect_" + Ogre::StringConverter::toString(m_uMyID);
}

void GameEffect::setTimeToLive(Ogre::Real time)
{
	m_rTimeToLive = time;
}

void GameEffect::sendEvent(EVENT event, STATE state)
{
	m_stateHandler->sendEvent(event,state);
}
void GameEffect::processEvent(EVENT event, STATE state)
{
	m_eCurrentMainState = state;
	switch (state)
	{
	case STATE_DEAD:
		_executeDie();
		break;
	case STATE_DYING:
		_executeDying();
		break;
	default:
		break;
	}
}

void GameEffect::_executeDie()
{
	m_bActive = false;
}

void GameEffect::_executeDying()
{
	setScale(Ogre::Vector3(1,1,1));
}
void GameEffect::setStartPos(const Ogre::Vector3& pos)
{
	m_sceneNode->_setDerivedPosition(pos);
}

void GameEffect::perceiveWorld()
{
	if (!m_terrainCollider)
	{
		m_terrainCollider = new TerrainCollisionHandler(this,GameManager::GetSingletonPtr()->getTerrainGroup());
	}
}

void GameEffect::reset()
{
	if (m_sceneNode)
	{
		m_sceneNode->resetOrientation();
		m_sceneNode->detachAllObjects();
	}
	m_bActive = true;
	m_rDyingTime = EFFECT_DYING_TIME;
	m_vSpeed = Ogre::Vector3::ZERO;
	m_stateHandler->clearAllStack();
}

EFFECT_RESOURCE GameEffect::getResourceName()
{
	return m_eMyResource;
}

void GameEffect::setSpeed(const Ogre::Vector3& direction, const Ogre::Real& v)
{
	m_rVelocity = v;
	m_vSpeed = direction*v;
}

GameEffect::~GameEffect()
{
	m_particles->clear();
	delete m_stateHandler;
	delete m_terrainCollider;
	delete m_sphereCollider;
}

void GameEffect::play()
{
	m_sceneNode->attachObject(m_particles);
	m_bActive = true;
}

void GameEffect::stop()
{
	m_sceneNode->detachObject(m_particles);
	m_particles->clear();
	m_bActive = false; 
}

void GameEffect::setRoate(const Ogre::Vector3& axis, const Ogre::Real& wRadian)
{
	m_vRoateAxis = axis;
	m_rRotateVelocity = wRadian;
}

size_t GameEffect::getID()
{
	return m_uMyID;
}

bool GameEffect::_checkCollision()
{
	Ogre::String name;
	m_sphereCollider->setCenter(getPosition());

	if (m_sphereCollider->checkCollision(name,m_vTargetType))
	{
		Ogre::StringVector strVec = Ogre::StringUtil::split(name,"@");
		GameObject* obj;

		for (int i=0;i<strVec.size();i++)
		{
			obj = GameManager::GetSingletonPtr()->getGameObjectByName(strVec[i]);
			if (NULL != obj)
			{
				obj->setDamage(100);
			}
		}

		if (strVec.size() > 0)
		{
			return true;
		}
	}

	return false;
}

void GameEffect::update(float deta /* = 0.0f */)
{
	m_particles->_update(deta);
	switch (m_eCurrentMainState)
	{
	case STATE_FLYING:
		if (m_rTimeToLive != -1)
		{
			m_rTimeToLive =- deta;
			if (m_rTimeToLive <= 0)
			{
				sendEvent(EVENT_CHANGE,STATE_DEAD);
				m_rTimeToLive = -1;
			}
		}
		if(_checkCollision())
		{
			sendEvent(EVENT_CHANGE,STATE_DYING);
			break;
		}
		if (isTerrainHit())
		{
			sendEvent(EVENT_CHANGE,STATE_DYING);
			break;
		}
		m_sceneNode->translate(m_vSpeed*deta);
		m_sceneNode->rotate(m_vRoateAxis,Ogre::Radian(m_rRotateVelocity*deta));
		break;
	case STATE_DYING:
		m_rDyingTime -= deta;
		if (m_rDyingTime <= 0)
		{
			sendEvent(EVENT_CHANGE,STATE_DEAD);
		}
		break;
	}
}

bool GameEffect::isTerrainHit()
{
	Ogre::Vector3 hitPos;
	Ogre::Vector3 Pos = m_sceneNode->_getDerivedPosition();

	if (m_terrainCollider->checkTerrainCollision(hitPos))
	{
		Ogre::Real intersectHeight = hitPos.y;
		//if the effect went to far, disable this effect
		if (intersectHeight - Pos.y > TERRAIN_CHECK_HEIGHT/3)
		{
			m_bActive = false;
		}
		if ((intersectHeight + 1.0) > Pos.y)
		{
			Pos.y = intersectHeight + 1.0;
			m_sceneNode->_setDerivedPosition(Pos);
			m_vSpeed.y = 0;
			return true;
		}
	}
	else
	{
		m_bActive = false;
	}

	return false;
}

ELEMENT_TYPE GameEffect::getType()
{
	return m_eMyType;
}


//----------------------------AttachedEffect-----------------------------------------------------------------------

AttachedEffect::AttachedEffect(EFFECT_RESOURCE ersc,const Ogre::Vector3& startPos,Ogre::SceneManager* sceneMgr,Ogre::String tepName, GameElement* elt,Ogre::Vector3 offset)
	:GameEffect(ersc,startPos, sceneMgr,tepName),m_target(elt),m_vOffset(offset)
{
	m_eMyType = TYPE_ATTACH_EFFECT;
}

AttachedEffect::~AttachedEffect()
{

}

void AttachedEffect::setTarget(GameElement* elt)
{
	m_target = elt;
}

void AttachedEffect::setOffset(const Ogre::Vector3& vec)
{
	m_vOffset = vec;
}

void AttachedEffect::update(float deta)
{
	m_particles->_update(deta);
	switch (m_eCurrentMainState)
	{
	case STATE_ATTACHING:
		if (m_rTimeToLive != -1)
		{
			m_rTimeToLive -= deta;
			if (m_rTimeToLive < 0)
			{
				sendEvent(EVENT_CHANGE,STATE_DEAD);
				m_rTimeToLive = -1;
			}
		}

		if (m_target)
		{
			m_sceneNode->_setDerivedPosition(m_target->getPosition()+ m_vOffset);
			static_cast<GameObject*>(m_target)->setDamage(1);
		}
		else
		{
			sendEvent(EVENT_CHANGE,STATE_DYING);
		}

		if (!m_target->isActive())
		{
			sendEvent(EVENT_CHANGE,STATE_DYING);
		}

		m_sceneNode->rotate(m_vRoateAxis,Ogre::Radian(m_rRotateVelocity*deta));
		break;
	case STATE_DYING:
		m_rDyingTime -= deta;
		if (m_rDyingTime <= 0)
		{
			sendEvent(EVENT_CHANGE,STATE_DEAD);
		}
		break;
	}
}

//-----------------------------TrackEffect-----------------------------------------------------------------------

TrackEffect::TrackEffect(EFFECT_RESOURCE ersc, const Ogre::Vector3& startPos,Ogre::SceneManager* sceneMgr,Ogre::String tepName, GameElement* elt)
	:GameEffect(ersc,startPos,sceneMgr,tepName),m_target(elt)
{
	m_eMyType = TYPE_TRACK_EFFECT;
}

TrackEffect::~TrackEffect()
{

}

void TrackEffect::setTarget(GameElement* elt)
{
	m_target = elt;
}

void TrackEffect::update(float deta)
{
	m_particles->_update(deta);

	switch (m_eCurrentMainState)
	{
	case STATE_FLYING:
		if (m_rTimeToLive != -1)
		{
			m_rTimeToLive =- deta;
			if (m_rTimeToLive <= 0)
			{
				sendEvent(EVENT_CHANGE,STATE_DYING);
				m_rTimeToLive = -1;
			}
		}
		m_sceneNode->translate(m_vSpeed*deta);
		m_sceneNode->rotate(m_vRoateAxis,Ogre::Radian(m_rRotateVelocity*deta));
		_trackTarget();
		break;
	case STATE_DYING:
		m_rDyingTime -= deta;
		if (m_rDyingTime <= 0)
		{
			sendEvent(EVENT_CHANGE,STATE_DEAD);
		}
		break;
	}
}

void TrackEffect::_trackTarget()
{
	if (!m_target)
	{
		sendEvent(EVENT_CHANGE,STATE_DYING);
		return;
	}

	if (!m_target->isActive())
	{
		sendEvent(EVENT_CHANGE,STATE_DYING);
		return;
	}

	if (m_sphereCollider->checkCollision(m_target))
	{
		sendEvent(EVENT_CHANGE,STATE_DYING);
		m_target->sendEvent(EVENT_CHANGE,STATE_DYING);
		return;
	}

	Ogre::Vector3 targetPos =m_target->getPosition();
	Ogre::Vector3 myPos = m_sceneNode->_getDerivedPosition();
	Ogre::Vector3 myDirect = getDirection();
	Ogre::Vector3 toTargetDirect = targetPos - myPos;

	//Ogre::Quaternion q = myDirect.getRotationTo(toTargetDirect+4*myDirect);
	Ogre::Quaternion q = myDirect.getRotationTo(toTargetDirect);
	m_sceneNode->rotate(q);

	m_vSpeed = getDirection()*m_rVelocity;
}

//----------------------------EffectManager--------------------------------------------------------------------

EffectManager::EffectManager()
{
	m_sceneMgr = GameManager::GetSingletonPtr()->getSceneManager();
	m_ResourceMap.clear();
	m_PlayList.clear();
	m_RecyclePool.clear();
}

EffectManager::~EffectManager()
{
	m_ResourceMap.clear();
	std::vector<GameEffect*>::iterator it;

	it = m_RecyclePool.begin();
	while (it != m_RecyclePool.end())
	{
		delete (*it);
		it++;	
	}
	m_RecyclePool.clear();

	std::map<size_t,GameEffect*>::iterator mit = m_PlayList.begin();
	while (mit != m_PlayList.end())
	{
		delete mit->second;
		mit++;
	}
	m_PlayList.clear();
}

void EffectManager::registerEffect(EFFECT_RESOURCE ersc,Ogre::String tepName)
{
	if(m_ResourceMap.find(ersc) == m_ResourceMap.end())
	{
		m_ResourceMap.insert(std::map<EFFECT_RESOURCE,Ogre::String>::value_type(ersc,tepName));
	}
}

void EffectManager::stopEffectByIndex(size_t index)
{
	std::map<size_t,GameEffect*>::iterator it = m_PlayList.find(index);
	if (it != m_PlayList.end())
	{
		it->second->stop();
		m_RecyclePool.push_back(it->second);
		m_PlayList.erase(it);
	}
}

size_t EffectManager::addAttachedEffect(EFFECT_RESOURCE ersc, GameElement* target, const Ogre::Vector3& offset)
{
	GameEffect* effect = _checkRecyclePool(ersc,TYPE_ATTACH_EFFECT);
	if (NULL != effect)  
	{
		static_cast<AttachedEffect*>(effect)->setTarget(target);
		static_cast<AttachedEffect*>(effect)->setOffset(offset);
		effect->play();
		effect->sendEvent(EVENT_PUSH,STATE_ATTACHING);
		m_PlayList.insert(std::map<size_t,GameEffect*>::value_type(effect->getID(),effect));
		return effect->getID();
	}


	Ogre::String tepName = _getTemplateNameByType(ersc);
	if (tepName == Ogre::StringUtil::BLANK)
		return 0;

	effect = new AttachedEffect(ersc,target->getPosition() + offset,m_sceneMgr,tepName,target,offset);
	size_t id = effect->getID();
	m_PlayList.insert(std::map<size_t,GameEffect*>::value_type(id,effect));
	effect->play();
	effect->perceiveWorld();
	effect->sendEvent(EVENT_PUSH,STATE_ATTACHING);
	return id;
}

size_t EffectManager::addTrackEffect(EFFECT_RESOURCE ersc, const Ogre::Vector3& startPos, GameElement* target)
{
	GameEffect* effect = _checkRecyclePool(ersc,TYPE_TRACK_EFFECT);
	if (NULL != effect)
	{
		effect->setStartPos(startPos);
		static_cast<TrackEffect*>(effect)->setTarget(target);		
		effect->play();
		effect->sendEvent(EVENT_PUSH,STATE_FLYING);
		m_PlayList.insert(std::map<size_t,GameEffect*>::value_type(effect->getID(),effect));
		return effect->getID();
	}

	Ogre::String tepName = _getTemplateNameByType(ersc);
	if (tepName == Ogre::StringUtil::BLANK)
		return 0;

	effect = new TrackEffect(ersc,startPos, m_sceneMgr,tepName,target);
	size_t id = effect->getID();
	m_PlayList.insert(std::map<size_t,GameEffect*>::value_type(id,effect));
	effect->play();
	effect->perceiveWorld();
	effect->sendEvent(EVENT_PUSH,STATE_FLYING);
	return id;
}

size_t EffectManager::addNormalEffect(EFFECT_RESOURCE ersc, const Ogre::Vector3& startPos)
{
	GameEffect* effect = _checkRecyclePool(ersc,TYPE_NORMAL_EFFECT);
	if (NULL != effect)
	{
		effect->setStartPos(startPos);
		effect->sendEvent(EVENT_PUSH,STATE_FLYING);
		m_PlayList.insert(std::map<size_t,GameEffect*>::value_type(effect->getID(),effect));
		effect->play();
		return effect->getID();
	}

	Ogre::String tepName = _getTemplateNameByType(ersc);
	if (tepName == Ogre::StringUtil::BLANK)
		return 0;
	
	effect = new GameEffect(ersc,startPos, m_sceneMgr,tepName);
	size_t id = effect->getID();
	m_PlayList.insert(std::map<size_t,GameEffect*>::value_type(id,effect));
	effect->play();
	effect->perceiveWorld();
	effect->sendEvent(EVENT_PUSH,STATE_FLYING);
	return id;
}

GameEffect* EffectManager::_checkRecyclePool(EFFECT_RESOURCE ersc, ELEMENT_TYPE etype)
{
	std::vector<GameEffect*>::iterator it = m_RecyclePool.begin();
	while (it != m_RecyclePool.end())
	{
		if ((*it)->getResourceName() == ersc && etype == (*it)->getType())
		{
			(*it)->reset();
			(*it)->perceiveWorld();
			GameEffect* effect = (*it);
			m_RecyclePool.erase(it);
			return effect;
		}
		it++;
	}
	return NULL;
}

Ogre::String EffectManager::_getTemplateNameByType(EFFECT_RESOURCE ersc)
{
	std::map<EFFECT_RESOURCE,Ogre::String>::iterator it;
	it = m_ResourceMap.find(ersc);
	if (it != m_ResourceMap.end())
	{
		return it->second;
	}
	return Ogre::StringUtil::BLANK;
}

void EffectManager::update(float deta)
{
	std::vector<size_t> recycleVec;
	recycleVec.clear();

	std::map<size_t,GameEffect*>::iterator it = m_PlayList.begin();
	while (it != m_PlayList.end())
	{
		it->second->update(deta);
		if (!it->second->isActive())
		{
			recycleVec.push_back(it->second->getID());
		}
		it++;
	}

	while (!recycleVec.empty())
	{
		stopEffectByIndex(recycleVec.back());
		recycleVec.pop_back();
	}
}

GameEffect* EffectManager::_getPlayingEffectByIndex(size_t index)
{
	std::map<size_t,GameEffect*>::iterator it = m_PlayList.find(index);
	if (it != m_PlayList.end())
	{
		return it->second;
	}
	return NULL;
}

void EffectManager::setEffectRotate(const size_t id, const Ogre::Vector3& axis, const Ogre::Real& w)
{
	GameEffect* effect = _getPlayingEffectByIndex(id);
	if (NULL == effect)
	{
		return;
	}

	effect->setRoate(axis,w);
}

void EffectManager::setEffectScale(const size_t id, const Ogre::Vector3& scale)
{
	GameEffect* effect = _getPlayingEffectByIndex(id);
	if (NULL == effect)
	{
		return;
	}

	effect->setScale(scale);
}

void EffectManager::setEffectLivingTime(const size_t id, const Ogre::Real time)
{
	GameEffect* effect = _getPlayingEffectByIndex(id);
	if (NULL == effect)
	{
		return;
	}
	effect->setTimeToLive(time);
}

void EffectManager::setEffectSpeed(const size_t id, const Ogre::Vector3& vec, const Ogre::Real& v)
{
	GameEffect* effect = _getPlayingEffectByIndex(id);
	if (NULL == effect)
	{
		return;
	}

	effect->setSpeed(vec,v);
}