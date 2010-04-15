#ifndef MA_EVENT_TYPE_HPP
#define MA_EVENT_TYPE_HPP

#include "EventType.hpp"
#include "KeyCodes.hpp"
#include "Logger.hpp"

namespace ma{

	//event types
	//! Enumeration for all event types there are.
	enum EEVENT_TYPE
	{
		//! An event of the graphical user interface.
		/** GUI events are created by the GUI environment or the GUI elements in response
		to mouse or keyboard events. When a GUI element receives an event it will either
		process it and return true, or pass the event to its parent. If an event is not absorbed
		before it reaches the root element then it will then be passed to the user receiver. */
		EET_GUI_EVENT = 0,

		//! A mouse input event.
		/** Mouse events are created by the device and passed to IrrlichtDevice::postEventFromUser
		in response to mouse input received from the operating system.
		Mouse events are first passed to the user receiver, then to the GUI environment (and possibly
		many GUI elements), then finally the input receiving scene manager (and possibly the active
		camera) */
		EET_MOUSE_INPUT_EVENT,

		//! A key input evant.
		/** Keyboard events are also created by the device and passed to IrrlichtDevice::postEventFromUser.
		They take the same path as mouse events. */
		EET_KEY_INPUT_EVENT,

		//! A log event
		/** Log events are only passed to the user receiver if there is one. If they are absorbed by the
		user receiver then no text will be sent to the console. */
		EET_LOG_TEXT_EVENT,

		//! A user event with user data.
		/** This is not used by Irrlicht and can be used
		to send user specific data though the system. */
		EET_USER_EVENT
	};

	//! Enumeration for all mouse input events
	enum EMOUSE_INPUT_EVENT
	{
		//! Left mouse button was pressed down.
		EMIE_LMOUSE_DOWN = 0,

		//! Right mouse button was pressed down.
		EMIE_RMOUSE_DOWN,

		//! Middle mouse button was pressed down.
		EMIE_MMOUSE_DOWN,

		//! Left mouse button was left up.
		EMIE_LMOUSE_UP,

		//! Right mouse button was left up.
		EMIE_RMOUSE_UP,

		//! Middle mouse button was left up.
		EMIE_MMOUSE_UP,

		//! The mouse cursor changed its position.
		EMIE_MOUSE_MOVED,

		//! The mouse wheel was moved. Use Wheel value in event data to find out
		//! in what direction and how fast.
		EMIE_MOUSE_WHEEL,

		//! No real event. Just for convenience to get number of events
		EMIE_COUNT
	};

	namespace gui
	{

		//class IGUIElement;

		//! Enumeration for all events which are sendable by the gui system
		enum EGUI_EVENT_TYPE
		{
			//! A gui element has lost its focus.
			/** GUIEvent.Caller is losing the focus to GUIEvent.Element.
			If the event is absorbed then the focus will not be changed. */
			EGET_ELEMENT_FOCUS_LOST = 0,

			//! A gui element has got the focus.
			/** If the event is absorbed then the focus will not be changed. */
			EGET_ELEMENT_FOCUSED,

			//! The mouse cursor hovered over a gui element.
			EGET_ELEMENT_HOVERED,

			//! The mouse cursor left the hovered element.
			EGET_ELEMENT_LEFT,

			//! An element would like to close.
			/** Windows and context menus use this event when they would like to close,
			this can be cancelled by absorbing the event. */
			EGET_ELEMENT_CLOSED,

			//! A button was clicked.
			EGET_BUTTON_CLICKED,

			//! A scrollbar has changed its position.
			EGET_SCROLL_BAR_CHANGED,

			//! A checkbox has changed its check state.
			EGET_CHECKBOX_CHANGED,

			//! A new item in a listbox was seleted.
			EGET_LISTBOX_CHANGED,

			//! An item in the listbox was selected, which was already selected.
			EGET_LISTBOX_SELECTED_AGAIN,

			//! A file has been selected in the file dialog
			EGET_FILE_SELECTED,

			//! A file open dialog has been closed without choosing a file
			EGET_FILE_CHOOSE_DIALOG_CANCELLED,

			//! 'Yes' was clicked on a messagebox
			EGET_MESSAGEBOX_YES,

			//! 'No' was clicked on a messagebox
			EGET_MESSAGEBOX_NO,

			//! 'OK' was clicked on a messagebox
			EGET_MESSAGEBOX_OK,

			//! 'Cancel' was clicked on a messagebox
			EGET_MESSAGEBOX_CANCEL,

			//! In an editbox was pressed 'ENTER'
			EGET_EDITBOX_ENTER,

			//! The tab was changed in an tab control
			EGET_TAB_CHANGED,

			//! A menu item was selected in a (context) menu
			EGET_MENU_ITEM_SELECTED,

			//! The selection in a combo box has been changed
			EGET_COMBO_BOX_CHANGED,

			//! The value of a spin box has changed
			EGET_SPINBOX_CHANGED
		};
	} // end namespace gui


	//! SEvents hold information about an event. See irr::IEventReceiver for details on event handling.
	struct MAEventDefaultConfig
	{
		//! Any kind of GUI event.
		struct GUIEvent
		{
			////! IGUIElement who called the event
			//gui::IGUIElement* Caller;

			////! If the event has something to do with another element, it will be held here.
			//gui::IGUIElement* Element;

			//! Type of GUI Event
			gui::EGUI_EVENT_TYPE EventType;

		};

		//! Any kind of mouse event.
		struct MouseInput
		{
			//! x position of mouse cursor
			int x;

			//! y position of mouse cursor
			int y;

			//! mouse wheel delta, usually 1.0 or -1.0.
			/** Only valid if event was EMIE_MOUSE_WHEEL */
			float Wheel;

			//! Type of mouse event
			EMOUSE_INPUT_EVENT Event;
		};

		//! Any kind of keyboard event.
		struct KeyInput
		{
			//! Character corresponding to the key (0, if not a character)
			wchar_t Char;

			//! Key which has been pressed or released
			EKEY_CODE Key;

			//! If not true, then the key was left up
			bool PressedDown;

			//! True if shift was also pressed
			bool Shift;

			//! True if ctrl was also pressed
			bool Control;
		};

		//! Any kind of log event.
		struct LogEvent
		{
			//! Pointer to text which has been logged
			const char* Text;

			//! Log level in which the text has been logged
			ELOG_LEVEL Level;
		};

		//! Any kind of user event.
		struct UserEvent
		{
			//! Some user specified data as int
			int UserData1;

			//! Another user specified data as int
			int UserData2;

			//! Some user specified data as float
			float UserData3;
		};

		typedef EEVENT_TYPE EVENT_TYPE;
		//union
		//{
		//	struct MAGUIEvent GUIEvent;
		//	struct MAMouseInput MouseInput;
		//	struct MAKeyInput KeyInput;
		//	struct MALogEvent LogEvent;
		//	struct MAUserEvent UserEvent;
		//};

	};

	typedef EventType<MAEventDefaultConfig> MAEventType;
}
#endif