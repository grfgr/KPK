// Transport: DayZ fails to read long strings in one RPC ("String CORRUPTED"), so every message is cut
// into chunks and glued back on arrival. Chunks are cut by BYTES (Length/Substring): the UTF-8 helpers
// (LengthUtf8/SubstringUtf8) miscounted long strings and silently dropped the end of the view.
// A cut is made right after an ASCII separator, so it never lands inside a multi-byte character.
// Wire format: Param5<int action, int index, int total, int totalBytes, string part>
class KPK_Net
{
	static const int CHUNK_BYTES = 400;

	// first position >= from right after an ASCII separator. ASCII bytes never occur inside a
	// multi-byte UTF-8 character, so cutting after one never splits a letter. -1 = none found.
	static int SafeCut(string s, int from)
	{
		int best = -1;
		array<string> seps = {",", "\"", " ", ":", "}"};
		foreach (string sep : seps)
		{
			int at = s.IndexOfFrom(from, sep);
			if (at >= 0 && (best < 0 || at < best))
				best = at;
		}
		if (best < 0)
			return -1;
		return best + 1;
	}

	static void Send(Object target, int action, string payload, PlayerIdentity recipient = null)
	{
		int len = payload.Length();
		array<string> parts = new array<string>();
		int start = 0;
		while (start < len)
		{
			int cut = start + CHUNK_BYTES;
			if (cut >= len)
			{
				cut = len;
			}
			else
			{
				cut = SafeCut(payload, cut);
				if (cut < 0 || cut > len)
					cut = len;
			}
			parts.Insert(payload.Substring(start, cut - start));
			start = cut;
		}
		if (parts.Count() == 0)
			parts.Insert("");

		int total = parts.Count();
		for (int i = 0; i < total; i++)
			GetGame().RPCSingleParam(target, KPK_RPC, new Param5<int, int, int, int, string>(action, i, total, len, parts[i]), true, recipient);
	}
};

// Collects the chunks of one sender. Chunks of one message arrive in order on the guaranteed channel.
class KPK_Assembler
{
	protected int m_Action = -1;
	protected int m_Total;
	protected ref array<string> m_Parts = new array<string>();

	// true when the message is complete; the text is in result
	bool Add(int action, int index, int total, int totalBytes, string part, out string result)
	{
		if (total <= 1)
		{
			result = part;
			return Check(action, result, totalBytes);
		}

		if (index == 0 || action != m_Action || total != m_Total)
		{
			m_Action = action;
			m_Total = total;
			m_Parts.Clear();
		}
		if (index != m_Parts.Count())
		{
			Print("[KPK] chunk out of order: action " + action + ", got " + index + ", expected " + m_Parts.Count());
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
		return Check(action, result, totalBytes);
	}

	protected bool Check(int action, string text, int totalBytes)
	{
		if (text.Length() == totalBytes)
			return true;
		Print("[KPK] message " + action + " has " + text.Length() + " bytes, expected " + totalBytes);
		return false;
	}
};
