// ---------------------------------------------------------------- stored on the server

class KPK_Stats
{
	int PlayerKills;
	int ZombieKills;
	int AnimalKills;
	int Deaths;
	int Suicides;
	float LongestKill;   // m
	int PlaytimeSec;
	float DistanceM;
	int CurrentLifeSec;
	int BestLifeSec;
};

class KPK_Marker
{
	int Id;
	string Name;
	int Type;
	int Scope;       // KPK_Scope
	float X;
	float Z;
	string Author;   // callsign
	string Date;
	string FactionId; // owner faction for Scope FACTION
};

class KPK_Note
{
	int Id;
	string Title;
	string Text;
	string Date;
};

// Tasks: groundwork. Fill from another mod / script through KPK_Server.GiveTask().
class KPK_Task
{
	string Id;
	string Title;
	string Description;
	string Issuer;     // who gave it: faction, trader, NPC
	string Reward;
	int Status;        // 0 active, 1 completed, 2 failed
	string Date;
};

// Reputation with a faction: groundwork. Change through KPK_Server.AddReputation().
class KPK_Reputation
{
	string FactionId;
	int Value;
};

class KPK_Account
{
	int Version = 1;
	string SteamId;
	string Callsign;
	int PasswordHash;        // 0 = password was reset by an admin, a new one is set on next login
	string Registered;
	string LastSeen;
	int NextId = 1;
	ref KPK_Stats Stats;
	ref array<ref KPK_Marker> Markers;
	ref array<ref KPK_Note> Notes;
	ref array<ref KPK_Task> Tasks;
	ref array<ref KPK_Reputation> Reputation;
	ref array<string> Contacts;   // steam ids

	void KPK_Account()
	{
		Contacts = new array<string>();
		Stats = new KPK_Stats();
		Markers = new array<ref KPK_Marker>();
		Notes = new array<ref KPK_Note>();
		Tasks = new array<ref KPK_Task>();
		Reputation = new array<ref KPK_Reputation>();
	}

	// JSON written by hand / older versions may miss arrays
	void Fix()
	{
		if (!Stats) Stats = new KPK_Stats();
		if (!Markers) Markers = new array<ref KPK_Marker>();
		if (!Notes) Notes = new array<ref KPK_Note>();
		if (!Tasks) Tasks = new array<ref KPK_Task>();
		if (!Reputation) Reputation = new array<ref KPK_Reputation>();
		if (!Contacts) Contacts = new array<string>();
		if (NextId < 1) NextId = 1;

		// text saved by older versions may hold characters that break the JSON view
		foreach (KPK_Note n : Notes)
		{
			n.Title = KPK_Util.Safe(n.Title);
			n.Text = KPK_Util.Safe(n.Text, false);
		}
		foreach (KPK_Marker m : Markers)
			m.Name = KPK_Util.Safe(m.Name);
		foreach (KPK_Task t : Tasks)
		{
			t.Title = KPK_Util.Safe(t.Title);
			t.Description = KPK_Util.Safe(t.Description, false);
			t.Issuer = KPK_Util.Safe(t.Issuer);
			t.Reward = KPK_Util.Safe(t.Reward);
		}
	}

	int TakeId()
	{
		NextId++;
		return NextId - 1;
	}
};

class KPK_FactionMember
{
	string SteamId;
	int Role;   // KPK_Role
};

class KPK_Faction
{
	string Id;
	string Name;
	string Description;
	int Color;              // palette index (KPK_Const.Palette)
	string Location;        // text, e.g. "Бар «100 рентген», Росток"
	bool HasLocationPos;
	float LocX;
	float LocZ;

	// what outsiders cannot read; set by the admin
	bool EncryptDescription;
	bool EncryptLeader;
	bool EncryptMembers;
	bool EncryptLocation;

	bool Hidden;            // not listed at all for outsiders
	bool ShowMatesInWorld = true; // members see each other (3D label + KPK map)

	ref array<ref KPK_FactionMember> Members;

	void KPK_Faction()
	{
		Members = new array<ref KPK_FactionMember>();
	}

	void Fix()
	{
		if (!Members) Members = new array<ref KPK_FactionMember>();
	}

	KPK_FactionMember Find(string steamId)
	{
		foreach (KPK_FactionMember m : Members)
		{
			if (m.SteamId == steamId)
				return m;
		}
		return null;
	}

	string LeaderId()
	{
		foreach (KPK_FactionMember m : Members)
		{
			if (m.Role == KPK_Role.LEADER)
				return m.SteamId;
		}
		return "";
	}
};

class KPK_FactionsFile
{
	int Version = 1;
	ref array<ref KPK_Faction> Factions;
	ref array<ref KPK_Marker> FactionMarkers;   // markers with Scope FACTION
	int NextMarkerId = 1;

	void KPK_FactionsFile()
	{
		Factions = new array<ref KPK_Faction>();
		FactionMarkers = new array<ref KPK_Marker>();
	}

	void Fix()
	{
		if (!Factions) Factions = new array<ref KPK_Faction>();
		if (!FactionMarkers) FactionMarkers = new array<ref KPK_Marker>();
		foreach (KPK_Faction f : Factions)
		{
			f.Fix();
			f.Name = KPK_Util.Safe(f.Name);
			f.Description = KPK_Util.Safe(f.Description, false);
			f.Location = KPK_Util.Safe(f.Location);
		}
		foreach (KPK_Marker fm : FactionMarkers)
			fm.Name = KPK_Util.Safe(fm.Name);
		if (NextMarkerId < 1) NextMarkerId = 1;
	}
};

class KPK_IndexEntry
{
	string SteamId;
	string Callsign;
};

class KPK_Index
{
	ref array<ref KPK_IndexEntry> Accounts;

	void KPK_Index()
	{
		Accounts = new array<ref KPK_IndexEntry>();
	}
};

// ---------------------------------------------------------------- network messages

// one generic request body, fields used per action
class KPK_Req
{
	string S1;
	string S2;
	int I1;
	int I2;
	int I3;
	float F1;
	float F2;
};

class KPK_PartyMemberView
{
	string Callsign;
	bool Online;
	bool Leader;
	int Distance;   // m, -1 unknown
};

class KPK_InviteView
{
	int Kind;       // 1 party, 2 faction
	string From;    // callsign of who invited
	string FactionId;
	string FactionName;
};

class KPK_FactionMemberView
{
	string Callsign;
	int Role;
	bool Online;
};

class KPK_FactionView
{
	string Id;
	string Name;
	int Color;
	string Description;
	bool DescriptionEnc;
	string Leader;
	bool LeaderEnc;
	ref array<ref KPK_FactionMemberView> Members;
	bool MembersEnc;
	int MemberCount;     // -1 when encrypted
	string Location;
	bool LocationEnc;
	bool HasLocationPos;
	float LocX;
	float LocZ;
	bool IsMine;
	bool Hidden;         // only admins / members get hidden factions
	bool ShowMatesInWorld;

	// the raw encryption flags, only filled for admins
	bool FlagDescription;
	bool FlagLeader;
	bool FlagMembers;
	bool FlagLocation;

	void KPK_FactionView()
	{
		Members = new array<ref KPK_FactionMemberView>();
	}
};

// a chat line as the client sees it
// who used a device (by its persistent id) and who had it before
class KPK_DeviceOwner
{
	string Id;
	string Owner;      // steam id
	string Previous;   // steam id
};

class KPK_OwnersFile
{
	ref array<ref KPK_DeviceOwner> Devices;
	void KPK_OwnersFile()
	{
		Devices = new array<ref KPK_DeviceOwner>();
	}
};

class KPK_ChatMessage
{
	int Id;
	string Time;       // hh:mm of the game world
	string From;       // callsign
	string Tag;        // faction name, "" when unknown or secret
	int Color;         // faction color, 0 = default
	string To;         // callsign of the receiver of a private message, "" = general channel
	string Text;
};

class KPK_ContactView
{
	string Callsign;
	bool Online;
	bool Mutual;       // they have you in their contacts too
	string Faction;    // "" when none or secret
	int Color;
	string LastSeen;
};

class KPK_RepView
{
	string FactionName;
	int Color;
	int Value;
};

class KPK_Vitals
{
	float Health;
	float Blood;
	float Water;
	float Energy;
	float Shock;
	float PosX;
	float PosZ;
};

class KPK_View
{
	// account state
	bool Registered;
	bool LoggedIn;
	bool NeedNewPassword;
	string SteamId;
	string Callsign;
	string RegisteredDate;
	bool IsAdmin;
	string ServerName;

	// battery 0..1, -1 when not required
	float Battery;
	int DeviceModel;   // KPK_Model of the device the KPK was opened with

	ref KPK_Stats Stats;
	string Rank;
	ref KPK_Vitals Vitals;
	ref array<ref KPK_RepView> Reputation;

	ref array<ref KPK_Marker> Markers;        // personal + party + faction, Scope tells which

	bool InParty;
	ref array<ref KPK_PartyMemberView> Party;
	bool PartyLeader;
	int PartyMax;
	ref array<ref KPK_InviteView> Invites;

	string MyFactionId;
	int MyRole;
	ref array<ref KPK_FactionView> Factions;

	ref array<ref KPK_Task> Tasks;
	ref array<ref KPK_Note> Notes;

	ref array<ref KPK_ChatMessage> Chat;        // general channel + own private messages, oldest first
	ref array<ref KPK_ContactView> Contacts;
	int ChatMaxLength;

	string PrevOwner;     // callsign of the previous owner of this device, "" = none
	int CallState;        // KPK_CallState (brick)
	string CallWith;      // callsign of the other side

	// settings the UI follows
	bool ShowSelfOnMap;
	int NoteMaxLength;

	void KPK_View()
	{
		Stats = new KPK_Stats();
		Vitals = new KPK_Vitals();
		Reputation = new array<ref KPK_RepView>();
		Markers = new array<ref KPK_Marker>();
		Party = new array<ref KPK_PartyMemberView>();
		Invites = new array<ref KPK_InviteView>();
		Factions = new array<ref KPK_FactionView>();
		Tasks = new array<ref KPK_Task>();
		Notes = new array<ref KPK_Note>();
		Chat = new array<ref KPK_ChatMessage>();
		Contacts = new array<ref KPK_ContactView>();
	}
};

class KPK_MatePos
{
	string SteamId;
	string Callsign;
	int Kind;       // KPK_MateKind
	int Color;
	int NetLow;     // network id of the player entity: the client draws the label at its live position
	int NetHigh;
	float X;
	float Y;
	float Z;
};

class KPK_MatesMsg
{
	bool RequireDevice;
	float LabelMaxDistance = 300;
	ref array<ref KPK_MatePos> Mates;

	void KPK_MatesMsg()
	{
		Mates = new array<ref KPK_MatePos>();
	}
};
