// Look of the device: worn plastic case and an old CRT-like screen, built from plain panels
// (no textures needed). Everything here is decorative and ignores the mouse.
class KPK_Skin
{
	// fixed seed: the scratches are the same every time, like on a real device
	protected int m_Seed = 1337;

	// animated screen effects
	protected Widget m_Flicker;
	protected Widget m_RollBand;
	protected Widget m_Glitch;
	protected float m_RollY;
	protected float m_GlitchTimer = 6;
	protected float m_GlitchLeft;
	protected float m_GlitchY;

	TextWidget m_Serial;
	Widget m_Led;
	Widget m_LedGlow;

	protected float Rand()
	{
		m_Seed = m_Seed * 1103515245 + 12345;
		int v = (m_Seed >> 8) & 0xFFFF;
		return v / 65535.0;
	}

	protected float Range(float a, float b)
	{
		return a + (b - a) * Rand();
	}

	protected Widget Line(Widget parent, float x, float y, float w, float h, int color, float angle = 0)
	{
		Widget p = KPK_UI.Panel(parent, x, y, w, h, color);
		if (angle != 0)
			p.SetRotation(0, 0, angle);
		return p;
	}

	// ------------------------------------------------------------------ case

	void BuildDevice(Widget device)
	{
		// bevel: light top/left edge, dark bottom/right edge
		Line(device, 0, 0, 1, 0.005, 0x40FFFFF0);
		Line(device, 0, 0, 0.003, 1, 0x30FFFFF0);
		Line(device, 0, 0.993, 1, 0.007, 0x70000000);
		Line(device, 0.997, 0, 0.003, 1, 0x60000000);
		// inner groove around the bezel
		Line(device, 0.014, 0.056, 0.972, 0.003, 0x50000000);
		Line(device, 0.014, 0.922, 0.972, 0.003, 0x25FFFFF0);

		// brand, embossed into the plastic
		TextWidget brand = KPK_UI.Text(device, "КПК-7  «ВЕСТНИК»", 0.03, 0.008, 0.3, 0.045, 17, 0xFF8A8670);
		brand.SetShadow(1, 0xFF000000, 0.8, 1, 1);
		TextWidget maker = KPK_UI.Text(device, "НИИ ЧАЗ  ·  изд. 1986-М", 0.25, 0.012, 0.3, 0.04, 13, 0xFF5E5B4C);
		maker.SetShadow(1, 0xFF000000, 0.7, 1, 1);

		// speaker grille
		for (int g = 0; g < 9; g++)
			Line(device, 0.55 + g * 0.012, 0.016, 0.005, 0.03, 0xFF0C0D0A);

		// serial plate
		Widget plate = KPK_UI.Panel(device, 0.78, 0.012, 0.19, 0.036, 0xFF4A4A40);
		Line(plate, 0, 0, 1, 0.08, 0x40FFFFFF);
		m_Serial = KPK_UI.Text(plate, "S/N ----", 0.05, 0, 0.9, 1, 13, 0xFF1A1A14);

		// screws in the corners
		Screw(device, 0.006, 0.012);
		Screw(device, 0.982, 0.012);
		Screw(device, 0.006, 0.948);
		Screw(device, 0.982, 0.948);

		// power LED
		m_LedGlow = KPK_UI.Panel(device, 0.026, 0.944, 0.018, 0.032, 0x3040FF40);
		m_Led = KPK_UI.Panel(device, 0.03, 0.951, 0.01, 0.018, 0xFF4DE64D);
		KPK_UI.Text(device, "ПИТ", 0.045, 0.94, 0.05, 0.04, 13, 0xFF6E6B5A);

		// hardware buttons
		string keys[5] = {"МЕНЮ", "<", "OK", ">", "ВЫКЛ"};
		for (int k = 0; k < 5; k++)
		{
			float bx = 0.33 + k * 0.07;
			Widget key = KPK_UI.Panel(device, bx, 0.936, 0.06, 0.044, 0xFF1E1F19);
			Line(key, 0, 0, 1, 0.1, 0x30FFFFFF);
			Line(key, 0, 0.88, 1, 0.12, 0x80000000);
			KPK_UI.TextC(key, keys[k], 0, 0, 1, 1, 0xFF77745F);
		}

		// inventory sticker, slightly crooked and worn
		Widget sticker = KPK_UI.Panel(device, 0.73, 0.932, 0.2, 0.05, 0xFFC9C1A0);
		sticker.SetRotation(0, 0, -1.5);
		Line(sticker, 0, 0, 1, 0.1, 0xFFB01E1E);
		KPK_UI.Text(sticker, "ИНВ. № 07-0347   НЕ ВСКРЫВАТЬ", 0.04, 0.12, 0.95, 0.85, 13, 0xFF2A2418);
		// torn corner and dirt on the sticker
		Line(sticker, 0.93, 0.55, 0.09, 0.6, 0xFF2A2C24, 35);
		Line(sticker, 0.2, 0.3, 0.25, 0.5, 0x18402810);

		// worn paint: lighter patches along the edges, where fingers hold the device
		for (int w = 0; w < 14; w++)
		{
			float wx = Range(0.0, 0.96);
			float wy;
			if (Rand() < 0.5)
				wy = Range(0.0, 0.04);
			else
				wy = Range(0.93, 0.975);
			Line(device, wx, wy, Range(0.01, 0.05), Range(0.005, 0.02), 0x14FFFFF0, Range(-10.0, 10.0));
		}

		// scratches on the case
		for (int s = 0; s < 34; s++)
		{
			float sx = Range(0.0, 0.97);
			float sy;
			float r = Rand();
			if (r < 0.45)
				sy = Range(0.002, 0.05);
			else if (r < 0.9)
				sy = Range(0.928, 0.99);
			else
				sy = Range(0.06, 0.92);
			if (r >= 0.9)
			{
				// along the side rims
				if (Rand() < 0.5)
					sx = Range(0.001, 0.012);
				else
					sx = Range(0.986, 0.996);
			}
			int alpha = Range(20.0, 58.0);
			Line(device, sx, sy, Range(0.008, 0.06), 0.002, (alpha << 24) | 0xE8E4D0, Range(-40.0, 40.0));
		}

		// dirt in the corners
		Line(device, 0, 0, 0.04, 0.05, 0x22000000);
		Line(device, 0.96, 0.95, 0.04, 0.05, 0x26000000);
		Line(device, 0, 0.95, 0.03, 0.05, 0x1C000000);
	}

	protected void Screw(Widget parent, float x, float y)
	{
		Widget head = KPK_UI.Panel(parent, x, y, 0.012, 0.022, 0xFF54554A);
		Line(head, 0, 0, 1, 0.2, 0x40FFFFFF);
		Line(head, 0, 0.8, 1, 0.2, 0x70000000);
		Line(head, 0.1, 0.42, 0.8, 0.16, 0xFF1B1C17, Range(20.0, 70.0));
	}

	// ------------------------------------------------------------------ screen

	// call after all pages exist: these widgets must lie on top of the content
	void BuildScreenFx(Widget screen)
	{
		// scanlines
		int lines = 150;
		for (int i = 0; i < lines; i++)
			Line(screen, 0, i / (lines * 1.0), 1, 0.0022, 0x16000000);

		// vignette: stacked translucent frames, darker towards the edges
		for (int v = 1; v <= 6; v++)
		{
			float d = 0.012 * v;
			Line(screen, 0, 0, 1, d, 0x0E000000);
			Line(screen, 0, 1 - d, 1, d, 0x0E000000);
			Line(screen, 0, 0, d * 0.6, 1, 0x0E000000);
			Line(screen, 1 - d * 0.6, 0, d * 0.6, 1, 0x0E000000);
		}

		// glass glare in the upper left corner
		Line(screen, -0.1, 0.06, 0.55, 0.05, 0x0AFFFFFF, -18);
		Line(screen, -0.05, 0.15, 0.4, 0.02, 0x07FFFFFF, -18);
		Line(screen, 0.72, 0.84, 0.35, 0.03, 0x05FFFFFF, -18);

		// scratches on the glass
		Line(screen, 0.12, 0.3, 0.42, 0.0025, 0x14FFFFFF, 8);
		Line(screen, 0.55, 0.62, 0.3, 0.002, 0x10FFFFFF, -14);
		Line(screen, 0.3, 0.8, 0.18, 0.002, 0x0EFFFFFF, 3);

		// dead pixels
		Line(screen, 0.413, 0.271, 0.0016, 0.0028, 0xFF000000);
		Line(screen, 0.781, 0.552, 0.0016, 0.0028, 0xC0B8FFB0);
		Line(screen, 0.236, 0.884, 0.0016, 0.0028, 0xFF000000);

		// animated: slow refresh band, flicker, rare interference line
		m_RollBand = Line(screen, 0, 0, 1, 0.07, 0x08B0FFB0);
		m_Flicker = Line(screen, 0, 0, 1, 1, 0x00000000);
		m_Glitch = Line(screen, 0, 0.5, 1, 0.006, 0x50C8FFC0);
		m_Glitch.Show(false);
	}

	void Update(float dt)
	{
		if (!m_RollBand)
			return;

		m_RollY += dt * 0.09;
		if (m_RollY > 1.1)
			m_RollY = -0.1;
		m_RollBand.SetPos(0, m_RollY);

		int flicker = Math.RandomInt(0, 14);
		m_Flicker.SetColor(flicker << 24);

		if (m_GlitchLeft > 0)
		{
			m_GlitchLeft -= dt;
			m_Glitch.SetPos(Math.RandomFloat(-0.02, 0.02), m_GlitchY);
			if (m_GlitchLeft <= 0)
				m_Glitch.Show(false);
			return;
		}

		m_GlitchTimer -= dt;
		if (m_GlitchTimer <= 0)
		{
			m_GlitchTimer = Math.RandomFloat(7.0, 18.0);
			m_GlitchLeft = Math.RandomFloat(0.06, 0.18);
			m_GlitchY = Math.RandomFloat(0.1, 0.9);
			m_Glitch.SetPos(0, m_GlitchY);
			m_Glitch.Show(true);
		}
	}

	void SetLed(bool low)
	{
		if (low)
		{
			m_Led.SetColor(0xFFE04030);
			m_LedGlow.SetColor(0x30FF4030);
		}
		else
		{
			m_Led.SetColor(0xFF4DE64D);
			m_LedGlow.SetColor(0x3040FF40);
		}
	}
};
