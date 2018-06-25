#include "GameObject.h"
#include "GameManager.h"

size_t GameObject::ms_uID = 0;
#define GRAVITY_FACTOR 0.1

GameObject::GameObject()
{
	m_uMyID = ms_uID++;
	m_eMyType = TYPE_NULL;
	m_vSpeed = Ogre::Vector3::ZERO;
	m_animationState = NULL;
	m_additionalAnimationState = NULL;
	m_bUpdating = true;
	m_bRotating = false;
	m_entity = NULL;
	m_sceneNode = NULL;
	m_sceneMgr = NULL;
	m_rLastCheckTime = 0;
	m_eCurrentMainState = STATE_NULL;
	m_stateHandler = NULL;
	m_terrainCollider = NULL;
	m_bActive = true;
	m_target = NULL;
	m_rVelocity = 0;
	m_rRotateVelocity = ROTATE_CONST;
	m_bSupported = false;
	m_vTargetType.clear();
	m_rotateDegreePerFrame = ROTATE_CONST;
	m_rMaxHitPoint = m_rHitPoint = -1000;
}

GameObject::~GameObject()
{
	delete m_stateHandler;
	delete m_terrainCollider;
}

void GameObject::_initialize()
{
	m_stateHandler = new StateHandler(this);
	Ogre::LogManager::getSingleton().logMessage("****creating object: " + Ogre::StringConverter::toString(m_eMyType) + "_" + Ogre::StringConverter::toString(m_uMyID) + "****");
	m_entity = m_sceneMgr->createEntity(Ogre::StringConverter::toString(m_eMyType) + "_" + Ogre::StringConverter::toString(m_uMyID),m_strMeshName);
	m_sceneNode = m_sceneMgr->getSceneNode("MonsterRootNode")->createChildSceneNode("obj_" + Ogre::StringConverter::toString(m_uMyID));
	m_sceneNode->attachObject(m_entity);
	m_sceneNode->setPosition(Ogre::Vector3::ZERO);
	_colliderSetting();
}

void GameObject::_colliderSetting()
{

}

Ogre::Real GameObject::getHitPoint()
{
	return m_rHitPoint;
}

Ogre::Real GameObject::getMaxPoint()
{
	return m_rMaxHitPoint;
}

void GameObject::_mergeSkeletonToMesh(){
	//default action -- attacks.
	m_entity->getMesh()->setSkeletonName(m_strSkeletonName + ".skeleton");

	if (m_entity->hasSkeleton())
	{
		Ogre::SkeletonInstance *skeleton = m_entity->getSkeleton();
		skeleton->addLinkedSkeletonAnimationSource(m_strSkeletonName+"_walk.skeleton");
		skeleton->addLinkedSkeletonAnimationSource(m_strSkeletonName+"_dead.skeleton");

		switch (m_eMyType)
		{
		case TYPE_JUMPMONSTER:
			skeleton->addLinkedSkeletonAnimationSource(m_strSkeletonName+"_run.skeleton");
			break;
		}		
		skeleton->_refreshAnimationState(m_entity->getAllAnimationStates());
	}
	m_entity->_initialise(true);
}

void GameObject::perceiveWorld()
{
	if (!m_terrainCollider)
	{
		m_terrainCollider = new TerrainCollisionHandler(this,GameManager::GetSingletonPtr()->getTerrainGroup());
	}
}

Ogre::Entity* GameObject::getEntity(){return m_entity;}

Ogre::String GameObject::getName(){return m_entity->getName();}

bool GameObject::update(const Ogre::FrameEvent& evt){
	if (!m_bActive)
		return false;


	//Trace traget, change the orientation of the object
	if (m_eCurrentMainState == STATE_TRACE && m_target)
	{
		_traceTarget(m_target, evt.timeSinceLastFrame);
	}

	//Reduce collision detect times
	m_rLastCheckTime += evt.timeSinceLastFrame;
	if (m_rLastCheckTime >= DETECT_SPAN)
	{
		m_rLastCheckTime = 0;
		checkCollision(evt.timeSinceLastFrame);
	}

	if (m_bUpdating)
	{
		if (m_bRotating)
		{
			m_sceneNode->yaw(m_rotateDegreePerFrame);
			if (!(m_vSpeed.x == 0 && m_vSpeed.z == 0))
			{
				Ogre::Real Yspeed = m_vSpeed.y;
	
				m_vSpeed = getDirection()*m_rVelocity;
	
				m_vSpeed.y = Yspeed;
			}
		}

		gravityCheck();

		if (getPosition().y < -200)
		{
			m_bActive = false;//fall into darkness
		}
		
		m_sceneNode->translate(m_vSpeed*evt.timeSinceLastFrame);

		if (m_animationState != NULL && m_additionalAnimationState != NULL) {
			m_animationState->addTime(evt.timeSinceLastFrame * 0.05f);
			m_additionalAnimationState->addTime(evt.timeSinceLastFrame * 0.8f);
		}
	}
	return true;
}

bool GameObject::checkCollision(float deta){return true;}

bool GameObject::gravityCheck(){
	Ogre::Vector3 hitPos;
	Ogre::Vector3 Pos = m_sceneNode->_getDerivedPosition();

	if (m_terrainCollider->checkTerrainCollision(hitPos))
	{
		Ogre::Real intersectHeight = hitPos.y;
		if ((intersectHeight + 10.0) > Pos.y)
		{
			Pos.y = intersectHeight + 10.0;
			m_sceneNode->_setDerivedPosition(Pos);
			m_vSpeed.y = 0;
		}
		else{
			m_vSpeed.y -= GRAVITY_FACTOR;
		}
	}
	else{
		m_vSpeed.y -= GRAVITY_FACTOR;
	}

	return m_bSupported = (m_vSpeed.y == 0);
}

void GameObject::startAnimation(Ogre::String AnimationName, bool loop){
	//stop current action
	stopAnimation();

	m_animationState = m_entity->getAnimationState(AnimationName);  
	m_animationState->setLoop( loop );
	m_animationState->setEnabled( true );
}

void GameObject::startAnimation(Ogre::String AnimationName, Ogre::String AdditionalAnimationName, bool loop) {
	//stop current action
	stopAnimation();

	m_entity->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
	m_animationState = m_entity->getAnimationState(AnimationName);
	m_animationState->setLoop(loop);
	m_animationState->setEnabled(true);

	m_additionalAnimationState = m_entity->getAnimationState(AdditionalAnimationName);
	m_additionalAnimationState->setLoop(loop);
	m_additionalAnimationState->setEnabled(true);

}

void GameObject::stopAnimation(){
	if (m_animationState)
	{
		m_animationState->setEnabled(false);
		m_animationState = NULL;
	}
}

void GameObject::sendEvent(EVENT event, STATE state)
{
	m_stateHandler->sendEvent(event,state);
}

void GameObject::processEvent(EVENT event, STATE state)
{

}

size_t GameObject::getID(){return m_uMyID;}

void GameObject::setVisible(){
	m_sceneNode->setVisible(false);
}

void GameObject::_traceTarget(GameElement* target, float deta){
	Ogre::Vector3 targetPos = target->getPosition();
	Ogre::Vector3 myPos = m_sceneNode->_getDerivedPosition();
	Ogre::Vector3 myDirect = getDirection();
	Ogre::Vector3 toTargetDirect = targetPos - myPos;

	Ogre::Quaternion q = myDirect.getRotationTo(toTargetDirect,Ogre::Vector3::UNIT_Y);
	m_sceneNode->yaw(q.getYaw());

	Ogre::Real ySpeed = m_vSpeed.y;
	m_vSpeed = getDirection()*m_rVelocity;
	m_vSpeed.y = ySpeed;
}

ELEMENT_TYPE GameObject::getType()
{
	return m_eMyType;
}

void GameObject::setRotateDegreePerFrame(Ogre::Degree degree)
{
	m_rotateDegreePerFrame = degree;
}

void GameObject::setDamage(Ogre::Real dmg)
{
	//indestructible
	if (m_rHitPoint == -1000)
	{
		return;
	}

	m_rHitPoint -= dmg;

	if (m_rHitPoint <= 0)
	{
		sendEvent(EVENT_CHANGE,STATE_DYING);
	}
}