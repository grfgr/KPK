// One RPC id for the whole mod. Payload: Param2<int action, string json>.
const int KPK_RPC = 0x4B504B01;
// broken KPK blast, sent to every client: Param1<vector position>
const int KPK_RPC_FX = 0x4B504B02;

// client -> server
enum KPK_C
{
	OPEN = 1,
	CLOSE,
	REGISTER,
	LOGIN,
	MARKER_ADD,
	MARKER_DEL,
	NOTE_SAVE,
	NOTE_DEL,
	PARTY_INVITE,
	PARTY_RESPOND,
	PARTY_LEAVE,
	PARTY_KICK,
	FACTION_INVITE,
	FACTION_RESPOND,
	FACTION_LEAVE,
	FACTION_KICK,
	FACTION_SETROLE,
	ADMIN_FACTION,
	MARKER_EDIT,     // I1 id, I2 scope, S1 name, S2 type, I3 = 1 -> move to F1/F2
	CHAT_SEND,       // S1 text, S2 callsign for a private message ("" = general channel)
	CONTACT_ADD,     // S1 callsign
	CONTACT_DEL,     // S1 callsign
	CALL_START,      // S1 callsign (brick only)
	CALL_ANSWER,     // I1 = 1 accept, 0 decline
	CALL_END
};

// radio call of the brick (KPK_View.CallState)
enum KPK_CallState
{
	NONE = 0,
	OUTGOING,    // ringing at the other side
	INCOMING,    // somebody calls this player
	ACTIVE       // on air: both radios on the same channel
};

// server -> client
enum KPK_S
{
	VIEW = 100,   // full snapshot (KPK_View)
	MATES,        // positions of party / faction mates (KPK_MatesMsg)
	MSG,          // status line text
	DENIED,       // cannot open (no battery, no device) - text in json
	CHAT          // one new chat message (KPK_ChatMessage)
};

// ADMIN_FACTION sub-operations (KPK_Req.I1)
enum KPK_AdminOp
{
	CREATE = 1,
	REMOVE,
	TOGGLE_ENC,     // I2 = KPK_EncField
	SET_LOCATION,   // S2 = text, I2 = 1 -> also store the admin's current position
	CLEAR_LOCATION_POS,
	SET_DESC,       // S2 = text
	SET_LEADER,     // S2 = callsign
	ADD_MEMBER,     // S2 = callsign (no invite, forced)
	TOGGLE_HIDDEN,
	TOGGLE_MATES,
	SET_COLOR       // I2 = palette index
};

enum KPK_EncField
{
	DESCRIPTION = 1,
	LEADER,
	MEMBERS,
	LOCATION
};

// device models: each has its own look and interface theme
enum KPK_Model
{
	BRICK = 0,     // КПК-7 «Вестник»: profile, map and party only
	STALKER,       // КПК-М1
	MILITARY,      // ВПК-7М (ОКСОП)
	MONOLITH,      // ПИК №ОС-031
	SCIENCE        // ПИК №042 (НИИ)
};

// faction roles
enum KPK_Role
{
	NONE = 0,
	MEMBER,
	DEPUTY,
	LEADER
};

// marker scopes
enum KPK_Scope
{
	PERSONAL = 0,
	PARTY,
	FACTION
};

// mate kinds for 3D labels
enum KPK_MateKind
{
	PARTY = 1,
	FACTION
};

class KPK_Const
{
	// the brick has no factions, tasks, notes or reputation
	static bool FullAccess(int model)
	{
		return model != KPK_Model.BRICK;
	}

	static string ModelName(int model)
	{
		switch (model)
		{
			case KPK_Model.STALKER: return "КПК-М1";
			case KPK_Model.MILITARY: return "ВПК-7М";
			case KPK_Model.MONOLITH: return "ПИК №ОС-031";
			case KPK_Model.SCIENCE: return "ПИК №042";
		}
		return "КПК";
	}

	// atmospheric marker types; icons are vanilla map icons
	static const int MARKER_TYPES = 12;

	static string MarkerName(int type)
	{
		switch (type)
		{
			case 0: return "Аномалия";
			case 1: return "Артефакт";
			case 2: return "Тайник";
			case 3: return "Лагерь";
			case 4: return "Логово мутантов";
			case 5: return "Радиация";
			case 6: return "Укрытие от выброса";
			case 7: return "Точка сбора";
			case 8: return "Торговец";
			case 9: return "Связь";
			case 10: return "Блокпост";
			case 11: return "Вода";
		}
		return "Метка";
	}

	static string MarkerIcon(int type)
	{
		switch (type)
		{
			case 0: return "\\DZ\\gear\\navigation\\data\\map_hill_ca.paa";
			case 1: return "\\DZ\\gear\\navigation\\data\\map_monument_ca.paa";
			case 2: return "\\DZ\\gear\\navigation\\data\\map_tsign_ca.paa";
			case 3: return "\\DZ\\gear\\navigation\\data\\map_camp_ca.paa";
			case 4: return "\\DZ\\gear\\navigation\\data\\map_fir_ca.paa";
			case 5: return "\\DZ\\gear\\navigation\\data\\map_border_cross_ca.paa";
			case 6: return "\\DZ\\gear\\navigation\\data\\map_tshelter_ca.paa";
			case 7: return "\\DZ\\gear\\navigation\\data\\map_viewpoint_ca.paa";
			case 8: return "\\DZ\\gear\\navigation\\data\\map_store_ca.paa";
			case 9: return "\\DZ\\gear\\navigation\\data\\map_transmitter_ca.paa";
			case 10: return "\\DZ\\gear\\navigation\\data\\map_police_ca.paa";
			case 11: return "\\DZ\\gear\\navigation\\data\\map_waterpump_ca.paa";
		}
		return "\\DZ\\gear\\navigation\\data\\map_viewpoint_ca.paa";
	}

	static int MarkerColor(int type)
	{
		switch (type)
		{
			case 0: return ARGB(255, 230, 120, 40);
			case 1: return ARGB(255, 240, 220, 60);
			case 2: return ARGB(255, 170, 140, 90);
			case 3: return ARGB(255, 240, 160, 60);
			case 4: return ARGB(255, 170, 60, 170);
			case 5: return ARGB(255, 220, 40, 40);
			case 6: return ARGB(255, 80, 200, 200);
			case 7: return ARGB(255, 110, 220, 90);
			case 8: return ARGB(255, 220, 180, 80);
			case 9: return ARGB(255, 90, 150, 240);
			case 10: return ARGB(255, 170, 170, 170);
			case 11: return ARGB(255, 90, 180, 240);
		}
		return ARGB(255, 200, 200, 200);
	}

	static string ScopeName(int scope)
	{
		switch (scope)
		{
			case KPK_Scope.PARTY: return "Группа";
			case KPK_Scope.FACTION: return "Группировка";
		}
		return "Личная";
	}

	static string RoleName(int role)
	{
		switch (role)
		{
			case KPK_Role.LEADER: return "Лидер";
			case KPK_Role.DEPUTY: return "Заместитель";
			case KPK_Role.MEMBER: return "Боец";
		}
		return "—";
	}

	// faction color palette (admin picks an index)
	static const int PALETTE_SIZE = 10;

	static int Palette(int i)
	{
		switch (i)
		{
			case 0: return ARGB(255, 200, 200, 200); // grey: loners
			case 1: return ARGB(255, 220, 60, 50);   // red: duty
			case 2: return ARGB(255, 90, 200, 80);   // green: freedom
			case 3: return ARGB(255, 70, 120, 230);  // blue: mercenaries
			case 4: return ARGB(255, 230, 190, 60);  // yellow: scientists
			case 5: return ARGB(255, 120, 120, 120); // dark grey: bandits
			case 6: return ARGB(255, 240, 240, 240); // white: monolith
			case 7: return ARGB(255, 200, 120, 50);  // orange
			case 8: return ARGB(255, 150, 80, 200);  // purple
			case 9: return ARGB(255, 60, 190, 190);  // cyan
		}
		return ARGB(255, 200, 200, 200);
	}

	static string Rank(int kills, int playtimeSec)
	{
		int hours = playtimeSec / 3600;
		int score = kills * 2 + hours;
		if (score >= 400) return "Легенда Зоны";
		if (score >= 200) return "Мастер";
		if (score >= 100) return "Ветеран";
		if (score >= 40) return "Опытный";
		if (score >= 10) return "Сталкер";
		return "Новичок";
	}

	static string ReputationName(int value)
	{
		if (value <= -500) return "Враг";
		if (value <= -100) return "Недружелюбно";
		if (value < 100) return "Нейтрально";
		if (value < 500) return "Дружелюбно";
		return "Свой";
	}
};
