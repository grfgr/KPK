// Widget factory: every widget is created from a tiny layout and placed in parent-relative coordinates (0..1).
class KPK_UI
{
	static const string DIR = "KPK/gui/layouts/";

	static int C_TEXT = 0xFF9EE084;
	static int C_DIM = 0xFF5E8C55;
	static int C_ACCENT = 0xFFE6AA3C;
	static int C_DANGER = 0xFFD8553F;
	static int C_WHITE = 0xFFE8E8DC;
	static int C_BTN = 0xFF172B19;
	static int C_BTN_HOVER = 0xFF27482A;
	static int C_BTN_ACTIVE = 0xFF3E6B32;
	static int C_BTN_OFF = 0xFF101410;
	static int C_ROW = 0xFF0D1A0F;
	static int C_ROW_HOVER = 0xFF183019;
	static int C_ROW_SEL = 0xFF2D5228;
	static int C_CARD = 0xFF0A170C;
	static int C_EDGE = 0xFF5A8C4D;
	static int C_LINE = 0xFF1F3A22;

	// set by Apply(theme) before the menu is built
	static int C_SCREEN = 0xFF060D08;
	static int C_TITLEBAR = 0xFF15301A;
	static int C_MENUBAR = 0xFF0C1C0F;
	static int C_SIDE = 0xFF081509;
	static int C_TITLE = 0xFFD8E8C8;
	static int C_TINT = 0x2230FF50;
	static int C_GRID = 0x1890FF90;
	static bool s_Glitch;

	static void Apply(KPK_Theme t)
	{
		C_TEXT = t.Text;
		C_DIM = t.Dim;
		C_ACCENT = t.Accent;
		C_DANGER = t.Danger;
		C_WHITE = t.White;
		C_BTN = t.Btn;
		C_BTN_HOVER = t.BtnHover;
		C_BTN_ACTIVE = t.BtnActive;
		C_BTN_OFF = t.BtnOff;
		C_ROW = t.Row;
		C_ROW_HOVER = t.RowHover;
		C_ROW_SEL = t.RowSel;
		C_CARD = t.Card;
		C_EDGE = t.Edge;
		C_LINE = t.Line;
		C_SCREEN = t.Screen;
		C_TITLEBAR = t.TitleBar;
		C_MENUBAR = t.MenuBar;
		C_SIDE = t.Side;
		C_TITLE = t.Title;
		C_TINT = t.Tint;
		C_GRID = t.Grid;
		s_Glitch = t.Glitch;
	}

	static Widget Make(string file, Widget parent, float x, float y, float w, float h)
	{
		Widget wd = GetGame().GetWorkspace().CreateWidgets(DIR + file, parent);
		if (!wd)
		{
			Print("[KPK] !!! widget not created: " + file);
			return null;
		}
		wd.SetPos(x, y);
		wd.SetSize(w, h);
		return wd;
	}

	static Widget Frame(Widget parent, float x, float y, float w, float h)
	{
		return Make("kpk_frame.layout", parent, x, y, w, h);
	}

	// decorative panels ignore the mouse; containers (cards) must not, their children are clickable
	static Widget Panel(Widget parent, float x, float y, float w, float h, int color, bool container = false)
	{
		Widget p = Make("kpk_panel.layout", parent, x, y, w, h);
		p.SetColor(color);
		if (!container)
			p.SetFlags(WidgetFlags.IGNOREPOINTER);
		return p;
	}

	// a card: dark panel with a title strip
	static Widget Card(Widget parent, float x, float y, float w, float h, string title, float titleH = 0.07)
	{
		Widget card = Panel(parent, x, y, w, h, C_CARD, true);
		Panel(card, 0, 0, 1, 0.003, C_EDGE);
		if (title != "")
		{
			string shown = title;
			if (s_Glitch)
				shown = KPK_Theme.Noise(title, 0.45);
			Text(card, shown, 0.03, 0.01, 0.94, titleH, 14, C_ACCENT);
		}
		return card;
	}

	static TextWidget Text(Widget parent, string text, float x, float y, float w, float h, int size = 16, int color = 0)
	{
		if (color == 0)
			color = C_TEXT;
		// same pattern as the button labels, which render fine: the text sits inside a frame
		// and its size is baked into the layout (no SetTextExactSize at runtime)
		// every size two points smaller than written: the window of the model is smaller than the old
		// drawn case and the texts ran out of their boxes
		size = size - 2;
		string file = "kpk_text20.layout";
		if (size <= 11)
			file = "kpk_text11.layout";
		else if (size <= 13)
			file = "kpk_text13.layout";
		else if (size <= 15)
			file = "kpk_text15.layout";
		else if (size <= 17)
			file = "kpk_text17.layout";
		Widget box = Make(file, parent, x, y, w, h);
		return FillText(box, text, color, file);
	}

	// the engine sometimes refuses to create the text inside the box (widget limit): no crash then
	protected static TextWidget FillText(Widget box, string text, int color, string file)
	{
		if (!box)
			return null;
		// the text layouts are a bare text widget now (no frame around it: one widget less per text)
		TextWidget t = TextWidget.Cast(box);
		if (!t)
			t = TextWidget.Cast(box.FindAnyWidget("text"));
		if (!t)
		{
			Print("[KPK] !!! text widget not created: " + file);
			return null;
		}
		t.SetText(text);
		t.SetColor(color);
		return t;
	}

	// small centred text (hardware key captions, window controls)
	static TextWidget TextC(Widget parent, string text, float x, float y, float w, float h, int color)
	{
		Widget box = Make("kpk_text13c.layout", parent, x, y, w, h);
		return FillText(box, text, color, "kpk_text13c.layout");
	}

	static MultilineTextWidget MText(Widget parent, float x, float y, float w, float h, int size = 15, int color = 0)
	{
		if (color == 0)
			color = C_TEXT;
		// the size is baked into the layout: setting it at runtime made the text render as empty boxes
		// once a menu used many different sizes (the font cache overflowed)
		size = size - 2;
		string file = "kpk_mtext17.layout";
		if (size <= 11)
			file = "kpk_mtext11.layout";
		else if (size <= 13)
			file = "kpk_mtext13.layout";
		else if (size <= 15)
			file = "kpk_mtext15.layout";
		MultilineTextWidget t = MultilineTextWidget.Cast(Make(file, parent, x, y, w, h));
		if (t)
			t.SetColor(color);
		return t;
	}

	static EditBoxWidget Edit(Widget parent, float x, float y, float w, float h)
	{
		Widget root = Make("kpk_edit.layout", parent, x, y, w, h);
		if (!root)
			return null;
		Panel(root, 0, 0.94, 1, 0.06, C_EDGE);
		return EditBoxWidget.Cast(root.FindAnyWidget("input"));
	}

	// a password box; if the engine refuses the password input, a plain edit box is used instead so
	// the KPK can still be logged into
	static EditBoxWidget Password(Widget parent, float x, float y, float w, float h)
	{
		Widget root = Make("kpk_password.layout", parent, x, y, w, h);
		if (root)
		{
			PasswordEditBoxWidget input = PasswordEditBoxWidget.Cast(root.FindAnyWidget("input"));
			if (input)
			{
				input.SetHideText(true);
				Panel(root, 0, 0.94, 1, 0.06, C_EDGE);
				return input;
			}
			root.Unlink();
		}
		Print("[KPK] password input could not be created, using a plain edit box");
		return Edit(parent, x, y, w, h);
	}

	static MultilineEditBoxWidget MEdit(Widget parent, float x, float y, float w, float h)
	{
		Widget root = Make("kpk_medit.layout", parent, x, y, w, h);
		return MultilineEditBoxWidget.Cast(root.FindAnyWidget("input"));
	}

	static string Percent(float value, float maxValue)
	{
		if (maxValue <= 0)
			return "0%";
		int pct = Math.Clamp(value / maxValue * 100, 0, 100);
		return pct.ToString() + "%";
	}

	static string Grid(float x, float z)
	{
		int gx = x / 100;
		int gz = z / 100;
		return gx.ToStringLen(3) + " " + gz.ToStringLen(3);
	}
};

class KPK_Button
{
	ButtonWidget m_Root;
	Widget m_Bg;
	Widget m_Edge;
	TextWidget m_Label;
	int m_Id;
	bool m_Active;
	bool m_Hover;
	bool m_Enabled = true;
	bool m_IsRow;
	int m_TextColor;
	bool m_Wrap;                     // list rows of long texts: a second, wrapping small label
	MultilineTextWidget m_WrapLabel;
	string m_Text;

	void KPK_Button(Widget parent, int id, string text, float x, float y, float w, float h, bool isRow = false)
	{
		m_Id = id;
		m_IsRow = isRow;
		m_TextColor = KPK_UI.C_TEXT;
		// list rows have their own layout with a left-aligned label: one text per row instead of two
		// (the engine caps how many widgets all mods together may create)
		string file = "kpk_button.layout";
		if (isRow)
			file = "kpk_row.layout";
		m_Root = ButtonWidget.Cast(KPK_UI.Make(file, parent, x, y, w, h));
		if (!m_Root)
		{
			Print("[KPK] could not create a button");
			return;
		}
		m_Bg = m_Root.FindAnyWidget("bg");
		m_Edge = m_Root.FindAnyWidget("edge");
		m_Label = TextWidget.Cast(m_Root.FindAnyWidget("label"));
		SetText(text);
		Refresh();
	}

	void SetText(string text)
	{
		m_Text = text;
		if (m_Wrap && m_IsRow && m_Root)
		{
			// a long row text wraps onto a second line in a smaller font
			bool wrap = text.LengthUtf8() > FitChars(15);
			if (wrap && !m_WrapLabel)
				m_WrapLabel = KPK_UI.MText(m_Root, 0.04, 0.02, 0.94, 0.96, 12, m_TextColor);
			if (m_WrapLabel)
			{
				m_WrapLabel.Show(wrap);
				if (wrap)
					m_WrapLabel.SetText(text);
				else
					m_WrapLabel.SetText("");
			}
			if (m_Label)
			{
				if (wrap)
					m_Label.SetText("");
				else
					m_Label.SetText(text);
			}
			return;
		}
		if (m_Label)
			m_Label.SetText(text);
	}

	// how many characters of the given text size fit into the button (screen size known after the first frame)
	int FitChars(int size)
	{
		if (!m_Root)
			return 30;
		float bw, bh;
		m_Root.GetScreenSize(bw, bh);
		if (bw <= 0)
			return 30;
		float charW = size * 0.55;
		return bw * 0.92 / charW;
	}

	void SetTextColor(int color)
	{
		m_TextColor = color;
		Refresh();
	}

	void SetActive(bool active)
	{
		m_Active = active;
		Refresh();
	}

	void SetEnabled(bool enabled)
	{
		m_Enabled = enabled;
		Refresh();
	}

	void SetHover(bool hover)
	{
		m_Hover = hover;
		Refresh();
	}

	void Show(bool show)
	{
		if (m_Root)
			m_Root.Show(show);
	}

	void Refresh()
	{
		if (!m_Root)
			return;
		int bg;
		if (m_IsRow)
		{
			bg = KPK_UI.C_ROW;
			if (m_Hover)
				bg = KPK_UI.C_ROW_HOVER;
			if (m_Active)
				bg = KPK_UI.C_ROW_SEL;
		}
		else
		{
			bg = KPK_UI.C_BTN;
			if (m_Hover)
				bg = KPK_UI.C_BTN_HOVER;
			if (m_Active)
				bg = KPK_UI.C_BTN_ACTIVE;
		}
		if (!m_Enabled)
			bg = KPK_UI.C_BTN_OFF;

		m_Bg.SetColor(bg);
		if (m_Enabled)
		{
			m_Label.SetColor(m_TextColor);
			if (m_WrapLabel)
				m_WrapLabel.SetColor(m_TextColor);
			m_Edge.SetColor(KPK_UI.C_EDGE);
		}
		else
		{
			m_Label.SetColor(KPK_UI.C_DIM);
			if (m_WrapLabel)
				m_WrapLabel.SetColor(KPK_UI.C_DIM);
			m_Edge.SetColor(KPK_UI.C_LINE);
		}
		if (m_Active)
			m_Edge.SetColor(KPK_UI.C_ACCENT);
	}
};

// A text box: the engine's edit box stays invisible and takes the keys, a normal text over it shows
// what was typed. Multi-line boxes wrap (MultilineTextWidget does it), one-line boxes show the end of the
// text so it never leaves the border. The cursor is drawn at the end while the box has the focus.
class KPK_TextBox
{
	EditBoxWidget m_Input;
	Widget m_Root;
	bool m_Multi;
	bool m_Mask;       // password: stars instead of the letters
	string m_WrapSource;
	int m_WrapChars;
	string m_Wrapped;
	TextWidget m_Line;
	MultilineTextWidget m_Lines;
	string m_Shown = "#";

	void KPK_TextBox(EditBoxWidget input, bool multi, bool mask = false)
	{
		m_Input = input;
		m_Root = input.GetParent();
		m_Multi = multi;
		m_Mask = mask;
		// the input itself is not drawn any more (its text would run out of the box)
		m_Input.SetColor(ARGB(0, 0, 0, 0));
		if (multi)
			m_Lines = KPK_UI.MText(m_Root, 0.015, 0.03, 0.97, 0.94, 15, KPK_UI.C_WHITE);
		else
			m_Line = KPK_UI.Text(m_Root, "", 0.02, 0, 0.96, 1, 15, KPK_UI.C_WHITE);
	}

	void Sync(int chars, bool cursor)
	{
		if (!m_Input)
			return;
		string text = m_Input.GetText();
		if (m_Mask)
		{
			string stars = "";
			for (int st = text.LengthUtf8(); st > 0; st--)
				stars += "*";
			text = stars;
		}
		string shown = text;
		if (m_Multi && chars > 2)
		{
			// wrapping is redone only when the text or the box changed
			if (text != m_WrapSource || chars != m_WrapChars)
			{
				m_WrapSource = text;
				m_WrapChars = chars;
				m_Wrapped = Wrap(text, chars);
			}
			shown = m_Wrapped;
		}
		if (!m_Multi && chars > 2)
		{
			// one line: only the end of a long text, so what is being typed stays visible
			int room = chars - 1;
			int len = text.LengthUtf8();
			if (len > room)
				shown = ".." + text.SubstringUtf8(len - room + 2, room - 2);
		}
		if (cursor)
			shown += "_";
		if (shown == m_Shown)
			return;
		m_Shown = shown;
		if (m_Lines)
			m_Lines.SetText(shown);
		if (m_Line)
			m_Line.SetText(shown);
	}

	// lines of at most "chars" characters: broken at the last space, a word longer than a line is cut
	// (the text widget itself wraps only at spaces)
	static string Wrap(string text, int chars)
	{
		string result = "";
		string rest = text;
		while (rest.LengthUtf8() > chars)
		{
			int len = rest.LengthUtf8();
			int cut = -1;
			for (int k = chars; k > chars / 2; k--)
			{
				if (rest.SubstringUtf8(k, 1) == " ")
				{
					cut = k;
					break;
				}
			}
			if (cut > 0)
			{
				result += rest.SubstringUtf8(0, cut) + "\n";
				rest = rest.SubstringUtf8(cut + 1, len - cut - 1);
			}
			else
			{
				result += rest.SubstringUtf8(0, chars) + "\n";
				rest = rest.SubstringUtf8(chars, len - chars);
			}
		}
		return result + rest;
	}
};

// Paged list: N row buttons and a pager below. No scroll widgets needed.
class KPK_List
{
	ref array<ref KPK_Button> m_Rows;
	ref KPK_Button m_Prev;
	ref KPK_Button m_Next;
	TextWidget m_PageText;
	ref array<string> m_Items;
	ref array<int> m_Colors;
	int m_Page;
	int m_Selected = -1;
	string m_Empty;

	// long rows wrap onto a second line in a smaller font
	void SetWrap(bool wrap)
	{
		foreach (KPK_Button row : m_Rows)
			row.m_Wrap = wrap;
		Redraw();
	}


	void KPK_List(Widget parent, float x, float y, float w, float h, int rows, string emptyText = "— пусто —")
	{
		m_Rows = new array<ref KPK_Button>();
		m_Items = new array<string>();
		m_Colors = new array<int>();
		m_Empty = emptyText;

		Widget box = KPK_UI.Frame(parent, x, y, w, h);
		float rowH = 1.0 / (rows + 1);
		for (int i = 0; i < rows; i++)
		{
			KPK_Button row = new KPK_Button(box, -1, "", 0, i * rowH, 1, rowH * 0.9, true);
			m_Rows.Insert(row);
		}
		m_Prev = new KPK_Button(box, -1, "<", 0, rows * rowH + rowH * 0.1, 0.22, rowH * 0.85);
		m_Next = new KPK_Button(box, -1, ">", 0.78, rows * rowH + rowH * 0.1, 0.22, rowH * 0.85);
		m_PageText = KPK_UI.Text(box, "", 0.22, rows * rowH + rowH * 0.1, 0.56, rowH * 0.85, 13, KPK_UI.C_DIM);
		Redraw();
	}

	int PageSize()
	{
		return m_Rows.Count();
	}

	int PageCount()
	{
		int count = (m_Items.Count() + PageSize() - 1) / PageSize();
		if (count < 1)
			count = 1;
		return count;
	}

	void Set(array<string> items, array<int> colors = null)
	{
		m_Items.Clear();
		m_Colors.Clear();
		for (int i = 0; i < items.Count(); i++)
		{
			m_Items.Insert(items[i]);
			if (colors && i < colors.Count())
				m_Colors.Insert(colors[i]);
			else
				m_Colors.Insert(KPK_UI.C_TEXT);
		}
		if (m_Selected >= m_Items.Count())
			m_Selected = -1;
		if (m_Page >= PageCount())
			m_Page = PageCount() - 1;
		Redraw();
	}

	int Selected()
	{
		return m_Selected;
	}

	void Select(int index)
	{
		m_Selected = index;
		if (index >= 0)
			m_Page = index / PageSize();
		Redraw();
	}

	void Redraw()
	{
		for (int r = 0; r < m_Rows.Count(); r++)
		{
			KPK_Button row = m_Rows[r];
			int index = m_Page * PageSize() + r;
			if (index < m_Items.Count())
			{
				row.Show(true);
				row.SetText(m_Items[index]);
				row.SetTextColor(m_Colors[index]);
				row.SetActive(index == m_Selected);
			}
			else if (r == 0 && m_Items.Count() == 0)
			{
				row.Show(true);
				row.SetText(m_Empty);
				row.SetTextColor(KPK_UI.C_DIM);
				row.SetActive(false);
			}
			else
			{
				row.Show(false);
			}
		}
		m_PageText.SetText(string.Format("стр. %1 / %2", m_Page + 1, PageCount()));
		m_Prev.SetEnabled(m_Page > 0);
		m_Next.SetEnabled(m_Page < PageCount() - 1);
	}

	// returns true when the click belonged to this list
	bool Click(Widget w)
	{
		if (w == m_Prev.m_Root)
		{
			if (m_Page > 0)
				m_Page--;
			Redraw();
			return true;
		}
		if (w == m_Next.m_Root)
		{
			if (m_Page < PageCount() - 1)
				m_Page++;
			Redraw();
			return true;
		}
		for (int r = 0; r < m_Rows.Count(); r++)
		{
			if (w != m_Rows[r].m_Root)
				continue;
			int index = m_Page * PageSize() + r;
			if (index < m_Items.Count())
				m_Selected = index;
			Redraw();
			return true;
		}
		return false;
	}

	bool Hover(Widget w, bool on)
	{
		if (w == m_Prev.m_Root)
		{
			m_Prev.SetHover(on);
			return true;
		}
		if (w == m_Next.m_Root)
		{
			m_Next.SetHover(on);
			return true;
		}
		foreach (KPK_Button row : m_Rows)
		{
			if (w == row.m_Root)
			{
				row.SetHover(on);
				return true;
			}
		}
		return false;
	}
};

class KPK_MapHandler extends ScriptedWidgetEventHandler
{
	KPK_Menu m_Menu;

	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (m_Menu && button == MouseState.LEFT)
		{
			m_Menu.OnMapDoubleClick(x, y);
			return true;
		}
		return false;
	}
};
