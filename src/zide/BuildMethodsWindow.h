#ifndef _zide_BuildMethodsWindow_h_
#define _zide_BuildMethodsWindow_h_

#include "zide.h"

class BuildMethodsWindow: public WithBuildMethodsLayout<TopWindow> {
public:
	typedef BuildMethodsWindow CLASSNAME;
	
	BuildMethodsWindow(Vector<BuildMethod>& meth): methods(meth) {
		CtrlLayout(*this, "Build methods");
		Sizeable();
		
		lstMethods.NoRoundSize();
		lstMethods.WhenAction = THISBACK(OnBuildMethodSelect);
		
		for (int i = 0; i < methods.GetCount(); i++) {
			BuildMethod& bm = methods[i];
			lstMethods.Add(bm.Name + " " + bm.Arch);
		}
				
		lstLib.SetReadOnly();
		
		btnOk.Ok();
		Acceptor(btnOk, IDOK);
	}
	
	void OnBuildMethodSelect() {
		int index = lstMethods.GetCursor();
		if (index == -1)
			return;
		
		BuildMethod& bm = methods[index];
		
		edtName.SetText(bm.Name);
		
		if (bm.Type == BuildMethod::btGCC) {
			optGCC.Set(1);
			optMSC.Set(0);
		}
		else if (bm.Type == BuildMethod::btMSC) {
			optGCC.Set(0);
			optMSC.Set(1);
		}
		else {
			optGCC.Set(0);
			optMSC.Set(0);
		}
		
		edtCompiler.SetText(bm.Compiler);
		edtSdk.SetText(bm.Sdk);

		lstLib.Clear();
		for (int i = 0; i < bm.Lib.GetCount(); i++)
			lstLib.Add(bm.Lib[i]);
	}
	
private:
	Vector<BuildMethod>& methods;
};

#endif
