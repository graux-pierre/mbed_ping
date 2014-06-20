OUTPUT_NAME=$(shell basename $(shell pwd))
SRC=$(wildcard *.c)
OBJS=$(SRC:.c=.o)

# Modify this variable at your own risk
RFLPC_DIR=../rflpc
include $(RFLPC_DIR)/Makefile.in

