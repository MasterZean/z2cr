#include <z2crlib/ZScanner.h>
#include <z2crlib/Assembly.h>
#include <z2crlib/ErrorReporter.h>

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
					ScanSingle(p, false);
				}
				
				parser.Expect('}');
				
				inNamespaceBlock = back;
			}
			else
				parser.ExpectEndStat();
		}
		else {
			ScanSingle(p, false);
		}
	}
}

void ZScanner::ScanSingle(const ZSourcePos& p, bool isStatic) {
	if (parser.Id("using"))
		ScanUsing(p);
	else if (parser.Id("private")) {
		if (parser.Char('{')) {
			while (!parser.IsChar('}')) {
				auto p = parser.GetFullPos();
				if (ScanDeclaration(p, AccessType::Private, isStatic)) {
					// ALL GOOD
				}
				else {
					Point p = parser.GetPoint();
					parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
				}
			}
		
			parser.Expect('}');
		}
		else if (ScanDeclaration(p, AccessType::Private, isStatic)) {
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
				if (ScanDeclaration(p, AccessType::Protected, isStatic)) {
					// ALL GOOD
				}
				else {
					Point p = parser.GetPoint();
					parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
				}
			}
			
			parser.Expect('}');
		}
		else if (ScanDeclaration(p, AccessType::Protected, isStatic)) {
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
				ScanSingle(p, isStatic);
			}
			
			parser.Expect('}');
			
			inNamespaceBlock = back;
		}
		else
			parser.ExpectEndStat();
	}
	else if (ScanDeclaration(p, AccessType::Public, isStatic)) {
		// ALL GOOD
	}
	else {
		Point p = parser.GetPoint();
		parser.Error(p, "syntax error: declaration expected: " + parser.Identify() + " found");
	}
}

bool ZScanner::ScanDeclaration(const ZSourcePos& p, AccessType accessType, bool isStatic) {
	ZTrait trait;
	
	if (parser.IsChar2('@', '[')) {
		trait.TP = &p;
		trait.Flags = TraitLoop();
	}
		
	bool newStatic = false;
	
	if (parser.IsId("static")) {
		parser.ReadId();
		newStatic = true;
	}
	
	return ScanDeclarationItem(accessType, trait, newStatic || isStatic);
}

bool ZScanner::ScanDeclarationItem(AccessType accessType, const ZTrait& trait, bool isStatic) {
	ZSourcePos p = parser.GetFullPos();
	
	if (parser.Id("class"))
		return ScanClassBody(p, accessType, isStatic, trait);
	else if (parser.Id("def"))
		return ScanFuncMulti(accessType, trait, 0, false, isStatic);
	else if (parser.Id("func"))
		return ScanFuncMulti(accessType, trait, 0, true, isStatic);
	else if (parser.Id("this"))
		return ScanFuncMulti(accessType, trait, 1, false, isStatic);
	else if (parser.Id("val"))
		return ScanVar(accessType, false, isStatic);
	else if (parser.Id("const"))
		return ScanVar(accessType, true, isStatic);
	
	return false;
}

bool ZScanner::ScanFuncMulti(AccessType accessType, const ZTrait& trait, int isCons, bool aFunc, bool isStatic) {
	Vector<ZFunction*> funcs;
		
	do {
		ZFunction& f = ScanFunc(accessType, isCons, aFunc, isStatic);
		f.Trait = trait;
		
		funcs.Add(&f);
	} while (parser.Char(','));
	
	if ((trait.Flags & ZTrait::BINDC) || (trait.Flags & ZTrait::BINDCPP))
		parser.ExpectEndStat();
	else {
		ZSourcePos bp = parser.GetFullPos();
		ScanBlock();
		
		for (int i = 0; i < funcs.GetCount(); i++)
			funcs[i]->BodyPos = bp;
	}
	
	return true;
}

bool ZScanner::ScanClassBody(const ZSourcePos& p, AccessType accessType, bool isStatic, const ZTrait& trait) {
	ASSERT(nameSpace);

	ZSourcePos dp = parser.GetFullPos();
	String name = parser.ExpectId();
	
	curClass = &nameSpace->PrepareClass(name);
	curClass->BackName = name;
	curClass->DefPos = dp;
	curClass->IsClass = true;
	curClass->LibLink = std::move(libLink);
	
	parser.Expect('{');
	
	bool back = inClass;
	inClass = true;
	
	while (!parser.IsChar('}')) {
		auto p = parser.GetFullPos();
		ScanSingle(p, isStatic);
	}
	
	parser.Expect('}');

	if (nameSpace == &ass.DefaultNamespace())
		throw ER::ErrDeclOutsideNamespace(p);
	
	curClass = nullptr;
	
	inClass = back;
	
	if (inClass)
		Errors << ER::ErrNestedClasses(p);
	
	return true;
}

void ZScanner::ScanType() {
	parser.ExpectId();
	while (parser.Char('.'))
		parser.ExpectId();
	
	if (parser.Char('<')) {
		ScanSubType();
		
		while (parser.Char(','))
			ScanSubType();
		parser.Expect('>');
	}
}

void ZScanner::ScanSubType() {
	if (parser.IsId("const"))
		ScanType();
	else if (parser.IsId())
		ScanType();
	else
		ScanToken();
}

void ZScanner::ScanUsing(const ZSourcePos& p) {
	bool def = parser.Id("def");
	
	String name = parser.ExpectId();
	String fullName = name;
	String alias;
	
	if (parser.Char('=')) {
		alias = fullName;
		fullName = parser.ExpectId();
	}
	
	fullName << ".";
	
	while (parser.Char('.')) {
		name = parser.ExpectId();
		fullName << name << ".";
	}
	
	parser.ExpectEndStat();
	
	if (def == true) {
		if (nameSpace == nullptr) {
			usingNames << fullName;
		}
		else {
			ASSERT(section);
			section->UsingNames.FindAdd(fullName);
		}
	}
	else {
		String mid = fullName.Mid(0, fullName.GetCount() - 1);
		source.AddReference(mid, p.P);
		
		if (alias.GetCount())
			source.AddAlias(alias, mid);
	}
	
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
	nameSpace->Sources.Add(&source);
	
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
	f.IsStatic = isStatic;
	if (curClass == nullptr)
		f.IsStatic = true;
		
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

ZFunction& ZScanner::ScanFunc(AccessType accessType, int isCons, bool aFunc, bool isStatic) {
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
		if (isCons == 0) {
			name = parser.ExpectId();
			bname = name;
		}
		else {
			name = "this";
			bname = name;
		}
	}
	
	if (isCons && curClass == nullptr)
		parser.Error(dp.P, "constructor defintion found outside of class");
	
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
	
	ZFunction& f = (isCons ? curClass->PrepareConstructor(name) : (curClass ? curClass: nameSpace)->PrepareFunction(name));
	f.IsFunction = aFunc;
	f.DefPos = dp;
	f.ParamPos = pp;
	f.BackName = bname;
	f.Section = section;
	f.Access = accessType;
	f.InClass = curClass != nullptr;
	f.IsStatic = isStatic;
	f.IsConstructor = isCons;
	if (curClass == nullptr)
		f.IsStatic = true;
	
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

int ZScanner::InterpretTrait(ZParser& parser, int flags, const String& trait) {
	if (trait == "bindc") {
		bindName = trait;
		flags = flags | ZTrait::BINDC;
	}
	else if (trait == "bindcpp") {
		bindName = trait;
		flags = flags | ZTrait::BINDCPP;
	}
	else if (trait == "liblink") {
		parser.Expect('=');
		if (!parser.IsString())
			parser.Error(parser.GetPoint(), "string literal expected");
		libLink << parser.ReadString();
	}
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
	
	return flags;
}

int ZScanner::TraitLoop() {
	int flags = 0;
	
	bindName = "";
	isIntrinsic = false;
	isDllImport = false;
	isStdCall = false;
	isCDecl = false;
	isNoDoc = false;
	isForce = false;
	libLink.Clear();
	
	if (parser.Char2('@', '[')) {
		
		String trait = parser.ExpectId();
		flags = InterpretTrait(parser, flags, trait);
		
		while (!parser.IsChar(']')) {
			parser.Expect(',');
			
			trait = parser.ExpectId();
			flags = InterpretTrait(parser, flags, trait);
		}
		
		parser.Expect(']');
	}
	
	return flags;
}
