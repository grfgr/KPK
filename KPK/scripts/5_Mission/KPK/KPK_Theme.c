// Look and wording of each KPK model. The layout is the same everywhere; colours, names,
// boot screen, greeting, case markings and the amount of interference differ.
class KPK_Theme
{
	int Model;

	// interface palette
	int Text;
	int Dim;
	int Accent;
	int Danger;
	int White;
	int Btn;
	int BtnHover;
	int BtnActive;
	int BtnOff;
	int Row;
	int RowHover;
	int RowSel;
	int Card;
	int Edge;
	int Line;
	int Screen;
	int TitleBar;
	int MenuBar;
	int Side;
	int Title;
	int Tint;
	int Grid;

	// case
	int Case;
	int Bezel;
	int Grip;
	int BrandColor;
	string Brand;
	string Maker;
	string Sticker;

	// wording
	string OsName;
	string MenuLine;
	ref array<string> Tabs;
	ref array<string> Boot;      // %NET% is replaced by the network name
	string Welcome;              // %1 = callsign
	bool Glitch;                 // Monolith: heavy interference, almost no plain text

	// dossier
	string ProfileTitle;
	string RankLabel;
	string NoFaction;            // shown when the player is in no faction
	ref array<string> Ranks;     // 6 ranks, matching KPK_Const.Rank from the novice up

	void KPK_Theme()
	{
		Tabs = new array<string>();
		Boot = new array<string>();
		Ranks = new array<string>();
	}

	static KPK_Theme Get(int model)
	{
		KPK_Theme t = new KPK_Theme();
		t.Model = model;
		t.Danger = 0xFFD8553F;
		switch (model)
		{
			case KPK_Model.STALKER: t.Stalker(); break;
			case KPK_Model.MILITARY: t.Military(); break;
			case KPK_Model.MONOLITH: t.Monolith(); break;
			case KPK_Model.SCIENCE: t.Science(); break;
			default: t.Brick(); break;
		}
		return t;
	}

	protected void SetRanks(string a, string b, string c, string d, string e, string f)
	{
		Ranks.Clear();
		Ranks.Insert(a); Ranks.Insert(b); Ranks.Insert(c); Ranks.Insert(d); Ranks.Insert(e); Ranks.Insert(f);
	}

	// the server sends the stalker rank; every model names it its own way
	string RankName(string stalkerRank)
	{
		array<string> base = {"Новичок", "Сталкер", "Опытный", "Ветеран", "Мастер", "Легенда Зоны"};
		int i = base.Find(stalkerRank);
		if (i < 0 || i >= Ranks.Count())
			return stalkerRank;
		return Ranks[i];
	}

	protected void SetTabs(string a, string b, string c, string d, string e, string f)
	{
		Tabs.Clear();
		Tabs.Insert(a); Tabs.Insert(b); Tabs.Insert(c); Tabs.Insert(d); Tabs.Insert(e); Tabs.Insert(f);
	}

	// КПК-7 «Вестник»: green phosphor
	protected void Brick()
	{
		Text = 0xFF9EE084; Dim = 0xFF5E8C55; Accent = 0xFFE6AA3C; White = 0xFFE8E8DC;
		Btn = 0xFF172B19; BtnHover = 0xFF27482A; BtnActive = 0xFF3E6B32; BtnOff = 0xFF101410;
		Row = 0xFF0D1A0F; RowHover = 0xFF183019; RowSel = 0xFF2D5228;
		Card = 0xFF0A170C; Edge = 0xFF5A8C4D; Line = 0xFF1F3A22;
		Screen = 0xFF060D08; TitleBar = 0xFF15301A; MenuBar = 0xFF0C1C0F; Side = 0xFF081509; Title = 0xFFD8E8C8;
		Tint = 0x2230FF50; Grid = 0x1890FF90;
		Case = 0xFF2A2C24; Bezel = 0xFF0A0B09; Grip = 0xFF1A1B17; BrandColor = 0xFF8E8A74;
		Brand = "КПК-7  «ВЕСТНИК»"; Maker = "НИИ ЧАЗ  ·  изд. 1986-М"; Sticker = "ИНВ. № 07-0347   НЕ ВСКРЫВАТЬ";
		OsName = "ВЕСТНИК ОС 2.11";
		MenuLine = "Файл     Сеть     Карта     Сервис     Справка";
		SetTabs("ПРОФИЛЬ", "КАРТА", "ГРУППА", "ГРУППИРОВКИ", "ЗАДАНИЯ", "ЗАМЕТКИ");
		Boot.Insert("КПК-7 «ВЕСТНИК»  ОС v2.11");
		Boot.Insert("> Проверка питания ........ OK");
		Boot.Insert("> Поиск сети .............. %NET%");
		Boot.Insert("> Спутник GPS ............. захвачен");
		Boot.Insert("> Режим ................... базовый (карта, группа)");
		Boot.Insert("> Загрузка профиля ........ OK");
		Welcome = "Доступ разрешён. %1, на связи.";
		ProfileTitle = "ДОСЬЕ"; RankLabel = "Ранг"; NoFaction = "сталкер";
		SetRanks("Новичок", "Сталкер", "Опытный", "Ветеран", "Мастер", "Легенда Зоны");
	}

	// КПК-М1: amber / sepia, like the boot screen of the device
	protected void Stalker()
	{
		Text = 0xFFD9B77A; Dim = 0xFF8C744C; Accent = 0xFFF0D8A8; White = 0xFFF2E8D2;
		Btn = 0xFF2A2116; BtnHover = 0xFF3E3120; BtnActive = 0xFF5C4628; BtnOff = 0xFF16120C;
		Row = 0xFF1C160E; RowHover = 0xFF2C2216; RowSel = 0xFF4A3820;
		Card = 0xFF15100A; Edge = 0xFFB08A50; Line = 0xFF3A2E1C;
		Screen = 0xFF120E09; TitleBar = 0xFF2E2416; MenuBar = 0xFF1A140C; Side = 0xFF100C07; Title = 0xFFF0E2C0;
		Tint = 0x22FFB040; Grid = 0x18FFC070;
		Case = 0xFF26241F; Bezel = 0xFF0E0D0B; Grip = 0xFF15140F; BrandColor = 0xFFB8A888;
		Brand = "КПК-М1"; Maker = "КАРМАННЫЙ ПЕРСОНАЛЬНЫЙ КОМПЬЮТЕР  ·  МОДЕЛЬ 1"; Sticker = "НИГ-0473";
		OsName = "СТАЛКЕР-ОС  ·  КПК-М1";
		MenuLine = "Файл     Сеть     Карта     Барахолка     Справка";
		SetTabs("ПРОФИЛЬ", "КАРТА", "ГРУППА", "ГРУППИРОВКИ", "ЗАДАНИЯ", "ЗАМЕТКИ");
		Boot.Insert("КПК-М1  ·  СТАЛКЕР-ОС");
		Boot.Insert("> Проверка питания ........ OK");
		Boot.Insert("> Сеть сталкеров .......... %NET%");
		Boot.Insert("> Детектор аномалий ....... активен");
		Boot.Insert("> Счётчик Гейгера ......... фон в норме");
		Boot.Insert("> Загрузка профиля ........ OK");
		Welcome = "Добро пожаловать, сталкер %1. Удачной охоты.";
		ProfileTitle = "ДОСЬЕ СТАЛКЕРА"; RankLabel = "Ранг"; NoFaction = "вольный сталкер";
		SetRanks("Новичок", "Сталкер", "Опытный", "Ветеран", "Мастер", "Легенда Зоны");
	}

	// ВПК-7М, ОКСОП: khaki / olive, amber accent
	protected void Military()
	{
		Text = 0xFFC8CFA8; Dim = 0xFF7F8763; Accent = 0xFFD9A441; White = 0xFFE6E8D8;
		Btn = 0xFF2B2F1C; BtnHover = 0xFF3C422A; BtnActive = 0xFF59613A; BtnOff = 0xFF151710;
		Row = 0xFF1E2114; RowHover = 0xFF2A2E1C; RowSel = 0xFF454C2C;
		Card = 0xFF181A10; Edge = 0xFF9AA068; Line = 0xFF34381F;
		Screen = 0xFF12140C; TitleBar = 0xFF353A1F; MenuBar = 0xFF1C1F12; Side = 0xFF14160D; Title = 0xFFE8EAD0;
		Tint = 0x22C8D080; Grid = 0x18D0D890;
		Case = 0xFF3A3926; Bezel = 0xFF101009; Grip = 0xFF24241A; BrandColor = 0xFFC9C09A;
		Brand = "ВПК-7М  №3184"; Maker = "ОКСОП  ·  ВОЕННЫЙ ПЕРСОНАЛЬНЫЙ КОМПЬЮТЕР"; Sticker = "ИНВ. № 027645-30   ДСП";
		OsName = "ОКСОП  ·  ЗАКРЫТЫЙ КАНАЛ";
		MenuLine = "Приказы     Связь     Карта     Допуск     Справка";
		SetTabs("ЛИЧНОЕ ДЕЛО", "ОПЕРКАРТА", "ОТДЕЛЕНИЕ", "ПОДРАЗДЕЛЕНИЯ", "ПРИКАЗЫ", "ЖУРНАЛ");
		Boot.Insert("ВПК-7М №3184  ·  ОКСОП");
		Boot.Insert("> Проверка допуска ........ ГРИФ «СЕКРЕТНО»");
		Boot.Insert("> Закрытый канал .......... %NET%");
		Boot.Insert("> Шифратор ................ включён");
		Boot.Insert("> Периметр ................ под контролем");
		Boot.Insert("> Загрузка личного дела ... OK");
		Welcome = "Боец %1, вход в закрытую сеть ОКСОП подтверждён. Доложите обстановку.";
		ProfileTitle = "ЛИЧНОЕ ДЕЛО ВОЕННОСЛУЖАЩЕГО"; RankLabel = "Звание"; NoFaction = "военнослужащий ОКСОП";
		SetRanks("Рядовой", "Ефрейтор", "Сержант", "Старшина", "Лейтенант", "Капитан");
	}

	// ПИК №ОС-031, Монолит: cyan glow, interference instead of words
	protected void Monolith()
	{
		Text = 0xFF7FE0E0; Dim = 0xFF3E8A8E; Accent = 0xFFB8F4F4; White = 0xFFE0F8F8;
		Btn = 0xFF0F2428; BtnHover = 0xFF163A40; BtnActive = 0xFF22585E; BtnOff = 0xFF0A1214;
		Row = 0xFF0B1A1D; RowHover = 0xFF12282C; RowSel = 0xFF1E4A50;
		Card = 0xFF081417; Edge = 0xFF3FB0B4; Line = 0xFF16343A;
		Screen = 0xFF050C0E; TitleBar = 0xFF0F282C; MenuBar = 0xFF0A181B; Side = 0xFF061012; Title = 0xFFC8F8F8;
		Tint = 0x2240F0F0; Grid = 0x1860E0E0;
		Case = 0xFF3A3632; Bezel = 0xFF0E0D0C; Grip = 0xFF221E1A; BrandColor = 0xFFBDB8AA;
		Brand = "ПИК  №ОС-031"; Maker = "М  О  Н  О  Л  И  Т"; Sticker = "#%& ОСОЗНАНИЕ &%#";
		OsName = "#  ОС-031  #";
		MenuLine = "";
		SetTabs("[ I ]", "[ II ]", "[ III ]", "[ IV ]", "[ V ]", "[ VI ]");
		Boot.Insert("#%&@  ОС-031  @&%#");
		Boot.Insert("> ..::..::..::..");
		Boot.Insert("> %NET% ..::.. СЛЫШИТ");
		Boot.Insert("> #%& ОСОЗНАНИЕ &%#");
		Boot.Insert("> ..::..::..::..::..");
		Boot.Insert("> МОНОЛИТ ЖДЁТ");
		Welcome = "%1 . . . МОНОЛИТ СЛЫШИТ ТЕБЯ";
		ProfileTitle = "#  АДЕПТ  #"; RankLabel = "Посвящение"; NoFaction = "брат Монолита";
		SetRanks("Послушник", "Адепт", "Брат", "Страж", "Апостол", "Пророк");
		Glitch = true;
	}

	// ПИК №042, НИИ: cold white and ice blue
	protected void Science()
	{
		Text = 0xFFD5E4EC; Dim = 0xFF7F96A3; Accent = 0xFF5FC9F0; White = 0xFFFFFFFF;
		Btn = 0xFF1E2A33; BtnHover = 0xFF2C3D4A; BtnActive = 0xFF3F5A6E; BtnOff = 0xFF12181C;
		Row = 0xFF17212A; RowHover = 0xFF223040; RowSel = 0xFF35506A;
		Card = 0xFF121A21; Edge = 0xFF8FB4C8; Line = 0xFF2A3A48;
		Screen = 0xFF0E151B; TitleBar = 0xFF2A3A47; MenuBar = 0xFF18222A; Side = 0xFF10171D; Title = 0xFFF0F6FA;
		Tint = 0x1880C8FF; Grid = 0x14A0D0FF;
		Case = 0xFFB9B9B2; Bezel = 0xFF141414; Grip = 0xFF161616; BrandColor = 0xFF2A2C2E;
		Brand = "НИИ  ·  ПИК №042"; Maker = "ПЕРСОНАЛЬНЫЙ ИССЛЕДОВАТЕЛЬСКИЙ КОМПЬЮТЕР"; Sticker = "ИНВ. № НИИ-042   ПОВЕРЕНО";
		OsName = "НИИ  ·  ИССЛЕДОВАТЕЛЬСКИЙ ТЕРМИНАЛ";
		MenuLine = "Протоколы     Данные     Карта     Аномалии     Справка";
		SetTabs("СОТРУДНИК", "КАРТА ПОЛЕЙ", "ГРУППА", "ГРУППИРОВКИ", "ПРОТОКОЛЫ", "НАБЛЮДЕНИЯ");
		Boot.Insert("НИИ  ·  ПИК №042  ·  ИССЛЕДОВАТЕЛЬСКИЙ ТЕРМИНАЛ");
		Boot.Insert("> Самодиагностика ......... OK");
		Boot.Insert("> Узел связи НИИ .......... %NET%");
		Boot.Insert("> Датчики аномалий ........ откалиброваны");
		Boot.Insert("> Синхронизация протоколов  OK");
		Boot.Insert("> Карточка сотрудника ..... OK");
		Welcome = "Сотрудник %1, сеанс открыт. Данные полевых наблюдений синхронизированы.";
		ProfileTitle = "КАРТОЧКА СОТРУДНИКА НИИ"; RankLabel = "Должность"; NoFaction = "научный сотрудник";
		SetRanks("Лаборант", "Мл. научный сотрудник", "Научный сотрудник", "Ст. научный сотрудник", "Ведущий научный сотрудник", "Профессор");
	}

	// Monolith interference: replace a share of the characters with noise
	static string Noise(string text, float amount)
	{
		string charset = "#%&@$?!/<>=+*01:.";
		string result = "";
		int len = text.LengthUtf8();
		for (int i = 0; i < len; i++)
		{
			string ch = text.SubstringUtf8(i, 1);
			if (ch != " " && Math.RandomFloat01() < amount)
				result += charset.Get(Math.RandomInt(0, charset.Length()));
			else
				result += ch;
		}
		return result;
	}
};
