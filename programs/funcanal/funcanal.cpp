// funcanal.cpp : Defines the entry point for the console application.
//
// $Log: funcanal.cpp,v $
// Revision 1.4  2003/02/09 06:59:55  dewhisna
// Split comparison logic from Function File I/O
//
// Revision 1.3  2003/01/26 06:33:12  dewhisna
// Improved usage help
//
// Revision 1.2  2002/09/15 22:54:01  dewhisna
// Added Symbol Table comparison support
//
// Revision 1.1  2002/07/01 05:49:59  dewhisna
// Initial Revision
//
//

#include "stdafx.h"
#include "funcanal.h"
#include "funcdesc.h"
#include <ReadVer.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void main(int argc, char* argv[])
{
	CProjectRCVersion aVerRC(::GetModuleHandle(NULL));
	CString strTemp;
	int nPos;
	WORD nVerMajor, nVerMinor;
	WORD nVerRev, nVerBuild;
	CStdioFile	FuncFile;
	CStdioFile	MatrixInFile;
	CStdioFile	MatrixOutFile;
	CStdioFile	DFROFile;
	CStdioFile	CompFile;
	CStdioFile	OESFile;
	CStdioFile	SymFile;
	CStdioFile	_StdOut(stdout);		// Make stdout into a CFile object for passing to parsers
	BOOL bNeedUsage;
	BOOL bOkFlag;
	CFuncDescFile *pFuncDescFileObj;
	int nReqArgs;
	int nMinReqInputFiles;
	int nArgPos;
	int i,j,k;
	int nTemp1, nTemp2;
	int cTemp;
	double **ppnCompResult;
	double nMaxCompResult;
	double nMinCompLimit;
	FUNC_COMPARE_METHOD nCompMethod;
	CFuncDescFile *pFuncFile1;
	CFuncDescFile *pFuncFile2;
	BOOL bFlag;
	CStringArray oes;
	BOOL bCompOESFlag;

	CSymbolMap aSymbolMap;

	CFuncDescFileArray m_FuncFiles;
	CString m_strMatrixInFilename;
	CString m_strMatrixOutFilename;
	CString m_strDFROFilename;
	CString m_strCompFilename;
	CString m_strOESFilename;
	CString m_strSymFilename;
	BOOL m_bForceOverwrite;

	if (aVerRC.GetInfo(VINFO_PRODUCTVERSION, strTemp)) {
		nVerRev = 0;
		nVerBuild = 0;

		nPos = strTemp.Find(",");
		if (nPos != -1) {
			nVerMajor = (WORD)strtoul(strTemp.Left(nPos), NULL, 0);
			strTemp = strTemp.Mid(nPos+1);
			nPos = strTemp.Find(",");
			if (nPos != - 1) {
				nVerMinor = (WORD)strtoul(strTemp.Left(nPos), NULL, 0);
				strTemp = strTemp.Mid(nPos+1);
				nPos = strTemp.Find(",");
				if (nPos != -1) {
					nVerRev = (WORD)strtoul(strTemp.Left(nPos), NULL, 0);
					nVerBuild = (WORD)strtoul(strTemp.Mid(nPos+1), NULL, 0);
				} else {
					nVerRev = (WORD)strtoul(strTemp, NULL, 0);
				}
			} else {
				nVerMinor = (WORD)strtoul(strTemp, NULL, 0);
			}
		} else {
			nVerMajor = (WORD)strtoul(strTemp, NULL, 0);
			nVerMinor = 0;
		}

		printf("Disassembler Function Analyzer V%d.%d", nVerMajor, nVerMinor);
		if (nVerRev) {
			printf(".%d", nVerRev);
		}
		printf(" (build %d)\n", nVerBuild);
	} else {
		printf("Disassembler Function Analyzer\n");
	}
	printf("Copyright(c)2002 by Donald Whisnant\n");
	printf("\n");

	// Setup Defaults:
	m_strMatrixOutFilename = "";
	m_strDFROFilename = "";
	m_strCompFilename = "";
	m_strOESFilename = "";
	m_strSymFilename = "";
	m_bForceOverwrite = FALSE;

	bNeedUsage = FALSE;

	nReqArgs = 1;			// Must have at least program name
	nMinReqInputFiles = 1;	//		and one input filename (determine others below)
	nArgPos = 1;			// Start after the program name

	nMinCompLimit = 0;
	nCompMethod = FCM_DYNPROG_XDROP;

	bCompOESFlag = FALSE;

	while ((nArgPos < argc) && (!bNeedUsage)) {
		if ((argv[nArgPos][0] == '-') || (argv[nArgPos][0] == '/')) {
			switch (argv[nArgPos][1]) {
				case 'm':
					switch (argv[nArgPos][2]) {
						case 'i':			// Matrix Input File
							if (!m_strMatrixInFilename.IsEmpty()) {
								bNeedUsage = TRUE;
								continue;
							}
							if (strlen(argv[nArgPos]) > 3) {
								nReqArgs += 1;
								m_strMatrixInFilename = argv[nArgPos] + 3;
							} else {
								nReqArgs += 2;
								nArgPos++;
								if (nArgPos < argc) {
									m_strMatrixInFilename = argv[nArgPos];
								} else {
									bNeedUsage = TRUE;
								}
							}
							break;

						case 'o':			// Matrix Output File
							if (!m_strMatrixOutFilename.IsEmpty()) {
								bNeedUsage = TRUE;
								continue;
							}
							if (strlen(argv[nArgPos]) > 3) {
								nReqArgs += 1;
								m_strMatrixOutFilename = argv[nArgPos] + 3;
							} else {
								nReqArgs += 2;
								nArgPos++;
								if (nArgPos < argc) {
									m_strMatrixOutFilename = argv[nArgPos];
								} else {
									bNeedUsage = TRUE;
								}
							}
							nMinReqInputFiles = 2;		// Must have 2 input files
							break;

						default:
							bNeedUsage = TRUE;
							break;
					}
					break;

				case 'd':
					if (!m_strDFROFilename.IsEmpty()) {
						bNeedUsage = TRUE;
						continue;
					}
					if (strlen(argv[nArgPos]) > 2) {
						nReqArgs += 1;
						m_strDFROFilename = argv[nArgPos] + 2;
					} else {
						nReqArgs += 2;
						nArgPos++;
						if (nArgPos < argc) {
							m_strDFROFilename = argv[nArgPos];
						} else {
							bNeedUsage = TRUE;
						}
					}
					break;

				case 'c':
					switch (argv[nArgPos][2]) {
						case 'n':			// Normal Diff
						case 'e':			// Normal Diff with OES inline
							if (!m_strCompFilename.IsEmpty()) {
								bNeedUsage = TRUE;
								continue;
							}
							if (argv[nArgPos][2] == 'e') bCompOESFlag = TRUE;
							if (strlen(argv[nArgPos]) > 3) {
								nReqArgs += 1;
								m_strCompFilename = argv[nArgPos] + 3;
							} else {
								nReqArgs += 2;
								nArgPos++;
								if (nArgPos < argc) {
									m_strCompFilename = argv[nArgPos];
								} else {
									bNeedUsage = TRUE;
								}
							}
							nMinReqInputFiles = 2;		// Must have 2 input files
							break;

						default:
							bNeedUsage = TRUE;
							break;
					}
					break;

				case 'e':
					if (!m_strOESFilename.IsEmpty()) {
						bNeedUsage = TRUE;
						continue;
					}
					if (strlen(argv[nArgPos]) > 2) {
						nReqArgs += 1;
						m_strOESFilename = argv[nArgPos] + 2;
					} else {
						nReqArgs += 2;
						nArgPos++;
						if (nArgPos < argc) {
							m_strOESFilename = argv[nArgPos];
						} else {
							bNeedUsage = TRUE;
						}
					}
					nMinReqInputFiles = 2;		// Must have 2 input files
					break;

				case 's':
					if (!m_strSymFilename.IsEmpty()) {
						bNeedUsage = TRUE;
						continue;
					}
					if (strlen(argv[nArgPos]) > 2) {
						nReqArgs += 1;
						m_strSymFilename = argv[nArgPos] + 2;
					} else {
						nReqArgs += 2;
						nArgPos++;
						if (nArgPos < argc) {
							m_strSymFilename = argv[nArgPos];
						} else {
							bNeedUsage = TRUE;
						}
					}
					nMinReqInputFiles = 2;		// Must have 2 input files
					break;

				case 'f':
					m_bForceOverwrite = TRUE;
					nReqArgs++;
					break;

				case 'l':
					if (strlen(argv[nArgPos]) > 2) {
						nReqArgs += 1;
						nMinCompLimit = atof(argv[nArgPos] + 2)/100;
					} else {
						nReqArgs += 2;
						nArgPos++;
						if (nArgPos < argc) {
							nMinCompLimit = atof(argv[nArgPos])/100;
						} else {
							bNeedUsage = TRUE;
						}
					}
					break;

				case 'a':
					if (strlen(argv[nArgPos]) > 2) {
						nReqArgs += 1;
						nCompMethod = (FUNC_COMPARE_METHOD)strtoul(argv[nArgPos] + 2, NULL, 10);
					} else {
						nReqArgs += 2;
						nArgPos++;
						if (nArgPos < argc) {
							nCompMethod = (FUNC_COMPARE_METHOD)strtoul(argv[nArgPos], NULL, 10);
						} else {
							bNeedUsage = TRUE;
						}
					}
					switch (nCompMethod) {
						case FCM_DYNPROG_XDROP:
							printf("Using Comparison Algorithm: DynProg X-Drop\n\n");
							break;
						case FCM_DYNPROG_GREEDY:
							printf("Using Comparison Algorithm: DynProg Greedy\n\n");
							break;
					}
					break;

				default:
					bNeedUsage = TRUE;
					break;
			}
		} else {
			// If we get here and don't have another option, assume rest is filenames
			//		(nArgPos will be pointing to the first filename)
			break;
		}
		nArgPos++;
	}

	nReqArgs += nMinReqInputFiles;

	if ((!m_strMatrixInFilename.IsEmpty()) &&
		(!m_strMatrixOutFilename.IsEmpty())) bNeedUsage = TRUE;

	if ((m_strMatrixOutFilename.IsEmpty()) &&
		(m_strDFROFilename.IsEmpty()) &&
		(m_strCompFilename.IsEmpty()) &&
		(m_strOESFilename.IsEmpty()) &&
		(m_strSymFilename.IsEmpty())) {
		fprintf(stderr, "\n\nNothing to do...\n\n");
		bNeedUsage = TRUE;
	}

	bNeedUsage = bNeedUsage || (argc<nReqArgs);

	if (bNeedUsage) {
		printf("Usage:\n");
		printf("FuncAnal [-a <alg>] [-f] [-e <oes-fn>] [-s <sym-fn>] [-mi <mtx-fn> | -mo <mtx-fn>] [-d <dro-fn>] [-cn <cmp-fn> | -ce <cmp-fn>] [-l <limit>] <func-fn1> [<func-fn2>]\n");
		printf("\n");
		printf("Where:\n\n");
		printf("    <oes-fn>   = Output Optimal Edit Script Filename to generate\n\n");
		printf("    <mtx-fn>   = Input or Output Filename of a CSV file to read or to generate\n");
		printf("                 that denotes percentage of function cross-similarity.\n\n");
		printf("    <dro-fn>   = Output Filename of a file to generate that contains the\n");
		printf("                 diff-ready version all of the functions from the input file(s)\n\n");
		printf("    <cmp-fn>   = Output Filename of a file to generate that contains the full\n");
		printf("                 cross-functional comparisons.\n\n");
		printf("    <func-fn1> = Input Filename of the primary functions-definition-file.\n\n");
		printf("    <func-fn2> = Input Filename of the secondary functions-definition-file.\n");
		printf("                 (Optional only if not using -mo, -cX, -e, or -s)\n\n");
		printf("    <alg>      = Comparison algorithm to use.\n\n");
		printf("    <limit>    = Lower-Match Limit.\n\n");
		printf("\n");
		printf("At least one of the following switches must be used:\n");
		printf("    -mo <mtx-fn> Perform cross comparison of files and output a matrix of\n");
		printf("                 percent match (requires 2 input files). Cannot be used\n");
		printf("                 with the -mi switch.\n\n");
		printf("    -d <dro-fn>  Dump the functions definition file(s) in Diff-Ready notation\n\n");
		printf("    -cn <cmp-fn> Perform cross comparison of files and output a side-by-side\n");
		printf("                 diff of most similar functions (Normal Output)\n\n");
		printf("    -ce <cmp-fn> Perform cross comparison of files and output a side-by-side\n");
		printf("                 diff of most similar functions (Include inline OES)\n\n");
		printf("    -e <oes-fn>  Perform cross comparison of files and output an optimal edit\n");
		printf("                 script file that details the most optimal way of editing the\n");
		printf("                 left-most file to get the right-most file\n\n");
		printf("    -s <sym-fn>  Perform cross comparison of files and output a cross-map\n");
		printf("                 probability based symbol table\n\n");
		printf("\n");
		printf("The following switches can be specified but are optional:\n");
		printf("    -mi <mtx-fn> Reads the specified matrix file to get function cross\n");
		printf("                 comparison information rather than recalculating it.\n");
		printf("                 Cannot be used with the -mo switch.\n\n");
		printf("    -f           Force output file overwrite without prompting\n\n");
		printf("    -l <limit>   Minimum-Match Limit.  This option is only useful with the -cX,\n");
		printf("                 -e, and -s options and limits output to functions having a\n");
		printf("                 match percentage greater than or equal to this value.  If not\n");
		printf("                 specified, the minimum required match is anything greater than\n");
		printf("                 0%%. This value should be specified as a percentage or\n");
		printf("                 fraction of a percent.  For example: -l 50 matches anything\n");
		printf("                 50%% or higher.  -l 23.7 matches anything 23.7%% or higher.\n\n");
		printf("    -a <alg>     Select a specific comparison algorithm to use.  Where <alg> is\n");
		printf("                 one of the following:\n");
		printf("                       0 = Dynamic Programming X-Drop Algorithm\n");
		printf("                       1 = Dynamic Programming Greedy Algorithm\n");
		printf("                 If not specified, the X-Drop algorithm will be used.\n\n");
		printf("\n");
		_StdOut.m_pStream = NULL;	// prevent closing of stdout
		return;
	}

	if (!m_strMatrixOutFilename.IsEmpty()) {
		if ((!m_bForceOverwrite) && (access(m_strMatrixOutFilename, 0) == 0)) {
			fprintf(stderr, "\nFile \"%s\" exists! -- Overwrite? (y/n): ", LPCTSTR(m_strMatrixOutFilename));
#ifdef LINUX
			fflush(stdout);
			cTemp = toupper(getchar());
#else
			cTemp = toupper(getche());
#endif
			fprintf(stderr, "\n\n");
			if (cTemp != 'Y') {
				_StdOut.m_pStream = NULL;	// prevent closing of stdout
				return;
			}
		}
		if ((access(m_strMatrixOutFilename, 0) == 0) && (access(m_strMatrixOutFilename, 2) != 0)) {
			fprintf(stderr, "*** Error: Opening \"%s\" for writing...\n\n", LPCTSTR(m_strMatrixOutFilename));
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
	}

	if (!m_strDFROFilename.IsEmpty()) {
		if ((!m_bForceOverwrite) && (access(m_strDFROFilename, 0) == 0)) {
			fprintf(stderr, "\nFile \"%s\" exists! -- Overwrite? (y/n): ", LPCTSTR(m_strDFROFilename));
#ifdef LINUX
			fflush(stdout);
			cTemp = toupper(getchar());
#else
			cTemp = toupper(getche());
#endif
			fprintf(stderr, "\n\n");
			if (cTemp != 'Y') {
				_StdOut.m_pStream = NULL;	// prevent closing of stdout
				return;
			}
		}
		if ((access(m_strDFROFilename, 0) == 0) && (access(m_strDFROFilename, 2)!=0)) {
			fprintf(stderr, "*** Error: Opening \"%s\" for writing...\n\n", LPCTSTR(m_strDFROFilename));
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
	}

	if (!m_strCompFilename.IsEmpty()) {
		if ((!m_bForceOverwrite) && (access(m_strCompFilename, 0) == 0)) {
			fprintf(stderr, "\nFile \"%s\" exists! -- Overwrite? (y/n): ", LPCTSTR(m_strCompFilename));
#ifdef LINUX
			fflush(stdout);
			cTemp = toupper(getchar());
#else
			cTemp = toupper(getche());
#endif
			fprintf(stderr, "\n\n");
			if (cTemp != 'Y') {
				_StdOut.m_pStream = NULL;	// prevent closing of stdout
				return;
			}
		}
		if ((access(m_strCompFilename, 0) == 0) && (access(m_strCompFilename, 2)!=0)) {
			fprintf(stderr, "*** Error: Opening \"%s\" for writing...\n\n", LPCTSTR(m_strCompFilename));
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
	}

	if (!m_strOESFilename.IsEmpty()) {
		if ((!m_bForceOverwrite) && (access(m_strOESFilename, 0) == 0)) {
			fprintf(stderr, "\nFile \"%s\" exists! -- Overwrite? (y/n): ", LPCTSTR(m_strOESFilename));
#ifdef LINUX
			fflush(stdout);
			cTemp = toupper(getchar());
#else
			cTemp = toupper(getche());
#endif
			fprintf(stderr, "\n\n");
			if (cTemp != 'Y') {
				_StdOut.m_pStream = NULL;	// prevent closing of stdout
				return;
			}
		}
		if ((access(m_strOESFilename, 0) == 0) && (access(m_strOESFilename, 2)!=0)) {
			fprintf(stderr, "*** Error: Opening \"%s\" for writing...\n\n", LPCTSTR(m_strOESFilename));
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
	}

	if (!m_strSymFilename.IsEmpty()) {
		if ((!m_bForceOverwrite) && (access(m_strSymFilename, 0) == 0)) {
			fprintf(stderr, "\nFile \"%s\" exists! -- Overwrite? (y/n): ", LPCTSTR(m_strSymFilename));
#ifdef LINUX
			fflush(stdout);
			cTemp = toupper(getchar());
#else
			cTemp = toupper(getche());
#endif
			fprintf(stderr, "\n\n");
			if (cTemp != 'Y') {
				_StdOut.m_pStream = NULL;	// prevent closing of stdout
				return;
			}
		}
		if ((access(m_strSymFilename, 0) == 0) && (access(m_strSymFilename, 2)!=0)) {
			fprintf(stderr, "*** Error: Opening \"%s\" for writing...\n\n", LPCTSTR(m_strSymFilename));
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
	}

	if (!m_strMatrixOutFilename.IsEmpty()) {
		if (!MatrixOutFile.Open(m_strMatrixOutFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareExclusive)) {
			fprintf(stderr, "*** Error: Opening Matrix Output File \"%s\" for writing...\n", LPCTSTR(m_strMatrixOutFilename));
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
	}

	if (!m_strDFROFilename.IsEmpty()) {
		if (!DFROFile.Open(m_strDFROFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareExclusive)) {
			fprintf(stderr, "*** Error: Opening Diff-Ready Output File \"%s\" for writing...\n", LPCTSTR(m_strDFROFilename));
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
	}

	if (!m_strCompFilename.IsEmpty()) {
		if (!CompFile.Open(m_strCompFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareExclusive)) {
			fprintf(stderr, "*** Error: Opening Compare Output File \"%s\" for writing...\n", LPCTSTR(m_strCompFilename));
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
	}

	if (!m_strOESFilename.IsEmpty()) {
		if (!OESFile.Open(m_strOESFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareExclusive)) {
			fprintf(stderr, "*** Error: Opening Optimal Edit Script Output File \"%s\" for writing...\n", LPCTSTR(m_strOESFilename));
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
	}

	if (!m_strSymFilename.IsEmpty()) {
		if (!SymFile.Open(m_strSymFilename, CFile::modeCreate | CFile::modeWrite | CFile::typeText | CFile::shareExclusive)) {
			fprintf(stderr, "*** Error: Opening Symbol Table Output File \"%s\" for writing...\n", LPCTSTR(m_strSymFilename));
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
	}

	bOkFlag = TRUE;
	while ((bOkFlag) && (nArgPos < argc)) {
		pFuncDescFileObj = new CFuncDescFile;
		if (pFuncDescFileObj == NULL) {
			fprintf(stderr, "*** Error: Out of Memory or Failure Creating FuncDescFile Object...\n");
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
		m_FuncFiles.Add(pFuncDescFileObj);

		if (!FuncFile.Open(argv[nArgPos], CFile::modeRead | CFile::typeText | CFile::shareDenyWrite)) {
			fprintf(stderr, "*** Error: Opening Function Definition File \"%s\" for reading...\n", argv[nArgPos]);
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
		bOkFlag = pFuncDescFileObj->ReadFuncDescFile(FuncFile, &_StdOut, &_StdOut);
		FuncFile.Close();
		nArgPos++;

		if (!m_strDFROFilename.IsEmpty()) {
			for (i=0; i<pFuncDescFileObj->GetFuncCount(); i++) {
				strTemp.Format("File \"%s\" Function \"%s\" (%ld):\n",
										LPCTSTR(pFuncDescFileObj->GetFuncFileName()),
										LPCTSTR(pFuncDescFileObj->GetFunc(i)->GetMainName()),
										i+1);
				DFROFile.WriteString(strTemp);
				DFROFile.WriteString(pFuncDescFileObj->GetFunc(i)->ExportToDiff());
				DFROFile.WriteString("\n\n");
			}
		}
	}


//m_FuncFiles.CompareFunctions(CFuncDescFileArray::FCM_DYNPROG_GREEDY, 0, 31, 1, 32,TRUE);
//_StdOut.m_pStream = NULL;	// prevent closing of stdout
//return;

	if ((!m_strMatrixOutFilename.IsEmpty()) ||
		(!m_strCompFilename.IsEmpty()) ||
		(!m_strOESFilename.IsEmpty()) ||
		(!m_strSymFilename.IsEmpty())) {

		if (m_strMatrixInFilename.IsEmpty()) {
			printf("Cross-Comparing Functions...\n");
		} else {
			printf("Using specified Cross-Comparison Matrix File: \"%s\"...\n", LPCTSTR(m_strMatrixInFilename));
			if (!MatrixInFile.Open(m_strMatrixInFilename, CFile::modeRead | CFile::typeText | CFile::shareDenyWrite)) {
				fprintf(stderr, "*** Error: Opening Matrix Input File \"%s\" for reading...\n", LPCTSTR(m_strMatrixInFilename));

				_StdOut.m_pStream = NULL;	// prevent closing of stdout
				return;
			}
		}

		pFuncFile1 = m_FuncFiles.GetAt(0);
		pFuncFile2 = m_FuncFiles.GetAt(1);

		// Allocate Memory:
		ppnCompResult = new double*[pFuncFile1->GetFuncCount()];
		if (ppnCompResult == NULL) {
			AfxThrowMemoryException();
			_StdOut.m_pStream = NULL;	// prevent closing of stdout
			return;
		}
		for (i=0; i<pFuncFile1->GetFuncCount(); i++) {
			ppnCompResult[i] = new double[pFuncFile2->GetFuncCount()];
			if (ppnCompResult[i] == NULL) AfxThrowMemoryException();
		}

		if (!m_strMatrixInFilename.IsEmpty()) {
			// If we have a MatrixIn file, read cross compare info:
			strTemp = "";
			MatrixInFile.ReadString(strTemp);
			strTemp.TrimLeft();
			strTemp.TrimRight();
			nPos = strTemp.Find(',');
			if (nPos != -1) {
				nTemp1 = atoi(strTemp.Left(nPos));
				nTemp2 = atoi(strTemp.Mid(nPos + 1));
			} else {
				nTemp1 = atoi(strTemp);
				nTemp2 = 0;
			}
			if ((nTemp1 != pFuncFile1->GetFuncCount()) ||
				(nTemp2 != pFuncFile2->GetFuncCount())) {
				fprintf(stderr, "*** Warning: Specified Input Matrix File doesn't match\n");
				fprintf(stderr, "        the specified function description files.  A full\n");
				fprintf(stderr, "        cross-comparison will be performed!\n\n");
				m_strMatrixInFilename.Empty();
			} else {
				MatrixInFile.ReadString(strTemp);		// Read and toss x break-points line
				for (i=0; i<pFuncFile1->GetFuncCount(); i++) {
					strTemp = "";
					MatrixInFile.ReadString(strTemp);
					strTemp.TrimLeft();
					strTemp.TrimRight();
					nPos = strTemp.Find(',');
					strTemp = strTemp.Mid(nPos+1);		// Toss y break-point
					for (j=0; j<pFuncFile2->GetFuncCount(); j++) {
						nPos = strTemp.Find(',');
						if (nPos != -1) {
							ppnCompResult[i][j] = atof(strTemp.Left(nPos));
							strTemp = strTemp.Mid(nPos+1);
							strTemp.TrimLeft();
						} else {
							ppnCompResult[i][j] = atof(strTemp);
							strTemp = "";
						}
					}
				}
			}

			MatrixInFile.Close();
		}

		if (m_strMatrixInFilename.IsEmpty()) {
			// If no MatrixIn file was specified, do complete cross comparison:
			fprintf(stderr, "Please Wait");
			if (!m_strMatrixOutFilename.IsEmpty()) {
				strTemp.Format("%ld,%ld\n", pFuncFile1->GetFuncCount(), pFuncFile2->GetFuncCount());
				MatrixOutFile.WriteString(strTemp);
				for (j=0; j<pFuncFile2->GetFuncCount(); j++) {
					strTemp.Format(",%ld", j+1);
					MatrixOutFile.WriteString(strTemp);
				}
			}
			if (!m_strMatrixOutFilename.IsEmpty()) MatrixOutFile.WriteString("\n");
			for (i=0; i<pFuncFile1->GetFuncCount(); i++) {
				fprintf(stderr, ".");
				if (!m_strMatrixOutFilename.IsEmpty()) {
					strTemp.Format("%ld", i+1);
					MatrixOutFile.WriteString(strTemp);
				}
				for (j=0; j<pFuncFile2->GetFuncCount(); j++) {
					ppnCompResult[i][j] = CompareFunctions(nCompMethod, pFuncFile1, i, pFuncFile2, j, FALSE);
					if (!m_strMatrixOutFilename.IsEmpty()) {
						strTemp.Format(",%.12g", ppnCompResult[i][j]);
						MatrixOutFile.WriteString(strTemp);
					}
				}
				if (!m_strMatrixOutFilename.IsEmpty()) MatrixOutFile.WriteString("\n");
			}
			fprintf(stderr, "\n\n");
		}

		printf("\nBest Matches:\n");

		if (!m_strCompFilename.IsEmpty()) {
			CompFile.WriteString("Left Filename  : ");
			CompFile.WriteString(pFuncFile1->GetFuncPathName() + "\n");
			CompFile.WriteString("Right Filename : ");
			CompFile.WriteString(pFuncFile2->GetFuncPathName() + "\n");
			CompFile.WriteString("\n");
		}

		if (!m_strOESFilename.IsEmpty()) {
			OESFile.WriteString("; Left Filename  : ");
			OESFile.WriteString(pFuncFile1->GetFuncPathName() + "\n");
			OESFile.WriteString("; Right Filename : ");
			OESFile.WriteString(pFuncFile2->GetFuncPathName() + "\n");
		}

		if (!m_strSymFilename.IsEmpty()) {
			SymFile.WriteString("; Left Filename  : ");
			SymFile.WriteString(pFuncFile1->GetFuncPathName() + "\n");
			SymFile.WriteString("; Right Filename : ");
			SymFile.WriteString(pFuncFile2->GetFuncPathName() + "\n");
		}

		aSymbolMap.FreeAll();

		for (i=0; i<pFuncFile1->GetFuncCount(); i++) {
			nMaxCompResult = 0;
			for (j=0; j<pFuncFile2->GetFuncCount(); j++) nMaxCompResult = __max(nMaxCompResult, ppnCompResult[i][j]);
			bFlag = FALSE;
			if ((nMaxCompResult > 0) && (nMaxCompResult >= nMinCompLimit)) {
				printf("    %s : ", LPCTSTR(PadString(pFuncFile1->GetFunc(i)->GetMainName(), CFuncObject::GetFieldWidth(CFuncObject::FC_LABEL))));
				for (j=0; j<pFuncFile2->GetFuncCount(); j++) {
					if (ppnCompResult[i][j] < nMaxCompResult) continue;

					if (bFlag) {
						if (!m_strCompFilename.IsEmpty()) {
							CompFile.WriteString("\n\n");
						}
						printf(", ");
					} else {
						if (!m_strCompFilename.IsEmpty()) {
							CompFile.WriteString("================================================================================\n");
						}
					}
					bFlag = TRUE;

					printf("%s", LPCTSTR(pFuncFile2->GetFunc(j)->GetMainName()));

					if (!m_strCompFilename.IsEmpty()) {
						CompFile.WriteString("--------------------------------------------------------------------------------\n");
						strTemp.Format("    Left Function  : %s (%ld)\n    Right Function : %s (%ld)\n    Matches by     : %f%%\n",
										LPCTSTR(pFuncFile1->GetFunc(i)->GetMainName()), i+1,
										LPCTSTR(pFuncFile2->GetFunc(j)->GetMainName()), j+1,
										ppnCompResult[i][j]*100);
						CompFile.WriteString(strTemp);
						CompFile.WriteString("--------------------------------------------------------------------------------\n");
						strTemp = DiffFunctions(nCompMethod, pFuncFile1, i, pFuncFile2, j, &aSymbolMap);
						if (bCompOESFlag) {
							if (GetLastEditScript(oes)) {
								for (k=0; k<oes.GetSize(); k++) {
									CompFile.WriteString(oes.GetAt(k) + "\n");
								}
							}
							CompFile.WriteString("--------------------------------------------------------------------------------\n");
						}
						CompFile.WriteString(strTemp);
						CompFile.WriteString("--------------------------------------------------------------------------------\n");
					}

					if (!m_strOESFilename.IsEmpty()) {
						CompareFunctions(nCompMethod, pFuncFile1, i, pFuncFile2, j, TRUE);
						if (GetLastEditScript(oes)) {
							OESFile.WriteString("\n");
							strTemp.Format("@%s(%ld)|%s(%ld)\n",
										LPCTSTR(pFuncFile1->GetFunc(i)->GetMainName()), i+1,
										LPCTSTR(pFuncFile2->GetFunc(j)->GetMainName()), j+1);
							OESFile.WriteString(strTemp);
							for (k=0; k<oes.GetSize(); k++) {
								OESFile.WriteString(oes.GetAt(k) + "\n");
							}
						}
					}
				}
				if (!m_strCompFilename.IsEmpty()) {
					CompFile.WriteString("================================================================================\n\n\n");
				}
				printf(" : (%f%%)", nMaxCompResult * 100);
				printf("\n");
			}
		}

		printf("\nFunctions in \"%s\" with No Matches:\n", LPCTSTR(pFuncFile1->GetFuncPathName()));
		bFlag = TRUE;
		for (i=0; i<pFuncFile1->GetFuncCount(); i++) {
			nMaxCompResult = 0;
			for (j=0; j<pFuncFile2->GetFuncCount(); j++) nMaxCompResult = __max(nMaxCompResult, ppnCompResult[i][j]);
			if ((nMaxCompResult <= 0) || (nMaxCompResult < nMinCompLimit)) {
				printf("    %s\n", LPCTSTR(pFuncFile1->GetFunc(i)->GetMainName()));
				bFlag = FALSE;
			}
		}
		if (bFlag) printf("    <None>\n");

		printf("\nFunctions in \"%s\" with No Matches:\n", LPCTSTR(pFuncFile2->GetFuncPathName()));
		bFlag = TRUE;
		for (j=0; j<pFuncFile2->GetFuncCount(); j++) {
			nMaxCompResult = 0;
			for (i=0; i<pFuncFile1->GetFuncCount(); i++) nMaxCompResult = __max(nMaxCompResult, ppnCompResult[i][j]);
			if ((nMaxCompResult <= 0) || (nMaxCompResult < nMinCompLimit)) {
				printf("    %s\n", LPCTSTR(pFuncFile2->GetFunc(j)->GetMainName()));
				bFlag = FALSE;
			}
		}
		if (bFlag) printf("    <None>\n");

		// Dump Symbol Tables:
		CStringArray arrSymbolList;
		CStringArray arrHitList;
		CDWordArray arrHitCount;
		DWORD nTotalHits;
		int nSymWidth;

		printf("\nCross-Comparing Symbol Tables...\n");

		printf("\nLeft-Side Code Symbol Matches:\n");
		printf("------------------------------\n");
		if (!m_strSymFilename.IsEmpty()) {
			SymFile.WriteString("\nLeft-Side Code Symbol Matches:\n");
			SymFile.WriteString("------------------------------\n");
		}
		aSymbolMap.GetLeftSideCodeSymbolList(arrSymbolList);
		nSymWidth = 0;
		for (i=0; i<arrSymbolList.GetSize(); i++) {
			if (arrSymbolList.GetAt(i).GetLength() > nSymWidth) nSymWidth = arrSymbolList.GetAt(i).GetLength();
		}
		for (i=0; i<arrSymbolList.GetSize(); i++) {
			nTotalHits = aSymbolMap.GetLeftSideCodeHitList(arrSymbolList.GetAt(i), arrHitList, arrHitCount);
			ASSERT(arrHitList.GetSize() == arrHitCount.GetSize());
			strTemp = arrSymbolList.GetAt(i);
			for (j=0; j<(nSymWidth-arrSymbolList.GetAt(i).GetLength()); j++) strTemp += ' ';
			printf("%s : ", LPCTSTR(strTemp));
			if (!m_strSymFilename.IsEmpty()) SymFile.WriteString(strTemp + " : ");
			if (arrHitList.GetSize() > 0) {
				if (nTotalHits == 0) nTotalHits = 1;	// Safe-guard for divide by zero
				for (j=0; j<arrHitList.GetSize(); j++) {
					if (arrHitList.GetAt(j).IsEmpty()) arrHitList.SetAt(j, "???");
					strTemp.Format("%s%s (%f%%)", ((j!=0) ? ", " : ""), LPCTSTR(arrHitList.GetAt(j)),
										((double)arrHitCount.GetAt(j)/nTotalHits)*100);
					printf("%s", LPCTSTR(strTemp));
					if (!m_strSymFilename.IsEmpty()) SymFile.WriteString(strTemp);
				}
				printf("\n");
				if (!m_strSymFilename.IsEmpty()) SymFile.WriteString("\n");
			} else {
				printf("<none>\n");
				if (!m_strSymFilename.IsEmpty()) SymFile.WriteString("<none>\n");
			}
		}

		printf("\nLeft-Side Data Symbol Matches:\n");
		printf("------------------------------\n");
		if (!m_strSymFilename.IsEmpty()) {
			SymFile.WriteString("\nLeft-Side Data Symbol Matches:\n");
			SymFile.WriteString("------------------------------\n");
		}
		aSymbolMap.GetLeftSideDataSymbolList(arrSymbolList);
		nSymWidth = 0;
		for (i=0; i<arrSymbolList.GetSize(); i++) {
			if (arrSymbolList.GetAt(i).GetLength() > nSymWidth) nSymWidth = arrSymbolList.GetAt(i).GetLength();
		}
		for (i=0; i<arrSymbolList.GetSize(); i++) {
			nTotalHits = aSymbolMap.GetLeftSideDataHitList(arrSymbolList.GetAt(i), arrHitList, arrHitCount);
			ASSERT(arrHitList.GetSize() == arrHitCount.GetSize());
			strTemp = arrSymbolList.GetAt(i);
			for (j=0; j<(nSymWidth-arrSymbolList.GetAt(i).GetLength()); j++) strTemp += ' ';
			printf("%s : ", LPCTSTR(strTemp));
			if (!m_strSymFilename.IsEmpty()) SymFile.WriteString(strTemp + " : ");
			if (arrHitList.GetSize() > 0) {
				if (nTotalHits == 0) nTotalHits = 1;	// Safe-guard for divide by zero
				for (j=0; j<arrHitList.GetSize(); j++) {
					if (arrHitList.GetAt(j).IsEmpty()) arrHitList.SetAt(j, "???");
					strTemp.Format("%s%s (%f%%)", ((j!=0) ? ", " : ""), LPCTSTR(arrHitList.GetAt(j)),
										((double)arrHitCount.GetAt(j)/nTotalHits)*100);
					printf("%s", LPCTSTR(strTemp));
					if (!m_strSymFilename.IsEmpty()) SymFile.WriteString(strTemp);
				}
				printf("\n");
				if (!m_strSymFilename.IsEmpty()) SymFile.WriteString("\n");
			} else {
				printf("<none>\n");
				if (!m_strSymFilename.IsEmpty()) SymFile.WriteString("<none>\n");
			}
		}

		printf("\nRight-Side Code Symbol Matches:\n");
		printf("-------------------------------\n");
		if (!m_strSymFilename.IsEmpty()) {
			SymFile.WriteString("\nRight-Side Code Symbol Matches:\n");
			SymFile.WriteString("-------------------------------\n");
		}
		aSymbolMap.GetRightSideCodeSymbolList(arrSymbolList);
		nSymWidth = 0;
		for (i=0; i<arrSymbolList.GetSize(); i++) {
			if (arrSymbolList.GetAt(i).GetLength() > nSymWidth) nSymWidth = arrSymbolList.GetAt(i).GetLength();
		}
		for (i=0; i<arrSymbolList.GetSize(); i++) {
			nTotalHits = aSymbolMap.GetRightSideCodeHitList(arrSymbolList.GetAt(i), arrHitList, arrHitCount);
			ASSERT(arrHitList.GetSize() == arrHitCount.GetSize());
			strTemp = arrSymbolList.GetAt(i);
			for (j=0; j<(nSymWidth-arrSymbolList.GetAt(i).GetLength()); j++) strTemp += ' ';
			printf("%s : ", LPCTSTR(strTemp));
			if (!m_strSymFilename.IsEmpty()) SymFile.WriteString(strTemp + " : ");
			if (arrHitList.GetSize() > 0) {
				if (nTotalHits == 0) nTotalHits = 1;	// Safe-guard for divide by zero
				for (j=0; j<arrHitList.GetSize(); j++) {
					if (arrHitList.GetAt(j).IsEmpty()) arrHitList.SetAt(j, "???");
					strTemp.Format("%s%s (%f%%)", ((j!=0) ? ", " : ""), LPCTSTR(arrHitList.GetAt(j)),
										((double)arrHitCount.GetAt(j)/nTotalHits)*100);
					printf("%s", LPCTSTR(strTemp));
					if (!m_strSymFilename.IsEmpty()) SymFile.WriteString(strTemp);
				}
				printf("\n");
				if (!m_strSymFilename.IsEmpty()) SymFile.WriteString("\n");
			} else {
				printf("<none>\n");
				if (!m_strSymFilename.IsEmpty()) SymFile.WriteString("<none>\n");
			}
		}

		printf("\nRight-Side Data Symbol Matches:\n");
		printf("-------------------------------\n");
		if (!m_strSymFilename.IsEmpty()) {
			SymFile.WriteString("\nRight-Side Data Symbol Matches:\n");
			SymFile.WriteString("-------------------------------\n");
		}
		aSymbolMap.GetRightSideDataSymbolList(arrSymbolList);
		nSymWidth = 0;
		for (i=0; i<arrSymbolList.GetSize(); i++) {
			if (arrSymbolList.GetAt(i).GetLength() > nSymWidth) nSymWidth = arrSymbolList.GetAt(i).GetLength();
		}
		for (i=0; i<arrSymbolList.GetSize(); i++) {
			nTotalHits = aSymbolMap.GetRightSideDataHitList(arrSymbolList.GetAt(i), arrHitList, arrHitCount);
			ASSERT(arrHitList.GetSize() == arrHitCount.GetSize());
			strTemp = arrSymbolList.GetAt(i);
			for (j=0; j<(nSymWidth-arrSymbolList.GetAt(i).GetLength()); j++) strTemp += ' ';
			printf("%s : ", LPCTSTR(strTemp));
			if (!m_strSymFilename.IsEmpty()) SymFile.WriteString(strTemp + " : ");
			if (arrHitList.GetSize() > 0) {
				if (nTotalHits == 0) nTotalHits = 1;	// Safe-guard for divide by zero
				for (j=0; j<arrHitList.GetSize(); j++) {
					if (arrHitList.GetAt(j).IsEmpty()) arrHitList.SetAt(j, "???");
					strTemp.Format("%s%s (%f%%)", ((j!=0) ? ", " : ""), LPCTSTR(arrHitList.GetAt(j)),
										((double)arrHitCount.GetAt(j)/nTotalHits)*100);
					printf("%s", LPCTSTR(strTemp));
					if (!m_strSymFilename.IsEmpty()) SymFile.WriteString(strTemp);
				}
				printf("\n");
				if (!m_strSymFilename.IsEmpty()) SymFile.WriteString("\n");
			} else {
				printf("<none>\n");
				if (!m_strSymFilename.IsEmpty()) SymFile.WriteString("<none>\n");
			}
		}

		// Deallocate Memory:
		for (i=0; i<pFuncFile1->GetFuncCount(); i++) delete[] (ppnCompResult[i]);
		delete[] ppnCompResult;
	}

	printf("\nFunction Analysis Complete...\n\n");

	if (!m_strMatrixOutFilename.IsEmpty()) MatrixOutFile.Close();
	if (!m_strDFROFilename.IsEmpty()) DFROFile.Close();
	if (!m_strCompFilename.IsEmpty()) CompFile.Close();
	if (!m_strOESFilename.IsEmpty()) OESFile.Close();
	if (!m_strSymFilename.IsEmpty()) SymFile.Close();

	_StdOut.m_pStream = NULL;	// prevent closing of stdout
}


