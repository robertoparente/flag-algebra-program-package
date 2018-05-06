#ifndef TESTFLAGTEMPLATES_CPP_INCLUDED
#define TESTFLAGTEMPLATES_CPP_INCLUDED

template<class UTO>
void testFlag() {
	using namespace std;

	/* UTO testing block */ {
		UTO uto1;
		cerr << "Type an object:\n";
		scanobj(&uto1);
		cerr << "Starting tests on object:\n";
		fprintobj(stderr, uto1);
		cerr << endl << endl;

		cerr << "size(): " << uto1.size() << endl;
		vbool sel(uto1.size());
		cerr << "Type subobject selecting vbool:\n";
		for (unsigned long i = 0; i < uto1.size(); ++i) {
			bool a;
			cin >> a;
			sel[i] = a;
		}
		cerr << "Selecting vbool:\n";
		for (unsigned long i = 0; i < sel.size(); ++i)
			cerr << sel[i] << " ";
		cerr << endl << endl;
		cerr << "subobject:\n";
		fprintobj(stderr, uto1.subobject(sel));
		cerr << endl << endl;

		UTO uto2;
		cerr << "Type another object to compare:\n";
		scanobj(&uto2);
		cerr << "Comparing objects:\n";
		fprintobj(stderr, uto1);
		cerr << "\nand:\n";
		fprintobj(stderr, uto2);
		cerr << endl << endl;

		cerr << "operator==: " << (uto1 == uto2) << endl;
		cerr << "operator!=: " << (uto1 != uto2) << endl;

		morphism f;
		cerr << "findIsomorphism: " << uto1.findIsomorphism(uto2, f) << endl;
		cerr << "  morphism returned:\n";
		for (unsigned long i = 0; i < f.size(); ++i)
			cerr << f[i] << " ";
		cerr << endl << endl;

		cerr << "Type an morphism to try to complete:\n";
		for (unsigned long i = 0; i < f.size(); ++i)
			cin >> f[i];
		cerr << "Trying to complete:\n";
		for (unsigned long i = 0; i < f.size(); ++i)
			cerr << f[i] << " ";
		cerr << "\n\ncompleteIsomorphism: " << uto1.completeIsomorphism(uto2, f) << endl;
		cerr << "  morphism returned:\n";
		for (unsigned long i = 0; i < f.size(); ++i)
			cerr << f[i] << " ";
		cerr << endl << endl;


		unsigned long s;
		cerr << "Type size of enumeration: ";
		cin >> s;
		cerr << "enumerateObjects(" << s << "):\n";
		const container<UTO>& e(enumerateObjects<UTO>(s));
		for (unsigned long i = 0; i < e.size(); ++i) {
			fprintobj(stderr, e[i]);
			cerr << endl;
		}
		cerr << endl;
	}

	/* Flag testing block */ {
		Flag<UTO> flag1;

		cerr << "Type a flag:\n";
		scanflag(&flag1);
		cerr << "Starting tests on flag:\n";
		fprintflag(stderr, flag1);
		cerr << endl << endl;

		cerr << "size(): " << flag1.size() << endl;
		cerr << "typeSize(): " << flag1.typeSize() << endl;

		cerr << "labeledVertices:\n";
		vbool v(flag1.labeledVertices());
		for (unsigned long i = 0; i < v.size(); ++i)
			cerr << v[i] << " ";
		cerr << endl << endl;

		cerr << "unlabeledType():\n";
		fprintobj(stderr, flag1.unlabeledType());
		cerr << endl << endl;

		cerr << "type():\n";
		fprintflag(stderr, flag1.type());
		cerr << endl << endl;

		cerr << "object():\n";
		fprintobj(stderr, flag1.object());
		cerr << endl << endl;

		cerr << "objectIndex(): " << flag1.objectIndex() << endl;

		cerr << "typeMorphism():\n";
		const morphism& f(flag1.typeMorphism());
		for (unsigned long i = 0; i < f.size(); ++i)
			cerr << f[i] << " ";
		cerr << endl << endl;

		cerr << "normFactor():\n" << flag1.normFactor().toStringDec() << endl << endl;

		vbool sel(flag1.size());
		cerr << "Type subflag selecting vbool:\n";
		for (unsigned long i = 0; i < flag1.size(); ++i) {
			bool a;
			cin >> a;
			sel[i] = a;
		}
		cerr << "Selecting vbool:\n";
		for (unsigned long i = 0; i < sel.size(); ++i)
			cerr << sel[i] << " ";
		cerr << endl << endl;
		cerr << "safeSubflag:\n";
		try {
			fprintflag(stderr, flag1.safeSubflag(sel));
		}
		catch (FlagException fe) {
			cerr << "FlagException caught:\n" << fe.what();
		}
		cerr << endl << endl;


		Flag<UTO> flag2;

		cerr << "Type another flag to compare:\n";
		scanflag(&flag2);
		cerr << "Comparing flags:\n";
		fprintflag(stderr, flag1);
		cerr << "\nand:\n";
		fprintflag(stderr, flag2);
		cerr << endl << endl;

		cerr << "operator==: " << (flag1 == flag2) << endl;
		cerr << "operator!=: " << (flag1 != flag2) << endl;

		morphism g;
		cerr << "findIsomorphism: " << flag1.findIsomorphism(flag2, g) << endl;
		cerr << "  morphism returned:\n";
		for (unsigned long i = 0; i < g.size(); ++i)
			cerr << g[i] << " ";
		cerr << endl << endl;

		cerr << "Type an morphism to try to complete:\n";
		for (unsigned long i = 0; i < g.size(); ++i)
			cin >> g[i];
		cerr << "Trying to complete:\n";
		for (unsigned long i = 0; i < g.size(); ++i)
			cerr << g[i] << " ";
		cerr << "\n\ncompleteIsomorphism: " << flag1.completeIsomorphism(flag2, g)
			 << endl;
		cerr << "  morphism returned:\n";
		for (unsigned long i = 0; i < g.size(); ++i)
			cerr << g[i] << " ";
		cerr << endl << endl;

		unsigned long s;
		cerr << "Type size of enumeration of flags (unlabeled type): ";
		cin >> s;
		UTO utp;
		cerr << "Type unlabeled type:\n";
		scanobj(&utp);
		cerr << "\n\nenumerateFlags:\n";
		const container< Flag<UTO> >& fe(enumerateFlags(s, utp, g));
		for (unsigned long i = 0; i < fe.size(); ++i) {
			fprintflag(stderr, fe[i]);
			cerr << endl;
		}
		cerr << "  with unlabeled type:\n";
		fprintobj(stderr, utp);
		cerr << "\n  and morphism:\n";
		for (unsigned long i = 0; i < g.size(); ++i)
			cerr << g[i] << " ";
		cerr << endl << endl;

		cerr << "Type size of enumeration of flags (labeled type): ";
		cin >> s;
		Flag<UTO> ltp;
		cerr << "Type a labeled type:\n";
		scanflag(&ltp);
		cerr << "\n\nenumerateFlags:\n";
		container< Flag<UTO> > fe1(enumerateFlags(s,ltp));
		for (unsigned long i = 0; i < fe1.size(); ++i) {
			fprintflag(stderr, fe1[i]);
			cerr << endl;
		}
		cerr << "  with labeled type:\n";
		fprintflag(stderr, ltp);
		cerr << endl << endl;

		cerr << "Type a flag to expand:\n";
		scanflag(&flag1);
		cerr << "Type a size to expand: ";
		cin >> s;
		fe1 = enumerateFlags(flag1.size(), flag1.type());
		container< Flag<UTO> > fe2(enumerateFlags(s, flag1.type()));
		const container< container<coeff_type> >& expCoeffs
			(expandFlagEnumerate(flag1.unlabeledType(),
								 flag1.size(),
								 s,
								 g));
		unsigned long index = 0;
		for ( ; index < fe1.size(); ++index)
			if (flag1 == fe1[index])
				break;
		if (index == fe1.size()) {
			cerr << "can't find flag\n";
			fprintflag(stderr, flag1);
			cerr << "\non enum.\n";
			return;
		}
		fprintflag(stderr, flag1);
		cerr << " =\n\n";
		for (unsigned long i = 0; i < expCoeffs[index].size(); ++i) {
			cerr << expCoeffs[index][i].second.toStringDec() << " *\n";
			fprintflag(stderr, fe2[expCoeffs[index][i].first]);
			cerr << (i < expCoeffs[index].size()? "\n+\n" : "\n\n");
		}

		cerr << "Type the first flag to expand the product:\n";
		scanflag(&flag1);
		cerr << "Type the second flag to expand the product:\n";
		scanflag(&flag2);
		cerr << "Type a size to expand: ";
		cin >> s;
		fe1 = enumerateFlags(flag1.size(), flag1.type());
		fe2 = enumerateFlags(flag2.size(), flag2.type());
		container< Flag<UTO> > fe3(enumerateFlags(s, flag1.type()));
		const container< container< container<coeff_type> > >& expProdCoeffs
			(expandFlagProductEnumerate(flag1.unlabeledType(),
										flag1.size(),
										flag2.size(),
										s,
										g));
		unsigned long index1 = 0;
		for ( ; index1 < fe1.size(); ++index1)
			if (flag1 == fe1[index1])
				break;
		if (index1 == fe1.size()) {
			cerr << "Error: can't find flag\n";
			fprintflag(stderr, flag1);
			cerr << "\non enum.\n";
			return;
		}
		unsigned long index2 = 0;
		for ( ; index2 < fe2.size(); ++index2)
			if (flag2 == fe2[index2])
				break;
		if (index2 == fe2.size()) {
			cerr << "Error: can't find flag\n";
			fprintflag(stderr, flag2);
			cerr << "\non enum.\n";
			return;
		}
		fprintflag(stderr, flag1);
		cerr << "\n*\n";
		fprintflag(stderr, flag2);
		cerr << " =\n\n";
		for (unsigned long i = 0; i < expProdCoeffs[index1][index2].size(); ++i) {
			cerr << expProdCoeffs[index1][index2][i].second.toStringDec() << " *\n";
			fprintflag(stderr, fe3[expProdCoeffs[index1][index2][i].first]);
			cerr << (i < expProdCoeffs[index1][index2].size()? "\n+\n" : "\n\n");
		}
	}
}

/* Local Variables: */
/* mode: c++ */
/* tab-width: 4 */
/* End: */
#endif
