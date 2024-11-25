#include "z2cr.h"

extern char SingleOp[24];
extern char DoubleOpCh1[9];
extern char DoubleOpCh2[9];

void ZScanner::Scan() {
	nameSpace = &ass.DefaultNamespace();
	
	while (!parser.IsEof()) {
		auto p = parser.GetFullPos();
		if (parser.Id("namespace")) {
			ScanNamespace(p);
			
			if (parser.Char('{')) {
				bool back = inNamespaceBlock;
				inNamespaceBlock = true;
				
				while (!parser.IsChar('}')) {
					auto p = parser.GetFullPos();
					ScanSingle(p);
				}
				
				parser.Expect('}');
				
				inNamespaceBlock = back;
			}
			else
				parser.ExpectEndStat();
		}
		else {
			ScanSingle(p);
		}
	}
}

void ZScanner::ScanSingle(const ZSourcePos& p) {
	if (parser.Id("using"))
		ScanUsing(p);
	else if (parser.Id("private")) {
		if (parser.Char('{')) {
			while (!parser.IsChar('}')) {
				auto p = parser.GetFullPos();
				if (ScanDeclaration(p, AccessType::Private)) {
					// ALL GOOD
				}
				else {
					Point p = parser.GetPoint();
					parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
				}
			}
		
			parser.Expect('}');
		}
		else if (ScanDeclaration(p, AccessType::Private)) {
			// ALL GOOD
		}
		else {
			Point p = parser.GetPoint();
			parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
		}
	}
	else if (parser.Id("protected")) {
		if (parser.Char('{')) {
			while (!parser.IsChar('}')) {
				auto p = parser.GetFullPos();
				if (ScanDeclaration(p, AccessType::Protected)) {
					// ALL GOOD
				}
				else {
					Point p = parser.GetPoint();
					parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
				}
			}
			
			parser.Expect('}');
		}
		else if (ScanDeclaration(p, AccessType::Protected)) {
			// ALL GOOD
		}
		else {
			Point p = parser.GetPoint();
			parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
		}
	}
	else if (parser.Id("namespace")) {
		ScanNamespace(p);
		
		if (parser.Char('{')) {
			bool back = inNamespaceBlock;
			inNamespaceBlock = true;
			
			while (!parser.IsChar('}')) {
				auto p = parser.GetFullPos();
				ScanSingle(p);
			}
			
			parser.Expect('}');
			
			inNamespaceBlock = back;
		}
		else
			parser.ExpectEndStat();
	}
	else if (ScanDeclaration(p, AccessType::Public)) {
		// ALL GOOD
	}
	else {
		Point p = parser.GetPoint();
		parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
	}
}

bool ZScanner::ScanDeclaration(const ZSourcePos& p, AccessType accessType) {
	const ZSourcePos* tp = nullptr;
	
	if (parser.IsChar2('@', '[')) {
		tp = &p;
		TraitLoop();
	}
		
	bool isStatic = false;
	
	if (parser.IsId("static")) {
		parser.ReadId();
		isStatic = true;
	}
	
	return ScanDeclarationItem(accessType, tp, isStatic);
}

bool ZScanner::ScanDeclarationItem(AccessType accessType, const ZSourcePos* tp, bool isStatic) {
	ZSourcePos p = parser.GetFullPos();
	
	if (parser.Id("class")) {
		ScanClassBody(p, accessType, isStatic, tp);
	
		return true;
	}
	else if (parser.Id("def")) {
		Vector<ZFunction*> funcs;
		do {
			ZFunction& f = ScanFunc(accessType, false, isStatic);
			if (tp)
				f.TraitPos = *tp;
			
			funcs.Add(&f);
		} while (parser.Char(','));
		
		ZSourcePos bp = parser.GetFullPos();
		ScanBlock();
		
		for (int i = 0; i < funcs.GetCount(); i++)
			funcs[i]->BodyPos = bp;
		
		return true;
	}
	else if (parser.Id("func")) {
		Vector<ZFunction*> funcs;
		
		do {
			ZFunction& f = ScanFunc(accessType, true, isStatic);
			if (tp)
				f.TraitPos = *tp;
			
			funcs.Add(&f);
		} while (parser.Char(','));
		
		ZSourcePos bp = parser.GetFullPos();
		ScanBlock();
		
		for (int i = 0; i < funcs.GetCount(); i++)
			funcs[i]->BodyPos = bp;
		
		return true;
	}
	else if (parser.Id("val")) {
		return ScanVar(accessType, false, isStatic);
	}
	else if (parser.Id("const")) {
		return ScanVar(accessType, true, isStatic);
	}
	
	return false;
}

void ZScanner::ScanClassBody(const ZSourcePos& p, AccessType accessType, bool isStatic, const ZSourcePos* tp) {
	ASSERT(nameSpace);

	ZSourcePos dp = parser.GetFullPos();
	String name = parser.ExpectId();
	
	curClass = &nameSpace->PrepareClass(name);
	curClass->DefPos = dp;
	
	parser.Expect('{');
	
	bool back = inClass;
	inClass = true;
	
	while (!parser.IsChar('}')) {
		auto p = parser.GetFullPos();
		ScanSingle(p);
	}
	
	parser.Expect('}');

	if (nameSpace == &ass.DefaultNamespace())
		throw ER::ErrDeclOutsideNamespace(p);
	
	curClass = nullptr;
	
	inClass = back;
	
	if (inClass)
		Errors << ER::ErrNestedClasses(p);
}

void ZScanner::ScanType() {
	parser.ExpectId();
	while (parser.Char('.'))
		parser.ExpectId();
}

void ZScanner::ScanUsing(const ZSourcePos& p) {
	parser.ExpectId("def");
	
	String name = parser.ExpectId();
	String fullName = name;
	fullName << ".";
	
	while (parser.Char('.')) {
		name = parser.ExpectId();
		fullName << name << ".";
	}
	
	if (nameSpace == nullptr) {
		usingNames << fullName;
	}
	else {
		ASSERT(section);
		section->UsingNames.FindAdd(fullName);
	}
	
	parser.ExpectEndStat();
	
	if (inClass)
		Errors << ER::ErrUsingInClass(p);
}

void ZScanner::ScanNamespace(const ZSourcePos& p) {
	ZSourcePos np = parser.GetFullPos();
	
	String name = parser.ExpectId();
	String fullName = name;
	fullName << ".";
	
	while (parser.Char('.')) {
		name = parser.ExpectId();
		fullName << name << ".";
	}
	
	// TODO: delay throw Errors <<
	if (inNamespaceBlock)
		throw ER::ErrNestedNamespace(np);
	if (inClass)
		Errors << ER::ErrNamespaceInClass(np);
	
	int index = ass.Namespaces.Find(fullName);
	
	nameSpace = &ass.FindAddNamespace(fullName);
	
	section = &nameSpace->Sections.Add();
	section->UsingNames = usingNames;
	
	namespaceCount++;
}

bool ZScanner::ScanVar(AccessType accessType, bool aConst, bool isStatic) {
	ZSourcePos dp = parser.GetFullPos();
		
	String name = parser.ExpectId();
	
	ZVariable& f = (curClass ? curClass: nameSpace)->PrepareVariable(name);
	f.DefPos = dp;
	f.BackName = name;
	f.Section = section;
	f.Access = accessType;
	f.IsConst = aConst;
	f.InClass = curClass != nullptr;
		
	if (parser.Char(':')) {
		ScanType();
		
		if (aConst && !parser.IsChar('='))
			parser.Error(dp.P, "constant must be intilized with a compile-time value");
		
		if (parser.Char('='))
			while (!(parser.IsChar(';')/* || (parser.GetSkipNewLines() == false && (parser.PeekChar() == '\n' || parser.PeekChar() == '\r'))*/)) {
				if (parser.IsChar('{'))
					ScanBlock();
				else
					ScanToken();
			}
					
	}
	else {
		if (!parser.IsChar('=')) {
			if (aConst)
				parser.Error(dp.P, "constant must be intilized with a compile-time value");
			else
				parser.Error(dp.P, String(aConst ? "constant" : "variable") + " must have either an explicit type or be initialized");
		}
		
		parser.Expect('=');
		
		while (!(parser.IsChar(';'))) {
			if (parser.IsChar('{'))
					ScanBlock();
				else
					ScanToken();
		}
	}
	
	if (nameSpace == &ass.DefaultNamespace())
		throw ER::ErrDeclOutsideNamespace(dp);
		
	parser.ExpectEndStat();
	
	if (curClass == nullptr && isStatic) {
		if (aConst)
			Errors << ER::ErrCantBeStatic(dp, "namespace symbolic constant");
		else
			Errors << ER::ErrCantBeStatic(dp, "namespace variable");
	}
	
	return true;
}

ZFunction& ZScanner::ScanFunc(AccessType accessType, bool aFunc, bool isStatic) {
	ASSERT(nameSpace);
	
	ZSourcePos dp = parser.GetFullPos();
	
	String name;
	String bname;
	
	if (parser.Char('@')) {
		String s = parser.ExpectId();
		if (s == "size")
			parser.Error(dp.P, "'@size' can't be overlaoded");
		name = "@" + s;
		bname = "_" + s;
	}
	else {
		name = parser.ExpectId();
		bname = name;
	}
	
	ZSourcePos pp = parser.GetFullPos();
	parser.Expect('(');
	
	while (!parser.IsChar(')')) {
		if (parser.IsId("val"))
			parser.ReadId();
		
		parser.ExpectId();
		parser.Expect(':');
		
		ScanType();
		
		if (parser.Char(',')) {
			if (parser.IsChar(')'))
				parser.Error(parser.GetPoint(), "identifier expected, " + parser.Identify() + " found");
		}
	}
	
	parser.Expect(')');
	
	if (parser.Char(':'))
		ScanType();
	
	ZFunction& f = (curClass ? curClass: nameSpace)->PrepareFunction(name);
	f.IsFunction = aFunc;
	f.DefPos = dp;
	f.ParamPos = pp;
	f.BackName = bname;
	f.Section = section;
	f.Access = accessType;
	f.InClass = curClass != nullptr;
	
	if (curClass == nullptr && isStatic) {
		Errors << ER::ErrCantBeStatic(dp, "namespace function");
	}
	
	return f;
}

void ZScanner::ScanBlock() {
	parser.Expect('{');
	parser.EatNewlines();
	
	while (!parser.IsChar('}')) {
		if (parser.IsChar('{')) {
			parser.EatNewlines();
		    ScanBlock();
		}
		else
			ScanToken();
	}
	
	parser.Expect('}');
	parser.EatNewlines();
}

void ZScanner::ScanToken() {
	if (parser.IsInt()) {
		int64 oInt;
		double oDub;
		int base;
		parser.ReadInt64(oInt, oDub, base);
	}
	else if (parser.IsString())
		parser.ReadString();
	else if (parser.IsId())
		parser.ReadId();
	else if (parser.IsCharConst())
		parser.ReadChar();
	else {
		for (int i = 0; i < 9; i++)
			if (parser.Char2(DoubleOpCh1[i], DoubleOpCh1[i]))
			    return;
		for (int i = 0; i < 24; i++)
			if (parser.Char(SingleOp[i]))
			    return;
		if (parser.Char('{') || parser.Char('}'))
			return;

		Point p = parser.GetPoint();
		parser.Error(p, "syntax error: " + parser.Identify() + " found");
	}
}

void ZScanner::InterpretTrait(const String& trait) {
	if (trait == "bindc")
		bindName = trait;
	else if (trait == "intrinsic")
		isIntrinsic = true;
	else if (trait == "dllimport")
		isDllImport = true;
	else if (trait == "stdcall")
		isStdCall = true;
	else if (trait == "cdecl")
		isCDecl = true;
	else if (trait == "nodoc")
		isNoDoc = true;
	else if (trait == "force")
		isForce = true;
}

void ZScanner::TraitLoop() {
	bindName = "";
	isIntrinsic = false;
	isDllImport = false;
	isStdCall = false;
	isCDecl = false;
	isNoDoc = false;
	isForce = false;
	
	if (parser.Char2('@', '[')) {
		
		String trait = parser.ExpectId();
		InterpretTrait(trait);
		
		while (!parser.IsChar(']')) {
			parser.Expect(',');
			
			trait = parser.ExpectId();
			InterpretTrait(trait);
		}
		
		parser.Expect(']');
	}
}
