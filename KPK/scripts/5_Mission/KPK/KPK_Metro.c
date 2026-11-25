// Карта метро на КПК «кирпич»: линии и входы берутся у мода Drezina, если он установлен
// (он объявляет define DREZINA). Без него КПК собирается как обычно, а карта метро пишет «нет данных».
class KPK_Metro
{
	static const int C_DIM = 0x9E000000;        // затемнение обычной карты
	static const int C_LINE_WIDE = 0x8C2F7DFF;  // полупрозрачная синяя линия
	static const int C_LINE_CORE = 0xB89CCBFF;  // светлая середина линии
	static const int C_ENTRANCE = 0xFF4FA3FF;   // кольцо входа
	static const int C_LETTER = 0xFFE8F0FF;     // буква «М»

	static bool Available()
	{
	#ifdef DREZINA
		return true;
	#else
		return false;
	#endif
	}

	static void Collect(array<ref array<vector>> lines, array<vector> entrances)
	{
	#ifdef DREZINA
		DrezinaMetroMap.Collect(lines, entrances, 20.0);
	#endif
	}

	// --- рисование на CanvasWidget (координаты — пиксели внутри канвы)

	protected static int Code(float x, float y, float w, float h)
	{
		int c = 0;
		if (x < 0) c |= 1;
		if (x > w) c |= 2;
		if (y < 0) c |= 4;
		if (y > h) c |= 8;
		return c;
	}

	// отрезок, обрезанный по прямоугольнику канвы (Коэн — Сазерленд), чтобы линии не вылезали за карту
	static void Line(CanvasWidget cv, float x1, float y1, float x2, float y2, float w, float h, float width, int color)
	{
		int c1 = Code(x1, y1, w, h);
		int c2 = Code(x2, y2, w, h);
		for (int guard = 0; guard < 8; guard++)
		{
			if ((c1 | c2) == 0)
			{
				cv.DrawLine(x1, y1, x2, y2, width, color);
				return;
			}
			if ((c1 & c2) != 0)
				return;
			int c = c1;
			if (c == 0)
				c = c2;
			float x, y;
			if (c & 8)
			{
				x = x1 + (x2 - x1) * (h - y1) / (y2 - y1);
				y = h;
			}
			else if (c & 4)
			{
				x = x1 + (x2 - x1) * (0 - y1) / (y2 - y1);
				y = 0;
			}
			else if (c & 2)
			{
				y = y1 + (y2 - y1) * (w - x1) / (x2 - x1);
				x = w;
			}
			else
			{
				y = y1 + (y2 - y1) * (0 - x1) / (x2 - x1);
				x = 0;
			}
			if (c == c1)
			{
				x1 = x;
				y1 = y;
				c1 = Code(x1, y1, w, h);
			}
			else
			{
				x2 = x;
				y2 = y;
				c2 = Code(x2, y2, w, h);
			}
		}
	}

	// значок входа: тёмный диск, синее кольцо и белая «М»
	static void Entrance(CanvasWidget cv, float x, float y, float w, float h)
	{
		float r = 11;
		if (x < -r || y < -r || x > w + r || y > h + r)
			return;
		cv.DrawLine(x - r * 0.7, y, x + r * 0.7, y, r * 1.6, 0xE0081420);
		int seg = 18;
		for (int i = 0; i < seg; i++)
		{
			float a0 = Math.PI2 * i / seg;
			float a1 = Math.PI2 * (i + 1) / seg;
			Line(cv, x + Math.Cos(a0) * r, y + Math.Sin(a0) * r, x + Math.Cos(a1) * r, y + Math.Sin(a1) * r, w, h, 3, C_ENTRANCE);
		}
		float s = r * 0.5;
		Line(cv, x - s, y + s, x - s, y - s, w, h, 2, C_LETTER);
		Line(cv, x - s, y - s, x, y + s * 0.2, w, h, 2, C_LETTER);
		Line(cv, x, y + s * 0.2, x + s, y - s, w, h, 2, C_LETTER);
		Line(cv, x + s, y - s, x + s, y + s, w, h, 2, C_LETTER);
	}
}
