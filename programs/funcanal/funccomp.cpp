// funccomp.cpp: implementation of the Fuzzy Function Comparison Logic
//
// $Log: funccomp.cpp,v $
// Revision 1.2  2003/09/13 05:39:49  dewhisna
// Added output options and returning of match percentage to function diff.
//
// Revision 1.1  2003/02/09 06:59:29  dewhisna
// Initial Revision - Split comparison logic from Function File I/O
//
//

//
//	The Edit Script is defined as follows:
//
//		During comparison an optimal edit script is calculated and is stored as a CStringArray.  Each
//			entry is a string of the following format, which is similar to the Diff format except
//			that each entry is unique rather than specifying ranges:
//				xxxCyyy
//
//					Where:
//						xxx = Left side index
//						yyy = Right side index
//						C is one of the following symbols:
//							'>' - Delete xxx from left at point yyy in right or
//									insert xxx from left at point yyy in right
//							'-' - Replace xxx in left with yyyy in right
//							'<' - Insert yyy from right at left point xxx or
//									delete yyy from right at point xxx in left
//

#include "stdafx.h"
#include "funcdesc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static CStringArray m_EditScript;
static BOOL m_bEditScriptValid = FALSE;


double CompareFunctions(FUNC_COMPARE_METHOD nMethod,
											CFuncDescFile *pFile1, int nFile1FuncNdx,
											CFuncDescFile *pFile2, int nFile2FuncNdx,
											BOOL bBuildEditScript)
{
	CFuncDesc *pFunction1;
	CFuncDesc *pFunction2;
	CStringArray zFunc1;
	CStringArray zFunc2;
	double nRetVal = 0;
	CString strTemp;

	m_bEditScriptValid = FALSE;
	m_EditScript.RemoveAll();

	if ((pFile1 == NULL) || (pFile2 == NULL)) return nRetVal;

	pFunction1 = pFile1->GetFunc(nFile1FuncNdx);
	pFunction2 = pFile2->GetFunc(nFile2FuncNdx);
	if ((pFunction1 == NULL) ||
		(pFunction2 == NULL)) return nRetVal;

	pFunction1->ExportToDiff(zFunc1);
	pFunction2->ExportToDiff(zFunc2);
	if ((zFunc1.GetSize() == 0) ||
		(zFunc2.GetSize() == 0)) return nRetVal;

	if ((bBuildEditScript) && (nMethod == FCM_DYNPROG_XDROP)) {
		// Note: XDROP Method currently doesn't support building
		//		of edit scripts, so if caller wants to build an
		//		edit script and has selected this method, replace
		//		it with the next best method that does support
		//		edit scripts:
		nMethod = FCM_DYNPROG_GREEDY;
	}

	switch (nMethod) {
		case FCM_DYNPROG_XDROP:
		{
			//
			//	The following algorithm comes from the following source:
			//			"A Greedy Algorithm for Aligning DNA Sequences"
			//			Zheng Zhang, Scott Schwartz, Lukas Wagner, and Webb Miller
			//			Journal of Computational Biology
			//			Volume 7, Numbers 1/2, 2000
			//			Mary Ann Liebert, Inc.
			//			Pp. 203-214
			//
			//			p. 205 : Figure 2 : "A dynamic-programming X-drop algorithm"
			//
			//		T' <- T <- S(0,0) <- 0
			//		k <- L <- U <- 0
			//		repeat {
			//			k <- k + 1
			//			for i <- ceiling(L) to floor(U)+1 in steps of 1/2 do {
			//				j <- k - i
			//				if i is an integer then {
			//					S(i, j) <- Max of:
			//								S(i-1/2, j-1/2) + mat/2		if L <= i-1/2 <= U and a(i) = b(j)
			//								S(i-1/2, j-1/2) + mis/2		if L <= i-1/2 <= U and a(i) != b(j)
			//								S(i, j-1) + ind				if i <= U
			//								S(i-1, j) + ind				if L <= i-1
			//				} else {
			//					S(i, j) <- S(i-1/2, j-1/2)
			//								+ mat/2	if a(i+1/2) = b(j+1/2)
			//								+ mis/2	if a(i+1/2) != b(j+1/2)
			//				}
			//				T' <- max{T', S(i, j)}
			//				if S(i, j) < (T - X) then S(i, j) <- -oo
			//			}
			//			L <- min{i : S(i, k-i) > -oo }
			//			U <- max{i : S(i, k-i) > -oo }
			//			L <- max{L, k+1-N}				<<< Should be: L <- max{L, k+1/2-N}
			//			U <- min{U, M-1}				<<< Should be: U <- min(U, M-1/2}
			//			T <- T'
			//		} until L > U+1
			//		report T'
			//
			//	Given:
			//		arrays: a(1..M), b(1..N) containing the two strings to compare
			//		mat : >0 : Weighting of a match
			//		mis : <0 : Weighting of a mismatch
			//		ind : <0 : Weighting of an insertion/deletion
			//		X = Clipping level : If scoring falls more than X below the
			//				best computed score thus far, then we don't consider
			//				additional extensions for that alignment.  Should
			//				be >= 0 or -1 for infinity.
			//
			//	Returning:
			//		T' = Composite similarity score
			//
			//	For programmatic efficiency, all S indexes have been changed from
			//		0, 1/2 to even/odd and the i loop runs as integers of even/odd
			//		instead of 1/2 increments:
			//
			//	Testing has also been done and it has been proven that the order of
			//		the two arrays has no effect on the outcome.
			//
			//	In the following, we will define oo as DBL_MAX and -oo as -DBL_MAX.
			//
			//	To hold to the non-Generalized Greedy Algorithm requirements, set
			//		ind = mis - mat/2
			//

			CStringArray &a = zFunc1;
			CStringArray &b = zFunc2;
			double Tp, T;
			double **S;
			int i, j, k, L, U;
			double nTemp;
			int M = a.GetSize();
			int N = b.GetSize();
			const double mat = 2;
			const double mis = -2;
			const double ind = -3;
			const double X = -1;

			// Allocate Memory:
			S = new double*[((M+1)*2)];
			if (S == NULL) {
				AfxThrowMemoryException();
				return nRetVal;
			}
			for (i=0; i<((M+1)*2); i++) {
				S[i] = new double[((N+1)*2)];
				if (S[i] == NULL) AfxThrowMemoryException();
			}

			// Initialize:
			for (i=0; i<((M+1)*2); i++) {
				for (j=0; j<((N+1)*2); j++) {
					S[i][j] = -DBL_MAX;
				}
			}

			// Algorithm:
			Tp = T = S[0][0] = 0;
			k = L = U = 0;

			do {
				k = k + 2;
				for (i = L+((L & 0x1) ? 1 : 0); i <= (U - ((U & 0x1) ? 1 : 0) + 2); i++) {
					j = k - i;
					ASSERT(i >= 0);
					ASSERT(i < ((M+1)*2));
					ASSERT(j >= 0);
					ASSERT(j < ((N+1)*2));
					if ((i&1) == 0) {
						nTemp = -DBL_MAX;
						if ((L <= (i-1)) &&
							((i-1) <= U)) {
							// TODO : Improve A/B Comparison:
							if (a.GetAt((i/2)-1).CompareNoCase(b.GetAt((j/2)-1)) == 0) {
								nTemp = __max(nTemp, S[i-1][j-1] + mat/2);
							} else {
								nTemp = __max(nTemp, S[i-1][j-1] + mis/2);
							}
						}
						if (i <= U) {
							nTemp = __max(nTemp, S[i][j-2] + ind);
						}
						if (L <= (i-2)) {
							nTemp = __max(nTemp, S[i-2][j] + ind);
						}
						S[i][j] = nTemp;
					} else {
						// TODO : Improve A/B Comparison:
						if (a.GetAt(((i+1)/2)-1).CompareNoCase(b.GetAt(((j+1)/2)-1)) == 0) {
							S[i][j] = S[i-1][j-1] + mat/2;
						} else {
							S[i][j] = S[i-1][j-1] + mis/2;
						}
					}
					Tp = __max(Tp, S[i][j]);
					if ((X>=0) && (S[i][j] < (T-X))) S[i][j] = -DBL_MAX;
				}

				for (L = 0; L < ((M+1)*2); L++) {
					if ((k - L) < 0) continue;
					if ((k - L) >= ((N+1)*2)) continue;
					if (S[L][k-L] > -DBL_MAX) break;
				}
				if (L == ((M+1)*2)) L=INT_MAX;

				for (U=((M+1)*2)-1; U>=0; U--) {
					if ((k - U) < 0) continue;
					if ((k - U) >= ((N+1)*2)) continue;
					if (S[U][k-U] > -DBL_MAX) break;
				}
				if (U == -1) U=INT_MIN;

				L = __max(L, k + 1 - (N*2));
				U = __min(U, (M*2) - 1);
				T = Tp;
			} while (L <= U+2);

			// If the two PrimaryLabels at the function's address don't match, decrement match by 1*mat.
			//		This helps if there are two are more functions that the user has labeled that
			//		are identical except for the labels:
			if (pFile1->GetPrimaryLabel(pFunction1->GetMainAddress()).CompareNoCase(
					pFile2->GetPrimaryLabel(pFunction2->GetMainAddress())) != 0) Tp = __max(0, Tp - mat);

			// Normalize it:
			nRetVal = Tp/(__max(M,N)*mat);

			// Deallocate Memory:
			for (i=0; i<((M+1)*2); i++) delete[] (S[i]);
			delete[] S;
		}
		break;

		case FCM_DYNPROG_GREEDY:
		{
			//
			//	The following algorithm comes from the following source:
			//			"A Greedy Algorithm for Aligning DNA Sequences"
			//			Zheng Zhang, Scott Schwartz, Lukas Wagner, and Webb Miller
			//			Journal of Computational Biology
			//			Volume 7, Numbers 1/2, 2000
			//			Mary Ann Liebert, Inc.
			//			Pp. 203-214
			//
			//			p. 209 : Figure 4 : "Greedy algorithm that is equivalent to the algorithm
			//								of Figure 2 if ind = mis - mat/2."
			//
			//		i <- 0
			//		while i < min{M, N} and a(i+1) = b(i+1) do i <- i + 1
			//		R(0, 0) <- i
			//		T' <- T[0] <- S'(i+i, 0)
			//		d <- L <- U <- 0
			//		repeat
			//			d <- d + 1
			//			d' <- d - floor( (X + mat/2)/(mat - mis) ) - 1
			//			for k <- L - 1 to U + 1 do
			//				i <- max of:
			//							R(d-1, k-1) + 1		if L < k
			//							R(d-1, k) + 1		if L <= k <= U
			//							R(d-1, k+1)			if k < U
			//				j <- i - k
			//				if i > -oo and S'(i+j, d) >= T[d'] - X then
			//					while i<M, j<N, and a(i+1) = b(j+1) do
			//						i <- i + 1;  j <- j + 1
			//					R(d, k) <- i
			//					T' <- max{T', S'(i+j, d)}
			//				else R(d, k) <- -oo
			//			T[d] <- T'
			//			L <- min{k : R(d, k) > -oo}
			//			U <- max{k : R(d, k) > -oo}
			//			L <- max{L, max{k : R(d, k) = N + k} + 2}	<<< Should be: L <- max{L, max{k : R(d, k) = N + k} + 1}
			//			U <- min{U, min{k : R(d, k) = M } - 2}		<<< Should be: U <- min{U, min{k : R(d, k) = M } - 1}
			//		until L > U + 2
			//		report T'
			//
			//	Given:
			//		arrays: a(1..M), b(1..N) containing the two strings to compare
			//		mat : >0 : Weighting of a match
			//		mis : <0 : Weighting of a mismatch
			//		ind : <0 : Weighting of an insertion/deletion
			//		(Satisfying: ind = mis - mat/2)
			//		X = Clipping level : If scoring falls more than X below the
			//				best computed score thus far, then we don't consider
			//				additional extensions for that alignment.  Should
			//				be >= 0 or -1 for infinity.
			//		S'(i+j, d) = (i+j)*mat/2 - d*(mat-mis)
			//		or S'(k, d) = k*mat/2 - d*(mat-mis)
			//
			//	Returning:
			//		T' = Composite similarity score
			//
			//	Testing has also been done and it has been proven that the order of
			//		the two arrays has no effect on the outcome.
			//
			//	In the following it will be assumed that the number of mismatches
			//	(i.e. array bounds) can't exceed M+N since the absolute maximum
			//	edit script to go from an array of M objects to an array of N
			//	objects is to perform M deletions and N insertions.  However,
			//	these differences can be tracked either forward or backward, so
			//	the memory will be allocated for the full search field.
			//
			//	We are also going to define -oo as being -2 since no index can be
			//	lower than 0.  The reason for the -2 instead of -1 is to allow
			//	for the i=R(d,k)+1 calculations to still be below 0.  That is
			//	to say so that -oo + 1 = -oo
			//

			CStringArray &a = zFunc1;
			CStringArray &b = zFunc2;
			double Tp;				// T' = Overall max for entire comparison
			double Tpp;				// T'' = Overall max for current d value
			double *T;
			double nTemp;
			int **Rm;
			int *Rvisitmin;			// Minimum k-index of R visited for a particular d (for speed)
			int *Rvisitmax;			// Maximum k-index of R visited for a particular k (for speed)
			int i, j, k, L, U;
			int d, dp;
			int dbest, kbest;
			int M = a.GetSize();
			int N = b.GetSize();
			const int dmax = ((M+N)*2)+1;
			const int kmax = (M+N+1);
			const int rksize = (kmax*2)+1;
			const double mat = 2;
			const double mis = -2;
			const double X = -1;
			const int floored_d_offset = (int)((X+(mat/2))/(mat-mis));
			#define Sp(x, y) ((double)((x)*(mat/2) - ((y)*(mat-mis))))
			#define R(x, y) (Rm[(x)][(y)+kmax])

			// Allocate Memory:
			T = new double[dmax];
			if (T == NULL) {
				AfxThrowMemoryException();
				return nRetVal;
			}

			Rvisitmin = new int[dmax];
			if (Rvisitmin == NULL) {
				AfxThrowMemoryException();
				delete T;
				return nRetVal;
			}

			Rvisitmax = new int[dmax];
			if (Rvisitmax == NULL) {
				AfxThrowMemoryException();
				delete T;
				delete Rvisitmin;
				return nRetVal;
			}

			Rm = new int*[dmax];
			if (Rm == NULL) {
				AfxThrowMemoryException();
				delete T;
				delete Rvisitmin;
				delete Rvisitmax;
				return nRetVal;
			}
			for (i=0; i<dmax; i++) {
				Rm[i] = new int[rksize];
				if (Rm[i] == NULL) AfxThrowMemoryException();
			}

			// Initialize:
			for (i=0; i<dmax; i++) {
				T[i] = 0;
				Rvisitmin[i] = kmax+1;
				Rvisitmax[i] = -kmax-1;
				for (j=0; j<rksize; j++) {
					Rm[i][j] = -2;
				}
			}

			// Algorithm:
			i=0;
			// TODO : Improve A/B Comparison:
			while ((i<__min(M, N)) && (a.GetAt(i).CompareNoCase(b.GetAt(i)) == 0)) i++;
			R(0, 0) = i;
			dbest = kbest = 0;
			Tp = T[0] = Sp(i+i, 0);
			d = L = U = 0;
			Rvisitmin[0] = 0;
			Rvisitmax[0] = 0;

/*
printf("\n");
*/

			if ((i != M) || (i != N)) {
				do {
					d++;
					dp = d - floored_d_offset - 1;
					Tpp = -DBL_MAX;
					for (k=(L-1); k<=(U+1); k++) {
						ASSERT(d > 0);
						ASSERT(d < dmax);
						ASSERT(abs(k) <= kmax);
						i = -2;
						if (L < k)			i = __max(i, R(d-1, k-1)+1);
						if ((L <= k) &&
							(k <= U))		i = __max(i, R(d-1, k)+1);
						if (k < U)			i = __max(i, R(d-1, k+1));
						j = i - k;
						if ((i >= 0) && (j >= 0) && ((X<0) || (Sp(i+j, d) >= (((dp >= 0) ? T[dp] : 0) - X)))) {
							// TODO : Improve A/B Comparison:
							while ((i<M) && (j<N) && (a.GetAt(i).CompareNoCase(b.GetAt(j)) == 0)) {
								i++; j++;
							}

							R(d, k) = i;
							if (Rvisitmin[d] > k) Rvisitmin[d] = k;
							if (Rvisitmax[d] < k) Rvisitmax[d] = k;
							nTemp = Sp(i+j, d);
							Tp = __max(Tp, nTemp);
/*
printf("d=%2ld : k=%2ld, i=%2ld, j=%2ld, M=%2ld, N=%2ld, T=%2ld, Tp=%2ld, Tpp=%2ld", d, k, i, j, M, N, (int)nTemp, (int)Tp, (int)Tpp);
*/
							if (nTemp > Tpp) {
								Tpp = nTemp;
/*
printf(" * Best (%2ld)", (int)Tpp);
*/
									dbest = d;
									kbest = k;


									// Account for hitting the max M or N boundaries:
									if ((i != M) || (j != N)) {
										if (j > N) {
											kbest++;
/*
printf(" >>>>>>  k++ j shift");
*/
										} else {
											if (i > M) {
												kbest--;
/*
printf(" <<<<<<  k-- i shift");
*/
											}
										}
									}

							}
/*
printf("\n");
*/

						} else {
							R(d, k) = -2;
							if (Rvisitmin[d] == k) Rvisitmin[d]++;
							if (Rvisitmax[d] >= k) Rvisitmax[d] = k-1;
						}
					}
					T[d] = Tp;

					L = Rvisitmin[d];
					U = Rvisitmax[d];
					for (k=Rvisitmax[d]; k>=Rvisitmin[d]; k--) if (R(d, k) == (N+k)) break;
					if (k<Rvisitmin[d]) k = INT_MIN;
					L = __max(L, k+1);
					for (k=Rvisitmin[d]; k<=Rvisitmax[d]; k++) if (R(d, k) == M) break;
					if (k>Rvisitmax[d]) k = INT_MAX;
					U = __min(U, k-1);
				} while (L <= U+2);
			}

			// If the two PrimaryLabels at the function's address don't match, decrement match by 1*mat.
			//		This helps if there are two are more functions that the user has labeled that
			//		are identical except for the labels:
			if (pFile1->GetPrimaryLabel(pFunction1->GetMainAddress()).CompareNoCase(
					pFile2->GetPrimaryLabel(pFunction2->GetMainAddress())) != 0) Tp = __max(0, Tp - mat);

			// Normalize it:
			nRetVal = Tp/(__max(M,N)*mat);

			// Build Edit Script:
			if (bBuildEditScript) {
				int last_i, last_j;
				int cur_i, cur_j;
				int k_rest;

				if (dbest > 0) {
					m_EditScript.SetSize(dbest);
					k = kbest;
					last_i = M+1;
					last_j = N+1;

/*
printf("\n%s with %s:\n", LPCTSTR(pFunction1->GetMainName()), LPCTSTR(pFunction2->GetMainName()));
*/

					for (d=dbest-1; d>=0; d--) {
						i = __max((R(d, k-1) + 1), __max((R(d, k) + 1), (R(d, k+1))));

/*
printf("(%3ld, %3ld) : %3ld(%5ld), %3ld(%5ld), %3ld(%5ld) :", d, k,
						(R(d, k-1) + 1), (int)Sp((R(d, k-1))*2-k+1, d),
						(R(d, k) + 1), (int)Sp((R(d, k))*2-k, d),
						(R(d, k+1)), (int)Sp((R(d, k+1))*2-k-1, d));

for (j=Rvisitmin[dbest-1]; j<=Rvisitmax[dbest-1]; j++) {
	if (j == k-1) printf("("); else printf(" ");
	if (R(d,j)<0) printf("   "); else printf("%3ld", R(d, j));
	if (j == k+1) printf(")"); else printf(" ");
}
printf("\n");
*/

						j = i-k;

						if (i == (R(d, k-1) + 1)) {
							strTemp.Format("%ld>%ld", i-1, j);
							cur_i = i-1;
							cur_j = j;
							k--;
							k_rest = 1;
						} else {
							if (i == (R(d, k+1))) {
								strTemp.Format("%ld<%ld", i, j-1);
								cur_i = i;
								cur_j = j-1;
								k++;
								k_rest = -1;
							} else {
								// if (i == (R(d, k) + 1))
								strTemp.Format("%ld-%ld", i-1, j-1);
								cur_i = i-1;
								cur_j = j-1;
								// k=k;
								k_rest = 0;
							}
						}
						m_EditScript.SetAt(d, strTemp);
						// The following test is needed since our insertion/deletion indexes are
						//		one greater than the stored i and/or j values from the R matrix.
						//		It is possible that the previous comparison added some extra
						//		entries to the R matrix than what was really needed.  This will
						//		cause extra erroneous entries to appear in the edit script.
						//		However, since the indexes should be always increasing, we simply
						//		filter out extra entries added to the end that don't satisfy
						//		this condition:
						if ((k_rest == 0) &&
							((cur_i == last_i) && (cur_j == last_j))) {
							m_EditScript.RemoveAt(d);
						}

						last_i = cur_i;
						last_j = cur_j;
					}
				}


				// Note: if the two are identical, array stays empty:
				m_bEditScriptValid = TRUE;
			}

			// Deallocate Memory:
			delete[] T;
			delete[] Rvisitmin;
			delete[] Rvisitmax;
			for (i=0; i<dmax; i++) delete[] (Rm[i]);
			delete[] Rm;
		}
		break;
	}

	return nRetVal;
}

BOOL GetLastEditScript(CStringArray &anArray)
{
	int i;

	anArray.RemoveAll();
	if (!m_bEditScriptValid) return FALSE;

	anArray.SetSize(m_EditScript.GetSize());
	for (i=0; i<m_EditScript.GetSize(); i++) anArray.SetAt(i, m_EditScript.GetAt(i));

	return TRUE;
}

CString DiffFunctions(FUNC_COMPARE_METHOD nMethod,
									CFuncDescFile *pFile1, int nFile1FuncNdx,
									CFuncDescFile *pFile2, int nFile2FuncNdx,
									DWORD nOutputOptions,
									double &nMatchPercent,
									CSymbolMap *pSymbolMap)
{
	CString strRetVal = "";
	CString strEntry;
	CString strTemp;
	CStringArray oes;
	CStringArray Func1Lines;
	CStringArray Func2Lines;
	CFuncDesc *pFunction1;
	CFuncDesc *pFunction2;
	CFuncObject *pFuncObj;
	int i;
	int nLeftMax;
	int nRightMax;
	int nLeftIndex;
	int nRightIndex;
	int nLeftPos;
	int nRightPos;
	int nPos;

	if ((pFile1 == NULL) || (pFile2 == NULL)) return strRetVal;

	pFunction1 = pFile1->GetFunc(nFile1FuncNdx);
	pFunction2 = pFile2->GetFunc(nFile2FuncNdx);
	if ((pFunction1 == NULL) ||
		(pFunction2 == NULL)) return strRetVal;

	nMatchPercent = CompareFunctions(nMethod, pFile1, nFile1FuncNdx, pFile2, nFile2FuncNdx, TRUE);
	if (!GetLastEditScript(oes)) return strRetVal;

/*
// The following is for special debugging only:
	for (i=0; i<oes.GetSize(); i++) {
		strRetVal += "    " + oes.GetAt(i) + "\n";
	}
//	return strRetVal;
	strRetVal += "\n";
*/

	nLeftMax = 0;
	for (i=0; i<pFunction1->GetSize(); i++) {
		pFuncObj = pFunction1->GetAt(i);
		if (pFuncObj == NULL) {
			Func1Lines.Add("");
			continue;
		}
		strTemp = pFuncObj->CreateOutputLine(nOutputOptions);
		nLeftMax = __max(nLeftMax, strTemp.GetLength());
		Func1Lines.Add(strTemp);
	}

	nRightMax = 0;
	for (i=0; i<pFunction2->GetSize(); i++) {
		pFuncObj = pFunction2->GetAt(i);
		if (pFuncObj == NULL) {
			Func2Lines.Add("");
			continue;
		}
		strTemp = pFuncObj->CreateOutputLine(nOutputOptions);
		nRightMax = __max(nRightMax, strTemp.GetLength());
		Func2Lines.Add(strTemp);
	}

	strTemp = "";
	for (i=0; i<((nLeftMax-pFunction1->GetMainName().GetLength())/2); i++) strTemp += ' ';
	strRetVal += PadString(strTemp + pFunction1->GetMainName(), nLeftMax);

	strRetVal += "      ";

	strTemp = "";
	for (i=0; i<((nRightMax-pFunction2->GetMainName().GetLength())/2); i++) strTemp += ' ';
	strRetVal += PadString(strTemp + pFunction2->GetMainName(), nRightMax);

	strRetVal += "\n";

	for (i=0; i<nLeftMax; i++) strRetVal += '-';
	strRetVal += "      ";
	for (i=0; i<nRightMax; i++) strRetVal += '-';
	strRetVal += "\n";

	nLeftPos = 0;
	nRightPos = 0;
	for (i=0; i<oes.GetSize(); i++) {
		strEntry = oes.GetAt(i);
		nPos = strEntry.FindOneOf("<->");
		if (nPos == -1) continue;
		nLeftIndex = strtoul(strEntry.Left(nPos), NULL, 0);
		nRightIndex = strtoul(strEntry.Mid(nPos+1), NULL, 0);
		for (; ((nLeftPos < nLeftIndex) && (nRightPos < nRightIndex)); nLeftPos++, nRightPos++) {
			strRetVal += PadString(Func1Lines.GetAt(nLeftPos), nLeftMax);
			if (pFunction1->GetAt(nLeftPos)->IsExactMatch(pFunction2->GetAt(nRightPos))) {
				strRetVal += "  ==  ";
			} else {
				strRetVal += "  --  ";
			}
			strRetVal += Func2Lines.GetAt(nRightPos) + "\n";
			if (pSymbolMap) pSymbolMap->AddObjectMapping(*pFunction1->GetAt(nLeftPos), *pFunction2->GetAt(nRightPos));
		}
		ASSERT(nLeftPos == nLeftIndex);
		ASSERT(nRightPos == nRightIndex);

		#ifdef _DEBUG
		// The following is for special debugging only:
		if (nLeftPos != nLeftIndex) {
			strTemp.Format("\n\n*** ERROR: nLeftPos = %ld,  Expected = %ld\n\n", nLeftPos, nLeftIndex);
			strRetVal += strTemp;
			nLeftPos = nLeftIndex;
		}

		if (nRightPos != nRightIndex) {
			strTemp.Format("\n\n*** ERROR: nRightPos = %ld,  Expected = %ld\n\n", nRightPos, nRightIndex);
			strRetVal += strTemp;
			nRightPos = nRightIndex;
		}
		#endif

		#ifdef _DEBUG
		switch (strEntry.GetAt(nPos)) {
			case '<':
				if (nRightPos >= Func2Lines.GetSize()) {
					strRetVal += "\n\n*** ERROR: Right-Side Index Out-Of-Range!\n\n";
					return strRetVal;
				}
				break;
			case '-':
				if (nLeftPos >= Func1Lines.GetSize()) {
					strRetVal += "\n\n*** ERROR: Left-Side Index Out-Of-Range!\n\n";
					if (nRightPos >= Func2Lines.GetSize()) {
						strRetVal += "\n\n*** ERROR: Right-Side Index Out-Of-Range!\n\n";
					}
					return strRetVal;
				}
				if (nRightPos >= Func2Lines.GetSize()) {
					strRetVal += "\n\n*** ERROR: Right-Side Index Out-Of-Range!\n\n";
					return strRetVal;
				}
				break;
			case '>':
				if (nLeftPos >= Func1Lines.GetSize()) {
					strRetVal += "\n\n*** ERROR: Left-Side Index Out-Of-Range!\n\n";
					return strRetVal;
				}
				break;
		}
		#endif

		switch (strEntry.GetAt(nPos)) {
			case '<':				// Insert in Left
				strRetVal += PadString("", nLeftMax) + "  <<  " + Func2Lines.GetAt(nRightPos) + "\n";
				nRightPos++;
				break;

			case '-':				// Change
				strRetVal += PadString(Func1Lines.GetAt(nLeftPos), nLeftMax) + "  ->  " + Func2Lines.GetAt(nRightPos) + "\n";
				if (pSymbolMap) pSymbolMap->AddObjectMapping(*pFunction1->GetAt(nLeftPos), *pFunction2->GetAt(nRightPos));
				nLeftPos++;
				nRightPos++;
				break;

			case '>':				// Insert in Right
				strRetVal += PadString(Func1Lines.GetAt(nLeftPos), nLeftMax) + "  >>  \n";
				nLeftPos++;
				break;
		}
	}

	nLeftIndex = Func1Lines.GetSize();
	nRightIndex = Func2Lines.GetSize();
	for (; ((nLeftPos < nLeftIndex) && (nRightPos < nRightIndex)); nLeftPos++, nRightPos++) {
		strRetVal += PadString(Func1Lines.GetAt(nLeftPos), nLeftMax);
		if (pFunction1->GetAt(nLeftPos)->IsExactMatch(pFunction2->GetAt(nRightPos))) {
			strRetVal += "  ==  ";
		} else {
			strRetVal += "  --  ";
		}
		strRetVal += Func2Lines.GetAt(nRightPos) + "\n";
		if (pSymbolMap) pSymbolMap->AddObjectMapping(*pFunction1->GetAt(nLeftPos), *pFunction2->GetAt(nRightPos));
	}

	return strRetVal;
}

