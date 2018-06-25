#ifndef _UIManager_h_
#define _UIManager_h_

//the UI part should be handled by scripts. do not have time to do this. just use UI manager

#include "Template.h"
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

class GameManager;
class GameObject;

class UIManager: public MySingleton<UIManager>
{
protected:
	UIManager();
	virtual ~UIManager();
	friend class MySingleton<UIManager>;

protected:
	CEGUI::Renderer* m_guiRenderer;
	CEGUI::WindowManager* m_windowMgr;
	CEGUI::Window* m_rootWindow;
	std::map<CEGUI::String, CEGUI::Window*> m_windowMap;

public:
	virtual bool initialize();
	virtual bool mouseMoved(const OIS::MouseEvent& arg);
	virtual bool mousePressed(const OIS::MouseEvent& arg, OIS::MouseButtonID id);
	virtual bool mouseReleased(const OIS::MouseEvent& arg, OIS::MouseButtonID id);
	virtual bool keyPressed(const OIS::KeyEvent &arg);
	virtual bool keyReleased(const OIS::KeyEvent &arg);
	virtual bool update(float deta);
	//create a button, given the text on the button, its position and size, and the function called when clicked
	virtual bool AddButton(CEGUI::String btnName, CEGUI::String btnText, float xRelPos, float yRelPos, int width, int height, bool (GameManager::*pFc)(const CEGUI::EventArgs& args), bool visible = true);
	virtual bool AddProgressBar(CEGUI::String barName, float xRelPos, float yRelPos, int width, int height, bool visible = true);
	virtual void setProgress(CEGUI::String barName, float progress);
	virtual void setVisible(CEGUI::String windowName,  bool visible);
	virtual void setText(CEGUI::String windowName, CEGUI::String text);
};
#endif