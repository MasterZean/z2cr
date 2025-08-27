#include "ZideWindow.h"

String ToString(Color c) {
	return String().Cat() << "(" << c.GetR() << "." << c.GetG() << "." << c.GetB() << ")";
}

String TrimDocLinks(const String& str) {
	String s;
	
	int i = 0;
	int l = 0;
	
	String cik = ToString(HighlightSetup::hl_style[HighlightSetup::INK_KEYWORD].color);
	
	while (i < str.GetCount()) {
		int first = str.Find('[', i);
		
		if (first != -1) {
			for (int j = l; j < first; j++)
				s << (char)str[j];
			l = first;
			
			int last = str.Find(']', first + 1);
			
			if (last != -1) {
				String t = str.Mid(first + 1, last - first - 1);
				
				int first2 = str.Find('[', last + 1);
				
				if (first2 == last + 1) {
					int last2 = str.Find(']', first2 + 1);
					
					if (last2 != -1) {
						s << "[C@" << cik << "B_ " << t << "]";

						i = last2;
						l = i + 1;
					}
				}
			}
		}
		else
			s << (char)str[i];
		
		i++;
	}
	
	return s;
}

void ZideWindow::OnAnnotation() {
	auto temp = GetEditor();
	if (!temp)
		return;
	
	SmartEditor& editor = *temp;
	
	if(annotation_popup.IsOpen())
		annotation_popup.Close();
	
	int ln = editor.GetActiveAnnotationLine();
	
	String ann = editor.GetAnnotation(ln);
	int aki = ann.Find(' ');
	
	String cik = ToString(HighlightSetup::hl_style[HighlightSetup::INK_KEYWORD].color);
	String bik = HighlightSetup::hl_style[HighlightSetup::INK_KEYWORD].bold ? "*" : "";
	String cin = ToString(HighlightSetup::hl_style[HighlightSetup::INK_NORMAL].color);
	String cpn = ToString(HighlightSetup::hl_style[HighlightSetup::PAPER_NORMAL].color);
	String cic = ToString(HighlightSetup::hl_style[HighlightSetup::INK_UPP].color);
	String bic = HighlightSetup::hl_style[HighlightSetup::INK_UPP].bold ? "*" : "";
	String cio = ToString(HighlightSetup::hl_style[HighlightSetup::INK_OPERATOR].color);
	
	annotation_popup.Background(HighlightSetup::hl_style[HighlightSetup::PAPER_NORMAL].color);
	
	if (aki == -1) {
		int ii = asbAss.Docs.Find(ann);
		if (ii == -1)
			return;
		
		DocEntry& doc = asbAss.Docs[ii];
		
		int lpi = ann.ReverseFind('.');
		String cls = ann.Mid(lpi + 1);
		String ns = ann.Mid(0, lpi + 1);
		
		String qtf;
		
		// text 10
		qtf = "[2@";
		qtf << cin << "\n";
		
		// class
		qtf << "[C ";
		qtf << "[@" << cik << bik << " class] ";
		qtf << DeQtf(ns);
		qtf << "[@" << cic << bic << " " << DeQtf(cls) << "]";
		qtf << "]&\n";
		qtf << "&\n";
		
		String b = TrimDocLinks(doc.Brief);
		b.Replace("\n", "&\n");
		b = TrimBoth(b);
		if (b.EndsWith("&"))
			b = b.Mid(0, b.GetCount() - 1);
		qtf << b;
		
		// text 10
		qtf << "\n]";
		
		annotation_popup.SetQTF(qtf);
		
		Rect r = editor.GetLineScreenRect(ln);
		int w = Zx(600) + Zx(6) * 2 + Zx(1) * 2;
		int h = annotation_popup.GetHeight(w) + Zy(6) * 2 + Zy(1) * 2;
		int y = r.top - h - StdFont(10).GetCy() / 2;
		if(y < GetWorkArea().top)
			y = r.bottom + StdFont(10).GetCy();
		annotation_popup.SetRect(r.left, y, w, h);
	
		if (annotation_popup.IsOpen())
			return;
		annotation_popup.Ctrl::PopUp(&editor, false, false, true);
	
		return;
	}
	
	aki = ann.Find(' ', aki + 1);
	if (aki == -1)
		return;
	
	String key = ann.Mid(0, aki);
	int rki = ann.Find(' ', aki + 1);
	
	String rest = TrimBoth(ann.Mid(rki));
	String toParse = rest;
	
	rest.Replace("[", "\1[\1");
	rest.Replace("]", "\1]\1");
	rest.Replace(":", String().Cat() << "[@" << cio << " :]");
	rest.Replace(";", String().Cat() << "[@" << cio << " ;]");
	rest.Replace("<", String().Cat() << "[@" << cio << " <]");
	rest.Replace(">", String().Cat() << "[@" << cio << " >]");
	rest.Replace("ref ", String().Cat() << "[C@" << cik << bik <<  " ref] ");
	rest.Replace("move ", String().Cat() << "[C@" << cik << bik <<  " move] ");
	rest.Replace("const ", String().Cat() << "[C@" << cik << bik << " const] ");
	
	Index<String> parLook;
	
	int fci = toParse.Find("{");
	if (fci != -1 && toParse.EndsWith("}")) {
		toParse = toParse.Mid(fci + 1, toParse.GetCount() - 1 - fci);
		
		Vector<String> p = Split(toParse, ", ", false);
		
		for (int k = 0; k < p.GetCount(); k++) {
			int iii = p[k].Find(":");
			
			if (iii != -1) {
				String par = p[k].Mid(0, iii);
				if (par.StartsWith("ref "))
					par = par.Mid(4);
				else if (par.StartsWith("move "))
					par = par.Mid(5);
				parLook.Add(par);
			}
		}
	}
	
	fci = toParse.Find("(");
	int fci2 = toParse.Find(")");
	if (fci != -1 && fci2 != -1 && fci2 > fci) {
		toParse = toParse.Mid(fci + 1, fci2 - 1 - fci);
			
		Vector<String> p = Split(toParse, ", ", false);
		
		for (int k = 0; k < p.GetCount(); k++) {
			int iii = p[k].Find(":");
			
			if (iii != -1) {
				String par = p[k].Mid(0, iii);
				if (par.StartsWith("ref "))
					par = par.Mid(4);
				else if (par.StartsWith("move "))
					par = par.Mid(5);
				parLook.Add(par);
			}
		}
	}
	
	if (ln < 0 || ann.GetCount() == 0)
		return;
	
	int ii = asbAss.Docs.Find(key);
	if (ii == -1)
		return;
	
	DocEntry& doc = asbAss.Docs[ii];
	
	Vector<String> v = Split(ann, " ", false);
	String ns = v[0];
	int lpi = ns.ReverseFind('.');
	String cls = ns.Mid(lpi + 1);
	ns = ns.Mid(0, lpi + 1);
	
	String qtf;
	
	// text 10
	qtf = "[2@";
	qtf << cin << "\n";
	
	// class
	qtf << "[C ";
	qtf << "[@" << cik << bik << " class] ";
	qtf << DeQtf(ns);
	qtf << "[@" << cic << bic << " " << DeQtf(cls) << "]";
	qtf << "]&\n";

	qtf << "[C ";
	if (v[2] == "const")
		qtf << "-|[@" << cik << bik << " const] " << DeQtf(v[1]);
	else if (v[2] == "prop")
		qtf << "-|[@" << cik << bik << " property] " << DeQtf(v[1]) << rest;
	else if (v[2] == "props")
		qtf << "-|[@" << cik << bik << " static property] " << DeQtf(v[1]) << rest;
	else if (v[2] == "def")
		qtf << "-|[@" << cik << bik << " def] " << DeQtf(v[1])  << rest;
	else if (v[2] == "defs")
		qtf << "-|[@" << cik << bik << " static def] " << DeQtf(v[1])  << rest;
	else if (v[2] == "func")
		qtf << "-|[@" << cik << bik << " func] " << DeQtf(v[1])  << rest;
	else if (v[2] == "funcs")
		qtf << "-|[@" << cik << bik << " static func] " << DeQtf(v[1]) << rest;
	else if (v[2] == "this")
		qtf << "-|[@" << cik << bik << " this]" << rest;
	else if (v[2] == "var")
		qtf << "-|[@" << cik << bik << " var] " << DeQtf(v[1]);
	qtf << ";";
	qtf << "]&\n";
	
	qtf << "&\n";
	
	String b = TrimDocLinks(TrimBoth(doc.Brief));
	b.Replace("\n", "&\n");
	b = TrimBoth(b);
	if (b.EndsWith("&"))
		b = b.Mid(0, b.GetCount() - 1);
	
	qtf << b;
	
	int count = 0;
	DUMP(parLook);
	DUMP(doc.Params);
	for (int i = 0; i < doc.Params.GetCount(); i++) {
		if (parLook.Find(doc.Params.GetKey(i)) != -1) {
			count++;
			break;
		}
	}
	
	if (count || doc.Returns.GetCount())
		qtf << "&\n&\n";
		
	if (count) {
		String qtf2;
		qtf2 << "{{1:2F" << cin << "G" << cin << "@" << cpn << " ";
		int len = qtf2.GetLength();
		
		for (int i = 0; i < doc.Params.GetCount(); i++) {
			if (parLook.Find(doc.Params.GetKey(i)) != -1) {
				if (qtf2.GetCount() > len)
					qtf2 << "||";
				qtf2 << "[C " << doc.Params.GetKey(i) << "]||" << doc.Params[i];
			}
		}
		
		if (doc.Returns.GetCount())
			qtf2 << "||[@" << cik << "C return]||" << doc.Returns;
		
		qtf2 << "}}";
		
		qtf << qtf2;
	}
	else if (doc.Returns.GetCount()) {
		qtf << "{{1:2F" << cin << "G" << cin << "@" << cpn << " ";
		qtf << "[@" << cik << "C return]||" << doc.Returns;
		qtf << "}}\n";
	}
	
	// text 10
	qtf << "]";
	
	DUMP(qtf);
	
	annotation_popup.SetQTF(qtf);
	
	Rect r = editor.GetLineScreenRect(ln);
	int w = Zx(600);
	int h = annotation_popup.GetHeight(w);
	int y = r.top - h - (StdFont(10).GetCy() / 2);
	if(y < GetWorkArea().top)
		y = r.bottom + StdFont(10).GetCy();
	annotation_popup.SetRect(r.left, y, w + (Zx(6) + 1) * 2, h + (Zx(6) + 1) * 2);

	if (annotation_popup.IsOpen())
		return;
	annotation_popup.Ctrl::PopUp(&editor, false, false, true);
}