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
	ref map<string, int> m_LoginFails;
	ref map<string, int> m_LockedUntil;           // GetTime() ms
	ref map<string, vector> m_LastPos;
	ref map<string, int> m_LastMatesCount;

	ref array<ref KPK_Party> m_Parties;
	ref map<string, int> m_PartyOf;               // steamId -> party id
	int m_NextPartyId = 1;
	ref array<ref KPK_Invite> m_Invites;
	ref map<string, ref KPK_Assembler> m_Incoming;   // chunked client messages per sender

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
		m_LoginFails = new map<string, int>();
		m_LockedUntil = new map<string, int>();
		m_LastPos = new map<string, vector>();
		m_LastMatesCount = new map<string, int>();
		m_Parties = new array<ref KPK_Party>();
		m_PartyOf = new map<string, int>();
		m_Invites = new array<ref KPK_Invite>();
		m_Incoming = new map<string, ref KPK_Assembler>();

		m_Settings = KPK_Settings.Load();
		LoadIndex();
		LoadFactions();

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
	}

	protected void CreateExampleFactions()
	{
		AddExampleFaction("loners", "Одиночки", "Вольные сталкеры. Каждый сам за себя, но своих не бросают.", 0, "Деревня новичков, Кордон", false);
		AddExampleFaction("duty", "Долг", "Военизированная группировка. Цель: защитить мир от Зоны.", 1, "Бар «100 рентген», Росток", false);
		AddExampleFaction("freedom", "Свобода", "Анархисты. Зона должна быть открыта для всех.", 2, "Военные склады", false);
		AddExampleFaction("mercs", "Наёмники", "Работают за деньги. Заказчики неизвестны.", 3, "Неизвестно", true);
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
		if (m_Settings.RequireBattery)
		{
			EntityAI device = KPK_Device.FindOn(player, true);
			if (device)
				view.Battery = KPK_Device.GetBatteryLevel(device);
		}

		KPK_Account acc = GetAccount(steamId);
		view.Registered = acc != null;
		if (!acc)
			return view;

		view.Callsign = acc.Callsign;
		view.NeedNewPassword = acc.PasswordHash == 0;
		view.LoggedIn = m_LoggedIn.Contains(steamId);
		if (!view.LoggedIn)
			return view;

		bool isAdmin = m_Settings.IsAdmin(steamId);
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

		// reputation: one line per faction the player can see
		foreach (KPK_Faction rf : m_Factions.Factions)
		{
			if (rf.Hidden && !isAdmin && rf != myFaction)
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
		if (myFaction)
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
			KPK_InviteView iv = new KPK_InviteView();
			iv.Kind = inv.Kind;
			iv.From = CallsignOf(inv.FromId);
			iv.FactionId = inv.FactionId;
			KPK_Faction invF = FindFaction(inv.FactionId);
			if (invF)
				iv.FactionName = invF.Name;
			view.Invites.Insert(iv);
		}

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

		return view;
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
				EntityAI device = KPK_Device.FindOn(player, true);
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
		}
	}

	// =================================================================== account

	protected void HandleOpen(PlayerBase player, string steamId)
	{
		if (!player.IsAlive())
			return;

		EntityAI device = KPK_Device.FindOn(player, true);
		if (!device)
		{
			SendRaw(player, KPK_S.DENIED, "КПК НЕ НАЙДЕН");
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

		if (!m_Settings.PasswordOncePerSession && !m_Open.Contains(steamId))
			m_LoggedIn.Remove(steamId);

		m_Open.Set(steamId, true);
		SendView(player);
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
