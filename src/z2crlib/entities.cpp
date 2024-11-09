#include <z2crlib/entities.h>
#include <z2crlib/Assembly.h>

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

