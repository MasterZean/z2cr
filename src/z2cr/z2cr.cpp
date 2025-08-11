#include <Core/Core.h>

using namespace Upp;

#include <z2crlib/StopWatch.h>
#include <z2crlib/ZCompiler.h>
#include <z2cr/CommandLine.h>
#include <z2crlib/BuildMethod.h>
#include <z2cr/Builder.h>
#include <z2cr/InlineTester.h>
#include <z2crlib/ErrorReporter.h>
#include <z2crlib/LLVMIR.h>

bool GetBuildMethod(const String& exeDir, const ::CommandLine& K, BuildMethod& bm) {
	Vector<BuildMethod> methods;
	
	// load existing BMs
	LoadFromXMLFile(methods, exeDir + "buildMethods.xml");
	if (methods.GetCount() == 0) {
		Cout() << "No cached build method found! Trying to auto-detect...\n";
		Cout().Flush();
		BuildMethod::Get(methods);
		if (methods.GetCount() == 0) {
			Cout() << ZCompiler::GetName() << " couldn't find a backend compiler. Skipping compilation step..." << '\n';
			Cout().Flush();
			//K.SCU = false;
		}
		else
			StoreAsXMLFile(methods, "methods", exeDir + "buildMethods.xml");
	}
	
	if (K.BM == false && K.ARCH == false && methods.GetCount() > 0) {
		bm = methods[0];
	
		return true;
	}
	
	int bmi = -1;
	
	// find input BM
	for (int i = 0; i < methods.GetCount(); i++) {
		bool found = true;
	
		if (K.BM && ToUpper(K.BMName) != ToUpper(methods[i].Name)) {
			found = false;
		}
		
		if (K.ARCH && ToUpper(K.ARCHName) != ToUpper(methods[i].Arch)) {
			found = false;
		}
		
		if (found) {
			bmi = i;
			break;
		}
	}
	
	if (bmi == -1) {
		String bms = K.BMName;
		if (bms.GetCount())
			bms << " ";
		bms << K.ARCHName;
		
		if (bms.GetCount())
			Cout() << "Build method '" << ToUpper(bms) << "' can't be found. Exiting!" << '\n';
		else
			Cout() << "No build method can be found. Exiting!" << '\n';
		Cout().Flush();
		
		return false;
	}
	
	bm = methods[bmi];
	
	return true;
}

void RunInlineTests(const String& path) {
	Cout() << "Searching for tests...\n";
	
	InlineTester tester;
	tester.AddTestFolder(path);
	
	if (tester.TestCount == 0) {
		Cout() << "Could not find any tests. Aborting testing!\n\n";
		return;
	}
	
	Cout() << "Testing status: " << tester.PassCount << "/" << tester.TestCount << " passed.\n\n";
}

int separate_console() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (!GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE), &csbi))
    {
        //printf( "GetConsoleScreenBufferInfo failed: %lu\n", GetLastError());
        return 0;
    }

    // if cursor position is (0,0) then we were launched in a separate console
    return ((!csbi.dwCursorPosition.X) && (!csbi.dwCursorPosition.Y));
}

#define ESC "\x1b"
#define CSI "\x1b["

bool EnableVTMode()
{
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        return false;
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        return false;
    }
    return true;
}

void PrintVerticalBorder()
{
    printf(ESC "(0"); // Enter Line drawing mode
    printf(CSI "104;93m"); // bright yellow on bright blue
    printf("x"); // in line drawing mode, \x78 -> \u2502 "Vertical Bar"
    printf(CSI "0m"); // restore color
    printf(ESC "(B"); // exit line drawing mode
}

void PrintHorizontalBorder(COORD const Size, bool fIsTop)
{
    printf(ESC "(0"); // Enter Line drawing mode
    printf(CSI "104;93m"); // Make the border bright yellow on bright blue
    printf(fIsTop ? "l" : "m"); // print left corner

    for (int i = 1; i < Size.X - 1; i++)
        printf("q"); // in line drawing mode, \x71 -> \u2500 "HORIZONTAL SCAN LINE-5"

    printf(fIsTop ? "k" : "j"); // print right corner
    printf(CSI "0m");
    printf(ESC "(B"); // exit line drawing mode
}

void PrintStatusLine(const char* const pszMessage, COORD const Size)
{
    printf(CSI "%d;1H", Size.Y);
    printf(CSI "K"); // clear the line
    printf(pszMessage);
}

int testansi()
{
    //First, enable VT mode
    bool fSuccess = EnableVTMode();
    if (!fSuccess)
    {
        printf("Unable to enter VT processing mode. Quitting.\n");
        return -1;
    }
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        printf("Couldn't get the console handle. Quitting.\n");
        return -1;
    }

    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
    GetConsoleScreenBufferInfo(hOut, &ScreenBufferInfo);
    COORD Size;
    Size.X = ScreenBufferInfo.srWindow.Right - ScreenBufferInfo.srWindow.Left + 1;
    Size.Y = ScreenBufferInfo.srWindow.Bottom - ScreenBufferInfo.srWindow.Top + 1;

    // Enter the alternate buffer
    printf(CSI "?1049h");

    // Clear screen, tab stops, set, stop at columns 16, 32
    printf(CSI "1;1H");
    printf(CSI "2J"); // Clear screen

    int iNumTabStops = 4; // (0, 20, 40, width)
    printf(CSI "3g"); // clear all tab stops
    printf(CSI "1;20H"); // Move to column 20
    printf(ESC "H"); // set a tab stop

    printf(CSI "1;40H"); // Move to column 40
    printf(ESC "H"); // set a tab stop

    // Set scrolling margins to 3, h-2
    printf(CSI "3;%dr", Size.Y - 2);
    int iNumLines = Size.Y - 4;

    printf(CSI "1;1H");
    printf(CSI "102;30m");
    printf("Windows 10 Anniversary Update - VT Example");
    printf(CSI "0m");

    // Print a top border - Yellow
    printf(CSI "2;1H");
    PrintHorizontalBorder(Size, true);

    // // Print a bottom border
    printf(CSI "%d;1H", Size.Y - 1);
    PrintHorizontalBorder(Size, false);

    wchar_t wch;

    // draw columns
    printf(CSI "3;1H");
    int line = 0;
    for (line = 0; line < iNumLines * iNumTabStops; line++)
    {
        PrintVerticalBorder();
        if (line + 1 != iNumLines * iNumTabStops) // don't advance to next line if this is the last line
            printf("\t"); // advance to next tab stop

    }

    PrintStatusLine("Press any key to see text printed between tab stops.", Size);
    wch = _getwch();

    // Fill columns with output
    printf(CSI "3;1H");
    for (line = 0; line < iNumLines; line++)
    {
        int tab = 0;
        for (tab = 0; tab < iNumTabStops - 1; tab++)
        {
            PrintVerticalBorder();
            printf("line=%d", line);
            printf("\t"); // advance to next tab stop
        }
        PrintVerticalBorder();// print border at right side
        if (line + 1 != iNumLines)
            printf("\t"); // advance to next tab stop, (on the next line)
    }

    PrintStatusLine("Press any key to demonstrate scroll margins", Size);
    wch = _getwch();

    printf(CSI "3;1H");
    for (line = 0; line < iNumLines * 2; line++)
    {
        printf(CSI "K"); // clear the line
        int tab = 0;
        for (tab = 0; tab < iNumTabStops - 1; tab++)
        {
            PrintVerticalBorder();
            printf("line=%d", line);
            printf("\t"); // advance to next tab stop
        }
        PrintVerticalBorder(); // print border at right side
        if (line + 1 != iNumLines * 2)
        {
            printf("\n"); //Advance to next line. If we're at the bottom of the margins, the text will scroll.
            printf("\r"); //return to first col in buffer
        }
    }

    PrintStatusLine("Press any key to exit", Size);
    wch = _getwch();

    // Exit the alternate buffer
    printf(CSI "?1049l");
    
    return 0;
}

CONSOLE_APP_MAIN {
	bool noConsole = separate_console();
	
	//BuildMethod::NewVs();
	
	String curDir = NativePath(GetCurrentDirectory() + "/");
	String exeDir = GetFileDirectory(GetExeFilePath());
	
	::CommandLine K;
	if (!K.Read()) {
		SetExitCode(-1);
		return;
	}
	
	if (K.UT) {
		RunInlineTests(AppendFileName(curDir, "tests"));
		
		if (K.Files.GetCount() == 0) {
			return;
		}
	}
		
	if (!K.SCU && !K.CPP) {
		Cout() << ZCompiler::GetName() << " requires the '-scu' or '-c++' parameters. Exiting!" << '\n';
		SetExitCode(-1);
		return;
	}
	
	if (K.PP_NOPATH)
		ER::PrintPath = false;
	else
		ER::PrintPath = true;
	
	ER::ErrorColor = K.ErrorColor;
	if (ER::ErrorColor == ErrorColorType::Ansi) {
		bool result = EnableVTMode();
		LOG(result);
	}
	
	if (K.NoConsoleTest)
		noConsole = false;
	
	if (K.Files.GetCount() == 0 && K.UT == false) {
		Cout() << ZCompiler::GetName() << " requires a list of input packages or files. Exiting!" << '\n';
		SetExitCode(-1);
		
		if (noConsole) {
			Cout() << "Press any key to continue...";
			ReadStdIn();
		}
		
		return;
	}
	
	BuildMethod bm;
	if (!GetBuildMethod(exeDir, K, bm)) {
		SetExitCode(-1);
		return;
	}
	
	StopWatch tm;
	
	Assembly ass;
	
	try {
		String prjPath;
		if (IsFullPath(K.EntryFile))
			prjPath = GetFileFolder(K.EntryFile);
		else
			prjPath = GetFileFolder(curDir + K.EntryFile);
		
		String prjPackage = GetFileName(prjPath);
		String prjCP = GetFileDirectory(prjPath);
		
		for (int i = 0; i < K.Packages.GetCount(); i++) {
			if (!ass.LoadPackage(K.Packages[i])) {
				SetExitCode(BuildMethod::ErrorCode(-1));
				return;
			}
		}
		
		String stdLibPath = exeDir + NativePath("source\\stdlib\\");
		
		if (!ass.LoadPackage(stdLibPath + "sys.core")) {
			SetExitCode(BuildMethod::ErrorCode(-1));
			return;
		}
		
		if (K.Files.GetCount()) {
			ZPackage& mainPak = ass.AddPackage("main", "");
			for (int i = 0; i < K.Files.GetCount(); i++)
				ZSource& source = mainPak.AddSource(K.Files[i], true);
		}
			
		ZCompiler compiler(ass);
		compiler.BuildMode = true;
		compiler.CppVersion = bm.CppVersion;
		
		if (IsFullPath(K.OutPath))
			compiler.OutPath = K.OutPath;
		else
			compiler.OutPath = curDir + K.OutPath;
		
		compiler.BuildProfile = compiler.PlatformString + ToUpper(bm.Arch) + "." + ToUpper(bm.Name) + K.O;
		compiler.BuildPath = exeDir + NativePath("builds/") + compiler.PlatformString + "." + ToUpper(bm.Arch) + "." + ToUpper(bm.Name);
		RealizeDirectory(compiler.BuildPath);
			
		compiler.SetMain(K.EntryClass, K.EntryFile);
		
		if (!compiler.Compile()) {
			if (compiler.MainFound == false) {
				if (K.EntryFile.GetCount())
					Cout() << "No main entry point found. Exiting.\n";
				else
					Cout() << "No main entry point found. Please provide one using the '-main' or '-mainfile' parameters. Exiting.\n";
			}
			else
				Cout() << "Compilation failed. Exiting.\n";
			SetExitCode(-1);
			return;
		}

		if (!compiler.Transpile()) {
			Cout() << "CPP code generation failed. Exiting.\n";
			SetExitCode(-1);
			return;
		}
		
		Builder builder(bm);
		builder.ZCompilerPath(exeDir);
		builder.TargetRoot(compiler.BuildPath);
		builder.Arch(bm.Arch);
		builder.Optimize(K.O);
		builder.Subsystem(K.SUBSYSTEM);
		
		if (compiler.MainFound == false) {
			if (compiler.MainFound == false) {
				if (K.EntryFile.GetCount())
					Cout() << "No main entry point found. Exiting.\n";
				else
					Cout() << "No main entry point found. Please provide one using the '-main' or '-mainfile' parameters. Exiting.\n";
			}
			SetExitCode(-1);
			return;
		}
		
		Cout() << "Using '" << ToUpper(bm.Name) << "." << ToUpper(bm.Arch) << "' back end compiler.\n\n";
		
		bool buildOk = builder.Build(compiler.CppPath, compiler.OutPath, ass.LibLink);
		if (buildOk) {
			Cout() << "\n";
			Cout() << bm.Name << " code generation finished in " << tm.ToString() << " seconds.\n";
		}
		else {
			SetExitCode(-1);
			Cout() << "\n";
			Cout() << bm.Name << " code generation failed.\n";
		}
		
		for (int i = 0; i < compiler.MCUPaths.GetCount(); i++) {
			String p = compiler.MCUPaths[i];
			Cout() << "Building: " << p << ": ";
			
			buildOk = builder.Compile(p, GetFileFolder(p) + "/" + GetFileTitle(p) + ".o", compiler.BuildPath);
			//Cout() << "\n";
		}
		
		Cout() << "\n";
		
#if defined(_RELEASE) && defined(flagLLVM) && defined(_MSC_VER)
		int main2();
		
		main2();
		Cout() << "\n";
			
		LLVMIR llvm;
		
		for (int i = 0; i < compiler.LLVMInput.GetCount(); i++) {
			llvm.AddFunc(*compiler.LLVMInput[i]);
		}
		
		llvm.Print();
#endif
	}
	catch (ZException& e) {
		e.PrettyPrint(Cout());
		Cout() << "\n";
		SetExitCode(-1);
	}
	catch (CParser::Error& e) {
		// should not reach, but...
		Cout() << e << "\n";
		SetExitCode(-1);
	}
}
