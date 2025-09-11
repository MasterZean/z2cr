#include <z2crlib/entities.h>
#include <z2crlib/Assembly.h>
#include <z2crlib/ErrorReporter.h>
#include <z2crlib/ZExprParser.h>

String ZEntity::dummy;

void ZVariable::GenerateSignatures() {
	csig = "";
	
	if (IsConst)
		csig << ER::Magenta << "const ";
	else
		csig << ER::Magenta << "val ";
	
	csig << ER::Green << Name;
	
	osig = ER::ToColor(Owner());
}

void ZClass::GenerateSignatures() {
	csig = "";
	
	csig << ER::Magenta << "class ";
	csig << ER::Green << Name;
	
	osig = ER::ToColor(Owner());
}

void ZClass::CopyPreSection(ZClass& cls) {
	for (int i = 0; i < cls.PreVariables.GetCount(); i++)
		PreVariables.Add(cls.PreVariables[i]);
	for (int i = 0; i < PreVariables.GetCount(); i++)
		PreVariables[i].SetOwner(*this);
	
	for (int i = 0; i < cls.PreFunctions.GetCount(); i++)
		PreFunctions.Add(cls.PreFunctions[i]);
	for (int i = 0; i < PreFunctions.GetCount(); i++)
		PreFunctions[i].SetOwner(*this);
	
	for (int i = 0; i < cls.PreConstructors.GetCount(); i++)
		PreConstructors.Add(cls.PreConstructors[i]);
	for (int i = 0; i < PreConstructors.GetCount(); i++)
		PreConstructors[i].SetOwner(*this);
}

void ZFunction::ParseSignatures(ZCompiler& comp) {
	ZParser parser(ParamPos);
	
	bool paramList = true;
	
	if (IsProperty && (IsGetter || IsSimpleGetter))
		paramList = false;
	
	if (paramList) {
		parser.Expect('(');
		
		while (!parser.IsChar(')')) {
			bool isVal = false;
			bool isRef = false;
			
			if (parser.IsId("val")) {
				parser.ReadId();
				isVal = true;
			}
			else if (parser.IsId("ref")) {
				parser.ReadId();
				isVal = true;
			}
			
			auto pp = parser.GetFullPos();
			String name = parser.ExpectId();
			
			if (name == Name)
				throw ER::Duplicate(name, pp, DefPos);
			parser.Expect(':');
			
			auto ti = ZExprParser::ParseType(comp, parser, nullptr, &Owner());
			
			if (parser.Char(',')) {
				if (parser.IsChar(')'))
					parser.Error(parser.GetPoint(), "identifier expected, " + parser.Identify() + " found");
			}
			
			bool found = false;
			for (int i = 0; i < Params.GetCount(); i++)
				if (Params[i].Name == name)
					found = true;
				
			if (!found) {
				ZVariable& var = Params.Add(ZVariable(Namespace()));
				var.Name = name;
				var.BackName = name;
				var.I = ti;
				var.DefPos = pp;
				var.PType = ZVariable::ParamType::tyAuto;
				var.IsConst = !isVal;
			}
		}
		
		parser.Expect(')');
	}
	
	if (parser.Char(':')) {
		auto ti = ZExprParser::ParseType(comp, parser);
		Return.Tt = ti.Tt;
	}
	else
		Return.Tt = Ass().CVoid->Tt;
}

void ZFunction::GenerateSignatures(ZCompiler& comp) {
	fsig = "";
	dsig = "";
	csig = "";
	
	if (InClass == false) {
		fsig << Namespace().ProperName << "::";
		
		if (IsFunction) {
			fsig << "func ";
			csig << ER::Magenta << "func ";
		}
		else {
			fsig << "def ";
			csig << ER::Magenta << "def ";
		}
		
		dsig << "global ";
		dsig << "func ";
	}
	else {
		if (IsConstructor) {
		}
		else if (IsStatic) {
			dsig << "static ";
			dsig << "func ";
			
			if (IsFunction) {
				csig << ER::Magenta << "func ";
			}
			else {
				csig << ER::Magenta << "def ";
			}
		}
		else {
			if (IsProperty) {
				if (IsFunction) {
					dsig << "func ";
					csig << ER::Magenta << "property ";
				}
				else {
					dsig << "def ";
					csig << ER::Magenta << "property def ";
				}
			}
			else if (IsFunction) {
				dsig << "func ";
				csig << ER::Magenta << "func ";
			}
			else {
				dsig << "def ";
				csig << ER::Magenta << "def ";
			}
		}
	}
	
	dsig << Name;
	fsig << Name;

	if (IsConstructor)
		csig << ER::Magenta << Name;
	else
		csig << ER::Green << Name;
	csig << ER::White;
	
	String lpsig = IsConstructor ? "{" : "(";
	String psigc = IsConstructor ? "{" : "(";
	
	for (int i = 0; i < Params.GetCount(); i++) {
		ZVariable& var = Params[i];
		
		if (i > 0) {
			psig << ", ";
			psigc << ", ";
		}
		
		psig << Ass().TypeToString(var.I.Tt);
		psigc << Ass().TypeToColor(var.I.Tt);
	}
	
	psig << (IsConstructor ? "}" : ")");
	psigc << (IsConstructor ? "}" : ")");
	
	dsig << psig;
	fsig << psig;
	csig << psigc;
	psig = lpsig;
	
	if (Return.Tt.Class && Return.Tt.Class != Ass().CVoid) {
		fsig << ": ";
		fsig << Ass().TypeToString(Return.Tt);
		
		csig << ": ";
		csig << Ass().TypeToColor(Return.Tt);
	}
	
	osig = ER::ToColor(Owner());
}

void ZFunction::SetInUse() {
	if (InUse)
		return;
	
	InUse = true;
	
	for (int i = 0; i < Params.GetCount(); i++)
		Params[i].I.Tt.Class->SetInUse();
}

ZFunction& ZNamespace::PrepareConstructor(const String& aName) {
	ZFunction& f = PreConstructors.Add(ZFunction(*this));
	f.Name = aName;
	return f;
}

ZFunction& ZNamespace::PrepareFunction(const String& aName) {
	ZFunction& f = PreFunctions.Add(ZFunction(*this));
	f.Name = aName;
	return f;
}

ZVariable& ZNamespace::PrepareVariable(const String& aName) {
	ZVariable& f = PreVariables.Add(ZVariable(*this));
	f.Name = aName;
	return f;
}

ZClass& ZNamespace::PrepareClass(const String& aName) {
	ZClass& f = PreClasses.Add(ZClass(*this));
	f.Name = aName;
	return f;
}

extern int tabAss[][14];

bool ObjectInfo::CanAssign(Assembly& ass, Node* node) {
	return CanAssign(ass, *node,
		node->Tt.Class == ass.CInt && node->NT == NodeType::Const && node->IntVal >= 0);
}

bool ObjectInfo::CanAssign(Assembly& ass, ObjectInfo& y, bool isCt) {
	if (Tt.Class == ass.CVoid || y.Tt.Class == ass.CVoid)
		return false;

	if (ass.IsPtr(Tt))
		return y.Tt.Class == ass.CNull ||
			(ass.IsPtr(y.Tt) && (y.Tt.Next->Class == Tt.Next->Class || Tt.Next->Class == ass.CVoid));

	if (isCt && Tt.Class == ass.CPtrSize && (y.Tt.Class == ass.CInt))
		return true;

	if (Tt.Class == y.Tt.Class)
		return true;
	
	if (ass.IsNumeric(Tt) && ass.IsNumeric(y.Tt)) {
		ASSERT(y.C1);
		int t1 = Tt.Class->Index - 4;
		int t2 = y.C1->Index - 4;
		ASSERT(t1 >= 0 && t1 <= 13);
		ASSERT(t2 >= 0 && t2 <= 13);
		if (tabAss[t1][t2])
			return true;
		else {
			if (y.C2 != NULL) {
				t2 = y.C2->Index - 4;
				return tabAss[t1][t2];
			}
			else
				return false;
		}
	}

	return false;//BaseExprParser::TypesEqualDeep(ass, &this->Tt, &y.Tt);
}

ZNamespaceItem* ZNamespaceItem::Add(const String& aName) {
	ZNamespaceItem& newns = Names.GetAdd(aName);
	
	return &newns;
}

bool ZNamespace::HasMember(const String& aName) {
	int index = Methods.Find(aName);
	if (index != -1)
		return true;
	index = Variables.Find(aName);
	if (index != -1)
		return true;
	
	return false;
}

void ZNamespace::SetInUse() {
	if (InUse)
		return;
	
	InUse = true;
	
	if (IsClass == false)
		return;
	
	for (int k = 0; k < LibLink.GetCount(); k++)
		ass.LibLink.FindAdd(LibLink[k]);
	
	for (int i = 0; i < Variables.GetCount(); i++) {
		ZVariable& v = *Variables[i];
		
		if (!v.IsStatic) {
			if (v.I.Tt.Class)
				v.I.Tt.Class->SetInUse();
		}
	}
}

int tabAss[][14] = {
	              /*    b,  s8, u8, s16, u16, s32, u32, s64, u64, f32, f64, f80, c,  p*/
	/*  0: Bool    */ { 1,  0,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0 },
	/*  1: Small   */ { 0,  1,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0 },
	/*  2: Byte    */ { 0,  0,  1,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0 },
	/*  3: Short   */ { 0,  1,  1,  1,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0 },
	/*  4: Word    */ { 0,  0,  1,  0,   1,   0,   0,   0,   0,   0,   0,   0,   0,  0 },
	/*  5: Int     */ { 0,  1,  1,  1,   1,   1,   0,   0,   0,   0,   0,   0,   0,  0 },
	/*  6: DWord   */ { 0,  0,  1,  0,   1,   0,   1,   0,   0,   0,   0,   0,   0,  0 },
	/*  7: Long    */ { 0,  1,  1,  1,   1,   1,   1,   1,   0,   0,   0,   0,   0,  0 },
	/*  8: QWord   */ { 0,  0,  1,  0,   1,   0,   1,   0,   1,   0,   0,   0,   0,  0 },
	/*  9: Float   */ { 0,  1,  1,  1,   1,   0,   0,   0,   0,   1,   0,   0,   0,  0 },
	/* 10: Double  */ { 0,  1,  1,  1,   1,   1,   1,   0,   0,   1,   1,   0,   0,  0 },
	/* 11: Real80  */ { 0,  1,  1,  1,   1,   1,   1,   1,   1,   1,   1,   1,   0,  0 },
	/* 12: Char    */ { 0,  1,  1,  1,   1,   1,   1,   0,   0,   0,   0,   0,   1,  0 },
	/* 13: PtrSize */ { 0,  0,  0,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  1 },
};

