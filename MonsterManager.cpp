#include "MonsterManager.h"
#include "GameManager.h"
#include "GameObject.h"
#include <OgreMaterial.h>
#include <OgreEntity.h>
#include <OgreSubEntity.h>

#define MONSTER_WALK_VELOCITY 12
#define MONSTER_ATTACK_DEGREE 30
#define MONSTER_ATTACK_RANGE 40
#define MONSTER_EVASION_CHECK_DEGREE 160
#define MONSTER_AWARENESS_RANGE 120
#define MONSTER_AWARENESS_DEGREE 300

#define ATTACK_SPAN 0.02

Monster::Monster(){
	m_strWalkAction = "RunBase";
	m_strWalkActionTop = "RunTop";
	m_strAttckAction = "SliceHorizontal";
	m_strDeadAction = "Dance";
	m_eEnemyType = TYPE_PLAYER;
	m_rAttackRange = MONSTER_ATTACK_RANGE;
	m_rAwareRange = MONSTER_AWARENESS_RANGE;
	m_rAwareDegree = MONSTER_AWARENESS_DEGREE;
	m_rVelocity = MONSTER_WALK_VELOCITY;
	m_rTolerentAttackAngle = MONSTER_ATTACK_DEGREE;
	m_rHitPoint = 400;
	m_rAttackTime = 0;
	m_rAttackPower = 15;
	m_bFadeOut = false;
	m_rFadeOutFactor = 1.0;
}

Monster::~Monster(){
	//delete m_sphereEvasionCollider;
	delete m_enemyCheckCollider;
	delete m_evadeCheckCollider;
}

void Monster::_initialize(){
	GameObject::_initialize();
	m_entity->setQueryFlags(MONSTER_MASK);
}

void Monster::_colliderSetting()
{
	m_enemyCheckCollider = new SphereCollisionHandler(this,PLAYER_MASK,Ogre::SceneManager::ENTITY_TYPE_MASK);
	m_enemyCheckCollider->setRadius(MONSTER_AWARENESS_RANGE);

	m_evadeCheckCollider = new BoxCollisionHandler(this,MONSTER_MASK | OBSTACLE_MASK,Ogre::SceneManager::ENTITY_TYPE_MASK);
}

void Monster::reset()
{
	m_stateHandler->clearAllStack();
	m_bRotating = false;
	m_target = NULL;
	m_bActive = true;
	m_vSpeed = Ogre::Vector3::ZERO;
	stopAnimation();
	m_sceneNode->resetOrientation();
	m_bFadeOut = false;
	m_rFadeOutFactor = 1.0;

	int nCnt = m_entity->getNumSubEntities(); 

	for (int i=0;i<nCnt;i++)
	{
		Ogre::MaterialPtr mat = m_entity->getSubEntity(i)->getMaterial();
		Ogre::MaterialPtr _MaterPt;
		_MaterPt=mat->clone(m_entity->getSubEntity(i)->getMaterialName());
		m_entity->getSubEntity(i)->setMaterial(_MaterPt);
		Ogre::Pass * _Pass;
		_Pass =_MaterPt->getTechnique(0)->getPass(0);
		_Pass->setLightingEnabled(true);
		_Pass->setDiffuse(1.0f,1.0f,1.0f,1.0f);
		_Pass->setSceneBlending(Ogre:: SBT_TRANSPARENT_ALPHA);
		_Pass->setDepthCheckEnabled(true);
	}
}

bool Monster::checkCollision(float deta){
	if (!m_bActive)
		return false;

	//not going out the boundary of world
	_checkWorldBound(deta);

	//function calling orders are based on status priority, the last one called has the highest priority
	switch(m_eCurrentMainState)
	{
	case STATE_WALK:
		_enemyCheck();
		_evasionCheck(deta);
		break;
	case STATE_EVADE:
		_evasionCheck(deta);
		//_attackCheck();
		break;
	case STATE_ATTACK:
		_attackCheck(deta);
		break;
	case STATE_TRACE:
		_enemyCheck();
		if (_evasionCheck(deta))
			break;
		_attackCheck(deta);
		break;
	case STATE_IDLE:
		_evasionCheck(deta);
		_attackCheck(deta);
		break;
	case STATE_TURNBACK:
		_checkWorldBound(deta);
		break;
	default:
		break;
	}

	return true;
}

bool Monster::_evasionCheck(float deta)
{
	//check collision of next position
	Ogre::String name;
	m_evadeCheckCollider->setPositionOffset(getDirection()*m_rVelocity*(DETECT_SPAN/deta) - getPosition());
	m_evadeCheckCollider->checkCollision(name,m_vTargetType);

	Ogre::StringVector strVec = Ogre::StringUtil::split(name,"@");
	GameObject* obj = NULL;
	Ogre::Vector3 MeToTargetVec;

	for (int i=0;i<strVec.size();i++)
	{
		obj = GameManager::GetSingletonPtr()->getGameObjectByName(strVec[i]);
		if (NULL != obj)
		{
			MeToTargetVec = obj->getPosition() - getPosition();
			if (getDirection().angleBetween(MeToTargetVec) < Ogre::Radian(Ogre::Degree(MONSTER_EVASION_CHECK_DEGREE/2)))
			{
				switch(m_eCurrentMainState){
				case STATE_EVADE:
					break;
				default:
					sendEvent(EVENT_PUSH,STATE_EVADE);
					Ogre::Quaternion q = getDirection().getRotationTo(MeToTargetVec);
					if (q.getYaw() > Ogre::Radian(0))
					{
						m_rotateDegreePerFrame = q.getYaw();
					}
					else
					{
						m_rotateDegreePerFrame = -q.getYaw();
					}

					break;
				}
				return true;
			}
		}
	}//end for

	//no collision
	switch(m_eCurrentMainState){
	case STATE_EVADE:
		sendEvent(EVENT_POP,STATE_EVADE);
		break;
	default:
		break;
	}

	return false;
}


bool Monster::_enemyCheck(){
	Ogre::String name = "";
	bool IsValidCollision = false;

	//Have seen the enemy
	if (m_target)
	{
		m_enemyCheckCollider->setAwareDegree(360);
	}
	else
	{
		m_enemyCheckCollider->setAwareDegree(m_rAwareDegree);
	}

	IsValidCollision = m_enemyCheckCollider->checkCollision(name,m_vTargetType);
	//Ogre::LogManager::getSingleton().logMessage(Ogre::StringConverter::toString(IsValidCollision) + "  "+ Ogre::StringConverter::toString(getID()) + "****collide name***" + name);
	Ogre::StringVector strVec = Ogre::StringUtil::split(name,"@");
	m_target =GameManager::GetSingletonPtr()->getGameObjectByName(strVec[0]);

	if (IsValidCollision)
	{

		switch(m_eCurrentMainState)
		{
			case STATE_TRACE:
				break;
			default:
				sendEvent(EVENT_PUSH,STATE_TRACE);
				break;
		}
	}
	else
	{
		switch(m_eCurrentMainState)
		{
		case STATE_TRACE:
			sendEvent(EVENT_POP,STATE_TRACE);
			m_target = NULL;
			break;
		default:
			break;
		}
	}

	return IsValidCollision;
}

bool Monster::_attackCheck(float deta)
{
	if (!m_target)
		return false;

	Ogre::Vector3 targetPos =m_target->getPosition();
	Ogre::Vector3 myPos = getPosition();
	Ogre::Vector3 myDirect = getDirection();
	Ogre::Vector3 toTargetDirect = targetPos - myPos;

	//squa distance to save computing resources
	bool IsAtRange = myPos.squaredDistance(targetPos) <= m_rAttackRange*m_rAttackRange;
	Ogre::Real degree = myDirect.angleBetween(toTargetDirect).valueDegrees();
	bool IsGoodAngle = degree <= m_rTolerentAttackAngle;

	if (IsAtRange)
	{
		if (IsGoodAngle)
		{
			m_bRotating = false;
			switch(m_eCurrentMainState)
			{
			case STATE_ATTACK:
				m_rAttackTime += deta;
				Ogre::LogManager::getSingleton().logMessage("attack here1 " + Ogre::StringConverter::toString(m_rAttackTime));

				if (m_rAttackTime >= ATTACK_SPAN)
				{
					Ogre::LogManager::getSingleton().logMessage("attack here2 " + Ogre::StringConverter::toString(m_rAttackTime));
					static_cast<GameObject*>(m_target)->setDamage(m_rAttackPower);
					m_rAttackTime = 0;
				}
				break;
			default:
				sendEvent(EVENT_PUSH,STATE_ATTACK);
				break;
			}
		}
		else
		{
			switch(m_eCurrentMainState)
			{
				m_bRotating = true;
				Ogre::Quaternion q = myDirect.getRotationTo(toTargetDirect);
				m_rotateDegreePerFrame = (q.getYaw().valueDegrees());
				break;
			}
		}

		return true;
	}
	else{
		m_bRotating = false;
		switch(m_eCurrentMainState)
		{
		case STATE_ATTACK:
			sendEvent(EVENT_POP,STATE_ATTACK);
			break;
		default:
			break;
		}
	}

	return true;
}

void Monster::processEvent(EVENT event, STATE state)
{
	m_eCurrentMainState = state;
	switch(state){
		case STATE_ATTACK:
			_executeAttack();
			break;
		case STATE_TRACE:
			_executeTrack();
			break;
		case STATE_EVADE:
			_executeEvade();
			break;
		case STATE_WALK:
			_executeWalk();
			break;
		case STATE_DEAD:
			_executeDie();
			break;
		case STATE_DYING:
			_executeDying();
			break;
		case STATE_IDLE:
			_executeIdle();
			break;
		case STATE_TURNBACK:
			_executeTurnBack();
		default:
			break;
	}
}

void Monster::_executeTurnBack()
{
	m_bUpdating = true;
	m_bRotating = true;
	m_rotateDegreePerFrame = ROTATE_CONST;
	m_vSpeed = Ogre::Vector3::ZERO;
}

bool Monster::_checkWorldBound(float deta)
{
	Ogre::Vector3 hitPos;

	m_terrainCollider->setPositionOffset(getDirection()*100);

	if (!m_terrainCollider->checkTerrainCollision(hitPos))
	{
		switch(m_eCurrentMainState)
		{
		case STATE_TURNBACK:
			break;
		default:
			sendEvent(EVENT_PUSH,STATE_TURNBACK);
			break;
		}
	}
	else{
		switch(m_eCurrentMainState)
		{
		case STATE_TURNBACK:
			sendEvent(EVENT_POP,STATE_TURNBACK);
			break;
		default:
			break;
		}
	}

	m_terrainCollider->setPositionOffset(Ogre::Vector3::ZERO);

	return true;
}

void Monster::_executeAttack(){
	startAnimation(m_strAttckAction,true);

	m_bUpdating = true;
	m_bRotating = false;
	m_vSpeed = Ogre::Vector3::ZERO;
}

void Monster::_executeTrack(){
	startAnimation(m_strWalkAction, m_strWalkActionTop,true);
	m_bUpdating = true;
	m_vSpeed = getDirection()*m_rVelocity;
}

void Monster::_executeDie(){
	m_bFadeOut = true;
}

void Monster::_executeDying()
{
	startAnimation(m_strDeadAction,false);

	m_bUpdating = false;
	m_bRotating = false;
	m_vSpeed = Ogre::Vector3::ZERO;
}

void Monster::_executeIdle(){
	stopAnimation();

	m_vSpeed = Ogre::Vector3::ZERO;
	m_bRotating = false;
	m_bUpdating = true;
}

void Monster::_executeEvade(){
	startAnimation(m_strWalkAction, m_strWalkActionTop,true);

	m_bUpdating = true;
	m_bRotating = true;

	m_vSpeed = Ogre::Vector3::ZERO;
}

void Monster::_executeWalk(){
	startAnimation(m_strWalkAction, m_strWalkActionTop,true);

	m_bUpdating = true;
	m_bRotating = false;
	m_vSpeed = getDirection()*m_rVelocity;
}

bool Monster::update(const Ogre::FrameEvent& evt){
	if(GameObject::update(evt))
	{
		if (m_animationState)
		{
			m_animationState->addTime(evt.timeSinceLastFrame);
		}
		//when dead animation is over, set the Monster to to inactive
		if (m_eCurrentMainState == STATE_DYING && m_animationState->hasEnded())
			sendEvent(EVENT_CHANGE,STATE_DEAD);

		if (m_bFadeOut)
		{
			m_rFadeOutFactor -= 0.01;

			if (m_rFadeOutFactor <= 0){
				m_bActive = false;
				return true;
			}

			int nCnt = m_entity->getNumSubEntities(); 

			for (int i=0;i<nCnt;i++)
			{
				Ogre::MaterialPtr mat = m_entity->getSubEntity(i)->getMaterial();
				Ogre::MaterialPtr _MaterPt;
				_MaterPt=mat->clone(m_entity->getSubEntity(i)->getMaterialName());
				m_entity->getSubEntity(i)->setMaterial(_MaterPt);
				Ogre::Pass * _Pass;
				_Pass =_MaterPt->getTechnique(0)->getPass(0);
				_Pass->setLightingEnabled(true);
				_Pass->setDiffuse(1.0f,1.0f,1.0f,m_rFadeOutFactor);
				_Pass->setSceneBlending(Ogre:: SBT_TRANSPARENT_ALPHA);
				_Pass->setDepthCheckEnabled(false);
			}
		}		
	}
	return true;
}

//-----------------------------------Soilder Monster-------------------------------------------------------------------------

SoilderMonster::SoilderMonster(){
	m_eMyType = TYPE_NORMALMONSTER;
	m_eEnemyType = TYPE_PLAYER;
	m_sceneMgr = GameManager::GetSingletonPtr()->getSceneManager();
	m_strMeshName = "Sinbad.mesh";
	m_strSkeletonName = "Sinbad";
	_initialize();
	//_mergeSkeletonToMesh();
}

void SoilderMonster::reset()
{
	Monster::reset();
	m_rHitPoint = 400;
}

SoilderMonster::~SoilderMonster(){

}
//----------------------------------Jump Monster---------------------------------------------------------------------------
JumpMonster::JumpMonster(){
	m_eMyType = TYPE_JUMPMONSTER;
	m_eEnemyType = TYPE_PLAYER;
	m_rVelocity = MONSTER_WALK_VELOCITY*1.5;
	m_sceneMgr = GameManager::GetSingletonPtr()->getSceneManager();
	m_strMeshName = "Sinbad.mesh";
	m_strWalkAction = "RunBase";
	m_strWalkActionTop = "RunTop";
	m_strAttckAction = "SliceHorizontal";
	m_strDeadAction = "Dance";
	m_strSkeletonName = "Sinbad";
	_initialize();
	//_mergeSkeletonToMesh();
	m_rMaxHitPoint = m_rHitPoint = 600;
	m_rAttackPower = 15;
}

void JumpMonster::reset()
{
	Monster::reset();
	m_rHitPoint = 600;
}

JumpMonster::~JumpMonster(){

}

//------------------------------------MonsterManager------------------------------------------------------------------------

MonsterManager::MonsterManager(){
	m_sceneMgr = GameManager::GetSingletonPtr()->getSceneManager();
	m_recyclePool.clear();
	m_monsterList.clear();
}

MonsterManager::~MonsterManager(){
	std::map<size_t, GameObject*>::iterator iter = m_monsterList.begin();
	while (iter != m_monsterList.end())
	{
		delete (iter->second);
		iter++;
	}

	std::vector<GameObject*>::iterator it = m_recyclePool.begin();
	while (it != m_recyclePool.end())
	{
		delete *it;
		it++;
	}

	m_monsterList.clear();
}

GameObject* MonsterManager::addMonster(ELEMENT_TYPE type){
	GameObject* monster = NULL;
	if (monster = _checkRecyclePool(type))
	{
		return monster;
	}

	switch(type){
	case TYPE_NORMALMONSTER:
		monster = new SoilderMonster();
		m_monsterList.insert(std::map<size_t, GameObject*>::value_type(monster->getID(),monster));
		return monster;
		break;
	case TYPE_JUMPMONSTER:
		monster = new JumpMonster();
		m_monsterList.insert(std::map<size_t, GameObject*>::value_type(monster->getID(),monster));
		return monster;
		break;
	default:
		return NULL;
	}
}

void MonsterManager::_disableMonster(size_t id){
	std::map<size_t, GameObject*>::iterator iter = m_monsterList.find(id);
	//Ogre::LogManager::getSingleton().logMessage("find disable  " + Ogre::StringConverter::toString(id));
	//Ogre::LogManager::getSingleton().logMessage("recycle monster " + getMonsterByID(id)->getName());
	if (iter != m_monsterList.end())
	{
		m_recyclePool.push_back(iter->second);
		iter->second->setPosition(Ogre::Vector3(-1000,-1000,-1000));
		Ogre::LogManager::getSingleton().logMessage("erase " + iter->second->getName());
		m_monsterList.erase(iter);
	}
}

GameObject* MonsterManager::_checkRecyclePool(ELEMENT_TYPE type){
	std::vector<GameObject*>::iterator iter = m_recyclePool.begin();
	while (iter != m_recyclePool.end())
	{
		if ((*iter)->getType() == type){
			m_monsterList.insert(std::map<size_t, GameObject*>::value_type((*iter)->getID(),*iter));
			static_cast<Monster*>((*iter))->reset();
			GameObject* obj = *iter;
			m_recyclePool.erase(iter);
			return obj;
		}
		iter++;
	}
	return NULL;
}

GameObject* MonsterManager::getMonsterByID(size_t id){
	std::map<size_t, GameObject*>::iterator iter = m_monsterList.find(id);
	if (iter != m_monsterList.end())
	{
		return iter->second;
	}

	std::vector<GameObject*>::iterator it = m_recyclePool.begin();
	while (it != m_recyclePool.end())
	{
		if ((*it)->getID() == id)
		{
			return *it;
		}
		it++;
	}

	return NULL;
}

GameObject* MonsterManager::getMonsterByName(Ogre::String name){
	std::map<size_t, GameObject*>::iterator iter = m_monsterList.begin();
	while (iter != m_monsterList.end())
	{
		if (iter->second->getName() == name)
			return iter->second;
		iter++;
	}

	std::vector<GameObject*>::iterator it = m_recyclePool.begin();
	while (it != m_recyclePool.end())
	{
		if ((*it)->getName() == name)
		{
			return *it;
		}
		it++;
	}

	return NULL;
}

bool MonsterManager::update(const Ogre::FrameEvent& evt)
{
	std::vector<size_t> recycleVec;
	recycleVec.clear();

	if (m_monsterList.empty())
	{
		GameManager::GetSingletonPtr()->notifyNewWave();
	}

	std::map<size_t, GameObject*>::iterator it = m_monsterList.begin();
	while (it != m_monsterList.end())
	{
		it->second->update(evt);
		//Ogre::LogManager::getSingleton().logMessage("update monster " +it->second->getName() + " pos " + Ogre::StringConverter::toString(it->second->getPosition()));

		if (!it->second->isActive())
		{
			//Ogre::LogManager::getSingleton().logMessage("put to recycle name" + it->second->getName());
			GameManager::GetSingletonPtr()->notifyMonsterDown(it->second->getType());
			recycleVec.push_back(it->second->getID());
		}

		it++;
	}

	while (!recycleVec.empty())
	{		
		//Ogre::LogManager::getSingleton().logMessage("put to recycle id " + Ogre::StringConverter::toString(recycleVec.back()));
		_disableMonster(recycleVec.back());
		recycleVec.pop_back();
	}

	return true;
}

void MonsterManager::addMonster(GameObject* monster)
{
	m_monsterList.insert(std::map<size_t, GameObject*>::value_type(monster->getID(),monster));
}