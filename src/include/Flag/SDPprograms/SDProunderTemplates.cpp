#ifndef SDPROUNDERTEMPLATES_CPP_INCLUDED
#define SDPROUNDERTEMPLATES_CPP_INCLUDED

/* SDProunderProgram(int argc, char **argv):
 * This program reads a solution written in the standard of SDPsolverProgram
 * and tries to round it to a rational solution
 */
template<class UTO>
int SDProunderProgram(int argc, char **argv) {
	using namespace std;

	bool autoMode = false;
	bool recomputeEigen = false;

	/* Options parsing block */ {
		int ind1 = 1, ind2 = 0;
		char opt;
		const char* arg = "";
		while (true) {
			if (Text::getoptions("ae", ind1, ind2, opt, arg, argc, argv)) {
				switch (opt) {
				case 'a':
					autoMode = true;
					break;
				case 'e':
					recomputeEigen = true;
#ifndef LAPACKE_AVAILABLE
					cerr << "Error: Since library lapacke was not included "
						"in compilation,\n"
						 << "       this program cannot recompute eigenvalues"
						" (ignoring option -e).\n";
					recomputeEigen = false;
#endif
						break;
				case '\0':
					cerr << "Invalid option: " << *arg
						 << "\nTry `" << argv[0] << " --help' for more information.\n";
					return 1;
				}
			}
			else if (ind1 < argc && !strcmp(argv[ind1] + ind2, "--help")) {
				cerr << "Usage: " << argv[0] << " [-a]\n"
					 << "Run flag algebra SDP solution rounder.\n\n"
					 << "  -a  run in automatic mode (rounder will make the decisions\n"
					 << "        on parameter change if it fails to round)\n"
					 << "  -e  recompute eigenvalues if solution is in squares format\n"
					 << "\n";
				return 0;
			}
			else 
				break;
		}
	}


	string inputname;
	cerr << "Type input file name: ";
	cin >> inputname;

	FILE *file;
	while ((file = fopen(inputname.c_str(), "r")) == 0) {
		cerr << "Error: Unable to open file " << inputname << ", type another name.\n";
		cin >> inputname;
	}

	string outputname;
	cerr << "Type output file name: ";
	cin >> outputname;

	char firstline[128];
	if (fscanf(file, "%127[^\n]", firstline) != 1) {
		cerr << "Error: badly formatted first line.\n";
		fclose(file);
		return 0;
	}

	bool matrixform;
	if (!strcmp(firstline, "Matrix format solution")) {
		matrixform = true;
		cerr << "File is in matrix format.\n";
#ifndef LAPACKE_AVAILABLE
		cerr << "Error: Since library lapacke was not included in compilation\n"
			 << "       matrix format solutions aren't accepted as input.\n";
		fclose(file);
		return 0;
#endif
	}
	else if (!strcmp(firstline, "Squares format solution")) {
		matrixform = false;
		cerr << "File is in squares format.\n";
	}
	else {
		cerr << "Error: badly formatted first line.\n";
		fclose(file);
		return 0;
	}
	// Local exception class
	class badFormat {};
	try {
		int min;
		unsigned long nCount;
		unsigned long minExpSize = 0;
		if (fscanf(file, "%d %lu", &min, &nCount) != 2 || (min != 0 && min != 1)) {
			throw badFormat();
		}
		container<UTO> countUTOs(nCount);
		vector<BigNum::frac> coeffsRead(nCount);
		for (unsigned long i = 0; i < nCount; ++i) {
			if (fscanobj(file, &countUTOs[i]) != 1)
				throw badFormat();
			if (countUTOs[i].size() > minExpSize)
				minExpSize = countUTOs[i].size();
			if (BigNum::fscanfrac(file, &coeffsRead[i]) != 1)
				throw badFormat();
		}

		unsigned long nForb;
		if (fscanf(file, "%lu", &nForb) != 1)
			throw badFormat();
		container<UTO> forbUTOs(nForb);
		for (unsigned long i = 0; i < nForb; ++i) {
			if (fscanobj(file, &forbUTOs[i]) != 1)
				throw badFormat();
			if (forbUTOs[i].size() > minExpSize)
				minExpSize = forbUTOs[i].size();
		}
		BigNum::frac conjectured, precision, obtained;
		if (BigNum::fscanfrac(file, &conjectured) != 1)
			throw badFormat();
		if (BigNum::fscanfrac(file, &precision) != 1)
			throw badFormat();
		if (BigNum::fscanfrac(file, &obtained) != 1)
			throw badFormat();

		unsigned long expSize, typesUsed;
		if (fscanf(file, "%lu %lu", &expSize, &typesUsed) != 2)
			throw badFormat();

		container< Flag<UTO> > types(typesUsed);
		for (unsigned long i = 0; i < typesUsed; ++i)
			if (fscanflag(file, &types[i]) != 1)
				throw badFormat();

		container<unsigned long> flagsUsed(typesUsed);
		container< container< Flag<UTO> > > flags(typesUsed);
		container<unsigned long> flagSizes(typesUsed);
		for (unsigned long i = 0; i < typesUsed; ++i) {
			if (fscanf(file, "%lu", &flagsUsed[i]) != 1)
				throw badFormat();
			flags[i].resize(flagsUsed[i]);
			for (unsigned long j = 0; j < flagsUsed[i]; ++j) {
				if (fscanflag(file, &flags[i][j]) != 1)
					throw badFormat();
				if (!j)
					flagSizes[i] = flags[i][j].size();
				else if (flagSizes[i] != flags[i][j].size()
						 || flags[i][j].type() != types[i])
					throw badFormat();
			}
			unsigned long prodSiz(2*flagSizes[i] - types[i].size());
			if (prodSiz > minExpSize)
				minExpSize = prodSiz;
		}
		if (expSize < minExpSize)
			throw badFormat();

		container<BigNum::bignum> rescalingFactors(typesUsed);
		for (unsigned long i = 0; i < rescalingFactors.size(); ++i)
			if (BigNum::fscanbignum(file, &rescalingFactors[i]) != 1
				|| !rescalingFactors[i].positive())
				throw badFormat();


		cerr << "Finished reading problem.\n";


		const container<UTO>& finalUTOsEnum(enumerateObjects<UTO>(expSize));
		vbool allowed(finalUTOsEnum.size(), true);
		cerr << "Computing allowed final UTOs.\n";
		for (unsigned long i = 0; i < forbUTOs.size(); ++i) {
			const container< container<coeff_type> >& expEnum
				(expandObjectEnumerate<UTO>(forbUTOs[i].size(), expSize));
			const container<UTO>& objEnum(enumerateObjects<UTO>(forbUTOs[i].size()));

			unsigned long objIndex = 0;
			for ( ;
				  /** The below shouldn't be needed, so it is commented out
					  objIndex < objEnum.size()
				  **/
				  ;
				  ++objIndex)
				if (forbUTOs[i] == objEnum[objIndex])
					break;
			for (unsigned long j = 0; j < expEnum[objIndex].size(); ++j)
				/** The below is redundant, so it is commented out
				if (!expEnum[objIndex][j].second.zero())
				**/
				allowed[expEnum[objIndex][j].first] = false;
		}

		container<BigNum::frac> countCoeffs(finalUTOsEnum.size());
		cerr << "Computing counted UTOs expansion coefficients.\n";
		for (unsigned long i = 0; i < countUTOs.size(); ++i) {
			const container< container<coeff_type> >& expEnum
				(expandObjectEnumerate<UTO>(countUTOs[i].size(), expSize));
			const container<UTO>& objEnum(enumerateObjects<UTO>(countUTOs[i].size()));

			unsigned long objIndex = 0;
			for ( ;
				  /** The below shouldn't be needed, so it is commented out
					  objIndex < objEnum.size()
				  **/
				  ;
				  ++objIndex)
				if (countUTOs[i] == objEnum[objIndex])
					break;
			for (unsigned long j = 0; j < expEnum[objIndex].size(); ++j)
				if (allowed[expEnum[objIndex][j].first])
					countCoeffs[expEnum[objIndex][j].first]
						+= expEnum[objIndex][j].second;
		}


		vector< vector< vector<BigNum::frac> > > M;
		vector< vector<BigNum::frac> > EVals;
		vector< vector< vector<BigNum::frac> > > EVecs;

		M.reserve(typesUsed);
		EVals.reserve(typesUsed);
		EVecs.reserve(typesUsed);
		for (unsigned long i = 0; i < flagsUsed.size(); ++i) {
			M.push_back(vector< vector<BigNum::frac> >
						(flagsUsed[i],
						 vector<BigNum::frac>(flagsUsed[i])));
			EVals.push_back(vector<BigNum::frac>(flagsUsed[i]));
			EVecs.push_back(vector< vector<BigNum::frac> >
							(flagsUsed[i],
							 vector<BigNum::frac>(flagsUsed[i])));
		}

		if (matrixform) {
#ifdef LAPACKE_AVAILABLE
			cerr << "Reading matrix format solution.\n";

			int fscanfret;
			unsigned long block, indi, indj;
			while ((fscanfret = fscanf(file, "%lu %lu %lu", &block, &indi, &indj))
				   == 3) {
				if (!block || block-- > M.size() ||
					!indi || indi-- > M[block].size() ||
					!indj || indj-- > M[block][indi].size() ||
					BigNum::fscanfrac(file, &M[block][indi][indj]) != 1)
					throw badFormat();
				M[block][indj][indi] = M[block][indi][indj];
			}
			if (fscanfret != EOF)
				throw badFormat();
			cerr << "Finished reading input.\n\n";

			cerr << "Computing matrix eigenvectors.\n";
			for (unsigned long i = 0; i < flagsUsed.size(); ++i) {
				vector<double> Mat(flagsUsed[i]*flagsUsed[i]), Eigenvalues(flagsUsed[i]);
				
				for (unsigned long j = 0; j < flagsUsed[i]; ++j)
					for (unsigned long k = 0; k < flagsUsed[i]; ++k)
						Mat[j*flagsUsed[i] + k] = fracToDouble(M[i][j][k]);

				if (lapack_int info =
					LAPACKE_dsyev(LAPACK_COL_MAJOR, // Matrix is repr. columns x lines
								  'V', // Compute eigenvalues and eigenvectors
								  'U', // Upper triangle of the matrix is stored
								  static_cast<lapack_int>(flagsUsed[i]),
								  // Order of the matrix
								  &Mat[0], // Matrix
								  static_cast<lapack_int>(flagsUsed[i]),
								  // Lead dimension of the matrix
								  &Eigenvalues[0] // Output of the eigenvalues
								  )) {
					cerr << "Error in LAPACKE_dsyev(): " << info << endl;
					fclose(file);
					return 0;
				}

				for (unsigned long j = 0; j < flagsUsed[i]; ++j) {
					for (unsigned long k = 0; k < flagsUsed[i]; ++k)
						EVecs[i][j][k] = BigNum::doubleToFrac(Mat[j*flagsUsed[i] + k]);
					EVals[i][j] = BigNum::doubleToFrac(Eigenvalues[j]);
				}
		    }
#endif
		}
		else /* Squares format */ {
			cerr << "Reading squares format solution.\n";

			int fscanfret;
			unsigned long block, indi, indj;
			while ((fscanfret = fscanf(file, "%lu %lu %lu", &block, &indi, &indj)) == 3) {
				if (!indj) {
					if (!block || block-- > EVals.size() ||
						!indi || indi-- > EVals[block].size() ||
						BigNum::fscanfrac(file, &EVals[block][indi]) != 1)
						throw badFormat();
				}
				else {
					if (!block || block-- > EVecs.size() ||
						!indi || indi-- > EVecs[block].size() ||
						indj-- > EVecs[block][indi].size() ||
						BigNum::fscanfrac(file, &EVecs[block][indi][indj]) != 1)
						throw badFormat();
				}
			}
			if (fscanfret != EOF)
				throw badFormat();
			cerr << "Finished reading input.\n\n";

			cerr << "Computing matrix.\n";
			for (unsigned long b = 0; b < EVecs.size(); ++b) {
				for (unsigned long i = 0; i < EVals[b].size(); ++i) {
					for (unsigned long j1 = 0; j1 < EVecs[b].size(); ++j1) {
						for (unsigned long j2 = 0; j2 < EVecs[b].size(); ++j2) {
							M[b][j1][j2]
								+= EVals[b][i]
								* EVecs[b][i][j1]
								* EVecs[b][i][j2];
						}
					}
				}
			}

#ifdef LAPACKE_AVAILABLE
			if (recomputeEigen) {
				cerr << "Computing matrix eigenvectors.\n";
				for (unsigned long i = 0; i < flagsUsed.size(); ++i) {
					vector<double> Mat(flagsUsed[i]*flagsUsed[i]),
						               Eigenvalues(flagsUsed[i]);
				
					for (unsigned long j = 0; j < flagsUsed[i]; ++j)
						for (unsigned long k = 0; k < flagsUsed[i]; ++k)
							Mat[j*flagsUsed[i] + k] = fracToDouble(M[i][j][k]);

					if (lapack_int info =
						LAPACKE_dsyev(LAPACK_COL_MAJOR, // Matrix is repr. columns x lines
									  'V', // Compute eigenvalues and eigenvectors
									  'U', // Upper triangle of the matrix is stored
									  static_cast<lapack_int>(flagsUsed[i]),
									  // Order of the matrix
									  &Mat[0], // Matrix
									  static_cast<lapack_int>(flagsUsed[i]),
									  // Lead dimension of the matrix
									  &Eigenvalues[0] // Output of the eigenvalues
									  )) {
						cerr << "Error in LAPACKE_dsyev(): " << info << endl;
						fclose(file);
						return 0;
					}

					for (unsigned long j = 0; j < flagsUsed[i]; ++j) {
						for (unsigned long k = 0; k < flagsUsed[i]; ++k)
							EVecs[i][j][k] =
								BigNum::doubleToFrac(Mat[j*flagsUsed[i] + k]);
						EVals[i][j] = BigNum::doubleToFrac(Eigenvalues[j]);
					}
				}
			}
#endif
		}
		fclose(file);

		cerr << "Finished input related tasks.\n\n";

		vector<MRef> indexToMRef;
		vector< vector< vector<unsigned long> > > MRefToIndex(M.size());

		for (unsigned long i = 0; i < M.size(); ++i) {
			MRefToIndex[i].resize(M[i].size(), vector<unsigned long>(M[i].size()));
			for (unsigned long j = 0; j < M[i].size(); ++j) {
				for (unsigned long k = j; k < M[i].size(); ++k) {
					MRefToIndex[i][j][k] = MRefToIndex[i][k][j] = indexToMRef.size();
					indexToMRef.push_back(MRef(i, j, k));
				}
			}
		}

		container<BigNum::frac> finalCoeffsApprox(countCoeffs);

		vector< map<unsigned long, BigNum::frac> > restrictions
			(finalUTOsEnum.size()
			 + accumulate(flagsUsed.begin(), flagsUsed.end(), 0, sumSquare()));

		for (unsigned long i = 0; i < finalUTOsEnum.size(); ++i)
			restrictions[i][indexToMRef.size()] = countCoeffs[i] - conjectured;

		// The rest of the restrictions will have coefficient 0

		// Computing indexes, expansion enumerators and product enumerators
		vector< vector<unsigned long> > indv(M.size());
		container< const container< container< container<coeff_type> > >* > enumProd
			(M.size());
		container< const container< Flag<UTO> >* > expFlagEnum(M.size());
		for (unsigned long b = 0; b < M.size(); ++b) {
			// Computing flag indexes
			container< Flag<UTO> > enumFlag(enumerateFlags(flagSizes[b], types[b]));
			indv[b].assign(flags[b].size(), 0);
			for (unsigned long i = 0; i < indv[b].size(); ++i) {
				for ( ;
					  /** The below shouldn't be needed, so it is commented out
						  indv[b][i] < enumFlag.size()
					  **/
					  ;
					  ++indv[b][i])
					if (flags[b][i] == enumFlag[indv[b][i]])
						break;
			}

			morphism g;
			// Computing expansion enumerator
			enumProd[b] = &expandFlagProductEnumerate(types[b].object(),
													  flagSizes[b],
													  flagSizes[b],
													  expSize,
													  g);
			// Computing product enumerator
			expFlagEnum[b] = &enumerateFlags(expSize, types[b].object(), g);
		}

		cerr << "Computing final values.\n\n";
		for (unsigned long b = 0; b < M.size(); ++b) {
			for (unsigned long i = 0; i < M[b].size(); ++i) {
				for (unsigned long j = 0; j < M[b][i].size(); ++j) {
					const container<coeff_type>& expanded
						((*enumProd[b])[indv[b][i]][indv[b][j]]);
					for (unsigned long k = 0; k < expanded.size(); ++k) {
						const Flag<UTO>& expFlag((*expFlagEnum[b])[expanded[k].first]);
						if (allowed[expFlag.objectIndex()]) {
							BigNum::frac factor(expanded[k].second
												* expFlag.normFactor()
												* BigNum::frac(rescalingFactors[b]));
							if (min) {
								finalCoeffsApprox[expFlag.objectIndex()]
									-= M[b][i][j] * factor;
								restrictions[expFlag.objectIndex()][MRefToIndex[b][i][j]]
									+= factor;
							}
							else {
								finalCoeffsApprox[expFlag.objectIndex()]
									+= M[b][i][j] * factor;
								restrictions[expFlag.objectIndex()][MRefToIndex[b][i][j]]
									-= factor;
							}
						}
					}
				}
			}
		}

		unsigned long nRoundEigen;
		BigNum::frac epsilonPos, epsilonConj, epsilonRoundMat,
			prevEpsilonPos, prevEpsilonConj;
		vbool select(restrictions.size());

		if (autoMode) {
			cerr << "Type initial epsilon 1 (precision to positive semidefinite"
				" restrictions):\n";
			cin >> epsilonPos;
			cerr << "Type initial epsilon 2 (precision to conjectured value"
				" obtaining):\n";
			cin >> epsilonConj;
			prevEpsilonPos = epsilonPos;
			prevEpsilonConj = epsilonConj;
			cerr << "Type epsilon 3 (precision for rounding of matrix values):\n";
			cin >> epsilonRoundMat;
			cerr << "Type nRoundEigen (maximum denominator for eigenvector rounding):\n";
			cin >> nRoundEigen;
		}

		/* This macro writes the rounded solution in matrix format
		 * (in file with name matFileName)
		 */
#define writeRoundedSolution(matFileName)								\
		do {															\
			const char* const& matName(matFileName);					\
			FILE *file;													\
			if ((file = fopen(matName, "w")) == 0) {					\
				cerr << "Error: unable to open file "					\
					 << matName << " in w mode.\n";						\
				return 0;												\
			}															\
			fprintf(file, "Matrix format solution\n");					\
			fprintf(file, "%d\n%lu\n", min, nCount);					\
			for (unsigned long i = 0; i < nCount; ++i) {				\
				fprintobj(file, countUTOs[i]);							\
				fprintf(file,											\
						"\n%s\n",										\
						coeffsRead[i].toStringDec().c_str());			\
			}															\
			fprintf(file, "%lu\n", nForb);								\
			for (unsigned long i = 0; i < nForb; ++i) {					\
				fprintobj(file, forbUTOs[i]);							\
				fprintf(file, "\n");									\
			}															\
			fprintf(file, "%s\n%s\n%s\n%lu\n",							\
					conjectured.toStringDec().c_str(),					\
					precision.toStringDec().c_str(),					\
					roundedObtained.toStringDec().c_str(),				\
					expSize);											\
																		\
			fprintf(file, "%lu\n", typesUsed);							\
			for (unsigned long i = 0; i < types.size(); ++i) {			\
				if (flagsUsed[i]) {										\
					fprintflag(file, types[i]);							\
					fprintf(file, "\n");								\
				}														\
			}															\
			for (unsigned long i = 0; i < typesUsed; ++i) {				\
				fprintf(file, "%lu\n", flagsUsed[i]);					\
				for (unsigned long j = 0; j < flagsUsed[i]; ++j) {		\
					fprintflag(file, flags[i][j]);						\
					fprintf(file, "\n");								\
				}														\
			}															\
			for (unsigned long i = 0;									\
				 i < rescalingFactors.size();							\
				 ++i) {													\
				/* The rescaling factors will be incorporated 			\
				 * into the matrix										\
				 */														\
				BigNum::fprintbignum(file, BigNum::bn1);				\
				fprintf(file, "\n");									\
			}															\
																		\
			for (unsigned long i = 0; i < indexToMRef.size(); ++i) {	\
				const MRef& mr(indexToMRef[i]);							\
				fprintf(file,											\
						"%lu %lu %lu %s\n",								\
						mr.block+1,										\
						mr.indi+1,										\
						mr.indj+1,										\
						(roundedValues[i]								\
						 * BigNum::frac(rescalingFactors[mr.block]))	\
						.toStringDec().c_str());						\
			}															\
			fclose(file);												\
		} while (false)


		do {
			if (!autoMode) {
				cerr << "Type epsilon 1 (precision to positive semidefinite"
					" restrictions):\n";
				cin >> epsilonPos;
				cerr << "Type epsilon 2 (precision to conjectured value obtaining):\n";
				cin >> epsilonConj;
				cerr << "Type epsilon 3 (precision for rounding of matrix values):\n";
				cin >> epsilonRoundMat;
				cerr << "Type nRoundEigen (maximum denominator for eigenvector"
					" rounding):\n";
				cin >> nRoundEigen;
			}

			cerr << "Starting rounding attempt with parameters:\n"
				 << "epsilon1 = " << epsilonPos << "\n"
				 << "epsilon2 = " << epsilonConj << "\n"
				 << "epsilon3 = " << epsilonRoundMat << "\n"
				 << "nRoundEigen = " << nRoundEigen << "\n\n";

			select.assign(select.size(), false);

			cerr << "Determining round-to-tight conjecture restrictions.\n";
			for (unsigned long i = 0; i < finalCoeffsApprox.size(); ++i) {
				if (min) {
					if (finalCoeffsApprox[i] < conjectured + epsilonConj)
						select[i] = true;
				}
				else {
					if (finalCoeffsApprox[i] > conjectured - epsilonConj)
						select[i] = true;
				}
			}

			cerr << "Determining round-to-zero eigenvalues.\n";
			for (unsigned long b = 0, baseInd = finalCoeffsApprox.size();
				 b < EVals.size();
				 baseInd += flagsUsed[b] * flagsUsed[b], ++b) {
				for (unsigned long i = 0; i < EVals[b].size(); ++i) {
					if (EVals[b][i] < epsilonPos) {
						unsigned long iInd(baseInd + i*flagsUsed[b]);
						for (unsigned long j = 0; j < flagsUsed[b]; ++j)
							select[iInd+j] = true;
					}
				}
			}

			// Reseting positive semidefinite related restrictions
			for (unsigned long i = finalUTOsEnum.size(); i < restrictions.size(); ++i)
				restrictions[i].clear();


			for (unsigned long b = 0, baseInd = finalCoeffsApprox.size();
				 b < EVecs.size();
				 baseInd += flagsUsed[b] * flagsUsed[b], ++b) {
				vector< vector<BigNum::frac> > NormEVecs(EVecs[b]);

				for (unsigned long i = 0; i < flagsUsed[b]; ++i) {
					if (select[baseInd + i*flagsUsed[b]]) {
						vector<BigNum::frac>::iterator
							min(min_element(NormEVecs[i].begin(), NormEVecs[i].end())),
							max(max_element(NormEVecs[i].begin(), NormEVecs[i].end()));
						
						unsigned long absMaxInd((-(*min) > *max? min : max)
												- NormEVecs[i].begin());

						for (unsigned long j = 0; j < NormEVecs[i].size(); ++j)
							if (j != absMaxInd)
								NormEVecs[i][j] /= NormEVecs[i][absMaxInd];
						NormEVecs[i][absMaxInd] = BigNum::frac(BigNum::bn1);

						for (unsigned long oi = 0; oi < flagsUsed[b]; ++oi) {
							if (oi != i && select[baseInd + oi*flagsUsed[b]]) {
								for (unsigned long j = 0; j < NormEVecs[oi].size(); ++j)
									if (j != absMaxInd)
										NormEVecs[oi][j] -= NormEVecs[i][j]
											* NormEVecs[oi][absMaxInd];
								NormEVecs[oi][absMaxInd] = BigNum::frac(BigNum::bn0);
							}
						}
					}
				}

				for (unsigned long i = 0; i < flagsUsed[b]; ++i) {
					unsigned long iInd(baseInd + i*flagsUsed[b]);
					if (select[iInd]) {
						// Rounding zero-eigenvectors entries
						for (unsigned long j = 0; j < NormEVecs[i].size(); ++j)
							NormEVecs[i][j] = NormEVecs[i][j].humanRounded(nRoundEigen);

						/* Writing zero-eigenvectors restrictions
						 * Note: this has to be done every iteration since the
						 *       linear space spanned by the zero-eigenvectors may
						 *       change with epsilonPos and their entries may change
						 *       with nRoundEigen
						 */
						for (unsigned long j = 0; j < NormEVecs[i].size(); ++j)
							for (unsigned long k = 0; k < NormEVecs[i].size(); ++k)
								restrictions[iInd+j][MRefToIndex[b][j][k]]
									= NormEVecs[i][k];
					}
				}
			}

			// A copy must be made since solveLinearSystem will alter its argument
			vector< map<unsigned long, BigNum::frac> > restCopy(restrictions);
			Selector::selector<vector< map<unsigned long, BigNum::frac> >::iterator>
				linearSystem(restCopy.begin(), restCopy.end(), select);

			/* The only potential throws that are treatable in the try-block below
			 * are due to solveSparseLinearSystem(const selector::iterator&,
			 *                                    const selector::iterator&,
			 *                                    const Monitoring::ProgressKeeper&)
			 */
			try {
				vector<unsigned long> freeVars
					(LinearAlgebra
					 ::solveSparseLinearSystem(linearSystem.begin(),
											   linearSystem.end(),
											   indexToMRef.size(),
											   Monitoring
											   ::ProgressBarFrac(progressBarSize)));
				vector<BigNum::frac> roundedValues(indexToMRef.size());

				for (unsigned long i = 0; i < freeVars.size(); ++i) {
					const MRef& mr(indexToMRef[freeVars[i]]);
					roundedValues[freeVars[i]]
						= M[mr.block][mr.indi][mr.indj].rounded(epsilonRoundMat);
				}
				LinearAlgebra::applySparseSolution(linearSystem.begin(),
												   linearSystem.end(),
												   roundedValues,
												   Monitoring
												   ::ProgressBarFrac(progressBarSize));

				// Checking if produced solution is good
				bool isPos = true, conjGood = true;
				/* Positive semidefinite check */ {
					unsigned long ind = 0;
					for (unsigned long b = 0; b < M.size(); ++b) {
						vector< vector<BigNum::frac> > RoundedBlock
							(M[b].size(),
							 vector<BigNum::frac>(M[b].size()));
						for ( ; 
							  ind < indexToMRef.size() && indexToMRef[ind].block == b;
							  ++ind) {
							const MRef& mr(indexToMRef[ind]);
							RoundedBlock[mr.indi][mr.indj]
								= RoundedBlock[mr.indj][mr.indi]
								= roundedValues[ind];
						}
						if (!LinearAlgebra::isPositiveSemidefinite(RoundedBlock)) {
							isPos = false;
							break;
						}
					}
				}
				BigNum::frac roundedObtained;
				/* Conjectured value check */ {
					container<BigNum::frac> finalCoeffsRounded(countCoeffs);

					for (unsigned long b = 0; b < M.size(); ++b) {
						morphism g;
						for (unsigned long i = 0; i < M[b].size(); ++i) {
							for (unsigned long j = 0; j < M[b][i].size(); ++j) {
								const container<coeff_type>& expanded
									((*enumProd[b])[indv[b][i]][indv[b][j]]);
								for (unsigned long k = 0; k < expanded.size(); ++k) {
									const Flag<UTO>& expFlag
										((*expFlagEnum[b])[expanded[k].first]);
									if (allowed[expFlag.objectIndex()]) {
										if (min) {
											finalCoeffsRounded[expFlag.objectIndex()]
												-= roundedValues[MRefToIndex[b][i][j]]
												* expanded[k].second
												* expFlag.normFactor()
												* BigNum::frac(rescalingFactors[b]);
										}
										else {
											finalCoeffsRounded[expFlag.objectIndex()]
												+= roundedValues[MRefToIndex[b][i][j]]
												* expanded[k].second
												* expFlag.normFactor()
												* BigNum::frac(rescalingFactors[b]);
										}
									}
								}
							}
						}
					}
					// Calculating minimum or maximum coefficient (depending on min)
					unsigned long valueIndex = 0;
					for (valueIndex = 0;
						 valueIndex < finalCoeffsRounded.size();
						 ++valueIndex)
						if (allowed[valueIndex])
							break;
					if (valueIndex == finalCoeffsRounded.size()) {
						if (min) {
							roundedObtained = BigNum::frac(BigNum::bn0);
							conjGood = (roundedObtained >= conjectured);
						}
						else {
							roundedObtained = BigNum::frac(BigNum::bn1);
							conjGood = (roundedObtained <= conjectured);
						}
					}
					else {
						if (min) {
							for (unsigned long i = valueIndex+1;
								 i < finalCoeffsRounded.size();
								 ++i)
								if (allowed[i])
									if (finalCoeffsRounded[i]
										< finalCoeffsRounded[valueIndex])
										valueIndex = i;
							roundedObtained = finalCoeffsRounded[valueIndex];
							conjGood = (roundedObtained >= conjectured);
						}
						else {
							for (unsigned long i = valueIndex+1;
								 i < finalCoeffsRounded.size();
								 ++i)
								if (allowed[i])
									if (finalCoeffsRounded[i]
										> finalCoeffsRounded[valueIndex])
										valueIndex = i;
							roundedObtained = finalCoeffsRounded[valueIndex];
							conjGood = (roundedObtained <= conjectured);
						}

					}
				}

				cerr << "\n";
				if (isPos) {
					if (conjGood) {
						cerr << "Rounding successfully completed.\n"
							 << "Printing rounded solution.\n";
						writeRoundedSolution(outputname.c_str());
						return 0;
					}
					cerr << "Parameters passed resulted in a valid solution,\n"
						 << "but without as good value as conjectured one.\n"
						 << (autoMode? "" :
							 "Possible causes: epsilon 2 is too small;\n"
							 "                 epsilon 3 is too big;\n"
							 "                 nRoundEigen is too small;\n"
							 "                 the input has solution value far worse"
							 " than conjectured value.\n")
						 << "Printing rounded solution.\n";
					writeRoundedSolution(outputname.c_str());
					if (autoMode) {
						swap(epsilonConj, prevEpsilonConj);
						epsilonConj = (prevEpsilonConj < epsilonConj)?
							(epsilonConj + prevEpsilonConj) / BigNum::frac(BigNum::bn2) :
							prevEpsilonConj * BigNum::frac(BigNum::bn2);
					}
				}
				else {
					if (conjGood) {
						cerr << "Parameters passed resulted in an invalid solution\n"
							 << "(i.e., the rounded matrix is not positive"
							" semidefinite),\n"
							 << "but the conjectured value was obtained.\n"
							 << (autoMode? "" :
								 "Possible causes: epsilon 1 is too small;\n"
								 "                 epsilon 3 is too big;\n"
								 "                 nRoundEigen is too small;\n"
								 "                 the input has invalid solution.\n");
						if (autoMode) {
							swap(epsilonPos, prevEpsilonPos);
							epsilonPos = (prevEpsilonPos < epsilonPos)?
								(epsilonPos + prevEpsilonPos)
								/ BigNum::frac(BigNum::bn2) :
								prevEpsilonPos * BigNum::frac(BigNum::bn2);
						}
					}
					else {
						cerr << "Parameters passed resulted in a completely"
							" invalid solution\n"
							 << "(i.e., the rounded matrix is not positive semidefinite\n"
							 << "and the conjectured value was not obtained).\n"
							 << (autoMode? "" :
								 "Possible causes: epsilon 1 and/or epsilon 2 are"
								 " too small;\n"
								 "                 epsilon 3 is too big;\n"
								 "                 nRoundEigen is too small;\n"
								 "                 the input has solution value far worse"
								 " than conjectured value;\n"
								 "                 the input has invalid solution.\n");
						if (autoMode) {
							swap(epsilonPos, prevEpsilonPos);
							epsilonPos = (prevEpsilonPos < epsilonPos)?
								(epsilonPos + prevEpsilonPos)
								/ BigNum::frac(BigNum::bn2) :
								prevEpsilonPos * BigNum::frac(BigNum::bn2);
							swap(epsilonConj, prevEpsilonConj);
							epsilonConj = (prevEpsilonConj < epsilonConj)?
								(epsilonConj + prevEpsilonConj)
								/ BigNum::frac(BigNum::bn2) :
								prevEpsilonConj * BigNum::frac(BigNum::bn2);
						}
					}
				}
			}
			catch (LinearAlgebra::noSolution ns) {
				cerr << "Parameters passed resulted in linear system without solution.\n"
					 << (autoMode? "" :
						 "Possible causes: epsilon 1 and/or epsilon 2 are too big;\n"
						 "                 nRoundEigen is too small;\n"
						 "                 the input has solution value far worse than"
						 " conjectured value;\n"
						 "                 the input has an invalid solution.\n");
				if (autoMode) {
					swap(epsilonPos, prevEpsilonPos);
					epsilonPos = (prevEpsilonPos > epsilonPos)?
						(epsilonPos + prevEpsilonPos) / BigNum::frac(BigNum::bn2) :
						prevEpsilonPos / BigNum::frac(BigNum::bn2);
					swap(epsilonConj, prevEpsilonConj);
					epsilonConj = (prevEpsilonConj > epsilonConj)?
						(epsilonConj + prevEpsilonConj) / BigNum::frac(BigNum::bn2) :
						prevEpsilonConj / BigNum::frac(BigNum::bn2);
				}
			}
			catch (LinearAlgebra::badSystem bs) {
				cerr << "Parameters passed resulted in no restrictions"
					" on the linear system.\n"
					 << (autoMode? "" :
						 "Possible causes: epsilon 1 and epsilon 2 are too small;\n"
						 "                 nRoundEigen is too small;\n"
						 "                 the input has solution value far better than"
						 " conjectured value.\n");
				if (autoMode) {
					swap(epsilonPos, prevEpsilonPos);
					epsilonPos = (prevEpsilonPos < epsilonPos)?
						(epsilonPos + prevEpsilonPos) / BigNum::frac(BigNum::bn2) :
						prevEpsilonPos * BigNum::frac(BigNum::bn2);
					swap(epsilonConj, prevEpsilonConj);
					epsilonConj = (prevEpsilonConj < epsilonConj)?
						(epsilonConj + prevEpsilonConj) / BigNum::frac(BigNum::bn2) :
						prevEpsilonConj * BigNum::frac(BigNum::bn2);
				}
			}
			if (!autoMode) {
				cerr << "Do you want to type new values for rounding parameters?\n"
					 << "(1 = yes, 0 = no)\n";
				bool b;
				cin >> b;
				if (!b)
					return 0;
			}
		} while (true);
	}
	catch (badFormat bf) {
		cerr << "Error: badly formatted input file.\n";
		fclose(file);
		return 0;
	}

#undef writeRoundedSolution
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
