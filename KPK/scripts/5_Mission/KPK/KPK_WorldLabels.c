// Callsign + distance above party / faction mates. Positions come from the server
// (players outside the network bubble are not streamed); nearby players use the live position.
class KPK_WorldLabels
{
	protected ref map<string, Widget> m_Labels;
	protected ref map<string, PlayerBase> m_Nearby;
	protected float m_ScanTime;
	protected bool m_HasDevice = true;

	void KPK_WorldLabels()
	{
		m_Labels = new map<string, Widget>();
		m_Nearby = new map<string, PlayerBase>();
	}

	void ~KPK_WorldLabels()
	{
		Clear();
	}

	void Clear()
	{
		foreach (string id, Widget w : m_Labels)
		{
			if (w)
				w.Unlink();
		}
		m_Labels.Clear();
	}

	protected void Scan(PlayerBase me)
	{
		m_Nearby.Clear();
		array<Man> men = new array<Man>();
		GetGame().GetPlayers(men);
		foreach (Man man : men)
		{
			PlayerBase p = PlayerBase.Cast(man);
			if (p && p != me && p.GetIdentity())
				m_Nearby.Set(p.GetIdentity().GetPlainId(), p);
		}
		m_HasDevice = KPK_Device.FindOn(me) != null;
	}

	protected Widget LabelFor(KPK_MatePos mate)
	{
		Widget w;
		if (m_Labels.Find(mate.SteamId, w) && w)
			return w;
		w = GetGame().GetWorkspace().CreateWidgets(KPK_UI.DIR + "kpk_label.layout");
		TextWidget name = TextWidget.Cast(w.FindAnyWidget("name"));
		TextWidget dist = TextWidget.Cast(w.FindAnyWidget("dist"));
		name.SetOutline(1, 0xFF000000);
		dist.SetOutline(1, 0xFF000000);
		m_Labels.Set(mate.SteamId, w);
		return w;
	}

	void Update(float timeslice)
	{
		PlayerBase me = PlayerBase.Cast(GetGame().GetPlayer());
		if (!me || !me.IsAlive())
		{
			Clear();
			return;
		}

		m_ScanTime -= timeslice;
		if (m_ScanTime <= 0)
		{
			m_ScanTime = 0.5;
			Scan(me);
		}

		KPK_MatesMsg msg = KPK_Client.s_Mates;
		bool visible = KPK_Client.s_LabelsEnabled && GetGame().GetUIManager().GetMenu() == null;
		if (msg.RequireDevice && !m_HasDevice)
			visible = false;

		// drop labels of mates that are gone
		array<string> keep = new array<string>();
		foreach (KPK_MatePos km : msg.Mates)
			keep.Insert(km.SteamId);
		array<string> ids = m_Labels.GetKeyArray();
		foreach (string oldId : ids)
		{
			if (keep.Find(oldId) == -1)
			{
				Widget old = m_Labels.Get(oldId);
				if (old)
					old.Unlink();
				m_Labels.Remove(oldId);
			}
		}

		vector cam = GetGame().GetCurrentCameraPosition();
		int sw, sh;
		GetScreenSize(sw, sh);

		foreach (KPK_MatePos mate : msg.Mates)
		{
			Widget label = LabelFor(mate);
			if (!visible)
			{
				label.Show(false);
				continue;
			}

			vector pos = Vector(mate.X, mate.Y, mate.Z);
			// live position of streamed players: by network id (the plain Steam id of other players is
			// often empty on a client), then by id; far away ones use the server position
			PlayerBase live = PlayerBase.Cast(GetGame().GetObjectByNetworkId(mate.NetLow, mate.NetHigh));
			if (!live)
				m_Nearby.Find(mate.SteamId, live);
			if (live)
				pos = live.GetPosition();
			pos[1] = pos[1] + 2.0;

			vector screen = GetGame().GetScreenPos(pos);
			if (screen[2] <= 0 || screen[0] < 0 || screen[1] < 0 || screen[0] > sw || screen[1] > sh)
			{
				label.Show(false);
				continue;
			}

			int distance = vector.Distance(cam, pos);
			TextWidget name = TextWidget.Cast(label.FindAnyWidget("name"));
			TextWidget dist = TextWidget.Cast(label.FindAnyWidget("dist"));
			name.SetText(mate.Callsign);
			name.SetColor(mate.Color);
			dist.SetText(distance.ToString() + " м");

			float lw, lh;
			label.GetScreenSize(lw, lh);
			label.SetPos(screen[0] - lw / 2, screen[1] - lh);
			label.Show(true);
		}
	}
};
