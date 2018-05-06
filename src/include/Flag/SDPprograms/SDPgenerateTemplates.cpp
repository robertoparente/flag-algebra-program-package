#ifndef SDPGENERATETEMPLATES_CPP_INCLUDED
#define SDPGENERATETEMPLATES_CPP_INCLUDED

/* SDPgenerateProgram(int argc, char **argv):
 * This is a template program to generate an CSDP input of a
 * SDP problem associated to minimization or maximization of
 * a linear combination of UTOs over a class of forbidden subobjects
 */
template<class UTO>
int SDPgenerateProgram(int argc, char **argv) {
	using namespace std;

	bool runSolverMode = false;
	bool makeSolvedBaseMode = false;
	enum Solvers solverUsed = CSDP;

	BigNum::bignum scale(BigNum::bn0);

	/* Options parsing block */ {
		int ind1 = 1, ind2 = 0;
		char opt;
		const char* arg = "";
		while (true) {
			if (Text::getoptions("pr:s:", ind1, ind2, opt, arg, argc, argv)) {
				stringstream ss(arg);
				unsigned long auxUL = 0;
				switch (opt) {
				case 'p':
					makeSolvedBaseMode = true;
					break;
				case 'r':
					ss >> scale;
					break;
				case 's':
					runSolverMode = true;
					ss >> auxUL;
					if (SolversSize <= auxUL) {
						cerr << "Invalid solver size: " << auxUL
							 << "\nTry `" << argv[0] << " --help for more information.\n";
						return 1;
					}
					solverUsed = static_cast<Solvers>(auxUL);
					break;
				case ':':
					cerr << "Missing argument for option: " << *arg
						 << "\nTry `" << argv[0] << " --help' for more information.\n";
					return 1;
				case '\0':
					cerr << "Invalid option: " << *arg
						 << "\nTry `" << argv[0] << " --help' for more information.\n";
					return 1;
				}
			}
			else if (ind1 < argc && !strcmp(argv[ind1] + ind2, "--help")) {
				cerr << "Usage: " << argv[0] << " [options]\n"
					 << "Generate flag algebra SDP problem.\n\n"
					 << "Valid options:\n"
					 << "  -p    print final solution base file too\n"
					 << "  -rN\n"
					 << "  -r N  rescale problem such that the entries are an integer\n"
					 << "          multiple of N.\n"
					 << "          N = 0 means don't rescale;\n"
					 << "          without -r N option, the program uses N = 0.\n"
					 << "          (Rescaling factor used may differ from N and\n"
					 << "          will be printed on screen.)\n"
					 << "  -sN\n"
					 << "  -s N  run solver number N in generated problem and generate\n"
					 << "          solution files from solvers solution.\n"
					 << SolversHelp
					 << "\n";
				return 0;
			}
			else
				break;
		}
	}


	bool again;
	do {
		bool min;
		cerr << "Type of problem.\nMaximization: 0; Minimization: 1\n";
		cin >> min;

		unsigned long nCount;
		cerr << "Number of subobjects to be counted: ";
		cin >> nCount;

		unsigned long countSize = 0;
		container<coeff_type> countCoeffs;

		container<UTO> countUTOs(nCount);
		std::vector<BigNum::frac> coeffsRead(nCount);
		/* Counted subobjects block */ {
			for (unsigned long i = 0; i < nCount; ++i) {
				cerr << "Subobject to be counted:\n";
				scanobj(&countUTOs[i]);
				if (countUTOs[i].size() > countSize)
					countSize = countUTOs[i].size();
				cerr << "Coefficient: ";
				cin >> coeffsRead[i];
			}

			for (unsigned long i = 0; i < nCount; ++i) {
				if (coeffsRead[i].zero())
					continue;

				const container<UTO> objEnum(enumerateObjects<UTO>(countUTOs[i].size()));
				const container< container<coeff_type> >& expEnum
					(expandObjectEnumerate<UTO>(countUTOs[i].size(), countSize));

				unsigned long objIndex = 0;
				for ( ;
					  /** The below shouldn't be needed, so it is commented out
					  objIndex < objEnum.size()
					  **/
					  ;
					  ++objIndex)
					if (countUTOs[i] == objEnum[objIndex])
						break;
				for (unsigned long j = 0; j < expEnum[objIndex].size(); ++j) {
					container<coeff_type>::iterator it(lower_bound(countCoeffs.begin(),
																   countCoeffs.end(),
																   expEnum[objIndex][j],
																   compFirstCoeffType()));
					if (expEnum[objIndex][j].first == it->first) {
						it->second += coeffsRead[i] * expEnum[objIndex][j].second;
					}
					else {
						it = countCoeffs.insert(it, expEnum[objIndex][j]);
						it->second *= coeffsRead[i];
					}
				}
			}
		}

		unsigned long nForb;
		cerr << "Number of subobjects to be forbidden: ";
		cin >> nForb;

		unsigned long forbSize = 0;
		vbool forbSel;

		container<UTO> forbUTOs(nForb);
		/* Forbidden subobjects block */ {
			for (unsigned long i = 0; i < nForb; ++i) {
				cerr << "Subobject to forbid:\n";
				scanobj(&forbUTOs[i]);
				if (forbUTOs[i].size() > forbSize)
					forbSize = forbUTOs[i].size();
			}

			forbSel.resize(enumerateObjects<UTO>(forbSize).size());
			for (unsigned long i = 0; i < nForb; ++i) {
				const container<UTO> objEnum(enumerateObjects<UTO>(forbUTOs[i].size()));
				const container< container<coeff_type> >& expEnum
					(expandObjectEnumerate<UTO>(forbUTOs[i].size(), forbSize));

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
					forbSel[expEnum[objIndex][j].first] = true;
			}
		}

		unsigned long expSize;
		cerr << "Size of final objects of expansion: ";
		cin >> expSize;

		unsigned long nTypes;
		cerr << "# of types to be used: ";
		cin >> nTypes;

        container<UTO> uTypes(nTypes);
        cerr << "Types (unlabeled) to be used:\n";
        for (unsigned long i = 0; i < nTypes; ++i)
            scanobj(&uTypes[i]);

		container<unsigned long> flagSizes(uTypes.size());
		container< Flag<UTO> > normTypes(uTypes.size());
		container<vbool> flagSelect(uTypes.size());
		container< const container< Flag<UTO> >* > flagEnums(uTypes.size());
		for (unsigned long i = 0; i < uTypes.size(); ++i) {
			morphism g;
			flagSizes[i] = (expSize - uTypes[i].size()) / 2 + uTypes[i].size();
			flagEnums[i] = &enumerateFlags(flagSizes[i], uTypes[i], g);
			normTypes[i] = Flag<UTO>(uTypes[i], g);
			flagSelect[i].resize(flagEnums[i]->size(), true);
		}

		/* flagSelect forbidden object removal block */ {
			for (unsigned long i = 0; i < forbSel.size(); ++i) {
				if (forbSel[i]) {
					for (unsigned long j = 0; j < flagSelect.size(); ++j) {
						if (forbSize <= flagSizes[j]) {
							const container< container<coeff_type> >& forbExp
								(expandObjectEnumerate<UTO>(forbSize, flagSizes[j]));
							for (unsigned long k = 0; k < flagSelect[j].size(); ++k) {
								if (flagSelect[j][k]) {
									for (unsigned long l = 0; l < forbExp.size(); ++l) {
										if ((*flagEnums[j])[k].objectIndex()
											== forbExp[i][l].first) {
											flagSelect[j][k] = false;
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}

        string gen_name;
        cerr << "Generated problem output file name (with extension):\n";
        cin >> gen_name;

		string prefix;
		BigNum::frac conjectured, precision;
		if (runSolverMode || makeSolvedBaseMode) {
			cerr << "Auxiliary files prefix: ";
			cin >> prefix;
			cerr << "Conjectured value: ";
			cin >> conjectured;
			cerr << "Precision: ";
			cin >> precision;
		}

        FILE *f;
        while ((f = fopen(gen_name.c_str(), "w")) == NULL) {
            cerr << "Error: Unable to open file " << gen_name << ", type another name.\n";
            cin >> gen_name;
        }
		container<BigNum::bignum> rescalingFactors
			(fprintSDPproblem(f,
							  min,
							  countSize,
							  countCoeffs,
							  forbSize,
							  forbSel,
							  expSize,
							  uTypes,
							  flagSizes,
							  flagSelect,
							  scale,
							  Monitoring::ProgressBarFrac(progressBarSize)));
		fclose(f);
		cerr << "\nSDP problem successfully written in " << gen_name << ".\n"
			 << "Global rescaling factor: " << rescalingFactors.back() << "\n\n";

		unsigned long typesUsed = 0;
		container<unsigned long> flagsUsed(uTypes.size());
		for (unsigned long i = 0; i < uTypes.size(); ++i) {
			for (unsigned long j = 0; j < flagSelect[i].size(); ++j)
				if (flagSelect[i][j])
					++flagsUsed[i];
			if (flagsUsed[i])
				++typesUsed;
		}

		if (makeSolvedBaseMode) {
			const string final_solution_base_name(prefix + final_solution_base_suffix
													+ extension);
			const BigNum::frac obtained(min? BigNum::bn0 : BigNum::bn1);

			FILE *file;
			if ((file = fopen(final_solution_base_name.c_str(), "w")) == 0) {
				cerr << "Error: unable to open file "
					 << final_solution_base_name << " in w mode.\n";
				return 0;
			}
			fprintf(file, "Solution base\n");
			fprintf(file, "%d\n%lu\n", (int) min, nCount);
			for (unsigned long i = 0; i < nCount; ++i) {
				fprintobj(file, countUTOs[i]);
				fprintf(file, "\n%s\n", coeffsRead[i].toStringDec().c_str());
			}
			fprintf(file, "%lu\n", nForb);
			for (unsigned long i = 0; i < nForb; ++i) {
				fprintobj(file, forbUTOs[i]);
				fprintf(file, "\n");
			}
			fprintf(file, "%s\n%s\n%s\n%lu\n",
					conjectured.toStringDec().c_str(),
					precision.toStringDec().c_str(),
					obtained.toStringDec().c_str(),
					expSize);

			fprintf(file, "%lu\n", typesUsed);
			for (unsigned long i = 0; i < normTypes.size(); ++i) {
				if (flagsUsed[i]) {
					fprintflag(file, normTypes[i]);
					fprintf(file, "\n");
				}
			}
			for (unsigned long i = 0; i < normTypes.size(); ++i) {
				if (flagsUsed[i]) {
					fprintf(file, "%lu\n", flagsUsed[i]);
					for (unsigned long j = 0;
						 j < flagSelect[i].size();
						 ++j) {
						if (flagSelect[i][j]) {
							fprintflag(file, (*flagEnums[i])[j]);
							fprintf(file, "\n");
						}
					}
				}
			}
			for (unsigned long i = 0; i < rescalingFactors.size()-1; ++i) {
				BigNum::fprintbignum(file, rescalingFactors[i]);
				fprintf(file, " ");
			}
			fprintf(file, "\n");
			fclose(file);
		}

		if (runSolverMode) {
			const string solver_solution_name(prefix + solver_solution_suffix
											  + extension),
				command_line(solver[solverUsed] + " " + gen_name + " "
							 + outputOption[solverUsed] + solver_solution_name),
				final_solution_name(prefix + final_solution_suffix + extension),
				final_solution_squares_name(prefix + final_solution_squares_suffix
											+ extension);
			enum failureOptions {rerun, accept, reject, terminate};
			int failureChoice;

			/* Solver block */ {
				cerr << "\nRunning solver.\n";
				failureChoice = accept;
				if (int ret = system(command_line.c_str())) {
					do {
						cerr << "Solver returned code " << ret;
						ret = (ret < 0 || SolverReturnSize[solverUsed] < ret)?
									 SolverUnknown[solverUsed] : ret;
						cerr << ":\n"
							 << SolverReturnStrings[solverUsed][ret-1]
							 << "\nWhat do you want me to do?\n"
							 << "0 = Run solver again,\n"
							 << "1 = Accept solution,\n"
							 << "2 = Reject solution,\n"
							 << "3 = Terminate program.\n";
						cin >> failureChoice;
						if (failureChoice != rerun)
							break;
					} while ((ret = system(command_line.c_str())));
					if (failureChoice == terminate)
						return 0;
				}
			}
			if (failureChoice != reject) {
				BigNum::frac obtained;
				/* Solver solution reading block */ {
					FILE *file;
					if ((file = fopen(solver_solution_name.c_str(), "r")) == 0) {
						cerr << "Error: unable to open file " << solver_solution_name
							 << " in r mode.\n";
						return 0;
					}

					int d;
					switch (solverUsed) {
					case CSDP:
						/* Skip slacknesses	*/
						if (fscanf(file, "%*[^\n]") != 0) {
							cerr << "Error: file " << solver_solution_name
								 << " corrupted.\n";
							return 0;
						}
						while (true) {
							unsigned long dual, block, i, j;
							if (fscanf(file,
									   "%lu %lu %lu %lu",
									   &dual,
									   &block,
									   &i,
									   &j) != 4) {
								cerr << "Error: file " << solver_solution_name
									 << " corrupted.\n";
								return 0;
							}
							if (dual == 2 && block == typesUsed+1 && i == 1 && j == 1) {
								if (BigNum::fscanfrac(file, &obtained) != 1) {
									cerr << "Error: file " << solver_solution_name
										 << " corrupted.\n";
									return 0;
								}
								obtained /= BigNum::frac(rescalingFactors.back());
								cerr << "Primal objective rescaled absolute value: "
									 << obtained.toStringPoint(fracPrecision) << "\n";
								break;
							}
							else if (fscanf(file, "%*[^\n]") != 0) {
								cerr << "Error: file " << solver_solution_name
									 << " corrupted.\n";
								return 0;
							}
						}
						break;
					case SDPA:
						do {
							d = -1;
							fscanf(file, "%*[^y]yMat = {%n", &d);
							if (feof(file)) {
								cerr << "Error: file " << solver_solution_name
									 << " corrupted.\n";
								return 0;
							}
						} while (d < 0);
						for (unsigned long i = 0; i < typesUsed; ++i) {
							if (fscanf(file, " {") != 0) {
								cerr << "Error: file " << solver_solution_name
									 << " corrupted.\n";
								return 0;
							}
							unsigned long depth = 1;
							while (true) {
								int c = fgetc(file);
								if (c == '{') {
									++depth;
								}
								else if (c == '}') {
									if (--depth == 0)
										break;
								}
								else if (c == EOF) {
									cerr << "Error: file " << solver_solution_name
										 << " corrupted.\n";
									return 0;
								}
							}
						}
						if (fscanf(file, " {") != 0) {
							cerr << "Error: file " << solver_solution_name
								 << " corrupted.\n";
							return 0;
						}
						if (BigNum::fscanfrac(file, &obtained) != 1) {
							cerr << "Error: file " << solver_solution_name
								 << " corrupted.\n";
							return 0;
						}
						obtained /= BigNum::frac(rescalingFactors.back());
						cerr << "Primal objective rescaled absolute value: "
							 << obtained.toStringPoint(fracPrecision) << "\n";
						if (fscanf(file, " }") != 0) {
							cerr << "Error: file " << solver_solution_name
								 << " corrupted.\n";
							return 0;
						}
						break;
					default:
						break;
					}
					fclose(file);
				}

				/* Final solution writing block */ {
					FILE *file;
					if ((file = fopen(final_solution_name.c_str(), "w")) == 0) {
						cerr << "Error: unable to open file "
							 << final_solution_name << " in w mode.\n";
						return 0;
					}
					fprintf(file, "Matrix format solution\n");
					fprintf(file, "%d\n%lu\n", (int) min, nCount);
					for (unsigned long i = 0; i < nCount; ++i) {
						fprintobj(file, countUTOs[i]);
						fprintf(file, "\n%s\n", coeffsRead[i].toStringDec().c_str());
					}
					fprintf(file, "%lu\n", nForb);
					for (unsigned long i = 0; i < nForb; ++i) {
						fprintobj(file, forbUTOs[i]);
						fprintf(file, "\n");
					}
					fprintf(file, "%s\n%s\n%s\n%lu\n",
							conjectured.toStringDec().c_str(),
							precision.toStringDec().c_str(),
							obtained.toStringDec().c_str(),
							expSize);

					fprintf(file, "%lu\n", typesUsed);
					for (unsigned long i = 0; i < normTypes.size(); ++i) {
						if (flagsUsed[i]) {
							fprintflag(file, normTypes[i]);
							fprintf(file, "\n");
						}
					}
					for (unsigned long i = 0; i < normTypes.size(); ++i) {
						if (flagsUsed[i]) {
							fprintf(file, "%lu\n", flagsUsed[i]);
							for (unsigned long j = 0;
								 j < flagSelect[i].size();
								 ++j) {
								if (flagSelect[i][j]) {
									fprintflag(file, (*flagEnums[i])[j]);
									fprintf(file, "\n");
								}
							}
						}
					}
					for (unsigned long i = 0; i < rescalingFactors.size()-1; ++i) {
						BigNum::fprintbignum(file, rescalingFactors[i]);
						fprintf(file, " ");
					}
					fprintf(file, "\n");


#ifdef LAPACKE_AVAILABLE
					/* variables to be used with lapacke.h*/
					vector< vector<double> > Mat;
					vector< vector<double> > EVs;
					vector<lapack_int> lapackSizes;
					Mat.reserve(typesUsed);
					EVs.reserve(typesUsed);
					lapackSizes.reserve(typesUsed);
					for (unsigned long i = 0;
						 i < flagsUsed.size();
						 ++i) {
						if (flagsUsed[i]) {
							Mat.push_back
								(vector<double>(flagsUsed[i]*flagsUsed[i]));
							EVs.push_back(vector<double>(flagsUsed[i]));
							lapackSizes.push_back(flagsUsed[i]);
						}
					}
#endif
					FILE *infile;
					if ((infile =
						 fopen(solver_solution_name.c_str(), "r")) == 0) {
						cerr << "Error: unable to open file "
							 << solver_solution_name << " in r mode.\n";
						return 0;
					}

					int d;
					switch (solverUsed) {
					case CSDP:
						/* Skip dual problem y variable values */
						if (fscanf(infile, "%*[^\n]") != 0) {
							cerr << "Error: file " << solver_solution_name
								 << " corrupted.\n";
							return 0;
						}
						while (true) {
							unsigned long dual, block, i, j;
							BigNum::frac value;
							if (fscanf(infile, "%lu %lu %lu %lu", &dual, &block, &i, &j)
								+ BigNum::fscanfrac(infile, &value) != 5)
								break;
							if (dual == 2 && block <= typesUsed) {
								fprintf(file, "%lu %lu %lu ", block, i, j);
								BigNum::fprintfrac(file, value);
								fprintf(file, "\n");

#ifdef LAPACKE_AVAILABLE
								/* store the value for use with lapacke.h */
								Mat[block-1][(i-1) * lapackSizes[block-1] + (j-1)]
									= Mat[block-1][(j-1) * lapackSizes[block-1] + (i-1)]
									= BigNum::fracToDouble(value);
#endif
							}
						}
						break;
					case SDPA:
						do {
							d = -1;
							fscanf(file, "%*[^y]yMat = {%n", &d);
							if (feof(file)) {
								cerr << "Error: file " << solver_solution_name
									 << " corrupted.\n";
								return 0;
							}
						} while (d < 0);
						for (unsigned long block = 0; block < typesUsed; ++block) {
							BigNum::frac value;
							if (fscanf(infile, " {") != 0) {
								cerr << "Error: file " << solver_solution_name
									 << " corrupted.\n";
								return 0;
							}
							for (unsigned long i = 0; i < flagsUsed[block]; ++i) {
								if (fscanf(infile, " {") != 0) {
									cerr << "Error: file " << solver_solution_name
										 << " corrupted.\n";
									return 0;
								}
								for (unsigned long j = 0; j < flagsUsed[block]; ++j) {
									if (fscanfrac(infile, &value) != 1) {
										cerr << "Error: file " << solver_solution_name
											 << " corrupted.\n";
										return 0;
									}
									if (i <= j) {
										fprintf(file, "%lu %lu %lu ", block+1, i+1, j+1);
										BigNum::fprintfrac(file, value);
										fprintf(file, "\n");
									}

#ifdef LAPACKE_AVAILABLE
									/* store the value for use with lapacke.h */
									Mat[block][i * lapackSizes[block] + j]
										= BigNum::fracToDouble(value);
#endif

									if ((j < flagsUsed[block]-1)
										&& fscanf(infile, " ,") != 0) {
										cerr << "Error: file " << solver_solution_name
											 << " corrupted.\n";
										return 0;
									}
								}
								if (fscanf(infile, " }") != 0
									|| ((i < flagsUsed[block]-1)
										&& fscanf(infile, " ,") != 0)) {
									cerr << "Error: file " << solver_solution_name
										 << " corrupted.\n";
									return 0;
								}
							}
						}
						break;
					default:
						break;
					}
					fclose(infile);
					fclose(file);

#ifdef LAPACKE_AVAILABLE
					/* this part uses lapacke.h to compute and write
					 * the square format of the matrix
					 */
					for (unsigned long i = 0; i < Mat.size(); ++i) {
						if (lapack_int info =
							LAPACKE_dsyev(LAPACK_COL_MAJOR,
										  /* Matrix is columns x lines */
										  'V',	/* Eigenvalues/eigenvectors */
										  'U',	/* Upper triangle matrix */
										  lapackSizes[i], /* Matrix order */
										  &Mat[i][0], /* Matrix */
										  lapackSizes[i], /* Lead dimension */
										  &EVs[i][0] /* Eigenvalues output */
										  )) {
							cerr << "Error in LAPACKE_dsyev(): "
								 << info << endl;
							return 0;
						}
					}

					if ((file = fopen(final_solution_squares_name.c_str(), "w")) == 0) {
						cerr << "Error: unable to open file "
							 << final_solution_squares_name << " in w mode.\n";
						return 0;
					}
					fprintf(file, "Squares format solution\n");
					fprintf(file, "%d\n%lu\n", (int) min, nCount);
					for (unsigned long i = 0; i < nCount; ++i) {
						fprintobj(file, countUTOs[i]);
						fprintf(file, "\n%s\n", coeffsRead[i].toStringDec().c_str());
					}
					fprintf(file, "%lu\n", nForb);
					for (unsigned long i = 0; i < nForb; ++i) {
						fprintobj(file, forbUTOs[i]);
						fprintf(file, "\n");
					}
					fprintf(file, "%s\n%s\n%s\n%lu\n",
							conjectured.toStringDec().c_str(),
							precision.toStringDec().c_str(),
							obtained.toStringDec().c_str(),
							expSize);
					fprintf(file, "%lu\n", typesUsed);
					for (unsigned long i = 0; i < normTypes.size(); ++i) {
						if (flagsUsed[i]) {
							fprintflag(file, normTypes[i]);
							fprintf(file, "\n");
						}
					}
					for (unsigned long i = 0; i < normTypes.size(); ++i) {
						if (flagsUsed[i]) {
							fprintf(file, "%lu\n", flagsUsed[i]);
							for (unsigned long j = 0;
								 j < flagSelect[i].size();
								 ++j) {
								if (flagSelect[i][j]) {
									fprintflag(file, (*flagEnums[i])[j]);
									fprintf(file, "\n");
								}
							}
						}
					}
					for (unsigned long i = 0; i < rescalingFactors.size()-1; ++i) {
						BigNum::fprintbignum(file, rescalingFactors[i]);
						fprintf(file, " ");
					}
					fprintf(file, "\n");

					for (unsigned long i = 0; i < Mat.size(); ++i) {
						for (lapack_int j = 0; j < lapackSizes[i]; ++j) {
							fprintf(file,
									"%lu %lu 0 %.18e\n",
									i+1,
									static_cast<unsigned long>(j+1),
									EVs[i][j]);
							for (lapack_int k = 0; k < lapackSizes[i]; ++k) {
								fprintf(file,
										"%lu %lu %lu %.18e\n",
										i+1,
										static_cast<unsigned long>(j+1),
										static_cast<unsigned long>(k+1),
										Mat[i][j * lapackSizes[i] + k]);
							}
						}
					}
					fclose(file);
#endif
				}
			}
		}
		cerr << "Print another problem? (1 = yes, 0 = no)\n";
		cin >> again;
	} while (again);
	return 0;
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
