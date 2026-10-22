/*
    mfind - an rlifesrc-like program for finding oscillators in INT rules (pseudo.h header file).
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

#define INFECTED_ON 2
#define INFECTED_OFF -1

void expand(unsigned long long pattern[], int MINPOP, int PERIOD, int (* colored)[][(2 * PERIOD) + MINPOP]) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			(*colored)[y][x] = (pattern[y] >> x) & 1;
		}
	}
}

void print_colored(int MINPOP, int PERIOD, int colored[][(2 * PERIOD) + MINPOP]) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			if (colored[y][x] == -1) printf("\033[1;35m0\033[0m");
			if (colored[y][x] == 0) printf(" ");
			if (colored[y][x] == 1) printf("0");
			if (colored[y][x] == 2) printf("\033[1;31m0\033[0m");
		}

		printf("\n");
	}
}

void infect(int MINPOP, int PERIOD, int (* colored)[][(2 * PERIOD) + MINPOP]) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	int next[DIM][DIM];

	int neigh[8][2] = {{1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			int susceptible = 0;

			next[y][x] = (*colored)[y][x];
			if ((*colored)[y][x] > 1) continue;

			for (int n = 0; n < 8; n++) {
				if (((y + neigh[n][0]) < 0) || ((y + neigh[n][0]) >= DIM) || ((x + neigh[n][1]) < 0) || ((x + neigh[n][1]) >= DIM)) {
					continue;
				}

				if ((*colored)[y + neigh[n][0]][x + neigh[n][1]] == INFECTED_ON) susceptible = 1;
				if ((*colored)[y + neigh[n][0]][x + neigh[n][1]] == INFECTED_OFF) susceptible = 1;
			}

			if (((*colored)[y][x] == 1) && susceptible) next[y][x] = INFECTED_ON;
		}
	}

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			(*colored)[y][x] = next[y][x];
		}
	}
}

void traverse(int MINPOP, int PERIOD, int (* colored)[][(2 * PERIOD) + MINPOP], int pbconds[]) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	int next[DIM][DIM];

	int dict[256] = {0, 13, 14, 29, 13, 26, 29, 43, 14, 29, 27, 42, 28, 44, 47, 55, 14, 28, 27, 47, 29, 44, 42, 55, 30, 48, 40, 61, 48, 56, 61, 69, 13, 26, 28, 44, 31, 39, 46, 57, 29, 43, 47, 55, 46, 57, 63, 68, 28, 45, 41, 54, 46, 58, 59, 73, 48, 62, 60, 70, 64, 74, 72, 81, 14, 28, 30, 48, 28, 45, 48, 62, 27, 47, 40, 61, 41, 54, 60, 70, 27, 41, 40, 60, 47, 54, 61, 70, 40, 60, 53, 65, 60, 71, 65, 78, 29, 44, 48, 56, 46, 58, 64, 74, 42, 55, 61, 69, 59, 73, 72, 81, 47, 54, 60, 71, 63, 67, 72, 80, 61, 70, 65, 78, 72, 80, 83, 91, 13, 31, 28, 46, 26, 39, 44, 57, 28, 46, 41, 59, 45, 58, 54, 73, 29, 46, 47, 63, 43, 57, 55, 68, 48, 64, 60, 72, 62, 74, 70, 81, 26, 39, 45, 58, 39, 52, 58, 66, 44, 57, 54, 73, 58, 66, 67, 79, 44, 58, 54, 67, 57, 66, 73, 79, 56, 74, 71, 80, 74, 82, 80, 92, 29, 46, 48, 64, 44, 58, 56, 74, 47, 63, 60, 72, 54, 67, 71, 80, 42, 59, 61, 72, 55, 73, 69, 81, 61, 72, 65, 83, 70, 80, 78, 91, 43, 57, 62, 74, 57, 66, 74, 82, 55, 68, 70, 81, 73, 79, 80, 92, 55, 73, 70, 80, 68, 79, 81, 92, 69, 81, 78, 91, 81, 92, 91, 104};

	int neigh[8][2] = {{1, 1}, {1, 0}, {1, -1}, {0, 1}, {0, -1}, {-1, 1}, {-1, 0}, {-1, -1}};

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			int binary = 0;
			int infected_binary = 0;
			int susceptible_off = 0;

			next[y][x] = (*colored)[y][x];

			if ((*colored)[y][x]) continue;

			for (int n = 0; n < 8; n++) {
				binary <<= 1;
				infected_binary <<= 1;
				if (((y + neigh[n][0]) < 0) || ((y + neigh[n][0]) >= DIM) || ((x + neigh[n][1]) < 0) || ((x + neigh[n][1]) >= DIM)) {
					continue;
				}
				if ((*colored)[y + neigh[n][0]][x + neigh[n][1]] > 0) binary++;
				if ((*colored)[y + neigh[n][0]][x + neigh[n][1]] == 2) infected_binary++;

				if ((*colored)[y + neigh[n][0]][x + neigh[n][1]] == INFECTED_ON) susceptible_off = 1;
			}

			if ((!(*colored)[y][x]) && (pbconds[dict[binary]] || pbconds[dict[infected_binary]]) && susceptible_off) next[y][x] = INFECTED_OFF;
		}
	}

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			(*colored)[y][x] = next[y][x];
		}
	}
}

int getpop_color(int MINPOP, int PERIOD, int colored[][(2 * PERIOD) + MINPOP]) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	int pop = 0;

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			if (colored[y][x] > 1) pop++;
			if (colored[y][x] < 0) pop++;
		}
	}

	return pop;
}

int fill_check(unsigned long long pattern[], int MINPOP, int PERIOD, int pbconds[]) {
	int DIM = (2 * PERIOD) + MINPOP;
	int DIAG_WIDTH = (2 * MINPOP) - 1;

	int old_colorpop = 0;
	int new_colorpop = 0;

	int colored[DIM][DIM];

	expand(pattern, MINPOP, PERIOD, &colored);

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			if (colored[y][x] == 1) {
				colored[y][x] = 2;
				goto spread;
			}
		}
	}

	spread:;

	do {
		old_colorpop = getpop_color(MINPOP, PERIOD, colored);

		infect(MINPOP, PERIOD, &colored);
		traverse(MINPOP, PERIOD, &colored, pbconds);

		new_colorpop = getpop_color(MINPOP, PERIOD, colored);
	} while (old_colorpop != new_colorpop);

	for (int y = 0; y < DIM; y++) {
		for (int x = 0; x < DIM; x++) {
			if (colored[y][x] == 1) return 0;
		}
	}

	return 1;
}
