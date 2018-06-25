#include "UIManager.h"
#include "GameManager.h"

CEGUI::MouseButton convertButton(OIS::MouseButtonID buttonID)
{
	switch (buttonID)
	{
	case OIS::MB_Left:
		return CEGUI::LeftButton;

	case OIS::MB_Right:
		return CEGUI::RightButton;

	case OIS::MB_Middle:
		return CEGUI::MiddleButton;

	default:
		return CEGUI::LeftButton;
	}
}

UIManager::UIManager(){}

UIManager::~UIManager(){}

bool UIManager::update(float deta)
{
	CEGUI::System::getSingleton().injectTimePulse(deta);
	return true;
}

bool UIManager::initialize()
{
	m_guiRenderer = &CEGUI::OgreRenderer::bootstrapSystem();

	//CEGUI::Imageset::setDefaultResourceGroup("Imagesets");
	//CEGUI::Font::setDefaultResourceGroup("Fonts");
	//CEGUI::Scheme::setDefaultResourceGroup("Schemes");
	//CEGUI::WidgetLookManager::setDefaultResourceGroup("LookNFeel");
	//CEGUI::WindowManager::setDefaultResourceGroup("Layouts");

	m_windowMgr = CEGUI::WindowManager::getSingletonPtr();
	m_rootWindow = m_windowMgr->createWindow("DefaultWindow", "WINDOW/ROOT");
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(m_rootWindow);

	CEGUI::SchemeManager::getSingleton().createFromFile((CEGUI::utf8*)"TaharezLook.scheme");
	CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");
	//CEGUI::MouseCursor::getSingleton().setImage("TaharezLook", "MouseArrow");

	return true;
}


//-------------------------------------------------------------------------------------
bool UIManager::keyPressed( const OIS::KeyEvent &arg )
{
	CEGUI::System &sys = CEGUI::System::getSingleton();
	sys.getSingleton().getDefaultGUIContext().injectKeyDown(static_cast<CEGUI::Key::Scan>(static_cast<int>(arg.key)));
	sys.getSingleton().getDefaultGUIContext().injectChar(arg.text);
	return true;
}
//-------------------------------------------------------------------------------------
bool UIManager::keyReleased( const OIS::KeyEvent &arg )
{
	CEGUI::System::getSingleton().getDefaultGUIContext().injectKeyUp(static_cast<CEGUI::Key::Scan>(static_cast<int>(arg.key)));
	return true;
}
//-------------------------------------------------------------------------------------
bool UIManager::mouseMoved( const OIS::MouseEvent &arg )
{
	GameState state = GameManager::GetSingletonPtr()->getCurrentGameState();
	switch(state)
	{
	case GAME_STOP:
	case GAME_OVER:
		CEGUI::System &sys = CEGUI::System::getSingleton();
		sys.getSingleton().getDefaultGUIContext().injectMouseMove(arg.state.X.rel, arg.state.Y.rel);

		//Scroll wheel.
		if (arg.state.Z.rel)
			sys.getSingleton().getDefaultGUIContext().injectMouseWheelChange(arg.state.Z.rel / 120.0f);
		break;
	}

	return true;
}
//-------------------------------------------------------------------------------------
bool UIManager::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonDown(convertButton(id));
	return true;
}
//-------------------------------------------------------------------------------------
bool UIManager::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	CEGUI::System::getSingleton().getDefaultGUIContext().injectMouseButtonUp(convertButton(id));
	return true;
}

bool UIManager::AddButton(CEGUI::String btnName, CEGUI::String btnText, float xRelPos, float yRelPos, int width, int height,  bool (GameManager::*pFc)(const CEGUI::EventArgs& args), bool visible)
{
	
	CEGUI::Window *newWindow = m_windowMgr->createWindow("TaharezLook/Button", "BUTTONS/" + btnName);
	
	if (NULL == newWindow)
	{
		return false;
	}

	m_windowMap.insert(std::map<CEGUI::String,CEGUI::Window*>::value_type(btnName,newWindow));

	newWindow->setText(btnText);
	newWindow->setSize(CEGUI::USize(CEGUI::UDim(0, width), CEGUI::UDim(0,height)));
	newWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0, xRelPos), CEGUI::UDim(0,yRelPos)));
	newWindow->setVisible(visible);

	m_rootWindow->addChild(newWindow);
	newWindow->subscribeEvent(CEGUI::PushButton::EventClicked,CEGUI::Event::Subscriber(pFc,GameManager::GetSingletonPtr()));

	return true;
}

bool UIManager::AddProgressBar(CEGUI::String barName, float xRelPos, float yRelPos, int width, int height, bool visible)
{
	CEGUI::ProgressBar *newProgress =  static_cast<CEGUI::ProgressBar*>(m_windowMgr->createWindow("TaharezLook/ProgressBar", "PROGRESSBAR/" + barName));

	if (NULL == newProgress)
	{
		return false;
	}

	m_windowMap.insert(std::map<CEGUI::String,CEGUI::Window*>::value_type(barName,newProgress));

	newProgress->setProgress(1.0);
	newProgress->setSize(CEGUI::USize(CEGUI::UDim(0, width), CEGUI::UDim(0,height)));
	newProgress->setPosition(CEGUI::UVector2(CEGUI::UDim(0, xRelPos), CEGUI::UDim(0,yRelPos)));
	newProgress->setVisible(visible);

	m_rootWindow->addChild(newProgress);

	return true;
}


void UIManager::setProgress(CEGUI::String barName, float progress)
{
	std::map<CEGUI::String,CEGUI::Window*>::iterator it= m_windowMap.find(barName);
	if (it != m_windowMap.end())
	{
		static_cast<CEGUI::ProgressBar*>(it->second)->setProgress(progress);
	}
}

void UIManager::setVisible(CEGUI::String windowName, bool visible)
{
	std::map<CEGUI::String,CEGUI::Window*>::iterator it= m_windowMap.find(windowName);
	if (it != m_windowMap.end())
	{
		it->second->setVisible(visible);
	}
}

void UIManager::setText(CEGUI::String windowName, CEGUI::String text)
{
	std::map<CEGUI::String,CEGUI::Window*>::iterator it= m_windowMap.find(windowName);
	if (it != m_windowMap.end())
	{
		it->second->setText(text);
	}
}