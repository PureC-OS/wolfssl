# PureC OS port build for wolfSSL (wolfcrypt subset, phase 1).
# Targets:
#   lib    -> bin/lib/libwolf.a          ring 3 static library
#   module -> bin/modules/wolf.{elf,ko,so}  kernel module triple
#   check-crypt -> wolfcrypt self-test on the host

ROOT_DIR ?= $(abspath ..)
include $(ROOT_DIR)/mk/toolchain.mk

WL := $(CURDIR)
SRC := $(WL)/wolfcrypt/src
PORT := $(WL)/purec

OBJ_DIR := $(BIN_DIR)/obj/wolfssl
USER_OBJ_DIR := $(OBJ_DIR)/user
KERN_OBJ_DIR := $(OBJ_DIR)/kernel
SO_OBJ_DIR := $(OBJ_DIR)/kernel_pic
LIB_DIR := $(BIN_DIR)/lib
MODULE_DIR := $(BIN_DIR)/modules

CPPFLAGS += -I$(WL) -I$(PORT) -DWOLFSSL_USER_SETTINGS

# Sources shared by library and kernel module build.
COMMON_SOURCES := \
	aes.c \
	chacha.c \
	chacha20_poly1305.c \
	poly1305.c \
	coding.c \
	cmac.c \
	error.c \
	hash.c \
	hmac.c \
	kdf.c \
	memory.c \
	pwdbased.c \
	random.c \
	sha.c \
	sha256.c \
	sha512.c \
	wc_encrypt.c \
	wc_port.c

# Userspace library extras: public key crypto and certificates.
USER_SOURCES := $(COMMON_SOURCES) \
	asn.c \
	ecc.c \
	rsa.c \
	signature.c \
	wolfmath.c \
	sp_int.c
KERNEL_SOURCES := $(COMMON_SOURCES)
KERNEL_GLUE := memory_kernel.c
USER_GLUE := memory_userspace.c
USER_SEED := seed_purec.c

COMMON_OBJECTS := $(COMMON_SOURCES:%.c=$(USER_OBJ_DIR)/%.o)
USER_OBJECTS := $(USER_SOURCES:%.c=$(USER_OBJ_DIR)/%.o)
USER_OBJECTS += $(USER_OBJ_DIR)/memory_userspace.o
USER_OBJECTS += $(USER_OBJ_DIR)/seed_purec.o
USER_OBJECTS += $(USER_OBJ_DIR)/snprintf_purec.o
KERNEL_OBJECTS := $(KERNEL_SOURCES:%.c=$(KERN_OBJ_DIR)/%.o)
KERNEL_OBJECTS += $(KERN_OBJ_DIR)/memory_kernel.o
KERNEL_OBJECTS += $(KERN_OBJ_DIR)/seed_purec.o
SO_OBJECTS := $(USER_OBJECTS:$(USER_OBJ_DIR)/%=$(SO_OBJ_DIR)/%)

USER_TARGET := $(LIB_DIR)/libwolf.a
USER_CFLAGS_W := $(USER_CFLAGS) -include $(PORT)/purec_snprintf.h
USER_HOST_FLAGS := -std=c11 -Wall -Wextra -O2 -g \
	-I$(WL) -I$(PORT) -DWOLFSSL_USER_SETTINGS
HOST_OBJ_DIR := $(OBJ_DIR)/host
HOST_TEST := $(BIN_DIR)/tests/wolfcrypt_test

MOD_CFLAGS := $(KERNEL_CFLAGS) -DWOLF_PUREC_KERNEL

.PHONY: all lib module clean check-crypt

all: lib module

lib: $(USER_TARGET)

module: $(MODULE_DIR)/wolf.elf $(MODULE_DIR)/wolf.ko $(MODULE_DIR)/wolf.so

$(USER_TARGET): $(USER_OBJECTS)
	@mkdir -p $(@D)
	$(AR) rcs $@ $^
	@echo "libwolf -> $@"

$(USER_OBJ_DIR)/%.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(USER_CFLAGS_W) -c $< -o $@

$(USER_OBJ_DIR)/memory_userspace.o: $(PORT)/memory_userspace.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(USER_CFLAGS_W) -c $< -o $@

$(USER_OBJ_DIR)/seed_purec.o: $(PORT)/seed_purec.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(USER_CFLAGS_W) -c $< -o $@

$(USER_OBJ_DIR)/snprintf_purec.o: $(PORT)/snprintf_purec.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(USER_CFLAGS_W) -c $< -o $@

$(KERN_OBJ_DIR)/%.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(MOD_CFLAGS) -c $< -o $@

$(KERN_OBJ_DIR)/memory_kernel.o: $(PORT)/$(KERNEL_GLUE)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(MOD_CFLAGS) -c $< -o $@

$(KERN_OBJ_DIR)/seed_purec.o: $(PORT)/$(USER_SEED)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(MOD_CFLAGS) -c $< -o $@

$(MODULE_DIR)/wolf.elf: $(KERNEL_OBJECTS)
	@mkdir -p $(@D)
	$(LD) -r -o $@ $(KERNEL_OBJECTS)
	@echo "wolf module -> $@"

$(MODULE_DIR)/wolf.ko: $(MODULE_DIR)/wolf.elf
	@cp $< $@
	@echo "wolf module -> $@"

$(MODULE_DIR)/wolf.so: $(SO_OBJECTS) $(KMOD_SO_LDSCRIPT)
	@mkdir -p $(@D)
	$(LD) $(KMOD_SO_LINK) -soname wolf.so -o $@ $(SO_OBJECTS)
	@echo "wolf module -> $@"

$(SO_OBJ_DIR)/%.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(KMOD_SO_CFLAGS) -DWOLF_PUREC_KERNEL -c $< -o $@

$(SO_OBJ_DIR)/memory_kernel.o: $(PORT)/$(KERNEL_GLUE)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(KMOD_SO_CFLAGS) -DWOLF_PUREC_KERNEL -c $< -o $@

$(SO_OBJ_DIR)/memory_userspace.o: $(PORT)/$(USER_GLUE)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(KMOD_SO_CFLAGS) -DWOLF_PUREC_KERNEL -c $< -o $@

$(SO_OBJ_DIR)/seed_purec.o: $(PORT)/$(USER_SEED)
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(KMOD_SO_CFLAGS) -DWOLF_PUREC_KERNEL -c $< -o $@

$(SO_OBJ_DIR)/snprintf_purec.o: $(PORT)/snprintf_purec.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(DEPFLAGS) $(KMOD_SO_CFLAGS) -DWOLF_PUREC_KERNEL -c $< -o $@

HOST_OBJECTS := $(USER_SOURCES:%.c=$(HOST_OBJ_DIR)/%.host.o)
HOST_OBJECTS += $(HOST_OBJ_DIR)/memory_host.host.o

$(HOST_OBJ_DIR)/%.host.o: $(SRC)/%.c
	@mkdir -p $(@D)
	$(HOST_CC) $(USER_HOST_FLAGS) -c $< -o $@

$(HOST_OBJ_DIR)/memory_host.host.o: $(PORT)/memory_host.c
	@mkdir -p $(@D)
	$(HOST_CC) $(USER_HOST_FLAGS) -c $< -o $@

$(HOST_TEST): $(HOST_OBJECTS) $(WL)/wolfcrypt/test/test.c
	@mkdir -p $(@D)
	$(HOST_CC) $(USER_HOST_FLAGS) $(HOST_OBJECTS) \
		$(WL)/wolfcrypt/test/test.c -o $@

check-crypt: $(HOST_TEST)
	$(HOST_TEST)

clean:
	rm -rf $(OBJ_DIR) $(USER_TARGET) $(MODULE_DIR)/wolf.elf \
		$(MODULE_DIR)/wolf.ko $(MODULE_DIR)/wolf.so

-include $(USER_OBJECTS:.o=.d)
-include $(KERNEL_OBJECTS:.o=.d)
-include $(SO_OBJECTS:.o=.d)
