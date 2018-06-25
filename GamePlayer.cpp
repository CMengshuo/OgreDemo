#include "GamePlayer.h"
#include "GameManager.h"
#include <OgreManualObject.h>
#include <OgreQuaternion.h>
#include "UIManager.h"

#define PLAYER_MAX_PITCH_DEGREE 40

GamePlayer::GamePlayer()
{
	m_sceneMgr = GameManager::GetSingletonPtr()->getSceneManager();
	//m_strMeshName =  "cangshu_bwjy_cgm_001.mesh";
	//m_strSkeletonName = "cangshu_bwjy_cgm_001_mesh";
	m_strMeshName = "Sinbad.mesh";
	m_strSkeletonName = "Sinbad";
	m_eMyType = TYPE_PLAYER;
	m_rRotateVelocity = 0.3;
	m_rVelocity = 60;
	m_eCurrentDirection = DIRECTION_NULL;
	m_rPitchSum = 0;
	m_rdFOV = Ogre::Radian(Ogre::Degree(120));
	m_eCurrentAttackMode = ATK_NORMAL_MISSILE;
	_initialize();
	//_mergeSkeletonToMesh();
	m_rMaxHitPoint = m_rHitPoint = 2000;
	m_nNormMissleCount = 100;
	m_nTrackMissleCount = 20;
	m_nCombustCount = 10;
}

GamePlayer::~GamePlayer(){
	delete m_qToLeft;
	delete m_boxCollider;
}

void GamePlayer::reset()
{
	m_sceneNode->resetOrientation();
	m_vSpeed = Ogre::Vector3::ZERO;
	m_stateHandler->clearAllStack();
	m_rPitchSum = 0;
}

void GamePlayer::_initialize(){
	Ogre::LogManager::getSingleton().logMessage("****_initialize Player*****");
	m_stateHandler = new StateHandler(this);
	m_entity = m_sceneMgr->createEntity(Ogre::StringConverter::toString(m_eMyType) + "_" + Ogre::StringConverter::toString(m_uMyID),m_strMeshName);
	m_sceneNode = m_sceneMgr->getSceneNode("PlayerRootNode")->createChildSceneNode("player_" + Ogre::StringConverter::toString(m_uMyID));
	m_camNode = m_sceneNode->createChildSceneNode(Ogre::Vector3(0, 1 ,0.3));
	m_sceneNode->attachObject(m_entity);
	m_entity->setQueryFlags(PLAYER_MASK);
	_colliderSetting();
}

void GamePlayer::_colliderSetting()
{
	Ogre::LogManager::getSingleton().logMessage("initiate player collider");

	m_boxCollider = new BoxCollisionHandler(this,MONSTER_MASK | OBSTACLE_MASK, Ogre::SceneManager::ENTITY_TYPE_MASK);
	m_rayCamCollider = new RayCollisionHandler(this,MONSTER_MASK | OBSTACLE_MASK, Ogre::SceneManager::ENTITY_TYPE_MASK);
	m_rayCamCollider->setDetectDistance(1000);

	Ogre::Radian r(Ogre::Degree(90));
	m_qToLeft = new Ogre::Quaternion(r,Ogre::Vector3::UNIT_Y);

/*
#if _DEBUG
	m_manuObj =  m_sceneMgr->createManualObject(); 
	m_manuNode = m_sceneMgr->getRootSceneNode()->createChildSceneNode(); 
	m_manuNode->attachObject(m_manuObj);
#endif
*/
}

Ogre::SceneNode* GamePlayer::getCameraNode()
{
	return m_camNode;
}

bool GamePlayer::keyPressed(const OIS::KeyEvent &arg)
{
	int key = arg.key;
	switch(key)
	{
	//case OIS::KC_SPACE:
	//	if (m_bSupported)
	//	{
	//		m_vSpeed.y += 40;
	//	}
	//	break;
	case OIS::KC_UP:
	case OIS::KC_W:
		m_vSpeed.z = m_rVelocity;
		m_eCurrentDirection = FORWARD;
		break;

	case OIS::KC_S:
	case OIS::KC_DOWN:
		m_vSpeed.z = -m_rVelocity;
		m_eCurrentDirection = BACKFORWARD;
		break;

	case OIS::KC_A:
	case OIS::KC_LEFT:
		m_vSpeed.x = m_rVelocity;
		m_eCurrentDirection = LEFT;
		break;

	case OIS::KC_D:
	case OIS::KC_RIGHT:
		m_vSpeed.x = -m_rVelocity;
		m_eCurrentDirection = RIGHT;
		break;

	default:
		break;
	}

	return true;
}

bool GamePlayer::keyReleased(const OIS::KeyEvent &arg)
{
	int key = arg.key;
	switch(key)
	{
	case OIS::KC_UP:
	case OIS::KC_W:
		m_vSpeed.z = 0;
		m_eCurrentDirection = DIRECTION_NULL;
		break;

	case OIS::KC_S:
	case OIS::KC_DOWN:
		m_vSpeed.z = 0;
		m_eCurrentDirection = DIRECTION_NULL;
		break;

	case OIS::KC_A:
	case OIS::KC_LEFT:
		m_vSpeed.x = 0;
		m_eCurrentDirection = DIRECTION_NULL;
		break;

	case OIS::KC_D:
	case OIS::KC_RIGHT:
		m_vSpeed.x = 0;
		m_eCurrentDirection = DIRECTION_NULL;
		break;

	default:
		break;
	}

	return true;
}

void GamePlayer::camProcess(int cmd, double time){

	//OIS::Mouse* mouse = GameManager::GetSingletonPtr()->GetMouse();

	switch(cmd + 1){
	case 1:
		m_vSpeed.z = m_rVelocity;
		m_eCurrentDirection = FORWARD;
		break;
	case 2:
		m_vSpeed.z = 0;
		m_vSpeed.x = 0;
		m_sceneNode->yaw(Ogre::Degree(m_rRotateVelocity*20),Ogre::Node::TS_WORLD);
		break;
	case 3:
		m_vSpeed.z = 0;
		m_vSpeed.x = 0;
		m_sceneNode->yaw(Ogre::Degree(-m_rRotateVelocity*20),Ogre::Node::TS_WORLD);
		break;
	case 4:
		m_vSpeed.z = -m_rVelocity;
		m_eCurrentDirection = BACKFORWARD;
		break;
	case 5:
		m_vSpeed.z = 0;
		m_vSpeed.x = 0;
		m_eCurrentDirection = DIRECTION_NULL;
		//_executeAttack(OIS::MouseEvent(mouse,mouse->getMouseState()));
		break;
	}
}

bool GamePlayer::mouseMoved(const OIS::MouseEvent &evt)
{
	//camera node should be bound on player's node

	m_sceneNode->yaw(Ogre::Degree(-m_rRotateVelocity*evt.state.X.rel),Ogre::Node::TS_WORLD);

	if (m_rPitchSum <= PLAYER_MAX_PITCH_DEGREE && m_rPitchSum >= -PLAYER_MAX_PITCH_DEGREE)
	{
		if (m_rPitchSum + m_rRotateVelocity*evt.state.Y.rel > PLAYER_MAX_PITCH_DEGREE || m_rPitchSum + m_rRotateVelocity*evt.state.Y.rel < -PLAYER_MAX_PITCH_DEGREE)
		{
			return false;
		}
		m_rPitchSum +=  m_rRotateVelocity*evt.state.Y.rel;
		m_camNode->pitch(Ogre::Degree(m_rRotateVelocity*evt.state.Y.rel),Ogre::Node::TS_LOCAL);
	}

	return true;
}

bool GamePlayer::mouseReleased(const OIS::MouseEvent &evt,OIS::MouseButtonID id)
{
	return true;
}

bool GamePlayer::mousePressed(const OIS::MouseEvent &evt,OIS::MouseButtonID id)
{
	if (id == OIS::MB_Left)
	{
		_executeAttack(evt);
	}
	return true;
}

bool GamePlayer::update(const Ogre::FrameEvent& evt){
	if (!m_bActive)
		return false;

	if (m_animationState)
		{
			m_animationState->addTime(evt.timeSinceLastFrame);
		}

	if (m_bUpdating)
	{
		gravityCheck();

		if (!checkCollision(evt.timeSinceLastFrame))
		{
			m_sceneNode->translate(m_vSpeed*evt.timeSinceLastFrame,Ogre::Node::TS_LOCAL);
		}
	}

	return true;
}

bool GamePlayer::checkCollision(float deta)
{
	Ogre::Vector3 worldSpeedDirection = Ogre::Vector3::ZERO;
	switch(m_eCurrentDirection){
	case FORWARD:
		worldSpeedDirection = getDirection();
		break;
	case BACKFORWARD:
		worldSpeedDirection = -getDirection();
		break;
	case LEFT:
		worldSpeedDirection = *m_qToLeft*getDirection();
		break;
	case RIGHT:
		worldSpeedDirection = -(*m_qToLeft*getDirection());
		break;
	}

	m_terrainCollider->setPositionOffset(worldSpeedDirection*100);
	Ogre::Vector3 hitPos;

	if (!m_terrainCollider->checkTerrainCollision(hitPos))
	{
		m_terrainCollider->setPositionOffset(Ogre::Vector3::ZERO);
		return true;
	}

	m_terrainCollider->setPositionOffset(Ogre::Vector3::ZERO);

	Ogre::String name = "";

	//check collision of next position
	m_boxCollider->setPositionOffset(worldSpeedDirection*m_rVelocity*deta);
	m_boxCollider->checkCollision(name,m_vTargetType);

	Ogre::StringVector strVec = Ogre::StringUtil::split(name,"@");
	GameObject* obj = NULL;
	Ogre::Vector3 MeToTargetVec = Ogre::Vector3::ZERO;

	for (int i=0;i<strVec.size();i++)
	{
		obj = GameManager::GetSingletonPtr()->getGameObjectByName(strVec[i]);
		if (NULL != obj)
		{
			MeToTargetVec = obj->getPosition() - getPosition();
			if (worldSpeedDirection.angleBetween(MeToTargetVec) < m_rdFOV/2)
			{
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
		m_manuObj->position(getPosition() + getPosition()+worldSpeedDirection*1000);  
		m_manuObj->end(); 
	}
#endif
*/
	return false;
}

void GamePlayer::processEvent(EVENT event, STATE state){
	m_eCurrentMainState = state;
	switch(state){
	default:
		break;
	}
}

void GamePlayer::sendEvent(EVENT event, STATE state)
{
	m_stateHandler->sendEvent(event,state);
}

void GamePlayer::notifyFOVData(Ogre::Radian r)
{
	m_rdFOV = r;
}

void GamePlayer::_executeAttack(const OIS::MouseEvent &evt)
{
	size_t id;

	CEGUI::Vector2f mousePos = CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().getPosition();
	Ogre::Ray mouseRay = static_cast<Ogre::Camera*>(m_camNode->getAttachedObject(0))->getCameraToViewportRay(mousePos.d_x / float(evt.state.width), mousePos.d_y / float(evt.state.height));

	Ogre::String name;
	Ogre::StringVector strVec;
	GameObject* obj = NULL;


	switch(m_eCurrentAttackMode)
	{
	case ATK_NORMAL_MISSILE:
		if (m_nNormMissleCount <=0 )
		{
			return;
		}

		id = GameManager::GetSingletonPtr()->playNormalEffect(EFFECT_FIRE_BALL,getPosition() + Ogre::Vector3(0,20,0));
		Ogre::LogManager::getSingleton().logMessage("effect id " + Ogre::StringConverter::toString(id));
		GameManager::GetSingletonPtr()->setEffectSpeed(id,mouseRay.getDirection(),100);
		UIManager::GetSingletonPtr()->setText("btnWeapon1", "Normal Missle: " + Ogre::StringConverter::toString(--m_nNormMissleCount));
		break;
	case ATK_TRACK_MISSILE:
		if (m_nTrackMissleCount <=0 )
		{
			return;
		}

		m_rayCamCollider->setDirection(mouseRay.getDirection());

		if (m_rayCamCollider->checkCollision(name,m_vTargetType))
		{
			strVec = Ogre::StringUtil::split(name,"@");
			obj = NULL;

			for (int i=0;i<strVec.size();i++)
			{
				obj = GameManager::GetSingletonPtr()->getGameObjectByName(strVec[i]);
				if (NULL != obj)
				{
					m_objTarget = obj;
					id = GameManager::GetSingletonPtr()->playTrackEffect(EFFECT_ENERGY_CLUSTER,getPosition(),m_objTarget);
					GameManager::GetSingletonPtr()->setEffectSpeed(id,Ogre::Vector3(0,0,0),100);
					GameManager::GetSingletonPtr()->setEffectScale(id,Ogre::Vector3(0.002,0.002,0.002));
					UIManager::GetSingletonPtr()->setText("btnWeapon2", "Track Missle: " + Ogre::StringConverter::toString(--m_nTrackMissleCount));
					m_target = NULL;
					return;
				}
			}
		}
		break;

	case ATK_COMBUSTION:
		if (m_nCombustCount <=0 )
		{
			return;
		}

		m_rayCamCollider->setDirection(mouseRay.getDirection());

		if (m_rayCamCollider->checkCollision(name,m_vTargetType))
		{
			strVec = Ogre::StringUtil::split(name,"@");
			obj = NULL;

			for (int i=0;i<strVec.size();i++)
			{
				obj = GameManager::GetSingletonPtr()->getGameObjectByName(strVec[i]);
				if (NULL != obj)
				{
					m_objTarget = obj;
					id = GameManager::GetSingletonPtr()->playAttachedEffect(EFFECT_FIRE_PILLAR,m_objTarget);
					UIManager::GetSingletonPtr()->setText("btnWeapon3", "Fire Combustion: " + Ogre::StringConverter::toString(--m_nCombustCount));
					m_target = NULL;
					return;
				}
			}
		}
		break;
	}
}

void GamePlayer::refreshAmunition(int norm, int track, int combust)
{
	m_nNormMissleCount += norm;
	m_nTrackMissleCount += track;
	m_nCombustCount += combust;
	UIManager::GetSingletonPtr()->setText("btnWeapon3", "Fire Combustion: " + Ogre::StringConverter::toString(m_nCombustCount));
	UIManager::GetSingletonPtr()->setText("btnWeapon2", "Track Missle: " + Ogre::StringConverter::toString(m_nTrackMissleCount));
	UIManager::GetSingletonPtr()->setText("btnWeapon1", "Normal Missle: " + Ogre::StringConverter::toString(m_nNormMissleCount));
}

void GamePlayer::setDamage(Ogre::Real dmg)
{
	GameObject::setDamage(dmg);
	float per = m_rHitPoint/m_rMaxHitPoint;
	//Ogre::LogManager::getSingleton().logMessage("player health " + Ogre::StringConverter::toString(per));
	//Ogre::LogManager::getSingleton().logMessage("player max " + Ogre::StringConverter::toString(m_rMaxHitPoint));
	if (per > 0)
	{
		GameManager::GetSingletonPtr()->refreshPlayerHitPointBar(per);
	}
	else
	{
		m_bActive = false;
	}
}

void GamePlayer::switchAttackMode(ATTACKMODE eAm)
{
	m_eCurrentAttackMode = eAm;
}