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
		// --- shading of the plastic: light top edge, dark bottom edge, soft vertical falloff
		Line(device, 0, 0, 1, 0.004, 0x50FFFFF0);
		Line(device, 0, 0.004, 1, 0.01, 0x12FFFFF0);
		Line(device, 0, 0.99, 1, 0.01, 0x80000000);
		Line(device, 0, 0.975, 1, 0.015, 0x30000000);
		// inner groove around the bezel
		Line(device, 0.034, 0.055, 0.932, 0.003, 0x60000000);
		Line(device, 0.034, 0.924, 0.932, 0.003, 0x28FFFFF0);
		Line(device, 0.034, 0.055, 0.002, 0.872, 0x50000000);
		Line(device, 0.964, 0.055, 0.002, 0.872, 0x20FFFFF0);

		// --- rubber side grips with ribs (left and right)
		SideGrip(device, 0.004);
		SideGrip(device, 0.966);

		// --- rubber corner bumpers, a bit larger than the case
		Bumper(device, -0.006, -0.012, 0.05, 0.085);
		Bumper(device, 0.956, -0.012, 0.05, 0.085);
		Bumper(device, -0.006, 0.927, 0.05, 0.085);
		Bumper(device, 0.956, 0.927, 0.05, 0.085);

		// --- antenna stub and connector cap above the top edge
		Widget antBase = KPK_UI.Panel(device, 0.075, -0.022, 0.034, 0.026, 0xFF3C3D36);
		Line(antBase, 0, 0, 1, 0.15, 0x40FFFFFF);
		Widget antenna = KPK_UI.Panel(device, 0.081, -0.115, 0.022, 0.095, 0xFF121311);
		for (int a = 0; a < 6; a++)
			Line(antenna, 0, 0.55 + a * 0.07, 1, 0.025, 0x30FFFFFF);
		Line(antenna, 0.1, 0, 0.15, 1, 0x18FFFFFF);
		Widget cap = KPK_UI.Panel(device, 0.86, -0.03, 0.05, 0.032, 0xFF151613);
		Line(cap, 0, 0, 1, 0.12, 0x30FFFFFF);
		Line(cap, 0.2, 0.45, 0.6, 0.1, 0xFF0A0A09);

		// --- side buttons sticking out on the left
		for (int b = 0; b < 2; b++)
		{
			Widget sideKey = KPK_UI.Panel(device, -0.012, 0.36 + b * 0.075, 0.014, 0.06, 0xFF1A1B17);
			Line(sideKey, 0, 0, 0.3, 1, 0x28FFFFFF);
		}

		// --- brand, embossed into the plastic
		TextWidget brand = KPK_UI.Text(device, "КПК-7  «ВЕСТНИК»", 0.055, 0.008, 0.3, 0.045, 17, 0xFF8A8670);
		brand.SetShadow(1, 0xFF000000, 0.8, 1, 1);
		TextWidget maker = KPK_UI.Text(device, "НИИ ЧАЗ  ·  изд. 1986-М", 0.27, 0.012, 0.3, 0.04, 13, 0xFF5E5B4C);
		maker.SetShadow(1, 0xFF000000, 0.7, 1, 1);

		// speaker grille
		for (int g = 0; g < 9; g++)
			Line(device, 0.55 + g * 0.012, 0.016, 0.005, 0.03, 0xFF0C0D0A);

		// serial plate with rivets
		Widget plate = KPK_UI.Panel(device, 0.74, 0.012, 0.19, 0.036, 0xFF4A4A40);
		Line(plate, 0, 0, 1, 0.08, 0x40FFFFFF);
		Line(plate, 0.015, 0.35, 0.02, 0.3, 0xFF2A2A24);
		Line(plate, 0.965, 0.35, 0.02, 0.3, 0xFF2A2A24);
		m_Serial = KPK_UI.Text(plate, "S/N ----", 0.06, 0, 0.88, 1, 13, 0xFF1A1A14);

		// screws next to the bezel corners
		Screw(device, 0.046, 0.014);
		Screw(device, 0.942, 0.014);
		Screw(device, 0.046, 0.94);
		Screw(device, 0.942, 0.94);

		// power LED
		m_LedGlow = KPK_UI.Panel(device, 0.07, 0.944, 0.018, 0.032, 0x3040FF40);
		m_Led = KPK_UI.Panel(device, 0.074, 0.951, 0.01, 0.018, 0xFF4DE64D);
		KPK_UI.Text(device, "ПИТ", 0.09, 0.94, 0.05, 0.04, 13, 0xFF6E6B5A);

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
		Widget sticker = KPK_UI.Panel(device, 0.71, 0.932, 0.2, 0.05, 0xFFC9C1A0);
		sticker.SetRotation(0, 0, -1.5);
		Line(sticker, 0, 0, 1, 0.1, 0xFFB01E1E);
		KPK_UI.Text(sticker, "ИНВ. № 07-0347   НЕ ВСКРЫВАТЬ", 0.04, 0.12, 0.95, 0.85, 13, 0xFF2A2418);
		Line(sticker, 0.93, 0.55, 0.09, 0.6, 0xFF2A2C24, 35.0);
		Line(sticker, 0.2, 0.3, 0.25, 0.5, 0x18402810);

		// worn paint: faint lighter patches on the top and bottom strips
		for (int w = 0; w < 10; w++)
		{
			float wx = Range(0.05, 0.92);
			float wy;
			if (Rand() < 0.5)
				wy = Range(0.0, 0.04);
			else
				wy = Range(0.93, 0.975);
			Line(device, wx, wy, Range(0.01, 0.04), Range(0.004, 0.012), 0x0CFFFFF0, Range(-6.0, 6.0));
		}

		// fine scratches, only on the case strips (never over the screen)
		for (int s = 0; s < 28; s++)
		{
			float sx = Range(0.05, 0.93);
			float sy;
			if (Rand() < 0.5)
				sy = Range(0.004, 0.05);
			else
				sy = Range(0.93, 0.99);
			int alpha = Range(12.0, 30.0);
			Line(device, sx, sy, Range(0.006, 0.035), 0.0012, (alpha << 24) | 0xE8E4D0, Range(-30.0, 30.0));
		}
	}

	protected void SideGrip(Widget device, float x)
	{
		Widget grip = KPK_UI.Panel(device, x, 0.1, 0.03, 0.8, 0xFF181915);
		Line(grip, 0, 0, 1, 0.006, 0x30FFFFFF);
		Line(grip, 0, 0.994, 1, 0.006, 0x70000000);
		Line(grip, 0, 0, 0.12, 1, 0x14FFFFFF);
		Line(grip, 0.88, 0, 0.12, 1, 0x50000000);
		// ribs
		for (int r = 0; r < 34; r++)
		{
			float ry = 0.03 + r * 0.028;
			Line(grip, 0.15, ry, 0.7, 0.011, 0xFF0B0C0A);
			Line(grip, 0.15, ry + 0.011, 0.7, 0.004, 0x18FFFFFF);
		}
	}

	protected void Bumper(Widget device, float x, float y, float w, float h)
	{
		Widget b = KPK_UI.Panel(device, x, y, w, h, 0xFF141512);
		Line(b, 0, 0, 1, 0.06, 0x38FFFFFF);
		Line(b, 0, 0, 0.08, 1, 0x20FFFFFF);
		Line(b, 0, 0.92, 1, 0.08, 0x80000000);
		Line(b, 0.92, 0, 0.08, 1, 0x60000000);
		// moulding seam and rubbed edge
		Line(b, 0.15, 0.5, 0.7, 0.03, 0xFF0A0A09);
		Line(b, 0.1, 0.06, 0.5, 0.02, 0x22FFFFF0);
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
		Line(screen, 0.0, 0.0, 0.35, 0.18, 0x05FFFFFF);
		Line(screen, 0.0, 0.0, 0.2, 0.1, 0x04FFFFFF);

		// scratches on the glass
		Line(screen, 0.22, 0.34, 0.3, 0.0012, 0x0CFFFFFF, 4.0);
		Line(screen, 0.6, 0.66, 0.2, 0.0012, 0x0AFFFFFF, -6.0);

		// dead pixels
		Line(screen, 0.413, 0.271, 0.0016, 0.0028, 0xFF000000);
		Line(screen, 0.781, 0.552, 0.0016, 0.0028, 0xC0B8FFB0);
		Line(screen, 0.236, 0.884, 0.0016, 0.0028, 0xFF000000);

		// animated: slow refresh band, flicker, rare interference line
		m_RollBand = Line(screen, 0, 0, 1, 0.05, 0x05B0FFB0);
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
