#C++ include flags
override CXXINCLUDE+=-Isrc/include/$(THISMODNAME)/

#Object files
THISOBJFILES+=\
	Flag$(THISMODNAME).o\
	TestFlag$(THISMODNAME).o\
	EnumerateFlag$(THISMODNAME).o\
	ExpandFlag$(THISMODNAME).o\
	ExpandProductFlag$(THISMODNAME).o\
	SDPgen$(THISMODNAME).o\
	SDPsolver$(THISMODNAME).o\
	SDPchecker$(THISMODNAME).o\
	SDProunder$(THISMODNAME).o
THISOBJFILES:=$(addprefix $(OBP)/$(THISMODNAME)/,$(THISOBJFILES))
OBJFILES+=$(THISOBJFILES)

#Ordinary executable files
THISORDEXECFILES+=\
	TestFlag$(THISMODNAME)\
	EnumerateFlag$(THISMODNAME)\
	ExpandFlag$(THISMODNAME)\
	ExpandProductFlag$(THISMODNAME)\
	SDPchecker$(THISMODNAME)
THISORDEXECFILES:=\
	$(addsuffix $(EXT),$(addprefix $(EXP)/$(THISMODNAME)/,$(THISORDEXECFILES)))
ORDEXECFILES+=$(THISORDEXECFILES)

#Executable files that require lapacke.h
THISLAPEXECFILES+=\
	SDPgen$(THISMODNAME)\
	SDPsolver$(THISMODNAME)\
	SDProunder$(THISMODNAME)
THISLAPEXECFILES:=\
	$(addsuffix $(EXT),$(addprefix $(EXP)/$(THISMODNAME)/,$(THISLAPEXECFILES)))
LAPEXECFILES+=$(THISLAPEXECFILES)

THISEXECFILES:=$(THISORDEXECFILES) $(THISLAPEXECFILES)
EXECFILES+=$(THISEXECFILES)

#Directory existence dependency
$(THISOBJFILES): | $(OBP)/$(THISMODNAME)
$(THISEXECFILES): | $(EXP)/$(THISMODNAME)

#Directory creation recipes
$(OBP)/$(THISMODNAME):
	$(GNUMKDIR) -p $@
$(EXP)/$(THISMODNAME):
	$(GNUMKDIR) -p $@
$(AUTOCPP)/$(THISMODNAME):
	$(GNUMKDIR) -p $@

#Header files path
vpath %.h \
	src/include/$(THISMODNAME)/

#Rule for building only this module
$(THISMODNAME): $(THISEXECFILES)

all: $(THISMODNAME)

.PHONY: $(THISMODNAME)

#Specific object dependencies
#note: the .cpp file with the same name needn't be included
$(OBP)/$(THISMODNAME)/Flag$(THISMODNAME).o: \
	Flag.h Flag$(THISMODNAME).h bignum.h
$(OBP)/$(THISMODNAME)/TestFlag$(THISMODNAME).o: \
	TestFlag.h Flag.h Flag$(THISMODNAME).h bignum.h
$(OBP)/$(THISMODNAME)/EnumerateFlag$(THISMODNAME).o: \
	Flag.h Flag$(THISMODNAME).h ExpandPrograms.h bignum.h getoptions.h
$(OBP)/$(THISMODNAME)/ExpandFlag$(THISMODNAME).o: \
	Flag.h Flag$(THISMODNAME).h ExpandPrograms.h bignum.h getoptions.h
$(OBP)/$(THISMODNAME)/ExpandProductFlag$(THISMODNAME).o: \
	Flag.h Flag$(THISMODNAME).h ExpandPrograms.h bignum.h getoptions.h
$(OBP)/$(THISMODNAME)/SDPgen$(THISMODNAME).o: \
	Flag.h Flag$(THISMODNAME).h SDPgenerate.h getoptions.h SolverStrings.h bignum.h \
	Monitoring.h
$(OBP)/$(THISMODNAME)/SDPsolver$(THISMODNAME).o: \
	Flag.h Flag$(THISMODNAME).h SDPsolver.h getoptions.h SolverStrings.h bignum.h \
	Monitoring.h
$(OBP)/$(THISMODNAME)/SDPchecker$(THISMODNAME).o: \
	Flag.h Flag$(THISMODNAME).h SDPchecker.h getoptions.h Determinant.h bignum.h
$(OBP)/$(THISMODNAME)/SDProunder$(THISMODNAME).o: \
	Flag.h Flag$(THISMODNAME).h SDProunder.h LinearSystem.h Determinant.h \
	getoptions.h bignum.h Monitoring.h

#Specific executable dependencies and recipes
#notes: only object files should be prerequisites
#       the object file with the same name needn't be included
$(EXP)/$(THISMODNAME)/TestFlag$(THISMODNAME)$(EXT): \
	$(addprefix $(OBP)/,$(THISMODNAME)/Flag$(THISMODNAME).o Flag.o bignum.o)
$(EXP)/$(THISMODNAME)/EnumerateFlag$(THISMODNAME)$(EXT): \
	$(addprefix $(OBP)/,$(THISMODNAME)/Flag$(THISMODNAME).o Flag.o bignum.o getoptions.o)
$(EXP)/$(THISMODNAME)/ExpandFlag$(THISMODNAME)$(EXT): \
	$(addprefix $(OBP)/,$(THISMODNAME)/Flag$(THISMODNAME).o Flag.o bignum.o getoptions.o)
$(EXP)/$(THISMODNAME)/ExpandProductFlag$(THISMODNAME)$(EXT): \
	$(addprefix $(OBP)/,$(THISMODNAME)/Flag$(THISMODNAME).o Flag.o bignum.o getoptions.o)
$(EXP)/$(THISMODNAME)/SDPgen$(THISMODNAME)$(EXT): \
	$(addprefix $(OBP)/,$(THISMODNAME)/Flag$(THISMODNAME).o Flag.o bignum.o \
	getoptions.o Monitoring.o)
$(EXP)/$(THISMODNAME)/SDPsolver$(THISMODNAME)$(EXT): \
	$(addprefix $(OBP)/,$(THISMODNAME)/Flag$(THISMODNAME).o Flag.o bignum.o \
	getoptions.o Monitoring.o)
$(EXP)/$(THISMODNAME)/SDPchecker$(THISMODNAME)$(EXT): \
	$(addprefix $(OBP)/,$(THISMODNAME)/Flag$(THISMODNAME).o Flag.o bignum.o getoptions.o)
$(EXP)/$(THISMODNAME)/SDProunder$(THISMODNAME)$(EXT): \
	$(addprefix $(OBP)/,$(THISMODNAME)/Flag$(THISMODNAME).o Flag.o bignum.o \
	getoptions.o Monitoring.o)

#.cpp automatic generating rules
define ruleForAuto
$(AUTOCPP)/$(THISMODNAME)/$(1)$(THISMODNAME).cpp: \
	| $(AUTOCPP)/CommonProgBase/$(1)BASE.cpp $(AUTOCPP)/$(THISMODNAME)
	@echo Generating $$@
	@sed s/BASE/$(THISMODNAME)/g <$(AUTOCPP)/CommonProgBase/$(1)BASE.cpp >$$@
endef

AUTOFILES:=\
	TestFlag\
	EnumerateFlag\
	ExpandFlag\
	ExpandProductFlag\
	SDPgen\
	SDPsolver\
	SDPchecker\
	SDProunder

$(foreach VAR, $(AUTOFILES), $(eval $(call ruleForAuto,$(VAR))))
