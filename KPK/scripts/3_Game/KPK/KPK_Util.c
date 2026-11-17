class KPK_Util
{
	static string Now()
	{
		int y, mo, d, h, mi, s;
		GetYearMonthDay(y, mo, d);
		GetHourMinuteSecond(h, mi, s);
		return string.Format("%1.%2.%3 %4:%5", d.ToStringLen(2), mo.ToStringLen(2), y, h.ToStringLen(2), mi.ToStringLen(2));
	}

	static string Duration(int sec)
	{
		int hours = sec / 3600;
		int minutes = (sec % 3600) / 60;
		if (hours > 0)
			return string.Format("%1 ч %2 мин", hours, minutes);
		return string.Format("%1 мин", minutes);
	}

	static string Km(float meters)
	{
		int whole = meters / 1000;
		int tenths = (meters - whole * 1000) / 100;
		return string.Format("%1.%2 км", whole, tenths);
	}

	// cut to a character (not byte) limit and strip characters that break one-line widgets
	static string Clean(string text, int maxChars, bool singleLine = true)
	{
		string result = text.Trim();
		if (singleLine)
		{
			result.Replace("\n", " ");
			result.Replace("\r", " ");
		}
		if (result.LengthUtf8() > maxChars)
			result = result.SubstringUtf8(0, maxChars);
		return result;
	}

	static string Lower(string text)
	{
		string result = text;
		result.ToLower();
		return result;
	}

	static int HashPassword(string password, string salt)
	{
		string s = salt + ":" + password + ":" + salt;
		int h = s.Hash();
		// 0 is reserved for "no password set"
		if (h == 0)
			h = 1;
		return h;
	}

	// Garbage of about the same length as the hidden text; the real text never leaves the server.
	static string Scramble(string seedText, int minLen = 8)
	{
		string charset = "#%&@$?!/\\<>=+*01";
		int len = seedText.LengthUtf8();
		if (len < minLen)
			len = minLen;
		if (len > 40)
			len = 40;
		int seed = seedText.Hash();
		string result = "";
		for (int i = 0; i < len; i++)
		{
			seed = seed * 1103515245 + 12345;
			int idx = (seed >> 16) & 15;
			result += charset.Get(idx);
		}
		return result;
	}
};

// JSON over RPC: the template keeps the real type for the serializer
class KPK_Json<Class T>
{
	static string To(T data)
	{
		string text;
		JsonSerializer js = new JsonSerializer();
		js.WriteToString(data, false, text);
		return text;
	}

	static bool From(string text, T data)
	{
		string error;
		JsonSerializer js = new JsonSerializer();
		if (!js.ReadFromString(data, text, error))
		{
			Print("[KPK] JSON error: " + error);
			return false;
		}
		return true;
	}
};
