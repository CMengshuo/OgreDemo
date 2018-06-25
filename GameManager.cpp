#include "GameManager.h"
#include "MonsterManager.h"
#include "GamePlayer.h"
#include "EffectManager.h"
#include "SwapWalker.h"
#include "UIManager.h"
#include <time.h>
#include <cstdlib>

#define SOILDERMONSTER_NUM 1
#define JUMPMONSTER_NUM 1
#define SWAPPLAYER_NUM 1

GameManager::GameManager() {
	m_player = NULL;
	m_eCurrentGameState = GAME_INITIALIZE;
	m_uWave = 0;
}

GameManager::~GameManager() {
	if (m_player)
	{
		delete m_player;
		m_player = NULL;
	}


	if (m_sceneMgr)
	{
		m_sceneMgr->clearScene();
	}

	m_sceneMgr->destroyAllParticleSystems();
	m_sceneMgr->destroyAllEntities();
}

//for outer use
void GameManager::attachCameraToNode(Ogre::SceneNode* node, const Ogre::Vector3& lookAt)
{
	m_camera->detachFromParent();
	m_camera->lookAt(m_player->getDirection());
	m_camNode = node;
	m_camNode->attachObject(m_camera);
}

//only for swap player game
void GameManager::refreshPlayer(GamePlayer* player) {
	m_player = player;
}

bool GameManager::_createCamera() {

	//bind the camera with the player
	m_camera = m_sceneMgr->createCamera("PlayCam");
	m_camera->setPosition(Ogre::Vector3(0, 0, 0));
	m_camera->setNearClipDistance(2);
	m_camera->setFarClipDistance(50000);

	m_debugCamNode = m_sceneMgr->getRootSceneNode()->createChildSceneNode("DebugCamNode");
	m_debugCamNode->_setDerivedPosition(Ogre::Vector3(0, 300, 0));

	return true;
}

bool GameManager::_createViewport()
{
	BasicApp::_createViewport();

	//initialize UI here, for we need the viewport data
	_initializeUI();
	m_eCurrentGameState = GAME_PLAY;

	return true;
}

Ogre::SceneManager* GameManager::getSceneManager()
{
	return m_sceneMgr;
}

void GameManager::refreshPlayerHitPointBar(float per)
{
	UIManager::GetSingletonPtr()->setProgress("barPlayerLife", per);
}

bool GameManager::_createScene() {
	BasicApp::_createScene();

	//Node to attach all the game objects
	m_sceneMgr->getRootSceneNode()->createChildSceneNode("GameObjectRootNode");
	//Node to attach all the particle nodes
	m_sceneMgr->getRootSceneNode()->createChildSceneNode("EffectRootNode");
	//Node to attach player
	m_sceneMgr->getSceneNode("GameObjectRootNode")->createChildSceneNode("PlayerRootNode");
	//Node to attach monsters
	m_sceneMgr->getSceneNode("GameObjectRootNode")->createChildSceneNode("MonsterRootNode");

	_initializeEffect();
	_initializePlayer();
	_initializeMonster();
	return true;
}

bool GameManager::_initializeUI()
{
	Ogre::Real screenWidth = m_camera->getViewport()->getActualWidth();
	Ogre::Real screenHeight = m_camera->getViewport()->getActualHeight();

	UIManager::GetSingletonPtr()->initialize();
	UIManager::GetSingletonPtr()->AddButton("btnQuit", "QuitGame", (screenWidth - 200) / 2, screenHeight / 2, 200, 50, &GameManager::quitGame, false);
	UIManager::GetSingletonPtr()->AddButton("btnGameOver", "", (screenWidth - 400) / 2, screenHeight / 2 - 100, 400, 50, &GameManager::onLevelBtnClicked, false);
	UIManager::GetSingletonPtr()->AddButton("btnWeapon1", "Normal Missle: " + Ogre::StringConverter::toString(m_player->getNormMissleCount()), 30, 20, 200, 50, &GameManager::onWeapon1Clicked, true);
	UIManager::GetSingletonPtr()->AddButton("btnWeapon2", "Track Missle: " + Ogre::StringConverter::toString(m_player->getTrackMissleCount()), 270, 20, 200, 50, &GameManager::onWeapon2Clicked, true);
	UIManager::GetSingletonPtr()->AddButton("btnWeapon3", "Fire Combustion: " + Ogre::StringConverter::toString(m_player->getCombustCount()), 510, 20, 200, 50, &GameManager::onWeapon3Clicked, true);
	UIManager::GetSingletonPtr()->AddButton("btnLevel", "Current Level: 0", 750, 20, 200, 50, &GameManager::onLevelBtnClicked, true);

	UIManager::GetSingletonPtr()->AddProgressBar("barPlayerLife", (screenWidth - 300) / 2, screenHeight - 150, 300, 50);

	return true;
}

bool GameManager::_initializePlayer() {
	//_initialize player here

	//m_player = new SwapWalker(true);
	m_player = new GamePlayer();
	m_player->perceiveWorld();
	m_player->setPosition(Ogre::Vector3(300, 30, 300));
	m_player->setScale(Ogre::Vector3(1, 1, 1));
	m_player->sendEvent(EVENT_PUSH, STATE_IDLE);

	//attach camera node
	m_camNode = m_player->getCameraNode();
	m_camera->lookAt(m_player->getDirection());
	m_camNode->attachObject(m_camera);
	if (m_camera->getCullingFrustum())
	{
		m_player->notifyFOVData(m_camera->getCullingFrustum()->getFOVy());
	}

	//for SwapPlayer only. don't have to update m_play by doing so. Could know which ancestor data is it using
	//MonsterManager::GetSingletonPtr()->addMonster(m_player);
	return true;
}

bool GameManager::_initializeMonster() {
	//Objects should be initialized here because scene manager will not be created in the constructor function

	Monster* monster;
	for (int i = 0; i<SOILDERMONSTER_NUM; i++)
	{
		monster = static_cast<SoilderMonster*>(MonsterManager::GetSingletonPtr()->addMonster(TYPE_NORMALMONSTER));
		monster->perceiveWorld();
		monster->setPosition(Ogre::Vector3(100 + i * 30, 10, 100 + i * 30));
		monster->getNode()->yaw(Ogre::Radian(Ogre::Degree(19 * i)));
		monster->setScale(Ogre::Vector3(2, 2, 2));
		monster->sendEvent(EVENT_PUSH, STATE_WALK);
	}

	for (int i = 0; i<JUMPMONSTER_NUM; i++)
	{
		monster = static_cast<JumpMonster*>(MonsterManager::GetSingletonPtr()->addMonster(TYPE_JUMPMONSTER));
		monster->perceiveWorld();
		monster->setPosition(Ogre::Vector3(100 + i * 30, 10, 100 - i * 30));
		monster->getNode()->yaw(Ogre::Radian(Ogre::Degree(36 * i)));
		monster->setScale(Ogre::Vector3(2, 2, 2));
		monster->sendEvent(EVENT_PUSH, STATE_WALK);
		//monster->getNode()->showBoundingBox(true);
	}


	//for (int i=0; i<SWAPPLAYER_NUM;i++)
	//{
	//	monster = new SwapWalker();
	//	MonsterManager::GetSingletonPtr()->addMonster(monster);
	//	monster->perceiveWorld();
	//	monster->setPosition(Ogre::Vector3(1600 - i*50, 10, 2100 + i*30));
	//	monster->getNode()->yaw(Ogre::Radian(Ogre::Degree(-80*i)));
	//	monster->setScale(Ogre::Vector3(16,16,16));
	//	monster->sendEvent(EVENT_PUSH,STATE_WALK);
	//}

	//size_t id = playTrackEffect(EFFECT_ENERGY_CLUSTER,Ogre::Vector3(1600, 100, 1000),JMonster);
	//setEffectScale(id,Ogre::Vector3(0.05, 0.0001, 0.05));
	//setEffectSpeed(id,Ogre::Vector3::ZERO,200);

	return true;
}

void GameManager::notifyNewWave()
{
	UIManager::GetSingletonPtr()->setText("btnLevel", "Current Level: " + Ogre::StringConverter::toString(++m_uWave));

	Monster* monster;
	for (int i = 0; i<SOILDERMONSTER_NUM + m_uWave * 2; i++)
	{
		monster = static_cast<SoilderMonster*>(MonsterManager::GetSingletonPtr()->addMonster(TYPE_NORMALMONSTER));
		monster->perceiveWorld();
		monster->setPosition(Ogre::Vector3(100, 2, 100));
		monster->setScale(Ogre::Vector3(2, 2, 2));
		monster->sendEvent(EVENT_PUSH, STATE_WALK);
	}

	for (int i = 0; i<JUMPMONSTER_NUM + m_uWave * 2; i++)
	{
		monster = static_cast<JumpMonster*>(MonsterManager::GetSingletonPtr()->addMonster(TYPE_JUMPMONSTER));
		monster->perceiveWorld();
		monster->setPosition(Ogre::Vector3(-100, 2, -100));
		monster->setScale(Ogre::Vector3(2, 2, 2));
		monster->sendEvent(EVENT_PUSH, STATE_WALK);
	}
}


bool GameManager::_initializeEffect()
{
	//map effect names to particle files
	EffectManager::GetSingletonPtr()->registerEffect(EFFECT_ENERGY_CLUSTER, "Examples/GreenyNimbus");
	EffectManager::GetSingletonPtr()->registerEffect(EFFECT_FIRE_PILLAR, "Examples/FireSplash");
	EffectManager::GetSingletonPtr()->registerEffect(EFFECT_ENERGY_SPLASH, "Examples/PurpleFountain");
	EffectManager::GetSingletonPtr()->registerEffect(EFFECT_SWARM, "Examples/Swarm");
	EffectManager::GetSingletonPtr()->registerEffect(EFFECT_FIRE_BALL, "Examples/JetEngine2");
	return true;
}

bool GameManager::frameRenderingQueued(const Ogre::FrameEvent& evt) {

	if (m_Window->isClosed())
	{
		return false;
	}

	m_Keyboard->capture();
	m_Mouse->capture();

	//UI update at all states
	UIManager::GetSingletonPtr()->update(evt.timeSinceLastFrame);

	switch (m_eCurrentGameState)
	{
	case GAME_INITIALIZE:
		break;
	case GAME_PLAY:
		//player update
		m_player->update(evt);

		//monster update
		MonsterManager::GetSingletonPtr()->update(evt);

		//effect update
		EffectManager::GetSingletonPtr()->update(evt.timeSinceLastFrame);

		if (!m_player->isActive())
		{
			m_eCurrentGameState = GAME_OVER;
			UIManager::GetSingletonPtr()->setText("btnGameOver", "Game Over, Final Level: " + Ogre::StringConverter::toString(m_uWave));
			UIManager::GetSingletonPtr()->setVisible("btnQuit", true);
			UIManager::GetSingletonPtr()->setVisible("btnGameOver", true);
		}

		m_rCurrentCamTime += evt.timeSinceLastFrame;
		if (m_rCurrentCamTime >= CAM_CHECK_SPAN)
		{
			int result = m_camController->runTesting();
			camProcess(result, m_rCurrentCamTime);
			m_rCurrentCamTime = 0.0;
		}
		break;
	case GAME_STOP:
		break;
	case GAME_OVER:
		break;
	}

	return true;
}

void GameManager::camProcess(int cmd, double time) {
	m_player->camProcess(cmd, time);
}

void GameManager::notifyMonsterDown(ELEMENT_TYPE eType)
{
	srand(time(NULL));

	int ran = rand() % 100;
	int nTrack = 0;
	int nComb = 0;

	switch (eType)
	{
	case TYPE_JUMPMONSTER:
		if (ran >= 90)
		{
			nComb = 1;
		}
		else if (ran <= 10)
		{
			nTrack = 1;
		}
		m_player->refreshAmunition(2, nTrack, nComb);
		break;
	case TYPE_NORMALMONSTER:
		if (ran >= 95)
		{
			nComb = 1;
		}
		else if (ran <= 5)
		{
			nTrack = 1;
		}
		m_player->refreshAmunition(3, nTrack, nComb);
		break;
	}
}

size_t  GameManager::playAttachedEffect(EFFECT_RESOURCE etype, GameElement* elt, const Ogre::Vector3& offset)
{
	return EffectManager::GetSingletonPtr()->addAttachedEffect(etype, elt, offset);
}

size_t GameManager::playNormalEffect(EFFECT_RESOURCE etype, const Ogre::Vector3& startPos)
{
	return EffectManager::GetSingletonPtr()->addNormalEffect(etype, startPos);
}

size_t GameManager::playTrackEffect(EFFECT_RESOURCE etype, const Ogre::Vector3& startPos, GameElement* target)
{
	return EffectManager::GetSingletonPtr()->addTrackEffect(etype, startPos, target);
}

void GameManager::stopEffectByIndex(size_t index)
{
	EffectManager::GetSingletonPtr()->stopEffectByIndex(index);
}

void GameManager::setEffectRotate(const size_t id, const Ogre::Vector3& axis, const Ogre::Real& w)
{
	EffectManager::GetSingletonPtr()->setEffectRotate(id, axis, w);
}

void GameManager::setEffectSpeed(const size_t id, const Ogre::Vector3& vec, const Ogre::Real& v)
{
	EffectManager::GetSingletonPtr()->setEffectSpeed(id, vec, v);
}

void GameManager::setEffectLivingTime(const size_t id, const Ogre::Real time)
{
	EffectManager::GetSingletonPtr()->setEffectLivingTime(id, time);
}

void GameManager::setEffectScale(const size_t id, const Ogre::Vector3& scale)
{
	EffectManager::GetSingletonPtr()->setEffectScale(id, scale);
}

GameState GameManager::getCurrentGameState()
{
	return m_eCurrentGameState;
}

bool GameManager::keyPressed(const OIS::KeyEvent &arg)
{
	m_player->keyPressed(arg);

	switch (arg.key)
	{
#if _DEBUG
	case OIS::KC_C:
		m_camera->detachFromParent();
		m_debugCamNode->attachObject(m_camera);
		m_camera->lookAt(m_player->getPosition());
		break;

#endif

	case OIS::KC_ESCAPE:
		if (m_eCurrentGameState == GAME_PLAY)
		{
			m_eCurrentGameState = GAME_STOP;
			UIManager::GetSingletonPtr()->setVisible("btnQuit", true);
		}
		else if (m_eCurrentGameState == GAME_STOP)
		{
			m_eCurrentGameState = GAME_PLAY;
			CEGUI::System &sys = CEGUI::System::getSingleton();

			sys.getDefaultGUIContext().injectMousePosition(m_camera->getViewport()->getActualWidth() / 2, m_camera->getViewport()->getActualHeight() / 2);

			UIManager::GetSingletonPtr()->setVisible("btnQuit", false);
		}
		break;
	case OIS::KC_1:
		m_player->switchAttackMode(ATK_NORMAL_MISSILE);
		break;
	case OIS::KC_2:
		m_player->switchAttackMode(ATK_TRACK_MISSILE);
		break;
	case OIS::KC_3:
		m_player->switchAttackMode(ATK_COMBUSTION);
		break;
	}

	UIManager::GetSingletonPtr()->keyPressed(arg);
	return BasicApp::keyPressed(arg);
}

bool GameManager::keyReleased(const OIS::KeyEvent &arg)
{
	UIManager::GetSingletonPtr()->keyReleased(arg);
	return m_player->keyReleased(arg);
}

bool GameManager::mouseMoved(const OIS::MouseEvent &evt)
{
	UIManager::GetSingletonPtr()->mouseMoved(evt);
	switch (m_eCurrentGameState)
	{
	case GAME_STOP:
	case GAME_OVER:
		break;
	case GAME_PLAY:
		return m_player->mouseMoved(evt);
		break;
	}

	return true;
}

bool GameManager::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	UIManager::GetSingletonPtr()->mouseReleased(evt, id);
	if (m_eCurrentGameState == GAME_PLAY)
	{
		m_player->mouseReleased(evt, id);
	}
	return true;
}

bool GameManager::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
{
	UIManager::GetSingletonPtr()->mousePressed(evt, id);
	if (m_eCurrentGameState == GAME_PLAY)
	{
		m_player->mousePressed(evt, id);
	}

	return true;
}

GameObject* GameManager::getGameObjectByName(Ogre::String targetName)
{
	if (m_player->getName() == targetName)
		return m_player;

	return MonsterManager::GetSingletonPtr()->getMonsterByName(targetName);
}

Ogre::TerrainGroup* GameManager::getTerrainGroup()
{
	return m_TerrainGroup;
}

//------------------------UI function--------------------------------------------

bool GameManager::quitGame(const CEGUI::EventArgs &e)
{
	m_Window->destroy();
	exit(1);
	return true;
}

bool GameManager::onWeapon1Clicked(const CEGUI::EventArgs &e)
{
	m_player->switchAttackMode(ATK_NORMAL_MISSILE);
	return true;
}

bool GameManager::onWeapon2Clicked(const CEGUI::EventArgs &e)
{
	m_player->switchAttackMode(ATK_TRACK_MISSILE);
	return true;
}

bool GameManager::onWeapon3Clicked(const CEGUI::EventArgs &e)
{
	m_player->switchAttackMode(ATK_COMBUSTION);
	return true;
}

bool GameManager::onLevelBtnClicked(const CEGUI::EventArgs &e)
{
	return true;
}


//------------------------UI function--------------------------------------------

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
	int main(int argc, char *argv[])
#endif
	{
		try
		{
			GameManager::GetSingletonPtr()->go();
		}
		catch (Ogre::Exception& e) {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
			std::cerr << "An exception has occured:" << e.getFullDescription().c_str() << std::end();
#endif		
		}

		return 0;
	}

#ifdef __cplusplus
}
#endif