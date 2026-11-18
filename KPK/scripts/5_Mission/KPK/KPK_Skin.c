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
	protected bool m_Heavy;                  // Monolith: much more interference
	protected int m_GripColor = 0xFF1A1B17;
	protected ref array<Widget> m_NoiseBlocks;

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

	// One moulded case, light from the top left: highlights on top/left edges, shadows bottom/right.
	// The screen sits in a recess (dark inner edge at the top/left, light inner edge at the bottom/right).
	void BuildDevice(Widget device, KPK_Theme theme)
	{
		m_Heavy = theme.Glitch;
		m_GripColor = theme.Grip;
		// soft vertical shading of the plastic
		for (int band = 0; band < 3; band++)
			Line(device, 0, band * 0.025, 1, 0.025, ((10 - band * 3) << 24) | 0xFFFFF0);
		for (int band2 = 0; band2 < 3; band2++)
			Line(device, 0, 0.925 + band2 * 0.025, 1, 0.025, ((band2 * 12 + 8) << 24));

		// outer edge of the case
		Line(device, 0, 0, 1, 0.003, 0x60FFFFF0);
		Line(device, 0, 0, 0.002, 1, 0x40FFFFF0);
		Line(device, 0, 0.995, 1, 0.005, 0xA0000000);
		Line(device, 0.997, 0, 0.003, 1, 0x80000000);

		// rubber inserts flush with the sides, finely ribbed
		SideGrip(device, 0.006);
		SideGrip(device, 0.966);

		// recess around the screen
		Line(device, 0.036, 0.057, 0.928, 0.004, 0x90000000);
		Line(device, 0.036, 0.057, 0.003, 0.867, 0x70000000);
		Line(device, 0.036, 0.921, 0.928, 0.003, 0x30FFFFF0);
		Line(device, 0.961, 0.057, 0.003, 0.867, 0x24FFFFF0);

		// brand, embossed
		TextWidget brand = KPK_UI.Text(device, theme.Brand, 0.05, 0.009, 0.3, 0.042, 17, theme.BrandColor);
		brand.SetShadow(1, 0xFF000000, 0.8, 1, 1);
		TextWidget maker = KPK_UI.Text(device, theme.Maker, 0.255, 0.012, 0.3, 0.038, 13, (theme.BrandColor & 0x00FFFFFF) | 0xA0000000);
		maker.SetShadow(1, 0xFF000000, 0.7, 1, 1);

		// speaker grille: slots with a lit lower lip
		for (int g = 0; g < 10; g++)
		{
			Line(device, 0.545 + g * 0.011, 0.016, 0.0045, 0.028, 0xFF0B0C09);
			Line(device, 0.545 + g * 0.011, 0.044, 0.0045, 0.003, 0x30FFFFF0);
		}

		// serial plate with rivets
		Widget plate = KPK_UI.Panel(device, 0.745, 0.013, 0.2, 0.034, 0xFF55554A);
		Line(plate, 0, 0, 1, 0.1, 0x50FFFFFF);
		Line(plate, 0, 0.9, 1, 0.1, 0x60000000);
		Line(plate, 0.012, 0.35, 0.018, 0.3, 0xFF2E2E28);
		Line(plate, 0.97, 0.35, 0.018, 0.3, 0xFF2E2E28);
		m_Serial = KPK_UI.Text(plate, "S/N ----", 0.05, 0, 0.9, 1, 13, 0xFF1C1C16);

		// screws in the corners of the case
		Screw(device, 0.012, 0.016);
		Screw(device, 0.976, 0.016);
		Screw(device, 0.012, 0.952);
		Screw(device, 0.976, 0.952);

		// power LED in a small dark well
		KPK_UI.Panel(device, 0.05, 0.943, 0.022, 0.034, 0xFF0E0F0C);
		m_LedGlow = KPK_UI.Panel(device, 0.052, 0.946, 0.018, 0.028, 0x2040FF40);
		m_Led = KPK_UI.Panel(device, 0.0565, 0.952, 0.009, 0.016, 0xFF4DE64D);
		KPK_UI.Text(device, "ПИТ", 0.077, 0.94, 0.05, 0.04, 13, 0xFF6E6B5A);

		// hardware keys in a common recess
		Widget keyWell = KPK_UI.Panel(device, 0.318, 0.934, 0.364, 0.05, 0xFF10110E);
		Line(keyWell, 0, 0, 1, 0.08, 0x90000000);
		Line(keyWell, 0, 0.94, 1, 0.06, 0x30FFFFF0);
		string keys[5] = {"МЕНЮ", "<", "OK", ">", "ВЫКЛ"};
		for (int k = 0; k < 5; k++)
		{
			Widget key = KPK_UI.Panel(keyWell, 0.012 + k * 0.197, 0.12, 0.185, 0.76, 0xFF23241E);
			Line(key, 0, 0, 1, 0.1, 0x38FFFFFF);
			Line(key, 0, 0.88, 1, 0.12, 0x90000000);
			KPK_UI.TextC(key, keys[k], 0, 0, 1, 1, 0xFF7A775F);
		}

		// inventory sticker, slightly crooked, with a torn corner
		Widget sticker = KPK_UI.Panel(device, 0.72, 0.934, 0.2, 0.046, 0xFFC4BC9B);
		sticker.SetRotation(0, 0, -1.2);
		Line(sticker, 0, 0, 1, 0.1, 0xFFA82020);
		KPK_UI.Text(sticker, theme.Sticker, 0.04, 0.12, 0.95, 0.85, 13, 0xFF2A2418);
		Line(sticker, 0.94, 0.6, 0.08, 0.55, 0xFF2B2D25, 35.0);
		Line(sticker, 0.15, 0.25, 0.3, 0.55, 0x14402810);

		// a few fine scratches on the plastic strips (never over the screen)
		for (int sc = 0; sc < 6; sc++)
		{
			float sx = Range(0.06, 0.9);
			float sy;
			if (Rand() < 0.5)
				sy = Range(0.006, 0.05);
			else
				sy = Range(0.93, 0.99);
			int alpha = Range(10.0, 22.0);
			Line(device, sx, sy, Range(0.006, 0.025), 0.0011, (alpha << 24) | 0xE8E4D0, Range(-25.0, 25.0));
		}
	}

	protected void SideGrip(Widget device, float x)
	{
		Widget grip = KPK_UI.Panel(device, x, 0.12, 0.026, 0.76, m_GripColor);
		Line(grip, 0, 0, 1, 0.004, 0x90000000);
		Line(grip, 0, 0.996, 1, 0.004, 0x30FFFFF0);
		Line(grip, 0, 0, 0.1, 1, 0x60000000);
		Line(grip, 0.9, 0, 0.1, 1, 0x18FFFFF0);
		for (int r = 0; r < 12; r++)
		{
			float ry = 0.03 + r * 0.08;
			Line(grip, 0.18, ry, 0.64, 0.03, 0xFF0F100D);
			Line(grip, 0.18, ry + 0.03, 0.64, 0.008, 0x14FFFFFF);
		}
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
		int lines = 45;
		for (int i = 0; i < lines; i++)
			Line(screen, 0, i / (lines * 1.0), 1, 0.006, 0x14000000);

		// vignette: stacked translucent frames, darker towards the edges
		for (int v = 1; v <= 3; v++)
		{
			float d = 0.024 * v;
			Line(screen, 0, 0, 1, d, 0x0E000000);
			Line(screen, 0, 1 - d, 1, d, 0x0E000000);
			Line(screen, 0, 0, d * 0.6, 1, 0x0E000000);
			Line(screen, 1 - d * 0.6, 0, d * 0.6, 1, 0x0E000000);
		}

		// dead pixels
		Line(screen, 0.413, 0.271, 0.0016, 0.0028, 0xFF000000);
		Line(screen, 0.781, 0.552, 0.0016, 0.0028, 0xC0B8FFB0);
		Line(screen, 0.236, 0.884, 0.0016, 0.0028, 0xFF000000);

		// animated: slow refresh band, flicker, rare interference line
		m_RollBand = Line(screen, 0, 0, 1, 0.002, 0x00000000);   // kept as a timer anchor, invisible
		m_Flicker = Line(screen, 0, 0, 1, 1, 0x00000000);
		m_Glitch = Line(screen, 0, 0.5, 1, 0.006, 0x50C8FFC0);
		m_Glitch.Show(false);

		// Monolith: blocks of interference popping up all over the screen
		m_NoiseBlocks = new array<Widget>();
		if (m_Heavy)
		{
			for (int nb = 0; nb < 12; nb++)
			{
				int blockColor = 0x3090F0F0;
				if (nb % 2 == 0)
					blockColor = 0x90000000;
				Widget blk = Line(screen, 0, 0, 0.1, 0.01, blockColor);
				blk.Show(false);
				m_NoiseBlocks.Insert(blk);
			}
		}
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
		if (m_Heavy)
			flicker = Math.RandomInt(0, 46);
		m_Flicker.SetColor(flicker << 24);

		if (m_Heavy && m_NoiseBlocks)
		{
			foreach (Widget nbw : m_NoiseBlocks)
			{
				float roll = Math.RandomFloat01();
				if (roll < 0.06)
				{
					nbw.SetPos(Math.RandomFloat(0.0, 0.9), Math.RandomFloat(0.0, 0.98));
					nbw.SetSize(Math.RandomFloat(0.02, 0.3), Math.RandomFloat(0.003, 0.02));
					nbw.Show(true);
				}
				else if (roll > 0.8)
				{
					nbw.Show(false);
				}
			}
		}

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
			if (m_Heavy)
				m_GlitchTimer = Math.RandomFloat(0.4, 2.0);
			m_GlitchLeft = Math.RandomFloat(0.06, 0.18);
			m_GlitchY = Math.RandomFloat(0.1, 0.9);
			m_Glitch.SetPos(0, m_GlitchY);
			m_Glitch.Show(true);
		}
	}

	void SetLed(bool low)
	{
		if (!m_Led || !m_LedGlow)
			return;
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
