// Client side: last snapshot from the server, send helpers, events for the UI (5_Mission).
class KPK_Client
{
	static ref KPK_View s_View;
	static ref KPK_MatesMsg s_Mates = new KPK_MatesMsg();
	static string s_Status = "";
	static bool s_WantOpen;
	static bool s_LabelsEnabled = true;
	static bool s_ContactNeedsHands = true;   // server setting, comes with the view

	static ref KPK_Assembler s_Assembler = new KPK_Assembler();

	static ref ScriptInvoker s_OnView = new ScriptInvoker();     // ()
	static ref ScriptInvoker s_OnOpen = new ScriptInvoker();     // () server allowed to open
	static ref ScriptInvoker s_OnDenied = new ScriptInvoker();   // (string reason)
	static ref ScriptInvoker s_OnStatus = new ScriptInvoker();   // (string text)
	static ref ScriptInvoker s_OnMates = new ScriptInvoker();    // ()
	static ref ScriptInvoker s_OnChat = new ScriptInvoker();     // (KPK_ChatMessage) new line arrived
	static ref ScriptInvoker s_OnNotify = new ScriptInvoker();   // (string title, string text, float seconds, bool sound)
	static ref ScriptInvoker s_OnSound = new ScriptInvoker();    // (string name) an interface sound

	// a KPK notification on the left of the screen (drawn in 5_Mission by KPK_Toast)
	static void Toast(string title, string text, float seconds = 5, bool sound = true)
	{
		s_OnNotify.Invoke(title, text, seconds, sound);
	}

	static void Send(int action, KPK_Req req = null)
	{
		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (!player)
			return;

		string json = "";
		if (req)
			json = KPK_Json<KPK_Req>.To(req);

		KPK_Net.Send(player, action, json);
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
				{
					Print("[KPK] could not read the view, " + json.Length() + " bytes");
					// the log line is cut at ~8 KB: print the text in pieces so the broken spot is visible
					for (int off = 0; off < json.Length(); off += 3000)
					{
						int len = json.Length() - off;
						if (len > 3000)
							len = 3000;
						Print("[KPK] view part " + (off / 3000) + ": " + json.Substring(off, len));
					}
					s_WantOpen = false;
					s_OnDenied.Invoke("ОШИБКА ДАННЫХ: см. script.log");
					return;
				}
				s_View = view;
				s_ContactNeedsHands = view.ContactNeedsHands;
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
			case KPK_S.TEST_VOICE:
			{
				// the test subscriber answers: a radio beep and the line as a card
				s_OnSound.Invoke("beep2");
				Toast("ТЕСТ (эфир)", json, 9, false);
				// and how the radio looks on this client (the speaking client decides what goes on air)
				PlayerBase me = PlayerBase.Cast(GetGame().GetPlayer());
				if (me && json.IndexOf("Принял") == 0)
					KPK_Device.RadioReport(me);
				break;
			}
			case KPK_S.NOTIFY:
			{
				KPK_NotifyMsg note = new KPK_NotifyMsg();
				if (KPK_Json<KPK_NotifyMsg>.From(json, note))
					Toast(note.Title, note.Text, note.Seconds, true);
				break;
			}
			case KPK_S.CHAT:
			{
				KPK_ChatMessage line = new KPK_ChatMessage();
				if (!KPK_Json<KPK_ChatMessage>.From(json, line))
					return;
				if (s_View)
				{
					s_View.Chat.Insert(line);
					while (s_View.Chat.Count() > Math.Max(40, s_View.ChatHistory))
						s_View.Chat.RemoveOrdered(0);
				}
				s_OnChat.Invoke(line);
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
