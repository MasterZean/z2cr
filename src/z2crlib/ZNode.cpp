#include "z2cr.h"

bool Node::IsZero(Assembly& ass) {
	if (IsCT == false)
		return false;
	if (ass.IsInteger(Tt) || Tt.Class == ass.CBool || Tt.Class == ass.CChar) {
		if (IntVal == 0)
			return true;
	}
	else if (Tt.Class == ass.CFloat || Tt.Class == ass.CDouble) {
		if (DblVal == 0)
			return true;
	}
	else
		ASSERT_(0, "is zero");

	return false;
}

void Node::PromoteToFloatValue(Assembly& ass) {
	if (Tt.Class == ass.CQWord)
		DblVal = (double)(uint32)IntVal;
	else if (ass.IsInteger(Tt) || Tt.Class == ass.CBool || Tt.Class == ass.CChar)
		DblVal = (double)IntVal;
}