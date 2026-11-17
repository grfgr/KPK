/*
	$profile:KPK/settings.json, created with defaults on the first start.
	Saved back after loading, so options added in newer versions appear automatically.
*/
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
	float MatesUpdateSec = 2.0;
	float MatesMaxDistance = 3000;   // m; mates farther away are not sent (0 = no limit)

	// 3D labels require a KPK (any, no battery check) in the inventory
	bool LabelsRequireDevice = true;

	// notes
	int MaxNotes = 20;
	int NoteMaxLength = 1000;   // characters; the whole KPK state travels in one RPC, keep it modest

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
	const static string INDEX_FILE = "$profile:KPK/accounts_index.json";

	void KPK_Settings()
	{
		Admins = new array<string>();
	}

	static KPK_Settings Load()
	{
		if (!FileExist(DIR))
			MakeDirectory(DIR);
		if (!FileExist(ACCOUNTS_DIR))
			MakeDirectory(ACCOUNTS_DIR);

		KPK_Settings settings = new KPK_Settings();
		if (FileExist(FILE))
			JsonFileLoader<KPK_Settings>.JsonLoadFile(FILE, settings);

		if (!settings.Admins)
			settings.Admins = new array<string>();

		if (settings.PasswordSalt == "")
		{
			Math.Randomize(-1);
			settings.PasswordSalt = string.Format("%1%2%3", Math.RandomInt(100000, 999999), GetGame().GetTime(), Math.RandomInt(100000, 999999));
		}

		JsonFileLoader<KPK_Settings>.JsonSaveFile(FILE, settings);
		return settings;
	}

	bool IsAdmin(string steamId)
	{
		return Admins.Find(steamId) != -1;
	}
};
