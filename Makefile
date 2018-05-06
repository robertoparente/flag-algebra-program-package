#Default
default: all

#C++ compiler
CXX=g++

#C++ include flags
override CXXINCLUDE:=$(CXXINCLUDE)
override CXXINCLUDE+=\
	-Isrc/include/Flag/\
	-Isrc/include/Flag/SDPprograms/\
	-Isrc/include/other/

#link llapacke
LAPACKELIB=-llapacke

#Optimization flags
OPTFLAGS=-O2

#C++ compiler flags
override CXXFLAGS+=-Wall -ansi -pedantic $(OPTFLAGS)

#mkdir command (must have GNU syntax)
GNUMKDIR=mkdir

#Binary files path prefix
BIN=bin/

#Object files path
#Since object files may be deleted, this has to be done manually
OBP=$(BIN)objects

#Executable extension
EXT=

#Executable files path
#Since executable files may be deleted, this has to be done manually
EXP=$(BIN)executables

#Automatically generated .cpp files base path
AUTOCPP=src/programs

#Object files
OBJFILES:=\
	bignum.o\
	getoptions.o\
	Flag.o\
	echoedges.o\
	testGetOptions.o\
	Monitoring.o
OBJFILES:=$(addprefix $(OBP)/,$(OBJFILES))

#Ordinary executable files
ORDEXECFILES:=\
	echoedges\
	testGetOptions
ORDEXECFILES:=$(addsuffix $(EXT),$(addprefix $(EXP)/,$(ORDEXECFILES)))

#Executable files that require lapacke.h
LAPEXECFILES:=
LAPEXECFILES:=$(addsuffix $(EXT),$(addprefix $(EXP)/,$(LAPEXECFILES)))

EXECFILES:=$(ORDEXECFILES) $(LAPEXECFILES)

#Compile everything
all: $(EXECFILES)

#Header files path
vpath %.h \
	src/include/Flag/\
	src/include/Flag/SDPprograms/\
	src/include/other/

#Header templates files path
vpath %Templates.cpp \
	src/include/Flag/\
	src/include/Flag/SDPprograms/\
	src/include/other/

#Source files path
vpath %.cpp \
	src/lib/\
	src/programs/\
	src/lib/Flag/\
	src/lib/other/\
	src/programs/Utilities/

#Templated libraries (must have full path)
TEMPLIB:=\
	src/include/other/injective.h\
	src/include/other/selector.h\
	src/include/other/LinearSystem.h\
	src/include/other/Determinant.h\
	src/include/Flag/Flag.h\
	src/include/Flag/TestFlag.h\
	src/include/Flag/ExpandPrograms.h\
	src/include/Flag/SDPprograms/SDPgenerate.h\
	src/include/Flag/SDPprograms/SDPsolver.h\
	src/include/Flag/SDPprograms/SDPchecker.h\
	src/include/Flag/SDPprograms/SDProunder.h

#Flag modules available
FLAGMODULES:=\
	Graph\
	Digraph\
	Tournament


#FlagGeneric makefile includes
define INCGEN
THISMODNAME:=$(1)
THISOBJFILES:=
THISORDEXECFILES:=
THISLAPEXECFILES:=
-include makefiles/$$(THISMODNAME).make
include makefiles/FlagGeneric.make
endef

$(foreach VAR, $(FLAGMODULES), $(eval $(call INCGEN,$(VAR))))



#Clean all
clean: cleanobj cleanexec cleantildes

#Clean object files
cleanobj:
	rm -f -- $(OBJFILES)

#Clean temporary files
cleantildes:
	rm -f -- $(wildcard *~ */*~ */*/*~ */*/*/*~ */*/*/*/*~)
#The above is done this way to avoid use of shopt and **

#Clean executables
cleanexec:
	rm -f -- $(EXECFILES)

.PHONY: default all clean cleanobj cleantildes cleanexec

#Generic object rule
$(OBJFILES): $(OBP)/%.o: %.cpp
	@echo $(CXX) $(CXXFLAGS) -I[...] $< -c -o $@
	@$(CXX) $(CXXFLAGS) $(CXXINCLUDE) $< -c -o $@

#Generic ordinary executable recipe
$(ORDEXECFILES): $(EXP)/%$(EXT): $(OBP)/%.o
	$(CXX) $(CXXFLAGS) $^ -o $@

#Generic lapacke linking executable recipe
$(LAPEXECFILES): $(EXP)/%$(EXT): $(OBP)/%.o
	$(CXX) $(CXXFLAGS) $^ $(LAPACKELIB) -o $@

#Generic template recipe
$(TEMPLIB): %.h: %Templates.cpp
	touch $@ -c -r $<

#Directory existence dependency
$(OBJFILES): | $(OBP)
$(EXECFILES): | $(EXP)

#Directory creation recipes
$(OBP):
	$(GNUMKDIR) -p $@
$(EXP):
	$(GNUMKDIR) -p $@

#Specific object dependencies
#note: the .cpp file with the same name needn't be included
$(OBP)/bignum.o: bignum.h
$(OBP)/getoptions.o: getoptions.h
$(OBP)/testGetOptions.o: getoptions.h
$(OBP)/Flag.o: Flag.h bignum.h injective.h selector.h bignum.h
$(OBP)/Monitoring.o: Monitoring.h

#Specific executable dependencies and recipes
#notes: only object files should be prerequisites
#       the object file with the same name needn't be included
$(EXP)/testGetOptions$(EXT): $(addprefix $(OBP)/,getoptions.o)
