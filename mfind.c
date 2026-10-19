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

#define MINPOP 20
#define PERIOD 1

#define MAX_FOUND 10000 // Increase if this value is too low

#define DIM ((2 * PERIOD) + MINPOP)

#define DIAG_WIDTH ((2 * MINPOP) - 1)

int testing = 0;

int depth = 0;
int found = 0;
int layers[DIAG_WIDTH] = {0};
int pattern[DIM][DIM] = {0};

char rule[274] = {0};

int bconds[117] = {0};
int sconds[117] = {0};

int max_layers[DIAG_WIDTH] = {0};

int foundlist[MAX_FOUND][DIM][DIM] = {0};

FILE * results;

void build_max_layers(int (* max_layers)[]) {
	for (int n = 1; n <= (DIAG_WIDTH / 2) + 1; n++) {
		(*max_layers)[n - 1] = 1 << ((n + 1) / 2);
	}

	for (int n = (DIAG_WIDTH / 2) + 2; n <= DIAG_WIDTH; n++) {
		(*max_layers)[n - 1] = 1 << ((DIAG_WIDTH - n + 2) / 2);
	}
}

void layers_to_pattern(int layers[], int (* pattern)[DIM][DIM]) {
	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			(*pattern)[y][x] = 0; // Clear pattern before assigning to it
		}
	}

	for (int n = 0; n <= (DIAG_WIDTH / 2); n++) {
		int m = n + 1;

		for (int nn = 0; nn < m; nn++) {
			int bin = ((layers[n] % (1 << (nn + 1))) < (1 << nn)) ? 0 : 1;
			(*pattern)[PERIOD + n - nn][PERIOD + nn] = bin;
		}
	}

	for (int n = (DIAG_WIDTH / 2) + 1; n < DIAG_WIDTH; n++) {
		int m = DIAG_WIDTH - n;

		for (int nn = 0; nn < m; nn++) {
			int bin = ((layers[n] % (1 << (nn + 1))) < (1 << nn)) ? 0 : 1;
			(*pattern)[PERIOD + MINPOP - nn - 1][PERIOD + (n - (DIAG_WIDTH / 2)) + nn] = bin;
		}
	}
}

void pattern_to_layers(int pattern[DIM][DIM], int (* layers)[]) {
	for (int n = 0; n < DIAG_WIDTH; n++) {
		(*layers)[n] = 0;
	}

	for (int n = 0; n < MINPOP; n++) {
		int m = n + 1;

		for (int nn = 0; nn < m; nn++) {
			(*layers)[n] += pattern[PERIOD + n - nn][PERIOD + nn] * (1 << nn);
		}
	}

	for (int n = 1; n < MINPOP; n++) {
		int m = MINPOP - n;

		for (int nn = 0; nn < m; nn++) {
			(*layers)[MINPOP + n - 1] += pattern[PERIOD + MINPOP - nn - 1][PERIOD + n + nn] * (1 << nn);
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
							(*bconds)[(lastdigit * 13) + nn] = 0;

							if (nn == i) (*bconds)[(lastdigit * 13) + nn] = 1;
						} else if (survival) {
							(*sconds)[(lastdigit * 13) + nn] = 0;

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

void advance(int (* pattern)[DIM][DIM]) {
	int dict[256] = {0, 13, 14, 29, 13, 26, 29, 43, 14, 29, 27, 42, 28, 44, 47, 55, 14, 28, 27, 47, 29, 44, 42, 55, 30, 48, 40, 61, 48, 56, 61, 69, 13, 26, 28, 44, 31, 39, 46, 57, 29, 43, 47, 55, 46, 57, 63, 68, 28, 45, 41, 54, 46, 58, 59, 73, 48, 62, 60, 70, 64, 74, 72, 81, 14, 28, 30, 48, 28, 45, 48, 62, 27, 47, 40, 61, 41, 54, 60, 70, 27, 41, 40, 60, 47, 54, 61, 70, 40, 60, 53, 65, 60, 71, 65, 78, 29, 44, 48, 56, 46, 58, 64, 74, 42, 55, 61, 69, 59, 73, 72, 81, 47, 54, 60, 71, 63, 67, 72, 80, 61, 70, 65, 78, 72, 80, 83, 91, 13, 31, 28, 46, 26, 39, 44, 57, 28, 46, 41, 59, 45, 58, 54, 73, 29, 46, 47, 63, 43, 57, 55, 68, 48, 64, 60, 72, 62, 74, 70, 81, 26, 39, 45, 58, 39, 52, 58, 66, 44, 57, 54, 73, 58, 66, 67, 79, 44, 58, 54, 67, 57, 66, 73, 79, 56, 74, 71, 80, 74, 82, 80, 92, 29, 46, 48, 64, 44, 58, 56, 74, 47, 63, 60, 72, 54, 67, 71, 80, 42, 59, 61, 72, 55, 73, 69, 81, 61, 72, 65, 83, 70, 80, 78, 91, 43, 57, 62, 74, 57, 66, 74, 82, 55, 68, 70, 81, 73, 79, 80, 92, 55, 73, 70, 80, 68, 79, 81, 92, 69, 81, 78, 91, 81, 92, 91, 104};

	int neigh[8][2] = {{1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};
	int next[DIM][DIM] = {0};

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			int binary = 0;

			for (int n = 0; n < 8; n++) {
				binary <<= 1;
				if (((y + neigh[n][0]) < 0) || ((y + neigh[n][0]) >= DIM) || ((x + neigh[n][1]) < 0) || ((x + neigh[n][1]) >= DIM)) {
					continue;
				}
				if ((*pattern)[y + neigh[n][0]][x + neigh[n][1]]) binary++;
			}

			if (!((*pattern)[y][x]) && bconds[dict[binary]]) next[y][x] = 1;
			if ((*pattern)[y][x] && sconds[dict[binary]]) next[y][x] = 1;
		}
	}

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			(*pattern)[y][x] = next[y][x];
		}
	}
}

void print(int pattern[DIM][DIM]) {
	printf("\033[1;32mFound an oscillator, %d so far.\033[0m\n", found);

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			if (pattern[y][x]) {
				fprintf(results, "o");

			} else {
				fprintf(results, "b");
			}
		}
		fprintf(results, "$\n");
	}
}

void print_layers(int layers[]) {
	for (int n = 0; n < DIAG_WIDTH; n++) {
		printf("%d ", layers[n]);
	}

	printf("\n");
}

void copy(int source[DIM][DIM], int (* target)[DIM][DIM]) {
	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			(*target)[y][x] = source[y][x];
		}
	}
}

int equal(int pattern1[DIM][DIM], int pattern2[DIM][DIM]) {
	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			if (pattern1[y][x] != pattern2[y][x]) return 0;
		}
	}

	return 1;
}

int compare(int l1[], int l2[]) {
	int nn = depth - (2 * PERIOD);

	for (int n = 0; n <= nn; n++) {
		if (l1[n] != l2[n]) return 0;
	}

	return 1;
}

int countzeroes(int layers[]) {
	int record = 0;
	int current = 0;

	for (int n = 0; n <= depth; n++) {
		if (layers[n] == 0) {
			if (current == record) record++;

			current++;
		} else {
			current = 0;
		}
	}

	return record;
}

int getpop(int pattern[DIM][DIM]) {
	int pop = 0;

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			if (pattern[y][x]) pop++;
		}
	}

	return pop;
}

int getminpop(int (* pattern)[DIM][DIM]) {
	int pop = getpop(*pattern);

	for (int n = 0; n < PERIOD; n++) {
		advance(pattern);

		if (getpop(*pattern) < pop) pop = getpop(*pattern);
	}

	return pop;
}

int verify(int (* pattern)[DIM][DIM]) {
	int orig[DIM][DIM] = {0};

	copy(*pattern, &orig);
	for (int n = 0; n < PERIOD; n++) {
		advance(pattern);
	}

	if (!(equal(orig, *pattern))) {
		int l1[DIAG_WIDTH] = {0};
		int l2[DIAG_WIDTH] = {0};

		pattern_to_layers(orig, &l1);
		pattern_to_layers(*pattern, &l2);

		for (int y = 0; y < DIM; y++) {
			for (int x = 0; x < DIM; x++) {
				(*pattern)[y][x] = orig[y][x]; // Reset pattern after advancing it PERIOD generations
			}
		}

		if (compare(l1, l2)) return 1;
		else return 0;
	}

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			(*pattern)[y][x] = orig[y][x]; // Reset pattern after advancing it PERIOD generations
		}
	}

	if (getpop(*pattern) != getminpop(pattern)) {
		return 1;
	}

	return 2;
}

int is_subperiodic(int (* pattern)[DIM][DIM]) {
	if (PERIOD < 2) return 0;

	int orig[DIM][DIM] = {0};

	copy(*pattern, &orig);
	advance(pattern);

	if (equal(*pattern, orig)) return 1;

	for (int n = 1; n < (PERIOD - 1); n++) {
		if (equal(*pattern, orig)) return 1;

		advance(pattern);
	}

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			(*pattern)[y][x] = orig[y][x]; // Reset pattern after advancing it PERIOD generations
		}
	}

	return 0;
}

void trim(int (* pattern)[DIM][DIM]) {
	int minx = 0;
	int maxx = 0;
	int miny = 0;
	int maxy = 0;

	int next[DIM][DIM] = {0};

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			if ((*pattern)[y][x]) {
				miny = y;
				goto mny;
			}
		}
	}

	mny:;

	for (int x = 0; x < DIM; x++) {
		for (int y = 0; y < DIM; y++) {
			if ((*pattern)[y][x]) {
				minx = x;
				goto mnx;
			}
		}
	}

	mnx:;

	for (int y = (DIM - 1); y >= 0; y--) {
		for (int x = (DIM - 1); x >= 0; x--) {
			if ((*pattern)[y][x]) {
				maxy = y + 1;
				goto mxy;
			}
		}
	}

	mxy:;

	for (int x = (DIM - 1); x >= 0; x--) {
		for (int y = (DIM - 1); y >= 0; y--) {
			if ((*pattern)[y][x]) {
				maxx = x + 1;
				goto mxx;
			}
		}
	}

	mxx:;

	for (int y = miny; y < maxy; y++) {
		for (int x = minx; x < maxx; x++) {
			next[y - miny][x - minx] = (*pattern)[y][x];
		}
	}

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			(*pattern)[y][x] = next[y][x];
		}
	}
}

int in_patternlist(int pattern[DIM][DIM]) {
	for (int n = 0; n < found; n++) {
		if (equal(pattern, foundlist[n])) return 1;
	}

	return 0;
}

int double_verify(int (* pattern)[DIM][DIM]) {
	// Placeholder, more tests coming soon

	int tr[DIM][DIM] = {0};

	copy(*pattern, &tr);
	if (getpop(*pattern) != MINPOP) return 0;

	if (is_subperiodic(pattern)) return 0;

	trim(&tr);
	if (in_patternlist(tr)) return 0;

	print(*pattern);

	copy(tr, &(foundlist[found]));

	return 1;
}

void increment(int (* layers)[DIAG_WIDTH], int * depth) {
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

int main(void) {
	results = fopen("/path/to/your/results/file", "w");

	build_max_layers(&max_layers);

	char rs[] = "B3/S23";
	write_to_rule(rs, &rule);

	fprintf(results, "x = 0, y = 0, rule = %s\n", rule);

	rparse(&bconds, &sconds);

	while (1) {
		layers_to_pattern(layers, &pattern);

		while ((getpop(pattern) > MINPOP) || (countzeroes(layers) > (2 * PERIOD))) {
			increment(&layers, &depth);
			layers_to_pattern(layers, &pattern);

			if ((depth == 0) && (layers[0] == 0)) {
				printf("%d oscillators found.\n", found);

				fclose(results);
				return 0;
			}
		}

		int status = verify(&pattern);

		if (status) {
			if (depth != (DIAG_WIDTH - 1)) {
				depth++;
			} else {
				increment(&layers, &depth);
				if (!found) print_layers(layers);

				if ((depth == 0) && (layers[0] == 0)) {
					printf("%d oscillators found.\n", found);

					fclose(results);
					return 0;
				}
			}

			if (status == 2) {
				if (double_verify(&pattern)) {
					found++;
				}
			}
		} else {
			increment(&layers, &depth);
			if (!found) print_layers(layers);

			if ((depth == 0) && (layers[0] == 0)) {
				printf("%d oscillators found.\n", found);

				fclose(results);
				return 0;
			}
		}
	}
}
