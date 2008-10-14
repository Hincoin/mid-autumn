#ifndef MADEVICEIMPL_HPP
#define MADEVICEIMPL_HPP

#include "MADevice.hpp"

namespace ma
{
//ctor and dtor
template<typename Derived>
MADevice<Derived>::MADevice(EventProcessorPtr evt_processor):
VideoDriver_(0),GUIEnvironment_(0),SceneManager_(0),UserReceiver_(evt_processor),
OSOperator_(0),FileSystem_(new FileSystem),InputReceivingSceneManager_(0),
Timer_(new Timer)
{
	if (Printer::Logger)
	{
		Logger_ = Printer::Logger;
		Logger_->setReceiver(UserReceiver_);
	}
	else
	{
		Logger_ = new Logger(UserReceiver_);
		Printer::Logger = Logger_;
	}
}


template<typename Derived>
MADevice<Derived>::~MADevice()
{


	Configure::delete_ptr(VideoDriver_);
	Configure::delete_ptr(GUIEnvironment_);
	Configure::delete_ptr(SceneManager_);
	Configure::delete_ptr(Timer_);
	Configure::delete_ptr(VideoModeList_);
	Configure::delete_ptr(UserReceiver_);
	Configure::delete_ptr(Logger_);
	Configure::delete_ptr(OSOperator_);
	Configure::delete_ptr(FileSystem_);
	Configure::delete_ptr(InputReceivingSceneManager_);

	Configure::empty_ptr(VideoDriver_);
	Configure::empty_ptr(GUIEnvironment_);
	Configure::empty_ptr(SceneManager_);
	Configure::empty_ptr(Timer_);
	Configure::empty_ptr(VideoModeList_);
	Configure::empty_ptr(UserReceiver_);
	Configure::empty_ptr(Logger_);
	Configure::empty_ptr(OSOperator_);
	Configure::empty_ptr(FileSystem_);
	Configure::empty_ptr(InputReceivingSceneManager_);
}

//template<typename Derived>
//bool MADevice<Derived>::execute()
//{
//	return true;
//}
//template<typename Derived>
//void MADevice<Derived>::yield()
//{
//
//}
//
//template<typename Derived>
//void MADevice<Derived>::sleep(std::size_t timeMs, bool is_pauseTimer)
//{
////
//}

template<typename Derived>
void MADevice<Derived>::postEventFromUser(const EventType& event) 
{
	bool absorbed = false;

	if (UserReceiver_)
		absorbed = UserReceiver_->OnEvent(event);

	if (!absorbed && GUIEnvironment_)
		absorbed = GUIEnvironment_->postEventFromUser(event);

	SceneManagerPtr inputReceiver = InputReceivingSceneManager_;
	if (!inputReceiver)
		inputReceiver = SceneManager_;

	if (!absorbed && inputReceiver)
		absorbed = inputReceiver->postEventFromUser(event);
}

template<typename Derived>
void MADevice<Derived>::setEventReceiver(EventProcessorPtr receiver)
{
	UserReceiver_ = receiver;
	Logger->setReceiver(receiver);
	if (GUIEnvironment_)
		GUIEnvironment_->setUserEventReceiver(receiver);
}
template<typename Derived>
void MADevice<Derived>::setInputReceivingSceneManager(SceneManagerPtr sceneManager)
{
	InputReceivingSceneManager_ = sceneManager;
}

template<typename Derived>
void MADevice<Derived>::createGUIAndScene()
{

	// create gui environment
	GUIEnvironment_ = Configure::createGUIEnvironment(FileSystem_, VideoDriver_, OSOperator_);


	// create Scene manager
	SceneManager_ = Configure::createSceneManager(VideoDriver_, FileSystem_, /*CursorControl_,*/ GUIEnvironment_);

	setEventReceiver(UserReceiver_);
}


}

#endif

