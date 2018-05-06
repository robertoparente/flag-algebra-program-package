#ifndef EXPANDPROGRAMSTEMPLATES_CPP_INCLUDED
#define EXPANDPROGRAMSTEMPLATES_CPP_INCLUDED

template<class UTO>
int enumerateProgram(int argc, char **argv) {
	bool flagMode = false;

	/* Options parsing block */ {
		int ind1 = 1, ind2 = 0;
		char opt;
		const char* arg = "";
		while (true) {
			if (Text::getoptions("f", ind1, ind2, opt, arg, argc, argv)) {
				switch (opt) {
				case 'f':
					flagMode = true;
					break;
				case '\0':
					std::cerr << "Invalid option: " << *arg
							  << "\nTry `" << argv[0]
							  << " --help' for more information.\n";
					return 1;
				}
			}
			else if (ind1 < argc && !strcmp(argv[ind1] + ind2, "--help")) {
				std::cerr << "Usage: " << argv[0] << " [options]\n"
						  << "Enumerate UTO objects.\n\n"
						  << "  -f  enumerate UTO flags instead.\n"
						  << "\n";
				return 0;
			}
			else
				break;
		}
	}
	unsigned long size;
	UTO utype;
	if (flagMode) {
		std::cerr << "Type the unlabeled type:\n";
		scanobj(&utype);
	}
	std::cerr << "Type the expansion size: ";
	std::cin >> size;
	if (flagMode) {
		if (size < utype.size()) {
			std::cerr << "Error: expansion size smaller than type size.\n";
			return 0;
		}
		morphism f;
		const container< Flag<UTO> >& flags(enumerateFlags(size, utype, f));
		for (unsigned long i = 0; i < flags.size(); ++i) {
			printflag(flags[i]);
			std::cout << "\n";
		}
		return 0;
	}
	const container<UTO> utos(enumerateObjects<UTO>(size));
	for (unsigned long i = 0; i < utos.size(); ++i) {
		printobj(utos[i]);
		std::cout << "\n";
	}
	return 0;
}

template<class UTO>
int expandProgram(int argc, char **argv) {
	bool onlyCoeffs = false;
	/* Options parsing block */ {
		int ind1 = 1, ind2 = 0;
		char opt;
		const char* arg = "";
		while (true) {
			if (Text::getoptions("c", ind1, ind2, opt, arg, argc, argv)) {
				switch (opt) {
				case 'c':
					onlyCoeffs = true;
					break;
				case '\0':
					std::cerr << "Invalid option: " << *arg
							  << "\nTry `" << argv[0]
							  << " --help' for more information.\n";
					return 1;
				}
			}
			else if (ind1 < argc && !strcmp(argv[ind1] + ind2, "--help")) {
				std::cerr << "Usage: " << argv[0] << " [options]\n"
						  << "Enumerate UTO objects.\n\n"
						  << "  -c  print only coefficients (with zeros).\n"
						  << "\n";
				return 0;
			}
			else
				break;
		}
	}

	Flag<UTO> flag;
	std::cerr << "Type a flag to expand:\n";
	scanflag(&flag);

	unsigned long s;
	std::cerr << "Type a size to expand:\n";
	std::cin >> s;

	if (flag.size() > s) {
		std::cerr << "Error: flag.size() > s\n";
		return 0;
	}

	Flag<UTO> tp(flag.type());
	morphism g;

	container< Flag<UTO> > baseEnum(enumerateFlags(flag.size(), tp)),
		expEnum(enumerateFlags(s, tp));

	unsigned long index = 0;
	while (flag != baseEnum[index])
		++index;

	const container<coeff_type>& expCoeffs
		(expandFlagEnumerate(tp.object(), flag.size(), s, g)[index]);

	if (onlyCoeffs) {
		unsigned long index = 0;
		for (unsigned long i = 0; i < expCoeffs.size(); ++i) {
			while (index++ < expCoeffs[i].first)
				BigNum::bn0.putOnStreamDec(std::cout) << "\n";
			expCoeffs[i].second.putOnStreamDec(std::cout) << "\n";
		}
		while (index++ < expEnum.size())
			BigNum::bn0.putOnStreamDec(std::cout) << "\n";
	}
	else {
		printflag(flag);
		std::cout << " =\n\n";
		for (unsigned long i = 0; i < expCoeffs.size()-1; ++i) {
			expCoeffs[i].second.putOnStreamDec(std::cout) << " *\n";
			printflag(expEnum[expCoeffs[i].first]);
			std::cout << "\n+\n";
		}
		if (expCoeffs.size()) {
			expCoeffs.back().second.putOnStreamDec(std::cout) << " *\n";
			printflag(expEnum[expCoeffs.back().first]);
		}
	}
	return 0;
}


template<class UTO>
int expandProductProgram(int argc, char **argv) {
	bool onlyCoeffs = false;
	/* Options parsing block */ {
		int ind1 = 1, ind2 = 0;
		char opt;
		const char* arg = "";
		while (true) {
			if (Text::getoptions("c", ind1, ind2, opt, arg, argc, argv)) {
				switch (opt) {
				case 'c':
					onlyCoeffs = true;
					break;
				case '\0':
					std::cerr << "Invalid option: " << *arg
							  << "\nTry `" << argv[0]
							  << " --help' for more information.\n";
					return 1;
				}
			}
			else if (ind1 < argc && !strcmp(argv[ind1] + ind2, "--help")) {
				std::cerr << "Usage: " << argv[0] << " [options]\n"
						  << "Enumerate UTO objects.\n\n"
						  << "  -c  print only coefficients (with zeros).\n"
						  << "\n";
				return 0;
			}
			else
				break;
		}
	}

	Flag<UTO> flag1;
	std::cerr << "Type first flag:\n";
	scanflag(&flag1);

	Flag<UTO> flag2;
	std::cerr << "Type second flag:\n";
	scanflag(&flag2);

	unsigned long s;
	std::cerr << "Type a size to expand:\n";
	std::cin >> s;

	Flag<UTO> tp(flag1.type());

	if (flag2.type() != tp) {
		std::cerr << "Error: flags have different types\n";
		return 0;
	}

	if (flag1.size() + flag2.size() - tp.size() > s) {
		std::cerr << "Error: Incompatible sizes\n";
		return 0;
	}

	morphism g;

	container< Flag<UTO> > base1Enum(enumerateFlags(flag1.size(), tp)),
		base2Enum(enumerateFlags(flag2.size(), tp)),
		expEnum(enumerateFlags(s, tp));

	unsigned long index1 = 0;
	while(flag1 != base1Enum[index1])
		++index1;
	unsigned long index2 = 0;
	while(flag2 != base2Enum[index2])
		++index2;

	const container<coeff_type>& expCoeffs
		(expandFlagProductEnumerate(tp.object(),
									flag1.size(),
									flag2.size(),
									s,
									g)[index1][index2]);


	if (onlyCoeffs) {
		unsigned long index = 0;
		for (unsigned long i = 0; i < expCoeffs.size(); ++i) {
			while (index++ < expCoeffs[i].first)
				BigNum::bn0.putOnStreamDec(std::cout) << "\n";
			expCoeffs[i].second.putOnStreamDec(std::cout) << "\n";
		}
		while (index++ < expEnum.size())
			BigNum::bn0.putOnStreamDec(std::cout) << "\n";
	}
	else {
		printflag(flag1);
		std::cout << "\n*\n";
		printflag(flag2);
		std::cout << " =\n\n";
		for (unsigned long i = 0; i < expCoeffs.size()-1; ++i) {
			expCoeffs[i].second.putOnStreamDec(std::cout) << " *\n";
			printflag(expEnum[expCoeffs[i].first]);
			std::cout << "\n+\n";
		}
		if (expCoeffs.size()) {
			expCoeffs.back().second.putOnStreamDec(std::cout) << " *\n";
			printflag(expEnum[expCoeffs.back().first]);
		}
	}
	return 0;
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
