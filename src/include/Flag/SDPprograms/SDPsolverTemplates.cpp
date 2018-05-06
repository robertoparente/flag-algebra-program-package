#ifndef SDPSOLVERTEMPLATES_CPP_INCLUDED
#define SDPSOLVERTEMPLATES_CPP_INCLUDED

/* SDPsolverProgram(int argc, char **argv):
 * This is a template program to generate an solver input of a
 * SDP problem associated to minimization or maximization of
 * a linear combination of UTOs over a class of forbidden subobjects
 * and call a solver (typically CSDP) to try to solve it.
 * The program uses the solver answer to change expansion sizes and types used
 * and try to get a solution better or equal to a conjectured value provided by the user.
 * After the user signals that he is satisfied with the answer, the program prints
 * the solution in a standard format.
 */
template<class UTO>
int SDPsolverProgram(int argc, char **argv) {
	using namespace std;
	enum failureOptions {rerun, accept, reject, terminate};
	int failureChoice;
	enum Solvers solverUsed = CSDP;

	bool parityMode = false;

	BigNum::bignum scale(BigNum::bn1);
	container<BigNum::bignum> rescalingFactors;

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
					parityMode = true;
					break;
				case 'r':
					ss >> scale;
					break;
				case 's':
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
					 << "Run automatic flag algebra SDP solver.\n\n"
					 << "  -p  run in parity mode (only include types that have the\n"
					 << "        same parity than the expansion).\n"
					 << "  -rN\n"
					 << "  -r N  rescale problem such that the entries are an integer\n"
					 << "          multiple of N.\n"
					 << "          N = 0 means don't rescale;\n"
					 << "          without -r N option, the program uses N = 1.\n"
					 << "          (Rescaling factor used may differ from N and\n"
					 << "          will be printed on screen.)\n"
					 << "  -sN\n"
					 << "  -s N  use solver number N.\n"
					 << SolversHelp
					 << "          without -s N option, the program uses N = 0.\n"
					 << "\n";
				return 0;
			}
			else
				break;
		}
	}

	bool min;
	cerr << "Type of problem.\nMaximization: 0; Minimization: 1\n";
	cin >> min;

	unsigned long nCount;
	cerr << "Number of subobjects to be counted: ";
	cin >> nCount;

	unsigned long countSize = 0;
	container<coeff_type> countCoeffs;
	container<UTO> countUTOs(nCount);
	vector<BigNum::frac> coeffsRead(nCount);

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

	string prefix;
	cerr << "Output file prefix: ";
	cin >> prefix;

	BigNum::frac conjectured, precision, obtained;
	cerr << "Conjectured value: ";
	cin >> conjectured;
	if (!min)
		conjectured.invertSum();

	cerr << "Precision: ";
	cin >> precision;

	unsigned long expSize = 0;
	cerr << "Starting expansion size: ";
	cin >> expSize;
	if (countSize >= expSize)
		expSize = countSize;
	if (forbSize > expSize)
		expSize = forbSize;
	if (expSize < 3)
		expSize = 3;

	/* These variables will be used to alternate between parities of types
	 * if option -p is supplied
	 */
	container<UTO> uTypesEven, uTypesOdd;
	container<UTO> *uTypesThis;
	container< Flag<UTO> > normTypesEven, normTypesOdd;
	container< Flag<UTO> > *normTypesThis;
	container<unsigned long> flagSizesEven, flagSizesOdd;
	container<unsigned long> *flagSizesThis;
	container<vbool> flagSelectEven, flagSelectOdd;
	container<vbool> *flagSelectThis;
	container< const container< Flag<UTO> >* > flagEnumsEven, flagEnumsOdd;
	container< const container< Flag<UTO> >* > *flagEnumsThis;
	vbool typeSelectEven, typeSelectOdd;
	vbool *typeSelectThis;

	unsigned long typesUsed;
	container<unsigned long> flagsUsed;

	morphism g;

	/* This macro sets which parity is going to be used
	 * (it can receive parameters `Even' or `Odd')
	 */
#define setParity(PAR)										\
	uTypesThis = &uTypes##PAR,								\
		normTypesThis = &normTypes##PAR,					\
		flagSizesThis =	&flagSizes##PAR,					\
		flagSelectThis = &flagSelect##PAR,					\
		flagEnumsThis = &flagEnums##PAR,					\
		typeSelectThis = &typeSelect##PAR

	// These macros are for readability improval
#define uTypes (*uTypesThis)
#define normTypes (*normTypesThis)
#define flagSizes (*flagSizesThis)
#define flagSelect (*flagSelectThis)
#define flagEnums (*flagEnumsThis)
#define typeSelect (*typeSelectThis)

	/* For all effects, the code works as if there where the following variables:
	 * container<UTO> uTypes;
	 * container< Flag<UTO> > normTypes;
	 * container<unsigned long> flagSizes;
	 * container<vbool> flagSelect;
	 * container< const container< Flag<UTO> >* > flagEnums;
	 * vbool typeSelect;
	 */

	// This macro removes flags that have forbidden subobjects
#define removeForbiddenFlags()											\
	do {																\
		for (unsigned long i = 0; i < forbSel.size(); ++i) {			\
			if (forbSel[i]) {											\
				for (unsigned long j = 0;								\
					 j < flagSelect.size();								\
					 ++j) {												\
					if (forbSize <= flagSizes[j]) {						\
						const container< container<coeff_type> >&		\
							forbExp										\
							(expandObjectEnumerate<UTO>(forbSize,		\
														flagSizes[j]));	\
						for (unsigned long k = 0;						\
							 k < flagSelect[j].size();					\
							 ++k) {										\
							if (flagSelect[j][k]) {						\
								for (unsigned long l = 0;				\
									 l < forbExp.size();				\
									 ++l) {								\
									if ((*flagEnums[j])[k].objectIndex() \
										== forbExp[i][l].first) {		\
										flagSelect[j][k] = false;		\
										break;							\
									}									\
								}										\
							}											\
						}												\
					}													\
				}														\
			}															\
		}																\
	} while (false)

	// This macro prints the SDP problem in the file with name filename and puts
	// the return value of fprintSDPproblem on retvar
#define printProblemOnFile(filename, retvar)							\
	do {																\
		FILE *file;														\
		const char* const& name(filename);								\
		if ((file = fopen(name, "w")) == 0) {							\
			cerr << "Error: unable to open file " << name				\
				 << " in w mode.\n";									\
			return 0;													\
		}																\
		(retvar)														\
			= fprintSDPproblem(file,									\
							   min,										\
							   countSize,								\
							   countCoeffs,								\
							   forbSize,								\
							   forbSel,									\
							   expSize,									\
							   uTypes,									\
							   flagSizes,								\
							   flagSelect,								\
							   scale,									\
							   Monitoring								\
							   ::ProgressBarFrac(progressBarSize));		\
		fclose(file);													\
	} while(false)

	// This macro prints the types and flags used in the file with name filename
#define printTypesAndFlagsUsed(filename) do {						\
		FILE *file;													\
		const char* const& name(filename);							\
		if ((file = fopen(name, "w")) == 0) {						\
			cerr << "Error: unable to open file "					\
				 << name << " in w mode.\n";						\
			return 0;												\
		}															\
		for (unsigned long i = 0; i < uTypes.size(); ++i) {			\
			if (typeSelect[i]) {									\
				fprintobj(file, uTypes[i]);							\
				fprintf(file, "\n");								\
			}														\
		}															\
		fprintf(file, "\n\n");										\
		for (unsigned long i = 0; i < flagEnums.size(); ++i) {		\
			if (typeSelect[i]) {									\
				for (unsigned long j = 0;							\
					 j < flagEnums[i]->size();						\
					 ++j) {											\
					if (flagSelect[i][j]) {							\
						fprintflag(file, (*flagEnums[i])[j]);		\
						fprintf(file, "\n");						\
					}												\
				}													\
				fprintf(file, "\n");								\
			}														\
		}															\
		fclose(file);												\
	} while (false)

#define computeTypesUsed() do {											\
		typesUsed = 0;													\
		flagsUsed.assign(uTypes.size(), 0);								\
		for (unsigned long i = 0; i < uTypes.size(); ++i) {				\
			for (unsigned long j = 0; j < flagSelect[i].size(); ++j)	\
				if (flagSelect[i][j])									\
					++flagsUsed[i];										\
			if (flagsUsed[i])											\
				++typesUsed;											\
		}																\
	} while(false)

	// This macro runs the solver program (passing commandLine to OS shell)
#define runSolver(commandLine)											\
	do {																\
		const char* const& cmdLine(commandLine);						\
		cerr << "\nRunning solver.\n"									\
			 << "Problem is rescaled by factor: "						\
			 << rescalingFactors.back() << "\n\n";						\
		failureChoice = accept;											\
		if (int ret = system(cmdLine)) {								\
			do {														\
				cerr << "Solver returned code " << ret;					\
				ret = (ret < 0 || SolverReturnSize[solverUsed] < ret)?	\
							 SolverUnknown[solverUsed] : ret;			\
				cerr << ":\n"											\
					 << SolverReturnStrings[solverUsed][ret-1]			\
					 << "\nWhat do you want me to do?\n"				\
					 << "0 = Run solver again,\n"						\
					 << "1 = Accept solution,\n"						\
					 << "2 = Reject solution,\n"						\
					 << "3 = Terminate program.\n";						\
				cin >> failureChoice;									\
				if (failureChoice != rerun)								\
					break;												\
			} while ((ret = system(cmdLine)));							\
			if (failureChoice == terminate)								\
				return 0;												\
		}																\
	} while (false)

	/* This macro reads the solver program's solution (in file with name
	 * filename) and puts the solution value on var
	 */
#define readSolution(filename, var)										\
	do {																\
		const char* const& name(filename);								\
		BigNum::frac& Var(var);											\
		FILE *file;														\
		if ((file = fopen(name, "r")) == 0) {							\
			cerr << "Error: unable to open file " << name				\
				 << " in r mode.\n";									\
			return 0;													\
		}																\
																		\
		int d;															\
		switch (solverUsed) {											\
		case CSDP:														\
			/* Skip slacknesses	*/										\
			if (fscanf(file, "%*[^\n]") != 0) {							\
				cerr << "Error: file " << name							\
					 << " corrupted.\n";								\
				return 0;												\
			}															\
			while (true) {												\
				unsigned long dual, block, i, j;						\
				if (fscanf(file,										\
						   "%lu %lu %lu %lu",							\
						   &dual,										\
						   &block,										\
						   &i,											\
						   &j) != 4) {									\
					cerr << "Error: file " << name						\
						 << " corrupted.\n";							\
					return 0;											\
				}														\
				if (dual == 2											\
					&& block == typesUsed+1								\
					&& i == 1											\
					&& j == 1) {										\
					if (BigNum::fscanfrac(file, &Var) != 1) {			\
						cerr << "Error: file " << name					\
							 << " corrupted.\n";						\
						return 0;										\
					}													\
					Var /= BigNum::frac(rescalingFactors.back());		\
					if (!min)											\
						Var.invertSum();								\
					cerr << "Primal objective rescaled value: "			\
						 << Var.toStringPoint(fracPrecision) << "\n";	\
					break;												\
				}														\
				else if (fscanf(file, "%*[^\n]") != 0) {				\
					cerr << "Error: file " << name						\
						 << " corrupted.\n";							\
					return 0;											\
				}														\
			}															\
			break;														\
		case SDPA:														\
			do {														\
				d = -1;													\
				fscanf(file, "%*[^y]yMat = {%n", &d);					\
				if (feof(file)) {										\
					cerr << "Error: file " << solver_solution_name		\
						 << " corrupted.\n";							\
					return 0;											\
				}														\
			} while (d < 0);											\
			for (unsigned long i = 0; i < typesUsed; ++i) {				\
				if (fscanf(file, " {") != 0) {							\
					cerr << "Error: file " << solver_solution_name		\
						 << " corrupted.\n";							\
					return 0;											\
				}														\
				unsigned long depth = 1;								\
				while (true) {											\
					int c = fgetc(file);								\
					if (c == '{') {										\
						++depth;										\
					}													\
					else if (c == '}') {								\
						if (--depth == 0)								\
							break;										\
					}													\
					else if (c == EOF) {								\
						cerr << "Error: file " << solver_solution_name	\
							 << " corrupted.\n";						\
						return 0;										\
					}													\
				}														\
			}															\
			if (fscanf(file, " {") != 0) {								\
				cerr << "Error: file " << solver_solution_name			\
					 << " corrupted.\n";								\
				return 0;												\
			}															\
			if (BigNum::fscanfrac(file, &Var) != 1) {					\
				cerr << "Error: file " << solver_solution_name			\
					 << " corrupted.\n";								\
				return 0;												\
			}															\
			Var /= BigNum::frac(rescalingFactors.back());				\
			cerr << "Primal objective rescaled absolute value: "		\
				 << obtained.toStringPoint(fracPrecision) << "\n";		\
			if (fscanf(file, " }") != 0) {								\
				cerr << "Error: file " << solver_solution_name			\
					 << " corrupted.\n";								\
				return 0;												\
			}															\
			break;														\
		default:														\
			break;														\
		}																\
		fclose(file);													\
	} while (false)

	/* This macro is used to exclude code that is dependant of lapacke.h
	 * from macro expansions and parameters if lapacke.h is not available
	 */
#ifdef LAPACKE_AVAILABLE
#define IF_LAPACKE(x) x
#else
#define IF_LAPACKE(x)
#endif

	/* This macro writes the final solution in matrix format (in file with name
	 * matFileName) and in squares format (in file with name sqFileName) [if the
	 * program was compiled with lapacke.h] provided the solver's solution (in
	 * file with name solvFileName)
	 */
#define writeFinalSolution(matFileName, sqFileName, solvFileName)		\
	do {																\
		cerr << "Writing final solution...\n";							\
		if (!min) {														\
			obtained.invertSum();										\
			conjectured.invertSum();									\
		}																\
		const char* const& matName(matFileName);						\
		IF_LAPACKE(const char* const& sqName(sqFileName););				\
		const char* const& solvName(solvFileName);						\
		FILE *file;														\
		if ((file = fopen(matName, "w")) == 0) {						\
			cerr << "Error: unable to open file "						\
				 << matName << " in w mode.\n";							\
			return 0;													\
		}																\
		fprintf(file, "Matrix format solution\n");						\
		fprintf(file, "%d\n%lu\n", (int) min, nCount);					\
		for (unsigned long i = 0; i < nCount; ++i) {					\
			fprintobj(file, countUTOs[i]);								\
			fprintf(file,												\
					"\n%s\n",											\
					coeffsRead[i].toStringDec().c_str());				\
		}																\
		fprintf(file, "%lu\n", nForb);									\
		for (unsigned long i = 0; i < nForb; ++i) {						\
			fprintobj(file, forbUTOs[i]);								\
			fprintf(file, "\n");										\
		}																\
		fprintf(file, "%s\n%s\n%s\n%lu\n",								\
				conjectured.toStringDec().c_str(),						\
				precision.toStringDec().c_str(),						\
				obtained.toStringDec().c_str(),							\
				expSize);												\
																		\
		fprintf(file, "%lu\n", typesUsed);								\
		for (unsigned long i = 0; i < normTypes.size(); ++i) {			\
			if (flagsUsed[i]) {											\
				fprintflag(file, normTypes[i]);							\
				fprintf(file, "\n");									\
			}															\
		}																\
		for (unsigned long i = 0; i < normTypes.size(); ++i) {			\
			if (flagsUsed[i]) {											\
				fprintf(file, "%lu\n", flagsUsed[i]);					\
				for (unsigned long j = 0;								\
					 j < flagSelect[i].size();							\
					 ++j) {												\
					if (flagSelect[i][j]) {								\
						fprintflag(file, (*flagEnums[i])[j]);			\
						fprintf(file, "\n");							\
					}													\
				}														\
			}															\
		}																\
		for (unsigned long i = 0; i < rescalingFactors.size()-1; ++i) {	\
			BigNum::fprintbignum(file, rescalingFactors[i]);			\
			fprintf(file, " ");											\
		}																\
		fprintf(file, "\n");											\
																		\
		IF_LAPACKE														\
			(															\
			 /* variables to be used with lapacke.h*/					\
			 vector< vector<double> > Mat;								\
			 vector< vector<double> > EVs;								\
			 vector<lapack_int> lapackSizes;							\
			 Mat.reserve(typesUsed);									\
			 EVs.reserve(typesUsed);									\
			 lapackSizes.reserve(typesUsed);							\
			 for (unsigned long i = 0;									\
				  i < flagsUsed.size();									\
				  ++i) {												\
				 if (flagsUsed[i]) {									\
					 Mat.push_back										\
						 (vector<double>(flagsUsed[i]*flagsUsed[i]));	\
					 EVs.push_back(vector<double>(flagsUsed[i]));		\
					 lapackSizes.push_back(flagsUsed[i]);				\
				 }														\
			 });														\
		FILE *infile;													\
		if ((infile =													\
			 fopen(solvName, "r")) == 0) {								\
			cerr << "Error: unable to open file "						\
				 << solvName << " in r mode.\n";						\
			return 0;													\
		}																\
																		\
		int d;															\
		switch (solverUsed)	{											\
		case CSDP:														\
			/* Skip dual problem y variable values */					\
			if (fscanf(infile, "%*[^\n]") != 0) {						\
				cerr << "Error: file " << solvName						\
					 << " corrupted.\n";								\
				return 0;												\
			}															\
			while (true) {												\
				unsigned long dual, block, i, j;						\
				BigNum::frac value;										\
				if (fscanf(infile, "%lu %lu %lu %lu",					\
						   &dual,										\
						   &block,										\
						   &i,											\
						   &j)											\
					+ BigNum::fscanfrac(infile, &value) != 5)			\
					break;												\
				if (dual == 2 && block <= typesUsed) {					\
					fprintf(file, "%lu %lu %lu ", block, i, j);			\
					BigNum::fprintfrac(file, value);					\
					fprintf(file, "\n");								\
																		\
					/* store the value for use with lapacke.h */		\
					IF_LAPACKE(											\
							   Mat										\
							   [block-1]								\
							   [(i-1) * lapackSizes[block-1] + (j-1)]	\
							   = Mat									\
							   [block-1]								\
							   [(j-1) * lapackSizes[block-1] + (i-1)]	\
							   = BigNum::fracToDouble(value););			\
				}														\
			}															\
			break;														\
		case SDPA:														\
			do {														\
				d = -1;													\
				fscanf(infile, "%*[^y]yMat = {%n", &d);					\
				if (feof(infile)) {										\
					cerr << "Error: file " << solver_solution_name		\
						 << " corrupted.\n";							\
					return 0;											\
				}														\
			} while (d < 0);											\
			for (unsigned long block = 0; block < typesUsed; ++block) {	\
				BigNum::frac value;										\
				if (fscanf(infile, " {") != 0) {						\
					cerr << "Error: file " << solver_solution_name		\
						 << " corrupted.\n";							\
					return 0;											\
				}														\
				for (unsigned long i = 0; i < flagsUsed[block]; ++i) {	\
					if (fscanf(infile, " {") != 0) {					\
						cerr << "Error: file " << solver_solution_name	\
							 << " corrupted.\n";						\
						return 0;										\
					}													\
					for (unsigned long j = 0;							\
						 j < flagsUsed[block];							\
						 ++j) {											\
						if (fscanfrac(infile, &value) != 1) {			\
							cerr << "Error: file "						\
								 << solver_solution_name				\
								 << " corrupted.\n";					\
							return 0;									\
						}												\
						if (i <= j) {									\
							fprintf(file,								\
									"%lu %lu %lu ",						\
									block+1,							\
									i+1,								\
									j+1);								\
							BigNum::fprintfrac(file, value);			\
							fprintf(file, "\n");						\
						}												\
																		\
						/* store the value for use with lapacke.h */	\
						IF_LAPACKE(										\
								   Mat									\
								   [block]								\
								   [i * lapackSizes[block] + j]			\
								   = BigNum::fracToDouble(value););		\
																		\
						if ((j < flagsUsed[block]-1)					\
							&& fscanf(infile, " ,") != 0) {				\
							cerr << "Error: file "						\
								 << solver_solution_name				\
								 << " corrupted.\n";					\
							return 0;									\
						}												\
					}													\
					if (fscanf(infile, " }") != 0						\
						|| ((i < flagsUsed[block]-1)					\
							&& fscanf(infile, " ,") != 0)) {			\
						cerr << "Error: file " << solver_solution_name	\
							 << " corrupted.\n";						\
						return 0;										\
					}													\
				}														\
			}															\
			break;														\
		default:														\
			break;														\
		}																\
		fclose(infile);													\
		fclose(file);													\
																		\
		IF_LAPACKE														\
			(															\
			 /* this part uses lapacke.h to compute and write			\
			  * the square format of the matrix							\
			  */														\
																		\
			 for (unsigned long i = 0; i < Mat.size(); ++i) {			\
				 if (lapack_int info =									\
					 LAPACKE_dsyev(LAPACK_COL_MAJOR,					\
								   /* Matrix is columns x lines */		\
								   'V',	/* Eigenvalues/eigenvectors */	\
								   'U',	/* Upper triangle matrix */		\
								   lapackSizes[i], /* Matrix order */	\
								   &Mat[i][0], /* Matrix */				\
								   lapackSizes[i], /* Lead dimension */	\
								   &EVs[i][0] /* Eigenvalues output */	\
								   )) {									\
					 cerr << "Error in LAPACKE_dsyev(): "				\
						  << info << endl;								\
					 return 0;											\
				 }														\
			 }															\
																		\
			 if ((file = fopen(sqName, "w")) == 0) {					\
				 cerr << "Error: unable to open file "					\
					  << sqName << " in w mode.\n";						\
				 return 0;												\
			 }															\
			 fprintf(file, "Squares format solution\n");				\
			 fprintf(file, "%d\n%lu\n", (int) min, nCount);				\
			 for (unsigned long i = 0; i < nCount; ++i) {				\
				 fprintobj(file, countUTOs[i]);							\
				 fprintf(file,											\
						 "\n%s\n",										\
						 coeffsRead[i].toStringDec().c_str());			\
			 }															\
			 fprintf(file, "%lu\n", nForb);								\
			 for (unsigned long i = 0; i < nForb; ++i) {				\
				 fprintobj(file, forbUTOs[i]);							\
				 fprintf(file, "\n");									\
			 }															\
			 fprintf(file, "%s\n%s\n%s\n%lu\n",							\
					 conjectured.toStringDec().c_str(),					\
					 precision.toStringDec().c_str(),					\
					 obtained.toStringDec().c_str(),					\
					 expSize);											\
			 fprintf(file, "%lu\n", typesUsed);							\
			 for (unsigned long i = 0; i < normTypes.size(); ++i) {		\
				 if (flagsUsed[i]) {									\
					 fprintflag(file, normTypes[i]);					\
					 fprintf(file, "\n");								\
				 }														\
			 }															\
			 for (unsigned long i = 0; i < normTypes.size(); ++i) {		\
				 if (flagsUsed[i]) {									\
					 fprintf(file, "%lu\n", flagsUsed[i]);				\
					 for (unsigned long j = 0;							\
						  j < flagSelect[i].size();						\
						  ++j) {										\
						 if (flagSelect[i][j]) {						\
							 fprintflag(file, (*flagEnums[i])[j]);		\
							 fprintf(file, "\n");						\
						 }												\
					 }													\
				 }														\
			 }															\
			 for (unsigned long i = 0;									\
				  i < rescalingFactors.size()-1;						\
				  ++i) {												\
				 BigNum::fprintbignum(file, rescalingFactors[i]);		\
				 fprintf(file, " ");									\
			 }															\
			 fprintf(file, "\n");										\
																		\
			 for (unsigned long i = 0; i < Mat.size(); ++i) {			\
				 for (lapack_int j = 0; j < lapackSizes[i]; ++j) {		\
					 fprintf(file,										\
							 "%lu %lu 0 %.18e\n",						\
							 i+1,										\
							 static_cast<unsigned long>(j+1),			\
							 EVs[i][j]);								\
					 for (lapack_int k = 0; k < lapackSizes[i]; ++k) {	\
						 fprintf(file,									\
								 "%lu %lu %lu %.18e\n",					\
								 i+1,									\
								 static_cast<unsigned long>(j+1),		\
								 static_cast<unsigned long>(k+1),		\
								 Mat[i][j * lapackSizes[i] + k]);		\
					 }													\
				 }														\
			 }															\
			 fclose(file););											\
		if (!min) {														\
			obtained.invertSum();										\
			conjectured.invertSum();									\
		}																\
	} while (false)

	for (unsigned long sz = 1; sz < expSize - 2; ++sz) {
		const container<UTO>& enumUTOs(enumerateObjects<UTO>(sz));
		const unsigned long flagSize((expSize - 1 - sz) / 2 + sz);
		if (parityMode && sz % 2)
			setParity(Odd);
		else
			setParity(Even);
		for (unsigned long i = 0; i < enumUTOs.size(); ++i) {
			uTypes.push_back(enumUTOs[i]);
			typeSelect.push_back(true);
			flagSizes.push_back(flagSize);
			flagEnums.push_back(&enumerateFlags<UTO>(flagSize, enumUTOs[i], g));
			normTypes.push_back(Flag<UTO>(enumUTOs[i], g));
			flagSelect.push_back(vbool(flagEnums.back()->size(), true));
		}
	}

	const string problem_name(prefix + problem_suffix + extension),
		tfused_name(prefix + tfused_suffix + extension),
		solver_solution_name(prefix + solver_solution_suffix + extension),
		command_line(solver[solverUsed] + " " + problem_name + " "
					 + outputOption[solverUsed] + solver_solution_name),
		final_solution_name(prefix + final_solution_suffix + extension),
		final_solution_squares_name(prefix + final_solution_squares_suffix + extension);

	while (true) {
		cerr << "\nExpansion size: " << expSize << endl;
		if (parityMode && expSize % 2)
			setParity(Odd);
		else
			setParity(Even);

		for (unsigned long i = 0; i < flagSizes.size(); ++i) {
			if ((uTypes[i].size() + expSize) % 2 == 0) {
				++flagSizes[i];
				flagEnums[i] = &enumerateFlags<UTO>(flagSizes[i], uTypes[i], g);
				flagSelect[i].resize(flagEnums[i]->size(), true);
			}
		}

		const container<UTO>& enumUTOs(enumerateObjects<UTO>(expSize-2));
		for (unsigned long i = 0; i < enumUTOs.size(); ++i) {
			uTypes.push_back(enumUTOs[i]);
			typeSelect.push_back(true);
			flagSizes.push_back(expSize-1);
			flagEnums.push_back(&enumerateFlags<UTO>(expSize-1, enumUTOs[i], g));
			normTypes.push_back(Flag<UTO>(enumUTOs[i], g));
			flagSelect.push_back(vbool(flagEnums.back()->size(), true));
		}

		removeForbiddenFlags();
		computeTypesUsed();
		cerr << "\nUsing " << typesUsed << " types.\n";
		printProblemOnFile(problem_name.c_str(), rescalingFactors);
		printTypesAndFlagsUsed(tfused_name.c_str());
		runSolver(command_line.c_str());
		readSolution(solver_solution_name.c_str(), obtained);

		bool choice = true;
		while (obtained >= conjectured - precision && failureChoice != reject) {
			if (!min) {
				obtained.invertSum();
				conjectured.invertSum();
			}
			cerr << "Solution value close enough or better than conjectured found.\n"
				 << "Solution value:\n" << obtained << "\nConjectured value:\n"
				 << conjectured << "\nApproximated values:\nSolution:\n";
			obtained.putOnStreamPoint(cerr, fracPrecision) << "\nConjectured:\n";
			conjectured.putOnStreamPoint(cerr, fracPrecision);
			if (!min) {
				obtained.invertSum();
				conjectured.invertSum();
			}
			cerr << "\nDo you want to type a new conjectured value?\n"
				"(1 = yes, 0 = no)\n";
			cin >> choice;
			if (!choice)
				break;

			cerr << "New conjectured value: ";
			cin >> conjectured;
			if (!min)
				conjectured.invertSum();
		}
		if (!choice)
			break;
		++expSize;
	}

	computeTypesUsed();
	writeFinalSolution(final_solution_name.c_str(),
					   final_solution_squares_name.c_str(),
					   solver_solution_name.c_str());

	cerr << "Flag sizes used:\n";
	for (unsigned long i = 0; i < flagSizes.size(); ++i)
		cerr << flagSizes[i] << " ";
	cerr << "\nDo you want me to try to reduce the flag sizes (this can take a while)?\n"
		"(1 = yes, 0 = no)\n";
	container<BigNum::bignum> rescalingFactorsRed;
	bool reduceFlagSizes;
	cin >> reduceFlagSizes;
	if (reduceFlagSizes) {
		const string problem_red_name(prefix + problem_suffix
									  + reductor_suffix + extension),
			tfused_red_name(prefix + tfused_suffix + reductor_suffix + extension),
			solver_solution_red_name(prefix + solver_solution_suffix
									 + reductor_suffix + extension),
			command_line_red(solver[solverUsed] + " " + problem_red_name + " "
							 + outputOption[solverUsed] + solver_solution_red_name);
		for (unsigned long toRed = 0; toRed < flagSizes.size(); ++toRed) {
			while (flagSizes[toRed] > uTypes[toRed].size()+1) {
				cerr << "Flag sizes used:\n";
				for (unsigned long i = 0; i < flagSizes.size(); ++i)
					cerr << flagSizes[i] << " ";

				--flagSizes[toRed];
				flagEnums[toRed] = &enumerateFlags<UTO>(flagSizes[toRed],
														uTypes[toRed],
														g);
				flagSelect[toRed].resize(flagEnums[toRed]->size(), true);

				cerr << "\nTrying to reduce flag size number " << toRed+1
					 << " to " << flagSizes[toRed] << endl;

				BigNum::frac newObtained;

				removeForbiddenFlags();
				computeTypesUsed();
				printProblemOnFile(problem_red_name.c_str(), rescalingFactorsRed);
				printTypesAndFlagsUsed(tfused_red_name.c_str());
				runSolver(command_line_red.c_str());
				readSolution(solver_solution_red_name.c_str(), newObtained);

				if (newObtained >= conjectured - precision && failureChoice != reject) {

					cerr << "\nFlag size successfully reduced.\n";
					obtained = newObtained;
					if (system((copier + " " + problem_red_name
								+ " " + problem_name).c_str()))
						cerr << "Error while copying " << problem_red_name << ".\n";
					if (system((copier + " " + solver_solution_red_name
								+ " " + solver_solution_name).c_str()))
						cerr << "Error while copying "
							 << solver_solution_red_name << ".\n";
					if (system((copier + " " + tfused_red_name
								+ " " + tfused_name).c_str()))
						cerr << "Error while copying " << tfused_red_name << ".\n";
					rescalingFactors.swap(rescalingFactorsRed);
				}
				else {
					++flagSizes[toRed];
					flagEnums[toRed] = &enumerateFlags<UTO>(flagSizes[toRed],
															uTypes[toRed],
															g);
					flagSelect[toRed].resize(flagEnums[toRed]->size(), true);
					break;
				}
			}
		}
		if (system((remover + " " + problem_red_name).c_str()))
			cerr << "Error while removing " << problem_red_name << ".\n";
		if (system((remover + " " + solver_solution_red_name).c_str()))
			cerr << "Error while removing " << solver_solution_red_name << ".\n";
		if (system((remover + " " + tfused_red_name).c_str()))
			cerr << "Error while removing " << tfused_red_name << ".\n";
		removeForbiddenFlags();
		computeTypesUsed();
		writeFinalSolution(final_solution_name.c_str(),
						   final_solution_squares_name.c_str(),
						   solver_solution_name.c_str());
	}

	cerr << "A total of " << uTypes.size() << " types were used.\n"
		"Do you want me to try to reduce this number (this can take a while)?\n"
		"(1 = yes, 0 = no)\n";
	bool reduceTypes;
	cin >> reduceTypes;
	if (reduceTypes) {
		unsigned long typesRemaining = typeSelect.size();
		const string problem_red_name(prefix + problem_suffix
									  + reductor_suffix + extension),
			tfused_red_name(prefix + tfused_suffix + reductor_suffix + extension),
			solver_solution_red_name(prefix + solver_solution_suffix
									 + reductor_suffix + extension),
			command_line_red(solver[solverUsed] + " " + problem_red_name + " "
							 + outputOption[solverUsed] + solver_solution_red_name);

		for (unsigned long toRem = 0; toRem < uTypes.size(); ++toRem) {
			BigNum::frac newObtained;

			cerr << "\nTrying to remove type number " << toRem+1
				 << " of " << uTypes.size() << ":\n";
			fprintobj(stderr, uTypes[toRem]);
			cerr << "\nwith subexpansion " << flagSizes[toRem] << endl;

			vbool oldSel(flagSelect.size(), false);
			flagSelect[toRem].swap(oldSel); // Swaps with oldSel

			cerr << "\nUsing " << typesRemaining-1 << " types and subexpansions.\n";

			printProblemOnFile(problem_red_name.c_str(), rescalingFactorsRed);
			printTypesAndFlagsUsed(tfused_red_name.c_str());
			runSolver(command_line_red.c_str());
			computeTypesUsed();
			readSolution(solver_solution_red_name.c_str(), newObtained);

			if (newObtained >= conjectured - precision && failureChoice != reject) {

				cerr << "\nType successfully removed.\n";
				obtained = newObtained;
				--typesRemaining;
				typeSelect[toRem] = false;
				if (system((copier + " " + problem_red_name
							+ " " + problem_name).c_str()))
					cerr << "Error while copying " << problem_red_name << ".\n";
				if (system((copier + " " + solver_solution_red_name
							+ " " + solver_solution_name).c_str()))
					cerr << "Error while copying " << solver_solution_red_name << ".\n";
				if (system((copier + " " + tfused_red_name + " " + tfused_name).c_str()))
					cerr << "Error while copying " << tfused_red_name << ".\n";
				rescalingFactors.swap(rescalingFactorsRed);
			}
			else {
				flagSelect[toRem].swap(oldSel); // Swaps back
			}
		}
		computeTypesUsed();
		writeFinalSolution(final_solution_name.c_str(),
						   final_solution_squares_name.c_str(),
						   solver_solution_name.c_str());

		unsigned long flagsRemaining = 0;
		for (unsigned long i = 0; i < flagSelect.size(); ++i)
			if (typeSelect[i])
				for (unsigned long j = 0; j < flagSelect[i].size(); ++j)
					if (flagSelect[i][j])
						++flagsRemaining;

		cerr << "A total of " << flagsRemaining << " flags were used.\n"
			"Do you want me to try to reduce this number "
			"(this can take a very long time)?\n"
			"(1 = yes, 0 = no)\n";
		bool reduceFlags;
		cin >> reduceFlags;
		if (reduceFlags) {
			const unsigned long flagsInitialValue(flagsRemaining);
			unsigned long flagRemCount = 0;
			for (unsigned long typeInd = 0; typeInd < typeSelect.size(); ++typeInd) {
				if (typeSelect[typeInd]) {
					for (unsigned long toRem = 0;
						 toRem < flagSelect[typeInd].size();
						 ++toRem) {
						if (!flagSelect[typeInd][toRem])
							continue;
						BigNum::frac newObtained;

						cerr << "\nTrying to remove flag number " << ++flagRemCount
							 << " of " << flagsInitialValue << ":\n";
						fprintflag(stderr, (*flagEnums[typeInd])[toRem]);
						cerr << endl;

						flagSelect[typeInd][toRem] = false;

						cerr << "\nUsing " << flagsRemaining-1 << " flags.\n";

						printProblemOnFile(problem_red_name.c_str(), rescalingFactorsRed);
						printTypesAndFlagsUsed(tfused_red_name.c_str());
						runSolver(command_line_red.c_str());
						computeTypesUsed();
						readSolution(solver_solution_red_name.c_str(), newObtained);

						if (newObtained >= conjectured - precision
							&& failureChoice != reject) {
							cerr << "\nFlag successfully removed.\n";
							obtained = newObtained;
							--flagsRemaining;
							if (system((copier + " " + problem_red_name
										+ " " + problem_name).c_str()))
								cerr << "Error while copying "
									 << problem_red_name << ".\n";
							if (system((copier + " " + solver_solution_red_name
										+ " " + solver_solution_name).c_str()))
								cerr << "Error while copying "
									 << solver_solution_red_name << ".\n";
							if (system((copier + " " + tfused_red_name
										+ " " + tfused_name).c_str()))
								cerr << "Error while copying "
									 << tfused_red_name << ".\n";
							rescalingFactors.swap(rescalingFactorsRed);
						}
						else {
							flagSelect[typeInd][toRem] = true;
						}
					}
				}
			}
			computeTypesUsed();
			writeFinalSolution(final_solution_name.c_str(),
							   final_solution_squares_name.c_str(),
							   solver_solution_name.c_str());
		}
		if (system((remover + " " + problem_red_name).c_str()))
			cerr << "Error while removing " << problem_red_name << ".\n";
		if (system((remover + " " + solver_solution_red_name).c_str()))
			cerr << "Error while removing " << solver_solution_red_name << ".\n";
		if (system((remover + " " + tfused_red_name).c_str()))
			cerr << "Error while removing " << tfused_red_name << ".\n";
	}
	return 0;
#undef setParity
#undef uTypes
#undef normTypes
#undef flagSizes
#undef flagSelect
#undef flagEnums
#undef typeSelect
#undef printProblemOnFile
#undef printTypesAndFlagsUsed
#undef runSolver
#undef computeTypesUsed
#undef readSolution
#undef IF_LAPACKE
#undef writeFinalSolution
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
