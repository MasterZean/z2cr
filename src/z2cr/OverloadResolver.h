#ifndef _z2clib_ZFunctionResolver_h_
#define _z2clib_ZFunctionResolver_h_

#include "Assembly.h"

class GatherInfo {
public:
	ZFunction* Rez = nullptr;
	int Count = 0;
};

class ZFunctionResolver {
public:
	ZFunctionResolver(Assembly& a): ass(a) {
	}
	
	ZFunction* Resolve(ZMethodBundle& def, Vector<Node*>& params, int limit, ZClass* spec = nullptr, bool conv = true);
	
	ZFunction* GatherParIndex(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi);
	void GatherRef(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node& n, ObjectInfo& a, ObjectType* ot);
	void Gather(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node& n, ObjectInfo& a, ObjectType* ot);
	void Gather(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node& n, ObjectInfo& a, ObjectType* ot, ObjectType* ot2);
	void GatherD(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node& n, ObjectInfo& a);
	void GatherD2(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node& n, ObjectInfo& a);
	void GatherDRef(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, ObjectInfo& a);
	void GatherDConst(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, ObjectInfo& a);
	void GatherDMove(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, ObjectInfo& a);
	void GatherS(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, Node& n, ObjectInfo& a);
	void GatherR(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, GatherInfo& gi, ObjectInfo& a);
	ZFunction* GatherNumeric(Vector<ZFunction*>& oo, Vector<Node*>& params, int pi, ZClass* cls);

	static bool TypesEqualD(Assembly& ass, ObjectType* t1, ObjectType* t2);
	static bool TypesEqualS(Assembly& ass, ObjectType* t1, ObjectType* t2);
	static bool TypesEqualSD(Assembly& ass, ObjectType* t1, ObjectType* t2);
	
	bool IsAmbig() const {
		return ambig;
	}
	
	int Score() const {
		return score;
	}

private:
	Assembly& ass;
	int score = 0;
	bool ambig = false;
	bool conv = false;
};

#endif
