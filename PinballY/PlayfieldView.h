// This file is part of PinballY
// Copyright 2018 Michael J Roberts | GPL v3 or later | NO WARRANTY
//
// Main playfield view Window
// This is a child window that serves as the D3D drawing surface for
// the main playfield window.


#pragma once

#include "../Utilities/Config.h"
#include "../Utilities/Joystick.h"
#include "../Utilities/KeyInput.h"
#include "../Utilities/InputManager.h"
#include "D3D.h"
#include "D3DWin.h"
#include "Camera.h"
#include "TextDraw.h"
#include "PerfMon.h"
#include "D3DView.h"
#include "BaseView.h"
#include "Sprite.h"
#include "VideoSprite.h"
#include "AudioVideoPlayer.h"
#include "HighScores.h"
#include "GameList.h"

class Sprite;
class TextureShader;
class GameListItem;
class GameCategory;
class MediaDropTarget;
class RealDMD;


// Playfield view
class PlayfieldView : public BaseView,
	JoystickManager::JoystickEventReceiver,
	InputManager::RawInputReceiver,
	ConfigManager::Subscriber,
	D3DView::IdleEventSubscriber
{
public:
	PlayfieldView();

	// Create our system window
	bool Create(HWND parent);

	// Initialize real DMD support
	void InitRealDMD(ErrorHandler &eh);

	// Update keyboard shortcut listings in a menu.  We call this when
	// creating a menu and again whenever the keyboard preferences are
	// updated.  The parent window can also call this to update Player
	// commands in its menus, if it includes Player comands in its menu bar.
	void UpdateMenuKeys(HMENU hMenu);

	// Update menu command item checkmarks and enabled statue for current 
	// UI state.  We call this before displaying the context menu or
	// parent frame's window menu to set the menu item states.  The parent
	// window can also call this to update Player commands in its menus,
	// if it includes Player commands in its own menu bar.
	virtual void UpdateMenu(HMENU hMenu, BaseWin *fromWin) override;

	// Reload the settings file.  The parent window can call this if
	// the configuration file is ever changed externally and it wants
	// to trigger a reload.
	void ReloadSettings();

	// Player menu update notification message.  The view window sends this
	// to the parent, with the menu handle in the WPARAM, to update a menu
	// that it's about to show with the current status of commands controlled
	// by the parent.
	static const UINT wm_parentUpdateMenu;

	// Global key event handlers.  All of our top-level windows (backglass,
	// DMD) route their keyboard input events here, since most key commands
	// are treated the same way in all of our windows.  A few commands apply
	// to individual windows, such as full-screen mode and monitor rotation,
	// so we provide a source window argument here for routing the command
	// (that we interpret from the key) back to the appropriate window.
	//
	// These routines are used in handling the low-level Windows key messages,
	// so we follow the usual convention that a 'true' return means that the
	// event was fully handled, and a 'false' return means that the system
	// define window proc should be invoked.
	bool HandleKeyEvent(BaseWin *win, UINT msg, WPARAM wParam, LPARAM lParam);
	bool HandleSysKeyEvent(BaseWin *win, UINT msg, WPARAM wParam, LPARAM lParam);
	bool HandleSysCharEvent(BaseWin *win, WPARAM wParam, LPARAM lParam);

	// Show the settings dialog
	void ShowSettingsDialog();

	// Show an error box.  If possible, we'll show it as a sprite
	// in the main view; if not, we'll show it as a system message box.
	void ShowError(ErrorIconType iconType, const TCHAR *groupMsg, const ErrorList *list = 0);
	void ShowSysError(const TCHAR *msg, const TCHAR *details);

	// enter/leave running game mode
	void BeginRunningGameMode();
	void EndRunningGameMode();

	// Application activation change notification.  The app calls this
	// when switching between foreground and background mode.
	void OnAppActivationChange(bool foreground);

	// frame window is being shown/hidden
	virtual void OnShowHideFrameWindow(bool show) override { }

	// Is Attract Mode active?
	bool IsAttractMode() const { return attractMode.active; }

	// reset attract mode
	void ResetAttractMode() { attractMode.Reset(this); }

	// change video enabling status
	virtual void OnEnableVideos(bool enable) override;

	// clear media
	void ClearMedia();

	// Determine if a file being dropped is a recognized type.  This
	// returns true if we're dropping a ZIP file or a media file of a
	// type accepted by the target window.
	bool CanDropFile(const TCHAR *fname, MediaDropTarget *dropTarget);

	// Begin a file drop operation.  Call this at the start of a 
	// group of DropFile() calls to reset the internal records of
	// the files being handled.
	void BeginFileDrop();

	// Process a file dropped onto the application.  This handles
	// dropping a ZIP file containing a HyperPin media pack, or any
	// media file accepted by the target window.  Returns true if
	// the file was a recognized type, false if not.
	bool DropFile(const TCHAR *fname, MediaDropTarget *dropTarget);

	// End a file drop operation
	void EndFileDrop();

	// Update the UI after new game list files are discovered in a 
	// file system scan
	void OnNewFilesAdded();

	// Handle a change to the game list manager
	void OnGameListRebuild();

protected:
	// destruction - called internally when the reference count reaches zero
	~PlayfieldView();

	// ConfigManager::Subscriber implementation
	virtual void OnConfigReload() override { OnConfigChange(); }

	// InputManager::RawInputReceiver implementation
	virtual bool OnRawInputEvent(UINT rawInputCode, RAWINPUT *raw, DWORD dwSize) override;

	// initialize the window
	virtual bool InitWin() override;

	// Update the sprite drawing list
	virtual void UpdateDrawingList() override;

	// Scale sprites that vary by window size
	virtual void ScaleSprites() override;

	// idle event handler
	virtual void OnIdleEvent() override;

	// window creation
	virtual bool OnCreate(CREATESTRUCT *cs) override;

	// window destruction
	virtual bool OnDestroy() override;

	// process a command
	virtual bool OnCommand(int cmd, int source, HWND hwndControl) override;

	// process a timer
	virtual bool OnTimer(WPARAM timer, LPARAM callback) override;
		
	// private window class messages (WM_USER to WM_APP-1)
	virtual bool OnUserMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;

	// private application message (WM_APP to 0xBFFF)
	virtual bool OnAppMessage(UINT msg, WPARAM wParam, LPARAM lParam) override;

	// context menu display
	virtual void ShowContextMenu(POINT pt) override;

	// JoystickManager::JoystickEventReceiver implementation
	virtual bool OnJoystickButtonChange(
		JoystickManager::PhysicalJoystick *js, 
		int button, bool pressed, bool foreground) override;
	virtual void OnJoystickAdded(
		JoystickManager::PhysicalJoystick *js, 
		bool logicalIsNew) override;

	// set internal variables according to the config settings
	void OnConfigChange();

	// initialize the status lines
	void InitStatusLines();

	// shut down the system - ask for confirmation through the menu system,
	// then actually do it
	void AskPowerOff();
	void PowerOff();

	// Timer IDs
	static const int animTimerID = 101;			  // animation timer
	static const int pfTimerID = 102;			  // playfield update timer
	static const int startupTimerID = 103;		  // startup initialization timer
	static const int infoBoxFadeTimerID = 104;    // info box fade timer
	static const int infoBoxSyncTimerID = 105;    // info box update timer
	static const int statusLineTimerID = 106;     // status line update
	static const int killGameTimerID = 107;		  // kill-game request pending
	static const int jsRepeatTimerID = 108;		  // joystick button auto-repeat timer
	static const int kbRepeatTimerID = 109;       // keyboard auto-repeat timer
	static const int attractModeTimerID = 110;    // attract mode timer
	static const int dofPulseTimerID = 111;		  // DOF signal pulse timer
	static const int attractModeStatusLineTimerID = 112;   // attract mode status line timer
	static const int creditsDispTimerID = 113;	  // number of credits display overlay timer
	static const int gameTimeoutTimerID = 114;    // game inactivity timeout timer
	static const int endSplashTimerID = 115;      // remove the "splash screen"
	static const int restoreDOFTimerID = 116;     // restore DOF access after a game terminates
	static const int cleanupTimerID = 117;        // periodic cleanup tasks

	// update the selection to match the game list
	void UpdateSelection();

	// Is the given game a real game?  Returns true if the game is
	// non-null and isn't the special "No Game" entry in the game list.
	static bool IsGameValid(const GameListItem *game);

	// Load the incoming playfield media.  This starts an asynchronous
	// thread that loads the new sprite.
	void LoadIncomingPlayfieldMedia(GameListItem *game);

	// Finish loading a new playfield.  This is called when the sprite
	// loader thread finishes.  This is called on the main UI thread,
	// so we don't have to do anything special for thread safety.
	void IncomingPlayfieldMediaDone(VideoSprite *sprite);

	// Load a wheel image
	Sprite *LoadWheelImage(const GameListItem *game);

	// Set a wheel image position.  'n' is the wheel image slot
	// relative to the current selection.  'rot' is the additional
	// rotation for animation.
	void SetWheelImagePos(Sprite *image, int n, float rot);

	// switch to the nth game from the current position
	void SwitchToGame(int n, bool fast, bool byUserCommand);

	// about box
	void ShowAboutBox();

	// commands on the current game
	void PlayGame(int cmd, int systemIndex = -1);
	void ShowFlyer(int pageNumber = 0);
	void ShowGameInfo();
	void ShowInstructionCard(int cardNumber = 0);
	void RateGame();
	void ShowHighScores();

	// Game inactivity timeout
	void OnGameTimeout();

	// Reset the game inactivity timer.  We call this to reset the
	// timer any time we observe a user input event (mouse, keyboard,
	// joystick) or when the running game state changes (such as
	// switching from "loading" to "running" state).
	void ResetGameTimeout();

	// Game inactivity timeout, in milliseconds.  Zero disables the
	// timeout.
	DWORD gameTimeout;

	// Last keyboard/joystick event time.  We keep track of the last
	// event times, even when we're in the background, via the raw
	// input mechanism.  We use this to detect activity in a running
	// game, for the purposes of the game timeout timer.
	DWORD lastInputEventTime;

	// Last PlayGame() command.  
	int lastPlayGameCmd;

	// remove the instructions card from other windows
	void RemoveInstructionsCard();

	// Initiate a high score request for the current game.  This sends
	// the request to PINemHi.exe asynchronously; the result will be
	// provided via a notification message when the program finishes.
	void RequestHighScores();

	// receive a high score data update from the HighScores object
	void ReceiveHighScores(const HighScores::NotifyInfo *ni);

	// Working rating for the current game.  This is the value shown
	// in the Rate Game dialog, which isn't committed to the database
	// until the player presses Select.
	float workingRating;

	// Update the RateGame dialog with the current working rating
	void UpdateRateGameDialog();

	// Stars image
	std::unique_ptr<Gdiplus::Image> stars;

	// Draw the rating stars image
	void DrawStars(Gdiplus::Graphics &g, float x, float y, float rating);

	// Get a text message with the number of stars
	TSTRING StarsAsText(float rating);

	// adjust the current game rating - used for the Next/Previous
	// keys when the Rate Game dialog is showing
	void AdjustRating(float delta);

	// show a filter submenu
	void ShowFilterSubMenu(int cmd);
	
	// show a recency filter menu for a particular filter subclass
	template<class FilterClass> void ShowRecencyFilterMenu(int idStrWithin, int idStrNotWithin)
	{
		ShowRecencyFilterMenu(
			[](const GameListFilter *f) { return dynamic_cast<const FilterClass*>(f) != nullptr; },
			idStrWithin, idStrNotWithin);
	}
	void ShowRecencyFilterMenu(
		std::function<bool(const GameListFilter*)> testFilter, 
		int idStrWithin, int idStrNotWithin);


	// update the animation
	void UpdateAnimation();

	// sync the playfield to the current selection in the game list
	enum SyncPlayfieldMode
	{
		SyncByTimer,
		SyncEndGame
	};
	void SyncPlayfield(SyncPlayfieldMode mode);

	// show the operator menu
	void ShowOperatorMenu();

	// show the game setup menu
	void ShowGameSetupMenu();

	// set categories for the current game
	void ShowGameCategoriesMenu(GameCategory *curSelection = nullptr, bool reshow = false);

	// show the game setup dialog
	void EditGameInfo();

	// toggle a given category in the edit list
	void ToggleCategoryInEditList(int cmd);

	// save category updates
	void SaveCategoryEdits();

	// edit category names
	void EditCategories();

	// Category editing list.  When a game category menu is active, this
	// contains a list of the categories currently assigned to the game.
	// We commit this back to the game's category list when we select
	// the Save command in the menu.
	std::unique_ptr<std::list<const GameCategory*>> categoryEditList;

	// show the media capture setup menu for the current game
	void CaptureMediaSetup();

	// begin media capture using the menu selections
	void CaptureMediaGo();

	// Media capture list.  This represents the items selected for
	// screen-shot capture in the menu UI.
	struct CaptureItem
	{
		CaptureItem(int cmd, const MediaType &mediaType, D3DView *win, bool exists, int mode) :
			cmd(cmd),
			mediaType(mediaType),
			win(win),
			exists(exists),
			mode(mode)
		{
		}

		// menu command ID
		int cmd;

		// Type descriptor for this item
		const MediaType &mediaType;

		// associated view window
		D3DView *win;

		// Capture mode for this item, as an IDS_CAPTURE_xxx string:
		//
		// IDS_CAPTURE_KEEP -> not capturing (keep existing item)
		// IDS_CAPTURE_SKIP -> not capturing (no existing item)
		// IDS_CAPTURE_CAPTURE -> capture
		// IDS_CAPTURE_SILENT -> capture without audio
		// IDS_CAPTURE_WITH_AUDIO -> capture with audio
		//
		int mode;

		// Is there an existing media item for this object?
		bool exists;
	};
	std::list<CaptureItem> captureList;

	// Startup delay time for the current item, in seconds
	int captureStartupDelay;

	// adjusted startup delay, in the adjustment dialog
	int adjustedCaptureStartupDelay;

	// Display/update the capture setup menu.  The menu includes
	// checkbox items for all of the available media types, and
	// these operate in "dialog" mode, meaning that selecting them
	// inverts the checkmark status in situ without closing the
	// menu.  Hence we have to update the menu in place any time
	// a checkmark item is selected, which is why this method is
	// for "display/update" rather than just "display".  An update
	// operation is really the same as the initial display, except
	// that the update skips the opening animation and just draws
	// a new version of the menu.
	//
	// The current status of the checkmark items for the media
	// type is stored in 'captureSpec' above, so that has to be
	// initialized before this is called.  'captureExist' also
	// has to be initialized, since we use it to indicate if each
	// items has an existing file.  This routine merely draws the
	// menu with the current settings in those structs.
	void DisplayCaptureMenu(bool updating, int selectedCmd);

	// Advance a capture item to the next state
	void AdvanceCaptureItemState(int cmd);

	// Show/update the capture startup delay dialog
	void ShowCaptureDelayDialog(bool update);

	// Media drop list.  
	struct MediaDropItem
	{
		MediaDropItem(const TCHAR *filename, int zipIndex, 
			const TCHAR *impliedGameName, const TCHAR *destFile,
			const MediaType *mediaType, bool exists) :
			filename(filename), 
			zipIndex(zipIndex), 
			impliedGameName(impliedGameName),
			destFile(destFile),
			mediaType(mediaType), 
			exists(exists),
			status(exists ? 
				(zipIndex == -1 ? IDS_MEDIA_DROP_REPLACE : IDS_MEDIA_DROP_KEEP) : 
				IDS_MEDIA_DROP_ADD),
			cmd(0)
		{
		}

		// Filename (with path).  For an item in a ZIP file, this is
		// the ZIP file path.
		TSTRING filename;

		// Index of the item in a ZIP file, or -1 for a media file
		// dropped directly.
		int zipIndex;
		
		// Implied game name.  This is the game name implied by the
		// filename, as interpreted through the naming conventions for
		// the source:
		//
		// - For dropped ZIP files that appear to be in HyperPin Media
		//   Pack format, each file in the ZIP should be named in the
		//   format "Prefix/Media Type Dir/Title (Manuf Year).ext".
		//   So the last path element (minus extension) should be the
		//   name of the game, in the "Title (Manuf Year)" format.
		//
		//   One exception: for indexed media types, there might be
		//   a numeric suffix in the name, separated from the rest by
		//   a space: "Title (Manuf Year) Num.ext".
		//
		//   The prefix part seems to be pretty random, judging by the
		//   Media Pack files on vpforums.org.  I'd guess that HyperPin
		//   and PinballX each have their own ad hoc list of patterns
		//   they accept, but for simplicitly we just ignore the prefix,
		//   as it doesn't contain any meaningful information anyway 
		//   (apart, perhaps, from some weak heuristic validation that
		//   this really is a Media Pack file).  Note that the prefix
		//   should generally be fixed within a file, EXCEPT that there
		//   will be one prefix for "per system" media types (such as
		//   playfield and backglass images) and another for "generic"
		//   media types (instruction cards and flyers).  But we don't
		//   enforce this.
		//
		// - For directly dropped media files (.JPG files and the like),
		//   there's no useful convention to apply from HyperPin or 
		//   PinballX.  They'd expect the name of every media file for 
		//   a given to have the same name (modulo extension), "Title 
		//   (Manuf Year).ext".  (Numeric suffixes might also be used
		//   for indexed types.)  The actual media types are determined
		//   by the directory location rather than the name.  This is
		//   inconvenient for users managing media files separately,
		//   though; they might prefer to use one of the following
		//   formats (showing concrete examples for clarity):
		//
		//       .../Xenon (Bally 1980)/Backglass Image.jpg
		//       .../Xenon (Bally 1990) Backglass Image.jpg
		//       .../Xenon (Bally 1990) - Backglass Image.jpg
		//       .../Backglass Image - Xenon (Bally 1990).jpg
		//
		//   So we'll try parsing the path to see if one of those
		//   formats matches.
		//
		// If we can't find a suitable implied name, we'll leave this
		// blank.
		//
		TSTRING impliedGameName;

		// Destination file, with path
		TSTRING destFile;

		// Media type for the item
		const MediaType *mediaType;

		// is there an existing media item of this type?
		bool exists;

		// menu command
		int cmd;

		// current menu status for this item:
		//
		//   IDS_MEDIA_DROP_ADD      - add a new item
		//   IDS_MEDIA_DROP_REPLACE  - replace existing item
		//   IDS_MEDIA_DROP_SKIP     - skip new item
		//   IDS_MEDIA_DROP_KEEP     - keep existing item
		int status;
	};
	std::list<MediaDropItem> dropList;

	// Media drop phase 2: show the selection menu.  This is broken out
	// from the initial drop processing, because that can be interrupted
	// by a confirmation prompt if the game name looks wrong in the 
	// dropped files.
	void MediaDropPhase2();

	// Display the "add dropped media" menu
	void DisplayDropMediaMenu(bool updating, int selectedCmd);

	// invert the state of a media item in the drop menu
	void InvertMediaDropState(int cmd);

	// Add the media from the drop list.  This is invoked when the
	// user clicks the "go" option from the drop confirmation menu.
	void MediaDropGo();

	// Check to see if we can add media to the game.  This checks to
	// see if the game's manufacturer, system, and year are configured.
	// If so, it simply returns true to indicate that media can be
	// added.  If not, it displays a menu asking the user to configure
	// the game now, and returns false to indicate that we can't add
	// media yet.
	bool CanAddMedia(GameListItem *game);

	// show the "find media online" menu
	void ShowMediaSearchMenu();

	// launch a Web browser to search for game media
	void LaunchMediaSearch();

	// hide/show the current game
	void ToggleHideGame();

	// Enable/disable the status line
	void EnableStatusLine();
	void DisableStatusLine();

	// Status line background.  This is displayed at the bottom of the
	// screen as the background for status text messages.
	RefPtr<Sprite> statusLineBkg;

	// Status line messages.  The message text comes from the config
	// file, so the content and number of messages can vary.  The
	// source text is fixed at load time, but it can contain macros
	// that change according to the current game selection and game 
	// filter, so we keep the source and display text separately. 
	// Each time we're about to rotate in a new message, we check
	// the display text against the current version of the display
	// text, and if it differs, we replace the sprite.  This lets
	// us reuse sprites for as long as they're valid, while still
	// updating them as needed.
	struct StatusItem
	{
		StatusItem(const TCHAR *srcText) : srcText(srcText) { }

		// Update the item's sprite if necessary
		void Update(PlayfieldView *pfv, float y);

		// determine if an update is needed
		bool NeedsUpdate(PlayfieldView *pfv);

		// expand macros in my text
		TSTRING ExpandText(PlayfieldView *pfv);

		TSTRING srcText;         // source text, which might contain {xxx} macros
		TSTRING dispText;        // display text, with macros expanded
		RefPtr<Sprite> sprite;   // sprite
	};

	// Status line 
	struct StatusLine
	{
		StatusLine() : curItem(items.end()), dispTime(2000), y(0), height(75), phase(DispPhase) { }

		// initialize
		void Init(PlayfieldView *pfv, int yOfs, int fadeSlide, int idleSlide,
			const TCHAR *cfgVar, int defaultMessageResId);

		// handle a timer update
		void TimerUpdate(PlayfieldView *pfv);

		// Do an explicit update.  We call this whenever one of the data
		// sources for a status line display changes.  This can check the
		// expanded text to see if a new sprite needs to be generated.
		void OnSourceDataUpdate(PlayfieldView *pfv);

		// add my sprites to the window's D3D drawing list
		void AddSprites(std::list<Sprite*> &sprites);

		// reset 
		void Reset(PlayfieldView *pfv);

		// messages for this status line
		std::list<StatusItem> items;

		// current active item, as an index in the items vector
		std::list<StatusItem>::iterator curItem;

		// get the next item
		std::list<StatusItem>::iterator NextItem()
		{
			auto i = curItem;
			if (i == items.end())
				return items.begin();
			++i;
			return (i == items.end() ? items.begin() : i);
		}

		// hide the current item
		void Hide()
		{
			if (curItem != items.end())
				curItem->sprite->alpha = 0.0f;
		}

		// Start time for current item display
		DWORD startTime;

		// Display time for this status line.  This is the time in
		// ticks between message changes.
		DWORD dispTime;

		// current display phase
		enum
		{
			FadeInPhase,    // fading in the new message
			DispPhase,      // displaying the current message
			FadeOutPhase    // fading out the outgoing message
		} phase;

		// height
		int height;

		// offset in normalized units from bottom of screen
		float y;

		// Horizontal slide distance while idle.  We'll slide by this 
		// distance (in normalized units) over the course of the
		// display time.
		float idleSlide;

		// Horizontal slide distance while fading
		float fadeSlide;
	};

	// Upper and lower status lines
	StatusLine upperStatus;
	StatusLine lowerStatus;

	// Attract mode status line
	StatusLine attractModeStatus;

	// Update status line text.  This calls OnSourceDataUpdate()
	// for each status line, to make sure that the expanded status
	// line text is current.
	void UpdateAllStatusText();

	// Current and incoming playfield media.  The current playfield
	// is the main background when idle.  During animations, the current
	// playfield is the background layer, and the incoming playfield is
	// drawn in front of it with gradually increasing opacity.
	template<class SpriteType> struct GameMedia
	{
		GameMedia() : game(0) { }

		void Clear()
		{
			game = 0;
			sprite = 0;
		}
		
		GameListItem *game;
		RefPtr<SpriteType> sprite;
	};
	GameMedia<VideoSprite> currentPlayfield, incomingPlayfield;

	// asynchronous loader for the playfield sprite
	AsyncSpriteLoader playfieldLoader;

	// List of wheel sprites.  When idle, the wheel shows the current
	// game's wheel image in the middle, and two wheel images on each
	// side for the previous/next games in the list.  During game
	// switch animations, we add the next game on the incoming side.
	std::list<RefPtr<Sprite>> wheelImages;

	// Game info box.  This is a popup that appears when we're idling
	// with a game selected, showing the title and other metadata for
	// the active selection.  This box is automatically removed when
	// any other animation occurs, and comes back when we return to
	// idle state.
	GameMedia<Sprite> infoBox;

	// show a message on the running game overlay
	void ShowRunningGameMessage(const TCHAR *msg, int ptSize);

	// Running game overlay.  This is a separate layer that we bring
	// up in front of everything else when we launch a game.
	RefPtr<Sprite> runningGamePopup;

	// boxes, dialogs, etc.
	RefPtr<Sprite> popupSprite;

	// Popup type.  This indicates which type of item is currently
	// displayed in the modal popup box.
	enum PopupType
	{
		PopupNone,			// no popup
		PopupFlyer,			// game flyer
		PopupGameInfo,		// game info
		PopupInstructions,  // game instruction card
		PopupAboutBox,		// about box
		PopupErrorMessage,	// error message alert
		PopupRateGame,		// enter game rating "dialog"
		PopupHighScores,    // high scores list
		PopupCaptureDelay   // capture delay dialog
	} 
	popupType;

	// start a popup animation
	void StartPopupAnimation(PopupType popupType, bool opening);

	// close the current popup
	void ClosePopup();

	// separate popup for the credit count overlay, used when a
	// coin is inserted
	RefPtr<Sprite> creditsSprite;
	DWORD creditsStartTime;

	// update/remove the credits display
	void OnCreditsDispTimer();

	// show the next queued error
	void ShowQueuedError();

	// popup animation mode and start time
	enum
	{
		PopupAnimNone,		// no animation
		PopupAnimOpen,		// opening a popup
		PopupAnimClose		// closing a popup
	} popupAnimMode;
	DWORD popupAnimStartTime;

	// Current flyer page, from 0.  Each game can have up to 8 pages
	// of flyers, stored as separate images in the media folder.  We
	// show one at a time; this keeps track of which one is up.
	int flyerPage;

	// Current instruction card page.  As with the flyer, a game can
	// have multiple instruction cards, but we only show one at a 
	// time, so we keep track of the current one here.
	int instCardPage;

	// Instruction card location, from the configuration
	TSTRING instCardLoc;

	// Active audio clips.  Audio clips that we play back via
	// AudioVideoPlayer have to be kept as active object references
	// while playing, and released when playback finishes.  The
	// player notifies us at end of playback by sending the private
	// message AVPMsgEndOfPresentation to our window.  This map
	// keeps track of the active clips that need to be released
	// at end of playback.  Note that clips are keyed by cookie,
	// not object pointer, since the object pointer isn't safe to
	// use for asynchronous purposes like this due to the possibility
	// that C++ could reuse a memory address for a new object after
	// an existing object is deleted.
	std::unordered_map<DWORD, RefPtr<AudioVideoPlayer>> activeAudio;

	// Queued errors.  If an error occurs while we're showing an
	// error popup, we'll queue it for display after the current
	// error is dismissed.
	struct QueuedError
	{
		QueuedError(ErrorIconType iconType, const TCHAR *groupMsg, const ErrorList *list)
			: iconType(iconType)
		{
			// set the group message, if present
			if (groupMsg != nullptr)
				this->groupMsg = groupMsg;

			// store the list, if present
			if (list != nullptr)
				this->list.Add(list);
		}

		ErrorIconType iconType;
		TSTRING groupMsg;
		SimpleErrorList list;
	};
	std::list<QueuedError> queuedErrors;

	// Adjust a sprite to our standard popup position.  This centers
	// the sprite in the top half of the screen, but leaves a minimum
	// margin above.
	void AdjustSpritePosition(Sprite *sprite);

	// Menu item flags.  MenuStayOpen is mostly intended for use with
	// checkmarks or radio buttons, to allow making a series of item
	// selections before dismissing the menu.
	static const UINT MenuChecked = 0x0001;			// checkmark
	static const UINT MenuRadio = 0x0002;			// radio button checkmark
	static const UINT MenuSelected = 0x0004;		// initial menu selection
	static const UINT MenuHasSubmenu = 0x0008;      // menu opens a submenu
	static const UINT MenuStayOpen = 0x0010;        // menu stays open on selection
	
	// Menu item descriptor.  This is used to create a menu.
	struct MenuItemDesc
	{
		MenuItemDesc(const TCHAR *text, int cmd, UINT flags = 0) :
			text(text), 
			cmd(cmd), 
			selected((flags & MenuSelected) != 0), 
			checked((flags & MenuChecked) != 0),
			radioChecked((flags & MenuRadio) != 0),
			hasSubmenu((flags & MenuHasSubmenu) != 0),
			stayOpen((flags & MenuStayOpen) != 0)
		{ }

		// text label
		TSTRING text;

		// Command ID.  This command is executed via the normal
		// WM_COMMAND mechanism, as though selected from a regular
		// Windows menu.
		int cmd;

		// Is the menu initially selected?
		bool selected;

		// Is the menu item checkmarked?
		bool checked;

		// Is the menu item radio-button checkmarked?
		bool radioChecked;

		// Does this menu open a submenu?
		bool hasSubmenu;

		// Does this menu stay open on selection?
		bool stayOpen;
	};

	// Menu item.  This describes a menu item currently being 
	// displayed.
	struct MenuItem
	{
		MenuItem(int x, int y, int cmd, bool stayOpen) :
			x(x), y(y), cmd(cmd), stayOpen(stayOpen)
		{ }

		// pixel offset from top left of menu
		int x, y;

		// command ID
		int cmd;

		// stay open when selected
		bool stayOpen;
	};

	// Menu.  This describes the menu currently being displayed.
	struct Menu : public RefCounted
	{
		Menu(DWORD flags);
		~Menu();

		// menu flags (SHOWMENU_xxx bit flags)
		DWORD flags;

		// menu items
		std::list<MenuItem> items;

		// original descriptor list for the menu
		std::list<MenuItemDesc> descs;

		// selected item
		std::list<MenuItem>::iterator selected;

		// select an item
		void Select(std::list<MenuItem>::iterator sel);

		// Is the menu paged?  This is true if the menu has page up/
		// page down command items to scroll through a list that's
		// too long to display all at once.
		bool paged;

		// Menu sprites: background, current highlight, menu items
		RefPtr<Sprite> sprBkg;
		RefPtr<Sprite> sprItems;
		RefPtr<Sprite> sprHilite;
	};

	// Current active menu
	RefPtr<Menu> curMenu;

	// Pending new menu.  If an old menu is showing when a new menu is
	// created, we store the new menu here while the exit animation for
	// the old menu finishes.
	RefPtr<Menu> newMenu;

	// Current menu's item descriptor list
	std::list<MenuItemDesc> curMenuDesc;

	// Current menu "page".  Some menus (e.g., the Categories menu)
	// can grow to arbitrary lengths and so might need to be broken
	// into pages.  This tells us which page we're currently showing
	// in such a menu.
	int menuPage;

	// Show a menu.  This creates the graphics resources for the menu
	// based on the list of item descriptors, and displays the menu.
	// If a menu is already showing, it's replaced by the new menu.
	void ShowMenu(const std::list<MenuItemDesc> &items, DWORD flags, int pageno = 0);

	// Show the next/previous page in the current menu
	void MenuPageUpDown(int dir);

	// menu is an exit menu -> the Escape key can select items from it
	const DWORD SHOWMENU_IS_EXIT_MENU = 0x00000001;

	// skip menu animation
	const DWORD SHOWMENU_NO_ANIMATION = 0x00000002;

	// "Dialog box" style menu.  This is almost like a regular menu,
	// but is for situations where we need a prompt at the top to
	// explain what the menu is about.  The first item is a static
	// text message that we display as the prompt message.  We then
	// display the menu items below this as usual.  We use a wider
	// box than usual to accommodate the longer text of the prompt
	// message, and we allow the prompt message to wrap if necessary
	// to fit it.
	const DWORD SHOWMENU_DIALOG_STYLE = 0x00000004;

	// Handle any cleanup tasks when the current menu is closing.
	// 'incomingMenu' is the new menu replacing the old menu, or
	// null if there's no new menu (i.e., we're just returning to
	// the wheel UI).  This doesn't actually do any of the work of
	// closing the menu in the UI; it just takes care of side 
	// effects of removing the menu.
	void OnCloseMenu(const std::list<MenuItemDesc> *incomingMenu);

	// Exit Key mode in the Exit menu.  If this is true, the Exit
	// key acts like the Select key within an Exit menu.  This is
	// configurable, since some people are bound to find this
	// treatment backwards (since the Exit key means Cancel in
	// every other menu).
	bool exitMenuExitKeyIsSelectKey;

	// move to the next/previous menu item
	void MenuNext(int dir);

	// menu animation mode
	enum MenuAnimMode
	{
		MenuAnimNone,			// idle
		MenuAnimOpen,			// opening a menu
		MenuAnimClose			// closing a menu
	} menuAnimMode;

	// menu animation start time
	DWORD menuAnimStartTime;

	// start a menu animation
	void StartMenuAnimation(bool fast);

	// Update menu animation for the given menu.  'progress' is a
	// value from 0.0f to 1.0f giving the point in time as a fraction
	// of the overall animation time for the operation.
	void UpdateMenuAnimation(Menu *menu, bool opening, float progress);

	// update popup animation
	void UpdatePopupAnimation(bool opening, float progress);

	// close any open menus and popups
	void CloseMenusAndPopups();

	// wheel animation mode
	enum WheelAnimMode
	{
		WheelAnimNone,				// idle
		WheelAnimNormal,			// normal speed wheel motion
		WheelAnimFast				// fast wheel switch, when auto-repeating keys
	} wheelAnimMode;

	// Wheel animation start time
	DWORD wheelAnimStartTime;

	// start an animation sequence
	void StartWheelAnimation(bool fast);

	// Start the animation timer if necessary.  This checks to see if
	// the timer is running, and starts it if not.  In any case, we
	// fill in startTime with the current time, as the reference time
	// for the current animation.
	void StartAnimTimer();
	void StartAnimTimer(DWORD &startTime);

	// is the animation timer running?
	bool isAnimTimerRunning;

	// end the current animation sequence
	void EndAnimation();

	// Update the game info box.  This just sets a timer to do a
	// deferred update in a few moments, so that the info box reappears
	// when we're idle.
	void UpdateInfoBox();

	// Sync the info box.  If we're idle, this displays the info box.
	void SyncInfoBox();

	// Hide the info box.  We do this whenever the UI switches out of
	// idle mode - showing a menu, showing a popup, switching games, etc.
	void HideInfoBox();

	// update the info box animation
	void UpdateInfoBoxAnimation();

	// Start a playfield crossfade.  This is a separate mode from the
	// other animations, as it can run in parallel.
	void StartPlayfieldCrossfade();

	// Incoming playfield load time.  This is
	DWORD incomingPlayfieldLoadTime;

	// Game info box fade start time
	DWORD infoBoxStartTime;

	// running game popup start time and animation mode
	DWORD runningGamePopupStartTime;
	enum
	{
		RunningGamePopupNone,
		RunningGamePopupOpen,
		RunningGamePopupClose
	} runningGamePopupMode;

	// Number of wheel positions we're moving in the game switch animation
	int animWheelDistance;

	// Game number of first game in wheel, relative to current game
	int animFirstInWheel;
	int animAddedToWheel;

	// Attract mode.  When there's no user input for a certain
	// length of time, we enter attract mode.  In attract mode,
	// we automatically change games every few seconds.  This
	// adds to the arcade ambiance for a pin cab.  It also
	// serves as a screen saver, since it changes the on-screen
	// graphics every few seconds.
	struct AttractMode
	{
		AttractMode()
		{
			active = false;
			enabled = true;
			t0 = GetTickCount();
			idleTime = 60000;
			switchTime = 5000;
			dofEventA = 1;
			dofEventB = 1;
			savePending = true;
		}

		// Are we in attract mode?
		bool active;

		// Is attract mode enabled?
		bool enabled;

		// Is a save pending?  We automatically save any uncommitted
		// changes to files (config, stats) after a certain amount of
		// idle time, or when entering attract mode.  The idea is to
		// defer file saves until the user won't notice the (slight)
		// time it takes; if the user hasn't interacted in a while,
		// there's a good change it'll be a while longer before they
		// do interact again, so a brief pause won't be noticeable.
		bool savePending;

		// Start time (system ticks) of last relevant event.  When
		// running normally, this is the time of the last user input
		// event.  During attract mode, it's the time of the last
		// game change.
		DWORD t0;

		// Idle time before entering attract mode, in millseconds
		DWORD idleTime;

		// Game switch interval when attract mode is running, in milliseconds
		DWORD switchTime;

		// Next DOF attract mode event IDs.  When attract mode is active,
		// we fire a series of named DOF events that the DOF config can
		// use to trigger lighting effects.  For flexibility in defining
		// animated effects in the DOF lighting, we provide several event
		// series of different lengths:
		//
		// PBYAttractA<N> is a series of 5 events at 1-second intervals,
		// with N running from 1 to 5: PBYAttract1, PBYAttract2, etc.
		// These can be used to trigger animated lighting effects on a
		// 5-second loop.
		//
		// PBYttractB<N> works the same way, firing at 1-second intervals,
		// but with a 60-second loop (so N runs from 1 to 60).  This can be 
		// used to trigger occasional effects on a longer cycle.
		//
		// PBYttractR<N> is a series of 5 events (N from 1 to 5) that fire
		// at random intervals.  Once per second, we randomly decide whether
		// to fire an event at all, and then randomly choose which of the 5
		// "R" events to fire.
		//
		// All of these sequences run concurrently.  The intention is that
		// the DOF config can use the "A" sequence for a basic background
		// sequence of blinking lights, and can fire occasional extra effects
		// on longer loops with the "B" sequence and/or the random "R" 
		// events.
		//
		// Note that attract mode also fires the following events related
		// to status changes:
		// 
		// PBYScreenSaverStart  - fires when attract mode starts
		// PBYScreenSaverQuit   - fires when attract mode ends
		// PBYScreenSaver       - ON (1) the whole time attract mode is active
		// PBYAttractWheelRight - fires when attract mode randomly switches games
		//
		int dofEventA;
		int dofEventB;

		// Handle the attract mode timer event
		void OnTimer(PlayfieldView *pfv);

		// Reset the idle timer and turn off attract mode
		void Reset(PlayfieldView *pfv);

		// Handle a keystroke event.  This updates the last key event
		// time, and exits attract mode if it's active.
		void OnKeyEvent(PlayfieldView *pfv);

	} attractMode;

	// Receive notification of attract mode entry/exit.  The AttractMode
	// subobject calls these when the mode changes.
	void OnBeginAttractMode();
	void OnEndAttractMode();

	// Play a button or event sound effect
	void PlayButtonSound(const TCHAR *effectName);

	// Are button/event sound effects muted?
	bool muteButtons;

	// DOF pulsed effect queue.  Some of the signals we send to DOF are
	// states, where we turn a named DOF item ON for as long as we're in
	// a particular UI state (e.g., showing a menu).  Other signals are
	// of the nature of events, where we want to send DOF a message that
	// something has happened, without leaving the effect ON beyond the
	// momentary trigger signal.  DOF itself doesn't have a concept of
	// events; it was designed around VPinMAME, which thinks in terms 
	// of the physical switches on pinball machines.  So DOF's notion
	// of an "event" is when a switch (or, in DOF terms, a named event)
	// is switched ON briefly and then switched back off.  But it's not
	// good enough to literally turn a DOF effect ON and immediately OFF
	// again, because, again, DOF doesn't think in terms of events, but
	// in terms of states, and it uses a polling model to check states.
	// So to create a pulse that DOF notices, we have to turn the effect
	// on and leave it on long enough for DOF's polling loop to see it.
	// 
	// This queue is for those sorts of events, where we have to pulse a
	// DOF effect briefly, but not *too* briefly.  We could more easily
	// turn an effect on, Sleep() for a few milliseconds, and then turn
	// it off, but that's sucky because it blocks the UI/rendering 
	// thread.  Instead, we use this queue.  Each time we need to pulse
	// a DOF effect, we push an ON/OFF event pair onto the tail of this
	// queue.  A WM_TIMER timer then pulls events off the queue and sends
	// them to DOF.  This lets us leave the effects on for long enough to
	// satisfy DOF without blocking the UI thread.
	struct QueuedDOFEffect
	{
		QueuedDOFEffect(const TCHAR *name, UINT8 val) : name(name), val(val) { }
		TSTRING name;	// effect name
		UINT8 val;		// value to set the effect to
	};
	std::list<QueuedDOFEffect> dofQueue;

	// Last DOF pulsed event time, as a GetTickCount64 value
	ULONGLONG lastDOFEventTime;

	// Queue a DOF ON/OFF pulse.   This queues an ON/OFF event pair
	// for the effect.
	void QueueDOFPulse(const TCHAR *name);

	// Queue a DOF event.  Pushes the event onto the end of the DOF
	// queue, and starts the timer if the queue was empty.
	void QueueDOFEvent(const TCHAR *name, UINT8 val);

	// Fire a DOF event immediately.
	void FireDOFEvent(const TCHAR *name, UINT8 val);

	// Handle the DOF queued event timer.  Pulls an event off the
	// front of the queue and sends it to DOF.  Stops the timer if 
	// the queue is empty after pulling this effect.
	void OnDOFTimer();

	// DOF interaction
	class DOFIfc
	{
	public:
		DOFIfc();
		~DOFIfc();

		// Set the UI context:  wheel, menu, popup, etc.  This allows DOF
		// to set different effects depending on the main UI state.  The
		// state names are arbitrarily defined in the config tool database.
		void SetUIContext(const WCHAR *context) { SetContextItem(context, this->context); }

		// Sync the selected game
		void SyncSelectedGame();

		// Set the ROM.  The config tool database uses the ROM name to 
		// determine which table is currently selected in the front-end UI, 
		// so that it can set effects specific to each table (e.g., flipper
		// button RGB colors).
		void SetRomContext(const TCHAR *rom) 
			{ SetContextItem(rom != nullptr ? TCHARToWide(rom).c_str() : _T(""), this->rom); }

		// get the current ROM
		const WCHAR *GetRom() const { return rom.c_str(); }

		// Set a keyboard key effect.  This sets a DOF effect associated
		// with a key/button according to the key state.  The key event
		// handlers calls this on key down and key up events.
		void SetKeyEffectState(const TCHAR *effect, bool keyDown);

		// Turn off all keyboard key effects
		void KeyEffectsOff();

	protected:
		// Update a context item.  A "context item" is a variable that 
		// keeps track of an element of the current program context that's
		// associated with a DOF effect.  At any given time, a context
		// item has a single string value, which is the name of a DOF
		// effect that will be turned on as long as this context is in
		// effect. 
		//
		// The context item itself doesn't have a named DOF state; the
		// DOF states are the possible *values* of the context.
		//
		// For example, the "UI context" keeps track of where we are in
		// the UI presentation: "PBYWheel" when we're in the base mode
		// showing the wheel, "PBYMenu" when a menu is displayed,
		// "PBYScreenSaver" when in attract mode.  Whenever one of these
		// UI contexts is in effect, that named DOF effect is activated
		// (value = 1).  When we change to a new UI context, the named 
		// DOF effect for the previous state is deactivated (value = 0),
		// and the DOF effect for the new state is activated.
		void SetContextItem(const WCHAR *newVal, WSTRING &itemVar);

		// Current UI context
		WSTRING context;

		// Current ROM
		WSTRING rom;

		// Keyboard key effect states.  Each entry here is true
		// the key is down, false if it's up.  Multiple key effects
		// states can be activated at once, since multiple keys can
		// be held down at once.
		std::unordered_map<TSTRING, bool> keyEffectState;

	} dof;

	// PINemHi version number string, if available.  We try to
	// retrieve at startup by running "PINemHi -v" and capturing
	// the version string it returns.
	TSTRING pinEmHiVersion;

	// Is an ALT key mapped to a command?  If so, we'll suppress
	// the Alt key's normal function of activating a menu's
	// keyboard shortcut when used in combination with another
	// key (e.g., Alt+F to open the &File menu), as well as its 
	// function when used alone to enter keyboard menu navigation
	// mode.
	bool leftAltHasCommand;
	bool rightAltHasCommand;

	// Is the F10 key used for a command?  If so, we'll suppress
	// its normal handling (activates keyboard menu navigation).
	bool f10HasCommand;

	// Is the ALT key used as a modifier for any mouse commands?
	// If so, we'll suppress the normal behavior of the Alt key of
	// entering keyboard menu navigation mode.
	bool altHasMouseCommand;

	// Key press event modes.  This is basically a bit mask, where
	// 0x01 is set if the key is down, and 0x02 is set if it's a 
	// repeated key.  The idiom 'if (mode)...' can be used to treat 
	// the all key-down events (normal, auto-repeat, and background)
	// as equivalent.
	//
	// KeyBgDown is set if the key is pressed while we're running
	// in the background.  In this case the KeyDown and KeyRepeat
	// bits are NOT set, so that command handlers can distinguish
	// foreground and background events.  Most commands only apply
	// when we're in the foreground.
	enum KeyPressType
	{
		KeyUp = 0x00,					// Key Up event
		KeyDown = 0x01,					// first Key Down event
		KeyRepeat = 0x02 | 0x01,		// auto-repeat event
		KeyBgDown = 0x10,				// app-in-background Key Down event
		KeyBgRepeat = 0x20 | 0x10		// app-in-background auto-repeat event
	};

	// Keyboard command dispatch table
	struct QueuedKey;
	typedef void (PlayfieldView::*KeyCommandFunc)(const QueuedKey &key);
	std::unordered_map<int, std::list<KeyCommandFunc>> vkeyToCommand;

	// add a command to the vkeyToCommand list
	void AddVkeyCommand(int vkey, KeyCommandFunc func);

	// key event queue
	struct QueuedKey
	{
		QueuedKey() : hWndSrc(NULL), mode(KeyUp), func(nullptr) { }

		QueuedKey(HWND hWndSrc, KeyPressType mode, KeyCommandFunc func)
			: hWndSrc(hWndSrc), mode(mode), func(func) { }

		HWND hWndSrc;			// source window
		KeyPressType mode;		// key press mode
		KeyCommandFunc func;	// command handler
	};
	std::list<QueuedKey> keyQueue;

	// Add a key press to the queue and process it
	void ProcessKeyPress(HWND hwndSrc, KeyPressType mode, std::list<KeyCommandFunc> funcs);

	// Process the key queue.  On a keyboard event, we add the key
	// to the queue and call this routine; we also call it whenever
	// an animation ends.  If an animation is in progress, we defer
	// key processing until the animation ends.
	void ProcessKeyQueue();

	// Keyboard auto-repeat.  Rather than using the native Windows
	// auto-repeat feature, we implement our own using a timer.  
	//
	// Handling auto-repeat ourselves lets us make the behavior more 
	// uniform across keyboards.  Windows's native handling is 
	// inconsistent, since the auto-repeat is implemented in hardware 
	// on some keyboards and in the Windows device drivers for other
	// keyboards.
	//
	// Explicit handling also lets us improve the responsiveness of
	// the application.  The native Windows handling is to simply
	// queue key events asynchronously, regardless of the rate at
	// which the application is consuming them.  If a command takes
	// longer for us to process than the repeat interval, holding
	// down a key creates a backlog of repeat events that keeps
	// growing as long as the key is held down, which makes the app
	// feel sluggish.  Using a timer, we can throttle the repeat
	// rate to our actual consumption rate and avoid a backlog.
	struct KbAutoRepeat
	{
		KbAutoRepeat() { active = false; }

		bool active;				// auto-repeat is active
		int vkey;					// virtual key code we're repeating
		KeyPressType repeatMode;	// key press mode for repeats
	} kbAutoRepeat;

	// Joystick auto-repeat button.  This simulates the keyboard 
	// auto-repeat feature for joystick buttons.  The last button
	// pressed auto-repeats until released, or until another button
	// is pressed.
	struct JsAutoRepeat
	{
		JsAutoRepeat() { active = false; }

		bool active;				// auto-repeat is active
		int unit;					// logical joystick unit number
		int button;					// button number
		KeyPressType repeatMode;	// key press mode for repeats
	} jsAutoRepeat;

	// Start joystick auto repeat mode
	void JsAutoRepeatStart(int unit, int button, KeyPressType repeatMode);

	// joystick auto repeat timer handler
	void OnJsAutoRepeatTimer();

	// Start keyboard auto-repeat mode
	void KbAutoRepeatStart(int vkey, KeyPressType repeatMode);

	// Keyboard auto repeat timer handler
	void OnKbAutoRepeatTimer();

	// Stop keyboard and joystick auto-repeat timers.  We call this
	// whenever a new joystick button or key press occurs, to stop
	// any previous auto-repeat.
	void StopAutoRepeat();

	// Keyboard command handlers by name.  We populate this at
	// construction, and use it to find the mapping from a command
	// by name to the handler function.  The config loader uses
	// this mapping to populate the command dispatch table.  We
	// index the commands by name to help reduce the chances of
	// something getting out of sync across versions or between
	// modules.
	//
	// For each command, we store the associated handler function,
	// and a list of the key/button assignments for the command.
	struct KeyCommand
	{
		KeyCommand(KeyCommandFunc func) : func(func) { }

		// handler function
		KeyCommandFunc func;

		// list of associated keys
		std::list<InputManager::Button> keys;
	};
	std::unordered_map<TSTRING, KeyCommand> commandsByName;

	// Mappings between our command names and the corresponding
	// menu command IDs.
	std::unordered_map<TSTRING, int> commandNameToMenuID;

	// Joystick command dispatch table.  This maps keys generated
	// by JsCommandKey() to command handler function pointers.
	//
	// Note that this table contains no entries here with the
	// unit number encoded as -1, representing generic buttons 
	// that match any joystick.  Instead, for each command
	// assigned to a unit -1 button, we create a separate entry
	// for the same command in each actual joystick unit.  That
	// means we can find every command in one lookup, whether
	// assigned to a particular joystick or to any joysticks.
	//
	// Unordered_map takes slightly longer for a lookup operation
	// than a simple array does (such as we use for keyboard key 
	// dispatch above), but it's still very fast - about 120ns
	// per lookup on average on my 4th gen i7.  That's negligible
	// for a joystick event, so I don't think anything more complex
	// is justified.
	std::unordered_map<int, std::list<KeyCommandFunc>> jsCommands;

	// add a command to the map
	void AddJsCommand(int unit, int button, KeyCommandFunc func);

	// create a key for the jsCommands table
	static inline int JsCommandKey(int unit, int button) { return (unit << 8) | button; }

	// Basic handlers for Next/Previous commands.  These handle
	// the core action part separately from the key processing, so
	// they can be called to carry out the effect of a Next/Prev
	// command for non-UI actions, such as attract mode.
	void DoCmdNext(bool fast);
	void DoCmdPrev(bool fast);

	// Common coin slot handler
	void DoCoinCommon(const QueuedKey &key, int slot);

	// Set the credit balance, updating the stored config value
	void SetCredits(float val);

	// Reset the coin balance.  This converts the current coin balance
	// to credits and clears the coin balance.
	void ResetCoins();

	// Display the current credit balance
	void DisplayCredits();

	// Get the effective credits.  This calculates the total number of
	// credits available, including banked credits and credits that
	// can be converted from the current coin balance.
	float GetEffectiveCredits();

	// Current banked credit balance.  This counts credits that have 
	// been converted from coins, but not the credits from the current
	// coin balance.  We keep the coin balance separate while coins
	// are being inserted, since the number of credits purchased can
	// vary as more coins are inserted (e.g., the typical US pricing
	// where 50 cents buys you one credit and $1 buys three).
	float bankedCredits;

	// Maximum credit balance.  This is the maximum allowed for the
	// effective credits.  Zero means there's no maximum.
	float maxCredits;

	// Current coin balance.  This counts the value of coins inserted
	// in the coin slots since the last coin reset.  The coin balance
	// is reset when a game is launched or when we exit the program.
	// Coins are also deducted whenever the coin value reaches the
	// highest level in the pricing model list: at that point we
	// deduct the coin value for the last entry from the coin balance,
	// and add the corresponding credit value to the credit balance.
	float coinBalance;

	// Coin slot values.  This sets the monetary value of each coin
	// slot, in arbitrary user-defined currency units.
	float coinVal[4];

	// Pricing model.  This is a list, in ascending order, of the
	// monetary values needed to reach each credit level, and the
	// corresponding number of credits at that level.
	struct PricePoint
	{
		PricePoint(float price, float credits) : price(price), credits(credits) { }
		float price;	// coin value needed to reach this level, in coinVal units
		float credits;	// credits awarded at this level; can be fractional (1/2, 1/4, 3/4)
	};
	std::list<PricePoint> pricePoints;

	// Real DMD interface
	std::unique_ptr<RealDMD> realDMD;

	// get/set the DMD enabling status
	enum RealDMDStatus { RealDMDAuto, RealDMDEnable, RealDMDDisable };
	RealDMDStatus GetRealDMDStatus() const;
	void SetRealDMDStatus(RealDMDStatus stat);

	//
	// Button command handlers
	//

	void CmdSelect(const QueuedKey &key);			// select menu item
	void CmdExit(const QueuedKey &key);				// exit menu level
	void CmdNext(const QueuedKey &key);				// next item
	void CmdPrev(const QueuedKey &key);				// previous item
	void CmdNextPage(const QueuedKey &key);			// next page/group
	void CmdPrevPage(const QueuedKey &key);			// previous page/group
	void CmdCoin1(const QueuedKey &key);			// insert coin in slot 1
	void CmdCoin2(const QueuedKey &key);			// insert coin in slot 2
	void CmdCoin3(const QueuedKey &key);			// insert coin in slot 3
	void CmdCoin4(const QueuedKey &key);			// insert coin in slot 4
	void CmdCoinDoor(const QueuedKey &key);			// coin door open/close
	void CmdService1(const QueuedKey &key);			// Service 1/Escape
	void CmdService2(const QueuedKey &key);			// Service 2/Down
	void CmdService3(const QueuedKey &key);			// Service 3/Up
	void CmdService4(const QueuedKey &key);			// Service 4/Enter
	void CmdFrameCounter(const QueuedKey &key);		// Toggle on-screen frame counter/performance stats
	void CmdFullScreen(const QueuedKey &key);		// Toggle full-screen mode
	void CmdSettings(const QueuedKey &key);			// Show settings dialog
	void CmdRotateMonitorCW(const QueuedKey &key);	// Rotate monitor by 90 degrees clockwise
	void CmdRotateMonitorCCW(const QueuedKey &key);	// Rotate monitor by 90 degrees counter-clockwise
	void CmdLaunch(const QueuedKey &key);           // Launch game
	void CmdExitGame(const QueuedKey &key);			// Exit running game
	void CmdGameInfo(const QueuedKey &key);         // Show game info box
	void CmdInstCard(const QueuedKey &key);			// Show instruction card
};

