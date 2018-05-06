#Extra specific object dependencies
$(OBP)/Tournament/FlagTournament.o: FlagDigraph.h
$(OBP)/Tournament/TestFlagTournament.o: FlagDigraph.h
$(OBP)/Tournament/EnumerateFlagTournament.o: FlagDigraph.h
$(OBP)/Tournament/ExpandFlagTournament.o: FlagDigraph.h
$(OBP)/Tournament/ExpandProductFlagTournament.o: FlagDigraph.h
$(OBP)/Tournament/SDPgenTournament.o: FlagDigraph.h
$(OBP)/Tournament/SDPsolverTournament.o: FlagDigraph.h
$(OBP)/Tournament/SDPcheckerTournament.o: FlagDigraph.h
$(OBP)/Tournament/SDProunderTournament.o: FlagDigraph.h

#Extra specific executable dependencies and recipes
$(EXP)/Tournament/TestFlagTournament$(EXT): \
	$(addprefix $(OBP)/,Digraph/FlagDigraph.o)
$(EXP)/Tournament/EnumerateFlagTournament$(EXT): \
	$(addprefix $(OBP)/,Digraph/FlagDigraph.o)
$(EXP)/Tournament/ExpandFlagTournament$(EXT): \
	$(addprefix $(OBP)/,Digraph/FlagDigraph.o)
$(EXP)/Tournament/ExpandProductFlagTournament$(EXT): \
	$(addprefix $(OBP)/,Digraph/FlagDigraph.o)
$(EXP)/Tournament/SDPgenTournament$(EXT): \
	$(addprefix $(OBP)/,Digraph/FlagDigraph.o)
$(EXP)/Tournament/SDPsolverTournament$(EXT): \
	$(addprefix $(OBP)/,Digraph/FlagDigraph.o)
$(EXP)/Tournament/SDPcheckerTournament$(EXT): \
	$(addprefix $(OBP)/,Digraph/FlagDigraph.o)
$(EXP)/Tournament/SDProunderTournament$(EXT): \
	$(addprefix $(OBP)/,Digraph/FlagDigraph.o)
