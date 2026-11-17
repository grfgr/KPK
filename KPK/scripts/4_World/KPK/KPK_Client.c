// Client side: last snapshot from the server, send helpers, events for the UI (5_Mission).
class KPK_Client
{
	static ref KPK_View s_View;
	static ref KPK_MatesMsg s_Mates = new KPK_MatesMsg();
	static string s_Status = "";
	static bool s_WantOpen;
	static bool s_LabelsEnabled = true;

	static ref ScriptInvoker s_OnView = new ScriptInvoker();     // ()
	static ref ScriptInvoker s_OnOpen = new ScriptInvoker();     // () server allowed to open
	static ref ScriptInvoker s_OnDenied = new ScriptInvoker();   // (string reason)
	static ref ScriptInvoker s_OnStatus = new ScriptInvoker();   // (string text)
	static ref ScriptInvoker s_OnMates = new ScriptInvoker();    // ()

	static void Send(int action, KPK_Req req = null)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		string json = "";
		if (req)
			json = KPK_Json<KPK_Req>.To(req);

		GetGame().RPCSingleParam(player, KPK_RPC, new Param2<int, string>(action, json), true);
	}

	static void SendText(int action, string s1, string s2 = "", int i1 = 0, int i2 = 0)
	{
		KPK_Req req = new KPK_Req();
		req.S1 = s1;
		req.S2 = s2;
		req.I1 = i1;
		req.I2 = i2;
		Send(action, req);
	}

	static void RequestOpen()
	{
		s_WantOpen = true;
		Send(KPK_C.OPEN);
	}

	static void NotifyClosed()
	{
		s_WantOpen = false;
		Send(KPK_C.CLOSE);
	}

	static void OnRpc(int action, string json)
	{
		switch (action)
		{
			case KPK_S.VIEW:
			{
				KPK_View view = new KPK_View();
				if (!KPK_Json<KPK_View>.From(json, view))
					return;
				s_View = view;
				s_OnView.Invoke();
				if (s_WantOpen)
				{
					s_WantOpen = false;
					s_OnOpen.Invoke();
				}
				break;
			}
			case KPK_S.MATES:
			{
				KPK_MatesMsg mates = new KPK_MatesMsg();
				if (!KPK_Json<KPK_MatesMsg>.From(json, mates))
					return;
				s_Mates = mates;
				s_OnMates.Invoke();
				break;
			}
			case KPK_S.MSG:
			{
				s_Status = json;
				s_OnStatus.Invoke(json);
				break;
			}
			case KPK_S.DENIED:
			{
				s_WantOpen = false;
				s_OnDenied.Invoke(json);
				break;
			}
		}
	}
};
