class KPK_Menu extends UIScriptedMenu
{
	protected static ref KPK_Menu s_Instance;
	protected static int s_ClosedAt;

	// pages
	static const int P_PROFILE = 0;
	static const int P_MAP = 1;
	static const int P_PARTY = 2;
	static const int P_FACTIONS = 3;
	static const int P_TASKS = 4;
	static const int P_NOTES = 5;
	static const int P_CHAT = 6;
	static const int P_CONTACTS = 7;
	static const int P_METRO = 8;        // «Куратор» only: metro lines (needs the Drezina mod)
	static const int P_LOGIN = 9;
	static const int P_BOOT = 10;
	static const int TAB_COUNT = 9;

	// button ids
	static const int B_TAB = 1;          // 1..9
	static const int B_CLOSE = 90;
	static const int B_LOGIN = 100;
	static const int B_MAP_SELF = 200;
	static const int B_MAP_GOTO = 201;
	static const int B_MAP_DEL = 202;
	static const int B_TYPE_PREV = 203;
	static const int B_TYPE_NEXT = 204;
	static const int B_SCOPE_PREV = 205;
	static const int B_SCOPE_NEXT = 206;
	static const int B_MAP_HERE = 207;
	static const int B_MAP_NEW = 208;
	static const int B_P_INVITE = 300;
	static const int B_P_LEAVE = 301;
	static const int B_P_KICK = 302;
	static const int B_INV_ACCEPT = 303;
	static const int B_INV_DECLINE = 304;
	static const int B_LABELS = 305;
	static const int B_F_LEAVE = 400;
	static const int B_F_INVITE = 401;
	static const int B_F_KICK = 402;
	static const int B_F_MEMBER = 403;
	static const int B_F_DEPUTY = 404;
	static const int B_F_LEADER = 405;
	static const int B_F_MAP = 406;
	static const int B_A_ENC_DESC = 420;
	static const int B_A_ENC_LEADER = 421;
	static const int B_A_ENC_MEMBERS = 422;
	static const int B_A_ENC_LOC = 423;
	static const int B_A_HIDDEN = 424;
	static const int B_A_MATES = 425;
	static const int B_A_COLOR = 426;
	static const int B_A_LOC_TEXT = 427;
	static const int B_A_LOC_ME = 428;
	static const int B_A_DESC = 429;
	static const int B_A_LEADER = 430;
	static const int B_A_ADD = 431;
	static const int B_A_CREATE = 432;
	static const int B_A_REMOVE = 433;
	static const int B_A_LOC_CLEAR = 434;
	static const int B_N_NEW = 500;
	static const int B_N_SAVE = 501;
	static const int B_N_DEL = 502;
	static const int B_TS_NEW = 510;
	static const int B_TS_DONE = 511;
	static const int B_TS_FAIL = 512;
	static const int B_TS_DEL = 513;
	static const int B_TS_SEND = 514;
	static const int B_TS_CANCEL = 515;
	static const int B_TS_SCOPE = 516;
	static const int B_C_SEND = 600;
	static const int B_C_PUBLIC = 601;
	static const int B_C_OLDER = 602;
	static const int B_C_NEWER = 603;
	static const int B_K_ADD = 700;
	static const int B_K_DEL = 701;
	static const int B_K_WRITE = 702;
	static const int B_K_PARTY = 703;
	static const int CHAT_LINES = 11;
	static const int B_R_CALL = 800;
	static const int B_T_INSPECT = 900;
	static const int B_T_SELF = 901;
	static const int B_T_ADMIN_ADD = 902;
	static const int B_T_ADMIN_DEL = 903;
	static const int B_R_ANSWER = 801;
	static const int B_R_END = 802;
	static const int B_R_DIAL = 803;      // brick: call the selected contact
	static const int B_R_DROP = 804;      // brick: remove the selected contact
	static const int B_M_LINE = 1000;     // metro: show the whole selected line
	static const int B_M_ALL = 1001;      // metro: show all lines
	static const int B_M_RELOAD = 1002;   // metro: read the lines again
	static const int B_M_DEL = 1003;      // metro: delete the selected line (press twice)

	protected ref array<ref KPK_Button> m_Buttons;
	protected ref array<ref KPK_Button> m_Tabs;
	protected ref array<Widget> m_Pages;
	protected ref array<KPK_List> m_Lists;
	protected int m_Page = -1;
	protected int m_LastTab = P_PROFILE;
	protected bool m_Visible;
	protected float m_Clock;
	protected float m_BlinkTime;
	protected bool m_Blink;
	protected float m_MapRedrawTime;

	protected Widget m_Screen;
	protected TextWidget m_HeaderLeft;
	protected TextWidget m_HeaderRight;
	protected TextWidget m_Footer;
	protected ref KPK_Skin m_Skin;
	protected ref KPK_Theme m_Theme;
	protected bool m_Full = true;            // false on the brick: profile, map and party only
	protected bool m_WasLoggedIn;
	protected Widget m_RepCard;
	protected MultilineTextWidget m_PartyHint;
	protected TextWidget m_StatusRight;
	protected MultilineTextWidget m_SideInfo;
	protected float m_SessionTime;
	protected float m_AmbientTime = 12;
	protected EffectSound m_Hiss;
	protected int m_Signal = 4;
	protected float m_SignalTime;
	protected Widget m_MapHolder;
	protected float m_MapScale = 0.3;
	protected vector m_MapPos;
	protected bool m_HasMapPos;
	protected string m_Status = "";

	// boot
	protected MultilineTextWidget m_BootText;
	protected float m_BootTime;
	protected bool m_Booting;

	// login
	protected TextWidget m_LoginTitle;
	protected TextWidget m_LoginCallsignLabel;
	protected EditBoxWidget m_LoginCallsign;
	protected TextWidget m_LoginCallsignFixed;
	protected EditBoxWidget m_LoginPassword;
	protected TextWidget m_LoginRepeatLabel;
	protected Widget m_LoginRepeatBox;
	protected EditBoxWidget m_LoginRepeat;
	protected TextWidget m_LoginSteam;
	protected MultilineTextWidget m_LoginHint;
	protected ref KPK_Button m_LoginButton;

	// profile
	protected MultilineTextWidget m_ProfileText;
	protected MultilineTextWidget m_VitalsText;
	protected MultilineTextWidget m_RepText;

	// metro (admin)
	protected ref KPK_List m_MetroLineList;
	protected ref KPK_List m_MetroPointList;
	protected MultilineTextWidget m_MetroInfo;
	protected ref array<ref KPK_MetroLine> m_MetroLines = new array<ref KPK_MetroLine>();
	protected ref array<ref KPK_MetroCross> m_MetroCross = new array<ref KPK_MetroCross>();
	protected ref array<vector> m_MetroPointPos = new array<vector>();
	protected ref array<string> m_MetroPointName = new array<string>();
	protected int m_MetroSel = -1;           // selected line (index in m_MetroLines)
	protected bool m_MetroHasFocus;          // a point of the line was picked: mark it on the map
	protected vector m_MetroFocus;
	protected string m_MetroFocusName;
	protected int m_MetroDelArm;             // line id waiting for the second press of «УДАЛИТЬ»
	protected int m_MetroDelWait;            // line id asked to be deleted, waiting for new data
	protected float m_MetroDelTime;
	protected int m_MetroVer = -1;           // Drezina data version the lists were built from

	// map
	protected MapWidget m_Map;
	protected ref KPK_MapHandler m_MapHandler;
	protected ref KPK_List m_MarkerList;
	protected EditBoxWidget m_MarkerName;            // shown wrapped by its KPK_TextBox
	protected Widget m_PageHost;                     // the tab pages, below the screen effects
	protected ref array<bool> m_Built;               // which tab pages exist already
	protected TextWidget m_MarkerType;
	protected TextWidget m_MarkerScope;
	protected TextWidget m_MapInfo;
	protected int m_NewType;
	protected int m_NewScope;
	protected int m_EditId = -1;       // marker being edited, -1 = creating a new one
	protected int m_EditScope;
	protected TextWidget m_EditorTitle;
	protected MultilineTextWidget m_MapHint;
	protected ref array<ref KPK_Marker> m_MarkerRows;

	// party
	protected ref KPK_List m_PartyList;
	protected ref KPK_List m_InviteList;
	protected ref KPK_List m_BrickContacts;   // brick: contacts for quick calls
	protected EditBoxWidget m_PartyInviteName;
	protected TextWidget m_PartyInfo;
	protected ref KPK_Button m_LabelsButton;
	protected MultilineTextWidget m_RadioText;
	protected EditBoxWidget m_RadioName;
	protected ref array<ref KPK_InviteView> m_InviteRows;

	// factions
	protected ref KPK_List m_FactionList;
	protected ref KPK_List m_MemberList;
	protected MultilineTextWidget m_FactionText;
	protected EditBoxWidget m_FactionInviteName;
	protected Widget m_FactionManage;
	protected Widget m_AdminBox;
	protected EditBoxWidget m_AdminText;
	protected TextWidget m_ManageHint;
	protected TextWidget m_FactionHint;
	protected ref map<int, ref KPK_Button> m_ById;
	protected string m_ConfirmRemove = "";

	// tasks
	protected ref KPK_List m_TaskList;
	protected MultilineTextWidget m_TaskText;
	protected Widget m_TaskViewBox;
	protected Widget m_TaskEditBox;
	protected EditBoxWidget m_TaskTitleEdit;
	protected EditBoxWidget m_TaskRewardEdit;
	protected EditBoxWidget m_TaskDescEdit;
	protected bool m_TaskEditing;
	protected Widget m_TaskPickBox;
	protected ref KPK_List m_TaskFactionList;
	protected ref array<string> m_TaskTargets = new array<string>();
	protected int m_TaskScope = KPK_Scope.PARTY;

	// notes
	protected ref KPK_List m_NoteList;
	protected EditBoxWidget m_NoteTitle;
	protected EditBoxWidget m_NoteText;
	protected TextWidget m_NoteInfo;
	protected int m_NoteId;            // 0 = new note
	protected bool m_SelectNewestNote;
	protected int m_NoteShown = -2;

	// chat
	protected MultilineTextWidget m_ChatLog;
	protected TextWidget m_ChatChannel;
	protected EditBoxWidget m_ChatEdit;            // shown wrapped on two lines; Enter sends
	protected string m_ChatTo = "";     // callsign for private messages, "" = general channel
	protected int m_ChatOffset;          // lines scrolled back from the newest

	// contacts
	protected ref KPK_List m_ContactList;
	// admin KPK «Куратор»: the terminal page instead of the profile
	protected bool m_Admin;
	int m_BuiltModel = -1;        // the model the widgets were built for
	protected int m_FitW;          // screen size the device was fitted for
	protected int m_FitH;
	protected int m_FitModel;
	protected Widget m_Device;
	protected ref KPK_List m_PlayerList;
	protected EditBoxWidget m_AdminName;
	protected MultilineTextWidget m_TermText;
	protected EditBoxWidget m_ContactName;
	protected MultilineTextWidget m_ContactText;

	// =================================================================== open / close

	static bool IsOpen()
	{
		return s_Instance && s_Instance.m_Visible;
	}

	static void Open()
	{
		if (IsOpen())
			return;
		// the window is built once and then only shown again: building it anew on every open made the
		// engine refuse new text widgets after a few opens (empty fields, black screen), even with the
		// old window destroyed. It is rebuilt only for another KPK model.
		int model = KPK_Model.BRICK;
		if (KPK_Client.s_View)
			model = KPK_Client.s_View.DeviceModel;
		if (s_Instance && s_Instance.layoutRoot && s_Instance.m_BuiltModel == model)
		{
			GetGame().GetUIManager().ShowScriptedMenu(s_Instance, null);
			return;
		}
		if (s_Instance)
			s_Instance.ReleaseLayout();
		s_Instance = new KPK_Menu();
		GetGame().GetUIManager().ShowScriptedMenu(s_Instance, null);
	}

	// the same key press that closed the KPK must not open it again
	static bool JustClosed()
	{
		return GetGame().GetTime() - s_ClosedAt < 400;
	}

	// leaving the server: the kept window goes away
	static void Shutdown()
	{
		CloseIfOpen();
		if (s_Instance)
			s_Instance.ReleaseLayout();
		s_Instance = null;
	}

	static void CloseIfOpen()
	{
		if (IsOpen())
			GetGame().GetUIManager().HideScriptedMenu(s_Instance);
	}

	void KPK_Menu()
	{
		m_Buttons = new array<ref KPK_Button>();
		m_Tabs = new array<ref KPK_Button>();
		m_Pages = new array<Widget>();
		m_Lists = new array<KPK_List>();
		m_MarkerRows = new array<ref KPK_Marker>();
		m_InviteRows = new array<ref KPK_InviteView>();
		m_ById = new map<int, ref KPK_Button>();
		m_MapHandler = new KPK_MapHandler();
		m_MapHandler.m_Menu = this;
	}

	void ReleaseLayout()
	{
		DestroyMap();
		if (layoutRoot)
		{
			layoutRoot.Unlink();
			layoutRoot = null;
		}
	}

	// a closed KPK frees its widgets a moment later (not inside OnHide, the manager still works with it)
	void ReleaseIfHidden()
	{
		if (!m_Visible)
			ReleaseLayout();
	}

	void ~KPK_Menu()
	{
		KPK_Client.s_OnView.Remove(OnView);
		KPK_Client.s_OnStatus.Remove(OnStatus);
		KPK_Client.s_OnMates.Remove(OnMates);
		KPK_Client.s_OnChat.Remove(OnChat);
		KPK_Client.s_OnChatOlder.Remove(OnChatOlder);
		KPK_Sound.StopHiss(m_Hiss);
		if (layoutRoot)
			layoutRoot.Unlink();
	}

	override Widget Init()
	{
		// shown again: the widgets are there, only the boot sequence and the data start over
		if (layoutRoot)
		{
			StartBoot();
			return layoutRoot;
		}
		// every model has its own theme; the view already says which device was opened
		int model = KPK_Model.BRICK;
		if (KPK_Client.s_View)
			model = KPK_Client.s_View.DeviceModel;
		m_BuiltModel = model;
		m_Theme = KPK_Theme.Get(model);
		m_Full = KPK_Const.FullAccess(model);
		KPK_UI.Apply(m_Theme);

		layoutRoot = GetGame().GetWorkspace().CreateWidgets(KPK_UI.DIR + "kpk_main.layout");
		m_Screen = layoutRoot.FindAnyWidget("Screen");
		m_Screen.SetColor(m_Theme.Screen);
		Widget device = layoutRoot.FindAnyWidget("Device");
		m_Device = device;
		m_FitModel = model;
		FitDevice(device, model);
		m_Skin = new KPK_Skin();

		BuildChrome();
		// the host of the tab pages comes before the login and boot pages: lying above them, the
		// full-screen host took the clicks and the password box could not be focused
		// it covers only the content area (not the tabs, title bar or status bar)
		if (m_Full)
			m_PageHost = KPK_UI.Frame(m_Screen, 0.178, 0.095, 0.812, 0.83);
		else
			m_PageHost = KPK_UI.Frame(m_Screen, 0.012, 0.165, 0.976, 0.762);
		// login and boot are built before everything else: if the engine's widget limit is reached it
		// must never be on the password box (the box appeared, the input inside it did not)
		Widget loginPage = BuildLogin();
		Widget bootPage = BuildBoot();
		m_Admin = model == KPK_Model.ADMIN;
		int metroTab = 0;
		if (m_Tabs.Count() > P_METRO && m_Tabs[P_METRO] && m_Tabs[P_METRO].m_Root)
			metroTab = 1;
		Print("[KPK] menu: tabs " + m_Tabs.Count() + ", metro tab " + metroTab + ", admin " + m_Admin + ", model " + model);
		// the tab pages are built when first shown: the engine caps how many widgets may be created
		// (and does not give them back at once when a window is destroyed). Building all eight pages up
		// front ran into that cap: empty fields, missing buttons. Pages live in a host under the screen
		// effects, so a page built later still lies below the glass.
		m_Built = new array<bool>();
		for (int tab = 0; tab < TAB_COUNT; tab++)
		{
			Widget slot = KPK_UI.Frame(m_PageHost, 0, 0, 0.01, 0.01);
			slot.Show(false);
			m_Pages.Insert(slot);
			m_Built.Insert(false);
		}
		m_Pages.Insert(loginPage);
		m_Pages.Insert(bootPage);

		// decoration last: the engine caps how many widgets a menu may create, and when the case and
		// screen effects came first the buttons built after them failed ("could not create a button")
		m_Skin.BuildScreenFx(m_Screen);
		m_Skin.BuildGlass(m_Screen);

		KPK_Client.s_OnView.Insert(OnView);
		KPK_Client.s_OnStatus.Insert(OnStatus);
		KPK_Client.s_OnMates.Insert(OnMates);
		KPK_Client.s_OnChat.Insert(OnChat);
		KPK_Client.s_OnChatOlder.Insert(OnChatOlder);

		StartBoot();
		return layoutRoot;
	}

	// The case around the screen is a front render of the model itself (KPK/data/kpk_ui_<model>_ca.paa,
	// the screen window cut out). The interface sits in that window, the case lies on top of it.
	protected void FitDevice(Widget device, int model)
	{
		string art = "kpk_ui_brick";
		float aspect = 1.0;
		float su = 0.24561;
		float sv = 0.13158;
		float sw = 0.50877;
		float sh = 0.43860;
		switch (model)
		{
			case KPK_Model.STALKER:
				art = "kpk_ui_m1"; aspect = 2.0; su = 0.19655; sv = 0.15384; sw = 0.58656; sh = 0.64162;
				break;
			case KPK_Model.MILITARY:
				art = "kpk_ui_mil"; aspect = 2.0; su = 0.21803; sv = 0.20196; sw = 0.56519; sh = 0.62245;
				break;
			case KPK_Model.SCIENCE:
				art = "kpk_ui_nii"; aspect = 2.0; su = 0.22051; sv = 0.19430; sw = 0.59730; sh = 0.65590;
				break;
			case KPK_Model.ADMIN:
				art = "kpk_ui_admin"; aspect = 2.0; su = 0.15244; sv = 0.12195; sw = 0.59146; sh = 0.64634;
				break;
			case KPK_Model.MONOLITH:
				art = "kpk_ui_mono"; aspect = 2.0; su = 0.18821; sv = 0.15213; sw = 0.56682; sh = 0.70821;
				break;
		}

		// keep the render's proportions on any screen: as large as fits
		int scrW, scrH;
		GetScreenSize(scrW, scrH);
		// a zero size (window minimised / not ready) made the device NaN: the interface filled the screen
		if (scrW <= 0 || scrH <= 0)
		{
			scrW = 1920;
			scrH = 1080;
		}
		m_FitW = scrW;
		m_FitH = scrH;
		float hFrac = 0.92;
		float wFrac = hFrac * scrH * aspect / scrW;
		if (wFrac > 0.97)
		{
			wFrac = 0.97;
			hFrac = wFrac * scrW / (aspect * scrH);
		}
		// the brick: a bigger screen; the case runs off the bottom of the screen and fades out there
		if (model == KPK_Model.BRICK)
		{
			hFrac = 1.3;
			wFrac = hFrac * scrH * aspect / scrW;
			float top = 0.0;
			device.SetPos(0, top + hFrac * 0.5 - 0.5);
		}
		device.SetSize(wFrac, hFrac);

		// the case picture lies over the interface (its window is cut out): the screen reaches a little
		// under the window edge so no gap shows, and the chamfered corners are covered by the case itself
		float pad = 0.004;
		m_Screen.SetPos(su - pad, sv - pad * aspect);
		m_Screen.SetSize(sw + pad * 2, sh + pad * 2 * aspect);

		// an ImageWidget without a picture draws a plain white box, so it stays hidden until one loads
		ImageWidget img = ImageWidget.Cast(layoutRoot.FindAnyWidget("Art"));
		if (!img)
			return;
		// the case always over the interface, whatever order the engine picks (it ignores the mouse)
		m_Screen.SetSort(10);
		img.SetSort(30);
		img.Show(false);
		array<string> paths = {"\\KPK\\data\\" + art + "_ca.paa", "KPK\\data\\" + art + "_ca.paa", "KPK/data/" + art + "_ca.paa"};
		foreach (string path : paths)
		{
			if (img.LoadImageFile(0, path))
			{
				img.SetImage(0);
				img.Show(true);
				Print("[KPK] case picture: " + path);
				return;
			}
		}
		Print("[KPK] !!! case picture not found: KPK\\data\\" + art + "_ca.paa (run tools/convert_textures.bat and rebuild the PBO)");
	}

	override void OnShow()
	{
		super.OnShow();
		m_Visible = true;
		if (layoutRoot)
			layoutRoot.Show(true);
		if (!m_Hiss)
			m_Hiss = KPK_Sound.StartHiss();
		// reopened window: the theme colours are static, put this window's theme back and boot again
		if (m_Theme)
			KPK_UI.Apply(m_Theme);
		if (!m_Booting && m_Pages && m_Pages.Count() > P_BOOT)
			StartBoot();

		GetGame().GetMission().AddActiveInputExcludes({"menu"});
		GetGame().GetInput().ChangeGameFocus(1);
		GetGame().GetUIManager().ShowUICursor(true);
		if (GetGame().GetMission().GetHud())
			GetGame().GetMission().GetHud().Show(false);
	}

	override void OnHide()
	{
		super.OnHide();
		m_Visible = false;
		s_ClosedAt = GetGame().GetTime();
		if (layoutRoot)
			layoutRoot.Show(false);
		GetGame().GetMission().RemoveActiveInputExcludes({"menu"}, true);
		GetGame().GetInput().ResetGameFocus();
		GetGame().GetUIManager().ShowUICursor(false);
		if (GetGame().GetMission().GetHud())
			GetGame().GetMission().GetHud().Show(true);
		KPK_Sound.StopHiss(m_Hiss);
		m_Hiss = null;
		KPK_Client.NotifyClosed();
	}

	override void OnPlayerDeath()
	{
		super.OnPlayerDeath();
		CloseIfOpen();
	}

	// =================================================================== chrome: header, tabs, footer

	protected KPK_Button AddButton(Widget parent, int id, string text, float x, float y, float w, float h)
	{
		KPK_Button b = new KPK_Button(parent, id, text, x, y, w, h);
		m_Buttons.Insert(b);
		if (id >= 0)
			m_ById.Set(id, b);
		return b;
	}

	string PageName(int page)
	{
		if (page >= 0 && page < TAB_COUNT)
			return m_Theme.Tabs[page];
		if (page == P_LOGIN)
			return "Вход в сеть";
		return "Загрузка";
	}

	// an old program window: title bar, menu bar, sidebar with modules, segmented status bar
	protected void BuildChrome()
	{
		// title bar
		KPK_UI.Panel(m_Screen, 0, 0, 1, 0.045, KPK_UI.C_TITLEBAR);
		KPK_UI.Panel(m_Screen, 0, 0, 1, 0.004, (KPK_UI.C_TITLE & 0x00FFFFFF) | 0x40000000);
		KPK_UI.Panel(m_Screen, 0.008, 0.012, 0.012, 0.022, KPK_UI.C_ACCENT);
		m_HeaderLeft = KPK_UI.Text(m_Screen, "", 0.026, 0, 0.7, 0.045, 15, KPK_UI.C_TITLE);
		KPK_UI.TextC(m_Screen, "_", 0.885, 0.004, 0.03, 0.037, KPK_UI.C_DIM);
		KPK_UI.TextC(m_Screen, "[]", 0.917, 0.004, 0.03, 0.037, KPK_UI.C_DIM);
		AddButton(m_Screen, B_CLOSE, "X", 0.952, 0.005, 0.04, 0.035);

		// menu bar
		KPK_UI.Panel(m_Screen, 0, 0.045, 1, 0.035, KPK_UI.C_MENUBAR);
		KPK_UI.Text(m_Screen, m_Theme.MenuLine, 0.015, 0.045, 0.6, 0.035, 13, KPK_UI.C_DIM);
		m_HeaderRight = KPK_UI.Text(m_Screen, "", 0.62, 0.045, 0.37, 0.035, 13, KPK_UI.C_TEXT);
		KPK_UI.Panel(m_Screen, 0, 0.08, 1, 0.003, KPK_UI.C_EDGE);

		// the brick: square screen, its three modules as a row of tabs under the menu bar
		if (!m_Full)
		{
			KPK_UI.Panel(m_Screen, 0, 0.083, 1, 0.07, KPK_UI.C_SIDE);
			for (int bt = 0; bt < TAB_COUNT; bt++)
			{
				string bName = m_Theme.Tabs[bt];
				int bNumber = bt + 1;
				if (!m_Theme.Glitch)
					bName = "[" + bNumber.ToString() + "] " + bName;
				float bx = 0.01 + bt * 0.33;
				if (bt > 2)
					bx = 0.01;
				KPK_Button btab = new KPK_Button(m_Screen, B_TAB + bt, bName, bx, 0.09, 0.32, 0.056, true);
				m_Tabs.Insert(btab);
				m_Buttons.Insert(btab);
			}
			KPK_UI.Panel(m_Screen, 0, 0.153, 1, 0.003, KPK_UI.C_LINE);
			m_SideInfo = KPK_UI.MText(m_Screen, 0, 0, 0.01, 0.01, 12, KPK_UI.C_DIM);
			m_SideInfo.Show(false);
		}
		else
		{
		// sidebar
		KPK_UI.Panel(m_Screen, 0, 0.083, 0.165, 0.852, KPK_UI.C_SIDE);
		string modules = "МОДУЛИ";
		if (m_Theme.Glitch)
			modules = "# # #";
		KPK_UI.Text(m_Screen, modules, 0.012, 0.09, 0.15, 0.03, 13, KPK_UI.C_ACCENT);
		for (int i = 0; i < TAB_COUNT; i++)
		{
			string tabName = m_Theme.Tabs[i];
			int tabNumber = i + 1;
			if (!m_Theme.Glitch)
				tabName = "[" + tabNumber.ToString() + "] " + tabName;
			KPK_Button tab = new KPK_Button(m_Screen, B_TAB + i, tabName, 0.008, 0.125 + i * 0.065, 0.15, 0.056, true);
			m_Tabs.Insert(tab);
			m_Buttons.Insert(tab);
		}
		KPK_UI.Panel(m_Screen, 0.012, 0.73, 0.14, 0.002, KPK_UI.C_LINE);
		m_SideInfo = KPK_UI.MText(m_Screen, 0.012, 0.74, 0.15, 0.19, 12, KPK_UI.C_DIM);
		KPK_UI.Panel(m_Screen, 0.165, 0.083, 0.002, 0.852, KPK_UI.C_LINE);
		}

		// status bar
		KPK_UI.Panel(m_Screen, 0, 0.935, 1, 0.065, KPK_UI.C_MENUBAR);
		KPK_UI.Panel(m_Screen, 0, 0.935, 1, 0.003, KPK_UI.C_EDGE);
		m_Footer = KPK_UI.Text(m_Screen, "", 0.012, 0.94, 0.6, 0.055, 14, KPK_UI.C_TEXT);
		KPK_UI.Panel(m_Screen, 0.62, 0.945, 0.002, 0.045, KPK_UI.C_LINE);
		m_StatusRight = KPK_UI.Text(m_Screen, "", 0.632, 0.94, 0.36, 0.055, 13, KPK_UI.C_DIM);
	}

	protected Widget Content()
	{
		if (m_PageHost)
			return KPK_UI.Frame(m_PageHost, 0, 0, 1, 1);
		if (!m_Full)
			return KPK_UI.Frame(m_Screen, 0.012, 0.165, 0.976, 0.762);
		return KPK_UI.Frame(m_Screen, 0.178, 0.095, 0.812, 0.83);
	}

	// a tab page is built on its first show
	protected void EnsurePage(int page)
	{
		if (!m_Built || page < 0 || page >= TAB_COUNT || m_Built[page])
			return;
		if (!m_Full && page > P_PARTY)
			return;
		Widget built;
		switch (page)
		{
			case P_PROFILE:
				if (m_Admin)
					built = BuildAdmin();
				else
					built = BuildProfile();
				break;
			case P_MAP: built = BuildMap(); break;
			case P_PARTY: built = BuildParty(); break;
			case P_FACTIONS: built = BuildFactions(); break;
			case P_TASKS: built = BuildTasks(); break;
			case P_NOTES: built = BuildNotes(); break;
			case P_CHAT: built = BuildChat(); break;
			case P_CONTACTS: built = BuildContacts(); break;
			case P_METRO:
				if (m_Admin)
					built = BuildMetro();
				break;
		}
		if (!built)
			return;
		if (m_Pages[page])
			m_Pages[page].Unlink();
		m_Pages[page] = built;
		m_Built[page] = true;
	}

	// a tab page that is left: its widgets go away (the engine caps the widgets of all mods together;
	// keeping every visited page made the later ones fail). It is built again on the next visit.
	protected void DestroyPage(int page)
	{
		if (!m_Built || page < 0 || page >= TAB_COUNT || !m_Built[page])
			return;
		if (page == P_MAP || page == P_METRO)
			DestroyMap();
		if (m_Pages[page])
			m_Pages[page].Unlink();
		Widget slot = KPK_UI.Frame(m_PageHost, 0, 0, 0.01, 0.01);
		if (slot)
			slot.Show(false);
		m_Pages[page] = slot;
		m_Built[page] = false;
		PruneDeadWidgets();
	}

	// buttons, lists and text boxes whose widgets were destroyed with their page
	protected void PruneDeadWidgets()
	{
		for (int b = m_Buttons.Count() - 1; b >= 0; b--)
		{
			if (!m_Buttons[b] || !m_Buttons[b].m_Root)
				m_Buttons.Remove(b);
		}
		array<int> ids = m_ById.GetKeyArray();
		foreach (int id : ids)
		{
			KPK_Button idButton = m_ById.Get(id);
			if (!idButton || !idButton.m_Root)
				m_ById.Remove(id);
		}
		for (int l = m_Lists.Count() - 1; l >= 0; l--)
		{
			KPK_List list = m_Lists[l];
			if (!list || !list.m_Prev || !list.m_Prev.m_Root)
				m_Lists.Remove(l);
		}
		for (int t = m_TextBoxes.Count() - 1; t >= 0; t--)
		{
			if (!m_TextBoxes[t] || !m_TextBoxes[t].m_Input)
				m_TextBoxes.Remove(t);
		}
	}

	protected void ShowTabs(bool show)
	{
		for (int i = 0; i < m_Tabs.Count(); i++)
			m_Tabs[i].Show(show && (m_Full || i <= P_PARTY) && (i != P_METRO || m_Admin));
	}

	protected void ShowPage(int page)
	{
		// only the shown tab page exists: the one left is destroyed before the next one is built
		if (page != m_Page)
			DestroyPage(m_Page);
		EnsurePage(page);
		if (page != m_Page)
			SetFocus(null);   // a field on a hidden page would keep taking the keys
		m_Page = page;
		for (int i = 0; i < m_Pages.Count(); i++)
			m_Pages[i].Show(i == page);
		for (int t = 0; t < m_Tabs.Count(); t++)
			m_Tabs[t].SetActive(t == page);
		if (page < TAB_COUNT)
			m_LastTab = page;
		// the map widget only exists while its page is shown: a hidden MapWidget
		// sometimes bleeds through other widgets (map tiles inside text fields)
		if (page == P_MAP || page == P_METRO)
			CreateMap();
		else
			DestroyMap();
		ShowTabs(page < TAB_COUNT);
		RefreshPage();

		if (m_HeaderLeft)
			RefreshHeader();
	}

	protected void CreateMap()
	{
		if (m_Map)
			return;
		m_Map = MapWidget.Cast(KPK_UI.Make("kpk_map.layout", m_MapHolder, 0, 0, 1, 1));
		if (!m_Map)
			return;
		m_Map.SetHandler(m_MapHandler);
		m_Map.SetScale(m_MapScale);
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (m_HasMapPos)
			m_Map.SetMapPos(m_MapPos);
		else if (player)
			m_Map.SetMapPos(player.GetPosition());
	}

	protected void DestroyMap()
	{
		if (!m_Map)
			return;
		m_MapScale = m_Map.GetScale();
		m_MapPos = m_Map.GetMapPos();
		m_HasMapPos = true;
		m_Map.Unlink();
		m_Map = null;
	}

	// =================================================================== boot

	protected Widget BuildBoot()
	{
		Widget page = KPK_UI.Frame(m_Screen, 0.19, 0.11, 0.78, 0.8);
		m_BootText = KPK_UI.MText(page, 0, 0, 1, 1, 17, KPK_UI.C_TEXT);
		return page;
	}

	protected void StartBoot()
	{
		// dial-up handshake while the KPK connects to the Zone network
		KPK_Sound.Play("modem");
		m_Booting = true;
		m_BootTime = 0;
		ShowPage(P_BOOT);
		UpdateBoot();
	}

	protected void UpdateBoot()
	{
		string net = "СЕТЬ ЗОНЫ";
		if (KPK_Client.s_View && KPK_Client.s_View.ServerName != "")
			net = KPK_Client.s_View.ServerName;

		string text = "";
		for (int i = 0; i < m_Theme.Boot.Count(); i++)
		{
			if (i > 0 && m_BootTime < i * 0.2 - 0.05)
				break;
			string line = m_Theme.Boot[i];
			line.Replace("%NET%", net);
			if (m_Theme.Glitch)
				line = KPK_Theme.Noise(line, 0.25);
			text += line + "\n";
			if (i == 0)
				text += "\n";
		}
		m_BootText.SetText(text);

		if (m_BootTime > 1.15)
		{
			m_Booting = false;
			OnView();
		}
	}

	// =================================================================== login / registration

	protected Widget BuildLogin()
	{
		Widget page = KPK_UI.Frame(m_Screen, 0.2, 0.12, 0.6, 0.8);
		KPK_UI.Card(page, 0, 0, 1, 0.86, "");
		m_LoginTitle = KPK_UI.Text(page, "", 0.05, 0.03, 0.9, 0.08, 20, KPK_UI.C_ACCENT);

		m_LoginCallsignLabel = KPK_UI.Text(page, "Позывной", 0.05, 0.14, 0.3, 0.07, 15, KPK_UI.C_DIM);
		m_LoginCallsign = AddEdit(KPK_UI.Edit(page, 0.35, 0.14, 0.6, 0.07));
		m_LoginCallsignFixed = KPK_UI.Text(page, "", 0.35, 0.14, 0.6, 0.07, 17, KPK_UI.C_WHITE);

		KPK_UI.Text(page, "Пароль", 0.05, 0.25, 0.3, 0.07, 15, KPK_UI.C_DIM);
		m_LoginPassword = AddPassword(KPK_UI.Password(page, 0.35, 0.25, 0.6, 0.07));

		m_LoginRepeatLabel = KPK_UI.Text(page, "Повтор пароля", 0.05, 0.36, 0.3, 0.07, 15, KPK_UI.C_DIM);
		m_LoginRepeatBox = KPK_UI.Frame(page, 0.35, 0.36, 0.6, 0.07);
		m_LoginRepeat = AddPassword(KPK_UI.Password(m_LoginRepeatBox, 0, 0, 1, 1));

		KPK_UI.Text(page, "Steam ID", 0.05, 0.47, 0.3, 0.07, 15, KPK_UI.C_DIM);
		m_LoginSteam = KPK_UI.Text(page, "", 0.35, 0.47, 0.6, 0.07, 17, KPK_UI.C_WHITE);

		m_LoginButton = AddButton(page, B_LOGIN, "", 0.3, 0.6, 0.4, 0.08);
		m_LoginHint = KPK_UI.MText(page, 0.05, 0.71, 0.9, 0.14, 13, KPK_UI.C_DIM);
		return page;
	}

	protected void RefreshLogin()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;

		if (!m_LoginSteam || !m_LoginCallsign || !m_LoginCallsignFixed || !m_LoginRepeatBox || !m_LoginButton)
		{
			Print("[KPK] login page incomplete (widget limit)");
			return;
		}
		m_LoginSteam.SetText(v.SteamId);
		bool newPassword = !v.Registered || v.NeedNewPassword;
		m_LoginRepeatLabel.Show(newPassword);
		m_LoginRepeatBox.Show(newPassword);
		m_LoginCallsign.GetParent().Show(!v.Registered);
		m_LoginCallsignFixed.Show(v.Registered);
		m_LoginCallsignFixed.SetText(v.Callsign);

		if (!v.Registered)
		{
			m_LoginTitle.SetText("РЕГИСТРАЦИЯ В СЕТИ");
			m_LoginButton.SetText("ЗАРЕГИСТРИРОВАТЬСЯ");
			m_LoginHint.SetText("Позывной виден другим сталкерам и не меняется. КПК привязывается к вашему Steam ID, пароль защищает доступ к данным.");
		}
		else if (v.NeedNewPassword)
		{
			m_LoginTitle.SetText("СБРОС ПАРОЛЯ");
			m_LoginButton.SetText("УСТАНОВИТЬ ПАРОЛЬ");
			m_LoginHint.SetText("Администратор сбросил ваш пароль. Задайте новый.");
		}
		else
		{
			m_LoginTitle.SetText("ИДЕНТИФИКАЦИЯ ПОЛЬЗОВАТЕЛЯ");
			m_LoginButton.SetText("ВОЙТИ");
			m_LoginHint.SetText("Введите пароль. После 5 ошибок доступ блокируется на минуту.");
		}
	}

	protected void SubmitLogin()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;

		if (!m_LoginPassword)
		{
			SetStatus("Ошибка интерфейса: поле пароля не создано. Закройте и откройте КПК");
			Print("[KPK] !!! login: password input missing");
			return;
		}
		string password = m_LoginPassword.GetText();
		if (!v.Registered || v.NeedNewPassword)
		{
			string repeat = "";
			if (m_LoginRepeat)
				repeat = m_LoginRepeat.GetText();
			if (password != repeat)
			{
				SetStatus("Пароли не совпадают");
				return;
			}
		}

		if (!v.Registered)
		{
			if (!m_LoginCallsign)
				return;
			KPK_Client.SendText(KPK_C.REGISTER, m_LoginCallsign.GetText(), password);
		}
		else
			KPK_Client.SendText(KPK_C.LOGIN, password);

		SetStatus("Обмен данными с сервером...");
	}

	// =================================================================== profile

	protected Widget BuildProfile()
	{
		Widget page = Content();
		Widget left = KPK_UI.Card(page, 0, 0, 0.49, 1, m_Theme.ProfileTitle);
		m_ProfileText = KPK_UI.MText(left, 0.04, 0.08, 0.92, 0.9, 15, KPK_UI.C_TEXT);

		Widget right = KPK_UI.Card(page, 0.51, 0, 0.49, 0.47, "СОСТОЯНИЕ ОРГАНИЗМА");
		m_VitalsText = KPK_UI.MText(right, 0.04, 0.16, 0.92, 0.82, 15, KPK_UI.C_TEXT);

		Widget rep = KPK_UI.Card(page, 0.51, 0.5, 0.49, 0.5, "РЕПУТАЦИЯ У ГРУППИРОВОК");
		m_RepCard = rep;
		m_RepText = KPK_UI.MText(rep, 0.04, 0.16, 0.92, 0.82, 15, KPK_UI.C_TEXT);
		return page;
	}

	protected void RefreshProfile()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;
		m_RepCard.Show(m_Full);
		KPK_Stats s = v.Stats;

		string faction = m_Theme.NoFaction;
		KPK_FactionView mine = MyFaction();
		if (mine)
			faction = mine.Name + " (" + KPK_Const.RoleName(v.MyRole) + ")";

		string text = "";
		text += "Позывной:  " + v.Callsign + "\n";
		text += m_Theme.RankLabel + ":  " + m_Theme.RankName(v.Rank) + "\n";
		if (m_Full)
			text += "Группировка:  " + faction + "\n";
		text += "Steam ID:  " + v.SteamId + "\n";
		text += "В сети с:  " + v.RegisteredDate + "\n";
		if (v.PrevOwner != "")
			text += "Прежний владелец КПК:  " + v.PrevOwner + "\n";
		text += "\n";
		text += "— БОЕВАЯ СТАТИСТИКА —\n";
		text += string.Format("Убито сталкеров:  %1\n", s.PlayerKills);
		text += string.Format("Убито заражённых:  %1\n", s.ZombieKills);
		text += string.Format("Добыто животных:  %1\n", s.AnimalKills);
		text += string.Format("Смертей:  %1   (самоубийств: %2)\n", s.Deaths, s.Suicides);
		int longest = s.LongestKill;
		text += string.Format("Самый дальний выстрел:  %1 м\n\n", longest);
		text += "— ЖИЗНЬ В ЗОНЕ —\n";
		text += "Текущая жизнь:  " + KPK_Util.Duration(s.CurrentLifeSec) + "\n";
		text += "Лучшая жизнь:  " + KPK_Util.Duration(s.BestLifeSec) + "\n";
		text += "Всего в Зоне:  " + KPK_Util.Duration(s.PlaytimeSec) + "\n";
		text += "Пройдено:  " + KPK_Util.Km(s.DistanceM) + "\n";
		m_ProfileText.SetText(text);

		KPK_Vitals vit = v.Vitals;
		string vt = "";
		vt += "Здоровье:  " + KPK_UI.Percent(vit.Health, 100) + "\n";
		int blood = vit.Blood;
		vt += "Кровь:  " + blood.ToString() + " мл  (" + KPK_UI.Percent(vit.Blood, 5000) + ")\n";
		vt += "Шок:  " + KPK_UI.Percent(100 - vit.Shock, 100) + "\n";
		vt += "Вода:  " + KPK_UI.Percent(vit.Water, PlayerConstants.SL_WATER_MAX) + "\n";
		vt += "Пища:  " + KPK_UI.Percent(vit.Energy, PlayerConstants.SL_ENERGY_MAX) + "\n\n";
		vt += "Квадрат:  " + KPK_UI.Grid(vit.PosX, vit.PosZ) + "\n";
		m_VitalsText.SetText(vt);

		string rt = "";
		foreach (KPK_RepView r : v.Reputation)
			rt += r.FactionName + ":  " + KPK_Const.ReputationName(r.Value) + "  (" + r.Value.ToString() + ")\n";
		if (v.Reputation.Count() == 0)
			rt += "Нет данных о группировках.\n";
		rt += "\n[ модуль репутации в разработке ]";
		m_RepText.SetText(rt);
	}

	// =================================================================== map

	protected Widget BuildMap()
	{
		m_EditId = -1;
		Widget page = Content();

		Widget mapBox = KPK_UI.Panel(page, 0, 0, 0.71, 1, 0xFF000000, true);
		m_MapHolder = KPK_UI.Frame(mapBox, 0.004, 0.004, 0.992, 0.992);
		// grid of a field map over the paper
		for (int gl = 1; gl < 10; gl++)
		{
			KPK_UI.Panel(mapBox, gl * 0.1, 0, 0.0015, 1, KPK_UI.C_GRID);
			KPK_UI.Panel(mapBox, 0, gl * 0.1, 1, 0.002, KPK_UI.C_GRID);
		}
		// green phosphor tint over the paper map
		KPK_UI.Panel(mapBox, 0, 0, 1, 1, KPK_UI.C_TINT);
		KPK_UI.Panel(mapBox, 0, 0, 1, 0.004, KPK_UI.C_EDGE);
		KPK_UI.Panel(mapBox, 0, 0.996, 1, 0.004, KPK_UI.C_EDGE);
		m_MapInfo = KPK_UI.Text(mapBox, "", 0.02, 0.94, 0.96, 0.05, 13, KPK_UI.C_ACCENT);

		Widget side = KPK_UI.Card(page, 0.725, 0, 0.275, 1, "МЕТКИ КПК");
		m_MarkerList = new KPK_List(side, 0.04, 0.06, 0.92, 0.44, 8, "— меток нет —");
		m_MarkerList.SetWrap(true);
		m_Lists.Insert(m_MarkerList);

		AddButton(side, B_MAP_GOTO, "НА МЕТКУ", 0.04, 0.505, 0.45, 0.04);
		AddButton(side, B_MAP_DEL, "УДАЛИТЬ", 0.51, 0.505, 0.45, 0.04);

		KPK_UI.Panel(side, 0.04, 0.56, 0.92, 0.002, KPK_UI.C_LINE);
		m_EditorTitle = KPK_UI.Text(side, "НОВАЯ МЕТКА", 0.04, 0.565, 0.6, 0.04, 14, KPK_UI.C_ACCENT);
		AddButton(side, B_MAP_NEW, "НОВАЯ", 0.64, 0.565, 0.32, 0.036);
		KPK_UI.Text(side, "Название", 0.04, 0.605, 0.92, 0.035, 13, KPK_UI.C_DIM);
		m_MarkerName = AddMulti(KPK_UI.Edit(side, 0.04, 0.64, 0.92, 0.06));

		AddButton(side, B_TYPE_PREV, "<", 0.04, 0.707, 0.14, 0.042);
		m_MarkerType = KPK_UI.Text(side, "", 0.19, 0.707, 0.62, 0.042, 14, KPK_UI.C_WHITE);
		AddButton(side, B_TYPE_NEXT, ">", 0.82, 0.707, 0.14, 0.042);

		AddButton(side, B_SCOPE_PREV, "<", 0.04, 0.756, 0.14, 0.042);
		m_MarkerScope = KPK_UI.Text(side, "", 0.19, 0.756, 0.62, 0.042, 14, KPK_UI.C_WHITE);
		AddButton(side, B_SCOPE_NEXT, ">", 0.82, 0.756, 0.14, 0.042);

		AddButton(side, B_MAP_HERE, "ПОСТАВИТЬ ЗДЕСЬ", 0.04, 0.805, 0.92, 0.042);
		m_MapHint = KPK_UI.MText(side, 0.04, 0.855, 0.92, 0.085, 12, KPK_UI.C_DIM);
		AddButton(side, B_MAP_SELF, "МОЯ ПОЗИЦИЯ", 0.04, 0.945, 0.92, 0.045);

		UpdateMarkerEditor();
		return page;
	}

	// the brick has no faction markers
	protected int ScopeCount()
	{
		if (m_Full)
			return 3;
		return 1;   // the brick: personal markers only (no party)
	}

	protected void UpdateMarkerEditor()
	{
		bool editing = m_EditId >= 0;
		m_MapHint.Show(!m_Theme.Glitch);
		m_MarkerType.SetText(KPK_Const.MarkerName(m_NewType));
		m_MarkerType.SetColor(KPK_Const.MarkerColor(m_NewType));
		int scope = m_NewScope;
		if (editing)
			scope = m_EditScope;
		m_MarkerScope.SetText("Доступ: " + KPK_Const.ScopeName(scope));
		m_ById.Get(B_SCOPE_PREV).SetEnabled(!editing);
		m_ById.Get(B_SCOPE_NEXT).SetEnabled(!editing);
		m_ById.Get(B_MAP_NEW).Show(editing);
		if (editing)
		{
			m_EditorTitle.SetText("ИЗМЕНИТЬ МЕТКУ");
			m_ById.Get(B_MAP_HERE).SetText("СОХРАНИТЬ");
			m_MapHint.SetText("Двойной клик по карте переносит метку в точку. «НОВАЯ» вернёт к созданию.");
		}
		else
		{
			m_EditorTitle.SetText("НОВАЯ МЕТКА");
			m_ById.Get(B_MAP_HERE).SetText("ПОСТАВИТЬ ЗДЕСЬ");
			m_MapHint.SetText("Двойной клик по карте ставит метку в точку. Метки видны только в КПК.");
		}
	}

	// load the selected marker into the editor
	protected void StartEdit(KPK_Marker mk)
	{
		m_EditId = mk.Id;
		m_EditScope = mk.Scope;
		m_NewType = mk.Type;
		m_MarkerName.SetText(mk.Name);
		UpdateMarkerEditor();
	}

	protected void StopEdit()
	{
		m_EditId = -1;
		m_MarkerName.SetText("");
		m_MarkerList.Select(-1);
		UpdateMarkerEditor();
	}

	protected void SaveEdit(bool move, float x, float z)
	{
		KPK_Req req = new KPK_Req();
		req.I1 = m_EditId;
		req.I2 = m_EditScope;
		req.S1 = MarkerNameText();
		req.S2 = m_NewType.ToString();
		req.F1 = x;
		req.F2 = z;
		if (move)
			req.I3 = 1;
		KPK_Client.Send(KPK_C.MARKER_EDIT, req);
	}

	protected void RefreshMap()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;

		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		m_MarkerRows.Clear();
		foreach (KPK_Marker mk : v.Markers)
		{
			m_MarkerRows.Insert(mk);
			string tag = "Л";
			if (mk.Scope == KPK_Scope.PARTY)
				tag = "Г";
			else if (mk.Scope == KPK_Scope.FACTION)
				tag = "Ф";
			names.Insert("[" + tag + "] " + mk.Name);
			colors.Insert(KPK_Const.MarkerColor(mk.Type));
		}
		m_MarkerList.Set(names, colors);

		// keep the edited marker selected; leave edit mode if it is gone
		if (m_EditId >= 0)
		{
			int found = -1;
			for (int ei = 0; ei < m_MarkerRows.Count(); ei++)
			{
				if (m_MarkerRows[ei].Id == m_EditId && m_MarkerRows[ei].Scope == m_EditScope)
					found = ei;
			}
			if (found >= 0)
			{
				m_MarkerList.Select(found);
			}
			else
			{
				m_EditId = -1;
				m_MarkerName.SetText("");
				UpdateMarkerEditor();
			}
		}
		RedrawMapMarks();
	}

	protected void RedrawMapMarks()
	{
		// the metro tab has its own map with the lines instead of the markers
		if (m_Page == P_METRO)
		{
			RedrawMetroMarks();
			return;
		}
		KPK_View v = KPK_Client.s_View;
		if (!v || !m_Map)
			return;

		m_Map.ClearUserMarks();

		foreach (KPK_Marker mk : v.Markers)
		{
			string label = mk.Name;
			if (mk.Scope == KPK_Scope.PARTY)
				label = mk.Name + " (группа)";
			else if (mk.Scope == KPK_Scope.FACTION)
				label = mk.Name + " (группировка)";
			m_Map.AddUserMark(Vector(mk.X, 0, mk.Z), label, KPK_Const.MarkerColor(mk.Type), KPK_Const.MarkerIcon(mk.Type));
		}

		// supposed faction bases the player may know about
		foreach (KPK_FactionView f : v.Factions)
		{
			if (f.HasLocationPos && !f.LocationEnc)
				m_Map.AddUserMark(Vector(f.LocX, 0, f.LocZ), f.Name + ": " + f.Location, f.Color, "\\DZ\\gear\\navigation\\data\\map_govoffice_ca.paa");
		}

		foreach (KPK_MatePos mate : KPK_Client.s_Mates.Mates)
			m_Map.AddUserMark(Vector(mate.X, 0, mate.Z), mate.Callsign, mate.Color, "\\DZ\\gear\\navigation\\data\\map_viewpoint_ca.paa");

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (player && v.ShowSelfOnMap)
		{
			vector pos = player.GetPosition();
			m_Map.AddUserMark(pos, "Я", KPK_UI.C_WHITE, "\\DZ\\gear\\navigation\\data\\map_viewpoint_ca.paa");
			m_MapInfo.SetText("Квадрат " + KPK_UI.Grid(pos[0], pos[2]) + "   ·   отметок: " + v.Markers.Count());
		}
		else
		{
			m_MapInfo.SetText("GPS: позиция скрыта   ·   отметок: " + v.Markers.Count());
		}
	}

	void OnMapDoubleClick(int x, int y)
	{
		if (!m_Map || m_Page != P_MAP)
			return;
		vector pos = m_Map.ScreenToMap(Vector(x, y, 0));
		if (m_EditId >= 0)
			SaveEdit(true, pos[0], pos[2]);
		else
			AddMarker(pos[0], pos[2]);
	}

	protected void AddMarker(float x, float z)
	{
		KPK_Req req = new KPK_Req();
		req.S1 = MarkerNameText();
		req.I1 = m_NewType;
		req.I2 = m_NewScope;
		req.F1 = x;
		req.F2 = z;
		KPK_Client.Send(KPK_C.MARKER_ADD, req);
		m_MarkerName.SetText("");
		SetStatus("Метка передана: квадрат " + KPK_UI.Grid(x, z));
	}

	// the name box wraps long names onto a second line: the line breaks are not part of the name
	protected string MarkerNameText()
	{
		string name = m_MarkerName.GetText();
		return name.Trim();
	}

	// Text boxes: the engine's own edit boxes neither wrap nor keep the text inside their border, and
	// the multi-line one hides its line breaks from scripts. So the input stays invisible and only takes
	// the keys; what was typed is drawn by a normal text over it: wrapped in the big boxes, the end of
	// the text in the one-line boxes (it scrolls along while typing). The cursor is drawn at the end.
	protected ref array<ref KPK_TextBox> m_TextBoxes = new array<ref KPK_TextBox>();
	protected float m_TextBoxTime;

	protected EditBoxWidget AddEdit(EditBoxWidget box)
	{
		if (box)
			m_TextBoxes.Insert(new KPK_TextBox(box, false));
		return box;
	}

	protected EditBoxWidget AddPassword(EditBoxWidget box)
	{
		if (box)
			m_TextBoxes.Insert(new KPK_TextBox(box, false, true));
		return box;
	}

	protected EditBoxWidget AddMulti(EditBoxWidget box)
	{
		if (box)
			m_TextBoxes.Insert(new KPK_TextBox(box, true));
		return box;
	}

	// characters of the 13 px text that fit into a box: ~7.7 px per character at 1080 lines (measured
	// on a screenshot), a little more for safety
	protected int BoxChars(Widget box)
	{
		if (!box)
			return 0;
		float bw, bh;
		box.GetScreenSize(bw, bh);
		if (bw <= 4)
			return 0;
		int sw, sh;
		GetScreenSize(sw, sh);
		if (sh <= 0)
			sh = 1080;
		float charW = 8.3 * sh / 1080.0;
		int chars = bw * 0.94 / charW;
		if (chars < 4)
			chars = 4;
		return chars;
	}

	protected void SyncTextBoxes()
	{
		Widget focus = GetFocus();
		bool blink = (GetGame().GetTime() / 500) % 2 == 0;
		foreach (KPK_TextBox tb : m_TextBoxes)
			tb.Sync(BoxChars(tb.m_Root), focus == tb.m_Input && blink);
	}

	protected KPK_Marker SelectedMarker()
	{
		int i = m_MarkerList.Selected();
		if (i < 0 || i >= m_MarkerRows.Count())
			return null;
		return m_MarkerRows[i];
	}


	// =================================================================== metro (admin «Куратор»)
	// The lines of the Drezina metro: where each line starts and ends and where it crosses the others.
	// The data comes from the Drezina mod (#ifdef DREZINA); without it the tab says so.

	protected Widget BuildMetro()
	{
		Widget page = Content();

		Widget mapBox = KPK_UI.Panel(page, 0, 0, 0.6, 1, 0xFF000000, true);
		m_MapHolder = KPK_UI.Frame(mapBox, 0.004, 0.004, 0.992, 0.992);
		KPK_UI.Panel(mapBox, 0, 0, 1, 1, KPK_UI.C_TINT);
		KPK_UI.Panel(mapBox, 0, 0, 1, 0.004, KPK_UI.C_EDGE);
		KPK_UI.Panel(mapBox, 0, 0.996, 1, 0.004, KPK_UI.C_EDGE);
		m_MapInfo = KPK_UI.Text(mapBox, "", 0.02, 0.94, 0.96, 0.05, 13, KPK_UI.C_ACCENT);

		Widget side = KPK_UI.Card(page, 0.615, 0, 0.385, 1, "ЛИНИИ МЕТРО");
		m_MetroLineList = new KPK_List(side, 0.04, 0.08, 0.92, 0.3, 5, "— метро не построено —");
		m_Lists.Insert(m_MetroLineList);
		m_MetroInfo = KPK_UI.MText(side, 0.04, 0.385, 0.92, 0.075, 12, KPK_UI.C_DIM);

		KPK_UI.Panel(side, 0.04, 0.465, 0.92, 0.002, KPK_UI.C_LINE);
		KPK_UI.Text(side, "ТОЧКИ ЛИНИИ", 0.04, 0.47, 0.92, 0.04, 14, KPK_UI.C_ACCENT);
		m_MetroPointList = new KPK_List(side, 0.04, 0.515, 0.92, 0.36, 6, "— выберите линию —");
		m_MetroPointList.SetWrap(true);
		m_Lists.Insert(m_MetroPointList);

		AddButton(side, B_M_LINE, "ВСЯ ЛИНИЯ", 0.04, 0.89, 0.45, 0.045);
		AddButton(side, B_M_ALL, "ВСЁ МЕТРО", 0.51, 0.89, 0.45, 0.045);
		AddButton(side, B_M_RELOAD, "ОБНОВИТЬ", 0.04, 0.943, 0.45, 0.045);
		AddButton(side, B_M_DEL, "УДАЛИТЬ ЛИНИЮ", 0.51, 0.943, 0.45, 0.045);
		m_MetroDelArm = 0;

		LoadMetro();
		if (m_MetroSel >= m_MetroLines.Count())
			m_MetroSel = -1;
		if (m_MetroSel >= 0)
			m_MetroLineList.Select(m_MetroSel);
		return page;
	}

	protected void LoadMetro()
	{
		m_MetroLines.Clear();
		m_MetroCross.Clear();
#ifdef DREZINA
		m_MetroVer = DrezinaMetroMap.s_Version;
		array<ref DrezinaMetroLine> lines = new array<ref DrezinaMetroLine>();
		array<ref DrezinaMetroCross> cross = new array<ref DrezinaMetroCross>();
		DrezinaMetroMap.CollectFull(lines, cross, 40.0);
		foreach (DrezinaMetroLine dl : lines)
		{
			KPK_MetroLine l = new KPK_MetroLine();
			l.LineId = dl.Id;
			l.InZone = dl.Zone;
			l.LenM = dl.LenM;
			l.EndA = dl.A;
			l.EndB = dl.B;
			l.JoinToA = dl.JoinA;
			l.JoinToB = dl.JoinB;
			foreach (vector dp : dl.Axis)
				l.Axis.Insert(dp);
			// by number
			int at = m_MetroLines.Count();
			for (int k = 0; k < m_MetroLines.Count(); k++)
			{
				if (m_MetroLines[k].LineId > l.LineId)
				{
					at = k;
					break;
				}
			}
			m_MetroLines.InsertAt(l, at);
		}
		foreach (DrezinaMetroCross dc : cross)
		{
			KPK_MetroCross c = new KPK_MetroCross();
			c.LineA = dc.A;
			c.LineB = dc.B;
			c.Spot = dc.Spot;
			c.IsTee = dc.Tee;
			c.IsRing = dc.Ring;
			m_MetroCross.Insert(c);
		}
#endif
	}

	// a colour of its own for every line
	protected int MetroColor(int index)
	{
		array<int> pal = {0xFFE04A3A, 0xFF3FA7E0, 0xFF58C24A, 0xFFE8B23A, 0xFFB063D8, 0xFF3AD0C0, 0xFFE87AB0, 0xFFC8C8C8};
		return pal[index % pal.Count()];
	}

	protected string CrossKind(KPK_MetroCross c)
	{
		if (c.IsTee)
			return "Т-развязка";
		if (c.IsRing)
			return "кольцо";
		return "два съезда";
	}

	protected string At(vector p)
	{
		return "кв. " + KPK_UI.Grid(p[0], p[2]);
	}

	// how far along the line a point lies (index of the nearest axis point), to sort the crossings
	protected int AlongLine(KPK_MetroLine l, vector p)
	{
		int best = 0;
		float bestD = -1;
		for (int i = 0; i < l.Axis.Count(); i++)
		{
			vector q = l.Axis[i];
			float d = (q[0] - p[0]) * (q[0] - p[0]) + (q[2] - p[2]) * (q[2] - p[2]);
			if (bestD < 0 || d < bestD)
			{
				bestD = d;
				best = i;
			}
		}
		return best;
	}

	protected void RefreshMetro()
	{
		if (!m_MetroLineList || !m_MetroPointList)
			return;
		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		for (int i = 0; i < m_MetroLines.Count(); i++)
		{
			KPK_MetroLine l = m_MetroLines[i];
			int nx = 0;
			foreach (KPK_MetroCross c : m_MetroCross)
			{
				if (c.LineA == l.LineId || c.LineB == l.LineId)
					nx++;
			}
			names.Insert("M" + l.LineId + "   " + KmText(l.LenM) + "   пересеч.: " + nx);
			colors.Insert(MetroColor(i));
		}
		m_MetroLineList.Set(names, colors);

		m_MetroPointPos.Clear();
		m_MetroPointName.Clear();
		array<string> rows = new array<string>();
		array<int> rowColors = new array<int>();
		if (m_MetroSel >= 0 && m_MetroSel < m_MetroLines.Count())
		{
			KPK_MetroLine sl = m_MetroLines[m_MetroSel];
			// start
			string startName = "НАЧАЛО: вход A";
			if (sl.JoinToA > 0)
				startName = "НАЧАЛО: примыкает к M" + sl.JoinToA + " (Т-развязка)";
			AddMetroPoint(rows, rowColors, sl.EndA, startName, KPK_UI.C_WHITE);

			// crossings in order along the line
			array<KPK_MetroCross> mine = new array<KPK_MetroCross>();
			array<int> order = new array<int>();
			foreach (KPK_MetroCross mc : m_MetroCross)
			{
				if (mc.LineA != sl.LineId && mc.LineB != sl.LineId)
					continue;
				if (mc.IsTee && mc.LineB == sl.LineId)
					continue;   // its own joined end: already the start / end row
				int along = AlongLine(sl, mc.Spot);
				int at = order.Count();
				for (int k = 0; k < order.Count(); k++)
				{
					if (order[k] > along)
					{
						at = k;
						break;
					}
				}
				order.InsertAt(along, at);
				mine.InsertAt(mc, at);
			}
			foreach (KPK_MetroCross xc : mine)
			{
				int other = xc.LineB;
				if (other == sl.LineId)
					other = xc.LineA;
				string xname = "ПЕРЕСЕЧЕНИЕ с M" + other + " (" + CrossKind(xc) + ")";
				if (xc.IsTee)
					xname = "ПРИМЫКАЕТ M" + other + " (Т-развязка)";
				AddMetroPoint(rows, rowColors, xc.Spot, xname, KPK_UI.C_ACCENT);
			}

			string endName = "КОНЕЦ: вход B";
			if (sl.JoinToB > 0)
				endName = "КОНЕЦ: примыкает к M" + sl.JoinToB + " (Т-развязка)";
			AddMetroPoint(rows, rowColors, sl.EndB, endName, KPK_UI.C_WHITE);

			string info = "M" + sl.LineId + ": " + KmText(sl.LenM) + ",  " + At(sl.EndA) + "  →  " + At(sl.EndB);
			if (sl.InZone)
				info += "\nтоннель в локации за картой (входы — здесь)";
			m_MetroInfo.SetText(info);
		}
		else
		{
			string none = "Выберите линию: список её точек и место на карте.";
#ifndef DREZINA
			none = "Мод Drezina не установлен: метро нет.";
#endif
			m_MetroInfo.SetText(none);
		}
		int keep = m_MetroPointList.Selected();
		m_MetroPointList.Set(rows, rowColors);
		if (keep >= 0 && keep < rows.Count())
			m_MetroPointList.Select(keep);
		RedrawMetroMarks();
	}

	// «УДАЛИТЬ ЛИНИЮ»: the first press asks, the second sends /metro del <id> to Drezina
	protected void MetroDelete()
	{
		if (m_MetroSel < 0 || m_MetroSel >= m_MetroLines.Count())
		{
			SetStatus("Сначала выберите линию в списке.");
			return;
		}
		int lineId = m_MetroLines[m_MetroSel].LineId;
		KPK_Button del = m_ById.Get(B_M_DEL);
		if (m_MetroDelArm != lineId)
		{
			m_MetroDelArm = lineId;
			if (del)
				del.SetText("ТОЧНО M" + lineId + "?");
			SetStatus("Удалить M" + lineId + " вместе с путём, входами и развязками? Нажмите ещё раз.");
			return;
		}
		DisarmMetroDelete();
#ifdef DREZINA
		DrezinaMetroMap.RequestDelete(lineId);
		m_MetroDelWait = lineId;
		m_MetroDelTime = 0;
		SetStatus("M" + lineId + ": запрос на удаление отправлен...");
#else
		SetStatus("Мод Drezina не установлен.");
#endif
	}

	protected void DisarmMetroDelete()
	{
		m_MetroDelArm = 0;
		KPK_Button del = m_ById.Get(B_M_DEL);
		if (del)
			del.SetText("УДАЛИТЬ ЛИНИЮ");
	}

	// new metro data from the server (a line built, deleted, rebuilt): read the lists again
	protected void MetroTick(float timeslice)
	{
		int ver = m_MetroVer;
#ifdef DREZINA
		ver = DrezinaMetroMap.s_Version;
#endif
		if (ver != m_MetroVer)
		{
			int keepId = -1;
			if (m_MetroSel >= 0 && m_MetroSel < m_MetroLines.Count())
				keepId = m_MetroLines[m_MetroSel].LineId;
			LoadMetro();
			m_MetroSel = -1;
			for (int i = 0; i < m_MetroLines.Count(); i++)
			{
				if (m_MetroLines[i].LineId == keepId)
					m_MetroSel = i;
			}
			if (m_MetroSel < 0)
				m_MetroHasFocus = false;
			if (m_MetroLineList)
				m_MetroLineList.Select(m_MetroSel);
			if (m_MetroPointList)
				m_MetroPointList.Select(-1);
			if (m_MetroDelWait > 0)
			{
				bool gone = true;
				foreach (KPK_MetroLine l : m_MetroLines)
				{
					if (l.LineId == m_MetroDelWait)
						gone = false;
				}
				if (gone)
					SetStatus("M" + m_MetroDelWait + " удалено.");
				m_MetroDelWait = 0;
			}
			RefreshMetro();
			return;
		}
		if (m_MetroDelWait > 0)
		{
			m_MetroDelTime += timeslice;
			if (m_MetroDelTime > 15)
			{
				SetStatus("M" + m_MetroDelWait + " не удалено: сервер не ответил. Нужны права админа Drezina (admins.json или суперадмин VPP).");
				m_MetroDelWait = 0;
			}
		}
	}

	protected void AddMetroPoint(array<string> rows, array<int> rowColors, vector p, string name, int color)
	{
		rows.Insert(name + "  ·  " + At(p));
		rowColors.Insert(color);
		m_MetroPointPos.Insert(p);
		m_MetroPointName.Insert(name);
	}

	protected string KmText(float m)
	{
		if (m < 1000)
			return Math.Round(m).ToString() + " м";
		float km = Math.Round(m / 100) / 10;
		return km.ToString() + " км";
	}

	protected void FitMetroLine()
	{
		if (m_MetroSel < 0 || m_MetroSel >= m_MetroLines.Count())
		{
			FitMetroAll();
			return;
		}
		FitMetro(m_MetroLines[m_MetroSel].Axis);
	}

	protected void FitMetroAll()
	{
		array<vector> all = new array<vector>();
		foreach (KPK_MetroLine l : m_MetroLines)
		{
			foreach (vector p : l.Axis)
				all.Insert(p);
		}
		FitMetro(all);
	}

	// centre the map on the points and zoom so that they fit
	protected void FitMetro(array<vector> pts)
	{
		if (!m_Map || pts.Count() == 0)
			return;
		float x0 = pts[0][0];
		float x1 = x0;
		float z0 = pts[0][2];
		float z1 = z0;
		foreach (vector p : pts)
		{
			x0 = Math.Min(x0, p[0]);
			x1 = Math.Max(x1, p[0]);
			z0 = Math.Min(z0, p[2]);
			z1 = Math.Max(z1, p[2]);
		}
		float size = Math.Max(x1 - x0, z1 - z0);
		// map scale ~ visible part of a 15 km map
		m_Map.SetScale(Math.Clamp(size * 1.6 / 15360.0, 0.05, 1.0));
		m_Map.SetMapPos(Vector((x0 + x1) * 0.5, 0, (z0 + z1) * 0.5));
	}

	protected void RedrawMetroMarks()
	{
		if (!m_Map || m_Page != P_METRO)
			return;
		m_Map.ClearUserMarks();
		string dot = "\\KPK\\data\\kpk_metro_dot_ca.paa";
		string gate = "\\KPK\\data\\kpk_metro_gate_ca.paa";
		string cross = "\\KPK\\data\\kpk_metro_cross_ca.paa";

		for (int i = 0; i < m_MetroLines.Count(); i++)
		{
			KPK_MetroLine l = m_MetroLines[i];
			int color = MetroColor(i);
			// other lines dimmer while one is picked
			if (m_MetroSel >= 0 && m_MetroSel != i)
				color = (color & 0x00FFFFFF) | 0x80000000;
			// the axis as a dotted line (the end points get their own marks)
			for (int k = 1; k < l.Axis.Count() - 1; k++)
				m_Map.AddUserMark(l.Axis[k], "", color, dot);
			if (l.JoinToA <= 0)
				m_Map.AddUserMark(l.EndA, "M" + l.LineId + " начало", color, gate);
			if (l.JoinToB <= 0)
				m_Map.AddUserMark(l.EndB, "M" + l.LineId + " конец", color, gate);
		}
		foreach (KPK_MetroCross c : m_MetroCross)
			m_Map.AddUserMark(c.Spot, "M" + c.LineA + " × M" + c.LineB, KPK_UI.C_WHITE, cross);

		if (m_MetroHasFocus)
			m_Map.AddUserMark(m_MetroFocus, "► " + m_MetroFocusName, KPK_UI.C_ACCENT, "\\DZ\\gear\\navigation\\data\\map_viewpoint_ca.paa");

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		string info = "Линий: " + m_MetroLines.Count() + "   ·   пересечений: " + m_MetroCross.Count();
		if (player)
		{
			vector pos = player.GetPosition();
			m_Map.AddUserMark(pos, "Я", KPK_UI.C_WHITE, "\\DZ\\gear\\navigation\\data\\map_viewpoint_ca.paa");
			info += "   ·   я: " + At(pos);
		}
		if (m_MapInfo)
			m_MapInfo.SetText(info);
	}

	// =================================================================== party

	// the brick: no party, only the contacts (quick calls) and the radio
	protected Widget BuildBrickComms()
	{
		Widget page = Content();
		Widget left = KPK_UI.Card(page, 0, 0, 0.49, 1, "КОНТАКТЫ");
		m_BrickContacts = new KPK_List(left, 0.04, 0.08, 0.92, 0.72, 9, "— контактов нет —");
		m_Lists.Insert(m_BrickContacts);
		AddButton(left, B_R_DIAL, "ВЫЗВАТЬ", 0.04, 0.84, 0.45, 0.07);
		AddButton(left, B_R_DROP, "УДАЛИТЬ", 0.51, 0.84, 0.45, 0.07);
		MultilineTextWidget ch = KPK_UI.MText(left, 0.04, 0.925, 0.92, 0.07, 11, KPK_UI.C_DIM);
		ch.SetText("Контакты добавляются на полном КПК — аккаунт общий.");

		Widget radio = KPK_UI.Card(page, 0.51, 0, 0.49, 1, "РАЦИЯ");
		m_RadioText = KPK_UI.MText(radio, 0.04, 0.08, 0.92, 0.3, 14, KPK_UI.C_TEXT);
		KPK_UI.Text(radio, "Вызов по позывному", 0.04, 0.42, 0.92, 0.05, 13, KPK_UI.C_DIM);
		m_RadioName = AddEdit(KPK_UI.Edit(radio, 0.04, 0.48, 0.6, 0.07));
		AddButton(radio, B_R_CALL, "ВЫЗОВ", 0.66, 0.48, 0.3, 0.07);
		AddButton(radio, B_R_ANSWER, "ОТВЕТИТЬ", 0.04, 0.62, 0.45, 0.08);
		AddButton(radio, B_R_END, "ОТБОЙ", 0.51, 0.62, 0.45, 0.08);
		MultilineTextWidget rh = KPK_UI.MText(radio, 0.04, 0.76, 0.92, 0.22, 12, KPK_UI.C_DIM);
		rh.SetText("Выберите контакт слева и нажмите «ВЫЗВАТЬ». Во время разговора так же добавляется ещё один участник.");
		rh.Show(!m_Theme.Glitch);
		return page;
	}

	protected Widget BuildParty()
	{
		if (!m_Full)
			return BuildBrickComms();
		Widget page = Content();

		Widget left = KPK_UI.Card(page, 0, 0, 0.49, 1, "ГРУППА");
		m_PartyInfo = KPK_UI.Text(left, "", 0.04, 0.07, 0.92, 0.05, 14, KPK_UI.C_DIM);
		m_PartyList = new KPK_List(left, 0.04, 0.13, 0.92, 0.55, 8, "— вы не в группе —");
		m_Lists.Insert(m_PartyList);
		AddButton(left, B_P_KICK, "ИСКЛЮЧИТЬ", 0.04, 0.71, 0.45, 0.06);
		AddButton(left, B_P_LEAVE, "ПОКИНУТЬ ГРУППУ", 0.51, 0.71, 0.45, 0.06);

		KPK_UI.Panel(left, 0.04, 0.8, 0.92, 0.002, KPK_UI.C_LINE);
		KPK_UI.Text(left, "Пригласить по позывному", 0.04, 0.81, 0.92, 0.05, 13, KPK_UI.C_DIM);
		m_PartyInviteName = AddEdit(KPK_UI.Edit(left, 0.04, 0.865, 0.6, 0.06));
		AddButton(left, B_P_INVITE, "ПРИГЛАСИТЬ", 0.66, 0.865, 0.3, 0.06);

		Widget right = KPK_UI.Card(page, 0.51, 0, 0.49, 0.62, "ВХОДЯЩИЕ ПРИГЛАШЕНИЯ");
		m_InviteList = new KPK_List(right, 0.04, 0.1, 0.92, 0.66, 6, "— приглашений нет —");
		m_Lists.Insert(m_InviteList);
		AddButton(right, B_INV_ACCEPT, "ПРИНЯТЬ", 0.04, 0.82, 0.45, 0.1);
		AddButton(right, B_INV_DECLINE, "ОТКЛОНИТЬ", 0.51, 0.82, 0.45, 0.1);

		Widget opts = KPK_UI.Card(page, 0.51, 0.65, 0.49, 0.35, "ДАТЧИКИ");
		m_LabelsButton = AddButton(opts, B_LABELS, "", 0.04, 0.2, 0.92, 0.18);
		m_PartyHint = KPK_UI.MText(opts, 0.04, 0.45, 0.92, 0.5, 13, KPK_UI.C_DIM);
		m_PartyHint.SetText("Метки над головами показывают позывные и дистанцию до членов группы (и группировки, если она это разрешает). Горячая клавиша: Ctrl + K.");
		m_PartyHint.Show(!m_Theme.Glitch);
		return page;
	}

	protected void RefreshParty()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;
		if (!m_Full)
		{
			RefreshBrickComms(v);
			return;
		}

		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		foreach (KPK_PartyMemberView m : v.Party)
		{
			string line = m.Callsign;
			if (m.Leader)
				line = line + "   · лидер";
			if (!m.Online)
				line += "   · не в сети";
			else if (m.Distance >= 0 && m.Callsign != v.Callsign)
				line += string.Format("   · %1 м", m.Distance);
			names.Insert(line);
			if (m.Online)
				colors.Insert(KPK_UI.C_TEXT);
			else
				colors.Insert(KPK_UI.C_DIM);
		}
		m_PartyList.Set(names, colors);
		if (!m_PartyInfo)
			return;

		if (v.InParty)
			m_PartyInfo.SetText(string.Format("Состав: %1 / %2", v.Party.Count(), v.PartyMax));
		else
			m_PartyInfo.SetText("Пригласите сталкера, чтобы создать группу");

		m_ById.Get(B_P_KICK).SetEnabled(v.PartyLeader);
		m_ById.Get(B_P_LEAVE).SetEnabled(v.InParty);

		array<string> inv = new array<string>();
		array<int> invColors = new array<int>();
		m_InviteRows.Clear();
		foreach (KPK_InviteView iv : v.Invites)
		{
			m_InviteRows.Insert(iv);
			if (iv.Kind == 1)
			{
				inv.Insert("Группа: " + iv.From);
				invColors.Insert(KPK_UI.C_TEXT);
			}
			else if (iv.Kind == 3)
			{
				inv.Insert("Контакт: " + iv.From);
				invColors.Insert(KPK_UI.C_WHITE);
			}
			else
			{
				inv.Insert("Группировка «" + iv.FactionName + "» (" + iv.From + ")");
				invColors.Insert(KPK_UI.C_ACCENT);
			}
		}
		m_InviteList.Set(inv, invColors);

		if (m_RadioText)
			RefreshRadio(v);
		if (!m_LabelsButton)
			return;
		if (KPK_Client.s_LabelsEnabled)
			m_LabelsButton.SetText("МЕТКИ В МИРЕ: ВКЛ");
		else
			m_LabelsButton.SetText("МЕТКИ В МИРЕ: ВЫКЛ");
		m_LabelsButton.SetActive(KPK_Client.s_LabelsEnabled);
	}

	protected void RefreshBrickComms(KPK_View v)
	{
		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		foreach (KPK_ContactView c : v.Contacts)
		{
			string line = c.Callsign;
			if (c.Online)
			{
				line += "   · в сети";
				colors.Insert(KPK_UI.C_TEXT);
			}
			else
			{
				line += "   · нет сигнала";
				colors.Insert(KPK_UI.C_DIM);
			}
			names.Insert(line);
		}
		m_BrickContacts.Set(names, colors);
		if (m_RadioText)
			RefreshRadio(v);
	}

	protected string BrickContactName()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v || !m_BrickContacts)
			return "";
		int i = m_BrickContacts.Selected();
		if (i < 0 || i >= v.Contacts.Count())
			return "";
		return v.Contacts[i].Callsign;
	}

	protected void RefreshRadio(KPK_View v)
	{
		string text = "Вызов другого «Вестника» по позывному.";
		switch (v.CallState)
		{
			case KPK_CallState.OUTGOING: text = "Вызов: " + v.CallWith + " . . . ждём ответа"; break;
			case KPK_CallState.INCOMING: text = "ВХОДЯЩИЙ ВЫЗОВ: " + v.CallWith; break;
			case KPK_CallState.ACTIVE: text = "НА СВЯЗИ: " + v.CallWith + "\nговорите в голосовой чат"; break;
		}
		m_RadioText.SetText(text);
		// in a call the same key adds one more participant
		KPK_Button callKey = m_ById.Get(B_R_CALL);
		if (v.CallState == KPK_CallState.NONE)
			callKey.SetText("ВЫЗОВ");
		else
			callKey.SetText("+ В ЗВОНОК");
		callKey.SetEnabled(v.CallState != KPK_CallState.INCOMING);
		m_ById.Get(B_R_ANSWER).SetEnabled(v.CallState == KPK_CallState.INCOMING);
		m_ById.Get(B_R_END).SetEnabled(v.CallState != KPK_CallState.NONE);
	}

	protected void RespondInvite(bool accept)
	{
		int i = m_InviteList.Selected();
		if (i < 0 || i >= m_InviteRows.Count())
		{
			SetStatus("Выберите приглашение");
			return;
		}
		KPK_InviteView iv = m_InviteRows[i];
		int answer = 0;
		if (accept)
			answer = 1;
		if (iv.Kind == 1)
			KPK_Client.SendText(KPK_C.PARTY_RESPOND, iv.From, "", answer);
		else if (iv.Kind == 3)
			KPK_Client.SendText(KPK_C.CONTACT_RESPOND, iv.From, "", answer);
		else
			KPK_Client.SendText(KPK_C.FACTION_RESPOND, iv.FactionId, "", answer);
	}

	// =================================================================== factions

	protected Widget BuildFactions()
	{
		m_ConfirmRemove = "";
		Widget page = Content();

		Widget left = KPK_UI.Card(page, 0, 0, 0.27, 1, "ГРУППИРОВКИ ЗОНЫ");
		m_FactionList = new KPK_List(left, 0.05, 0.06, 0.9, 0.6, 8, "— нет данных —");
		m_Lists.Insert(m_FactionList);
		m_FactionHint = KPK_UI.Text(left, "", 0.05, 0.68, 0.9, 0.04, 12, KPK_UI.C_DIM);

		Widget info = KPK_UI.Card(page, 0.285, 0, 0.415, 0.58, "ДОСЬЕ");
		m_FactionText = KPK_UI.MText(info, 0.04, 0.08, 0.92, 0.9, 14, KPK_UI.C_TEXT);

		Widget members = KPK_UI.Card(page, 0.715, 0, 0.285, 0.58, "СОСТАВ");
		m_MemberList = new KPK_List(members, 0.05, 0.08, 0.9, 0.9, 8, "— данные недоступны —");
		m_Lists.Insert(m_MemberList);

		// management: leader / deputy of the own faction, or the admin (any faction)
		m_FactionManage = KPK_UI.Card(page, 0.285, 0.6, 0.715, 0.17, "УПРАВЛЕНИЕ СОСТАВОМ", 0.22);
		m_ManageHint = KPK_UI.Text(m_FactionManage, "Выбранный боец:", 0.02, 0.27, 0.28, 0.28, 12, KPK_UI.C_DIM);
		AddButton(m_FactionManage, B_F_KICK, "ИСКЛЮЧИТЬ", 0.31, 0.26, 0.14, 0.3);
		// ranks: only the admin («Куратор») appoints leaders and deputies
		AddButton(m_FactionManage, B_F_MEMBER, "В БОЙЦЫ", 0.46, 0.26, 0.12, 0.3);
		AddButton(m_FactionManage, B_F_DEPUTY, "В ЗАМЫ", 0.59, 0.26, 0.12, 0.3);
		AddButton(m_FactionManage, B_F_LEADER, "В ЛИДЕРЫ", 0.72, 0.26, 0.12, 0.3);
		AddButton(m_FactionManage, B_F_MAP, "НА КАРТЕ", 0.85, 0.26, 0.13, 0.3);
		m_FactionInviteName = AddEdit(KPK_UI.Edit(m_FactionManage, 0.02, 0.62, 0.42, 0.3));
		AddButton(m_FactionManage, B_F_INVITE, "ПРИНЯТЬ В РЯДЫ", 0.46, 0.62, 0.25, 0.3);
		AddButton(m_FactionManage, B_F_LEAVE, "ПОКИНУТЬ", 0.74, 0.62, 0.24, 0.3);

		// admin: settings of the selected faction
		m_AdminBox = KPK_UI.Card(page, 0, 0.79, 1, 0.21, "АДМИНИСТРАТОР · ВЫБРАННАЯ ГРУППИРОВКА", 0.19);
		float bw = 0.118;
		float gap = 0.006;
		AddButton(m_AdminBox, B_A_ENC_DESC, "", 0.01, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_ENC_LEADER, "", 0.01 + (bw + gap), 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_ENC_MEMBERS, "", 0.01 + (bw + gap) * 2, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_ENC_LOC, "", 0.01 + (bw + gap) * 3, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_HIDDEN, "", 0.01 + (bw + gap) * 4, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_MATES, "", 0.01 + (bw + gap) * 5, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_COLOR, "ЦВЕТ >", 0.01 + (bw + gap) * 6, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_REMOVE, "УДАЛИТЬ", 0.01 + (bw + gap) * 7, 0.2, bw, 0.22);

		// one text field: place, description or the name of a new faction
		KPK_UI.Text(m_AdminBox, "Текст:", 0.01, 0.48, 0.05, 0.22, 12, KPK_UI.C_DIM);
		m_AdminText = AddEdit(KPK_UI.Edit(m_AdminBox, 0.06, 0.48, 0.432, 0.22));
		AddButton(m_AdminBox, B_A_LOC_TEXT, "ТЕКСТ -> МЕСТО", 0.5, 0.48, 0.16, 0.22);
		AddButton(m_AdminBox, B_A_DESC, "ТЕКСТ -> ОПИСАНИЕ", 0.665, 0.48, 0.16, 0.22);
		AddButton(m_AdminBox, B_A_CREATE, "НОВАЯ ГРУППИРОВКА", 0.83, 0.48, 0.16, 0.22);

		KPK_UI.Text(m_AdminBox, "Точка на карте:", 0.01, 0.75, 0.12, 0.22, 12, KPK_UI.C_DIM);
		AddButton(m_AdminBox, B_A_LOC_ME, "ГДЕ Я СТОЮ", 0.13, 0.75, 0.15, 0.22);
		AddButton(m_AdminBox, B_A_LOC_CLEAR, "УБРАТЬ С КАРТЫ", 0.29, 0.75, 0.15, 0.22);
		MultilineTextWidget ah = KPK_UI.MText(m_AdminBox, 0.46, 0.72, 0.53, 0.28, 11, KPK_UI.C_DIM);
		ah.SetText("Зачислить игрока — поле «УПРАВЛЕНИЕ СОСТАВОМ» (позывной или Steam64 ID). Звания — выберите бойца в «Составе».");
		return page;
	}

	protected KPK_FactionView MyFaction()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v || v.MyFactionId == "")
			return null;
		foreach (KPK_FactionView f : v.Factions)
		{
			if (f.Id == v.MyFactionId)
				return f;
		}
		return null;
	}

	protected KPK_FactionView SelectedFaction()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v || !m_FactionList)
			return null;
		int i = m_FactionList.Selected();
		if (i < 0 || i >= v.Factions.Count())
			return null;
		return v.Factions[i];
	}

	protected string SelectedMemberName()
	{
		KPK_FactionView f = SelectedFaction();
		if (!f)
			return "";
		int i = m_MemberList.Selected();
		if (i < 0 || i >= f.Members.Count())
			return "";
		return f.Members[i].Callsign;
	}

	protected string EncField(string value, bool enc)
	{
		if (enc)
			return value;
		return value;
	}

	protected void RefreshFactions()
	{
		if (!m_FactionList)
			return;
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;

		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		foreach (KPK_FactionView f : v.Factions)
		{
			string line = f.Name;
			if (f.IsMine)
				line = "> " + line;
			if (f.Hidden)
				line += "  (скрыта)";
			names.Insert(line);
			colors.Insert(f.Color);
		}
		m_FactionList.Set(names, colors);
		if (m_FactionList.Selected() < 0 && v.Factions.Count() > 0)
		{
			int mineIndex = 0;
			for (int i = 0; i < v.Factions.Count(); i++)
			{
				if (v.Factions[i].IsMine)
					mineIndex = i;
			}
			m_FactionList.Select(mineIndex);
		}

		if (v.MyFactionId != "")
			m_FactionHint.SetText("> ваша группировка");
		else
			m_FactionHint.SetText("Вы не состоите в группировке");

		RefreshFactionDetails();
	}

	protected void RefreshFactionDetails()
	{
		if (!m_FactionText)
			return;
		KPK_View v = KPK_Client.s_View;
		KPK_FactionView f = SelectedFaction();
		if (!v || !f)
		{
			m_FactionText.SetText("Выберите группировку из списка.");
			m_MemberList.Set(new array<string>());
			m_FactionManage.Show(false);
			m_AdminBox.Show(v && v.IsAdmin);
			return;
		}

		string deputies = "";
		foreach (KPK_FactionMemberView dm : f.Members)
		{
			if (dm.Role != KPK_Role.DEPUTY)
				continue;
			if (deputies != "")
				deputies += ", ";
			deputies += dm.Callsign;
		}
		if (f.MembersEnc)
			deputies = KPK_Util.Scramble(f.Id + "dep", 10);
		else if (deputies == "")
			deputies = "—";

		string count = KPK_Util.Scramble(f.Id + "cnt", 3);
		if (!f.MembersEnc)
			count = f.MemberCount.ToString();

		string text = "";
		text += f.Name + "\n\n";
		text += "Лидер:  " + EncField(f.Leader, f.LeaderEnc) + "\n";
		text += "Заместители:  " + deputies + "\n";
		text += "Численность:  " + count + "\n";
		text += "Предположительное местоположение:\n   " + EncField(f.Location, f.LocationEnc) + "\n\n";
		text += KPK_Util.FromWire(EncField(f.Description, f.DescriptionEnc)) + "\n";
		if (v.IsAdmin)
		{
			text += "\n[админ] шифр: ";
			if (f.FlagDescription) text += "описание ";
			if (f.FlagLeader) text += "лидер ";
			if (f.FlagMembers) text += "состав ";
			if (f.FlagLocation) text += "место ";
			if (!f.FlagDescription && !f.FlagLeader && !f.FlagMembers && !f.FlagLocation) text += "нет";
			if (f.Hidden) text += " | скрыта";
			if (f.ShowMatesInWorld) text += " | видны в мире";
		}
		m_FactionText.SetText(text);

		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		foreach (KPK_FactionMemberView m : f.Members)
		{
			string line = m.Callsign + "  · " + KPK_Const.RoleName(m.Role);
			if (!m.Online)
				line += "  · офлайн";
			names.Insert(line);
			if (m.Role == KPK_Role.LEADER)
				colors.Insert(KPK_UI.C_ACCENT);
			else if (m.Online)
				colors.Insert(KPK_UI.C_TEXT);
			else
				colors.Insert(KPK_UI.C_DIM);
		}
		m_MemberList.m_Empty = "— данные недоступны —";
		if (!f.MembersEnc)
			m_MemberList.m_Empty = "— никого —";
		m_MemberList.Set(names, colors);

		bool mine = f.IsMine;
		bool canManage = v.IsAdmin || (mine && v.MyRole >= KPK_Role.DEPUTY);
		m_FactionManage.Show(mine || v.IsAdmin || f.HasLocationPos);
		m_ManageHint.Show(canManage);
		m_ById.Get(B_F_KICK).Show(canManage);
		// ranks are given by the admin only: on the ordinary KPKs these buttons are gone
		m_ById.Get(B_F_MEMBER).Show(v.IsAdmin);
		m_ById.Get(B_F_DEPUTY).Show(v.IsAdmin);
		m_ById.Get(B_F_LEADER).Show(v.IsAdmin);
		m_FactionInviteName.GetParent().Show(canManage);
		m_ById.Get(B_F_INVITE).Show(canManage);
		// the admin enrols at once (callsign or Steam64 ID), the leader / deputy sends an invitation
		if (v.IsAdmin)
			m_ById.Get(B_F_INVITE).SetText("ЗАЧИСЛИТЬ");
		else
			m_ById.Get(B_F_INVITE).SetText("ПРИНЯТЬ В РЯДЫ");
		m_ById.Get(B_F_LEAVE).Show(mine);
		m_ById.Get(B_F_MAP).SetEnabled(f.HasLocationPos && !f.LocationEnc);

		m_AdminBox.Show(v.IsAdmin);
		if (v.IsAdmin)
		{
			AdminToggle(B_A_ENC_DESC, "ОПИС.", f.FlagDescription, "ШИФР", "ОТКР");
			AdminToggle(B_A_ENC_LEADER, "ЛИДЕР", f.FlagLeader, "ШИФР", "ОТКР");
			AdminToggle(B_A_ENC_MEMBERS, "СОСТАВ", f.FlagMembers, "ШИФР", "ОТКР");
			AdminToggle(B_A_ENC_LOC, "МЕСТО", f.FlagLocation, "ШИФР", "ОТКР");
			AdminToggle(B_A_HIDDEN, "ВИДНА", !f.Hidden, "ДА", "НЕТ");
			AdminToggle(B_A_MATES, "МЕТКИ", f.ShowMatesInWorld, "ВКЛ", "ВЫКЛ");
			m_ById.Get(B_A_COLOR).SetTextColor(f.Color);
			if (m_ConfirmRemove == f.Id)
				m_ById.Get(B_A_REMOVE).SetText("ТОЧНО УДАЛИТЬ?");
			else
				m_ById.Get(B_A_REMOVE).SetText("УДАЛИТЬ");
		}
	}

	// a toggle of the admin box: "СОСТАВ: ШИФР", lit while on
	protected void AdminToggle(int id, string name, bool on, string onText, string offText)
	{
		KPK_Button b = m_ById.Get(id);
		if (!b)
			return;
		if (on)
			b.SetText(name + ": " + onText);
		else
			b.SetText(name + ": " + offText);
		b.SetActive(on);
	}

	protected void Admin(int op, string text = "", int arg = 0)
	{
		KPK_FactionView f = SelectedFaction();
		KPK_Req req = new KPK_Req();
		req.I1 = op;
		req.I2 = arg;
		req.S2 = text;
		if (f)
			req.S1 = f.Id;
		if (op != KPK_AdminOp.CREATE && !f)
		{
			SetStatus("Выберите группировку");
			return;
		}
		KPK_Client.Send(KPK_C.ADMIN_FACTION, req);
	}

	// faction id for admin actions on someone else's faction
	protected string AdminFactionArg()
	{
		KPK_View v = KPK_Client.s_View;
		KPK_FactionView f = SelectedFaction();
		if (v && v.IsAdmin && f)
			return f.Id;
		return "";
	}

	protected void SetRole(int role)
	{
		string name = SelectedMemberName();
		if (name == "")
		{
			SetStatus("Выберите бойца в списке состава");
			return;
		}
		KPK_Client.SendText(KPK_C.FACTION_SETROLE, name, AdminFactionArg(), role);
	}

	protected int PaletteIndexOf(int color)
	{
		for (int i = 0; i < KPK_Const.PALETTE_SIZE; i++)
		{
			if (KPK_Const.Palette(i) == color)
				return i;
		}
		return 0;
	}

	// =================================================================== tasks

	protected Widget BuildTasks()
	{
		m_TaskEditing = false;
		Widget page = Content();
		Widget left = KPK_UI.Card(page, 0, 0, 0.38, 1, "ЗАДАНИЯ");
		m_TaskList = new KPK_List(left, 0.04, 0.06, 0.92, 0.8, 10, "— заданий нет —");
		m_Lists.Insert(m_TaskList);
		AddButton(left, B_TS_NEW, "НОВОЕ ЗАДАНИЕ", 0.04, 0.9, 0.92, 0.06);

		Widget right = KPK_UI.Card(page, 0.4, 0, 0.6, 1, "ПОДРОБНОСТИ");
		// reading a task
		m_TaskViewBox = KPK_UI.Panel(right, 0, 0, 1, 1, 0, true);
		m_TaskText = KPK_UI.MText(m_TaskViewBox, 0.04, 0.07, 0.92, 0.8, 15, KPK_UI.C_TEXT);
		AddButton(m_TaskViewBox, B_TS_DONE, "ВЫПОЛНЕНО", 0.04, 0.9, 0.3, 0.06);
		AddButton(m_TaskViewBox, B_TS_FAIL, "ПРОВАЛЕНО", 0.36, 0.9, 0.3, 0.06);
		AddButton(m_TaskViewBox, B_TS_DEL, "УДАЛИТЬ", 0.68, 0.9, 0.28, 0.06);
		// writing a task for the party / faction (leaders only)
		m_TaskEditBox = KPK_UI.Panel(right, 0, 0, 1, 1, 0, true);
		KPK_UI.Text(m_TaskEditBox, "Название задания", 0.03, 0.08, 0.6, 0.04, 13, KPK_UI.C_DIM);
		m_TaskTitleEdit = AddEdit(KPK_UI.Edit(m_TaskEditBox, 0.03, 0.12, 0.94, 0.055));
		KPK_UI.Text(m_TaskEditBox, "Награда (необязательно)", 0.03, 0.19, 0.6, 0.04, 13, KPK_UI.C_DIM);
		m_TaskRewardEdit = AddEdit(KPK_UI.Edit(m_TaskEditBox, 0.03, 0.23, 0.94, 0.055));
		KPK_UI.Text(m_TaskEditBox, "Описание", 0.03, 0.3, 0.6, 0.04, 13, KPK_UI.C_DIM);
		m_TaskDescEdit = AddMulti(KPK_UI.Edit(m_TaskEditBox, 0.03, 0.34, 0.94, 0.52));
		// «Куратор»: which factions get the task (click toggles)
		m_TaskPickBox = KPK_UI.Panel(m_TaskEditBox, 0.03, 0.59, 0.94, 0.29, 0, true);
		KPK_UI.Text(m_TaskPickBox, "Группировки (нажмите, чтобы отметить):", 0, 0, 1, 0.14, 12, KPK_UI.C_DIM);
		m_TaskFactionList = new KPK_List(m_TaskPickBox, 0, 0.15, 1, 0.85, 4, "— группировок нет —");
		m_Lists.Insert(m_TaskFactionList);
		m_TaskPickBox.Show(false);
		AddButton(m_TaskEditBox, B_TS_SCOPE, "", 0.03, 0.9, 0.34, 0.06);
		AddButton(m_TaskEditBox, B_TS_CANCEL, "ОТМЕНА", 0.39, 0.9, 0.26, 0.06);
		AddButton(m_TaskEditBox, B_TS_SEND, "ВЫДАТЬ", 0.67, 0.9, 0.3, 0.06);
		m_TaskEditBox.Show(false);
		return page;
	}

	protected string TaskStatus(int status)
	{
		if (status == 1) return "выполнено";
		if (status == 2) return "провалено";
		return "активно";
	}

	protected string TaskTag(KPK_Task task)
	{
		int scope = task.Scope;
		if (scope == KPK_Scope.PARTY) return "[ГР] ";
		if (scope == KPK_Scope.FACTION) return "[" + KPK_Const.FactionTaskName(task.FactionId) + "] ";
		if (scope == KPK_TASK_PUBLIC) return "[ВСЕМ] ";
		return "";   // personal and admin tasks: a plain task
	}

	// may this player change the task (status / delete)?
	protected bool TaskEditable(KPK_View v, KPK_Task task)
	{
		if (!v || !task)
			return false;
		// «Куратор»: any party / faction / public / admin task
		if (v.TaskGlobal && task.Scope != KPK_Scope.PERSONAL)
			return true;
		if (task.Scope == KPK_Scope.PARTY)
			return v.TaskParty;
		if (task.Scope == KPK_Scope.FACTION)
			return v.TaskFaction;
		if (task.Scope == KPK_TASK_PUBLIC)
			return v.TaskPublic && task.FactionId == v.MyFactionId;
		if (task.Scope == KPK_TASK_GLOBAL)
			return v.TaskGlobal;
		return false;
	}

	protected KPK_Task SelectedTask()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v || !m_TaskList)
			return null;
		int i = m_TaskList.Selected();
		if (i < 0 || i >= v.Tasks.Count())
			return null;
		return v.Tasks[i];
	}

	protected string FactionNameOf(string id)
	{
		KPK_View v = KPK_Client.s_View;
		if (v)
		{
			foreach (KPK_FactionView f : v.Factions)
			{
				if (f.Id == id)
					return f.Name;
			}
		}
		return "группировка";
	}

	protected bool TaskScopeAllowed(KPK_View v, int scope)
	{
		// «Куратор»: a plain task for everybody, or a task for the chosen factions
		if (v.TaskGlobal)
			return scope == KPK_TASK_GLOBAL || scope == KPK_TASK_ADMIN_FACTIONS;
		if (scope == KPK_Scope.PARTY)
			return v.TaskParty;
		if (scope == KPK_TASK_PUBLIC)
			return v.TaskPublic;
		if (scope == KPK_TASK_GLOBAL)
			return v.TaskGlobal;
		return v.TaskFaction;
	}

	// party -> faction -> everybody, skipping what this player may not post
	protected void NextTaskScope()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;
		for (int step = 0; step < 5; step++)
		{
			if (m_TaskScope == KPK_Scope.PARTY)
				m_TaskScope = KPK_Scope.FACTION;
			else if (m_TaskScope == KPK_Scope.FACTION)
				m_TaskScope = KPK_TASK_PUBLIC;
			else if (m_TaskScope == KPK_TASK_PUBLIC)
				m_TaskScope = KPK_TASK_GLOBAL;
			else if (m_TaskScope == KPK_TASK_GLOBAL)
				m_TaskScope = KPK_TASK_ADMIN_FACTIONS;
			else
				m_TaskScope = KPK_Scope.PARTY;
			if (TaskScopeAllowed(v, m_TaskScope))
				break;
		}
		UpdateTaskScope();
	}

	protected void UpdateTaskScope()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;
		if (!TaskScopeAllowed(v, m_TaskScope))
		{
			if (v.TaskGlobal)
				m_TaskScope = KPK_TASK_GLOBAL;
			else if (v.TaskParty)
				m_TaskScope = KPK_Scope.PARTY;
			else
				m_TaskScope = KPK_Scope.FACTION;
		}
		if (m_TaskScope == KPK_TASK_GLOBAL)
			m_ById.Get(B_TS_SCOPE).SetText("КОМУ: ВСЕМ");
		else if (m_TaskScope == KPK_TASK_ADMIN_FACTIONS)
			m_ById.Get(B_TS_SCOPE).SetText("КОМУ: ГРУППИРОВКАМ");
		else if (m_TaskScope == KPK_TASK_PUBLIC)
			m_ById.Get(B_TS_SCOPE).SetText("КОМУ: ВСЕМ");
		else if (m_TaskScope == KPK_Scope.FACTION)
			m_ById.Get(B_TS_SCOPE).SetText("КОМУ: ГРУППИРОВКЕ");
		else
			m_ById.Get(B_TS_SCOPE).SetText("КОМУ: ГРУППЕ");
		m_ById.Get(B_TS_SCOPE).SetEnabled(true);
		RefreshTaskPicker();
	}

	// «Куратор», task for factions: the description gets shorter, the faction picker shows below it
	protected void RefreshTaskPicker()
	{
		if (!m_TaskPickBox)
			return;
		bool picking = m_TaskScope == KPK_TASK_ADMIN_FACTIONS;
		m_TaskPickBox.Show(picking);
		Widget descRoot = m_TaskDescEdit.GetParent();
		if (picking)
			descRoot.SetSize(0.94, 0.24);
		else
			descRoot.SetSize(0.94, 0.52);
		if (!picking)
			return;
		KPK_View v = KPK_Client.s_View;
		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		if (v)
		{
			foreach (KPK_FactionView f : v.Factions)
			{
				if (m_TaskTargets.Find(f.Id) != -1)
				{
					names.Insert("[x]  " + f.Name + "  · " + KPK_Const.FactionTaskName(f.Id));
					colors.Insert(KPK_UI.C_ACCENT);
				}
				else
				{
					names.Insert("[  ]  " + f.Name);
					colors.Insert(KPK_UI.C_DIM);
				}
			}
		}
		m_TaskFactionList.Set(names, colors);
	}

	protected void ToggleTaskTarget()
	{
		KPK_View v = KPK_Client.s_View;
		int i = m_TaskFactionList.Selected();
		m_TaskFactionList.m_Selected = -1;
		if (v && i >= 0 && i < v.Factions.Count())
		{
			string id = v.Factions[i].Id;
			int at = m_TaskTargets.Find(id);
			if (at == -1)
				m_TaskTargets.Insert(id);
			else
				m_TaskTargets.Remove(at);
		}
		RefreshTaskPicker();
	}

	protected void StartTaskEdit()
	{
		m_TaskEditing = true;
		m_TaskTitleEdit.SetText("");
		m_TaskRewardEdit.SetText("");
		m_TaskDescEdit.SetText("");
		m_TaskList.m_Selected = -1;
		m_TaskList.Redraw();
		RefreshTasks();
	}

	protected void SendTask()
	{
		string text = m_TaskDescEdit.GetText();
		KPK_Req req = new KPK_Req();
		req.I1 = m_TaskScope;
		req.S1 = m_TaskTitleEdit.GetText();
		req.S2 = text;
		req.S3 = m_TaskRewardEdit.GetText();
		if (req.S1.Trim() == "")
		{
			SetStatus("Введите название задания");
			return;
		}
		if (m_TaskScope == KPK_TASK_ADMIN_FACTIONS)
		{
			if (m_TaskTargets.Count() == 0)
			{
				SetStatus("Отметьте группировки в списке");
				return;
			}
			string targets = "";
			foreach (string target : m_TaskTargets)
			{
				if (targets != "")
					targets += ",";
				targets += target;
			}
			req.S4 = targets;
		}
		KPK_Client.Send(KPK_C.TASK_ADD, req);
		m_TaskEditing = false;
		RefreshTasks();
	}

	// status / delete buttons of the selected party or faction task
	protected void TaskAction(int id)
	{
		KPK_Task selTask = SelectedTask();
		if (!selTask)
			return;
		int newStatus = 1;
		if (id == B_TS_FAIL)
			newStatus = 2;
		else if (selTask.Status != 0)
			newStatus = 0;
		int remove = 0;
		if (id == B_TS_DEL)
			remove = 1;
		// the owner of the task, for the admin who manages tasks of any party / faction
		string owner = selTask.FactionId;
		if (selTask.Scope == KPK_Scope.PARTY)
			owner = selTask.PartyId.ToString();
		KPK_Client.SendText(KPK_C.TASK_SET, selTask.Id, owner, newStatus, remove);
	}

	protected void RefreshTasks()
	{
		// a part of the page that the engine refused to create must not break the whole window
		if (!m_TaskList || !m_TaskText || !m_TaskEditBox || !m_TaskViewBox)
			return;
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;

		bool canWrite = v.TaskParty || v.TaskFaction || v.TaskGlobal;
		if (!canWrite)
			m_TaskEditing = false;
		m_ById.Get(B_TS_NEW).Show(canWrite);

		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		foreach (KPK_Task t : v.Tasks)
		{
			names.Insert(TaskTag(t) + t.Title);
			if (t.Status == 1)
				colors.Insert(KPK_UI.C_DIM);
			else if (t.Status == 2)
				colors.Insert(KPK_UI.C_DANGER);
			else
				colors.Insert(KPK_UI.C_ACCENT);
		}
		m_TaskList.Set(names, colors);

		m_TaskEditBox.Show(m_TaskEditing);
		m_TaskViewBox.Show(!m_TaskEditing);
		if (m_TaskEditing)
		{
			UpdateTaskScope();
			return;
		}

		KPK_Task task = SelectedTask();
		bool editable = TaskEditable(v, task);
		bool active = false;
		if (task)
			active = task.Status == 0;
		m_ById.Get(B_TS_DONE).Show(editable);
		m_ById.Get(B_TS_FAIL).Show(editable && active);
		m_ById.Get(B_TS_DEL).Show(editable);
		if (active)
			m_ById.Get(B_TS_DONE).SetText("ВЫПОЛНЕНО");
		else
			m_ById.Get(B_TS_DONE).SetText("ВОЗОБНОВИТЬ");

		if (!task)
		{
			if (v.Tasks.Count() == 0)
			{
				string empty = "Активных заданий нет.\n\nЗадания выдают лидер группы, лидер и заместитель группировки, а также торговцы и другие сталкеры. Когда вам поручат работу, она появится здесь.";
				if (canWrite)
					empty += "\n\nВы можете выдать задание сами: кнопка «НОВОЕ ЗАДАНИЕ».";
				m_TaskText.SetText(empty);
			}
			else
			{
				m_TaskText.SetText("Выберите задание из списка.");
			}
			return;
		}

		string text = task.Title + "\n\n";
		if (task.Scope == KPK_Scope.PARTY)
			text += "Тип:  задание группы\n";
		else if (task.Scope == KPK_Scope.FACTION)
			text += "Тип:  " + KPK_Const.FactionTaskName(task.FactionId) + " · " + FactionNameOf(task.FactionId) + "\n";
		else if (task.Scope == KPK_TASK_PUBLIC)
			text += "Тип:  общее задание для всех сталкеров\n";
		if (task.Issuer != "")
			text += "Заказчик:  " + task.Issuer + "\n";
		if (task.Reward != "")
			text += "Награда:  " + task.Reward + "\n";
		text += "Статус:  " + TaskStatus(task.Status) + "\n";
		text += "Получено:  " + task.Date + "\n\n";
		text += KPK_Util.FromWire(task.Description);
		m_TaskText.SetText(text);
	}

	// =================================================================== notes

	protected Widget BuildNotes()
	{
		m_NoteShown = -2;
		Widget page = Content();
		Widget left = KPK_UI.Card(page, 0, 0, 0.32, 1, "ЗАПИСИ");
		m_NoteList = new KPK_List(left, 0.04, 0.06, 0.92, 0.8, 10, "— записей нет —");
		m_Lists.Insert(m_NoteList);
		AddButton(left, B_N_NEW, "НОВАЯ", 0.04, 0.9, 0.45, 0.06);
		AddButton(left, B_N_DEL, "УДАЛИТЬ", 0.51, 0.9, 0.45, 0.06);

		Widget right = KPK_UI.Card(page, 0.34, 0, 0.66, 1, "ЗАМЕТКА");
		KPK_UI.Text(right, "Заголовок", 0.03, 0.06, 0.3, 0.04, 13, KPK_UI.C_DIM);
		m_NoteTitle = AddEdit(KPK_UI.Edit(right, 0.03, 0.1, 0.94, 0.055));
		m_NoteText = AddMulti(KPK_UI.Edit(right, 0.03, 0.18, 0.94, 0.7));
		m_NoteInfo = KPK_UI.Text(right, "", 0.03, 0.9, 0.6, 0.06, 13, KPK_UI.C_DIM);
		AddButton(right, B_N_SAVE, "СОХРАНИТЬ", 0.67, 0.9, 0.3, 0.06);
		return page;
	}

	protected void RefreshNotes()
	{
		if (!m_NoteList)
			return;
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;

		array<string> names = new array<string>();
		foreach (KPK_Note n : v.Notes)
			names.Insert(n.Title);
		m_NoteList.Set(names);

		if (m_SelectNewestNote && v.Notes.Count() > 0)
		{
			m_SelectNewestNote = false;
			m_NoteList.Select(v.Notes.Count() - 1);
			m_NoteShown = -2;
		}
		ShowSelectedNote(false);
	}

	// force: reload the edit boxes even if the same note is shown
	protected void ShowSelectedNote(bool force)
	{
		KPK_View v = KPK_Client.s_View;
		int i = m_NoteList.Selected();
		if (!force && i == m_NoteShown)
			return;
		m_NoteShown = i;

		if (!v || i < 0 || i >= v.Notes.Count())
		{
			if (force)
			{
				m_NoteId = 0;
				m_NoteTitle.SetText("");
				m_NoteText.SetText("");
				m_NoteInfo.SetText("Новая запись");
			}
			return;
		}
		KPK_Note note = v.Notes[i];
		m_NoteId = note.Id;
		m_NoteTitle.SetText(note.Title);
		// the one-line input has no line breaks: they become spaces while editing
		string noteBody = KPK_Util.FromWire(note.Text);
		noteBody.Replace("\n", " ");
		m_NoteText.SetText(noteBody);
		m_NoteInfo.SetText("Изменено: " + note.Date);
	}

	protected void SaveNote()
	{
		string text = m_NoteText.GetText();
		KPK_Req req = new KPK_Req();
		req.I1 = m_NoteId;
		req.S1 = m_NoteTitle.GetText();
		req.S2 = text;
		if (m_NoteId == 0)
			m_SelectNewestNote = true;
		KPK_Client.Send(KPK_C.NOTE_SAVE, req);
	}

	// =================================================================== chat

	protected Widget BuildChat()
	{
		Widget page = Content();
		Widget card = KPK_UI.Card(page, 0, 0, 1, 1, "ЭФИР");
		m_ChatLog = KPK_UI.MText(card, 0.03, 0.08, 0.86, 0.72, 14, KPK_UI.C_TEXT);
		AddButton(card, B_C_OLDER, "/\\", 0.91, 0.08, 0.06, 0.08);
		AddButton(card, B_C_NEWER, "\\/", 0.91, 0.72, 0.06, 0.08);
		KPK_UI.Panel(card, 0.03, 0.805, 0.94, 0.002, KPK_UI.C_LINE);
		m_ChatChannel = KPK_UI.Text(card, "", 0.03, 0.815, 0.62, 0.055, 13, KPK_UI.C_DIM);
		AddButton(card, B_C_PUBLIC, "ОБЩИЙ КАНАЛ", 0.67, 0.815, 0.3, 0.055);
		m_ChatEdit = AddMulti(KPK_UI.Edit(card, 0.03, 0.885, 0.72, 0.065));
		AddButton(card, B_C_SEND, "ПЕРЕДАТЬ", 0.77, 0.885, 0.2, 0.065);
		return page;
	}

	protected string KPK_ChatLineText(KPK_ChatMessage m)
	{
		string who = m.From;
		if (m.Tag != "")
			who = "[" + m.Tag + "] " + who;
		if (m.To != "")
			who = "ЛС " + who + " > " + m.To;
		return "[" + m.Time + "] " + who + ": " + KPK_Util.FromWire(m.Text);
	}

	// older lines came from the server: scroll up onto them
	void OnChatOlder(int added)
	{
		if (added > 0)
			m_ChatOffset += CHAT_LINES / 2;
		RefreshChat();
	}

	protected void RefreshChat()
	{
		if (!m_ChatLog)
			return;
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;

		int count = v.Chat.Count();
		int maxOffset = count - CHAT_LINES;
		if (maxOffset < 0)
			maxOffset = 0;
		if (m_ChatOffset > maxOffset)
			m_ChatOffset = maxOffset;
		int last = count - 1 - m_ChatOffset;
		int first = last - CHAT_LINES + 1;
		if (first < 0)
			first = 0;

		string text = "";
		if (count > 0 && first == 0)
		{
			if (KPK_Client.s_ChatLoading)
				text = ". . . загрузка архива . . .";
			else if (KPK_Client.s_ChatMore)
				text = ". . . (/\\ — более старые сообщения)";
			else
				text = "— начало истории эфира —";
		}
		if (count == 0)
			text = "В эфире тихо.\n\nСообщения общего канала видят все сталкеры с КПК в Зоне. Личное сообщение: вкладка контактов, кнопка «НАПИСАТЬ».";
		for (int i = first; i <= last; i++)
		{
			if (text != "")
				text += "\n";
			text += KPK_ChatLineText(v.Chat[i]);
		}
		if (m_ChatOffset > 0)
			text += "\n. . . (ещё " + m_ChatOffset.ToString() + " новее)";
		m_ChatLog.SetText(text);

		if (m_ChatTo == "")
			m_ChatChannel.SetText("Канал: ОБЩИЙ   (до " + v.ChatMaxLength + " символов)");
		else
			m_ChatChannel.SetText("Канал: ЛИЧНО > " + m_ChatTo);
		m_ById.Get(B_C_PUBLIC).Show(m_ChatTo != "");
	}

	protected void SendChat()
	{
		string text = m_ChatEdit.GetText();
		m_ChatEdit.SetText("");
		if (text.Trim() == "")
			return;
		KPK_Client.SendText(KPK_C.CHAT_SEND, text, m_ChatTo);
		m_ChatOffset = 0;
	}

	// =================================================================== contacts

	// =================================================================== admin terminal («Куратор»)

	protected Widget BuildAdmin()
	{
		Widget page = Content();
		Widget left = KPK_UI.Card(page, 0, 0, 0.42, 1, "ИГРОКИ СЕТИ");
		m_PlayerList = new KPK_List(left, 0.04, 0.06, 0.92, 0.62, 9, "— игроков нет —");
		m_Lists.Insert(m_PlayerList);
		KPK_UI.Text(left, "Позывной или Steam ID", 0.04, 0.695, 0.9, 0.045, 13, KPK_UI.C_DIM);
		m_AdminName = AddEdit(KPK_UI.Edit(left, 0.04, 0.745, 0.92, 0.06));
		AddButton(left, B_T_INSPECT, "СМОТРЕТЬ КПК", 0.04, 0.83, 0.45, 0.06);
		AddButton(left, B_T_SELF, "СВОЙ КПК", 0.51, 0.83, 0.45, 0.06);
		AddButton(left, B_T_ADMIN_ADD, "+ АДМИН", 0.04, 0.91, 0.45, 0.06);
		AddButton(left, B_T_ADMIN_DEL, "- АДМИН", 0.51, 0.91, 0.45, 0.06);

		Widget right = KPK_UI.Card(page, 0.44, 0, 0.56, 1, "СВЕДЕНИЯ");
		m_TermText = KPK_UI.MText(right, 0.04, 0.08, 0.92, 0.88, 15, KPK_UI.C_TEXT);
		return page;
	}

	protected KPK_PlayerEntry SelectedPlayer()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v || !m_PlayerList)
			return null;
		int i = m_PlayerList.Selected();
		if (i < 0 || i >= v.Players.Count())
			return null;
		return v.Players[i];
	}

	protected void RefreshAdmin()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v || !m_PlayerList || !m_TermText)
			return;
		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		int online = 0;
		foreach (KPK_PlayerEntry pe : v.Players)
		{
			string row = pe.Callsign;
			if (pe.Admin)
				row += "  [А]";
			if (pe.Faction != "")
				row += "  · " + pe.Faction;
			names.Insert(row);
			if (pe.Online)
			{
				online++;
				colors.Insert(KPK_UI.C_ACCENT);
			}
			else
				colors.Insert(KPK_UI.C_DIM);
		}
		m_PlayerList.Set(names, colors);

		string text = "";
		if (!v.IsAdmin)
			text = "Нет прав администратора.\n";
		if (v.Inspect != "")
		{
			text += "ПРОСМОТР КПК (только чтение)\n" + v.InspectInfo + "\n\n";
			text += "Вкладки «Карта», «Группа», «Группировки», «Задания», «Заметки», «Чат» и «Контакты» показывают его КПК.\n";
			text += "Кнопка «СВОЙ КПК» возвращает к вашему.\n\n";
		}
		else
		{
			text += "Администратор: " + v.Callsign + "\n";
			text += "Игроков в сети: " + online.ToString() + " из " + v.Players.Count().ToString() + "\n\n";
		}
		KPK_PlayerEntry sel = SelectedPlayer();
		if (sel)
		{
			text += "Выбран: " + sel.Callsign;
			if (sel.Online)
				text += "  · в сети";
			else
				text += "  · не в сети";
			if (sel.Admin)
				text += "  · администратор";
			if (sel.Faction != "")
				text += "\nГруппировка: " + sel.Faction;
			text += "\n\n";
		}
		text += "Группировки: состав, лидер, шифрование — во вкладке «Группировки» (панель администратора).\n";
		text += "[А] — администратор. Добавленный админ может пользоваться «Куратором».";
		m_TermText.SetText(text);
	}

	protected Widget BuildContacts()
	{
		Widget page = Content();
		Widget left = KPK_UI.Card(page, 0, 0, 0.42, 1, "КОНТАКТЫ");
		m_ContactList = new KPK_List(left, 0.04, 0.06, 0.92, 0.7, 9, "— контактов нет —");
		m_Lists.Insert(m_ContactList);
		KPK_UI.Text(left, "Дать свой контакт (позывной):", 0.04, 0.775, 0.92, 0.045, 13, KPK_UI.C_DIM);
		m_ContactName = AddEdit(KPK_UI.Edit(left, 0.04, 0.825, 0.92, 0.06));
		AddButton(left, B_K_ADD, "ДАТЬ КОНТАКТ", 0.04, 0.91, 0.45, 0.06);
		AddButton(left, B_K_DEL, "УДАЛИТЬ", 0.51, 0.91, 0.45, 0.06);

		Widget right = KPK_UI.Card(page, 0.44, 0, 0.56, 1, "СВЕДЕНИЯ");
		m_ContactText = KPK_UI.MText(right, 0.04, 0.08, 0.92, 0.58, 15, KPK_UI.C_TEXT);
		AddButton(right, B_K_WRITE, "НАПИСАТЬ", 0.04, 0.7, 0.44, 0.07);
		AddButton(right, B_K_PARTY, "В ГРУППУ", 0.52, 0.7, 0.44, 0.07);
		MultilineTextWidget hint = KPK_UI.MText(right, 0.04, 0.8, 0.92, 0.18, 12, KPK_UI.C_DIM);
		hint.SetText("Статус «в сети» виден у всех контактов.");
		if (m_Theme.Glitch)
			hint.Show(false);
		return page;
	}

	protected KPK_ContactView SelectedContact()
	{
		KPK_View v = KPK_Client.s_View;
		int i = m_ContactList.Selected();
		if (!v || i < 0 || i >= v.Contacts.Count())
			return null;
		return v.Contacts[i];
	}

	protected void RefreshContacts()
	{
		if (!m_ContactList)
			return;
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;

		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		foreach (KPK_ContactView c : v.Contacts)
		{
			string row = c.Callsign;

			if (c.Online)
			{
				row += "   · в сети";
				colors.Insert(KPK_UI.C_ACCENT);
			}
			else
				colors.Insert(KPK_UI.C_DIM);
			names.Insert(row);
		}
		m_ContactList.Set(names, colors);

		KPK_ContactView sel = SelectedContact();
		if (!sel)
		{
			if (v.Contacts.Count() == 0)
				m_ContactText.SetText("Список пуст.\n\nВпишите позывной сталкера слева и нажмите «ДОБАВИТЬ».");
			else
				m_ContactText.SetText("Выберите контакт из списка.");
			return;
		}
		string text = sel.Callsign + "\n\n";
		if (sel.Online)
			text += "Статус:  в сети\n";
		else
			text += "Статус:  нет сигнала\n";
		string faction = sel.Faction;
		if (faction == "")
			faction = "неизвестно";
		text += "Группировка:  " + faction + "\n";
		if (sel.LastSeen != "")
			text += "Последний вход:  " + sel.LastSeen + "\n";
		if (sel.Mutual)
			text += "\nКонтакт взаимный.";
		else
			text += "\nВас нет в его контактах.";
		m_ContactText.SetText(text);
	}

	// =================================================================== data events

	void OnView()
	{
		if (m_Booting)
			return;
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;

		if (!v.LoggedIn)
		{
			if (m_Page != P_LOGIN)
				ShowPage(P_LOGIN);
			else
				RefreshLogin();
		}
		else if (m_Page == P_LOGIN || m_Page == P_BOOT || m_Page < 0)
		{
			// the hidden login boxes must not keep the password nor the keyboard focus: the typing went
			// on into the hidden box and its text showed up in other fields' commands
			if (m_LoginPassword)
				m_LoginPassword.SetText("");
			if (m_LoginRepeat)
				m_LoginRepeat.SetText("");
			SetFocus(null);
			ShowPage(m_LastTab);
		}
		else
		{
			RefreshPage();
		}

		// greeting of the model, once per login
		if (v.LoggedIn && !m_WasLoggedIn)
		{
			string welcome = m_Theme.Welcome;
			welcome.Replace("%1", v.Callsign);
			SetStatus(welcome);
		}
		m_WasLoggedIn = v.LoggedIn;
		RefreshHeader();
	}

	void OnChat(KPK_ChatMessage line)
	{
		if (m_Page == P_CHAT)
		{
			m_ChatOffset = 0;
			RefreshChat();
			return;
		}
		KPK_View v = KPK_Client.s_View;
		if (v && line.From != v.Callsign && v.LoggedIn)
		{
			KPK_Sound.Play("sms");
			if (line.To != "")
				SetStatus("Личное сообщение от " + line.From);
			else
				SetStatus("Сообщение в эфире: " + line.From);
		}
	}

	void OnStatus(string text)
	{
		SetStatus(text);
	}

	void OnMates()
	{
		if (m_Page == P_MAP)
			RedrawMapMarks();
	}

	protected void SetStatus(string text)
	{
		m_Status = text;
		RefreshFooter();
	}

	protected void RefreshPage()
	{
		switch (m_Page)
		{
			case P_LOGIN: RefreshLogin(); break;
			case P_PROFILE:
				if (m_Admin)
					RefreshAdmin();
				else
					RefreshProfile();
				break;
			case P_MAP: RefreshMap(); break;
			case P_PARTY: RefreshParty(); break;
			case P_FACTIONS: RefreshFactions(); break;
			case P_TASKS: RefreshTasks(); break;
			case P_NOTES: RefreshNotes(); break;
			case P_CHAT: RefreshChat(); break;
			case P_CONTACTS: RefreshContacts(); break;
			case P_METRO: RefreshMetro(); break;
		}
	}

	protected void RefreshHeader()
	{
		KPK_View v = KPK_Client.s_View;
		string net = "СЕТЬ ЗОНЫ";
		string user = "гость";
		string battery = "ПИТ --";
		bool low = false;
		if (v)
		{
			if (v.ServerName != "")
				net = v.ServerName;
			if (v.LoggedIn)
				user = v.Callsign;
			if (v.Battery >= 0)
			{
				battery = "ПИТ " + KPK_UI.Percent(v.Battery, 1);
				low = v.Battery < 0.15;
			}
			if (m_Skin.m_Serial)
				m_Skin.m_Serial.SetText("S/N " + v.SteamId);
		}
		m_Skin.SetLed(low);
		string header = m_Theme.OsName + "  —  " + PageName(m_Page);
		if (v && v.Inspect != "")
		{
			header = "ПРОСМОТР КПК: " + v.Inspect + "  ·  только чтение  —  " + PageName(m_Page);
			user = v.Inspect + " (просмотр)";
		}
		if (m_Theme.Glitch)
			header = KPK_Theme.Noise(header, 0.3);
		m_HeaderLeft.SetText(header);
		// the module tabs sometimes vanished: show them again with every clock tick
		ShowTabs(m_Page >= 0 && m_Page < TAB_COUNT);

		int year, month, day, hour, minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);
		string clock = hour.ToStringLen(2) + ":" + minute.ToStringLen(2);
		m_HeaderRight.SetText(string.Format("%1.%2.%3   %4", day.ToStringLen(2), month.ToStringLen(2), year, clock));

		string bars = "||||";
		if (m_Signal == 3) bars = "|||.";
		if (m_Signal == 2) bars = "||..";
		if (m_Signal <= 1) bars = "|...";
		m_StatusRight.SetText("СИГН " + bars + "   " + battery + "   GPS: OK");

		int sec = m_SessionTime;
		int mins = sec / 60;
		int secs = sec % 60;
		m_SideInfo.SetText("ПОЛЬЗ.: " + user + "\nУЗЕЛ: " + net + "\nСЕАНС: " + mins.ToStringLen(2) + ":" + secs.ToStringLen(2) + "\nПАМ: 640 КБ");
	}

	protected void RefreshFooter()
	{
		string cursor = " ";
		if (m_Blink)
			cursor = "_";
		string status = m_Status;
		if (m_Theme.Glitch)
			status = KPK_Theme.Noise(status, 0.18);
		m_Footer.SetText("> " + status + cursor);
	}

	// =================================================================== input

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		// closing comes first: it must work even if part of the interface failed to build. The "menu"
		// input exclude can swallow UAUIBack, so the Escape key itself is read too
		if (GetUApi().GetInputByID(UAUIBack).LocalPress() || KeyState(KeyCode.KC_ESCAPE))
		{
			ClearKey(KeyCode.KC_ESCAPE);
			CloseIfOpen();
			return;
		}

		if (m_Booting)
		{
			m_BootTime += timeslice;
			UpdateBoot();
		}

		// the text boxes show what was typed (see KPK_TextBox)
		m_TextBoxTime += timeslice;
		if (m_TextBoxTime > 0.05)
		{
			m_TextBoxTime = 0;
			SyncTextBoxes();
		}

		m_BlinkTime += timeslice;
		if (m_BlinkTime > 0.5)
		{
			m_BlinkTime = 0;
			m_Blink = !m_Blink;
			RefreshFooter();
		}

		m_Skin.Update(timeslice);
		m_SessionTime += timeslice;

		// now and then the device beeps or crackles by itself
		m_AmbientTime -= timeslice;
		if (m_AmbientTime <= 0)
		{
			m_AmbientTime = Math.RandomFloat(14.0, 40.0);
			KPK_Sound.Ambient(m_Theme.Glitch);
		}

		m_SignalTime -= timeslice;
		if (m_SignalTime <= 0)
		{
			m_SignalTime = Math.RandomFloat(2.0, 6.0);
			m_Signal = Math.RandomInt(2, 5);
		}

		m_Clock += timeslice;
		if (m_Clock > 1)
		{
			m_Clock = 0;
			// the screen size changed (or was not known when the KPK opened): fit the device again
			int nowW, nowH;
			GetScreenSize(nowW, nowH);
			if (m_Device && nowW > 0 && nowH > 0 && (nowW != m_FitW || nowH != m_FitH))
				FitDevice(m_Device, m_FitModel);
			RefreshHeader();
		}

		if (m_Page == P_METRO)
			MetroTick(timeslice);
		if (m_Page == P_MAP || m_Page == P_METRO)
		{
			m_MapRedrawTime += timeslice;
			if (m_MapRedrawTime > 1)
			{
				m_MapRedrawTime = 0;
				RedrawMapMarks();
			}
		}

		if (GetUApi().GetInputByID(UAUIBack).LocalPress())
		{
			CloseIfOpen();
			return;
		}

		// the toggle key closes the KPK unless the player is typing
		Widget focus = GetFocus();
		bool typing = (EditBoxWidget.Cast(focus) != null) || (MultilineEditBoxWidget.Cast(focus) != null);
		if (!typing && GetUApi().GetInputByName("UAKPKToggle").LocalPress())
			CloseIfOpen();
	}

	override bool OnKeyDown(Widget w, int x, int y, int key)
	{
		if (key == KeyCode.KC_ESCAPE)
		{
			CloseIfOpen();
			return true;
		}
		return super.OnKeyDown(w, x, y, key);
	}

	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		// Enter in the password box submits the login form
		if (finished && (w == m_LoginPassword || w == m_LoginRepeat) && m_Page == P_LOGIN)
		{
			SubmitLogin();
			return true;
		}
		// Enter in the chat line sends it
		if (finished && w == m_ChatEdit && m_Page == P_CHAT)
		{
			SendChat();
			return true;
		}
		return super.OnChange(w, x, y, finished);
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		foreach (KPK_Button b : m_Buttons)
		{
			if (b.m_Root == w)
			{
				b.SetHover(true);
				return true;
			}
		}
		foreach (KPK_List list : m_Lists)
		{
			if (list.Hover(w, true))
				return true;
		}
		return super.OnMouseEnter(w, x, y);
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		foreach (KPK_Button b : m_Buttons)
		{
			if (b.m_Root == w)
			{
				b.SetHover(false);
				return true;
			}
		}
		foreach (KPK_List list : m_Lists)
		{
			if (list.Hover(w, false))
				return true;
		}
		return super.OnMouseLeave(w, enterW, x, y);
	}

	// a click into a text field always takes the keyboard there
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button == MouseState.LEFT && w && (EditBoxWidget.Cast(w) || MultilineEditBoxWidget.Cast(w)))
		{
			SetFocus(w);
			// the click put the (invisible) cursor where the mouse was, in the middle of the text, while
			// the drawn cursor is always at the end: Backspace then left the rest of the text. Writing the
			// text back moves the cursor to its end, a moment after the engine placed it.
			m_CaretBox = EditBoxWidget.Cast(w);
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(CaretToEnd, 60, false);
		}
		return super.OnMouseButtonDown(w, x, y, button);
	}

	protected EditBoxWidget m_CaretBox;

	void CaretToEnd()
	{
		if (!m_CaretBox)
			return;
		string text = m_CaretBox.GetText();
		m_CaretBox.SetText("");
		m_CaretBox.SetText(text);
		m_CaretBox = null;
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button != MouseState.LEFT)
			return super.OnClick(w, x, y, button);

		// lists first: rows and pagers
		foreach (KPK_List list : m_Lists)
		{
			if (!list.Click(w))
				continue;
			KPK_Sound.Play("click");
			OnListClicked(list);
			return true;
		}

		foreach (KPK_Button b : m_Buttons)
		{
			if (b.m_Root != w)
				continue;
			if (b.m_Enabled)
			{
				// module tabs blip, every other key clicks
				if (b.m_Id >= B_TAB && b.m_Id < B_TAB + TAB_COUNT)
					KPK_Sound.Play("tab");
				else
					KPK_Sound.Play("click");
				OnButton(b.m_Id);
			}
			return true;
		}
		return super.OnClick(w, x, y, button);
	}

	protected void OnListClicked(KPK_List list)
	{
		if (list == m_FactionList)
		{
			m_MemberList.m_Selected = -1;
			m_ConfirmRemove = "";
			RefreshFactionDetails();
		}
		else if (list == m_TaskFactionList)
		{
			ToggleTaskTarget();
		}
		else if (list == m_TaskList)
		{
			m_TaskEditing = false;
			RefreshTasks();
		}
		else if (list == m_NoteList)
		{
			ShowSelectedNote(false);
		}
		else if (list == m_ContactList)
		{
			RefreshContacts();
		}
		else if (list == m_PlayerList)
		{
			KPK_PlayerEntry pe = SelectedPlayer();
			if (pe && m_AdminName)
				m_AdminName.SetText(pe.Callsign);
			RefreshAdmin();
		}
		else if (list == m_MetroLineList)
		{
			m_MetroSel = m_MetroLineList.Selected();
			m_MetroHasFocus = false;
			DisarmMetroDelete();
			m_MetroPointList.Select(-1);
			RefreshMetro();
			FitMetroLine();
		}
		else if (list == m_MetroPointList)
		{
			int mp = m_MetroPointList.Selected();
			if (mp >= 0 && mp < m_MetroPointPos.Count())
			{
				m_MetroHasFocus = true;
				m_MetroFocus = m_MetroPointPos[mp];
				m_MetroFocusName = m_MetroPointName[mp];
				if (m_Map)
				{
					m_Map.SetScale(0.06);
					m_Map.SetMapPos(m_MetroFocus);
				}
				SetStatus(m_MetroFocusName + ": квадрат " + KPK_UI.Grid(m_MetroFocus[0], m_MetroFocus[2]) + "  (X " + Math.Round(m_MetroFocus[0]) + ", Z " + Math.Round(m_MetroFocus[2]) + ")");
				RedrawMetroMarks();
			}
		}
		else if (list == m_MarkerList)
		{
			KPK_Marker mk = SelectedMarker();
			if (mk)
			{
				StartEdit(mk);
				if (m_Map)
					m_Map.SetMapPos(Vector(mk.X, 0, mk.Z));
			}
		}
	}

	protected void OnButton(int id)
	{
		if (id >= B_TAB && id < B_TAB + TAB_COUNT)
		{
			ShowPage(id - B_TAB);
			return;
		}
		if (id == B_M_LINE)
		{
			m_MetroHasFocus = false;
			FitMetroLine();
			RedrawMetroMarks();
			return;
		}
		if (id == B_M_ALL)
		{
			m_MetroHasFocus = false;
			FitMetroAll();
			RedrawMetroMarks();
			return;
		}
		if (id == B_M_DEL)
		{
			MetroDelete();
			return;
		}
		if (id == B_M_RELOAD)
		{
			LoadMetro();
			RefreshMetro();
			SetStatus("Метро: линий " + m_MetroLines.Count() + ", пересечений " + m_MetroCross.Count());
			return;
		}

		KPK_View v = KPK_Client.s_View;
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		KPK_FactionView f = SelectedFaction();

		switch (id)
		{
			case B_CLOSE:
				CloseIfOpen();
				break;
			case B_LOGIN:
				SubmitLogin();
				break;

			// map
			case B_TYPE_PREV:
				m_NewType = (m_NewType + KPK_Const.MARKER_TYPES - 1) % KPK_Const.MARKER_TYPES;
				UpdateMarkerEditor();
				break;
			case B_TYPE_NEXT:
				m_NewType = (m_NewType + 1) % KPK_Const.MARKER_TYPES;
				UpdateMarkerEditor();
				break;
			case B_SCOPE_PREV:
				m_NewScope = (m_NewScope + ScopeCount() - 1) % ScopeCount();
				UpdateMarkerEditor();
				break;
			case B_SCOPE_NEXT:
				m_NewScope = (m_NewScope + 1) % ScopeCount();
				UpdateMarkerEditor();
				break;
			case B_MAP_HERE:
			{
				if (m_EditId >= 0)
				{
					SaveEdit(false, 0, 0);
					break;
				}
				if (!player)
					break;
				vector here = player.GetPosition();
				AddMarker(here[0], here[2]);
				break;
			}
			case B_MAP_NEW:
				StopEdit();
				break;
			case B_MAP_SELF:
				if (player && m_Map)
					m_Map.SetMapPos(player.GetPosition());
				break;
			case B_MAP_GOTO:
			{
				KPK_Marker gm = SelectedMarker();
				if (gm && m_Map)
					m_Map.SetMapPos(Vector(gm.X, 0, gm.Z));
				break;
			}
			case B_MAP_DEL:
			{
				KPK_Marker dm = SelectedMarker();
				if (dm)
				{
					KPK_Client.SendText(KPK_C.MARKER_DEL, "", "", dm.Id, dm.Scope);
					StopEdit();
				}
				else
					SetStatus("Выберите метку в списке");
				break;
			}

			// party
			case B_P_INVITE:
				KPK_Client.SendText(KPK_C.PARTY_INVITE, m_PartyInviteName.GetText());
				m_PartyInviteName.SetText("");
				break;
			case B_P_LEAVE:
				KPK_Client.Send(KPK_C.PARTY_LEAVE);
				break;
			case B_P_KICK:
			{
				int pi = m_PartyList.Selected();
				if (v && pi >= 0 && pi < v.Party.Count())
					KPK_Client.SendText(KPK_C.PARTY_KICK, v.Party[pi].Callsign);
				else
					SetStatus("Выберите сталкера в списке группы");
				break;
			}
			case B_INV_ACCEPT:
				RespondInvite(true);
				break;
			case B_INV_DECLINE:
				RespondInvite(false);
				break;
			case B_LABELS:
				KPK_Client.s_LabelsEnabled = !KPK_Client.s_LabelsEnabled;
				RefreshParty();
				break;

			// faction
			case B_F_LEAVE:
				KPK_Client.Send(KPK_C.FACTION_LEAVE);
				break;
			case B_F_INVITE:
				if (KPK_Client.s_View && KPK_Client.s_View.IsAdmin)
					Admin(KPK_AdminOp.ADD_MEMBER, m_FactionInviteName.GetText());
				else
					KPK_Client.SendText(KPK_C.FACTION_INVITE, m_FactionInviteName.GetText());
				m_FactionInviteName.SetText("");
				break;
			case B_F_KICK:
			{
				string kickName = SelectedMemberName();
				if (kickName != "")
					KPK_Client.SendText(KPK_C.FACTION_KICK, kickName, AdminFactionArg());
				else
					SetStatus("Выберите бойца в списке состава");
				break;
			}
			case B_F_MEMBER:
				SetRole(KPK_Role.MEMBER);
				break;
			case B_F_DEPUTY:
				SetRole(KPK_Role.DEPUTY);
				break;
			case B_F_LEADER:
				SetRole(KPK_Role.LEADER);
				break;
			case B_F_MAP:
				if (f && f.HasLocationPos)
				{
					ShowPage(P_MAP);
					if (m_Map)
						m_Map.SetMapPos(Vector(f.LocX, 0, f.LocZ));
				}
				break;

			// admin
			case B_A_ENC_DESC: Admin(KPK_AdminOp.TOGGLE_ENC, "", KPK_EncField.DESCRIPTION); break;
			case B_A_ENC_LEADER: Admin(KPK_AdminOp.TOGGLE_ENC, "", KPK_EncField.LEADER); break;
			case B_A_ENC_MEMBERS: Admin(KPK_AdminOp.TOGGLE_ENC, "", KPK_EncField.MEMBERS); break;
			case B_A_ENC_LOC: Admin(KPK_AdminOp.TOGGLE_ENC, "", KPK_EncField.LOCATION); break;
			case B_A_HIDDEN: Admin(KPK_AdminOp.TOGGLE_HIDDEN); break;
			case B_A_MATES: Admin(KPK_AdminOp.TOGGLE_MATES); break;
			case B_A_COLOR:
				if (f)
					Admin(KPK_AdminOp.SET_COLOR, "", (PaletteIndexOf(f.Color) + 1) % KPK_Const.PALETTE_SIZE);
				break;
			case B_A_LOC_TEXT: Admin(KPK_AdminOp.SET_LOCATION, m_AdminText.GetText(), 0); break;
			case B_A_LOC_ME: Admin(KPK_AdminOp.SET_LOCATION, m_AdminText.GetText(), 1); break;
			case B_A_LOC_CLEAR: Admin(KPK_AdminOp.CLEAR_LOCATION_POS); break;
			case B_A_DESC: Admin(KPK_AdminOp.SET_DESC, m_AdminText.GetText()); break;
			case B_A_CREATE: Admin(KPK_AdminOp.CREATE, m_AdminText.GetText()); break;
			case B_A_REMOVE:
			{
				if (!f)
					break;
				if (m_ConfirmRemove == f.Id)
				{
					// send first: Admin() takes the faction from the selection
					m_ConfirmRemove = "";
					Admin(KPK_AdminOp.REMOVE);
					m_FactionList.m_Selected = -1;
				}
				else
				{
					m_ConfirmRemove = f.Id;
					RefreshFactionDetails();
				}
				break;
			}

			// tasks
			case B_TS_NEW:
				StartTaskEdit();
				break;
			case B_TS_CANCEL:
				m_TaskEditing = false;
				RefreshTasks();
				break;
			case B_TS_SCOPE:
				NextTaskScope();
				break;
			case B_TS_SEND:
				SendTask();
				break;
			case B_TS_DONE:
				TaskAction(id);
				break;
			case B_TS_FAIL:
				TaskAction(id);
				break;
			case B_TS_DEL:
				TaskAction(id);
				break;

			// notes
			case B_N_NEW:
				m_NoteList.m_Selected = -1;
				m_NoteList.Redraw();
				ShowSelectedNote(true);
				break;
			case B_N_SAVE:
				SaveNote();
				break;
			case B_N_DEL:
				if (m_NoteId > 0)
				{
					KPK_Client.SendText(KPK_C.NOTE_DEL, "", "", m_NoteId);
					m_NoteList.m_Selected = -1;
					ShowSelectedNote(true);
				}
				break;

			// radio (brick)
			case B_R_CALL:
				KPK_Client.SendText(KPK_C.CALL_START, m_RadioName.GetText());
				m_RadioName.SetText("");
				break;
			case B_R_DIAL:
			{
				string dialName = BrickContactName();
				if (dialName == "")
					SetStatus("Выберите контакт в списке");
				else
					KPK_Client.SendText(KPK_C.CALL_START, dialName);
				break;
			}
			case B_R_DROP:
			{
				string dropName = BrickContactName();
				if (dropName != "")
				{
					KPK_Client.SendText(KPK_C.CONTACT_DEL, dropName);
					m_BrickContacts.Select(-1);
				}
				break;
			}
			case B_R_ANSWER:
			{
				// like SFP: the brick goes into empty hands when answering
				PlayerBase answerer = PlayerBase.Cast(GetGame().GetPlayer());
				EntityAI myBrick = null;
				if (answerer)
					myBrick = KPK_Device.FindOn(answerer);
				if (answerer && myBrick && KPK_Device.IsBrick(myBrick) && !answerer.GetHumanInventory().GetEntityInHands())
					answerer.PredictiveTakeEntityToHands(myBrick);
				KPK_Client.SendText(KPK_C.CALL_ANSWER, "", "", 1);
			}
				break;
			case B_R_END:
			{
				int answerOrEnd = KPK_C.CALL_END;
				if (v && v.CallState == KPK_CallState.INCOMING)
				{
					KPK_Client.SendText(KPK_C.CALL_ANSWER, "", "", 0);
					break;
				}
				KPK_Client.Send(answerOrEnd);
				break;
			}

			// chat
			case B_C_SEND:
				SendChat();
				break;
			case B_C_PUBLIC:
				m_ChatTo = "";
				RefreshChat();
				break;
			case B_C_OLDER:
			{
				if (!KPK_Client.s_View)
					break;
				int topOffset = KPK_Client.s_View.Chat.Count() - CHAT_LINES;
				if (m_ChatOffset >= topOffset)
					KPK_Client.RequestOlderChat();   // at the top: ask the server for the archive
				m_ChatOffset += CHAT_LINES / 2;
				RefreshChat();
				break;
			}
			case B_C_NEWER:
				m_ChatOffset -= CHAT_LINES / 2;
				if (m_ChatOffset < 0)
					m_ChatOffset = 0;
				RefreshChat();
				break;

			// contacts
			case B_T_INSPECT:
				if (m_AdminName)
					KPK_Client.SendText(KPK_C.ADMIN, m_AdminName.GetText(), "", KPK_AdminCmd.INSPECT);
				break;
			case B_T_SELF:
				KPK_Client.SendText(KPK_C.ADMIN, "", "", KPK_AdminCmd.INSPECT);
				break;
			case B_T_ADMIN_ADD:
				if (m_AdminName)
					KPK_Client.SendText(KPK_C.ADMIN, m_AdminName.GetText(), "", KPK_AdminCmd.ADD_ADMIN);
				break;
			case B_T_ADMIN_DEL:
				if (m_AdminName)
					KPK_Client.SendText(KPK_C.ADMIN, m_AdminName.GetText(), "", KPK_AdminCmd.REMOVE_ADMIN);
				break;
			case B_K_ADD:
				KPK_Client.SendText(KPK_C.CONTACT_ADD, m_ContactName.GetText());
				m_ContactName.SetText("");
				break;
			case B_K_DEL:
			{
				KPK_ContactView dc = SelectedContact();
				if (dc)
				{
					KPK_Client.SendText(KPK_C.CONTACT_DEL, dc.Callsign);
					m_ContactList.m_Selected = -1;
				}
				else
					SetStatus("Выберите контакт в списке");
				break;
			}
			case B_K_WRITE:
			{
				KPK_ContactView wc = SelectedContact();
				if (!wc)
				{
					SetStatus("Выберите контакт в списке");
					break;
				}
				m_ChatTo = wc.Callsign;
				m_ChatOffset = 0;
				ShowPage(P_CHAT);
				SetFocus(m_ChatEdit);
				break;
			}
			case B_K_PARTY:
			{
				KPK_ContactView pc = SelectedContact();
				if (pc)
					KPK_Client.SendText(KPK_C.PARTY_INVITE, pc.Callsign);
				else
					SetStatus("Выберите контакт в списке");
				break;
			}
		}
	}
};

// metro data for the admin tab (copied from the Drezina mod, which may be missing)
class KPK_MetroLine
{
	int LineId;
	bool InZone;
	float LenM;
	vector EndA;
	vector EndB;
	int JoinToA;
	int JoinToB;
	ref array<vector> Axis = new array<vector>();
}

class KPK_MetroCross
{
	int LineA;
	int LineB;
	vector Spot;
	bool IsTee;
	bool IsRing;
}
