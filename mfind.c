/*
    mfind - an rlifesrc-like program for finding oscillators in INT rules.
    Copyright (C) 2026 MDA

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#include "pseudo.h"

#define MAX_FOUND 10000

int testing = 0;

int depth = 0;
int found = 0;

char rule[274] = {0};

int bconds[117] = {0};
int sconds[117] = {0};

FILE * results;

void build_max_layers(unsigned long long (* max_layers)[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	for (int n = 1; n <= (DIAG_WIDTH / 2) + 1; n++) {
		(*max_layers)[n - 1] = 1 << ((n + 1) / 2);
	}

	for (int n = (DIAG_WIDTH / 2) + 2; n <= DIAG_WIDTH; n++) {
		(*max_layers)[n - 1] = 1 << ((DIAG_WIDTH - n + 2) / 2);
	}
}

void layers_to_pattern(unsigned long long layers[], unsigned long long (* pattern)[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	for (int y = 0; y < DIM; y++) {
		(*pattern)[y] = 0;
	}

	for (int n = 0; n <= (DIAG_WIDTH / 2); n++) {
		int m = n + 1;

		for (int nn = 0; nn < m; nn++) {
			int bin = (layers[n] >> nn) & 1;
			(*pattern)[PERIOD + n - nn] |= bin << (PERIOD + nn);
		}
	}

	for (int n = (DIAG_WIDTH / 2) + 1; n < DIAG_WIDTH; n++) {
		int m = DIAG_WIDTH - n;

		for (int nn = 0; nn < m; nn++) {
			int bin = (layers[n] >> nn) & 1;
			(*pattern)[PERIOD + MINPOP - nn - 1] |= bin << (PERIOD + (n - (DIAG_WIDTH / 2)) + nn);
		}
	}
}

void pattern_to_layers(unsigned long long pattern[], unsigned long long (* layers)[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	for (int n = 0; n < DIAG_WIDTH; n++) {
		(*layers)[n] = 0;
	}

	for (int n = 0; n < MINPOP; n++) {
		int m = n + 1;

		for (int nn = 0; nn < m; nn++) {
			(*layers)[n] += ((pattern[PERIOD + n - nn] >> (PERIOD + nn)) & 1) * (1 << nn);
		}
	}

	for (int n = 1; n < MINPOP; n++) {
		int m = MINPOP - n;

		for (int nn = 0; nn < m; nn++) {
			(*layers)[MINPOP + n - 1] += ((pattern[PERIOD + MINPOP - nn - 1] >> (PERIOD + n + nn)) & 1) * (1 << nn);
		}
	}
}

int in_letters(char c) {
	char letters[] = "cekainyqjrtwz";

	for (int n = 0; n < 13; n++) {
		if (letters[n] == c) return n;
	}

	return -1;
}

void rparse(int (* bconds)[], int (* sconds)[]) {
	int lastdigit = 0;

	int birth = 0;
	int survival = 0;

	int negating = 0;

	int i;

	char c;
	char letters[] = "cekainyqjrtwz";

	for (int n = 0; n < 273; n++) {
		c = rule[n];

		switch (c) {
			case '0':;
			case '1':;
			case '2':;
			case '3':;
			case '4':;
			case '5':;
			case '6':;
			case '7':;
			case '8':
				lastdigit = c - '0';
				negating = 0;

				for (int nn = 0; nn < 13; nn++) {
					if (birth) {
						(*bconds)[(lastdigit * 13) + nn] = 2;
					} else if (survival) {
						(*sconds)[(lastdigit * 13) + nn] = 2;
					}
				}

				break;
			case 'B':
				birth = 1;
				survival = 0;
				break;
			case 'S':
				birth = 0;
				survival = 1;
				break;
			case 'c':;
			case 'e':;
			case 'k':;
			case 'a':;
			case 'i':;
			case 'n':;
			case 'y':;
			case 'q':;
			case 'j':;
			case 'r':;
			case 't':;
			case 'w':;
			case 'z':
				i = in_letters(c);

				if (negating) {
					if (birth) {
						(*bconds)[(lastdigit * 13) + i] = 0;
					} else if (survival) {
						(*sconds)[(lastdigit * 13) + i] = 0;
					}
				} else {
					for (int nn = 0; nn < 13; nn++) {
						if (birth) {
							if ((*bconds)[(lastdigit * 13) + nn] == 2) (*bconds)[(lastdigit * 13) + nn] = 0;

							if (nn == i) (*bconds)[(lastdigit * 13) + nn] = 1;
						} else if (survival) {
							if ((*sconds)[(lastdigit * 13) + nn] == 2) (*sconds)[(lastdigit * 13) + nn] = 0;

							if (nn == i) (*sconds)[(lastdigit * 13) + nn] = 1;
						}
					}
				}

				break;
			case '-':
				negating = 1;
			default:
				break;
		}
	}
}

void advance(unsigned long long (* pattern)[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	int dict[256] = {0, 13, 14, 29, 13, 26, 29, 43, 14, 29, 27, 42, 28, 44, 47, 55, 14, 28, 27, 47, 29, 44, 42, 55, 30, 48, 40, 61, 48, 56, 61, 69, 13, 26, 28, 44, 31, 39, 46, 57, 29, 43, 47, 55, 46, 57, 63, 68, 28, 45, 41, 54, 46, 58, 59, 73, 48, 62, 60, 70, 64, 74, 72, 81, 14, 28, 30, 48, 28, 45, 48, 62, 27, 47, 40, 61, 41, 54, 60, 70, 27, 41, 40, 60, 47, 54, 61, 70, 40, 60, 53, 65, 60, 71, 65, 78, 29, 44, 48, 56, 46, 58, 64, 74, 42, 55, 61, 69, 59, 73, 72, 81, 47, 54, 60, 71, 63, 67, 72, 80, 61, 70, 65, 78, 72, 80, 83, 91, 13, 31, 28, 46, 26, 39, 44, 57, 28, 46, 41, 59, 45, 58, 54, 73, 29, 46, 47, 63, 43, 57, 55, 68, 48, 64, 60, 72, 62, 74, 70, 81, 26, 39, 45, 58, 39, 52, 58, 66, 44, 57, 54, 73, 58, 66, 67, 79, 44, 58, 54, 67, 57, 66, 73, 79, 56, 74, 71, 80, 74, 82, 80, 92, 29, 46, 48, 64, 44, 58, 56, 74, 47, 63, 60, 72, 54, 67, 71, 80, 42, 59, 61, 72, 55, 73, 69, 81, 61, 72, 65, 83, 70, 80, 78, 91, 43, 57, 62, 74, 57, 66, 74, 82, 55, 68, 70, 81, 73, 79, 80, 92, 55, 73, 70, 80, 68, 79, 81, 92, 69, 81, 78, 91, 81, 92, 91, 104};

	int neigh[8][2] = {{1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};

	unsigned long long next[DIM];

	for (int y = 0; y < DIM; y++) {
		next[y] = 0;

		for (int x = 0; x < DIM; x++) {
			int binary = 0;

			for (int n = 0; n < 8; n++) {
				binary <<= 1;
				if (((y + neigh[n][0]) < 0) || ((y + neigh[n][0]) >= DIM) || ((x + neigh[n][1]) < 0) || ((x + neigh[n][1]) >= DIM)) {
					continue;
				}
				if (((*pattern)[y + neigh[n][0]] >> (x + neigh[n][1])) & 1) binary++;
			}

			if (!(((*pattern)[y] >> x) & 1) && bconds[dict[binary]]) next[y] |= 1 << x;
			if ((((*pattern)[y] >> x) & 1) && sconds[dict[binary]]) next[y] |= 1 << x;
		}
	}

	for (int y = 0; y < DIM; y++) {
		(*pattern)[y] = next[y];
	}
}

void print(unsigned long long pattern[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	printf("\033[1;32mFound an oscillator, %d so far.\033[0m\n", found + 1);

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			if ((pattern[y] >> x) & 1) {
				fprintf(results, "o");
			} else {
				fprintf(results, "b");
			}
		}
		fprintf(results, "$\n");
	}
}

void print_layers(unsigned long long layers[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	for (int n = 0; n < DIAG_WIDTH; n++) {
		printf("%4lld", layers[n]);
	}

	printf("\n");
}

void copy(unsigned long long source[], unsigned long long (* target)[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	for (int y = 0; y < DIM; y++) {
		(*target)[y] = source[y];
	}
}

int equal(unsigned long long pattern1[], unsigned long long pattern2[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	for (int y = 0; y < DIM; y++) {
		if (pattern1[y] != pattern2[y]) return 0;
	}

	return 1;
}

int compare(unsigned long long l1[], unsigned long long l2[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	int nn = depth - (2 * PERIOD);

	for (int n = 0; n <= nn; n++) {
		if (l1[n] != l2[n]) return 0;
	}

	return 1;
}

int countzeroes(unsigned long long layers[]) {
	int record = 0;
	int current = 0;

	int n = 0;

	while (layers[n] == 0) n++;

	for (; n <= depth; n++) {
		if (layers[n] == 0) {
			if (current == record) record++;

			current++;
		} else {
			current = 0;
		}
	}

	return record;
}

int countleadingzeroes(unsigned long long layers[]) {
	int n = 0;

	while ((layers[n] == 0) && (n <= depth)) n++;

	return n;
}

int getpop(unsigned long long pattern[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	int pop = 0;

	for (int y = 0; y < DIM; y++) {
		pop += __builtin_popcountll(pattern[y]);
	}

	return pop;
}

int getminpop(unsigned long long (* pattern)[], int MINPOP, int PERIOD) {
	int pop = getpop(*pattern, MINPOP, PERIOD);

	for (int n = 0; n < PERIOD; n++) {
		advance(pattern, MINPOP, PERIOD);

		if (getpop(*pattern, MINPOP, PERIOD) < pop) pop = getpop(*pattern, MINPOP, PERIOD);
	}

	return pop;
}

int verify(unsigned long long (* pattern)[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	unsigned long long orig[DIM];

	copy(*pattern, &orig, MINPOP, PERIOD);
	for (int n = 0; n < PERIOD; n++) {
		advance(pattern, MINPOP, PERIOD);
	}

	if (!(equal(orig, *pattern, MINPOP, PERIOD))) { // We do not have a complete oscillator
		unsigned long long l1[DIAG_WIDTH];
		unsigned long long l2[DIAG_WIDTH];

		pattern_to_layers(orig, &l1, MINPOP, PERIOD);
		pattern_to_layers(*pattern, &l2, MINPOP, PERIOD);

		for (int y = 0; y < DIM; y++) {
			(*pattern)[y] = orig[y]; // Reset pattern after advancing it PERIOD generations
		}

		if (compare(l1, l2, MINPOP, PERIOD)) return 1; // Partial
		else return 0; // Not a partial
	}

	for (int y = 0; y < DIM; y++) {
		(*pattern)[y] = orig[y]; // Reset pattern after advancing it PERIOD generations
	}

	// We have a complete oscillator

	if (getpop(*pattern, MINPOP, PERIOD) != getminpop(pattern, MINPOP, PERIOD)) {
		return 2;
	}

	return 3;
}

int is_subperiodic(unsigned long long (* pattern)[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	if (PERIOD < 2) return 0;

	unsigned long long orig[DIM];

	copy(*pattern, &orig, MINPOP, PERIOD);
	advance(pattern, MINPOP, PERIOD);

	if (equal(*pattern, orig, MINPOP, PERIOD)) return 1;

	for (int n = 1; n < (PERIOD - 1); n++) {
		if (equal(*pattern, orig, MINPOP, PERIOD)) return 1;

		advance(pattern, MINPOP, PERIOD);
	}

	for (int y = 0; y < DIM; y++) {
		(*pattern)[y] = orig[y]; // Reset pattern after advancing it PERIOD generations
	}

	return 0;
}

void trim(unsigned long long (* pattern)[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	int minx = 0;
	int maxx = 0;
	int miny = 0;
	int maxy = 0;

	unsigned long long next[DIM];

	for (int y = 0; y < DIM; y++) {
		next[y] = 0;
	}

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			if (((*pattern)[y] >> x) & 1) {
				miny = y;
				goto mny;
			}
		}
	}

	mny:;

	for (int x = 0; x < DIM; x++) {
		for (int y = 0; y < DIM; y++) {
			if (((*pattern)[y] >> x) & 1) {
				minx = x;
				goto mnx;
			}
		}
	}

	mnx:;

	for (int y = (DIM - 1); y >= 0; y--) {
		for (int x = (DIM - 1); x >= 0; x--) {
			if (((*pattern)[y] >> x) & 1) {
				maxy = y + 1;
				goto mxy;
			}
		}
	}

	mxy:;

	for (int x = (DIM - 1); x >= 0; x--) {
		for (int y = (DIM - 1); y >= 0; y--) {
			if (((*pattern)[y] >> x) & 1) {
				maxx = x + 1;
				goto mxx;
			}
		}
	}

	mxx:;

	for (int y = miny; y < maxy; y++) {
		for (int x = minx; x < maxx; x++) {
			next[y - miny] |= (((*pattern)[y] >> x) & 1) << (x - minx);
		}
	}

	for (int y = 0; y < DIM; y++) {
		(*pattern)[y] = next[y];
	}
}

void rotate(unsigned long long (* pattern)[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	unsigned long long rot[DIM];

	for (int y = 0; y < DIM; y++) rot[y] = 0;

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			rot[DIM - x - 1] |= (((*pattern)[y] >> x) & 1) << y;
		}
	}

	for (int y = 0; y < DIM; y++) {
		(*pattern)[y] = rot[y];
	}
}

void reflect(unsigned long long (* pattern)[], int MINPOP, int PERIOD) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	unsigned long long ref[DIM];

	for (int y = 0; y < DIM; y++) ref[y] = 0;

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			ref[y] |= (((*pattern)[y] >> x) & 1) << (DIM - x - 1);
		}
	}

	for (int y = 0; y < DIM; y++) {
		(*pattern)[y] = ref[y];
	}
}

int is_pseudo(unsigned long long (* pattern)[], int MINPOP, int PERIOD, int pbconds[]) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	int r = 1;

	for (int n = 0; n < PERIOD; n++) {
		if (fill_check(*pattern, MINPOP, PERIOD, pbconds)) r = 0;

		advance(pattern, MINPOP, PERIOD);
	}

	return r;
}

int in_patternlist(unsigned long long (* pattern)[], int MINPOP, int PERIOD, unsigned long long foundlist[MAX_FOUND][(2 * PERIOD) + MINPOP]) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	int r = 0;
	unsigned long long tr[DIM];

	for (int n = 0; n < found; n++) {
		for (int nn = 0; nn < 4; nn++) {
			copy(*pattern, &tr, MINPOP, PERIOD);
			trim(&tr, MINPOP, PERIOD);

			if (equal(tr, foundlist[n], MINPOP, PERIOD)) r = 1;
			reflect(pattern, MINPOP, PERIOD);

			copy(*pattern, &tr, MINPOP, PERIOD);
			trim(&tr, MINPOP, PERIOD);

			if (equal(tr, foundlist[n], MINPOP, PERIOD)) r = 1;
			reflect(pattern, MINPOP, PERIOD);
			rotate(pattern, MINPOP, PERIOD);
		}
	}

	return r;
}

int double_verify(unsigned long long (* pattern)[], int MINPOP, int PERIOD, unsigned long long foundlist[MAX_FOUND][(2 * PERIOD) + MINPOP]) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	if (is_pseudo(pattern, MINPOP, PERIOD, bconds)) return 0;

	if (getpop(*pattern, MINPOP, PERIOD) != MINPOP) return 0;

	if (is_subperiodic(pattern, MINPOP, PERIOD)) return 0;

	int r = 1;
	unsigned long long tr[DIM];

	copy(*pattern, &tr, MINPOP, PERIOD);
	trim(&tr, MINPOP, PERIOD);
	for (int n = 0; n < PERIOD; n++) {
		if (in_patternlist(pattern, MINPOP, PERIOD, foundlist)) r = 0;

		advance(pattern, MINPOP, PERIOD);
	}

	if (r) {
		print(*pattern, MINPOP, PERIOD);
		copy(tr, &(foundlist[found]), MINPOP, PERIOD);
	}

	return r;
}

void increment(unsigned long long (* layers)[], int * depth, int MINPOP, int PERIOD, unsigned long long max_layers[(2 * MINPOP) - 1]) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	(*layers)[*depth] += 1;

	while ((*layers)[*depth] >= max_layers[*depth]) {
		(*layers)[*depth] = 0;
		if ((*depth) != 0) {
			(*depth)--;
			(*layers)[*depth]++;
		}
	}
}

void write_to_rule(char rs[], char (* rule)[]) {
	int n = 0;

	char c = rs[n];

	while (c) {
		(*rule)[n] = c;
		n++;
		c = rs[n];
	}
}

int main(int argc, char * argv[]) {
	int MINPOP;
	int PERIOD;

	int dividing;

	char rs[274] = "B3/S23";

	if (argc == 3) {
		MINPOP = atoi(argv[1]);
		PERIOD = atoi(argv[2]);

		if ((MINPOP <= 0) || (PERIOD <= 0)) {
			printf("Invalid input: ./mfind %d %d\n", MINPOP, PERIOD);
			printf("               minpop and period must be greater than zero.\n");
			return 1;
		}
	} else if (argc == 4) {
		int n = 0;
		while (argv[1][n]) {
			rs[n] = argv[1][n];
			n++;
		}

		while (n < 274) {
			rs[n] = 0;
			n++;
		}

		MINPOP = atoi(argv[2]);
		PERIOD = atoi(argv[3]);
	} else {
		printf("Usage: mfind <rule> <minpop> <period>\n");
		printf("       mfind        <minpop> <period>\n");
		return 0;
	}

	dividing = (MINPOP > 3) ? 1 : 0;

	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	unsigned long long layers[DIAG_WIDTH];
	unsigned long long pattern[DIM];

	unsigned long long max_layers[DIAG_WIDTH];
	unsigned long long foundlist[MAX_FOUND][DIM];

	for (int n = 0; n < DIAG_WIDTH; n++) {
		layers[n] = 0;
	}

	for (int n = 0; n < DIM; n++) {
		pattern[n] = 0;
	}

	results = fopen("/path/to/your/results/file", "w");

	build_max_layers(&max_layers, MINPOP, PERIOD);
	write_to_rule(rs, &rule);

	fprintf(results, "x = 0, y = 0, rule = %s\n", rule);

	rparse(&bconds, &sconds);

	if (bconds[0]) {
		printf("Invalid input: ./mfind %s %d %d\n", rule, MINPOP, PERIOD);
		printf("               Rule must not contain B0.\n");
		return 1;
	}

	if (DIM > 64) {
		printf("Invalid search: ./mfind %s %d %d\n", rule, MINPOP, PERIOD);
		printf("                Search too big!\n");
		return 1;
	}

	if (dividing) printf("\033[1;36mStarting branch 0...\033[0m\n");

	while (1) {
		layers_to_pattern(layers, &pattern, MINPOP, PERIOD);

		while ((getpop(pattern, MINPOP, PERIOD) > MINPOP) || (countzeroes(layers) > (2 * PERIOD)) || (countleadingzeroes(layers) > (MINPOP / 2))) {
			increment(&layers, &depth, MINPOP, PERIOD, max_layers);
			layers_to_pattern(layers, &pattern, MINPOP, PERIOD);

			if ((depth == 0) && (layers[0] == 0)) {
				printf("%d oscillators found.\n", found);

				fclose(results);
				return 0;
			}

			if ((depth <= 3) && dividing) {
				unsigned long long l = (layers[0] * 32) + (layers[1] * 16) + (layers[2] * 4) + layers[3];
				printf("\033[1;36mFinished branch %2lld. Starting branch %2lld...\033[0m\n", l - 1, l);
			}
		}

		int status = verify(&pattern, MINPOP, PERIOD);

		if (status) {
			if (depth != (DIAG_WIDTH - 1)) {
				depth++;
			} else {
				increment(&layers, &depth, MINPOP, PERIOD, max_layers);
				if (!found) print_layers(layers, MINPOP, PERIOD);

				if ((depth == 0) && (layers[0] == 0)) {
					printf("%d oscillators found.\n", found);

					fclose(results);
					return 0;
				}

				if ((depth <= 3) && dividing) {
					unsigned long long l = (layers[0] * 32) + (layers[1] * 16) + (layers[2] * 4) + layers[3];
					printf("\033[1;36mFinished branch %2lld. Starting branch %2lld...\033[0m\n", l - 1, l);
				}
			}

			if (status == 3) {
				if (double_verify(&pattern, MINPOP, PERIOD, foundlist)) {
					found++;
				}
			}
		} else {
			increment(&layers, &depth, MINPOP, PERIOD, max_layers);
			if (!found) print_layers(layers, MINPOP, PERIOD);

			if ((depth == 0) && (layers[0] == 0)) {
				printf("%d oscillators found.\n", found);

				fclose(results);
				return 0;
			}

			if ((depth <= 3) && dividing) {
				unsigned long long l = (layers[0] * 32) + (layers[1] * 16) + (layers[2] * 4) + layers[3];
				printf("\033[1;36mFinished branch %2lld. Starting branch %2lld...\033[0m\n", l - 1, l);
			}
		}
	}
}
