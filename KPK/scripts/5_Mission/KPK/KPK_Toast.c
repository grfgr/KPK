// KPK notifications: small PDA-style cards on the left of the screen, a little below the centre, newest on
// top; they slide in with the message sound and slide out after a few seconds (at most four at a time).
class KPK_ToastItem
{
	Widget m_Root;
	float m_Left;
	float m_Age;
};

class KPK_Toast
{
	static const float X = 0.012;
	static const float Y = 0.56;
	static const float W = 0.23;
	static const float H = 0.072;
	static const float GAP = 0.008;
	static const float SLIDE = 0.18;
	static ref array<ref KPK_ToastItem> s_Items = new array<ref KPK_ToastItem>();

	static void Show(string title, string text, float seconds, bool sound)
	{
		if (!GetGame() || GetGame().IsDedicatedServer())
			return;
		Widget root = KPK_UI.Frame(null, -W, Y, W, H);
		if (!root)
			return;
		root.SetSort(1000);
		root.SetFlags(WidgetFlags.IGNOREPOINTER);
		KPK_UI.Panel(root, 0, 0, 1, 1, 0xE6081209);
		KPK_UI.Panel(root, 0, 0, 0.012, 1, 0xFFE6AA3C);
		KPK_UI.Panel(root, 0.012, 0, 0.988, 0.03, 0xFF5A8C4D);
		KPK_UI.Panel(root, 0.012, 0.97, 0.988, 0.03, 0x805A8C4D);
		KPK_UI.Text(root, title, 0.04, 0.06, 0.92, 0.34, 17, 0xFFE6AA3C);
		MultilineTextWidget body = KPK_UI.MText(root, 0.04, 0.42, 0.92, 0.54, 15, 0xFFC8E8B8);
		if (body)
			body.SetText(text);

		KPK_ToastItem item = new KPK_ToastItem();
		item.m_Root = root;
		item.m_Left = Math.Max(seconds, 2.0);
		s_Items.InsertAt(item, 0);
		while (s_Items.Count() > 4)
		{
			KPK_ToastItem old = s_Items[s_Items.Count() - 1];
			if (old.m_Root)
				old.m_Root.Unlink();
			s_Items.Remove(s_Items.Count() - 1);
		}
		if (sound)
			KPK_Sound.Play("sms");
		Layout();
	}

	static void Update(float dt)
	{
		if (s_Items.Count() == 0)
			return;
		for (int i = s_Items.Count() - 1; i >= 0; i--)
		{
			KPK_ToastItem it = s_Items[i];
			it.m_Age += dt;
			it.m_Left -= dt;
			if (it.m_Left <= -SLIDE || !it.m_Root)
			{
				if (it.m_Root)
					it.m_Root.Unlink();
				s_Items.Remove(i);
			}
		}
		Layout();
	}

	// slide in from the left, slide back out at the end; stacked downwards from Y
	protected static void Layout()
	{
		for (int i = 0; i < s_Items.Count(); i++)
		{
			KPK_ToastItem it = s_Items[i];
			if (!it.m_Root)
				continue;
			float k = 1.0;
			if (it.m_Age < SLIDE)
				k = it.m_Age / SLIDE;
			if (it.m_Left < 0)
				k = Math.Max(0.0, 1 + it.m_Left / SLIDE);
			k = 1 - (1 - k) * (1 - k);
			it.m_Root.SetPos(-W + (X + W) * k, Y + i * (H + GAP));
		}
	}

	static void Clear()
	{
		foreach (KPK_ToastItem it : s_Items)
		{
			if (it.m_Root)
				it.m_Root.Unlink();
		}
		s_Items.Clear();
	}
};
