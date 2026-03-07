# SYNOPSIS:
#
#   make [all]  	- makes everything.
#   make TARGET 	- makes the given target.
#   make clean  	- removes all files except executables.
#	make distclean 	- removes executables.

# Points to the root of Google Test, relative to where this file is.
GTEST_DIR=../googletest/googletest

# Where to find user code.
USER_DIR=.

# All Google Test headers.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

# Compilers. Use MPICXX to compile MPI code and CXX otherwise.
CXX = g++
ifeq ($(WITH_MPI),1)
    CXX = mpicxx
    CXXFLAGS += -DWITH_MPI
endif

# Flags to compiler, defined as separate variables for ease of use.
CXXFLAGS += -std=c++17 -g
CXXFLAGS += -Wall -Wextra -Wno-unknown-pragmas -Wno-sign-compare
OMPFLAGS =
FFTWFLAGS = -lfftw3 -lm
FFTWOMPFLAGS = -lfftw3 -lfftw3_omp -lm
NETCDFFLAGS = -lnetcdf_c++4 -lnetcdf
ifeq ($(DEBUG),1)
	CXXFLAGS += -O0 -fsanitize=undefined -fno-sanitize-recover=all
	CXXFLAGS += -DDEBUG
else
	CXXFLAGS += -O2 -march=native 
endif
ifeq ($(WITH_OMP),1)
    OMPFLAGS = -fopenmp
    CXXFLAGS += -DWITH_OMP
endif

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler for googletest.
GTEST_CXXFLAGS += -std=c++17 -g -Wall -Wextra -pthread

# Path for linking.
INCLUDE = -I./include -I/usr/local/include -L/usr/local/lib

# House-keeping build targets.
all: main

clean:
	$(RM) *.o gtest.a gtest_main.a

distclean:
	$(RM) stft_unittest main verifier

# Builds gtest.a and gtest_main.a.
# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(GTEST_CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(GTEST_CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^	

# Builds the project.
main: main.o stft.o stft_io.o
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) $(INCLUDE) $^ $(NETCDFFLAGS) -o $@

main.o: main.cc
	$(CXX) $(INCLUDE) -c $(CXXFLAGS) $(OMPFLAGS) -o $@ $<

stft.o: stft.cc stft.h
	$(CXX) $(INCLUDE) -c $(CXXFLAGS) $(OMPFLAGS) -o $@ $<	

stft_io.o: stft_io.cc stft_io.h
	$(CXX) $(INCLUDE) -c $(CXXFLAGS) -o $@ $<		

verifier: verifier.o  stft_io.o
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) $(INCLUDE) $^ $(NETCDFFLAGS) $(FFTWOMPFLAGS) -o $@

verifier.o: verifier.cc
	$(CXX) $(INCLUDE) -c $(CXXFLAGS) $(OMPFLAGS) $(FFTWOMPFLAGS) -o $@ $<

debug: test.o
	$(CXX) $(CXXFLAGS) $(OMPFLAGS) $^ -o $@

test.o: test.cc
	$(CXX) -c $(CXXFLAGS) $(OMPFLAGS) -o $@ $<

# Builds unit test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.
stft_unittest.o : $(USER_DIR)/stft_unittest.cc $(GTEST_HEADERS)
	$(CXX) $(INCLUDE) $(CPPFLAGS) $(GTEST_CXXFLAGS) $(FFTWFLAGS) -c $(USER_DIR)/stft_unittest.cc

stft_unittest : stft_unittest.o stft.o gtest_main.a
	$(CXX) $^ $(FFTWFLAGS) $(INCLUDE) $(CPPFLAGS) $(GTEST_CXXFLAGS) $(OMPFLAGS) -o $@	

test: stft_unittest
	./stft_unittest

.PHONY: all clean distclean test
