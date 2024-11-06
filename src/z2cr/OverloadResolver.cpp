#include "OverloadResolver.h"
#include "ZNode.h"

ZFunction* ZFunctionResolver::Resolve(ZMethodBundle& def, Vector<Node*>& params, int limit, ZClass* spec, bool convert) {
	// no result if no overloads exist
	if (def.Functions.GetCount() == 0)
		return nullptr;
	
	Vector<ZFunction*> candidates;
	
	ZFunction *ct = nullptr;
	ZFunction *mt = nullptr;
	
	for (int ii = 0; ii < def.Functions.GetCount(); ii++) {
		ZFunction& over = *def.Functions[ii];
		over.Score = 0;
		
		if (params.GetCount() == over.Params.GetCount()) {
			if (!spec || (spec && over.TParam.GetCount() && over.TParam[0] == spec)) {
				// function that take no parameters are a match by definition
				if (params.GetCount() == 0) {
					if (over.IsFunction)
						ct = &over;
					else
						mt = &over;
					
					/*if (limit == 1) {
						if (over.isconst == false)
					}
					else*/
					if (limit == 0)
						return &over;
					else if (limit == 2) {
						if (ct && mt)
							return ct;
					}
					else if (limit == 1) {
						if (ct && mt)
							return mt;
					}
				}
				
				if (limit == 0 || (limit == 2 && over.IsFunction) || (limit == 1 && !over.IsFunction)) {
					over.Score = 1;
					candidates.Add(&over);
				}
			}
		}
	}
	
	if (ct)
		return ct;
	if (mt)
		return mt;
	
	// no result if no overloads match parameter number
	if (candidates.GetCount() == 0) {
		if (limit == 1)
			return Resolve(def, params, 2, spec, convert);
		else
			return nullptr;
	}
	
	// if specialization if requested, try to look it up for early return
	//if (spec && def.Functions.GetCount() != 0/* && candidates.GetCount() != 0*/) {
		// TODO: fix
		/*ASSERT(candidates.GetCount() == 1);
		
		ZFunction& over = *candidates[0];
		bool fail = false;
		
		for (int jj = 0; jj < params.GetCount(); jj++) {
			Node& ap = *params[jj];
			ObjectInfo& api = ap;
			ObjectInfo& fpi = over.Params[jj].I;
			
			if (!fpi.CanAssign(ass, &ap)) {
				fail = true;
				break;
			}
		}
		
		if (fail)
			return nullptr;
		else
			return &over;
	}*/
	
	score = 1;
	conv = convert;

	ZFunction* result = GatherParIndex(candidates, params, 0);
	
	if (result)
		return result;
	
	if (ambig == 0 && limit == 1)
		return Resolve(def, params, 2, spec, convert);
	else
		return nullptr;
}

ZFunction* ZFunctionResolver::GatherParIndex(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi) {
	Node& n = *params[pi];
	ObjectInfo& a = n;
	
	if (ass.IsNumeric(a.Tt)) {
		ZFunction* o1 = GatherNumeric(oo, params, pi, a.Tt.Class);
		if (conv && !o1 && a.C1) {
			o1 = GatherNumeric(oo, params, pi, a.C1);
			if (!o1 && a.C2) {
				o1 = GatherNumeric(oo, params, pi, a.C2);
				return o1;
			}
			else
				return o1;
		}
		else
			return o1;
	}
	else if (a.Tt.Class->MIsRawVec) {
		GatherInfo gi;
		gi.Rez = nullptr;
		
		gi.Count = 0; GatherD(oo, params, pi, gi, n, a);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		//if (conv) {
			gi.Count = 0; GatherR(oo, params, pi, gi, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		//}
	}
	else {
		GatherInfo gi;
		gi.Rez = nullptr;

		if (n.IsTemporary/* || a.IsMove*/) {
			gi.Count = 0; GatherDMove(oo, params, pi, gi, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		if (n.IsAddressable) {
			gi.Count = 0; GatherDRef(oo, params, pi, gi, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
			
			gi.Count = 0; GatherD2(oo, params, pi, gi, n, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
				
		gi.Count = 0; GatherD(oo, params, pi, gi, n, a);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		
		gi.Count = 0; GatherS(oo, params, pi, gi, n, a);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
	}
	
	return nullptr;
}

int aa;

ZFunction* ZFunctionResolver::GatherNumeric(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, ZClass* cls) {
	Node& n = *params[pi];
	ObjectInfo& a = n;

	if (cls == ass.CInt) {
		GatherInfo gi;
		gi.Rez = nullptr;
		
		if (n.IsTemporary) {
			gi.Count = 0; GatherDMove(oo, params, pi, gi, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		if (n.IsAddressable) {
			gi.Count = 0; GatherRef(oo, params, pi, gi, n, a, &ass.CInt->Tt);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CInt->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CLong->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CDouble->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		
		if (n.IsCT && n.IsLiteral && (int)n.IntVal >= 0) {
			gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CPtrSize->Tt);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
	}
	else if (cls == ass.CByte) {
		GatherInfo gi;
		gi.Rez = nullptr;
		
		if (n.IsTemporary) {
			gi.Count = 0; GatherDMove(oo, params, pi, gi, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		if (n.IsAddressable) {
			gi.Count = 0; GatherRef(oo, params, pi, gi, n, a, &ass.CByte->Tt);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1 && ambig) { ambig = true; return nullptr; }
		}
		
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CByte->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }

		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CWord->Tt, &ass.CDWord->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CDWord->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CQWord->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CShort->Tt, &ass.CInt->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CLong->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CFloat->Tt, &ass.CDouble->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
	}
	else if (cls == ass.CWord) {
		GatherInfo gi;
		gi.Rez = nullptr;
		
		if (n.IsTemporary) {
			gi.Count = 0; GatherDMove(oo, params, pi, gi, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		if (n.IsAddressable) {
			gi.Count = 0; GatherRef(oo, params, pi, gi, n, a, &ass.CWord->Tt);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CWord->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CDWord->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CQWord->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CInt->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CLong->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CFloat->Tt, &ass.CDouble->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
	}
	else if (cls == ass.CDWord) {
		GatherInfo gi;
		gi.Rez = nullptr;
		
		if (n.IsTemporary) {
			gi.Count = 0; GatherDMove(oo, params, pi, gi, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		if (n.IsAddressable) {
			gi.Count = 0; GatherRef(oo, params, pi, gi, n, a, &ass.CDWord->Tt);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CDWord->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CQWord->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CLong->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CDouble->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		
		if (n.IsCT && n.IsLiteral && (int)n.IntVal >= 0) {
			gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CPtrSize->Tt);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
	}
	else if (cls == ass.CSmall) {
		GatherInfo gi;
		gi.Rez = nullptr;
		
		if (n.IsTemporary) {
			gi.Count = 0; GatherDMove(oo, params, pi, gi, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		if (n.IsAddressable) {
			gi.Count = 0; GatherRef(oo, params, pi, gi, n, a, &ass.CSmall->Tt);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CSmall->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CShort->Tt, &ass.CInt->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CInt->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CLong->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CFloat->Tt, &ass.CDouble->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
	}
	else if (cls == ass.CShort) {
		GatherInfo gi;
		gi.Rez = nullptr;
		
		if (n.IsTemporary) {
			gi.Count = 0; GatherDMove(oo, params, pi, gi, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		if (n.IsAddressable) {
			gi.Count = 0; GatherRef(oo, params, pi, gi, n, a, &ass.CShort->Tt);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CShort->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CInt->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CLong->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CFloat->Tt, &ass.CDouble->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
	}
	else if (cls == ass.CFloat) {
		GatherInfo gi;
		gi.Rez = nullptr;
		
		if (n.IsTemporary) {
			gi.Count = 0; GatherDMove(oo, params, pi, gi, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		if (n.IsAddressable) {
			gi.Count = 0; GatherRef(oo, params, pi, gi, n, a, &ass.CFloat->Tt);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CFloat->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; Gather(oo, params, pi, gi, n, a, &ass.CDouble->Tt);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
	}
	else {
		GatherInfo gi;
		gi.Rez = nullptr;
		
		if (n.IsTemporary) {
			gi.Count = 0; GatherDMove(oo, params, pi, gi, a);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		if (n.IsAddressable) {
			gi.Count = 0; GatherRef(oo, params, pi, gi, n, a, &a.Tt);
			if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		}
		
		gi.Count = 0; GatherD(oo, params, pi, gi, n, a);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
		gi.Count = 0; GatherS(oo, params, pi, gi, n, a);
		if (gi.Count == 1)	return gi.Rez; else if (gi.Count > 1) { ambig = true; return nullptr; }
	}
	
	return nullptr;
}

void ZFunctionResolver::GatherRef(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node&n, ObjectInfo& a, ObjectType* ot) {
	score++;
	Vector<ZFunction*> temp;
	for (int i = 0; i < oo.GetCount(); i++) {
		ZFunction& over = *oo[i];
		ZVariable& par = over.Params[pi];
		//DUMP(over.PSig);
	
		// match ref only
		// TODO: fix
		/*
		if (par.PType == ZVariable::tyRef && par.I.Tt == ot && a.IsConst == false)
			temp.Add(&over);
		else if (par.PType == ZVariable::tyConstRef && par.I.Tt == ot)
			temp.Add(&over);
		*/
	}
	
	for (int i = 0; i < temp.GetCount(); i++) {
		ZFunction& over = *temp[i];
		
		if (pi < params.GetCount() - 1) {
			ZFunction* ret = GatherParIndex(temp, params, pi + 1);
			
			if (ret && ret != gi.Rez) {
				gi.Rez = ret;
				gi.Count++;
				over.Score = score;
			}
		}
		else {
			gi.Rez = &over;
			gi.Count++;
			over.Score = score;
		}
	}
}

void ZFunctionResolver::Gather(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node&n, ObjectInfo& a, ObjectType* ot) {
	score++;
	Vector<ZFunction*> temp;
	for (int i = 0; i < oo.GetCount(); i++) {
		ZFunction& over = *oo[i];
		ObjectInfo& f = over.Params[pi].I;
		if (conv || (!conv && over.Params[pi].FromTemplate == false)) {
			if (f.IsRef) {
				if (f.IsTemporary == false && n.IsAddressable && f.IsConst == a.IsConst) {
					if (f.Tt == ot)
						temp.Add(&over);
				}
			}
			else if (over.Params[pi].PType == ZVariable::tyAuto || over.Params[pi].PType == ZVariable::tyVal) {
				if (f.Tt == ot)
					temp.Add(&over);
			}
		}
		else {
			if (f.Tt == ot)
				temp.Add(&over);
		}
	}
	
	for (int i = 0; i < temp.GetCount(); i++) {
		ZFunction& over = *temp[i];
		
		if (pi < params.GetCount() - 1) {
			ZFunction* ret = GatherParIndex(temp, params, pi + 1);
			
			if (ret && ret != gi.Rez) {
				gi.Rez = ret;
				gi.Count++;
				over.Score = score;
			}
		}
		else {
			gi.Rez = &over;
			gi.Count++;
			over.Score = score;
		}
	}
}


void ZFunctionResolver::Gather(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node& n, ObjectInfo& a, ObjectType* ot, ObjectType* ot2) {
	score++;
	Vector<ZFunction*> temp;
	for (int i = 0; i < oo.GetCount(); i++) {
		ZFunction& over = *oo[i];
		ObjectInfo& f = over.Params[pi].I;
		if (conv || (!conv && over.Params[pi].FromTemplate == false))
			if (f.IsRef) {
				if (f.IsTemporary == false && n.IsAddressable && f.IsConst == a.IsConst) {
					if (f.Tt == ot || f.Tt == ot2)
						temp.Add(&over);
				}
			}
			else if (over.Params[pi].PType == ZVariable::tyAuto || over.Params[pi].PType == ZVariable::tyVal) {
				if (f.Tt == ot || f.Tt == ot2)
					temp.Add(&over);
			}
	}
	
	for (int i = 0; i < temp.GetCount(); i++) {
		ZFunction& over = *temp[i];
		
		if (pi < params.GetCount() - 1) {
			ZFunction* ret = GatherParIndex(temp, params, pi + 1);
			
			if (ret && ret != gi.Rez) {
				gi.Rez = ret;
				gi.Count++;
				over.Score = score;
			}
		}
		else {
			gi.Rez = &over;
			gi.Count++;
			over.Score = score;
		}
	}
}

void ZFunctionResolver::GatherD2(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node& n, ObjectInfo& a) {
	score++;
	Vector<ZFunction*> temp;
	for (int i = 0; i < oo.GetCount(); i++) {
		ZFunction& over = *oo[i];
		//ObjectInfo& f = over.Params[pi].I;
		
		//if (over.Params[pi].PType == Variable::tyRef || over.Params[pi].PType == Variable::tyConstRef)
		//	continue;
		ZVariable& par = over.Params[pi];
		//DUMP(over.PSig);
	
		// match ref only
		if (par.PType == ZVariable::tyRef && TypesEqualS(ass, &par.I.Tt, &a.Tt) && a.IsConst == false)
			temp.Add(&over);
		else if (par.PType == ZVariable::tyConstRef && TypesEqualS(ass, &par.I.Tt, &a.Tt))
			temp.Add(&over);
		
		/*if (f.IsRef && !f.IsConst && !a.IsConst && !f.IsTemporary) {
			if (n.IsAddressable) {
				if (TypesEqualD(ass, &f.Tt, &a.Tt))
					temp.Add(&over);
			}
		}
		else {
			if (!f.IsTemporary && (f.IsConst == a.IsConst || (f.IsConst && !a.IsConst)) && TypesEqualD(ass, &f.Tt, &a.Tt))
				temp.Add(&over);
		}*/
	}
	
	for (int i = 0; i < temp.GetCount(); i++) {
		ZFunction& over = *temp[i];
		
		if (pi < params.GetCount() - 1) {
			ZFunction* ret = GatherParIndex(temp, params, pi + 1);
			
			if (ret && ret != gi.Rez) {
				gi.Rez = ret;
				gi.Count++;
				over.Score = score;
			}
		}
		else {
			gi.Rez = &over;
			gi.Count++;
			over.Score = score;
		}
	}
}

void ZFunctionResolver::GatherD(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node& n, ObjectInfo& a) {
	score++;
	Vector<ZFunction*> temp;
	for (int i = 0; i < oo.GetCount(); i++) {
		ZFunction& over = *oo[i];
		ObjectInfo& f = over.Params[pi].I;
		
		if (over.Params[pi].PType == ZVariable::tyRef || over.Params[pi].PType == ZVariable::tyConstRef)
			continue;
		
		if (f.IsRef && !f.IsConst && !a.IsConst && !f.IsTemporary) {
			if (n.IsAddressable) {
				if (TypesEqualD(ass, &f.Tt, &a.Tt))
					temp.Add(&over);
			}
		}
		else {
			if (!f.IsTemporary && (f.IsConst == a.IsConst || (f.IsConst && !a.IsConst)) && TypesEqualD(ass, &f.Tt, &a.Tt))
				temp.Add(&over);
		}
	}
	
	for (int i = 0; i < temp.GetCount(); i++) {
		ZFunction& over = *temp[i];
		
		if (pi < params.GetCount() - 1) {
			ZFunction* ret = GatherParIndex(temp, params, pi + 1);
			
			if (ret && ret != gi.Rez) {
				gi.Rez = ret;
				gi.Count++;
				over.Score = score;
			}
		}
		else {
			gi.Rez = &over;
			gi.Count++;
			over.Score = score;
		}
	}
}

void ZFunctionResolver::GatherDRef(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, ObjectInfo& a) {
	score++;
	Vector<ZFunction*> temp;
	for (int i = 0; i < oo.GetCount(); i++) {
		ZFunction& over = *oo[i];
		
		ZVariable& par = over.Params[pi];
		//DUMP(over.PSig);
	
		// match ref only
		if (par.PType == ZVariable::tyRef && TypesEqualD(ass, &par.I.Tt, &a.Tt) && a.IsConst == false)
			temp.Add(&over);
		else if (par.PType == ZVariable::tyConstRef && TypesEqualD(ass, &par.I.Tt, &a.Tt))
			temp.Add(&over);
		//ObjectInfo& f = over.Params[pi].I;
		//if (f.IsRef && !f.IsTemporary && !f.IsConst && !a.IsConst && TypesEqualD(ass, &f.Tt, &a.Tt))
		//	temp.Add(&over);
	}
	
	for (int i = 0; i < temp.GetCount(); i++) {
		ZFunction& over = *temp[i];
		
		if (pi < params.GetCount() - 1) {
			ZFunction* ret = GatherParIndex(temp, params, pi + 1);
			
			if (ret && ret != gi.Rez) {
				gi.Rez = ret;
				gi.Count++;
				over.Score = score;
			}
		}
		else {
			gi.Rez = &over;
			gi.Count++;
			over.Score = score;
		}
	}
}

void ZFunctionResolver::GatherDMove(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, ObjectInfo& a) {
	score++;
	Vector<ZFunction*> temp;
	for (int i = 0; i < oo.GetCount(); i++) {
		ZFunction& over = *oo[i];
		ZVariable& par = over.Params[pi];

		if (par.PType == ZVariable::tyMove && TypesEqualD(ass, &par.I.Tt, &a.Tt))
			temp.Add(&over);
	}
	
	for (int i = 0; i < temp.GetCount(); i++) {
		ZFunction& over = *temp[i];
		
		if (pi < params.GetCount() - 1) {
			ZFunction* ret = GatherParIndex(temp, params, pi + 1);
			
			if (ret && ret != gi.Rez) {
				gi.Rez = ret;
				gi.Count++;
				over.Score = score;
			}
		}
		else {
			gi.Rez = &over;
			gi.Count++;
			over.Score = score;
		}
	}
}

void ZFunctionResolver::GatherR(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, ObjectInfo& a) {
	score++;
	Vector<ZFunction*> temp;
	
	// TODO: fix
	/*
	for (int i = 0; i < oo.GetCount(); i++) {
		ZFunction& over = *oo[i];
		ObjectInfo& f = over.Params[pi].I;
		if (f.Tt.Class->MIsRawVec && f.Tt.Class->T == a.Tt.Class->T && f.Tt.Param == -1)
			temp.Add(&over);
	}*/
	
	for (int i = 0; i < temp.GetCount(); i++) {
		ZFunction& over = *temp[i];
		
		if (pi < params.GetCount() - 1) {
			ZFunction* ret = GatherParIndex(temp, params, pi + 1);
			
			if (ret && ret != gi.Rez) {
				gi.Rez = ret;
				gi.Count++;
				over.Score = score;
			}
		}
		else {
			gi.Rez = &over;
			gi.Count++;
			over.Score = score;
		}
	}
}

void ZFunctionResolver::GatherS(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node& n, ObjectInfo& a) {
	score++;
	Vector<ZFunction*> temp;
	for (int i = 0; i < oo.GetCount(); i++) {
		ZFunction& over = *oo[i];
		ObjectInfo& f = over.Params[pi].I;

		if (over.Params[pi].PType == ZVariable::tyRef || over.Params[pi].PType == ZVariable::tyConstRef)
			continue;
		
		if (f.IsRef && !f.IsConst && !f.IsTemporary) {
			if (n.IsAddressable) {
				if (TypesEqualS(ass, &f.Tt, &a.Tt))
					temp.Add(&over);
			}
		}
		else {
			if (!f.IsTemporary && TypesEqualS(ass, &f.Tt, &a.Tt))
				temp.Add(&over);
		}
	}
	
	for (int i = 0; i < temp.GetCount(); i++) {
		ZFunction& over = *temp[i];
		
		if (pi < params.GetCount() - 1) {
			ZFunction* ret = GatherParIndex(temp, params, pi + 1);
			
			if (ret && ret != gi.Rez) {
				gi.Rez = ret;
				gi.Count++;
				over.Score = score;
			}
		}
		else {
			gi.Rez = &over;
			gi.Count++;
			over.Score = score;
		}
	}
}

bool ZFunctionResolver::TypesEqualD(Assembly& ass, ObjectType* t1, ObjectType* t2) {
	if (t1->Class == ass.CPtr && t2->Class == ass.CNull)
		return true;
	
	while (t1) {
		if (t1->Class != t2->Class || t1->Param != t2->Param)
			return false;
		t1 = t1->Next;
		t2 = t2->Next;
	}
	return true;
}

bool ZFunctionResolver::TypesEqualSD(Assembly& ass, ObjectType* t1, ObjectType* t2) {
	// TODO: fix
	/*
	if (t2->Class->Super.Class != nullptr) {
		ZClass* c = t2->Class;
		t2->Class = t2->Class->Super.Class;
		bool b = TypesEqualSD(ass, t1, t2);
		t2->Class = c;
		return b;
	}
	*/
	while (t1) {
		if (t1->Class != t2->Class || t1->Param != t2->Param)
			return false;
		t1 = t1->Next;
		t2 = t2->Next;
	}
	return true;
}

bool ZFunctionResolver::TypesEqualS(Assembly& ass, ObjectType* t1, ObjectType* t2) {
	return TypesEqualSD(ass, t1, t2);
}