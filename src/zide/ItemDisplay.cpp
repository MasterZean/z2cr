#include "ItemDisplay.h"

int ItemDisplay::DoPaint(Draw& w, const Rect& r, const Value& q,
							Color ink, Color paper, dword style) const {
	if (!q.Is<ZItem>())
		return 0;
	
	const ZItem& m = ValueTo<ZItem>(q);
	
	int x = r.left;
	
	Font font = StdFont();
	Font bold = StdFont().Bold();

	Color opc = LtBlue();//HighlightSetup::hl_style[HighlightSetup::INK_OPERATOR].color;
	
	w.DrawRect(r, paper);
	
	int top = r.top;
	int t = r.top + (font.GetCy() - ZImg::prot().GetHeight()) / 2 + 1;
	
	if (m.Access == AccessType::Public)
		x += ZImg::prot().GetWidth() + 2;
	else if (m.Access == AccessType::Protected) {
		w.DrawImage(x, t, ZImg::prot());
		x += ZImg::prot().GetWidth() + 2;
	}
	else if (m.Access == AccessType::Private) {
		w.DrawImage(x, t, ZImg::priv());
		x += ZImg::prot().GetWidth() + 2;
	}
	
	if (m.Kind == ZItem::itClass) {
		Image img = ZImg::cls;
		x -= 4;
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
	}
	else if (m.Kind == ZItem::itEnum) {
		Image img = ZImg::enm;
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
	}
	else if (m.Kind == ZItem::itFunc) {
		Image img = ZImg::func;
		if (m.Static)
			img = ZImg::statfunc();
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
		
	}
	else if (m.Kind == ZItem::itDef) {
		Image img = ZImg::def;
		if (m.Static)
			img = ZImg::statdef();
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
	}
	else if (m.Kind == ZItem::itThis) {
		Image img = ZImg::constructor;
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
	}
	else if (m.Kind == ZItem::itGet) {
		Image img = ZImg::get;
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
	}
	else if (m.Kind == ZItem::itNamed) {
		Image img = ZImg::nconstructor;
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
	}
	else if (m.Kind == ZItem::itSet) {
		Image img = ZImg::set;
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
	}
	else if (m.Kind == ZItem::itGetSet) {
		Image img = ZImg::getset;
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
	}
	else if (m.Kind == ZItem::itConst) {
		Image img = ZImg::cst;
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
	}
	else if (m.Kind == ZItem::itVar) {
		Image img = ZImg::var;
		if (m.Static)
			img = ZImg::statvar();
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
	}
	else if (m.Kind == ZItem::itDest) {
		Image img = ZImg::destructor;
		w.DrawImage(x, t, img);
		x += img.GetWidth() + 2;
	}
	
	w.DrawText(x, top, m.Name, font, ink);
	x += GetTextSize(m.Name, font).cx;
	
	int i = m.Sig.Find('{');
	
	if (i != -1) {
		String first = m.Sig.Mid(0, i);
		w.DrawText(x, top, first, font, ink);
		x += GetTextSize(first, font).cx;
		x++;
		w.DrawText(x, top, "{", bold, opc);
		x += GetTextSize("{", bold).cx;
		x++;
		
		String rest = m.Sig.Mid(i + 1);
		i = rest.Find('}');
		if (i != -1) {
			String second = rest.Mid(0, i);
			int j = second.Find(':');
			if (j == -1) {
				w.DrawText(x, top, second, font, ink);
				x += GetTextSize(second, font).cx;
			}
			else {
				String pn = second.Mid(0, j);
				String pt = second.Mid(j + 1);
				w.DrawText(x, top, pn, font, ink);
				x += GetTextSize(pn, font).cx;
				w.DrawText(x, top, ":", bold, opc);
				x += GetTextSize(":", bold).cx;
				w.DrawText(x, top, pt, font, ink);
				x += GetTextSize(pt, font).cx;
			}
		
			x++;
			w.DrawText(x, top, "}", bold, opc);
			x += GetTextSize("}", bold).cx;
		}
		else {
			w.DrawText(x, top, rest, font, ink);
			x += GetTextSize(m.Sig, font).cx;
		}
	}
	
	return x;
}
