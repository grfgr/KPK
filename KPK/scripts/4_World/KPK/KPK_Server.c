// Runtime party: lives until the server restarts.
class KPK_Party
{
	int Id;
	string LeaderId;
	ref array<string> Members;
	ref array<ref KPK_Marker> Markers;
	int NextMarkerId = 1;

	void KPK_Party()
	{
		Members = new array<string>();
		Markers = new array<ref KPK_Marker>();
	}
};

class KPK_Invite
{
	int Kind;          // 1 party, 2 faction
	string FromId;
	string ToId;
	string FactionId;
	int Time;
};

// chat line on the server: who may read it
class KPK_ChatEntry
{
	string FromId;
	string ToId;      // "" = general channel
	ref KPK_ChatMessage Msg;
};

class KPK_Server
{
	protected static ref KPK_Server s_Instance;

	ref KPK_Settings m_Settings;
	ref KPK_FactionsFile m_Factions;
	ref KPK_Index m_Index;

	ref map<string, ref KPK_Account> m_Accounts;   // steamId -> account (cache)
	ref map<string, bool> m_Dirty;
	ref map<string, bool> m_LoggedIn;
	ref map<string, bool> m_Open;
	ref map<string, int> m_OpenModel;               // steamId -> KPK_Model the KPK was opened with
	ref map<string, EntityAI> m_OpenDevice;         // steamId -> the device itself
	ref map<string, int> m_LoginFails;
	ref map<string, int> m_LockedUntil;           // GetTime() ms
	ref map<string, vector> m_LastPos;
	ref map<string, int> m_LastMatesCount;

	ref array<ref KPK_Party> m_Parties;
	ref map<string, int> m_PartyOf;               // steamId -> party id
	int m_NextPartyId = 1;
	ref array<ref KPK_Invite> m_Invites;
	ref map<string, ref KPK_Assembler> m_Incoming;   // chunked client messages per sender
	ref array<ref KPK_ChatEntry> m_Chat;
	int m_ChatNextId = 1;
	ref map<string, int> m_ChatLast;                // steamId -> GetTime() of the last message

	static KPK_Server Get()
	{
		return s_Instance;
	}

	static void Start()
	{
		if (!s_Instance)
			s_Instance = new KPK_Server();
	}

	static void Stop()
	{
		if (s_Instance)
			s_Instance.SaveAll();
	}

	void KPK_Server()
	{
		m_Accounts = new map<string, ref KPK_Account>();
		m_Dirty = new map<string, bool>();
		m_LoggedIn = new map<string, bool>();
		m_Open = new map<string, bool>();
		m_OpenModel = new map<string, int>();
		m_OpenDevice = new map<string, EntityAI>();
		m_LoginFails = new map<string, int>();
		m_LockedUntil = new map<string, int>();
		m_LastPos = new map<string, vector>();
		m_LastMatesCount = new map<string, int>();
		m_Parties = new array<ref KPK_Party>();
		m_PartyOf = new map<string, int>();
		m_Invites = new array<ref KPK_Invite>();
		m_Incoming = new map<string, ref KPK_Assembler>();
		m_Chat = new array<ref KPK_ChatEntry>();
		m_ChatLast = new map<string, int>();

		m_Settings = KPK_Settings.Load();
		LoadIndex();
		LoadFactions();
		RemoveOldExampleFactions();
		ApplyRoster();

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TickStats, m_Settings.StatsTickSec * 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(TickMates, m_Settings.MatesUpdateSec * 1000, true);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(SaveDirty, m_Settings.SaveIntervalSec * 1000, true);

		Print("[KPK] server started, accounts: " + m_Index.Accounts.Count() + ", factions: " + m_Factions.Factions.Count());
	}

	// =================================================================== storage

	protected void LoadIndex()
	{
		m_Index = new KPK_Index();
		if (FileExist(KPK_Settings.INDEX_FILE))
			JsonFileLoader<KPK_Index>.JsonLoadFile(KPK_Settings.INDEX_FILE, m_Index);
		if (!m_Index.Accounts)
			m_Index.Accounts = new array<ref KPK_IndexEntry>();
	}

	protected void SaveIndex()
	{
		JsonFileLoader<KPK_Index>.JsonSaveFile(KPK_Settings.INDEX_FILE, m_Index);
	}

	protected void LoadFactions()
	{
		m_Factions = new KPK_FactionsFile();
		if (FileExist(KPK_Settings.FACTIONS_FILE))
		{
			JsonFileLoader<KPK_FactionsFile>.JsonLoadFile(KPK_Settings.FACTIONS_FILE, m_Factions);
			m_Factions.Fix();
		}
		else
		{
			CreateExampleFactions();
			SaveFactions();
		}
		EnsureDeviceFactions();
	}

	// the factions the locked devices belong to must exist, also on servers with an older factions.json
	protected void EnsureDeviceFactions()
	{
		bool added = false;
		if (m_Settings.MilitaryFaction != "" && !FindFaction(m_Settings.MilitaryFaction))
		{
			AddExampleFaction(m_Settings.MilitaryFaction, "ОКСОП", "Объединённое командование специальных операций. Закрытая сеть ВПК-7М.", 7, "Блокпост на Кордоне", true);
			added = true;
		}
		if (m_Settings.ScienceFaction != "" && !FindFaction(m_Settings.ScienceFaction))
		{
			AddExampleFaction(m_Settings.ScienceFaction, "НИИ «Агропром»", "Научная экспедиция. Полевые наблюдения, аномалии, артефакты.", 4, "Бункер учёных, Янтарь", false);
			added = true;
		}
		if (m_Settings.MonolithFaction != "" && !FindFaction(m_Settings.MonolithFaction))
		{
			AddExampleFaction(m_Settings.MonolithFaction, "Монолит", "Братья Монолита. Чужим здесь не место.", 6, "Неизвестно", true);
			added = true;
		}
		if (added)
			SaveFactions();
	}

	// older versions created Долг, Свобода and Наёмники; only factions with their own KPK stay
	protected void RemoveOldExampleFactions()
	{
		if (m_Settings.OldExampleFactionsRemoved)
			return;
		array<string> old = {"duty", "freedom", "mercs"};
		for (int i = m_Factions.Factions.Count() - 1; i >= 0; i--)
		{
			if (old.Find(m_Factions.Factions[i].Id) != -1)
				m_Factions.Factions.RemoveOrdered(i);
		}
		for (int m = m_Factions.FactionMarkers.Count() - 1; m >= 0; m--)
		{
			if (old.Find(m_Factions.FactionMarkers[m].FactionId) != -1)
				m_Factions.FactionMarkers.RemoveOrdered(m);
		}
		SaveFactions();
		m_Settings.OldExampleFactionsRemoved = true;
		m_Settings.Save();
	}

	// FactionRoster from settings.json: the owner decides who leads and who belongs to a faction
	protected void ApplyRoster()
	{
		bool changed = false;
		foreach (KPK_RosterEntry e : m_Settings.FactionRoster)
		{
			KPK_Faction f = FindFaction(e.Faction);
			if (!f || !IsSteamId(e.SteamId))
				continue;
			KPK_Faction current = FactionOf(e.SteamId);
			if (current && current != f)
				RemoveFromFaction(current, e.SteamId);
			KPK_FactionMember member = f.Find(e.SteamId);
			if (!member)
			{
				member = new KPK_FactionMember();
				member.SteamId = e.SteamId;
				f.Members.Insert(member);
			}
			string role = e.Role;
			role.ToLower();
			if (role == "leader")
			{
				foreach (KPK_FactionMember lm : f.Members)
				{
					if (lm.Role == KPK_Role.LEADER && lm != member)
						lm.Role = KPK_Role.DEPUTY;
				}
				member.Role = KPK_Role.LEADER;
			}
			else if (role == "deputy")
				member.Role = KPK_Role.DEPUTY;
			else if (member.Role == KPK_Role.NONE)
				member.Role = KPK_Role.MEMBER;
			changed = true;
		}
		if (changed)
			SaveFactions();
	}

	// faction id whose members may use this model, "" = anyone
	string DeviceFaction(int model)
	{
		string id = "";
		switch (model)
		{
			case KPK_Model.STALKER: id = m_Settings.StalkerFaction; break;
			case KPK_Model.MILITARY: id = m_Settings.MilitaryFaction; break;
			case KPK_Model.MONOLITH: id = m_Settings.MonolithFaction; break;
			case KPK_Model.SCIENCE: id = m_Settings.ScienceFaction; break;
		}
		// a lock on a faction that was removed would make the device useless for everyone
		if (id != "" && !FindFaction(id))
			return "";
		return id;
	}

	protected void CreateExampleFactions()
	{
		AddExampleFaction("loners", "Одиночки", "Вольные сталкеры. Каждый сам за себя, но своих не бросают.", 0, "Деревня новичков, Кордон", false);
	}

	protected void AddExampleFaction(string id, string name, string desc, int color, string location, bool secret)
	{
		KPK_Faction f = new KPK_Faction();
		f.Id = id;
		f.Name = name;
		f.Description = desc;
		f.Color = color;
		f.Location = location;
		f.EncryptMembers = secret;
		f.EncryptLeader = secret;
		f.EncryptLocation = secret;
		m_Factions.Factions.Insert(f);
	}

	void SaveFactions()
	{
		JsonFileLoader<KPK_FactionsFile>.JsonSaveFile(KPK_Settings.FACTIONS_FILE, m_Factions);
	}

	protected string AccountFile(string steamId)
	{
		return KPK_Settings.ACCOUNTS_DIR + "/" + steamId + ".json";
	}

	KPK_Account GetAccount(string steamId)
	{
		KPK_Account cached;
		if (m_Accounts.Find(steamId, cached))
			return cached;

		string path = AccountFile(steamId);
		if (!FileExist(path))
			return null;

		KPK_Account acc = new KPK_Account();
		JsonFileLoader<KPK_Account>.JsonLoadFile(path, acc);
		acc.Fix();
		acc.SteamId = steamId;
		m_Accounts.Set(steamId, acc);
		return acc;
	}

	void MarkDirty(string steamId)
	{
		m_Dirty.Set(steamId, true);
	}

	void SaveAccount(string steamId)
	{
		KPK_Account acc;
		if (!m_Accounts.Find(steamId, acc))
			return;
		JsonFileLoader<KPK_Account>.JsonSaveFile(AccountFile(steamId), acc);
		m_Dirty.Remove(steamId);
	}

	void SaveDirty()
	{
		array<string> ids = m_Dirty.GetKeyArray();
		foreach (string id : ids)
			SaveAccount(id);
	}

	void SaveAll()
	{
		SaveDirty();
		SaveFactions();
	}

	static bool IsSteamId(string text)
	{
		if (text.Length() != 17)
			return false;
		string digits = "0123456789";
		for (int i = 0; i < 17; i++)
		{
			if (digits.IndexOf(text.Get(i)) == -1)
				return false;
		}
		return true;
	}

	string CallsignOf(string steamId)
	{
		foreach (KPK_IndexEntry e : m_Index.Accounts)
		{
			if (e.SteamId == steamId)
				return e.Callsign;
		}
		return "?";
	}

	string SteamIdOfCallsign(string callsign)
	{
		string wanted = KPK_Util.Lower(callsign.Trim());
		foreach (KPK_IndexEntry e : m_Index.Accounts)
		{
			if (KPK_Util.Lower(e.Callsign) == wanted)
				return e.SteamId;
		}
		return "";
	}

	// =================================================================== online players

	map<string, PlayerBase> OnlinePlayers()
	{
		map<string, PlayerBase> result = new map<string, PlayerBase>();
		array<Man> men = new array<Man>();
		GetGame().GetPlayers(men);
		foreach (Man man : men)
		{
			PlayerBase p = PlayerBase.Cast(man);
			if (p && p.GetIdentity())
				result.Set(p.GetIdentity().GetPlainId(), p);
		}
		return result;
	}

	PlayerBase FindOnline(string steamId)
	{
		map<string, PlayerBase> online = OnlinePlayers();
		PlayerBase p;
		if (online.Find(steamId, p))
			return p;
		return null;
	}

	void OnClientDisconnected(string steamId)
	{
		SaveAccount(steamId);
		m_LoggedIn.Remove(steamId);
		m_Open.Remove(steamId);
		m_OpenModel.Remove(steamId);
		m_OpenDevice.Remove(steamId);
		m_LastPos.Remove(steamId);
		m_LastMatesCount.Remove(steamId);
		m_Incoming.Remove(steamId);
	}

	// one chunk from a client; handles the message once it is complete
	void OnChunk(PlayerBase player, PlayerIdentity sender, int action, int index, int total, int totalBytes, string part)
	{
		string steamId = sender.GetPlainId();
		KPK_Assembler assembler;
		if (!m_Incoming.Find(steamId, assembler))
		{
			assembler = new KPK_Assembler();
			m_Incoming.Set(steamId, assembler);
		}
		// a client message is small; refuse anything absurd
		if (total > 64)
			return;
		string json;
		if (assembler.Add(action, index, total, totalBytes, part, json))
			OnRpc(player, sender, action, json);
	}

	// =================================================================== sending

	void SendRaw(PlayerBase player, int action, string payload)
	{
		if (!player || !player.GetIdentity())
			return;
		KPK_Net.Send(player, action, payload, player.GetIdentity());
	}

	void Msg(PlayerBase player, string text)
	{
		SendRaw(player, KPK_S.MSG, text);
	}

	void Notify(PlayerBase player, string title, string text)
	{
		if (player && player.GetIdentity())
			NotificationSystem.SendNotificationToPlayerIdentityExtended(player.GetIdentity(), 6, title, text, "set:dayz_gui image:icon");
	}

	// refresh every online member of a list who has the KPK open
	void RefreshIds(array<string> ids)
	{
		map<string, PlayerBase> online = OnlinePlayers();
		foreach (string sid : ids)
		{
			PlayerBase p;
			if (online.Find(sid, p) && m_Open.Contains(sid))
				SendView(p);
		}
	}

	void RefreshParty(KPK_Party party)
	{
		if (party)
			RefreshIds(party.Members);
	}

	void RefreshFaction(KPK_Faction faction)
	{
		if (!faction)
			return;
		array<string> ids = new array<string>();
		foreach (KPK_FactionMember m : faction.Members)
			ids.Insert(m.SteamId);
		RefreshIds(ids);
	}

	// factions are visible to everybody: refresh every open KPK
	void RefreshAllOpen()
	{
		RefreshIds(m_Open.GetKeyArray());
	}

	// =================================================================== view building

	void SendView(PlayerBase player)
	{
		if (!player || !player.GetIdentity())
			return;
		KPK_View view = BuildView(player);
		SendRaw(player, KPK_S.VIEW, KPK_Json<KPK_View>.To(view));
	}

	KPK_View BuildView(PlayerBase player)
	{
		string steamId = player.GetIdentity().GetPlainId();
		KPK_View view = new KPK_View();
		view.SteamId = steamId;
		view.ServerName = m_Settings.NetworkName;
		view.ShowSelfOnMap = m_Settings.ShowSelfOnMap;
		view.NoteMaxLength = m_Settings.NoteMaxLength;
		view.Battery = -1;
		EntityAI device = OpenDevice(player, steamId);
		if (m_Settings.RequireBattery && device)
			view.Battery = KPK_Device.GetBatteryLevel(device);
		int model = KPK_Device.ModelOf(device);
		int openedWith;
		if (m_OpenModel.Find(steamId, openedWith))
			model = openedWith;
		view.DeviceModel = model;
		// admins get every function on every device, the brick included, to manage factions
		bool isAdmin = m_Settings.IsAdmin(steamId);
		bool full = KPK_Const.FullAccess(model) || isAdmin;

		KPK_Account acc = GetAccount(steamId);
		view.Registered = acc != null;
		if (!acc)
			return view;

		view.Callsign = acc.Callsign;
		view.NeedNewPassword = acc.PasswordHash == 0;
		view.LoggedIn = m_LoggedIn.Contains(steamId);
		if (!view.LoggedIn)
			return view;

		view.IsAdmin = isAdmin;
		view.RegisteredDate = acc.Registered;
		view.Stats = acc.Stats;
		view.Rank = KPK_Const.Rank(acc.Stats.PlayerKills + acc.Stats.ZombieKills / 20, acc.Stats.PlaytimeSec);

		vector pos = player.GetPosition();
		view.Vitals.Health = player.GetHealth("", "Health");
		view.Vitals.Blood = player.GetHealth("", "Blood");
		view.Vitals.Shock = player.GetHealth("", "Shock");
		view.Vitals.Water = player.GetStatWater().Get();
		view.Vitals.Energy = player.GetStatEnergy().Get();
		view.Vitals.PosX = pos[0];
		view.Vitals.PosZ = pos[2];

		map<string, PlayerBase> online = OnlinePlayers();
		KPK_Faction myFaction = FactionOf(steamId);

		// reputation: one line per faction the player can see (not on the brick)
		foreach (KPK_Faction rf : m_Factions.Factions)
		{
			if (!full)
				break;
			if (rf.Hidden && !isAdmin && rf != myFaction)
				continue;
			if (m_Settings.ReputationFactions.Count() > 0 && m_Settings.ReputationFactions.Find(rf.Id) == -1)
				continue;
			KPK_RepView rep = new KPK_RepView();
			rep.FactionName = rf.Name;
			rep.Color = KPK_Const.Palette(rf.Color);
			rep.Value = GetReputation(acc, rf.Id);
			view.Reputation.Insert(rep);
		}

		// markers
		foreach (KPK_Marker pm : acc.Markers)
			view.Markers.Insert(pm);

		KPK_Party party = PartyOf(steamId);
		if (party)
		{
			foreach (KPK_Marker gm : party.Markers)
				view.Markers.Insert(gm);
		}
		if (myFaction && full)
		{
			foreach (KPK_Marker fm : m_Factions.FactionMarkers)
			{
				if (fm.FactionId == myFaction.Id)
					view.Markers.Insert(fm);
			}
		}

		// party
		view.PartyMax = m_Settings.PartyMaxSize;
		if (party)
		{
			view.InParty = true;
			view.PartyLeader = party.LeaderId == steamId;
			foreach (string mid : party.Members)
			{
				KPK_PartyMemberView mv = new KPK_PartyMemberView();
				mv.Callsign = CallsignOf(mid);
				mv.Leader = mid == party.LeaderId;
				mv.Distance = -1;
				PlayerBase mate;
				if (online.Find(mid, mate))
				{
					mv.Online = true;
					mv.Distance = vector.Distance(pos, mate.GetPosition());
				}
				view.Party.Insert(mv);
			}
		}

		// invites for this player
		foreach (KPK_Invite inv : m_Invites)
		{
			if (inv.ToId != steamId)
				continue;
			if (inv.Kind == 2 && !full)
				continue;
			KPK_InviteView iv = new KPK_InviteView();
			iv.Kind = inv.Kind;
			iv.From = CallsignOf(inv.FromId);
			iv.FactionId = inv.FactionId;
			KPK_Faction invF = FindFaction(inv.FactionId);
			if (invF)
				iv.FactionName = invF.Name;
			view.Invites.Insert(iv);
		}

		// the brick stops here: no factions, tasks or notes
		if (!full)
			return view;

		// factions, with encryption applied for outsiders
		if (myFaction)
		{
			view.MyFactionId = myFaction.Id;
			view.MyRole = myFaction.Find(steamId).Role;
		}
		foreach (KPK_Faction f : m_Factions.Factions)
		{
			bool insider = isAdmin || f == myFaction;
			if (f.Hidden && !insider)
				continue;
			view.Factions.Insert(BuildFactionView(f, insider, isAdmin, f == myFaction, online));
		}

		foreach (KPK_Task t : acc.Tasks)
			view.Tasks.Insert(t);
		foreach (KPK_Note n : acc.Notes)
			view.Notes.Insert(n);

		// chat: the general channel and the player's own private messages, newest 30
		view.ChatMaxLength = m_Settings.ChatMaxLength;
		array<KPK_ChatMessage> visible = new array<KPK_ChatMessage>();
		for (int ci = m_Chat.Count() - 1; ci >= 0 && visible.Count() < 30; ci--)
		{
			KPK_ChatEntry ce = m_Chat[ci];
			if (ce.ToId == "" || ce.ToId == steamId || ce.FromId == steamId)
				visible.Insert(ce.Msg);
		}
		for (int vi = visible.Count() - 1; vi >= 0; vi--)
			view.Chat.Insert(visible[vi]);

		foreach (string contactId : acc.Contacts)
		{
			KPK_ContactView cv = new KPK_ContactView();
			cv.Callsign = CallsignOf(contactId);
			cv.Online = online.Contains(contactId);
			KPK_Account other = GetAccount(contactId);
			if (other)
			{
				cv.Mutual = other.Contacts.Find(steamId) != -1;
				cv.LastSeen = other.LastSeen;
			}
			string tagName;
			int tagColor;
			PublicFaction(contactId, tagName, tagColor);
			cv.Faction = tagName;
			cv.Color = tagColor;
			view.Contacts.Insert(cv);
		}

		return view;
	}

	// the faction other players may see: none for hidden factions or ones with a secret member list
	void PublicFaction(string steamId, out string name, out int color)
	{
		name = "";
		color = 0;
		KPK_Faction f = FactionOf(steamId);
		if (!f || f.Hidden || f.EncryptMembers)
			return;
		name = f.Name;
		color = KPK_Const.Palette(f.Color);
	}

	KPK_FactionView BuildFactionView(KPK_Faction f, bool insider, bool isAdmin, bool mine, map<string, PlayerBase> online)
	{
		KPK_FactionView fv = new KPK_FactionView();
		fv.Id = f.Id;
		fv.Name = f.Name;
		fv.Color = KPK_Const.Palette(f.Color);
		fv.IsMine = mine;
		fv.Hidden = f.Hidden;
		fv.ShowMatesInWorld = f.ShowMatesInWorld;

		if (f.EncryptDescription && !insider)
		{
			fv.Description = KPK_Util.Scramble(f.Description, 24);
			fv.DescriptionEnc = true;
		}
		else
		{
			fv.Description = f.Description;
		}

		string leaderName = "—";
		string leaderId = f.LeaderId();
		if (leaderId != "")
			leaderName = CallsignOf(leaderId);
		if (f.EncryptLeader && !insider)
		{
			fv.Leader = KPK_Util.Scramble(leaderName + f.Id, 10);
			fv.LeaderEnc = true;
		}
		else
		{
			fv.Leader = leaderName;
		}

		if (f.EncryptMembers && !insider)
		{
			fv.MembersEnc = true;
			fv.MemberCount = -1;
		}
		else
		{
			fv.MemberCount = f.Members.Count();
			foreach (KPK_FactionMember m : f.Members)
			{
				KPK_FactionMemberView mv = new KPK_FactionMemberView();
				mv.Callsign = CallsignOf(m.SteamId);
				mv.Role = m.Role;
				mv.Online = online.Contains(m.SteamId);
				fv.Members.Insert(mv);
			}
		}

		if (f.EncryptLocation && !insider)
		{
			fv.Location = KPK_Util.Scramble(f.Location, 16);
			fv.LocationEnc = true;
		}
		else
		{
			fv.Location = f.Location;
			fv.HasLocationPos = f.HasLocationPos;
			fv.LocX = f.LocX;
			fv.LocZ = f.LocZ;
		}

		if (isAdmin)
		{
			fv.FlagDescription = f.EncryptDescription;
			fv.FlagLeader = f.EncryptLeader;
			fv.FlagMembers = f.EncryptMembers;
			fv.FlagLocation = f.EncryptLocation;
		}
		return fv;
	}

	// =================================================================== factions / parties lookup

	KPK_Faction FindFaction(string id)
	{
		if (id == "")
			return null;
		foreach (KPK_Faction f : m_Factions.Factions)
		{
			if (f.Id == id)
				return f;
		}
		return null;
	}

	KPK_Faction FactionOf(string steamId)
	{
		foreach (KPK_Faction f : m_Factions.Factions)
		{
			if (f.Find(steamId))
				return f;
		}
		return null;
	}

	KPK_Party PartyOf(string steamId)
	{
		int partyId;
		if (!m_PartyOf.Find(steamId, partyId))
			return null;
		foreach (KPK_Party p : m_Parties)
		{
			if (p.Id == partyId)
				return p;
		}
		return null;
	}

	protected void RemoveInvites(string toId, int kind)
	{
		for (int i = m_Invites.Count() - 1; i >= 0; i--)
		{
			if (m_Invites[i].ToId == toId && m_Invites[i].Kind == kind)
				m_Invites.RemoveOrdered(i);
		}
	}

	protected KPK_Invite FindInvite(string toId, int kind, string fromId, string factionId)
	{
		foreach (KPK_Invite inv : m_Invites)
		{
			if (inv.ToId != toId || inv.Kind != kind)
				continue;
			if (kind == 1 && inv.FromId == fromId)
				return inv;
			if (kind == 2 && inv.FactionId == factionId)
				return inv;
		}
		return null;
	}

	// =================================================================== public API for other mods

	static int GetReputation(KPK_Account acc, string factionId)
	{
		foreach (KPK_Reputation r : acc.Reputation)
		{
			if (r.FactionId == factionId)
				return r.Value;
		}
		return 0;
	}

	// e.g. KPK_Server.Get().AddReputation(steamId, "duty", 50);
	void AddReputation(string steamId, string factionId, int delta)
	{
		KPK_Account acc = GetAccount(steamId);
		if (!acc)
			return;
		foreach (KPK_Reputation r : acc.Reputation)
		{
			if (r.FactionId == factionId)
			{
				r.Value = r.Value + delta;
				MarkDirty(steamId);
				return;
			}
		}
		KPK_Reputation created = new KPK_Reputation();
		created.FactionId = factionId;
		created.Value = delta;
		acc.Reputation.Insert(created);
		MarkDirty(steamId);
	}

	// e.g. KPK_Server.Get().GiveTask(steamId, "q1", "Найти флешку", "...", "Сидорович", "5000 руб.");
	void GiveTask(string steamId, string id, string title, string description, string issuer, string reward)
	{
		KPK_Account acc = GetAccount(steamId);
		if (!acc)
			return;
		KPK_Task task = new KPK_Task();
		task.Id = KPK_Util.Safe(id);
		task.Title = KPK_Util.Safe(title);
		task.Description = KPK_Util.Safe(description, false);
		task.Issuer = KPK_Util.Safe(issuer);
		task.Reward = KPK_Util.Safe(reward);
		task.Status = 0;
		task.Date = KPK_Util.Now();
		acc.Tasks.Insert(task);
		MarkDirty(steamId);
		PlayerBase p = FindOnline(steamId);
		if (p)
		{
			Notify(p, "КПК: новое задание", title);
			if (m_Open.Contains(steamId))
				SendView(p);
		}
	}

	// status: 0 active, 1 completed, 2 failed
	void SetTaskStatus(string steamId, string id, int status)
	{
		KPK_Account acc = GetAccount(steamId);
		if (!acc)
			return;
		foreach (KPK_Task t : acc.Tasks)
		{
			if (t.Id == id)
			{
				t.Status = status;
				MarkDirty(steamId);
			}
		}
	}

	// =================================================================== broken KPK

	// one-shot: a random grid square from the memory, then the device burns out
	void OnBrokenSwitchedOn(PlayerBase player, ItemBase device)
	{
		if (!player || !device || device.IsRuined())
			return;

		float level = KPK_Device.GetBatteryLevel(device);
		if (level < 0)
		{
			Notify(player, "Разбитый КПК", "Нет питания: вставьте батарейку 9V");
			return;
		}
		if (level <= 0)
		{
			Notify(player, "Разбитый КПК", "Батарейка разряжена");
			return;
		}

		float size = GetGame().GetWorld().GetWorldSize();
		if (size <= 0)
			size = 15360;
		float x = Math.RandomFloat(size * 0.1, size * 0.9);
		float z = Math.RandomFloat(size * 0.1, size * 0.9);
		int gx = x / 100;
		int gz = z / 100;
		string grid = gx.ToStringLen(3) + " " + gz.ToStringLen(3);
		Notify(player, "Разбитый КПК", "…последняя запись… квадрат " + grid + " …СБОЙ… КОРОТКОЕ ЗАМЫКАНИЕ");

		// the coordinate also lands in the player's own KPK, if they have one
		if (player.GetIdentity())
		{
			string steamId = player.GetIdentity().GetPlainId();
			KPK_Account acc = GetAccount(steamId);
			if (acc && acc.Markers.Count() < m_Settings.MaxPersonalMarkers)
			{
				KPK_Marker marker = new KPK_Marker();
				marker.Id = acc.TakeId();
				marker.Name = "Запись разбитого КПК";
				marker.Type = 2;
				marker.Scope = KPK_Scope.PERSONAL;
				marker.X = x;
				marker.Z = z;
				marker.Author = acc.Callsign;
				marker.Date = KPK_Util.Now();
				acc.Markers.Insert(marker);
				MarkDirty(steamId);
			}
		}

		// it blows up in the hands: a small grenade-like blast (visual only), battery drained,
		// device ruined, 10-20 damage and 50 shock to the one holding it
		vector blastPos = device.GetPosition();
		GetGame().RPCSingleParam(player, KPK_RPC_FX, new Param1<vector>(blastPos), true);
		Battery9V battery = KPK_Device.GetBattery(device);
		if (battery && battery.HasEnergyManager())
			battery.GetCompEM().SetEnergy(0);
		device.SetHealth("", "", 0);
		float blastDamage = Math.RandomFloat(10.0, 20.0);
		player.AddHealth("", "Health", -blastDamage);
		player.AddHealth("", "Shock", -50);
		Print("[KPK] broken KPK burnt out, grid " + grid);
	}

	// =================================================================== stats

	void OnPlayerKilled(PlayerBase victim, Object killer)
	{
		if (!victim.GetIdentity())
			return;

		string victimId = victim.GetIdentity().GetPlainId();
		KPK_Account victimAcc = GetAccount(victimId);
		PlayerBase killerPlayer = ResolveKiller(killer);

		if (victimAcc)
		{
			victimAcc.Stats.Deaths = victimAcc.Stats.Deaths + 1;
			if (killerPlayer == victim)
				victimAcc.Stats.Suicides = victimAcc.Stats.Suicides + 1;
			victimAcc.Stats.CurrentLifeSec = 0;
			MarkDirty(victimId);
		}
		m_Open.Remove(victimId);

		if (!killerPlayer || killerPlayer == victim || !killerPlayer.GetIdentity())
			return;

		string killerId = killerPlayer.GetIdentity().GetPlainId();
		KPK_Account killerAcc = GetAccount(killerId);
		if (!killerAcc)
			return;

		killerAcc.Stats.PlayerKills = killerAcc.Stats.PlayerKills + 1;
		float dist = vector.Distance(killerPlayer.GetPosition(), victim.GetPosition());
		if (dist > killerAcc.Stats.LongestKill)
			killerAcc.Stats.LongestKill = dist;
		MarkDirty(killerId);
	}

	void OnCreatureKilled(EntityAI creature, Object killer, bool animal)
	{
		PlayerBase killerPlayer = ResolveKiller(killer);
		if (!killerPlayer || !killerPlayer.GetIdentity())
			return;

		string killerId = killerPlayer.GetIdentity().GetPlainId();
		KPK_Account acc = GetAccount(killerId);
		if (!acc)
			return;

		if (animal)
			acc.Stats.AnimalKills = acc.Stats.AnimalKills + 1;
		else
			acc.Stats.ZombieKills = acc.Stats.ZombieKills + 1;
		MarkDirty(killerId);
	}

	static PlayerBase ResolveKiller(Object killer)
	{
		if (!killer)
			return null;
		PlayerBase direct = PlayerBase.Cast(killer);
		if (direct)
			return direct;
		EntityAI entity = EntityAI.Cast(killer);
		if (entity)
			return PlayerBase.Cast(entity.GetHierarchyRootPlayer());
		return null;
	}

	void TickStats()
	{
		int dt = m_Settings.StatsTickSec;
		map<string, PlayerBase> online = OnlinePlayers();
		foreach (string steamId, PlayerBase player : online)
		{
			if (!player.IsAlive())
				continue;

			// battery drain while the KPK is open
			if (m_Open.Contains(steamId) && m_Settings.RequireBattery)
			{
				EntityAI device = OpenDevice(player, steamId);
				float level = -1;
				if (device)
					level = KPK_Device.Drain(device, m_Settings.BatteryDrainPercentPerMinute * dt / 60.0);
				if (level <= 0)
				{
					m_Open.Remove(steamId);
					SendRaw(player, KPK_S.DENIED, "НЕТ ПИТАНИЯ");
				}
			}

			KPK_Account acc = GetAccount(steamId);
			if (!acc)
				continue;

			acc.Stats.PlaytimeSec = acc.Stats.PlaytimeSec + dt;
			acc.Stats.CurrentLifeSec = acc.Stats.CurrentLifeSec + dt;
			if (acc.Stats.CurrentLifeSec > acc.Stats.BestLifeSec)
				acc.Stats.BestLifeSec = acc.Stats.CurrentLifeSec;

			vector now = player.GetPosition();
			vector last;
			if (m_LastPos.Find(steamId, last))
			{
				float moved = vector.Distance(now, last);
				// a jump this big is a teleport / respawn, not a walk
				if (moved < 60 * dt)
					acc.Stats.DistanceM = acc.Stats.DistanceM + moved;
			}
			m_LastPos.Set(steamId, now);
			MarkDirty(steamId);
		}
	}

	// =================================================================== mates for 3D labels and the map

	void TickMates()
	{
		map<string, PlayerBase> online = OnlinePlayers();
		float maxDist = m_Settings.MatesMaxDistance;

		foreach (string steamId, PlayerBase player : online)
		{
			KPK_MatesMsg msg = new KPK_MatesMsg();
			msg.RequireDevice = m_Settings.LabelsRequireDevice;
			vector myPos = player.GetPosition();

			KPK_Party party = PartyOf(steamId);
			if (party)
			{
				foreach (string mid : party.Members)
				{
					PlayerBase pm;
					if (mid == steamId || !online.Find(mid, pm) || !pm.IsAlive())
						continue;
					AddMate(msg, mid, pm, KPK_MateKind.PARTY, ARGB(255, 110, 230, 90), myPos, maxDist);
				}
			}

			KPK_Faction faction = FactionOf(steamId);
			if (faction && faction.ShowMatesInWorld)
			{
				int color = KPK_Const.Palette(faction.Color);
				foreach (KPK_FactionMember fm : faction.Members)
				{
					PlayerBase fp;
					if (fm.SteamId == steamId || !online.Find(fm.SteamId, fp) || !fp.IsAlive())
						continue;
					if (party && party.Members.Find(fm.SteamId) != -1)
						continue;
					AddMate(msg, fm.SteamId, fp, KPK_MateKind.FACTION, color, myPos, maxDist);
				}
			}

			int lastCount = 0;
			m_LastMatesCount.Find(steamId, lastCount);
			if (msg.Mates.Count() == 0 && lastCount == 0)
				continue;
			m_LastMatesCount.Set(steamId, msg.Mates.Count());
			SendRaw(player, KPK_S.MATES, KPK_Json<KPK_MatesMsg>.To(msg));
		}
	}

	protected void AddMate(KPK_MatesMsg msg, string steamId, PlayerBase p, int kind, int color, vector myPos, float maxDist)
	{
		vector pos = p.GetPosition();
		if (maxDist > 0 && vector.Distance(pos, myPos) > maxDist)
			return;
		KPK_MatePos mate = new KPK_MatePos();
		mate.SteamId = steamId;
		mate.Callsign = CallsignOf(steamId);
		mate.Kind = kind;
		mate.Color = color;
		mate.X = pos[0];
		mate.Y = pos[1];
		mate.Z = pos[2];
		msg.Mates.Insert(mate);
	}

	// =================================================================== RPC entry

	void OnRpc(PlayerBase player, PlayerIdentity sender, int action, string json)
	{
		string steamId = sender.GetPlainId();
		KPK_Req req = new KPK_Req();
		if (json != "" && !KPK_Json<KPK_Req>.From(json, req))
			return;

		switch (action)
		{
			case KPK_C.OPEN:
				HandleOpen(player, steamId);
				return;
			case KPK_C.CLOSE:
				m_Open.Remove(steamId);
				return;
			case KPK_C.REGISTER:
				HandleRegister(player, steamId, req);
				return;
			case KPK_C.LOGIN:
				HandleLogin(player, steamId, req);
				return;
		}

		// everything below needs a logged in account and an open, powered device
		KPK_Account acc = GetAccount(steamId);
		if (!acc || !m_LoggedIn.Contains(steamId) || !m_Open.Contains(steamId))
			return;

		int openModel = KPK_Model.BRICK;
		m_OpenModel.Find(steamId, openModel);
		if (!KPK_Const.FullAccess(openModel) && !m_Settings.IsAdmin(steamId))
		{
			bool blocked = action == KPK_C.NOTE_SAVE || action == KPK_C.NOTE_DEL || action == KPK_C.ADMIN_FACTION;
			blocked = blocked || action == KPK_C.FACTION_INVITE || action == KPK_C.FACTION_RESPOND || action == KPK_C.FACTION_LEAVE;
			blocked = blocked || action == KPK_C.FACTION_KICK || action == KPK_C.FACTION_SETROLE;
			blocked = blocked || action == KPK_C.CHAT_SEND || action == KPK_C.CONTACT_ADD || action == KPK_C.CONTACT_DEL;
			blocked = blocked || ((action == KPK_C.MARKER_ADD || action == KPK_C.MARKER_EDIT || action == KPK_C.MARKER_DEL) && req.I2 == KPK_Scope.FACTION);
			if (blocked)
			{
				Msg(player, "Функция недоступна на этой модели КПК");
				return;
			}
		}

		switch (action)
		{
			case KPK_C.MARKER_ADD: HandleMarkerAdd(player, acc, req); break;
			case KPK_C.MARKER_DEL: HandleMarkerDel(player, acc, req); break;
			case KPK_C.MARKER_EDIT: HandleMarkerEdit(player, acc, req); break;
			case KPK_C.NOTE_SAVE: HandleNoteSave(player, acc, req); break;
			case KPK_C.NOTE_DEL: HandleNoteDel(player, acc, req); break;
			case KPK_C.PARTY_INVITE: HandlePartyInvite(player, acc, req); break;
			case KPK_C.PARTY_RESPOND: HandlePartyRespond(player, acc, req); break;
			case KPK_C.PARTY_LEAVE: LeaveParty(acc.SteamId, true); SendView(player); break;
			case KPK_C.PARTY_KICK: HandlePartyKick(player, acc, req); break;
			case KPK_C.FACTION_INVITE: HandleFactionInvite(player, acc, req); break;
			case KPK_C.FACTION_RESPOND: HandleFactionRespond(player, acc, req); break;
			case KPK_C.FACTION_LEAVE: HandleFactionLeave(player, acc); break;
			case KPK_C.FACTION_KICK: HandleFactionKick(player, acc, req); break;
			case KPK_C.FACTION_SETROLE: HandleFactionSetRole(player, acc, req); break;
			case KPK_C.ADMIN_FACTION: HandleAdmin(player, acc, req); break;
			case KPK_C.CHAT_SEND: HandleChat(player, acc, req); break;
			case KPK_C.CONTACT_ADD: HandleContact(player, acc, req, true); break;
			case KPK_C.CONTACT_DEL: HandleContact(player, acc, req, false); break;
		}
	}

	// =================================================================== chat and contacts

	protected void HandleChat(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		string text = KPK_Util.Clean(req.S1, m_Settings.ChatMaxLength);
		if (text == "")
			return;

		int now = GetGame().GetTime();
		int last;
		if (m_ChatLast.Find(acc.SteamId, last) && now - last < m_Settings.ChatCooldownSec * 1000)
		{
			Msg(player, "Передатчик перегрет, подождите");
			return;
		}

		string toId = "";
		string toName = KPK_Util.Clean(req.S2, 64);
		if (toName != "")
		{
			toId = SteamIdOfCallsign(toName);
			if (toId == "")
			{
				Msg(player, "Позывной «" + toName + "» не найден");
				return;
			}
			if (toId == acc.SteamId)
			{
				Msg(player, "Нельзя писать самому себе");
				return;
			}
			toName = CallsignOf(toId);
		}
		m_ChatLast.Set(acc.SteamId, now);

		KPK_ChatMessage msg = new KPK_ChatMessage();
		msg.Id = m_ChatNextId;
		m_ChatNextId++;
		int year, month, day, hour, minute;
		GetGame().GetWorld().GetDate(year, month, day, hour, minute);
		msg.Time = hour.ToStringLen(2) + ":" + minute.ToStringLen(2);
		msg.From = acc.Callsign;
		msg.To = toName;
		msg.Text = text;
		string tagName;
		int tagColor;
		PublicFaction(acc.SteamId, tagName, tagColor);
		msg.Tag = tagName;
		msg.Color = tagColor;

		KPK_ChatEntry entry = new KPK_ChatEntry();
		entry.FromId = acc.SteamId;
		entry.ToId = toId;
		entry.Msg = msg;
		m_Chat.Insert(entry);
		while (m_Chat.Count() > m_Settings.ChatHistory && m_Chat.Count() > 0)
			m_Chat.RemoveOrdered(0);

		Print("[KPK] chat " + acc.Callsign + " -> " + toName + ": " + text);
		BroadcastChat(entry);
	}

	// open KPKs get the line, closed full KPKs in the inventory show it as a notification
	protected void BroadcastChat(KPK_ChatEntry entry)
	{
		string json = KPK_Json<KPK_ChatMessage>.To(entry.Msg);
		map<string, PlayerBase> online = OnlinePlayers();
		foreach (string sid, PlayerBase p : online)
		{
			if (entry.ToId != "" && sid != entry.ToId && sid != entry.FromId)
				continue;
			if (!GetAccount(sid))
				continue;

			int openModel;
			if (m_Open.Contains(sid) && m_OpenModel.Find(sid, openModel) && (KPK_Const.FullAccess(openModel) || m_Settings.IsAdmin(sid)))
			{
				SendRaw(p, KPK_S.CHAT, json);
				continue;
			}
			if (!m_Settings.ChatNotifyClosed || sid == entry.FromId || !p.IsAlive())
				continue;
			EntityAI device = KPK_Device.FindOn(p, true);
			if (!device || !KPK_Const.FullAccess(KPK_Device.ModelOf(device)))
				continue;
			if (m_Settings.RequireBattery && KPK_Device.GetBatteryLevel(device) <= 0)
				continue;
			// a locked device of somebody else's faction stays silent
			if (!MayUse(sid, KPK_Device.ModelOf(device)))
				continue;
			string title = "КПК: " + entry.Msg.From;
			if (entry.ToId != "")
				title = "КПК: личное от " + entry.Msg.From;
			Notify(p, title, KPK_Util.FromWire(entry.Msg.Text));
		}
	}

	protected void HandleContact(PlayerBase player, KPK_Account acc, KPK_Req req, bool add)
	{
		string name = KPK_Util.Clean(req.S1, 64);
		string targetId = SteamIdOfCallsign(name);
		if (targetId == "")
		{
			Msg(player, "Позывной «" + name + "» не найден");
			return;
		}
		int index = acc.Contacts.Find(targetId);
		if (add)
		{
			if (targetId == acc.SteamId)
			{
				Msg(player, "Это ваш собственный позывной");
				return;
			}
			if (index != -1)
			{
				Msg(player, "Уже в контактах");
				return;
			}
			if (acc.Contacts.Count() >= m_Settings.MaxContacts)
			{
				Msg(player, "Список контактов заполнен");
				return;
			}
			acc.Contacts.Insert(targetId);
			Msg(player, "Контакт «" + CallsignOf(targetId) + "» добавлен");
		}
		else
		{
			if (index == -1)
				return;
			acc.Contacts.RemoveOrdered(index);
			Msg(player, "Контакт «" + CallsignOf(targetId) + "» удалён");
		}
		MarkDirty(acc.SteamId);
		SendView(player);
	}

	// =================================================================== account

	protected void HandleOpen(PlayerBase player, string steamId)
	{
		if (!player.IsAlive())
			return;

		EntityAI device = PickDevice(player, steamId);
		if (!device)
		{
			SendRaw(player, KPK_S.DENIED, "КПК НЕ НАЙДЕН");
			return;
		}
		if (device.IsRuined())
		{
			SendRaw(player, KPK_S.DENIED, "УСТРОЙСТВО НЕИСПРАВНО");
			return;
		}
		if (m_Settings.RequireBattery)
		{
			float level = KPK_Device.GetBatteryLevel(device);
			if (level < 0)
			{
				SendRaw(player, KPK_S.DENIED, "НЕТ ПИТАНИЯ: вставьте батарейку 9V");
				return;
			}
			if (level <= 0)
			{
				SendRaw(player, KPK_S.DENIED, "БАТАРЕЯ РАЗРЯЖЕНА");
				return;
			}
		}

		// faction-locked models: strangers lose the device (and the Monolith one bites back)
		int deviceModel = KPK_Device.ModelOf(device);
		if (!MayUse(steamId, deviceModel))
		{
			PunishStranger(player, ItemBase.Cast(device), deviceModel);
			return;
		}

		if (!m_Settings.PasswordOncePerSession && !m_Open.Contains(steamId))
			m_LoggedIn.Remove(steamId);

		m_Open.Set(steamId, true);
		m_OpenModel.Set(steamId, deviceModel);
		m_OpenDevice.Set(steamId, device);
		SendView(player);
	}

	bool MayUse(string steamId, int model)
	{
		string lockFaction = DeviceFaction(model);
		if (lockFaction == "")
			return true;
		if (m_Settings.AdminsBypassDeviceLock && m_Settings.IsAdmin(steamId))
			return true;
		KPK_Faction f = FactionOf(steamId);
		return f && f.Id == lockFaction;
	}

	// the device in the hands is always the one switched on. Otherwise from the inventory:
	// an own powered one first, then an own unpowered one (power message), a foreign one last
	EntityAI PickDevice(PlayerBase player, string steamId)
	{
		EntityAI inHands = player.GetHumanInventory().GetEntityInHands();
		if (KPK_Device.IsKPK(inHands))
			return inHands;

		EntityAI unpowered = null;
		EntityAI foreign = null;
		array<EntityAI> items = new array<EntityAI>();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI item : items)
		{
			if (!KPK_Device.IsKPK(item) || item.IsRuined())
				continue;
			if (!MayUse(steamId, KPK_Device.ModelOf(item)))
			{
				if (!foreign)
					foreign = item;
				continue;
			}
			if (!m_Settings.RequireBattery || KPK_Device.GetBatteryLevel(item) > 0)
				return item;
			if (!unpowered)
				unpowered = item;
		}
		if (unpowered)
			return unpowered;
		return foreign;
	}

	// the device the KPK was opened with, while it is still on the player
	EntityAI OpenDevice(PlayerBase player, string steamId)
	{
		EntityAI device;
		if (m_OpenDevice.Find(steamId, device) && device && !device.IsRuined() && device.GetHierarchyRootPlayer() == player)
			return device;
		return KPK_Device.FindOn(player, true);
	}

	protected void PunishStranger(PlayerBase player, ItemBase device, int model)
	{
		if (!device)
			return;
		string steamId = player.GetIdentity().GetPlainId();
		m_Open.Remove(steamId);
		Battery9V battery = KPK_Device.GetBattery(device);
		if (battery && battery.HasEnergyManager())
			battery.GetCompEM().SetEnergy(0);

		if (model == KPK_Model.MONOLITH)
		{
			SendRaw(player, KPK_S.DENIED, "ТЫ НЕ ИЗ НАС");
			Notify(player, KPK_Const.ModelName(model), "…ЧУЖОЙ… МОНОЛИТ НЕ ПРИНИМАЕТ ТЕБЯ…");
			GetGame().RPCSingleParam(player, KPK_RPC_FX, new Param1<vector>(device.GetPosition()), true);
			device.SetHealth("", "", 0);
			player.AddHealth("", "Health", -m_Settings.MonolithTrapDamage);
			if (m_Settings.MonolithTrapKnockout)
				player.SetHealth("", "Shock", 0);
			Print("[KPK] Monolith device blew up in the hands of " + steamId);
			return;
		}

		SendRaw(player, KPK_S.DENIED, "ДОСТУП ЗАПРЕЩЁН");
		Notify(player, KPK_Const.ModelName(model), "ДОСТУП ЗАПРЕЩЁН. Несанкционированный доступ: память стёрта, устройство заблокировано.");
		device.SetHealth("", "", 0);
		Print("[KPK] " + KPK_Const.ModelName(model) + " locked out " + steamId);
	}

	protected void HandleRegister(PlayerBase player, string steamId, KPK_Req req)
	{
		if (!m_Open.Contains(steamId))
			return;
		if (GetAccount(steamId))
		{
			Msg(player, "Аккаунт для этого Steam ID уже зарегистрирован");
			return;
		}

		string callsign = KPK_Util.Clean(req.S1, 64);
		string password = req.S2;
		string error = ValidateCallsign(callsign);
		if (error != "")
		{
			Msg(player, error);
			return;
		}
		if (password.LengthUtf8() < m_Settings.PasswordMinLength)
		{
			Msg(player, string.Format("Пароль: минимум %1 символов", m_Settings.PasswordMinLength));
			return;
		}

		KPK_Account acc = new KPK_Account();
		acc.SteamId = steamId;
		acc.Callsign = callsign;
		acc.PasswordHash = KPK_Util.HashPassword(password, m_Settings.PasswordSalt);
		acc.Registered = KPK_Util.Now();
		acc.LastSeen = acc.Registered;
		m_Accounts.Set(steamId, acc);
		SaveAccount(steamId);

		KPK_IndexEntry entry = new KPK_IndexEntry();
		entry.SteamId = steamId;
		entry.Callsign = callsign;
		m_Index.Accounts.Insert(entry);
		SaveIndex();

		m_LoggedIn.Set(steamId, true);
		Print("[KPK] registered " + callsign + " (" + steamId + ")");
		Msg(player, "Регистрация завершена. Добро пожаловать в Зону, " + callsign + ".");
		SendView(player);
	}

	protected string ValidateCallsign(string callsign)
	{
		int len = callsign.LengthUtf8();
		if (len < m_Settings.CallsignMinLength || len > m_Settings.CallsignMaxLength)
			return string.Format("Позывной: от %1 до %2 символов", m_Settings.CallsignMinLength, m_Settings.CallsignMaxLength);
		if (callsign.Contains("\"") || callsign.Contains("\\") || callsign.Contains("<") || callsign.Contains(">"))
			return "Позывной содержит недопустимые символы";
		if (SteamIdOfCallsign(callsign) != "")
			return "Позывной «" + callsign + "» уже занят";
		return "";
	}

	protected void HandleLogin(PlayerBase player, string steamId, KPK_Req req)
	{
		if (!m_Open.Contains(steamId))
			return;
		KPK_Account acc = GetAccount(steamId);
		if (!acc)
			return;

		int now = GetGame().GetTime();
		int lockedUntil = 0;
		if (m_LockedUntil.Find(steamId, lockedUntil) && now < lockedUntil)
		{
			Msg(player, string.Format("Доступ заблокирован. Повторите через %1 с", (lockedUntil - now) / 1000 + 1));
			return;
		}

		string password = req.S1;
		if (acc.PasswordHash == 0)
		{
			// admin reset the password: the first login sets a new one
			if (password.LengthUtf8() < m_Settings.PasswordMinLength)
			{
				Msg(player, string.Format("Новый пароль: минимум %1 символов", m_Settings.PasswordMinLength));
				return;
			}
			acc.PasswordHash = KPK_Util.HashPassword(password, m_Settings.PasswordSalt);
			MarkDirty(steamId);
		}
		else if (KPK_Util.HashPassword(password, m_Settings.PasswordSalt) != acc.PasswordHash)
		{
			int fails = 0;
			m_LoginFails.Find(steamId, fails);
			fails++;
			m_LoginFails.Set(steamId, fails);
			if (fails >= 5)
			{
				m_LoginFails.Set(steamId, 0);
				m_LockedUntil.Set(steamId, now + 60000);
				Msg(player, "Неверный пароль. Доступ заблокирован на 60 с");
			}
			else
			{
				Msg(player, string.Format("Неверный пароль (%1/5)", fails));
			}
			return;
		}

		m_LoginFails.Remove(steamId);
		m_LoggedIn.Set(steamId, true);
		acc.LastSeen = KPK_Util.Now();
		MarkDirty(steamId);
		Msg(player, "Доступ разрешён. " + acc.Callsign + ", на связи.");
		SendView(player);
	}

	// =================================================================== markers

	protected void HandleMarkerAdd(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		KPK_Marker marker = new KPK_Marker();
		marker.Name = KPK_Util.Clean(req.S1, 40);
		marker.Type = Math.Clamp(req.I1, 0, KPK_Const.MARKER_TYPES - 1);
		marker.Scope = req.I2;
		marker.X = req.F1;
		marker.Z = req.F2;
		marker.Author = acc.Callsign;
		marker.Date = KPK_Util.Now();
		if (marker.Name == "")
			marker.Name = KPK_Const.MarkerName(marker.Type);

		switch (marker.Scope)
		{
			case KPK_Scope.PERSONAL:
			{
				if (acc.Markers.Count() >= m_Settings.MaxPersonalMarkers)
				{
					Msg(player, "Память КПК заполнена: удалите старые метки");
					return;
				}
				marker.Id = acc.TakeId();
				acc.Markers.Insert(marker);
				MarkDirty(acc.SteamId);
				SendView(player);
				break;
			}
			case KPK_Scope.PARTY:
			{
				KPK_Party party = PartyOf(acc.SteamId);
				if (!party)
				{
					Msg(player, "Вы не в группе");
					return;
				}
				if (party.Markers.Count() >= m_Settings.MaxPartyMarkers)
				{
					Msg(player, "Слишком много меток группы");
					return;
				}
				marker.Id = party.NextMarkerId;
				party.NextMarkerId++;
				party.Markers.Insert(marker);
				RefreshParty(party);
				break;
			}
			case KPK_Scope.FACTION:
			{
				KPK_Faction faction = FactionOf(acc.SteamId);
				if (!faction)
				{
					Msg(player, "Вы не состоите в группировке");
					return;
				}
				int count = 0;
				foreach (KPK_Marker fm : m_Factions.FactionMarkers)
				{
					if (fm.FactionId == faction.Id)
						count++;
				}
				if (count >= m_Settings.MaxFactionMarkers)
				{
					Msg(player, "Слишком много меток группировки");
					return;
				}
				marker.Id = m_Factions.NextMarkerId;
				m_Factions.NextMarkerId++;
				marker.FactionId = faction.Id;
				m_Factions.FactionMarkers.Insert(marker);
				SaveFactions();
				RefreshFaction(faction);
				break;
			}
		}
	}

	protected void HandleMarkerDel(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		int id = req.I1;
		int i;
		switch (req.I2)
		{
			case KPK_Scope.PERSONAL:
			{
				for (i = 0; i < acc.Markers.Count(); i++)
				{
					if (acc.Markers[i].Id == id)
					{
						acc.Markers.RemoveOrdered(i);
						MarkDirty(acc.SteamId);
						break;
					}
				}
				SendView(player);
				break;
			}
			case KPK_Scope.PARTY:
			{
				KPK_Party party = PartyOf(acc.SteamId);
				if (!party)
					return;
				for (i = 0; i < party.Markers.Count(); i++)
				{
					KPK_Marker pm = party.Markers[i];
					if (pm.Id == id && (pm.Author == acc.Callsign || party.LeaderId == acc.SteamId))
					{
						party.Markers.RemoveOrdered(i);
						break;
					}
				}
				RefreshParty(party);
				break;
			}
			case KPK_Scope.FACTION:
			{
				KPK_Faction faction = FactionOf(acc.SteamId);
				if (!faction)
					return;
				int role = faction.Find(acc.SteamId).Role;
				for (i = 0; i < m_Factions.FactionMarkers.Count(); i++)
				{
					KPK_Marker fm = m_Factions.FactionMarkers[i];
					if (fm.Id != id || fm.FactionId != faction.Id)
						continue;
					if (fm.Author == acc.Callsign || role >= KPK_Role.DEPUTY || m_Settings.IsAdmin(acc.SteamId))
					{
						m_Factions.FactionMarkers.RemoveOrdered(i);
						SaveFactions();
					}
					else
					{
						Msg(player, "Удалять чужие метки может только лидер или заместитель");
					}
					break;
				}
				RefreshFaction(faction);
				break;
			}
		}
	}

	protected void HandleMarkerEdit(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		KPK_Marker marker = null;
		bool allowed = false;
		KPK_Party party = null;
		KPK_Faction faction = null;

		switch (req.I2)
		{
			case KPK_Scope.PERSONAL:
			{
				foreach (KPK_Marker pm : acc.Markers)
				{
					if (pm.Id == req.I1)
						marker = pm;
				}
				allowed = true;
				break;
			}
			case KPK_Scope.PARTY:
			{
				party = PartyOf(acc.SteamId);
				if (!party)
					return;
				foreach (KPK_Marker gm : party.Markers)
				{
					if (gm.Id == req.I1)
						marker = gm;
				}
				if (marker)
					allowed = marker.Author == acc.Callsign || party.LeaderId == acc.SteamId;
				break;
			}
			case KPK_Scope.FACTION:
			{
				faction = FactionOf(acc.SteamId);
				if (!faction)
					return;
				foreach (KPK_Marker fm : m_Factions.FactionMarkers)
				{
					if (fm.Id == req.I1 && fm.FactionId == faction.Id)
						marker = fm;
				}
				if (marker)
					allowed = marker.Author == acc.Callsign || faction.Find(acc.SteamId).Role >= KPK_Role.DEPUTY || m_Settings.IsAdmin(acc.SteamId);
				break;
			}
		}

		if (!marker)
		{
			Msg(player, "Метка не найдена");
			SendView(player);
			return;
		}
		if (!allowed)
		{
			Msg(player, "Изменять чужие метки может только лидер или заместитель");
			return;
		}

		marker.Type = Math.Clamp(req.S2.ToInt(), 0, KPK_Const.MARKER_TYPES - 1);
		marker.Name = KPK_Util.Clean(req.S1, 40);
		if (marker.Name == "")
			marker.Name = KPK_Const.MarkerName(marker.Type);
		if (req.I3 == 1)
		{
			marker.X = req.F1;
			marker.Z = req.F2;
		}
		marker.Date = KPK_Util.Now();

		if (req.I2 == KPK_Scope.PERSONAL)
		{
			MarkDirty(acc.SteamId);
			SendView(player);
		}
		else if (req.I2 == KPK_Scope.PARTY)
		{
			RefreshParty(party);
		}
		else
		{
			SaveFactions();
			RefreshFaction(faction);
		}
		Msg(player, "Метка «" + marker.Name + "» обновлена");
	}

	// =================================================================== notes

	protected void HandleNoteSave(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		string title = KPK_Util.Clean(req.S1, 60);
		string text = KPK_Util.Clean(req.S2, m_Settings.NoteMaxLength, false);
		if (title == "")
			title = "Без названия";

		if (req.I1 > 0)
		{
			foreach (KPK_Note n : acc.Notes)
			{
				if (n.Id == req.I1)
				{
					n.Title = title;
					n.Text = text;
					n.Date = KPK_Util.Now();
					MarkDirty(acc.SteamId);
					Msg(player, "Заметка сохранена");
					SendView(player);
					return;
				}
			}
		}

		if (acc.Notes.Count() >= m_Settings.MaxNotes)
		{
			Msg(player, "Память КПК заполнена: удалите старые заметки");
			return;
		}
		KPK_Note note = new KPK_Note();
		note.Id = acc.TakeId();
		note.Title = title;
		note.Text = text;
		note.Date = KPK_Util.Now();
		acc.Notes.Insert(note);
		MarkDirty(acc.SteamId);
		Msg(player, "Заметка записана");
		SendView(player);
	}

	protected void HandleNoteDel(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		for (int i = 0; i < acc.Notes.Count(); i++)
		{
			if (acc.Notes[i].Id == req.I1)
			{
				acc.Notes.RemoveOrdered(i);
				MarkDirty(acc.SteamId);
				break;
			}
		}
		SendView(player);
	}

	// =================================================================== party

	protected void HandlePartyInvite(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		string targetId = SteamIdOfCallsign(req.S1);
		if (targetId == "" || targetId == acc.SteamId)
		{
			Msg(player, "Сталкер с позывным «" + req.S1 + "» не найден в сети");
			return;
		}
		PlayerBase target = FindOnline(targetId);
		if (!target)
		{
			Msg(player, CallsignOf(targetId) + " сейчас не в сети");
			return;
		}

		KPK_Party party = PartyOf(acc.SteamId);
		if (party)
		{
			if (party.LeaderId != acc.SteamId)
			{
				Msg(player, "Приглашать может только лидер группы");
				return;
			}
			if (party.Members.Count() >= m_Settings.PartyMaxSize)
			{
				Msg(player, "Группа заполнена");
				return;
			}
			if (party.Members.Find(targetId) != -1)
			{
				Msg(player, CallsignOf(targetId) + " уже в вашей группе");
				return;
			}
		}

		if (!FindInvite(targetId, 1, acc.SteamId, ""))
		{
			KPK_Invite inv = new KPK_Invite();
			inv.Kind = 1;
			inv.FromId = acc.SteamId;
			inv.ToId = targetId;
			inv.Time = GetGame().GetTime();
			m_Invites.Insert(inv);
		}

		Msg(player, "Приглашение отправлено: " + CallsignOf(targetId));
		Notify(target, "КПК: приглашение в группу", acc.Callsign + " зовёт вас в группу. Откройте КПК, вкладка «Группа».");
		if (m_Open.Contains(targetId))
			SendView(target);
	}

	protected void HandlePartyRespond(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		string fromId = SteamIdOfCallsign(req.S1);
		KPK_Invite inv = FindInvite(acc.SteamId, 1, fromId, "");
		if (!inv)
		{
			SendView(player);
			return;
		}
		m_Invites.RemoveItem(inv);

		PlayerBase inviter = FindOnline(fromId);
		if (req.I1 == 0)
		{
			if (inviter)
				Msg(inviter, acc.Callsign + " отклонил приглашение в группу");
			SendView(player);
			return;
		}

		KPK_Party party = PartyOf(fromId);
		if (party && party.LeaderId != fromId)
		{
			Msg(player, "Приглашение устарело");
			SendView(player);
			return;
		}
		if (party && party.Members.Count() >= m_Settings.PartyMaxSize)
		{
			Msg(player, "Группа уже заполнена");
			SendView(player);
			return;
		}

		LeaveParty(acc.SteamId, false);

		if (!party)
		{
			party = new KPK_Party();
			party.Id = m_NextPartyId;
			m_NextPartyId++;
			party.LeaderId = fromId;
			party.Members.Insert(fromId);
			m_PartyOf.Set(fromId, party.Id);
			m_Parties.Insert(party);
		}
		party.Members.Insert(acc.SteamId);
		m_PartyOf.Set(acc.SteamId, party.Id);
		RemoveInvites(acc.SteamId, 1);

		foreach (string mid : party.Members)
		{
			PlayerBase mp = FindOnline(mid);
			if (mp && mid != acc.SteamId)
				Msg(mp, acc.Callsign + " вступил в группу");
		}
		SendView(player);
		RefreshParty(party);
	}

	// notify: tell the remaining members
	void LeaveParty(string steamId, bool notify)
	{
		KPK_Party party = PartyOf(steamId);
		if (!party)
			return;

		party.Members.RemoveItem(steamId);
		m_PartyOf.Remove(steamId);

		if (party.Members.Count() <= 1)
		{
			// a party of one is no party
			foreach (string last : party.Members)
			{
				m_PartyOf.Remove(last);
				PlayerBase lp = FindOnline(last);
				if (lp && notify)
					Msg(lp, "Группа распущена");
			}
			array<string> rest = new array<string>();
			rest.Copy(party.Members);
			m_Parties.RemoveItem(party);
			RefreshIds(rest);
			return;
		}

		if (party.LeaderId == steamId)
			party.LeaderId = party.Members[0];

		if (notify)
		{
			string name = CallsignOf(steamId);
			foreach (string mid : party.Members)
			{
				PlayerBase mp = FindOnline(mid);
				if (mp)
					Msg(mp, name + " покинул группу");
			}
		}
		RefreshParty(party);
	}

	protected void HandlePartyKick(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		KPK_Party party = PartyOf(acc.SteamId);
		if (!party || party.LeaderId != acc.SteamId)
			return;
		string targetId = SteamIdOfCallsign(req.S1);
		if (targetId == "" || targetId == acc.SteamId || party.Members.Find(targetId) == -1)
			return;

		PlayerBase target = FindOnline(targetId);
		LeaveParty(targetId, true);
		if (target)
		{
			Msg(target, "Вас исключили из группы");
			if (m_Open.Contains(targetId))
				SendView(target);
		}
		SendView(player);
	}

	// =================================================================== faction membership

	protected void HandleFactionInvite(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		KPK_Faction faction = FactionOf(acc.SteamId);
		if (!faction || faction.Find(acc.SteamId).Role < KPK_Role.DEPUTY)
		{
			Msg(player, "Принимать в группировку могут лидер и заместитель");
			return;
		}
		string targetId = SteamIdOfCallsign(req.S1);
		if (targetId == "")
		{
			Msg(player, "Сталкер «" + req.S1 + "» не зарегистрирован в сети");
			return;
		}
		if (FactionOf(targetId))
		{
			Msg(player, CallsignOf(targetId) + " уже состоит в группировке");
			return;
		}

		// direct enrolment: the player can use the faction KPK right away
		if (m_Settings.FactionDirectAdd)
		{
			RemoveInvites(targetId, 2);
			KPK_FactionMember added = new KPK_FactionMember();
			added.SteamId = targetId;
			added.Role = KPK_Role.MEMBER;
			faction.Members.Insert(added);
			SaveFactions();
			Msg(player, CallsignOf(targetId) + " зачислен в «" + faction.Name + "»");
			PlayerBase enrolled = FindOnline(targetId);
			if (enrolled)
				Notify(enrolled, "КПК: группировка", "Вы зачислены в «" + faction.Name + "». Доступ к КПК группировки открыт.");
			RefreshAllOpen();
			return;
		}

		if (!FindInvite(targetId, 2, "", faction.Id))
		{
			KPK_Invite inv = new KPK_Invite();
			inv.Kind = 2;
			inv.FromId = acc.SteamId;
			inv.ToId = targetId;
			inv.FactionId = faction.Id;
			inv.Time = GetGame().GetTime();
			m_Invites.Insert(inv);
		}

		Msg(player, "Приглашение отправлено: " + CallsignOf(targetId));
		PlayerBase target = FindOnline(targetId);
		if (target)
		{
			Notify(target, "КПК: вербовка", "Группировка «" + faction.Name + "» предлагает вступить. Откройте КПК.");
			if (m_Open.Contains(targetId))
				SendView(target);
		}
	}

	protected void HandleFactionRespond(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		KPK_Invite inv = FindInvite(acc.SteamId, 2, "", req.S1);
		KPK_Faction faction = FindFaction(req.S1);
		if (!inv || !faction)
		{
			SendView(player);
			return;
		}
		m_Invites.RemoveItem(inv);

		if (req.I1 == 0)
		{
			SendView(player);
			return;
		}
		if (FactionOf(acc.SteamId))
		{
			Msg(player, "Сначала покиньте текущую группировку");
			SendView(player);
			return;
		}

		RemoveInvites(acc.SteamId, 2);
		KPK_FactionMember member = new KPK_FactionMember();
		member.SteamId = acc.SteamId;
		member.Role = KPK_Role.MEMBER;
		if (faction.LeaderId() == "")
			member.Role = KPK_Role.LEADER;
		faction.Members.Insert(member);
		SaveFactions();
		Msg(player, "Вы вступили в группировку «" + faction.Name + "»");
		RefreshAllOpen();
	}

	protected void RemoveFromFaction(KPK_Faction faction, string steamId)
	{
		KPK_FactionMember member = faction.Find(steamId);
		if (!member)
			return;
		bool wasLeader = member.Role == KPK_Role.LEADER;
		faction.Members.RemoveItem(member);

		if (wasLeader && faction.Members.Count() > 0)
		{
			// the first deputy takes over, otherwise the first member
			KPK_FactionMember heir = faction.Members[0];
			foreach (KPK_FactionMember m : faction.Members)
			{
				if (m.Role == KPK_Role.DEPUTY)
				{
					heir = m;
					break;
				}
			}
			heir.Role = KPK_Role.LEADER;
		}
		SaveFactions();
	}

	protected void HandleFactionLeave(PlayerBase player, KPK_Account acc)
	{
		KPK_Faction faction = FactionOf(acc.SteamId);
		if (!faction)
			return;
		RemoveFromFaction(faction, acc.SteamId);
		Msg(player, "Вы покинули группировку «" + faction.Name + "»");
		RefreshAllOpen();
	}

	// leader / deputy of the faction, or an admin with S2 = faction id
	protected KPK_Faction ManagedFaction(KPK_Account acc, string adminFactionId, out int role)
	{
		role = KPK_Role.NONE;
		if (m_Settings.IsAdmin(acc.SteamId) && adminFactionId != "")
		{
			role = KPK_Role.LEADER + 1;   // above the leader
			return FindFaction(adminFactionId);
		}
		KPK_Faction faction = FactionOf(acc.SteamId);
		if (faction)
			role = faction.Find(acc.SteamId).Role;
		return faction;
	}

	protected void HandleFactionKick(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		int myRole;
		KPK_Faction faction = ManagedFaction(acc, req.S2, myRole);
		if (!faction || myRole < KPK_Role.DEPUTY)
			return;

		string targetId = SteamIdOfCallsign(req.S1);
		KPK_FactionMember target = faction.Find(targetId);
		if (!target || targetId == acc.SteamId)
			return;
		if (target.Role >= myRole)
		{
			Msg(player, "Недостаточно полномочий");
			return;
		}
		RemoveFromFaction(faction, targetId);
		Msg(player, CallsignOf(targetId) + " исключён из группировки");
		PlayerBase tp = FindOnline(targetId);
		if (tp)
			Msg(tp, "Вас исключили из группировки «" + faction.Name + "»");
		RefreshAllOpen();
	}

	// I1 = new role. Leader: DEPUTY / MEMBER / LEADER (hands leadership over). Admin: anything.
	protected void HandleFactionSetRole(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		int myRole;
		KPK_Faction faction = ManagedFaction(acc, req.S2, myRole);
		if (!faction || myRole < KPK_Role.LEADER)
		{
			Msg(player, "Менять звания может только лидер");
			return;
		}
		string targetId = SteamIdOfCallsign(req.S1);
		KPK_FactionMember target = faction.Find(targetId);
		if (!target)
			return;

		int newRole = Math.Clamp(req.I1, KPK_Role.MEMBER, KPK_Role.LEADER);
		if (newRole == KPK_Role.LEADER)
		{
			foreach (KPK_FactionMember m : faction.Members)
			{
				if (m.Role == KPK_Role.LEADER)
					m.Role = KPK_Role.DEPUTY;
			}
		}
		target.Role = newRole;
		SaveFactions();
		RefreshAllOpen();
	}

	// =================================================================== admin

	protected void HandleAdmin(PlayerBase player, KPK_Account acc, KPK_Req req)
	{
		if (!m_Settings.IsAdmin(acc.SteamId))
		{
			Msg(player, "Нет прав администратора");
			return;
		}

		if (req.I1 == KPK_AdminOp.CREATE)
		{
			string name = KPK_Util.Clean(req.S2, 40);
			if (name == "")
			{
				Msg(player, "Введите название группировки");
				return;
			}
			KPK_Faction created = new KPK_Faction();
			created.Id = string.Format("f%1", Math.RandomInt(100000, 999999));
			while (FindFaction(created.Id))
				created.Id = string.Format("f%1", Math.RandomInt(100000, 999999));
			created.Name = name;
			created.Location = "Неизвестно";
			m_Factions.Factions.Insert(created);
			SaveFactions();
			Msg(player, "Группировка «" + name + "» создана");
			RefreshAllOpen();
			return;
		}

		KPK_Faction f = FindFaction(req.S1);
		if (!f)
			return;

		if (req.I1 == KPK_AdminOp.SET_LEADER || req.I1 == KPK_AdminOp.ADD_MEMBER)
		{
			string targetId = SteamIdOfCallsign(req.S2);
			// a Steam64 ID works too, also for a player who has no account yet
			string raw = req.S2.Trim();
			if (targetId == "" && IsSteamId(raw))
				targetId = raw;
			if (targetId == "")
			{
				Msg(player, "Позывной «" + req.S2 + "» не найден");
				return;
			}
			KPK_Faction current = FactionOf(targetId);
			if (current && current != f)
				RemoveFromFaction(current, targetId);
			KPK_FactionMember member = f.Find(targetId);
			if (!member)
			{
				member = new KPK_FactionMember();
				member.SteamId = targetId;
				member.Role = KPK_Role.MEMBER;
				f.Members.Insert(member);
			}
			if (req.I1 == KPK_AdminOp.SET_LEADER)
			{
				foreach (KPK_FactionMember lm : f.Members)
				{
					if (lm.Role == KPK_Role.LEADER)
						lm.Role = KPK_Role.DEPUTY;
				}
				member.Role = KPK_Role.LEADER;
			}
			SaveFactions();
			RefreshAllOpen();
			return;
		}

		switch (req.I1)
		{
			case KPK_AdminOp.REMOVE:
			{
				for (int i = m_Factions.FactionMarkers.Count() - 1; i >= 0; i--)
				{
					if (m_Factions.FactionMarkers[i].FactionId == f.Id)
						m_Factions.FactionMarkers.RemoveOrdered(i);
				}
				Msg(player, "Группировка «" + f.Name + "» удалена");
				m_Factions.Factions.RemoveItem(f);
				break;
			}
			case KPK_AdminOp.TOGGLE_ENC:
			{
				switch (req.I2)
				{
					case KPK_EncField.DESCRIPTION: f.EncryptDescription = !f.EncryptDescription; break;
					case KPK_EncField.LEADER: f.EncryptLeader = !f.EncryptLeader; break;
					case KPK_EncField.MEMBERS: f.EncryptMembers = !f.EncryptMembers; break;
					case KPK_EncField.LOCATION: f.EncryptLocation = !f.EncryptLocation; break;
				}
				break;
			}
			case KPK_AdminOp.SET_LOCATION:
			{
				string location = KPK_Util.Clean(req.S2, 80);
				if (location != "")
					f.Location = location;
				if (req.I2 == 1)
				{
					vector pos = player.GetPosition();
					f.HasLocationPos = true;
					f.LocX = pos[0];
					f.LocZ = pos[2];
				}
				break;
			}
			case KPK_AdminOp.CLEAR_LOCATION_POS:
				f.HasLocationPos = false;
				break;
			case KPK_AdminOp.SET_DESC:
				f.Description = KPK_Util.Clean(req.S2, 400, false);
				break;
			case KPK_AdminOp.TOGGLE_HIDDEN:
				f.Hidden = !f.Hidden;
				break;
			case KPK_AdminOp.TOGGLE_MATES:
				f.ShowMatesInWorld = !f.ShowMatesInWorld;
				break;
			case KPK_AdminOp.SET_COLOR:
				f.Color = Math.Clamp(req.I2, 0, KPK_Const.PALETTE_SIZE - 1);
				break;
		}

		SaveFactions();
		RefreshAllOpen();
	}
};
