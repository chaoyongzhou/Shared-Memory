################################################################################
#
# Copyright (C) Chaoyong Zhou
# Email: bgnvendor@163.com
# QQ: 2796796
#
################################################################################

#ROOT_DIR = $(shell pwd)
ROOT_DIR = .

MBITS = $(shell getconf LONG_BIT)

LIB_DIR  = $(ROOT_DIR)/lib
OBJ_DIR  = $(ROOT_DIR)/obj
BIN_DIR  = $(ROOT_DIR)/bin
TOOL_DIR = $(ROOT_DIR)/tool

EXT_INC = 
EXT_LIB = -lm -lrt -lz -rdynamic -ldl -lpthread

SPSC_SRC = $(ROOT_DIR)/spsc_src
SPSC_INC = -I $(ROOT_DIR)/spsc_inc
SPSC_LIB = -L $(LIB_DIR) -lspsc

MPSC_SRC = $(ROOT_DIR)/mpsc_src
MPSC_INC = -I $(ROOT_DIR)/mpsc_inc
MPSC_LIB = -L $(LIB_DIR) -lmpsc

MISC_SRC = $(ROOT_DIR)/misc_src
MISC_INC = -I $(ROOT_DIR)/misc_inc

TST_SRC = $(ROOT_DIR)/test
TST_INC =

CMACRO = \
         -D_GNU_SOURCE \
         -D_LARGEFILE_SOURCE \
         -D_LARGEFILE64_SOURCE \
         -D_FILE_OFFSET_BITS=64 \
         -DSWITCH_ON=1 \
         -DSWITCH_OFF=0 \

# C Language
CC       = /usr/bin/gcc
CLINKER  = /usr/bin/gcc

# C++ Language
#CC       = /usr/bin/g++
#CLINKER  = /usr/bin/g++

AR       = /usr/bin/ar cr

# ignore Wunused-function, Wuninitialized, Wunused-parameter
CWARNS   = -Wall -Werror

CFLAGS   = ${CWARNS} -g -m${MBITS} ${CMACRO} -fstack-check
#CFLAGS   = ${CWARNS} -g -O2 -m${MBITS} ${CMACRO} -fstack-check

SPSC_OBJS = $(patsubst $(SPSC_SRC)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(SPSC_SRC)/*.c))
MPSC_OBJS = $(patsubst $(MPSC_SRC)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(MPSC_SRC)/*.c))
MISC_OBJS = $(patsubst $(MISC_SRC)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(MISC_SRC)/*.c))
TST_OBJS  = $(patsubst $(TST_SRC)/%.c, $(OBJ_DIR)/%.o, $(wildcard $(TST_SRC)/*.c))

libspsc: $(SPSC_OBJS) $(MISC_OBJS)
	$(AR) $(LIB_DIR)/libspsc.a $(SPSC_OBJS) $(MISC_OBJS)

libmpsc: $(MPSC_OBJS) $(MISC_OBJS)
	$(AR) $(LIB_DIR)/libmpsc.a $(MPSC_OBJS) $(MISC_OBJS)

test_cspscq: libspsc $(OBJ_DIR)/test_cspscq.o
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test $(OBJ_DIR)/test_cspscq.o $(SPSC_INC) $(MISC_INC) $(SPSC_LIB) $(EXT_LIB)

test_cspscrb: libspsc $(OBJ_DIR)/test_cspscrb.o
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test $(OBJ_DIR)/test_cspscrb.o $(SPSC_INC) $(MISC_INC) $(SPSC_LIB) $(EXT_LIB)

test_cmpscq: libmpsc $(OBJ_DIR)/test_cmpscq.o
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test $(OBJ_DIR)/test_cmpscq.o $(MPSC_INC) $(MISC_INC) $(MPSC_LIB) $(EXT_LIB)

test_cmpscrb: libmpsc $(OBJ_DIR)/test_cmpscrb.o
	$(CLINKER) $(CFLAGS) -o $(BIN_DIR)/test $(OBJ_DIR)/test_cmpscrb.o $(MPSC_INC) $(MISC_INC) $(MPSC_LIB) $(EXT_LIB)

$(SPSC_OBJS): $(OBJ_DIR)/%.o: $(SPSC_SRC)/%.c
	$(CC) $(CFLAGS) -c $< $(SPSC_INC) $(MISC_INC) $(EXT_INC) -o $@

$(MPSC_OBJS): $(OBJ_DIR)/%.o: $(MPSC_SRC)/%.c
	$(CC) $(CFLAGS) -c $< $(MPSC_INC) $(MISC_INC) $(EXT_INC) -o $@

$(MISC_OBJS): $(OBJ_DIR)/%.o: $(MISC_SRC)/%.c
	$(CC) $(CFLAGS) -c $< $(MISC_INC) $(EXT_INC) -o $@

$(TST_OBJS): $(OBJ_DIR)/%.o: $(TST_SRC)/%.c
	$(CC) $(CFLAGS) -c $< $(TST_INC) $(SPSC_INC) $(MPSC_INC) $(MISC_INC) $(EXT_INC) -o $@

clean-obj:
	$(if $(wildcard $(OBJ_DIR)/*.o), rm -f $(OBJ_DIR)/*.o,)	

clean-lib:
	$(if $(wildcard $(LIB_DIR)/*.a),rm -f $(LIB_DIR)/*.a,)	
	$(if $(wildcard $(LIB_DIR)/*.so),rm -f $(LIB_DIR)/*.so,)

clean-log:
	$(if $(wildcard $(BIN_DIR)/*.log),rm -f $(BIN_DIR)/*.log,)	

clean-bin:
	$(if $(wildcard $(BIN_DIR)/test),rm -f $(BIN_DIR)/test,)

clean-trial: clean-log clean-bin clean-obj
	
clean: clean-log clean-bin clean-obj clean-lib
