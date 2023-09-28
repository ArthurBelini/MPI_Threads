GCC = gcc
MPICC = mpicc
GCCFLAGS =
MPICCFLAGS =

all: trmm_s trmm_t trmm_m trmm_mt

trmm_s: polybench.c trmm_s.c
	$(GCC) $(CFLAGS) -o $@ $^

trmm_t: polybench.c trmm_t.c
	$(GCC) $(CFLAGS) -o $@ $^

trmm_m: polybench.c trmm_m.c
	$(MPICC) $(MPICCFLAGS) -o $@ $^

trmm_mt: polybench.c trmm_mt.c
	$(MPICC) $(MPICCFLAGS) -o $@ $^

clean:
	rm -f trmm_s trmm_t trmm_m trmm_mt