#include "z2cr.h"

 void IR::fillSignedTypeInfo(int64 l, Node* node, ZClass* cls) {
	node->Tt = ass.CInt->Tt;
	if (l >= -128 && l <= 127) {
		node->C1 = ass.CSmall;
		if (l >= 0)
			node->C2 = ass.CByte;
	}
	else if (l >= 0 && l <= 255) {
		node->C1 = ass.CShort;
		node->C2 = ass.CByte;
	}
	else if (l >= -32768 && l <= 32767) {
		node->C1 = ass.CShort;
		if (l >= 0)
			node->C2 = ass.CWord;
	}
	else if (l >= 0 && l <= 65535) {
		node->C1 = ass.CInt;
		node->C2 = ass.CWord;
	}
	else if (l >= -2147483648LL && l <= 2147483647) {
		node->C1 = ass.CInt;
		if (l >= 0)
			node->C2 = ass.CDWord;
	}
	else {
		node->Tt = ass.CLong->Tt;
		node->C1 = ass.CLong;
		if (l >= 0 && l <= 4294967295)
			node->C2 = ass.CDWord;
		else if (l >= 0 && l <= 9223372036854775807ul)
			node->C2 = ass.CQWord;
	}
	//if (cls)
	//	node->Tt = cls->Tt;
}

void IR::fillUnsignedTypeInfo(uint64 l, Node* node, ZClass* cls) {
	node->Tt = ass.CDWord->Tt;
	node->C1 = ass.CDWord;
	if (l <= 127) {
		node->C1 = ass.CByte;
		node->C2 = ass.CSmall;
	}
	else if (l <= 255)
		node->C1 = ass.CByte;
	else if (l <= 32767) {
		node->C1 = ass.CWord;
		node->C2 = ass.CShort;
	}
	else if (l <= 65535)
		node->C1 = ass.CWord;
	else if (l <= 2147483647)
		node->C2 = ass.CInt;
	else if (l <= 4294967295) {
	}
	else if (l <= 9223372036854775807ul) {
		node->Tt = ass.CQWord->Tt;
		node->C1 = ass.CQWord;
		node->C2 = ass.CLong;
	}
	else {
		node->Tt = ass.CQWord->Tt;
		node->C1 = ass.CQWord;
	}
	if (cls)
		node->Tt = cls->Tt;
}

ConstNode* IR::const_i(int64 l, ZClass* cls, int base) {
	ConstNode* node = constNodes.Get();
	
	fillSignedTypeInfo(l, node, cls);
	
	node->IntVal = l;
	node->IsConst = true;
	node->IsCT = true;
	node->IsLiteral = true;
	node->Base = base;
	node->IsTemporary = true;
	ASSERT(node->Tt.Class);
	
	return node;
}

ConstNode* IR::const_u(uint64 l, ZClass* cls, int base) {
	ConstNode* node = constNodes.Get();
	
	fillUnsignedTypeInfo(l, node, cls);
	
	node->IntVal = l;
	node->IsConst = true;
	node->IsCT = true;
	node->IsLiteral = true;
	node->Base = base;
	node->IsTemporary = true;
	ASSERT(node->Tt.Class);
	
	return node;
}

ConstNode* IR::const_r32(double l) {
	ConstNode* node = constNodes.Get();
	
	node->SetType(ass.CFloat->Tt);
	node->IsConst = true;
	node->IsLiteral = true;
	node->IsCT = true;
	node->DblVal = l;
	node->IsTemporary = true;
	ASSERT(node->Tt.Class);
	
	return node;
}

ConstNode* IR::const_r64(double l) {
	ConstNode* node = constNodes.Get();
	
	node->SetType(ass.CDouble->Tt);
	node->IsConst = true;
	node->IsLiteral = true;
	node->IsCT = true;
	node->DblVal = l;
	node->IsTemporary = true;
	ASSERT(node->Tt.Class);
	
	return node;
}