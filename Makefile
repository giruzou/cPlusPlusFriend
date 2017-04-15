# コンソール出力に色を付ける
ECHO_START="\e[104m
ECHO_END=\e[0m"

# Google Test / Google Mockがあるディレクトリ
GTEST_GMOCK_TOP_DIR=$(HOME)/googletest
GTEST_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googletest
GMOCK_TOP_DIR=$(GTEST_GMOCK_TOP_DIR)/googlemock
GTEST_GMOCK_INCLUDE=$(addprefix -isystem, $(GTEST_TOP_DIR)/include $(GTEST_TOP_DIR) $(GMOCK_TOP_DIR)/include $(GMOCK_TOP_DIR))
GTEST_SOURCE=$(GTEST_TOP_DIR)/src/gtest-all.cc
GMOCK_SOURCE=$(GMOCK_TOP_DIR)/src/gmock-all.cc
GTEST_OBJ=$(patsubst %.cc, %.o, $(notdir $(GTEST_SOURCE)))
GMOCK_OBJ=$(patsubst %.cc, %.o, $(notdir $(GMOCK_SOURCE)))

TARGET=cppFriends
TARGET_C=cFriends
TARGETS=$(TARGET) $(TARGET_C)
OUTPUT_ASM87_C=cFriends87.s
OUTPUT_ASM87_STORE_C=cFriends87-store.s
OUTPUT_ASM64_C=cFriends64.s
OUTPUT_ASMS=$(OUTPUT_ASM87_C) $(OUTPUT_ASM87_STORE_C) $(OUTPUT_ASM64_C)

CASMFLAGS=-S -masm=intel
CASMFLAGS_87=-mfpmath=387 -mno-sse
CASMFLAGS_87_STORE=-mfpmath=387 -mno-sse -ffloat-store
CASMFLAGS_64=

SOURCE=cppFriends.cpp
SOURCE_C=cFriends.c
SOURCE_ERROR=cppFriendsError.cpp

OBJ=cppFriends.o
OBJS=$(OBJ) $(GTEST_OBJ) $(GMOCK_OBJ)
VPATH=$(dir $(GTEST_SOURCE) $(GMOCK_SOURCE))

CPP=gcc
CXX=g++
LD=g++
CFLAGS=-std=gnu11 -O2 -Wall
CPPFLAGS=-std=gnu++14 -O2 -Wall $(GTEST_GMOCK_INCLUDE)
LIBPATH=
LDFLAGS=
LIBS=-lboost_serialization

.PHONY: all test clean cprog force
.SUFFIXES: .o .cpp .cc

all: $(TARGETS) force

$(TARGET): $(OBJS)
	$(LD) $(LIBPATH) -o $@ $^ $(LDFLAGS) $(LIBS)
	./$@

cprog: $(TARGET_C)

$(TARGET_C): $(SOURCE_C)
	$(CPP) $(CFLAGS) $(CASMFLAGS) $(CASMFLAGS_87) -o $(OUTPUT_ASM87_C)  $<
	$(CPP) $(CFLAGS) $(CASMFLAGS) $(CASMFLAGS_87_STORE) -o $(OUTPUT_ASM87_STORE_C)  $<
	$(CPP) $(CFLAGS) $(CASMFLAGS) $(CASMFLAGS_64) -o $(OUTPUT_ASM64_C)  $<
	$(CPP) $(CFLAGS) $(CASMFLAGS_87) -o $@ $<
	@echo -e $(ECHO_START)Using x87 and compile with $(CASMFLAGS_87) $(ECHO_END)
	@./$@
	$(CPP) $(CFLAGS) $(CASMFLAGS_87_STORE) -o $@ $<
	@echo -e $(ECHO_START)Using x87 and compile with $(CASMFLAGS_87_STORE) $(ECHO_END)
	@./$@
	$(CPP) $(CFLAGS) $(CASMFLAGS_64) -o $@ $<
	@echo -e $(ECHO_START)Using x64 and SSE $(ECHO_END)
	@./$@
	-$(CXX) $(CPPFLAGS) -c $<

force : $(SOURCE_ERROR)
	-$(CXX) $(CPPFLAGS) -c $<

$(OBJ): $(SOURCE)
	$(CXX) $(CPPFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: %.cc
	$(CXX) $(CPPFLAGS) -o $@ -c $<

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGETS) $(OBJS) $(OUTPUT_ASMS) ./*.o ./*.s
