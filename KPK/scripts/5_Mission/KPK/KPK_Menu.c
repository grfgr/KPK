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
	static const int P_LOGIN = 6;
	static const int P_BOOT = 7;
	static const int TAB_COUNT = 6;

	// button ids
	static const int B_TAB = 1;          // 1..6
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
	protected TextWidget m_StatusRight;
	protected MultilineTextWidget m_SideInfo;
	protected float m_SessionTime;
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
	protected PasswordEditBoxWidget m_LoginPassword;
	protected TextWidget m_LoginRepeatLabel;
	protected Widget m_LoginRepeatBox;
	protected PasswordEditBoxWidget m_LoginRepeat;
	protected TextWidget m_LoginSteam;
	protected MultilineTextWidget m_LoginHint;
	protected ref KPK_Button m_LoginButton;

	// profile
	protected MultilineTextWidget m_ProfileText;
	protected MultilineTextWidget m_VitalsText;
	protected MultilineTextWidget m_RepText;

	// map
	protected MapWidget m_Map;
	protected ref KPK_MapHandler m_MapHandler;
	protected ref KPK_List m_MarkerList;
	protected EditBoxWidget m_MarkerName;
	protected TextWidget m_MarkerType;
	protected TextWidget m_MarkerScope;
	protected TextWidget m_MapInfo;
	protected int m_NewType;
	protected int m_NewScope;
	protected ref array<ref KPK_Marker> m_MarkerRows;

	// party
	protected ref KPK_List m_PartyList;
	protected ref KPK_List m_InviteList;
	protected EditBoxWidget m_PartyInviteName;
	protected TextWidget m_PartyInfo;
	protected ref KPK_Button m_LabelsButton;
	protected ref array<ref KPK_InviteView> m_InviteRows;

	// factions
	protected ref KPK_List m_FactionList;
	protected ref KPK_List m_MemberList;
	protected MultilineTextWidget m_FactionText;
	protected EditBoxWidget m_FactionInviteName;
	protected Widget m_FactionManage;
	protected Widget m_AdminBox;
	protected EditBoxWidget m_AdminText;
	protected TextWidget m_FactionHint;
	protected ref map<int, ref KPK_Button> m_ById;
	protected string m_ConfirmRemove = "";

	// tasks
	protected ref KPK_List m_TaskList;
	protected MultilineTextWidget m_TaskText;

	// notes
	protected ref KPK_List m_NoteList;
	protected EditBoxWidget m_NoteTitle;
	protected MultilineEditBoxWidget m_NoteText;
	protected TextWidget m_NoteInfo;
	protected int m_NoteId;            // 0 = new note
	protected bool m_SelectNewestNote;
	protected int m_NoteShown = -2;

	// =================================================================== open / close

	static bool IsOpen()
	{
		return s_Instance && s_Instance.m_Visible;
	}

	static void Open()
	{
		if (IsOpen())
			return;
		s_Instance = new KPK_Menu();
		GetGame().GetUIManager().ShowScriptedMenu(s_Instance, null);
	}

	// the same key press that closed the KPK must not open it again
	static bool JustClosed()
	{
		return GetGame().GetTime() - s_ClosedAt < 400;
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

	void ~KPK_Menu()
	{
		KPK_Client.s_OnView.Remove(OnView);
		KPK_Client.s_OnStatus.Remove(OnStatus);
		KPK_Client.s_OnMates.Remove(OnMates);
		if (layoutRoot)
			layoutRoot.Unlink();
	}

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets(KPK_UI.DIR + "kpk_main.layout");
		m_Screen = layoutRoot.FindAnyWidget("Screen");
		m_Skin = new KPK_Skin();
		m_Skin.BuildDevice(layoutRoot.FindAnyWidget("Device"));

		BuildChrome();
		m_Pages.Insert(BuildProfile());
		m_Pages.Insert(BuildMap());
		m_Pages.Insert(BuildParty());
		m_Pages.Insert(BuildFactions());
		m_Pages.Insert(BuildTasks());
		m_Pages.Insert(BuildNotes());
		m_Pages.Insert(BuildLogin());
		m_Pages.Insert(BuildBoot());

		// CRT effects go last so they lie on top of every page
		m_Skin.BuildScreenFx(m_Screen);

		KPK_Client.s_OnView.Insert(OnView);
		KPK_Client.s_OnStatus.Insert(OnStatus);
		KPK_Client.s_OnMates.Insert(OnMates);

		StartBoot();
		return layoutRoot;
	}

	override void OnShow()
	{
		super.OnShow();
		m_Visible = true;
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
		GetGame().GetMission().RemoveActiveInputExcludes({"menu"}, true);
		GetGame().GetInput().ResetGameFocus();
		GetGame().GetUIManager().ShowUICursor(false);
		if (GetGame().GetMission().GetHud())
			GetGame().GetMission().GetHud().Show(true);
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

	static string PageName(int page)
	{
		switch (page)
		{
			case P_PROFILE: return "Профиль";
			case P_MAP: return "Карта";
			case P_PARTY: return "Группа";
			case P_FACTIONS: return "Группировки";
			case P_TASKS: return "Задания";
			case P_NOTES: return "Заметки";
			case P_LOGIN: return "Вход в сеть";
		}
		return "Загрузка";
	}

	// an old program window: title bar, menu bar, sidebar with modules, segmented status bar
	protected void BuildChrome()
	{
		// title bar
		KPK_UI.Panel(m_Screen, 0, 0, 1, 0.045, 0xFF15301A);
		KPK_UI.Panel(m_Screen, 0, 0, 1, 0.004, 0x40C8FFC0);
		KPK_UI.Panel(m_Screen, 0.008, 0.012, 0.012, 0.022, KPK_UI.C_ACCENT);
		m_HeaderLeft = KPK_UI.Text(m_Screen, "", 0.026, 0, 0.7, 0.045, 15, 0xFFD8E8C8);
		KPK_UI.TextC(m_Screen, "_", 0.885, 0.004, 0.03, 0.037, KPK_UI.C_DIM);
		KPK_UI.TextC(m_Screen, "[]", 0.917, 0.004, 0.03, 0.037, KPK_UI.C_DIM);
		AddButton(m_Screen, B_CLOSE, "X", 0.952, 0.005, 0.04, 0.035);

		// menu bar
		KPK_UI.Panel(m_Screen, 0, 0.045, 1, 0.035, 0xFF0C1C0F);
		KPK_UI.Text(m_Screen, "Файл     Сеть     Карта     Сервис     Справка", 0.015, 0.045, 0.6, 0.035, 13, KPK_UI.C_DIM);
		m_HeaderRight = KPK_UI.Text(m_Screen, "", 0.62, 0.045, 0.37, 0.035, 13, KPK_UI.C_TEXT);
		KPK_UI.Panel(m_Screen, 0, 0.08, 1, 0.003, KPK_UI.C_EDGE);

		// sidebar
		KPK_UI.Panel(m_Screen, 0, 0.083, 0.165, 0.852, 0xFF081509);
		KPK_UI.Text(m_Screen, "МОДУЛИ", 0.012, 0.09, 0.15, 0.03, 13, KPK_UI.C_ACCENT);
		string names[6] = {"[1] ПРОФИЛЬ", "[2] КАРТА", "[3] ГРУППА", "[4] ГРУППИРОВКИ", "[5] ЗАДАНИЯ", "[6] ЗАМЕТКИ"};
		for (int i = 0; i < TAB_COUNT; i++)
		{
			KPK_Button tab = new KPK_Button(m_Screen, B_TAB + i, names[i], 0.008, 0.125 + i * 0.065, 0.15, 0.056, true);
			m_Tabs.Insert(tab);
			m_Buttons.Insert(tab);
		}
		KPK_UI.Panel(m_Screen, 0.012, 0.73, 0.14, 0.002, KPK_UI.C_LINE);
		m_SideInfo = KPK_UI.MText(m_Screen, 0.012, 0.74, 0.15, 0.19, 12, KPK_UI.C_DIM);
		KPK_UI.Panel(m_Screen, 0.165, 0.083, 0.002, 0.852, KPK_UI.C_LINE);

		// status bar
		KPK_UI.Panel(m_Screen, 0, 0.935, 1, 0.065, 0xFF0B1A0D);
		KPK_UI.Panel(m_Screen, 0, 0.935, 1, 0.003, KPK_UI.C_EDGE);
		m_Footer = KPK_UI.Text(m_Screen, "", 0.012, 0.94, 0.6, 0.055, 14, KPK_UI.C_TEXT);
		KPK_UI.Panel(m_Screen, 0.62, 0.945, 0.002, 0.045, KPK_UI.C_LINE);
		m_StatusRight = KPK_UI.Text(m_Screen, "", 0.632, 0.94, 0.36, 0.055, 13, KPK_UI.C_DIM);
	}

	protected Widget Content()
	{
		return KPK_UI.Frame(m_Screen, 0.178, 0.095, 0.812, 0.83);
	}

	protected void ShowTabs(bool show)
	{
		foreach (KPK_Button tab : m_Tabs)
			tab.Show(show);
	}

	protected void ShowPage(int page)
	{
		m_Page = page;
		for (int i = 0; i < m_Pages.Count(); i++)
			m_Pages[i].Show(i == page);
		for (int t = 0; t < m_Tabs.Count(); t++)
			m_Tabs[t].SetActive(t == page);
		if (page < TAB_COUNT)
			m_LastTab = page;
		// the map widget only exists while its page is shown: a hidden MapWidget
		// sometimes bleeds through other widgets (map tiles inside text fields)
		if (page == P_MAP)
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

		string text = "КПК-7 «ВЕСТНИК»  ОС v2.11\n\n";
		if (m_BootTime > 0.15) text += "> Проверка питания ........ OK\n";
		if (m_BootTime > 0.35) text += "> Поиск сети .............. " + net + "\n";
		if (m_BootTime > 0.55) text += "> Спутник GPS ............. захвачен\n";
		if (m_BootTime > 0.75) text += "> Шифрование канала ....... ГОСТ-28147\n";
		if (m_BootTime > 0.95) text += "> Загрузка профиля ........ OK\n";
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
		m_LoginCallsign = KPK_UI.Edit(page, 0.35, 0.14, 0.6, 0.07);
		m_LoginCallsignFixed = KPK_UI.Text(page, "", 0.35, 0.14, 0.6, 0.07, 17, KPK_UI.C_WHITE);

		KPK_UI.Text(page, "Пароль", 0.05, 0.25, 0.3, 0.07, 15, KPK_UI.C_DIM);
		m_LoginPassword = KPK_UI.Password(page, 0.35, 0.25, 0.6, 0.07);

		m_LoginRepeatLabel = KPK_UI.Text(page, "Повтор пароля", 0.05, 0.36, 0.3, 0.07, 15, KPK_UI.C_DIM);
		m_LoginRepeatBox = KPK_UI.Frame(page, 0.35, 0.36, 0.6, 0.07);
		m_LoginRepeat = KPK_UI.Password(m_LoginRepeatBox, 0, 0, 1, 1);

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

		string password = m_LoginPassword.GetText();
		if (!v.Registered || v.NeedNewPassword)
		{
			if (password != m_LoginRepeat.GetText())
			{
				SetStatus("Пароли не совпадают");
				return;
			}
		}

		if (!v.Registered)
			KPK_Client.SendText(KPK_C.REGISTER, m_LoginCallsign.GetText(), password);
		else
			KPK_Client.SendText(KPK_C.LOGIN, password);

		SetStatus("Обмен данными с сервером...");
	}

	// =================================================================== profile

	protected Widget BuildProfile()
	{
		Widget page = Content();
		Widget left = KPK_UI.Card(page, 0, 0, 0.49, 1, "ДОСЬЕ СТАЛКЕРА");
		m_ProfileText = KPK_UI.MText(left, 0.04, 0.08, 0.92, 0.9, 15, KPK_UI.C_TEXT);

		Widget right = KPK_UI.Card(page, 0.51, 0, 0.49, 0.47, "СОСТОЯНИЕ ОРГАНИЗМА");
		m_VitalsText = KPK_UI.MText(right, 0.04, 0.16, 0.92, 0.82, 15, KPK_UI.C_TEXT);

		Widget rep = KPK_UI.Card(page, 0.51, 0.5, 0.49, 0.5, "РЕПУТАЦИЯ У ГРУППИРОВОК");
		m_RepText = KPK_UI.MText(rep, 0.04, 0.16, 0.92, 0.82, 15, KPK_UI.C_TEXT);
		return page;
	}

	protected void RefreshProfile()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;
		KPK_Stats s = v.Stats;

		string faction = "вольный сталкер";
		KPK_FactionView mine = MyFaction();
		if (mine)
			faction = mine.Name + " (" + KPK_Const.RoleName(v.MyRole) + ")";

		string text = "";
		text += "Позывной:  " + v.Callsign + "\n";
		text += "Ранг:  " + v.Rank + "\n";
		text += "Группировка:  " + faction + "\n";
		text += "Steam ID:  " + v.SteamId + "\n";
		text += "В сети с:  " + v.RegisteredDate + "\n\n";
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
		Widget page = Content();

		Widget mapBox = KPK_UI.Panel(page, 0, 0, 0.71, 1, 0xFF000000, true);
		m_MapHolder = KPK_UI.Frame(mapBox, 0.004, 0.004, 0.992, 0.992);
		// grid of a field map over the paper
		for (int gl = 1; gl < 10; gl++)
		{
			KPK_UI.Panel(mapBox, gl * 0.1, 0, 0.0015, 1, 0x1890FF90);
			KPK_UI.Panel(mapBox, 0, gl * 0.1, 1, 0.002, 0x1890FF90);
		}
		// green phosphor tint over the paper map
		KPK_UI.Panel(mapBox, 0, 0, 1, 1, 0x2230FF50);
		KPK_UI.Panel(mapBox, 0, 0, 1, 0.004, KPK_UI.C_EDGE);
		KPK_UI.Panel(mapBox, 0, 0.996, 1, 0.004, KPK_UI.C_EDGE);
		m_MapInfo = KPK_UI.Text(mapBox, "", 0.02, 0.94, 0.96, 0.05, 13, KPK_UI.C_ACCENT);

		Widget side = KPK_UI.Card(page, 0.725, 0, 0.275, 1, "МЕТКИ КПК");
		m_MarkerList = new KPK_List(side, 0.04, 0.06, 0.92, 0.44, 8, "— меток нет —");
		m_Lists.Insert(m_MarkerList);

		AddButton(side, B_MAP_GOTO, "НА МЕТКУ", 0.04, 0.505, 0.45, 0.04);
		AddButton(side, B_MAP_DEL, "УДАЛИТЬ", 0.51, 0.505, 0.45, 0.04);

		KPK_UI.Panel(side, 0.04, 0.56, 0.92, 0.002, KPK_UI.C_LINE);
		KPK_UI.Text(side, "НОВАЯ МЕТКА", 0.04, 0.565, 0.92, 0.04, 14, KPK_UI.C_ACCENT);
		KPK_UI.Text(side, "Название", 0.04, 0.605, 0.92, 0.035, 13, KPK_UI.C_DIM);
		m_MarkerName = KPK_UI.Edit(side, 0.04, 0.64, 0.92, 0.045);

		AddButton(side, B_TYPE_PREV, "<", 0.04, 0.7, 0.14, 0.045);
		m_MarkerType = KPK_UI.Text(side, "", 0.19, 0.7, 0.62, 0.045, 14, KPK_UI.C_WHITE);
		AddButton(side, B_TYPE_NEXT, ">", 0.82, 0.7, 0.14, 0.045);

		AddButton(side, B_SCOPE_PREV, "<", 0.04, 0.755, 0.14, 0.045);
		m_MarkerScope = KPK_UI.Text(side, "", 0.19, 0.755, 0.62, 0.045, 14, KPK_UI.C_WHITE);
		AddButton(side, B_SCOPE_NEXT, ">", 0.82, 0.755, 0.14, 0.045);

		AddButton(side, B_MAP_HERE, "ПОСТАВИТЬ ЗДЕСЬ", 0.04, 0.81, 0.92, 0.045);
		MultilineTextWidget hint = KPK_UI.MText(side, 0.04, 0.865, 0.92, 0.075, 12, KPK_UI.C_DIM);
		hint.SetText("Двойной клик по карте ставит метку в точку. Метки видны только в КПК.");
		AddButton(side, B_MAP_SELF, "МОЯ ПОЗИЦИЯ", 0.04, 0.945, 0.92, 0.045);

		UpdateMarkerEditor();
		return page;
	}

	protected void UpdateMarkerEditor()
	{
		m_MarkerType.SetText(KPK_Const.MarkerName(m_NewType));
		m_MarkerType.SetColor(KPK_Const.MarkerColor(m_NewType));
		m_MarkerScope.SetText("Доступ: " + KPK_Const.ScopeName(m_NewScope));
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
		RedrawMapMarks();
	}

	protected void RedrawMapMarks()
	{
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
		if (!m_Map)
			return;
		vector pos = m_Map.ScreenToMap(Vector(x, y, 0));
		AddMarker(pos[0], pos[2]);
	}

	protected void AddMarker(float x, float z)
	{
		KPK_Req req = new KPK_Req();
		req.S1 = m_MarkerName.GetText();
		req.I1 = m_NewType;
		req.I2 = m_NewScope;
		req.F1 = x;
		req.F2 = z;
		KPK_Client.Send(KPK_C.MARKER_ADD, req);
		m_MarkerName.SetText("");
		SetStatus("Метка передана: квадрат " + KPK_UI.Grid(x, z));
	}

	protected KPK_Marker SelectedMarker()
	{
		int i = m_MarkerList.Selected();
		if (i < 0 || i >= m_MarkerRows.Count())
			return null;
		return m_MarkerRows[i];
	}

	// =================================================================== party

	protected Widget BuildParty()
	{
		Widget page = Content();

		Widget left = KPK_UI.Card(page, 0, 0, 0.49, 1, "ГРУППА");
		m_PartyInfo = KPK_UI.Text(left, "", 0.04, 0.07, 0.92, 0.05, 14, KPK_UI.C_DIM);
		m_PartyList = new KPK_List(left, 0.04, 0.13, 0.92, 0.55, 8, "— вы не в группе —");
		m_Lists.Insert(m_PartyList);
		AddButton(left, B_P_KICK, "ИСКЛЮЧИТЬ", 0.04, 0.71, 0.45, 0.06);
		AddButton(left, B_P_LEAVE, "ПОКИНУТЬ ГРУППУ", 0.51, 0.71, 0.45, 0.06);

		KPK_UI.Panel(left, 0.04, 0.8, 0.92, 0.002, KPK_UI.C_LINE);
		KPK_UI.Text(left, "Пригласить по позывному", 0.04, 0.81, 0.92, 0.05, 13, KPK_UI.C_DIM);
		m_PartyInviteName = KPK_UI.Edit(left, 0.04, 0.865, 0.6, 0.06);
		AddButton(left, B_P_INVITE, "ПРИГЛАСИТЬ", 0.66, 0.865, 0.3, 0.06);

		Widget right = KPK_UI.Card(page, 0.51, 0, 0.49, 0.62, "ВХОДЯЩИЕ ПРИГЛАШЕНИЯ");
		m_InviteList = new KPK_List(right, 0.04, 0.1, 0.92, 0.66, 6, "— приглашений нет —");
		m_Lists.Insert(m_InviteList);
		AddButton(right, B_INV_ACCEPT, "ПРИНЯТЬ", 0.04, 0.82, 0.45, 0.1);
		AddButton(right, B_INV_DECLINE, "ОТКЛОНИТЬ", 0.51, 0.82, 0.45, 0.1);

		Widget opts = KPK_UI.Card(page, 0.51, 0.65, 0.49, 0.35, "ДАТЧИКИ");
		m_LabelsButton = AddButton(opts, B_LABELS, "", 0.04, 0.2, 0.92, 0.18);
		MultilineTextWidget hint = KPK_UI.MText(opts, 0.04, 0.45, 0.92, 0.5, 13, KPK_UI.C_DIM);
		hint.SetText("Метки над головами показывают позывные и дистанцию до членов группы (и группировки, если она это разрешает). Горячая клавиша: Ctrl + K.");
		return page;
	}

	protected void RefreshParty()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;

		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		foreach (KPK_PartyMemberView m : v.Party)
		{
			string line = m.Callsign;
			if (m.Leader)
				line = "[Л] " + line;
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
			else
			{
				inv.Insert("Группировка «" + iv.FactionName + "» (" + iv.From + ")");
				invColors.Insert(KPK_UI.C_ACCENT);
			}
		}
		m_InviteList.Set(inv, invColors);

		if (KPK_Client.s_LabelsEnabled)
			m_LabelsButton.SetText("МЕТКИ В МИРЕ: ВКЛ");
		else
			m_LabelsButton.SetText("МЕТКИ В МИРЕ: ВЫКЛ");
		m_LabelsButton.SetActive(KPK_Client.s_LabelsEnabled);
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
		else
			KPK_Client.SendText(KPK_C.FACTION_RESPOND, iv.FactionId, "", answer);
	}

	// =================================================================== factions

	protected Widget BuildFactions()
	{
		Widget page = Content();

		Widget left = KPK_UI.Card(page, 0, 0, 0.27, 1, "ГРУППИРОВКИ ЗОНЫ");
		m_FactionList = new KPK_List(left, 0.05, 0.06, 0.9, 0.6, 10, "— нет данных —");
		m_Lists.Insert(m_FactionList);
		m_FactionHint = KPK_UI.Text(left, "", 0.05, 0.68, 0.9, 0.04, 12, KPK_UI.C_DIM);

		Widget info = KPK_UI.Card(page, 0.285, 0, 0.415, 0.58, "ДОСЬЕ");
		m_FactionText = KPK_UI.MText(info, 0.04, 0.08, 0.92, 0.9, 14, KPK_UI.C_TEXT);

		Widget members = KPK_UI.Card(page, 0.715, 0, 0.285, 0.58, "СОСТАВ");
		m_MemberList = new KPK_List(members, 0.05, 0.08, 0.9, 0.9, 8, "— данные недоступны —");
		m_Lists.Insert(m_MemberList);

		// management: leader / deputy of the selected (own) faction, or an admin
		m_FactionManage = KPK_UI.Card(page, 0.285, 0.6, 0.715, 0.17, "УПРАВЛЕНИЕ");
		AddButton(m_FactionManage, B_F_KICK, "ИСКЛЮЧИТЬ", 0.02, 0.26, 0.15, 0.3);
		AddButton(m_FactionManage, B_F_MEMBER, "В БОЙЦЫ", 0.18, 0.26, 0.15, 0.3);
		AddButton(m_FactionManage, B_F_DEPUTY, "В ЗАМЫ", 0.34, 0.26, 0.15, 0.3);
		AddButton(m_FactionManage, B_F_LEADER, "СДЕЛАТЬ ЛИДЕРОМ", 0.5, 0.26, 0.23, 0.3);
		AddButton(m_FactionManage, B_F_MAP, "НА КАРТЕ", 0.74, 0.26, 0.24, 0.3);
		m_FactionInviteName = KPK_UI.Edit(m_FactionManage, 0.02, 0.62, 0.47, 0.3);
		AddButton(m_FactionManage, B_F_INVITE, "ПРИНЯТЬ В РЯДЫ", 0.5, 0.62, 0.23, 0.3);
		AddButton(m_FactionManage, B_F_LEAVE, "ПОКИНУТЬ", 0.74, 0.62, 0.24, 0.3);

		m_AdminBox = KPK_UI.Card(page, 0, 0.79, 1, 0.21, "АДМИНИСТРАТОР: выбранная группировка");
		float bw = 0.118;
		float gap = 0.006;
		AddButton(m_AdminBox, B_A_ENC_DESC, "ШИФР: ОПИСАНИЕ", 0.01, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_ENC_LEADER, "ШИФР: ЛИДЕР", 0.01 + (bw + gap), 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_ENC_MEMBERS, "ШИФР: СОСТАВ", 0.01 + (bw + gap) * 2, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_ENC_LOC, "ШИФР: МЕСТО", 0.01 + (bw + gap) * 3, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_HIDDEN, "СКРЫТА", 0.01 + (bw + gap) * 4, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_MATES, "ВИДНЫ В МИРЕ", 0.01 + (bw + gap) * 5, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_COLOR, "ЦВЕТ", 0.01 + (bw + gap) * 6, 0.2, bw, 0.22);
		AddButton(m_AdminBox, B_A_REMOVE, "УДАЛИТЬ", 0.01 + (bw + gap) * 7, 0.2, bw, 0.22);

		m_AdminText = KPK_UI.Edit(m_AdminBox, 0.01, 0.48, 0.358, 0.22);
		AddButton(m_AdminBox, B_A_LOC_TEXT, "МЕСТО = ТЕКСТ", 0.01 + (bw + gap) * 3, 0.48, bw, 0.22);
		AddButton(m_AdminBox, B_A_LOC_ME, "МЕСТО = Я ТУТ", 0.01 + (bw + gap) * 4, 0.48, bw, 0.22);
		AddButton(m_AdminBox, B_A_LOC_CLEAR, "УБРАТЬ С КАРТЫ", 0.01 + (bw + gap) * 5, 0.48, bw, 0.22);
		AddButton(m_AdminBox, B_A_DESC, "ОПИСАНИЕ = ТЕКСТ", 0.01 + (bw + gap) * 6, 0.48, bw * 2 + gap, 0.22);

		AddButton(m_AdminBox, B_A_LEADER, "ЛИДЕР = ПОЗЫВНОЙ", 0.01, 0.75, bw * 1.5, 0.22);
		AddButton(m_AdminBox, B_A_ADD, "ЗАЧИСЛИТЬ ПОЗЫВНОЙ", 0.01 + bw * 1.5 + gap, 0.75, bw * 1.5, 0.22);
		AddButton(m_AdminBox, B_A_CREATE, "СОЗДАТЬ ГРУППИРОВКУ (ТЕКСТ = НАЗВАНИЕ)", 0.01 + (bw * 1.5 + gap) * 2, 0.75, bw * 3, 0.22);
		MultilineTextWidget ah = KPK_UI.MText(m_AdminBox, 0.01 + (bw * 1.5 + gap) * 2 + bw * 3 + gap, 0.72, 0.99 - (0.01 + (bw * 1.5 + gap) * 2 + bw * 3 + gap), 0.28, 11, KPK_UI.C_DIM);
		ah.SetText("Поле ввода слева используется для текста, позывного и названия.");
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
		if (!v)
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
			return "[ЗАШИФРОВАНО]  " + value;
		return value;
	}

	protected void RefreshFactions()
	{
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
			deputies = "[ЗАШИФРОВАНО]  " + KPK_Util.Scramble(f.Id + "dep", 10);
		else if (deputies == "")
			deputies = "—";

		string count = "[ЗАШИФРОВАНО]";
		if (!f.MembersEnc)
			count = f.MemberCount.ToString();

		string text = "";
		text += f.Name + "\n\n";
		text += "Лидер:  " + EncField(f.Leader, f.LeaderEnc) + "\n";
		text += "Заместители:  " + deputies + "\n";
		text += "Численность:  " + count + "\n";
		text += "Предположительное местоположение:\n   " + EncField(f.Location, f.LocationEnc) + "\n\n";
		text += EncField(f.Description, f.DescriptionEnc) + "\n";
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
		bool isLeader = v.IsAdmin || (mine && v.MyRole == KPK_Role.LEADER);
		m_FactionManage.Show(mine || v.IsAdmin || f.HasLocationPos);
		m_ById.Get(B_F_KICK).SetEnabled(canManage);
		m_ById.Get(B_F_MEMBER).SetEnabled(isLeader);
		m_ById.Get(B_F_DEPUTY).SetEnabled(isLeader);
		m_ById.Get(B_F_LEADER).SetEnabled(isLeader);
		m_ById.Get(B_F_INVITE).SetEnabled(mine && v.MyRole >= KPK_Role.DEPUTY);
		m_ById.Get(B_F_LEAVE).SetEnabled(mine);
		m_ById.Get(B_F_MAP).SetEnabled(f.HasLocationPos && !f.LocationEnc);

		m_AdminBox.Show(v.IsAdmin);
		if (v.IsAdmin)
		{
			m_ById.Get(B_A_ENC_DESC).SetActive(f.FlagDescription);
			m_ById.Get(B_A_ENC_LEADER).SetActive(f.FlagLeader);
			m_ById.Get(B_A_ENC_MEMBERS).SetActive(f.FlagMembers);
			m_ById.Get(B_A_ENC_LOC).SetActive(f.FlagLocation);
			m_ById.Get(B_A_HIDDEN).SetActive(f.Hidden);
			m_ById.Get(B_A_MATES).SetActive(f.ShowMatesInWorld);
			m_ById.Get(B_A_COLOR).SetTextColor(f.Color);
			if (m_ConfirmRemove == f.Id)
				m_ById.Get(B_A_REMOVE).SetText("ТОЧНО УДАЛИТЬ?");
			else
				m_ById.Get(B_A_REMOVE).SetText("УДАЛИТЬ");
		}
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
		Widget page = Content();
		Widget left = KPK_UI.Card(page, 0, 0, 0.38, 1, "ЗАДАНИЯ");
		m_TaskList = new KPK_List(left, 0.04, 0.06, 0.92, 0.9, 12, "— заданий нет —");
		m_Lists.Insert(m_TaskList);

		Widget right = KPK_UI.Card(page, 0.4, 0, 0.6, 1, "ПОДРОБНОСТИ");
		m_TaskText = KPK_UI.MText(right, 0.04, 0.07, 0.92, 0.9, 15, KPK_UI.C_TEXT);
		return page;
	}

	protected string TaskStatus(int status)
	{
		if (status == 1) return "выполнено";
		if (status == 2) return "провалено";
		return "активно";
	}

	protected void RefreshTasks()
	{
		KPK_View v = KPK_Client.s_View;
		if (!v)
			return;

		array<string> names = new array<string>();
		array<int> colors = new array<int>();
		foreach (KPK_Task t : v.Tasks)
		{
			names.Insert(t.Title);
			if (t.Status == 1)
				colors.Insert(KPK_UI.C_DIM);
			else if (t.Status == 2)
				colors.Insert(KPK_UI.C_DANGER);
			else
				colors.Insert(KPK_UI.C_ACCENT);
		}
		m_TaskList.Set(names, colors);

		int i = m_TaskList.Selected();
		if (i < 0 || i >= v.Tasks.Count())
		{
			if (v.Tasks.Count() == 0)
				m_TaskText.SetText("Активных заданий нет.\n\nЗадания выдают торговцы, группировки и другие сталкеры. Когда вам поручат работу, она появится здесь.\n\n[ модуль заданий в разработке ]");
			else
				m_TaskText.SetText("Выберите задание из списка.");
			return;
		}

		KPK_Task task = v.Tasks[i];
		string text = task.Title + "\n\n";
		text += "Заказчик:  " + task.Issuer + "\n";
		text += "Награда:  " + task.Reward + "\n";
		text += "Статус:  " + TaskStatus(task.Status) + "\n";
		text += "Получено:  " + task.Date + "\n\n";
		text += task.Description;
		m_TaskText.SetText(text);
	}

	// =================================================================== notes

	protected Widget BuildNotes()
	{
		Widget page = Content();
		Widget left = KPK_UI.Card(page, 0, 0, 0.32, 1, "ЗАПИСИ");
		m_NoteList = new KPK_List(left, 0.04, 0.06, 0.92, 0.8, 12, "— записей нет —");
		m_Lists.Insert(m_NoteList);
		AddButton(left, B_N_NEW, "НОВАЯ", 0.04, 0.9, 0.45, 0.06);
		AddButton(left, B_N_DEL, "УДАЛИТЬ", 0.51, 0.9, 0.45, 0.06);

		Widget right = KPK_UI.Card(page, 0.34, 0, 0.66, 1, "ЗАМЕТКА");
		KPK_UI.Text(right, "Заголовок", 0.03, 0.06, 0.3, 0.04, 13, KPK_UI.C_DIM);
		m_NoteTitle = KPK_UI.Edit(right, 0.03, 0.1, 0.94, 0.055);
		m_NoteText = KPK_UI.MEdit(right, 0.03, 0.18, 0.94, 0.7);
		m_NoteInfo = KPK_UI.Text(right, "", 0.03, 0.9, 0.6, 0.06, 13, KPK_UI.C_DIM);
		AddButton(right, B_N_SAVE, "СОХРАНИТЬ", 0.67, 0.9, 0.3, 0.06);
		return page;
	}

	protected void RefreshNotes()
	{
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
		m_NoteText.SetText(note.Text);
		m_NoteInfo.SetText("Изменено: " + note.Date);
	}

	protected void SaveNote()
	{
		string text;
		m_NoteText.GetText(text);
		KPK_Req req = new KPK_Req();
		req.I1 = m_NoteId;
		req.S1 = m_NoteTitle.GetText();
		req.S2 = text;
		if (m_NoteId == 0)
			m_SelectNewestNote = true;
		KPK_Client.Send(KPK_C.NOTE_SAVE, req);
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
			ShowPage(m_LastTab);
		}
		else
		{
			RefreshPage();
		}
		RefreshHeader();
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
			case P_PROFILE: RefreshProfile(); break;
			case P_MAP: RefreshMap(); break;
			case P_PARTY: RefreshParty(); break;
			case P_FACTIONS: RefreshFactions(); break;
			case P_TASKS: RefreshTasks(); break;
			case P_NOTES: RefreshNotes(); break;
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
			m_Skin.m_Serial.SetText("S/N " + v.SteamId);
		}
		m_Skin.SetLed(low);
		m_HeaderLeft.SetText("ВЕСТНИК ОС 2.11  —  " + PageName(m_Page));

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
		m_Footer.SetText("> " + m_Status + cursor);
	}

	// =================================================================== input

	override void Update(float timeslice)
	{
		super.Update(timeslice);

		if (m_Booting)
		{
			m_BootTime += timeslice;
			UpdateBoot();
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
			RefreshHeader();
		}

		if (m_Page == P_MAP)
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

	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		// Enter in the password box submits the login form
		if (finished && (w == m_LoginPassword || w == m_LoginRepeat) && m_Page == P_LOGIN)
		{
			SubmitLogin();
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

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button != MouseState.LEFT)
			return super.OnClick(w, x, y, button);

		// lists first: rows and pagers
		foreach (KPK_List list : m_Lists)
		{
			if (!list.Click(w))
				continue;
			OnListClicked(list);
			return true;
		}

		foreach (KPK_Button b : m_Buttons)
		{
			if (b.m_Root != w)
				continue;
			if (b.m_Enabled)
				OnButton(b.m_Id);
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
		else if (list == m_TaskList)
		{
			RefreshTasks();
		}
		else if (list == m_NoteList)
		{
			ShowSelectedNote(false);
		}
		else if (list == m_MarkerList)
		{
			KPK_Marker mk = SelectedMarker();
			if (mk && m_Map)
				m_Map.SetMapPos(Vector(mk.X, 0, mk.Z));
		}
	}

	protected void OnButton(int id)
	{
		if (id >= B_TAB && id < B_TAB + TAB_COUNT)
		{
			ShowPage(id - B_TAB);
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
				m_NewScope = (m_NewScope + 2) % 3;
				UpdateMarkerEditor();
				break;
			case B_SCOPE_NEXT:
				m_NewScope = (m_NewScope + 1) % 3;
				UpdateMarkerEditor();
				break;
			case B_MAP_HERE:
			{
				if (!player)
					break;
				vector here = player.GetPosition();
				AddMarker(here[0], here[2]);
				break;
			}
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
					KPK_Client.SendText(KPK_C.MARKER_DEL, "", "", dm.Id, dm.Scope);
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
			case B_A_LEADER: Admin(KPK_AdminOp.SET_LEADER, m_AdminText.GetText()); break;
			case B_A_ADD: Admin(KPK_AdminOp.ADD_MEMBER, m_AdminText.GetText()); break;
			case B_A_CREATE: Admin(KPK_AdminOp.CREATE, m_AdminText.GetText()); break;
			case B_A_REMOVE:
			{
				if (!f)
					break;
				if (m_ConfirmRemove == f.Id)
				{
					m_ConfirmRemove = "";
					m_FactionList.m_Selected = -1;
					Admin(KPK_AdminOp.REMOVE);
				}
				else
				{
					m_ConfirmRemove = f.Id;
					RefreshFactionDetails();
				}
				break;
			}

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
		}
	}
};
