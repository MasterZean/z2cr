#include "z2cr.h"

String strops[] = { "@add", "@sub", "@mul", "@div", "@mod", "@shl", "@shr", "@less", "@lesseq", "@more", "@moreeq", "@eq", "@neq", "@minus", "@plus", "@not", "@bitnot" };
String ops[]  = { "add", "sub", "mul", "div", "mod", "shl",  "shr",  "less", "lseq", "more", "mreq", "eq",  "neq" "band", "bxor", "bor",  "land", "lor"  };
String opss[] = { "+",   "-",   "*",   "/",   "%",   "<<",   ">>",   "<",    "<=",   ">",    ">=",   "==",  "!=", "&",    "^",    "|",    "&&",   "|| "  , "=",  "+",   "-",  "!",   "~",   "++", "--", "?"  };

int tabAdd[][14] = {
	              /*   b,  s8, u8, s16, u16, s32, u32, s64, u64, f32, f64,  f80, c,   p */
	/*  0: Bool    */ { 1,  1,  2,  3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13 },
	/*  1: Small   */ { 1,  1,  2,  3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13 },
	/*  2: Byte    */ { 2,  2,  2,  3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13 },
	/*  3: Short   */ { 3,  3,  3,  3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13 },
	/*  4: Word    */ { 4,  4,  4,  4,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13 },
	/*  5: Int     */ { 5,  5,  5,  5,   5,   5,   6,   7,   8,   9,   10,  11,  12,  13 },
	/*  6: DWord   */ { 6,  6,  6,  6,   6,   6,   6,   7,   8,   9,   10,  11,  12,  13 },
	/*  7: Long    */ { 7,  7,  7,  7,   7,   7,   7,   7,   8,   9,   10,  11,  12,  13 },
	/*  8: QWord   */ { 8,  8,  8,  8,   8,   8,   8,   8,   8,   9,   10,  11,  12,  13 },
	/*  9: Float   */ { 9,  9,  9,  9,   9,   9,   9,   9,   9,   9,   10,  11,  -1,  -1 },
	/* 10: Double  */ { 10, 10, 10, 10,  10,  10,  10,  10,  10,  10,  10,  11,  -1,  -1 },
	/* 11: Real80  */ { 11, 11, 11, 11,  11,  11,  11,  11,  11,  11,  11,  11,  -1,  -1 },
	/* 12: Char    */ { 12, 12, 12, 12,  12,  12,  12,  12,  12,  -1,  -1,  -1,  12,  -1 },
	/* 13: PtrSize */ { 13, 13, 13, 13,  13,  13,  13,  13,  13,  -1,  -1,  -1,  -1,  13 },
};

int tabRel[][14] = {
	              /*   b,  s8, u8, s16, u16, s32, u32, s64, u64, f32, f64,  f80, c,   p */
	/*  0: Bool    */ { 5,  5,  5,  5,   5,   5,   5,   5,   5,  10,   10,  10,  5,   5  },
	/*  1: Small   */ { 5,  5,  5,  5,   5,   5,   5,   5,   5,  10,   10,  10,  5,   5  },
	/*  2: Byte    */ { 5,  5,  5,  5,   5,   5,   5,   5,   5,  10,   10,  10,  5,   5  },
	/*  3: Short   */ { 5,  5,  5,  5,   5,   5,   5,   5,   5,  10,   10,  10,  5,   5  },
	/*  4: Word    */ { 4,  4,  4,  4,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13 },
	/*  5: Int     */ { 5,  5,  5,  5,   5,   5,   5,   5,   5,  10,   10,  10,  5,   5  },
	/*  6: DWord   */ { 5,  5,  5,  5,   5,   5,   5,   5,   5,  10,   10,  10,  5,   5  },
	/*  7: Long    */ { 5,  5,  5,  5,   5,   5,   5,   5,   5,  10,   10,  10,  5,   5  },
	/*  8: QWord   */ { 5,  5,  5,  5,   5,   5,   5,   5,   5,  10,   10,  10,  5,   5  },
	/*  9: Float   */ { 10, 10, 10, 10,  10,  10,  10,  10,  10, 10,   10,  10,  10,  10 },
	/* 10: Double  */ { 10, 10, 10, 10,  10,  10,  10,  10,  10, 10,   10,  10,  10,  10 },
	/* 11: Real80  */ { 10, 10, 10, 10,  10,  10,  10,  10,  10, 10,   10,  10,  10,  10 },
	/* 12: Char    */ { 5,  5,  5,  5,   5,   5,   5,   5,   5,  10,   10,  10,  5,   5  },
	/* 13: PtrSize */ { 5,  5,  5,  5,   5,   5,   5,   5,   5,  10,   10,  10,  5,   5  },
};

int tabSft[][14] = {
	              /*    b,  s8, u8, s16, u16, s32, u32, s64, u64, f32, f64, f80, c,   p       */
	/*  0: Bool    */ { -1, -1, -1, -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1 },
	/*  1: Small   */ { -1,  5,  5,  5,   5,   5,   5,   5,   5,  -1,  -1,  -1,  -1,   5 },
	/*  2: Byte    */ { -1,  6,  6,  6,   6,   6,   6,   6,   6,  -1,  -1,  -1,  -1,   6 },
	/*  3: Short   */ { -1,  5,  5,  5,   5,   5,   5,   5,   5,  -1,  -1,  -1,  -1,   5 },
	/*  4: Word    */ { -1,  6,  6,  6,   6,   6,   6,   6,   6,  -1,  -1,  -1,  -1,   6 },
	/*  5: Int     */ { -1,  5,  5,  5,   5,   5,   5,   5,   5,  -1,  -1,  -1,  -1,   5 },
	/*  6: DWord   */ { -1,  6,  6,  6,   6,   6,   6,   6,   6,  -1,  -1,  -1,  -1,   6 },
	/*  7: Long    */ { -1,  7,  7,  7,   7,   7,   7,   7,   7,  -1,  -1,  -1,  -1,   7 },
	/*  8: QWord   */ { -1,  8,  8,  8,   8,   8,   8,   8,   8,  -1,  -1,  -1,  -1,   8 },
	/*  9: Float   */ { -1,  9,  9,  9,   9,   9,   9,   9,   9,  -1,  -1,  -1,  -1,   9 },
	/* 10: Double  */ { -1, 10, 10, 10,  10,  10,  10,  10,  10,  -1,  -1,  -1,  -1,  10 },
	/* 11: Real80  */ { -1, 11, 11, 11,  11,  11,  11,  11,  11,  -1,  -1,  -1,  -1,  11 },
	/* 12: Char    */ { -1, 12, 12, 12,  12,  12,  12,  12,  12,  -1,  -1,  -1,  -1,  12 },
	/* 13: PtrSize */ { -1, 13, 13, 13,  13,  13,  13,  13,  13,  -1,  -1,  -1,  -1,  13 },
};

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

Node* IR::deref(Node* node) {
	// TODO fix
	return node;
};

Node* IR::opArit(Node* left, Node* right, OpNode::Type op, const Point& p) {
	bool valid = ass.IsNumeric(left->Tt) && ass.IsNumeric(right->Tt);
	bool valid2 = false;
	bool valid3 = false;
	
	if ((op == 0 || op == 1) && (
			(left->Tt.Class == ass.CPtr && ass.IsInteger(right->Tt)) ||
			(right->Tt.Class == ass.CPtr && ass.IsInteger(left->Tt))))
		valid2 = true;
	else if (op == 1 &&
			(left->Tt.Class == ass.CPtr && right->Tt.Class == ass.CPtr))
		valid3 = true;
	
	ZFunction* over = nullptr;
	ZClass* cls = nullptr;
	if (!valid && !valid2 && !valid3) {
		// TODO: fix
		ASSERT(0);
		//return GetOp(Over, strops[op], left, right, ass, this, *Comp, Point(1, 1));
	}
	
		ObjectType* type = 0;
	ObjectType* e = 0;
	ZClass* secType = 0;
	bool cst = false;
	
	if (valid2 == false && valid3 == false) {
		int t1 = left->Tt.Class->Index - 4;
		int t2 = right->Tt.Class->Index - 4;
		ASSERT(t1 >= 0 && t1 <= 13);
		ASSERT(t2 >= 0 && t2 <= 13);
		
		int t = tabAdd[t1][t2];

		if (t == -1)
			return NULL;

		t += 4;

		cst = left->IsCT && right->IsCT;
		type = &ass.Classes[t].Tt;
		
		if (left->Tt.Class != right->Tt.Class) {
			if (left->Tt.Class == right->C1)
				secType = right->C1;
			if (left->Tt.Class == right->C2)
				secType = right->C2;
			
			if (right->Tt.Class == left->C1)
				secType = left->C1;
			if (right->Tt.Class == left->C2)
				secType = left->C2;
		}
		
		e = type;
	}
	else if (valid2 == true) {
		if (ass.IsPtr(left->Tt))
			type = &left->Tt;
		else if (ass.IsPtr(right->Tt))
			type = &right->Tt;
		e = type;
	}
	else  if (valid3 == true) {
		type = &ass.CPtrSize->Tt;
		e = type;
	}
	
	bool backCst = cst;
	
	int64 dInt = 0;
	double dDouble = 0;
	
	if (cst) {
		if (op == OpNode::opAdd) {
			if (e->Class == ass.CByte) {
				dInt = (byte)((uint32)left->IntVal + (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CWord) {
				dInt = (word)((uint32)left->IntVal + (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CDWord) {
				dInt = (dword)((uint32)left->IntVal + (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CQWord) {
				dInt = (qword)((uint64)left->IntVal + (uint64)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CSmall) {
				dInt = (int8)((int32)left->IntVal + (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CShort) {
				dInt = (int16)((int32)left->IntVal + (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CInt) {
				dInt = (int32)((int32)left->IntVal + (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CLong) {
				dInt = (int64)((int64)left->IntVal + (int64)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CPtrSize) {
				dInt = (uint64)left->IntVal + (uint64)right->IntVal;
				type = &ass.CPtrSize->Tt;
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CFloat) {
				left->PromoteToFloatValue(ass);
				right->PromoteToFloatValue(ass);
				dDouble = left->DblVal + right->DblVal;
				if (FoldConstants)
					return const_r32(dDouble);
			}
			else if (e->Class == ass.CDouble) {
				left->PromoteToFloatValue(ass);
				right->PromoteToFloatValue(ass);
				dDouble = left->DblVal + right->DblVal;
				if (FoldConstants)
					return const_r64(dDouble);
			}
			else
				ASSERT(0);
		}
		else if (op == OpNode::opSub) {
			if (e->Class == ass.CByte) {
				dInt = (byte)((uint32)left->IntVal - (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CWord) {
				dInt = (word)((uint32)left->IntVal - (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CDWord) {
				dInt = (dword)((uint32)left->IntVal - (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CQWord) {
				dInt = (qword)((uint64)left->IntVal - (uint64)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CSmall) {
				dInt = (int8)((int32)left->IntVal - (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CShort) {
				dInt = (int16)((int32)left->IntVal - (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CInt) {
				dInt = (int32)((int32)left->IntVal - (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CLong) {
				dInt = (int64)((int64)left->IntVal - (int64)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CPtrSize) {
				dInt = (uint64)left->IntVal - (uint64)right->IntVal;
				type = &ass.CPtrSize->Tt;
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CFloat) {
				left->PromoteToFloatValue(ass);
				right->PromoteToFloatValue(ass);
				dDouble = left->DblVal - right->DblVal;
				if (FoldConstants)
					return const_r32(dDouble);
			}
			else if (e->Class == ass.CDouble) {
				left->PromoteToFloatValue(ass);
				right->PromoteToFloatValue(ass);
				dDouble = left->DblVal - right->DblVal;
				if (FoldConstants)
					return const_r64(dDouble);
			}
			else
				ASSERT(0);
		}
		else if (op == OpNode::opMul) {
			if (e->Class == ass.CByte) {
				dInt = (byte)((uint32)left->IntVal * (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CWord) {
				dInt = (word)((uint32)left->IntVal * (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CDWord) {
				dInt = (dword)((uint32)left->IntVal * (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CQWord) {
				dInt = (qword)((uint64)left->IntVal * (uint64)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CSmall) {
				dInt = (int8)((int32)left->IntVal * (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CShort) {
				dInt = (int16)((int32)left->IntVal * (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CInt) {
				dInt = (int32)((int32)left->IntVal * (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CLong) {
				dInt = (int64)((int64)left->IntVal * (int64)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CPtrSize) {
				dInt = (uint64)left->IntVal * (uint64)right->IntVal;
				type = &ass.CPtrSize->Tt;
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CFloat) {
				left->PromoteToFloatValue(ass);
				right->PromoteToFloatValue(ass);
				dDouble = left->DblVal * right->DblVal;
				if (FoldConstants)
					return const_r32(dDouble);
			}
			else if (e->Class == ass.CDouble) {
				left->PromoteToFloatValue(ass);
				right->PromoteToFloatValue(ass);
				dDouble = left->DblVal * right->DblVal;
				if (FoldConstants)
					return const_r64(dDouble);
			}
			else
				ASSERT(0);
		}
		else if (op == OpNode::opDiv) {
			if (e->Class == ass.CByte) {
				dInt = (byte)((uint32)left->IntVal / (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CWord) {
				dInt = (word)((uint32)left->IntVal / (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CDWord) {
				dInt = (dword)((uint32)left->IntVal / (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CQWord) {
				dInt = (qword)((uint64)left->IntVal / (uint64)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CSmall) {
				dInt = (int8)((int32)left->IntVal / (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CShort) {
				dInt = (int16)((int32)left->IntVal / (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CInt) {
				dInt = (int32)((int32)left->IntVal / (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CLong) {
				dInt = (int64)((int64)left->IntVal / (int64)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CPtrSize) {
				dInt = (uint64)left->IntVal / (uint64)right->IntVal;
				type = &ass.CPtrSize->Tt;
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CFloat) {
				left->PromoteToFloatValue(ass);
				right->PromoteToFloatValue(ass);
				dDouble = left->DblVal / right->DblVal;
				if (FoldConstants)
					return const_r32(dDouble);
			}
			else if (e->Class == ass.CDouble) {
				left->PromoteToFloatValue(ass);
				right->PromoteToFloatValue(ass);
				dDouble = left->DblVal / right->DblVal;
				if (FoldConstants)
					return const_r64(dDouble);
			}
			else
				ASSERT(0);
		}
		else if (op == OpNode::opMod) {
			if (e->Class == ass.CByte) {
				dInt = (byte)((uint32)left->IntVal % (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CWord) {
				dInt = (word)((uint32)left->IntVal % (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CDWord) {
				dInt = (dword)((uint32)left->IntVal % (uint32)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CQWord) {
				dInt = (qword)((uint64)left->IntVal % (uint64)right->IntVal);
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CSmall) {
				dInt = (int8)((int32)left->IntVal % (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CShort) {
				dInt = (int16)((int32)left->IntVal % (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CInt) {
				dInt = (int32)((int32)left->IntVal % (int32)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CLong) {
				dInt = (int64)((int64)left->IntVal % (int64)right->IntVal);
				if (FoldConstants)
					return const_i(dInt, type->Class);
			}
			else if (e->Class == ass.CPtrSize) {
				dInt = (uint64)left->IntVal % (uint64)right->IntVal;
				type = &ass.CPtrSize->Tt;
				if (FoldConstants)
					return const_u(dInt, type->Class);
			}
			else if (e->Class == ass.CFloat) {
				left->PromoteToFloatValue(ass);
				right->PromoteToFloatValue(ass);
				dDouble = fmod(left->DblVal, right->DblVal);
				if (FoldConstants)
					return const_r32(dDouble);
			}
			else if (e->Class == ass.CDouble) {
				left->PromoteToFloatValue(ass);
				right->PromoteToFloatValue(ass);
				dDouble = fmod(left->DblVal, right->DblVal);
				if (FoldConstants)
					return const_r64(dDouble);
			}
			else
				ASSERT(0);
		}
		else
			ASSERT(0);
	}
	
	OpNode* node = opNodes.Get();

	if (left->C1 == ass.CBool && right->C1 == ass.CBool && e->Class == ass.CSmall)
		secType = ass.CByte;

	node->OpA = left->IsIndirect ? deref(left) : left;
	node->OpB = right->IsIndirect ? deref(right) : right;
	node->Op = op;

	node->IsCT = backCst;
	node->SetType(type, e->Class, secType);
	node->IntVal = dInt;
	node->DblVal = dDouble;

	ASSERT(node->Tt.Class);
	
	return node;
}