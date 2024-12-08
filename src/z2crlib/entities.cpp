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

void ZFunction::GenerateSignatures(ZCompiler& comp) {
	ZParser parser(ParamPos);
	
	parser.Expect('(');
	
	while (!parser.IsChar(')')) {
		bool isVal = false;
		
		if (parser.IsId("val")) {
			parser.ReadId();
			isVal = true;
		}
		
		auto pp = parser.GetFullPos();
		String name = parser.ExpectId();
		
		if (name == Name)
			throw ER::Duplicate(name, pp, DefPos);
		parser.Expect(':');
		
		auto ti = ZExprParser::ParseType(comp, parser);
		
		if (parser.Char(',')) {
			if (parser.IsChar(')'))
				parser.Error(parser.GetPoint(), "identifier expected, " + parser.Identify() + " found");
		}
		
		ZVariable& var = Params.Add(ZVariable(Namespace()));
		var.Name = name;
		var.BackName = name;
		var.I = ti;
		var.DefPos = pp;
		var.PType = ZVariable::ParamType::tyAuto;
		var.IsConst = !isVal;
	}
	
	parser.Expect(')');
	
	if (parser.Char(':')) {
		auto ti = ZExprParser::ParseType(comp, parser);
		Return.Tt = ti.Tt;
	}
	else
		Return.Tt = Ass().CVoid->Tt;
	
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
		if (IsStatic) {
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
			if (IsFunction) {
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

	csig << ER::Green << Name;
	
	String psig = "(";
	String psigc = String(ER::White) + "(";
	
	for (int i = 0; i < Params.GetCount(); i++) {
		ZVariable& var = Params[i];
		
		if (i > 0) {
			psig << ", ";
			psigc << ", ";
		}
		
		psig << Ass().TypeToString(var.I.Tt);
		psigc << Ass().TypeToColor(var.I.Tt);
	}
	
	psig << ")";
	psigc << ")";
	
	dsig << psig;
	fsig << psig;
	csig << psigc;
	
	if (Return.Tt.Class && Return.Tt.Class != Ass().CVoid) {
		fsig << ": ";
		fsig << Ass().TypeToString(Return.Tt);
		
		csig << ": ";
		csig << Ass().TypeToColor(Return.Tt);
	}
	
	osig = ER::ToColor(Owner());
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

