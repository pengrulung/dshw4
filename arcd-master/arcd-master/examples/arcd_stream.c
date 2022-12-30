#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "arcd.h"
#include "adaptive_model.h"
double pi(int n) {  //calculate time
  srand(5);
  int count = 0;
  double x, y;
  for (int i = 0; i < n; ++i) {
    x = (double) rand() / RAND_MAX;
    y = (double) rand() / RAND_MAX;
    if (x * x + y * y <= 1) ++count;
  }
  return (double) count / n * 4;
}
void output(const arcd_buf_t buf, const unsigned buf_bits, void *const io)
{
	(void)buf_bits;
	FILE *const f = (FILE *)io;
	fwrite(&buf, sizeof(buf), 1, f);
}

unsigned input(arcd_buf_t *const buf, void *const io)
{
	FILE *const f = (FILE *)io;
	return 8 * fread(buf, sizeof(*buf), 1, f);
}

void usage(FILE *const out)
{
	fprintf(out, "Usage:\n");
	fprintf(out, "    arcd_stream [-e | -d | -h]\n\n");
	fprintf(out, "-e - encode stdin to stdout\n");
	fprintf(out, "-d - decode stdin to stdout\n");
	fprintf(out, "-h - help\n\n");
	fflush(out);
}

typedef unsigned char symbol_t;
static const arcd_char_t EOS = 1 << (8 * sizeof(symbol_t));

int main(int argc, char *argv[])
{
	clock_t start, end; //cal time
  	double cpu_time_used; //cal time
	double result = pi(1e8); //cal
	if (2 != argc)
	{
		usage(stderr);
		return 1;
	}
	if (0 == strcmp("-h", argv[1]))
	{
		usage(stdout);
		return 0;
	}
	FILE *const in = fdopen(dup(fileno(stdin)), "rb");
	FILE *const out = fdopen(dup(fileno(stdout)), "wb");
	adaptive_model model;
	adaptive_model_create(&model, EOS + 1);
	if (0 == strcmp("-e", argv[1]))
	{
		start = clock(); //cal
		arcd_enc enc;
		arcd_enc_init(&enc, adaptive_model_getprob, &model, output, out);
		symbol_t sym;
		while (0 < fread(&sym, sizeof(sym), 1, in))
		{
			arcd_enc_put(&enc, sym);
		}
		arcd_enc_put(&enc, EOS);
		arcd_enc_fin(&enc);
	}
	else if (0 == strcmp("-d", argv[1]))
	{
		start = clock(); //cal
		arcd_dec dec;
		arcd_dec_init(&dec, adaptive_model_getch, &model, input, in);
		arcd_char_t ch;
		while (EOS != (ch = arcd_dec_get(&dec)))
		{
			const symbol_t sym = (unsigned char)ch;
			fwrite(&sym, sizeof(sym), 1, out);
		}
	}
	end = clock(); //cal time end
	adaptive_model_free(&model);
	fclose(in);
	fclose(out);
	
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  	printf("\nPI = %f\n", result);
  	printf("Time = %f\n", cpu_time_used);
	return 0;
}
