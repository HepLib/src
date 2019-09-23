osName := $(shell uname -s)

ifeq ($(osName),Darwin)
opt = -w -std=gnu++14 -fPIC -I ../include -L .. -L /usr/local/feng/lib -I /usr/local/feng/include
else
opt = -w -std=gnu++14 -fPIC -I ../include -L .. -I. -L $$HOME/usr/local/lib -L $$HOME/usr/local/lib64 -I $$HOME/usr/local/include -L $$HOME/glibc/lib -L /WORK/app/glibc/2.14/lib
endif

ifeq ($(osName),Darwin)
prefix = /usr/local/feng
else
prefix = $$HOME/usr/local
endif





