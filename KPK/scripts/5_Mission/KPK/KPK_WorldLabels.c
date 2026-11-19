// Callsign + distance above party / faction mates. Positions come from the server
// (players outside the network bubble are not streamed); nearby players use the live position.
class KPK_WorldLabels
{
	protected ref map<string, Widget> m_Labels;
	protected ref map<string, PlayerBase> m_Nearby;
	protected float m_ScanTime;
	protected float m_SightTime;
	protected ref map<string, bool> m_InSight;
	protected bool m_HasDevice = true;

	void KPK_WorldLabels()
	{
		m_Labels = new map<string, Widget>();
		m_Nearby = new map<string, PlayerBase>();
		m_InSight = new map<string, bool>();
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

	// line of sight from the camera to the mate's head or chest: walls, terrain, trees hide the label.
	// Only for streamed players (the live entity); the mate himself and what he carries do not block.
	protected bool InSight(PlayerBase me, PlayerBase mate)
	{
		if (!mate)
			return false;
		vector cam = GetGame().GetCurrentCameraPosition();
		vector base = mate.GetPosition();
		for (int k = 0; k < 2; k++)
		{
			vector target = base;
			target[1] = target[1] + 1.65 - k * 0.5;
			vector hitPos, hitDir;
			int hitComp;
			set<Object> hits = new set<Object>();
			if (!DayZPhysics.RaycastRV(cam, target, hitPos, hitDir, hitComp, hits, null, me, false, false, ObjIntersectView, 0.0))
				return true;
			if (vector.Distance(cam, hitPos) > vector.Distance(cam, target) - 0.4)
				return true;
			bool blocked = hits.Count() == 0;   // terrain
			for (int h = 0; h < hits.Count(); h++)
			{
				Object hit = hits.Get(h);
				if (hit == mate)
					continue;
				EntityAI hitEntity = EntityAI.Cast(hit);
				if (hitEntity && hitEntity.GetHierarchyRootPlayer() == mate)
					continue;
				blocked = true;
			}
			if (!blocked)
				return true;
		}
		return false;
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

		m_SightTime -= timeslice;
		bool checkSight = m_SightTime <= 0;
		if (checkSight)
			m_SightTime = 0.25;

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
			if (msg.LabelMaxDistance > 0 && distance > msg.LabelMaxDistance)
			{
				label.Show(false);
				continue;
			}
			// behind a wall: hidden (checked a few times a second)
			if (msg.LabelLineOfSight)
			{
				bool seen;
				if (checkSight || !m_InSight.Find(mate.SteamId, seen))
				{
					seen = InSight(me, live);
					m_InSight.Set(mate.SteamId, seen);
				}
				if (!seen)
				{
					label.Show(false);
					continue;
				}
			}
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
