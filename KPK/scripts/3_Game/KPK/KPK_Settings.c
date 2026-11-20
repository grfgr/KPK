/*
	$profile:KPK/settings.json, created with defaults on the first start.
	Saved back after loading, so options added in newer versions appear automatically.
*/
// faction membership set by the server owner: applied on every server start
class KPK_RosterEntry
{
	string Faction;    // id from factions.json: oksop, nii, monolith, loners ...
	string SteamId;    // Steam64 ID of the player
	string Role;       // "leader", "deputy" or "member"
};

class KPK_Settings
{
	int ConfigVersion = 1;

	// Steam64 IDs of admins: they manage factions and see every faction unencrypted
	ref array<string> Admins;

	string NetworkName = "СЕТЬ ЗОНЫ";

	// registration
	int CallsignMinLength = 3;
	int CallsignMaxLength = 20;
	int PasswordMinLength = 4;
	// true: password once per connection. false: every time the device is opened.
	bool PasswordOncePerSession = true;

	// device power
	bool RequireBattery = true;
	float BatteryDrainPercentPerMinute = 1.0;   // while the KPK is open

	// map
	bool ShowSelfOnMap = true;
	int MaxPersonalMarkers = 50;
	int MaxPartyMarkers = 30;
	int MaxFactionMarkers = 100;

	// party
	int PartyMaxSize = 6;
	float MatesUpdateSec = 1.0;
	bool MatesRateFixed;   // older files had 2 s: lowered once
	float MatesMaxDistance = 3000;   // m; mates farther away are not sent (0 = no limit)

	// 3D labels require a KPK (any, no battery check) in the inventory
	bool LabelsRequireDevice = true;
	bool MateNeedsDevice = true;
	float LogoutWithoutDeviceSec = 60;   // s without any KPK on the player: logged out, password again (0 = never)   // a mate without a KPK (thrown away, dropped) has no label for the others
	float LabelMaxDistance = 300;   // m; the 3D label of a mate disappears farther away (0 = always shown)
	bool LabelLineOfSight = true;   // the label shows only while the mate is in sight (not behind a wall)

	// notes
	int MaxNotes = 20;
	int NoteMaxLength = 1000;   // characters; the whole KPK state travels in one RPC, keep it modest

	// faction-locked devices: the faction id (factions.json) whose members may use the model, "" = anyone.
	// A stranger who switches on the ВПК-7М or ПИК №042 gets "ДОСТУП ЗАПРЕЩЁН" and the device wipes itself
	// (ruined). The Monolith device blows up in the stranger's hands: damage and unconsciousness.
	// Members are set by an admin (Factions tab -> "+ боец") or in factions.json.
	string MilitaryFaction = "oksop";
	string ScienceFaction = "nii";
	string MonolithFaction = "monolith";
	string StalkerFaction = "";
	bool AdminsBypassDeviceLock = true;   // admins may switch on faction-locked devices (no admin powers there)
	// leaders and deputies enrol a player at once (no invite to accept); the player must not be in another faction
	bool FactionDirectAdd = true;
	// leaders / members written by the server owner; entries with an invalid SteamId are skipped
	ref array<ref KPK_RosterEntry> FactionRoster;
	bool RosterExampleAdded;
	bool OldExampleFactionsRemoved;   // Долг / Свобода / Наёмники from older versions removed once
	// factions listed in the reputation card of the profile (ids from factions.json), empty = all
	ref array<string> ReputationFactions;
	bool ReputationDefaultsAdded;   // set after the defaults went in once, so [] can mean "all"
	float MonolithTrapDamage = 50;
	bool MonolithTrapKnockout = true;

	// radio calls between bricks: both radios go to the same vanilla walkie-talkie channel.
	// The game has few channels: an ordinary walkie-talkie on that channel hears the call too.
	int CallChannels = 6;
	int CallMaxMembers = 6;          // participants of one brick call (the caller included)
	bool TestCallEnabled = false;    // admins can call "ТЕСТ" on the brick: answered through a brick lying nearby
	float CallRingSec = 40;          // an unanswered call is dropped after this

	// chat and contacts (not on the brick)
	int ChatHistory = 40;            // messages the server keeps (until restart)
	int ChatMaxLength = 160;
	float ChatCooldownSec = 2;
	bool ChatNotifyClosed = true;    // a closed KPK in the inventory shows new messages as a notification
	int MaxContacts = 30;

	// stats
	float StatsTickSec = 10;

	// how often changed accounts are written to disk
	float SaveIntervalSec = 60;

	// salt for password hashes, generated on the first start. Changing it invalidates every password.
	string PasswordSalt = "";

	const static string DIR = "$profile:KPK";
	const static string ACCOUNTS_DIR = "$profile:KPK/accounts";
	const static string FILE = "$profile:KPK/settings.json";
	const static string FACTIONS_FILE = "$profile:KPK/factions.json";
	static bool s_LoadFailed;
	const static string SALT_FILE = "$profile:KPK/password_salt.txt";
	const static int RADIO_FREQUENCIES = 9;   // frequencies of the vanilla walkie-talkie
	const static string ADMINS_FILE = "$profile:KPK/admins.txt";
	const static string INDEX_FILE = "$profile:KPK/accounts_index.json";
	const static string OWNERS_FILE = "$profile:KPK/device_owners.json";

	void KPK_Settings()
	{
		Admins = new array<string>();
		ReputationFactions = new array<string>();
		FactionRoster = new array<ref KPK_RosterEntry>();
	}

	static KPK_Settings Load()
	{
		if (!FileExist(DIR))
			MakeDirectory(DIR);
		if (!FileExist(ACCOUNTS_DIR))
			MakeDirectory(ACCOUNTS_DIR);

		KPK_Settings settings = new KPK_Settings();
		bool broken = false;
		if (FileExist(FILE))
		{
			// a typo in the file (missing comma or quote) used to give default settings, which were then
			// written back over the file: the admin list was lost. Now a broken file is left untouched.
			string error;
			if (!JsonFileLoader<KPK_Settings>.LoadFile(FILE, settings, error))
			{
				broken = true;
				settings = new KPK_Settings();
				Print("[KPK] !!! settings.json has an error and was NOT loaded, defaults are used: " + error);
				s_LoadFailed = true;
			}
		}

		if (!settings.Admins)
			settings.Admins = new array<string>();
		if (!settings.ReputationFactions)
			settings.ReputationFactions = new array<string>();
		// the walkie-talkie the brick is built on has 9 frequencies: more channels would put calls on the
		// same frequency (they would hear each other)
		if (settings.CallChannels > RADIO_FREQUENCIES)
			settings.CallChannels = RADIO_FREQUENCIES;
		if (settings.CallChannels < 1)
			settings.CallChannels = 1;
		if (!settings.MatesRateFixed)
		{
			if (settings.MatesUpdateSec > 1.0)
				settings.MatesUpdateSec = 1.0;
			settings.MatesRateFixed = true;
		}
		if (!settings.FactionRoster)
			settings.FactionRoster = new array<ref KPK_RosterEntry>();
		if (!settings.RosterExampleAdded)
		{
			settings.AddRosterExample("oksop", "leader");
			settings.AddRosterExample("nii", "leader");
			settings.AddRosterExample("monolith", "leader");
			settings.RosterExampleAdded = true;
		}
		if (!settings.ReputationDefaultsAdded)
		{
			settings.ReputationFactions.Clear();
			settings.ReputationFactions.Insert("loners");
			settings.ReputationFactions.Insert("oksop");
			settings.ReputationFactions.Insert("nii");
			settings.ReputationFactions.Insert("monolith");
			settings.ReputationDefaultsAdded = true;
		}

		// the salt is also kept in its own file: if settings.json is broken or reset, passwords still match
		string savedSalt = ReadSalt();
		if (broken || settings.PasswordSalt == "")
			settings.PasswordSalt = savedSalt;
		if (settings.PasswordSalt == "")
		{
			Math.Randomize(-1);
			settings.PasswordSalt = string.Format("%1%2%3", Math.RandomInt(100000, 999999), GetGame().GetTime(), Math.RandomInt(100000, 999999));
		}
		if (savedSalt != settings.PasswordSalt)
			WriteSalt(settings.PasswordSalt);

		settings.LoadAdminsTxt();
		if (!broken)
			JsonFileLoader<KPK_Settings>.JsonSaveFile(FILE, settings);
		return settings;
	}

	static string ReadSalt()
	{
		if (!FileExist(SALT_FILE))
			return "";
		FileHandle handle = OpenFile(SALT_FILE, FileMode.READ);
		if (handle == 0)
			return "";
		string salt;
		FGets(handle, salt);
		CloseFile(handle);
		return salt.Trim();
	}

	static void WriteSalt(string salt)
	{
		FileHandle handle = OpenFile(SALT_FILE, FileMode.WRITE);
		if (handle == 0)
			return;
		FPrintln(handle, salt);
		CloseFile(handle);
	}

	// $profile:KPK/admins.txt: one Steam64 ID per line, a simpler alternative to the JSON list
	void LoadAdminsTxt()
	{
		if (!FileExist(ADMINS_FILE))
		{
			FileHandle created = OpenFile(ADMINS_FILE, FileMode.WRITE);
			if (created != 0)
			{
				FPrintln(created, "// Steam64 ID администраторов КПК, по одному в строке. Строки с // не читаются.");
				CloseFile(created);
			}
			return;
		}
		FileHandle handle = OpenFile(ADMINS_FILE, FileMode.READ);
		if (handle == 0)
			return;
		string line;
		string numbers = "0123456789";
		while (FGets(handle, line) >= 0)
		{
			if (line.IndexOf("//") != -1)
				continue;
			// only the digits count: spaces, quotes, commas or an invisible BOM do not matter
			string id = "";
			for (int i = 0; i < line.Length(); i++)
			{
				string ch = line.Get(i);
				if (numbers.IndexOf(ch) != -1)
					id += ch;
			}
			if (id.Length() == 17 && Admins.Find(id) == -1)
				Admins.Insert(id);
		}
		CloseFile(handle);
		Print("[KPK] admins: " + Admins.Count());
	}

	protected void AddRosterExample(string faction, string role)
	{
		KPK_RosterEntry e = new KPK_RosterEntry();
		e.Faction = faction;
		e.SteamId = "STEAM64_ID";
		e.Role = role;
		FactionRoster.Insert(e);
	}

	void Save()
	{
		if (s_LoadFailed)
			return;
		JsonFileLoader<KPK_Settings>.JsonSaveFile(FILE, this);
	}

	bool IsAdmin(string steamId)
	{
		return Admins.Find(steamId) != -1;
	}

	// from the admin KPK: the list is kept in settings.json and admins.txt (rewritten)
	void AddAdmin(string steamId)
	{
		if (Admins.Find(steamId) == -1)
			Admins.Insert(steamId);
		WriteAdminsTxt();
		Save();
	}

	void RemoveAdmin(string steamId)
	{
		int i = Admins.Find(steamId);
		if (i != -1)
			Admins.Remove(i);
		WriteAdminsTxt();
		Save();
	}

	protected void WriteAdminsTxt()
	{
		FileHandle handle = OpenFile(ADMINS_FILE, FileMode.WRITE);
		if (handle == 0)
			return;
		FPrintln(handle, "// Steam64 ID администраторов КПК, по одному в строке. Строки с // не читаются.");
		FPrintln(handle, "// Список меняется и из админского КПК «Куратор».");
		foreach (string id : Admins)
			FPrintln(handle, id);
		CloseFile(handle);
	}
};
