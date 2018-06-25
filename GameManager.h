//Build Game logic here
#ifndef _GameManager_h_
#define _GameManager_h_
#include "BasicApp.h"
#include "Template.h"
#include <CEGUI/CEGUI.h>

class GameElement;
class GamePlayer;
class GameObject;
enum ELEMENT_TYPE;

enum GameState {
	GAME_INITIALIZE,
	GAME_PLAY,
	GAME_STOP,
	GAME_OVER,
};

//Access to EffectManager and MonsterManager Singleton should go through GameManager

//effect name, used to bind with certain resources
enum EFFECT_RESOURCE {
	EFFECT_ENERGY_CLUSTER,
	EFFECT_FIRE_PILLAR,
	EFFECT_ENERGY_SPLASH,
	EFFECT_SWARM,
	EFFECT_FIRE_BALL,
};

class GameManager : public BasicApp, public MySingleton<GameManager> {
private:
	GamePlayer* m_player;
	Ogre::SceneNode* m_debugCamNode;
	GameState m_eCurrentGameState;
	size_t m_uWave;

protected:
	GameManager();
	virtual ~GameManager();
	//to get access to its derived class constructor
	friend class MySingleton<GameManager>;

public:
	virtual Ogre::SceneManager* getSceneManager();
	virtual GameObject* getGameObjectByName(Ogre::String targetName);

protected:
	virtual bool _createCamera();
	virtual bool _createScene();
	virtual bool _initializeMonster();
	virtual bool _initializePlayer();
	virtual bool _initializeEffect();
	virtual bool _initializeUI();
	virtual bool _createViewport();

	virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	//KeyListener
	virtual bool keyPressed(const OIS::KeyEvent &arg);
	virtual bool keyReleased(const OIS::KeyEvent &arg);
	//MouseListener
	virtual bool mouseMoved(const OIS::MouseEvent &evt);
	virtual bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
	virtual bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

	virtual void camProcess(int cmd, double time);

public:
	//Only EffectManager could get access to effect instance, costs more time but is safer
	virtual size_t playAttachedEffect(EFFECT_RESOURCE etype, GameElement* elt, const Ogre::Vector3& offset = Ogre::Vector3::ZERO);
	virtual size_t playNormalEffect(EFFECT_RESOURCE etype, const Ogre::Vector3& startPos);
	virtual size_t playTrackEffect(EFFECT_RESOURCE etype, const Ogre::Vector3& startPos, GameElement* target);
	virtual void stopEffectByIndex(size_t index);
	virtual void setEffectSpeed(const size_t id, const Ogre::Vector3& vec, const Ogre::Real& v);
	virtual void setEffectRotate(const size_t id, const Ogre::Vector3& axis, const Ogre::Real& w);
	virtual void setEffectScale(const size_t id, const Ogre::Vector3& scale);
	virtual void setEffectLivingTime(const size_t id, const Ogre::Real time);
	Ogre::TerrainGroup* getTerrainGroup();
	virtual void attachCameraToNode(Ogre::SceneNode* node, const Ogre::Vector3& lookAt);
	virtual void refreshPlayer(GamePlayer* player);
	virtual void refreshPlayerHitPointBar(float per);
	GameState getCurrentGameState();
	virtual void notifyNewWave();
	virtual void notifyMonsterDown(ELEMENT_TYPE eType);

	//UI calling functions
public:
	virtual bool quitGame(const CEGUI::EventArgs &e);
	virtual bool onWeapon1Clicked(const CEGUI::EventArgs &e);
	virtual bool onWeapon2Clicked(const CEGUI::EventArgs &e);
	virtual bool onWeapon3Clicked(const CEGUI::EventArgs &e);
	virtual bool onLevelBtnClicked(const CEGUI::EventArgs &e);
};

#endif

