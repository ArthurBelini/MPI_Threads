GCC = gcc
MPICC = mpicc
GCCFLAGS =
MPICCFLAGS =

all: trmm_s trmm_t trmm_m trmm_mt

trmm_s: trmm_s.c
	$(GCC) $(CFLAGS) -o $@ $^

trmm_t: trmm_t.c
	$(GCC) $(CFLAGS) -o $@ $^

trmm_m: trmm_m.c
	$(MPICC) $(MPICCFLAGS) -o $@ $^

trmm_mt: trmm_mt.c
	$(MPICC) $(MPICCFLAGS) -o $@ $^

clean:
	rm -f trmm_s trmm_t trmm_m trmm_mt