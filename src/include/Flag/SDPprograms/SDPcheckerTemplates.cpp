#ifndef SDPCHECKERTEMPLATES_CPP_INCLUDED
#define SDPCHECKERTEMPLATES_CPP_INCLUDED

/* SDPcheckerProgram(int argc, char **argv):
 * This program reads a solution written in the standard of SDPsolverProgram
 * and checks its validity
 */
template<class UTO>
int SDPcheckerProgram(int argc, char **argv) {
	using namespace std;

	bool printCoeffsMode = false;

	/* Options parsing block */ {
		int ind1 = 1, ind2 = 0;
		char opt;
		const char* arg = "";
		while (true) {
			if (Text::getoptions("c", ind1, ind2, opt, arg, argc, argv)) {
				switch (opt) {
				case 'c':
					printCoeffsMode = true;
					break;
				case '\0':
					cerr << "Invalid option: " << *arg
						 << "\nTry `" << argv[0] << " --help' for more information.\n";
					return 1;
				}
			}
			else if (ind1 < argc && !strcmp(argv[ind1] + ind2, "--help")) {
				cerr << "Usage: " << argv[0] << " [-c]\n"
					 << "Run flag algebra SDP solution checker.\n\n"
					 << "  -c  print objects and coefficients of final expansion\n"
					 << "      aftwerwards\n"
					 << "\n";
				return 0;
			}
			else 
				break;
		}
	}


	string filename;
	cerr << "Type file name: ";
	cin >> filename;

	FILE *file;
	while ((file = fopen(filename.c_str(), "r")) == 0) {
		cerr << "Error: Unable to open file " << filename << ", type another name.\n";
		cin >> filename;
	}

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

		container<BigNum::frac> finalCoeffs(finalUTOsEnum.size());
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
					finalCoeffs[expEnum[objIndex][j].first]
						+= expEnum[objIndex][j].second;
		}

		if (matrixform) {
			cerr << "Reading matrix format solution.\n";
			vector< vector< vector<BigNum::frac> > > M;
			M.reserve(typesUsed);
			for (unsigned long i = 0; i < flagsUsed.size(); ++i)
				M.push_back(vector< vector<BigNum::frac> >
							(flagsUsed[i],
							 vector<BigNum::frac>(flagsUsed[i])));

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

			cerr << "Testing if matrix is positive semidefinite.\n";
			bool isPSD = true;
			for (unsigned long b = 0; b < flagsUsed.size(); ++b) {
				if (!LinearAlgebra::isPositiveSemidefinite(M[b])) {
					cout << "Block number " << b+1
						 << " of matrix is not positive semidefinite\n";
					isPSD = false;
				}
			}
			if (isPSD)
				cout << "Matrix is positive semidefinite.\n";

			cerr << "Computing final value.\n\n";
			for (unsigned long b = 0; b < M.size(); ++b) {

				container< Flag<UTO> > enumFlag(enumerateFlags(flagSizes[b], types[b]));
				vector<unsigned long> indv(flags[b].size());
				// Computing flag indexes
				for (unsigned long i = 0; i < indv.size(); ++i) {
					for ( ;
						  /** The below shouldn't be needed, so it is commented out
						  indv[i] < enumFlag.size()
						  **/
						  ;
						  ++indv[i])
						if (flags[b][i] == enumFlag[indv[i]])
							break;
				}

				morphism g;
				const container< container< container<coeff_type> > >& enumProd
					(expandFlagProductEnumerate(types[b].object(),
												flagSizes[b],
												flagSizes[b],
												expSize,
												g));
				const container< Flag<UTO> >& expFlagEnum
					(enumerateFlags(expSize, types[b].object(), g));
				for (unsigned long i = 0; i < M[b].size(); ++i) {
					for (unsigned long j = 0; j < M[b][i].size(); ++j) {
						const container<coeff_type>& expanded(enumProd[indv[i]][indv[j]]);
						for (unsigned long k = 0; k < expanded.size(); ++k) {
							const Flag<UTO>& expFlag(expFlagEnum[expanded[k].first]);
							if (allowed[expFlag.objectIndex()]) {
								if (min)
									finalCoeffs[expFlag.objectIndex()]
										-= M[b][i][j]
										* expanded[k].second
										* expFlag.normFactor()
										* BigNum::frac(rescalingFactors[b]);
								else
									finalCoeffs[expFlag.objectIndex()]
										+= M[b][i][j]
										* expanded[k].second
										* expFlag.normFactor()
										* BigNum::frac(rescalingFactors[b]);
							}
						}
					}
				}
			}
		}
		else /* Squares format */ {
			cerr << "Reading squares format solution.\n";
			vector< vector< vector<BigNum::frac> > > M;
			vector< vector<BigNum::frac> > EVals;
			M.reserve(typesUsed);
			EVals.reserve(typesUsed);
			for (unsigned long i = 0; i < flagsUsed.size(); ++i) {
				M.push_back(vector< vector<BigNum::frac> >
							(flagsUsed[i],
							 vector<BigNum::frac>(flagsUsed[i])));
				EVals.push_back(vector<BigNum::frac>(flagsUsed[i]));
			}

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
					if (!block || block-- > M.size() ||
						!indi || indi-- > M[block].size() ||
						indj-- > M[block][indi].size() ||
						BigNum::fscanfrac(file, &M[block][indi][indj]) != 1)
						throw badFormat();
				}
			}
			if (fscanfret != EOF)
				throw badFormat();
			cerr << "Finished reading input.\n\n";

			cerr << "Testing if matrix is positive semidefinite.\n";
			bool isPSD = true;
			for (unsigned long b = 0; b < EVals.size(); ++b) {
				unsigned long i = 0;
				for ( ; i < EVals[b].size(); ++i) {
					if (EVals[b][i].negative()) {
						cout << "Block number " << b+1
							 << " of matrix is not positive semidefinite\n"
							 << "(has eigenvalue " << EVals[b][i] << ")\n";
						isPSD = false;
					}
				}
				if (i < EVals[b].size())
					break;
			}
			if (isPSD)
				cout << "Matrix is positive semidefinite.\n";


			cerr << "Computing final value.\n\n";
			for (unsigned long b = 0; b < M.size(); ++b) {
				morphism g;

				container< Flag<UTO> > enumFlag(enumerateFlags(flagSizes[b], types[b]));
				vector<unsigned long> indv(flags[b].size());
				// Computing flag indexes
				for (unsigned long i = 0; i < indv.size(); ++i) {
					for ( ;
						  /** The below shouldn't be needed, so it is commented out
						  indv[i] < enumFlag.size()
						  **/
						  ;
						  ++indv[i])
						if (flags[b][i] == enumFlag[indv[i]])
							break;
				}

				const container< container< container<coeff_type> > >& enumProd
					(expandFlagProductEnumerate(types[b].object(),
												flagSizes[b],
												flagSizes[b],
												expSize,
												g));
				const container< Flag<UTO> >& expFlagEnum
					(enumerateFlags(expSize, types[b].object(), g));
				for (unsigned long j1 = 0; j1 < flagsUsed[b]; ++j1) {
					for (unsigned long j2 = 0; j2 < flagsUsed[b]; ++j2) {
						const container<coeff_type>& expanded
							(enumProd[indv[j1]][indv[j2]]);
						for (unsigned long k = 0; k < expanded.size(); ++k) {
							const Flag<UTO>& expFlag(expFlagEnum[expanded[k].first]);
							if (allowed[expFlag.objectIndex()]) {
								for (unsigned long i = 0; i < M[b].size(); ++i) {
									if (min)
										finalCoeffs[expFlag.objectIndex()]
											-= EVals[b][i]
											* M[b][i][j1]
											* M[b][i][j2]
											* expanded[k].second
											* expFlag.normFactor()
											* BigNum::frac(rescalingFactors[b]);
									else
										finalCoeffs[expFlag.objectIndex()]
											+= EVals[b][i]
											* M[b][i][j1]
											* M[b][i][j2]
											* expanded[k].second
											* expFlag.normFactor()
											* BigNum::frac(rescalingFactors[b]);
								}
							}
						}
					}
				}
			}
		}

		// Calculating minimum or maximum coefficient (depending on min)
		unsigned long valueIndex = 0;
		for (valueIndex = 0; valueIndex < finalCoeffs.size(); ++valueIndex)
			if (allowed[valueIndex])
				break;
		if (valueIndex == finalCoeffs.size()) {
			cerr << "Warning: every object in final expansion size "
				 << "is forbidden.\n";
			cout << "Value certified: 0 / 1\n"
				 << "Conjectured: " << conjectured
				 << "\nDistance: " << conjectured
				 << "\nClaimed: " << obtained
				 << "\nDistance: " << obtained
				 << "\nPrecision: " << precision
				 << "\n\nApproximated values:\n"
				 << "Value certified: 0\n"
				 << "Conjectured: ";
			conjectured.putOnStreamPoint(cout, fracPrecision)
				<< "\nDistance: ";
			conjectured.putOnStreamPoint(cout, fracPrecision)
				<< "\nClaimed: ";
			obtained.putOnStreamPoint(cout, fracPrecision)
				<< "\nDistance: ";
			obtained.putOnStreamPoint(cout, fracPrecision)
				<< "\nPrecision: ";
			precision.putOnStreamPoint(cout, fracPrecision)
				<< "\n";
		}
		else {
			stringstream coeffsSS;
			if (printCoeffsMode) {
				cout << "Final expansion objects:\n";
				printobj(finalUTOsEnum[valueIndex]);
				cout << endl;
				coeffsSS << finalCoeffs[valueIndex] << "\n";
			}
			for (unsigned long i = valueIndex+1; i < finalCoeffs.size(); ++i) {
				if (allowed[i]) {
					if (printCoeffsMode) {
						printobj(finalUTOsEnum[i]);
						cout << endl;
						coeffsSS << finalCoeffs[i] << "\n";
					}
					if ((finalCoeffs[i] < finalCoeffs[valueIndex]) == min)
						valueIndex = i;
				}
			}
			if (printCoeffsMode)
				cout << endl << "Coefficients:\n" << coeffsSS.str() << endl;
			BigNum::frac certified(finalCoeffs[valueIndex]),
				distConj(conjectured - certified),
				distClaimed(obtained - certified);
			cout << "Value certified: " << certified
				 << "\nConjectured: " << conjectured
				 << "\nDistance: " << distConj
				 << "\nClaimed: " << obtained
				 << "\nDistance: " << distClaimed
				 << "\nPrecision: " << precision
				 << "\n\nApproximated values:\n"
				 << "Value certified: ";
			certified.putOnStreamPoint(cout, fracPrecision)
				<< "\nConjectured: ";
			conjectured.putOnStreamPoint(cout, fracPrecision)
				<< "\nDistance: ";
			distConj.putOnStreamPoint(cout, fracPrecision)
				<< "\nClaimed: ";
			obtained.putOnStreamPoint(cout, fracPrecision)
				<< "\nDistance: ";
			distClaimed.putOnStreamPoint(cout, fracPrecision)
				<< "\nPrecision: ";
			precision.putOnStreamPoint(cout, fracPrecision)
				<< "\n";
		}
	}
	catch (badFormat bf) {
		cerr << "Error: badly formatted file.\n";
	}
	fclose(file);
	return 0;
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
