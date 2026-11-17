// Transport: DayZ fails to read long strings in one RPC ("String CORRUPTED"), so every message
// is cut into short chunks (by characters, never inside a UTF-8 character) and glued back on arrival.
// Wire format: Param4<int action, int index, int total, string part>
class KPK_Net
{
	static const int CHUNK_CHARS = 200;

	static void Send(Object target, int action, string payload, PlayerIdentity recipient = null)
	{
		int len = payload.LengthUtf8();
		int total = (len + CHUNK_CHARS - 1) / CHUNK_CHARS;
		if (total < 1)
			total = 1;

		for (int i = 0; i < total; i++)
		{
			int start = i * CHUNK_CHARS;
			int count = len - start;
			if (count > CHUNK_CHARS)
				count = CHUNK_CHARS;
			string part = "";
			if (count > 0)
				part = payload.SubstringUtf8(start, count);
			GetGame().RPCSingleParam(target, KPK_RPC, new Param4<int, int, int, string>(action, i, total, part), true, recipient);
		}
	}
};

// Collects the chunks of one sender. Chunks of one message arrive in order on the guaranteed channel.
class KPK_Assembler
{
	protected int m_Action = -1;
	protected int m_Total;
	protected ref array<string> m_Parts = new array<string>();

	// true when the message is complete; the text is in result
	bool Add(int action, int index, int total, string part, out string result)
	{
		if (total <= 1)
		{
			result = part;
			return true;
		}

		if (index == 0 || action != m_Action || total != m_Total)
		{
			m_Action = action;
			m_Total = total;
			m_Parts.Clear();
		}
		if (index != m_Parts.Count())
		{
			// lost or out-of-order chunk: drop this message
			m_Action = -1;
			m_Parts.Clear();
			return false;
		}

		m_Parts.Insert(part);
		if (m_Parts.Count() < total)
			return false;

		result = "";
		foreach (string p : m_Parts)
			result += p;
		m_Action = -1;
		m_Parts.Clear();
		return true;
	}
};
