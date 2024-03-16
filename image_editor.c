#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#define NON_RGB 1
#define RGB 3
#define MSIZE 4096

typedef struct img {
	char p_nr[3];
	int lin, col;
	int max_gray;
	int x1, x2, y1, y2;
	int **mat;
} img_t;

void eliminare_mat(int **mat, int lin)
{
	for (int i = 0; i < lin; i++)
		free(mat[i]);
	free(mat);
}

// scoate comentariile care incep cu #
void fara_comentarii(FILE *intrare)
{
	char primul_caracter;
	char buf[MSIZE];
	fscanf(intrare, "\n%c", &primul_caracter);
	fseek(intrare, -1, SEEK_CUR);
	if (primul_caracter == '#')
		fgets(buf, sizeof(buf), intrare);
}

// in caz ca esueaza o comanda, ignora erorile de dupa
void ignora_erori_suplimentare(void)
{
	char buf[MSIZE];
	fgets(buf, MSIZE, stdin);
}

int incarcare_text(FILE *intrare, img_t *img, int culori)
{
	fara_comentarii(intrare);
	fscanf(intrare, "%d%d", &img->col, &img->lin);
	fara_comentarii(intrare);
	fscanf(intrare, "%d", &img->max_gray);
	fara_comentarii(intrare);

	img->mat = (int **)malloc(img->lin * sizeof(int *));
	if (!img->mat)
		return -1;
	for (int i = 0; i < img->lin; i++) {
		img->mat[i] = (int *)malloc(culori * img->col * sizeof(int));
		if (!img->mat[i])
			return -1;
	}
	img->x1 = 0;
	img->x2 = img->lin;
	img->y1 = 0;
	img->y2 = img->col;
	for (int i = img->x1; i < img->x2; i++)
		for (int j = img->y1; j < culori * img->y2; j++)
			fscanf(intrare, "%d", &img->mat[i][j]);
	return 0;
}

int incarcare_bin(FILE **intrare, img_t *img, int culori, char fisier[])
{
	fara_comentarii(*intrare);
	fscanf(*intrare, "%d%d", &img->col, &img->lin);
	fara_comentarii(*intrare);
	fscanf(*intrare, "%d", &img->max_gray);
	long pozitie = ftell(*intrare);
	fclose(*intrare);
	*intrare = fopen(fisier, "rb");
	if (!*intrare) {
		printf("Failed to load %s\n", fisier);
		return -1;
	}
	fseek(*intrare, pozitie + 1, SEEK_SET);
	fara_comentarii(*intrare);

	img->mat = (int **)malloc(img->lin * sizeof(int *));
	if (!img->mat)
		return -1;
	for (int i = 0; i < img->lin; i++) {
		img->mat[i] = (int *)malloc(culori * img->col * sizeof(int));
		if (!img->mat[i])
			return -1;
	}
	img->x1 = 0;
	img->x2 = img->lin;
	img->y1 = 0;
	img->y2 = img->col;
	unsigned char numar;
	for (int i = img->x1; i < img->x2; i++)
		for (int j = img->y1; j < culori * img->y2; j++) {
			fread(&numar, sizeof(char), 1, *intrare);
			img->mat[i][j] = (int)numar;
		}
	return 0;
}

int incarcare_cu_succes(img_t *img, int *nr_fisiere)
{
	FILE *intrare;
	char fisier[MSIZE];
	scanf("%s", fisier);
	if ((*nr_fisiere) > 0) {
		eliminare_mat(img->mat, img->lin);
		(*nr_fisiere) = 0;
	}
	intrare = fopen(fisier, "rt");
	if (!intrare) {
		printf("Failed to load %s\n", fisier);
	} else {
		printf("Loaded %s\n", fisier);
		(*nr_fisiere)++;
		fara_comentarii(intrare);
		fscanf(intrare, "%s", img->p_nr);

		if (strcmp(img->p_nr, "P2") == 0)
			if (incarcare_text(intrare, img, NON_RGB) == -1) {
				eliminare_mat(img->mat, img->lin);
				return -1;
			}

		if (strcmp(img->p_nr, "P3") == 0)
			if (incarcare_text(intrare, img, RGB) == -1) {
				eliminare_mat(img->mat, img->lin);
				return -1;
			}

		if (strcmp(img->p_nr, "P5") == 0)
			if (incarcare_bin(&intrare, img, NON_RGB, fisier) == -1) {
				eliminare_mat(img->mat, img->lin);
				return -1;
			}

		if (strcmp(img->p_nr, "P6") == 0)
			if (incarcare_bin(&intrare, img, RGB, fisier) == -1) {
				eliminare_mat(img->mat, img->lin);
				return -1;
			}
		fclose(intrare);
	}

	return 0;
}

// verificam daca sirul de caractere este un numar
int verifica_numar(char *sir)
{
	if (sir[0] == '-')
		for (int i = 1; sir[i] != '\0'; i++)
			if (!isdigit(sir[i]))
				return 0;

	for (int i = 1; sir[i] != '\0'; i++)
		if (!isdigit(sir[i]))
			return 0;

	return 1;
}

// verificam daca sirul de caractere formeaza un string fara numere
int verifica_string(char *sir)
{
	for (int i = 0; sir[i] != '\0'; i++)
		if (isdigit(sir[i]))
			return 1;

	return 0;
}

void selecteaza_tot(img_t *img)
{
	img->x1 = 0;
	img->x2 = img->lin;
	img->y1 = 0;
	img->y2 = img->col;
}

// verificam daca coordonatele sunt numere si daca sunt in numar de 4
int verifica_coordonate(img_t *img, int *copie_x1, int *copie_y1,
						int *copie_x2, int *copie_y2)
{
	char parametri[MSIZE];
	fgets(parametri, MSIZE, stdin);

	int lungime = strlen(parametri);
	parametri[lungime - 1] = '\0';
	if (!parametri[0]) {
		printf("Invalid command\n");
		return -1;
	}

	char *param1 = strtok(parametri, " ");
	if (strcmp(param1, "ALL") == 0) {
		selecteaza_tot(img);
		printf("Selected ALL\n");
		return -1;
	}
	if (param1 && verifica_numar(param1)) {
		*copie_x1 = atoi(param1);
	} else {
		printf("Invalid command\n");
		return -1;
	}

	char *param2 = strtok(NULL, " ");
	if (param2 && verifica_numar(param2)) {
		*copie_y1 = atoi(param2);
	} else {
		printf("Invalid command\n");
		return -1;
	}

	char *param3 = strtok(NULL, " ");
	if (param3 && verifica_numar(param3)) {
		*copie_x2 = atoi(param3);
	} else {
		printf("Invalid command\n");
		return -1;
	}

	char *param4 = strtok(NULL, " ");
	if (param4 && verifica_numar(param4)) {
		*copie_y2 = atoi(param4);
	} else {
		printf("Invalid command\n");
		return -1;
	}

	char *param5 = strtok(NULL, " ");
	if (param5) {
		printf("Invalid command\n");
		return -1;
	}

	return 0;
}

int selecteaza(img_t *img, int nr_fisiere)
{
	if (nr_fisiere != 0) {
		int copie_x1, copie_x2, copie_y1, copie_y2;
		if (verifica_coordonate(img, &copie_x1, &copie_y1, &copie_x2,
								&copie_y2) == -1)
			return 0;
		int tmp = 0;
		if (copie_x1 >= 0 && copie_x2 >= 0 && copie_y1 >= 0 && copie_y2 >= 0 &&
			copie_y1 <= img->lin && copie_y2 <= img->lin &&
			copie_x1 <= img->col && copie_x2 <= img->col &&
			copie_x1 != copie_x2 &&	copie_y1 != copie_y2) {
			img->x1 = copie_y1;
			img->x2 = copie_y2;
			img->y1 = copie_x1;
			img->y2 = copie_x2;
		} else {
			printf("Invalid set of coordinates\n");
			return 0;
		}
		if (img->x1 > img->x2) {
			tmp = img->x1;
			img->x1 = img->x2;
			img->x2 = tmp;
		}
		if (img->y1 > img->y2) {
			tmp = img->y1;
			img->y1 = img->y2;
			img->y2 = tmp;
		}
		printf("Selected %d %d %d %d\n",
			   img->y1, img->x1, img->y2, img->x2);
	} else {
		ignora_erori_suplimentare();
		printf("No image loaded\n");
	}

	return 0;
}

// roteste in sensul acelor de ceasornic pt imagini grayscale
int roteste_gri_ceas(img_t *img)
{
	int lin_tmp = img->x2 - img->x1;
	int col_tmp = img->y2 - img->y1;
	if (img->x1 == 0 && img->x2 == img->lin && img->y1 == 0 &&
		img->y2 == img->col) {
		int **rotita = (int **)malloc(col_tmp * sizeof(int *));
		if (!rotita)
			return -1;
		for (int i = 0; i < col_tmp; i++) {
			rotita[i] = (int *)malloc(lin_tmp * sizeof(int));
			if (!rotita[i])
				return -1;
		}
		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < col_tmp; j++)
				rotita[j][lin_tmp - i - 1] = img->mat[i][j];

		eliminare_mat(img->mat, img->lin);
		img->lin = col_tmp;
		img->col = lin_tmp;
		selecteaza_tot(img);
		img->mat = rotita;
	} else {
		int **rotita = (int **)malloc(lin_tmp * sizeof(int *));
		if (!rotita)
			return -1;
		for (int i = 0; i < lin_tmp; i++) {
			rotita[i] = (int *)malloc(col_tmp * sizeof(int));
			if (!rotita[i])
				return -1;
		}
		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < col_tmp; j++)
				rotita[j][lin_tmp - i - 1] = img->mat[img->x1 + i][img->y1 + j];
		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < col_tmp; j++)
				img->mat[img->x1 + i][img->y1 + j] = rotita[i][j];

		eliminare_mat(rotita, lin_tmp);
	}
	return 0;
}

// roteste in sens trigonometric de ceasornic pt imagini grayscale
int roteste_gri_trigo(img_t *img)
{
	int lin_tmp = img->x2 - img->x1;
	int col_tmp = img->y2 - img->y1;
	if (img->x1 == 0 && img->x2 == img->lin && img->y1 == 0 &&
		img->y2 == img->col) {
		int **rotita = (int **)malloc(col_tmp * sizeof(int *));
		if (!rotita)
			return -1;
		for (int i = 0; i < col_tmp; i++) {
			rotita[i] = (int *)malloc(lin_tmp * sizeof(int));
			if (!rotita[i])
				return -1;
		}
		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < col_tmp; j++)
				rotita[col_tmp - j - 1][i] = img->mat[i][j];

		eliminare_mat(img->mat, img->lin);
		img->lin = col_tmp;
		img->col = lin_tmp;
		selecteaza_tot(img);
		img->mat = rotita;
	} else {
		int **rotita = (int **)malloc(lin_tmp * sizeof(int *));
		if (!rotita)
			return -1;
		for (int i = 0; i < lin_tmp; i++) {
			rotita[i] = (int *)malloc(col_tmp * sizeof(int));
			if (!rotita[i])
				return -1;
		}
		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < col_tmp; j++)
				rotita[col_tmp - j - 1][i] = img->mat[img->x1 + i][img->y1 + j];
		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < col_tmp; j++)
				img->mat[img->x1 + i][img->y1 + j] = rotita[i][j];

		eliminare_mat(rotita, lin_tmp);
	}
	return 0;
}

// roteste in sensul acelor de ceasornic pt imagini color
int roteste_color_ceas(img_t *img)
{
	int lin_tmp = img->x2 - img->x1;
	int col_tmp = img->y2 - img->y1;
	if (img->x1 == 0 && img->x2 == img->lin && img->y1 == 0 &&
		img->y2 == img->col) {
		int **rotita = (int **)malloc(col_tmp * sizeof(int *));
		if (!rotita)
			return -1;
		for (int i = 0; i < col_tmp; i++) {
			rotita[i] = (int *)malloc(RGB * lin_tmp * sizeof(int));
			if (!rotita[i])
				return -1;
		}
		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < RGB * col_tmp; j += RGB) {
				rotita[j / RGB][RGB * lin_tmp - RGB * i - 1 - 2] =
				img->mat[i][j];
				rotita[j / RGB][RGB * lin_tmp - RGB * i - 1 - 1] =
				img->mat[i][j + 1];
				rotita[j / RGB][RGB * lin_tmp - RGB * i - 1] =
				img->mat[i][j + 2];
			}

		eliminare_mat(img->mat, img->lin);
		img->lin = col_tmp;
		img->col = lin_tmp;
		selecteaza_tot(img);
		img->mat = rotita;
	} else {
		int **rotita = (int **)malloc(lin_tmp * sizeof(int *));
		if (!rotita)
			return -1;
		for (int i = 0; i < lin_tmp; i++) {
			rotita[i] = (int *)malloc(RGB * col_tmp * sizeof(int));
			if (!rotita[i])
				return -1;
		}
		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < RGB * col_tmp; j += RGB) {
				rotita[j / RGB][RGB * lin_tmp - RGB * i - 1 - 2] =
				img->mat[img->x1 + i][RGB * img->y1 + j];
				rotita[j / RGB][RGB * lin_tmp - RGB * i - 1 - 1] =
				img->mat[img->x1 + i][RGB * img->y1 + j + 1];
				rotita[j / RGB][RGB * lin_tmp - RGB * i - 1] =
				img->mat[img->x1 + i][RGB * img->y1 + j + 2];
			}

		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < RGB * col_tmp; j++)
				img->mat[img->x1 + i][RGB * img->y1 + j] = rotita[i][j];

		eliminare_mat(rotita, lin_tmp);
	}
	return 0;
}

// roteste in sens trigonometric de ceasornic pt imagini color
int roteste_color_trigo(img_t *img)
{
	int lin_tmp = img->x2 - img->x1;
	int col_tmp = img->y2 - img->y1;
	if (img->x1 == 0 && img->x2 == img->lin && img->y1 == 0 &&
		img->y2 == img->col) {
		int **rotita = (int **)malloc(col_tmp * sizeof(int *));
		if (!rotita)
			return -1;
		for (int i = 0; i < col_tmp; i++) {
			rotita[i] = (int *)malloc(RGB * lin_tmp * sizeof(int));
			if (!rotita[i])
				return -1;
		}
		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < RGB * col_tmp; j += RGB) {
				rotita[col_tmp - j / RGB - 1][RGB * i] =
				img->mat[i][j];
				rotita[col_tmp - j / RGB - 1][RGB * i + 1] =
				img->mat[i][j + 1];
				rotita[col_tmp - j / RGB - 1][RGB * i + 2] =
				img->mat[i][j + 2];
			}

		eliminare_mat(img->mat, img->lin);
		img->lin = col_tmp;
		img->col = lin_tmp;
		selecteaza_tot(img);
		img->mat = rotita;
	} else {
		int **rotita = (int **)malloc(lin_tmp * sizeof(int *));
		if (!rotita)
			return -1;
		for (int i = 0; i < lin_tmp; i++) {
			rotita[i] = (int *)malloc(RGB * col_tmp * sizeof(int));
			if (!rotita[i])
				return -1;
		}
		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < RGB * col_tmp; j += RGB) {
				rotita[col_tmp - j / RGB - 1][RGB * i] =
				img->mat[img->x1 + i][RGB * img->y1 + j];
				rotita[col_tmp - j / RGB - 1][RGB * i + 1] =
				img->mat[img->x1 + i][RGB * img->y1 + j + 1];
				rotita[col_tmp - j / RGB - 1][RGB * i + 2] =
				img->mat[img->x1 + i][RGB * img->y1 + j + 2];
			}

		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < RGB * col_tmp; j++)
				img->mat[img->x1 + i][RGB * img->y1 + j] = rotita[i][j];

		eliminare_mat(rotita, lin_tmp);
	}
	return 0;
}

int roteste_imagine(img_t *img, int culori, int nr_fisiere)
{
	if (nr_fisiere != 0) {
		if ((img->x2 - img->x1 == img->y2 - img->y1) ||
			(img->x1 == 0 && img->x2 == img->lin &&
			 img->y1 == 0 && img->y2 == img->col)) {
			int unghi;
			scanf("%d", &unghi);
			if (unghi == 0) {
				printf("Rotated %d\n", unghi);
			} else if (unghi == 90 && culori == NON_RGB) {
				if (roteste_gri_ceas(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == 90 && culori == RGB) {
				if (roteste_color_ceas(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == -90 && culori == NON_RGB) {
				if (roteste_gri_trigo(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == -90 && culori == RGB) {
				if (roteste_color_trigo(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == 180 && culori == NON_RGB) {
				if (roteste_gri_ceas(img) == -1)
					return -1;
				if (roteste_gri_ceas(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == 180 && culori == RGB) {
				if (roteste_color_ceas(img) == -1)
					return -1;
				if (roteste_color_ceas(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == -180 && culori == NON_RGB) {
				if (roteste_gri_trigo(img) == -1)
					return -1;
				if (roteste_gri_trigo(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == -180 && culori == RGB) {
				if (roteste_color_trigo(img) == -1)
					return -1;
				if (roteste_color_trigo(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == 270 && culori == NON_RGB) {
				if (roteste_gri_trigo(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == 270 && culori == RGB) {
				if (roteste_color_trigo(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == -270 && culori == NON_RGB) {
				if (roteste_gri_ceas(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == -270 && culori == RGB) {
				if (roteste_color_ceas(img) == -1)
					return -1;
				printf("Rotated %d\n", unghi);
			} else if (unghi == 360 || unghi == -360) {
				printf("Rotated %d\n", unghi);
			} else {
				printf("Unsupported rotation angle\n");
			}
		} else {
			ignora_erori_suplimentare();
			printf("The selection must be square\n");
		}
	} else {
		ignora_erori_suplimentare();
		printf("No image loaded\n");
	}
	return 0;
}

int taie_imagine(img_t *img, int culori, int nr_fisiere)
{
	if (nr_fisiere != 0) {
		int lin_tmp = img->x2 - img->x1;
		int col_tmp = img->y2 - img->y1;
		int **mat_tmp = (int **)malloc(lin_tmp * sizeof(int *));
		if (!mat_tmp)
			return -1;
		for (int i = 0; i < lin_tmp; i++) {
			mat_tmp[i] = (int *)malloc(culori * col_tmp * sizeof(int));
			if (!mat_tmp[i])
				return -1;
		}
		for (int i = 0; i < lin_tmp; i++)
			for (int j = 0; j < culori * col_tmp; j++)
				mat_tmp[i][j] = img->mat[img->x1 + i][culori * img->y1 + j];

		eliminare_mat(img->mat, img->lin);
		img->lin = lin_tmp;
		img->col = col_tmp;
		img->mat = mat_tmp;
		selecteaza_tot(img);
		printf("Image cropped\n");
	} else {
		printf("No image loaded\n");
	}
	return 0;
}

int histograma(img_t *img, int nr_fisiere)
{
	if (nr_fisiere != 0) {
		int stele_max, intervale, *frecventa;
		char parametri[MSIZE];
		fgets(parametri, MSIZE, stdin);
		int lungime = strlen(parametri);
		parametri[lungime - 1] = '\0';
		if (!parametri[0]) {
			printf("Invalid command\n");
			return 0;
		}
		char *param1 = strtok(parametri, " ");
		if (verifica_numar(param1)) {
			stele_max = atoi(param1);
		} else {
			printf("Invalid command\n");
			return 0;
		}
		char *param2 = strtok(NULL, " ");
		if (param2 && verifica_numar(param2)) {
			intervale = atoi(param2);
		} else {
			printf("Invalid command\n");
			return 0;
		}

		char *param3 = strtok(NULL, " ");
		if (param3) {
			printf("Invalid command\n");
			return 0;
		}

		if (strcmp(img->p_nr, "P2") == 0 || strcmp(img->p_nr, "P5") == 0) {
			frecventa = (int *)calloc(256, sizeof(int));
			for (int i = 0; i < img->lin; i++)
				for (int j = 0; j < img->col; j++)
					frecventa[img->mat[i][j]]++;

			int *max = (int *)calloc(intervale, sizeof(int));
			int k = 0;
			for (int i = 0; i < intervale; i++) {
				for (int j = i * 256 / intervale;
				     j < (i + 1) * 256 / intervale; j++)
					max[k] += frecventa[j];
				k++;
			}

			int max_global = -1;
			for (int i = 0; i < intervale; i++)
				if (max_global < max[i])
					max_global = max[i];

			int *nr_stelute = (int *)calloc(intervale, sizeof(int));

			for (int i = 0; i < intervale; i++)
				nr_stelute[i] = (int)((double)max[i] / max_global * stele_max);
			for (int i = 0; i < intervale; i++) {
				printf("%d", nr_stelute[i]);
				printf("\t");
				printf("|");
				printf("\t");
				for (int j = 0; j < nr_stelute[i]; j++)
					printf("*");
				printf("\n");
			}

			free(frecventa);
			free(max);
			free(nr_stelute);
		} else {
			printf("Black and white image needed\n");
		}
	} else {
		ignora_erori_suplimentare();
		printf("No image loaded\n");
	}

	return 0;
}

int clamp(int pixel, int min, int max)
{
	if (pixel < min)
		return min;
	else if (pixel > max)
		return max;
	else
		return pixel;
}

int egalizare(img_t *img, int nr_fisiere)
{
	if (nr_fisiere != 0) {
		if (strcmp(img->p_nr, "P2") == 0 || strcmp(img->p_nr, "P5") == 0) {
			int *frecventa = (int *)calloc(256, sizeof(int));
			if (!frecventa)
				return -1;
			double suma = 0;
			for (int i = 0; i < img->lin; i++)
				for (int j = 0; j < img->col; j++)
					frecventa[img->mat[i][j]]++;
			for (int i = 0; i < img->lin; i++)
				for (int j = 0; j < img->col; j++) {
					for (int k = 0; k <= img->mat[i][j]; k++)
						suma += frecventa[k];
					img->mat[i][j] = round(255.0 * suma /
									 (img->lin * img->col));
					img->mat[i][j] = clamp(img->mat[i][j], 0, 255);
					suma = 0;
				}
			free(frecventa);
			printf("Equalize done\n");
		} else {
			printf("Black and white image needed\n");
		}
	} else {
		printf("No image loaded\n");
	}
	return 0;
}

void salveaza_binar(char fisier[], img_t *img, int culori)
{
	FILE *iesire = fopen(fisier, "wb");
	if (!iesire) {
		printf("Failed to save %s\n", fisier);
	} else {
		printf("Saved %s\n", fisier);

		if (strcmp(img->p_nr, "P2") == 0)
			fprintf(iesire, "%s\n", "P5");
		else if (strcmp(img->p_nr, "P3") == 0)
			fprintf(iesire, "%s\n", "P6");
		else
			fprintf(iesire, "%s\n", img->p_nr);

		fprintf(iesire, "%d %d\n", img->col, img->lin);
		fprintf(iesire, "%d\n", img->max_gray);
		unsigned char numar;
		for (int i = 0; i < img->lin; i++)
			for (int j = 0; j < culori * img->col; j++) {
				numar = (unsigned char)img->mat[i][j];
				fwrite(&numar, sizeof(char), 1, iesire);
			}
		fclose(iesire);
	}
}

void salveaza_ascii(char fisier[], img_t *img, int culori)
{
	FILE *iesire = fopen(fisier, "wt");
	if (!iesire) {
		printf("Failed to save %s\n", fisier);
	} else {
		printf("Saved %s\n", fisier);

		if (strcmp(img->p_nr, "P5") == 0)
			fprintf(iesire, "%s\n", "P2");
		else if (strcmp(img->p_nr, "P6") == 0)
			fprintf(iesire, "%s\n", "P3");
		else
			fprintf(iesire, "%s\n", img->p_nr);

		fprintf(iesire, "%d %d\n", img->col, img->lin);
		fprintf(iesire, "%d\n", img->max_gray);
		for (int i = 0; i < img->lin; i++) {
			for (int j = 0; j < culori * img->col; j++)
				fprintf(iesire, "%d ", img->mat[i][j]);
			fprintf(iesire, "\n");
		}
		fclose(iesire);
	}
}

int salveaza(img_t *img, int culori, int nr_fisiere)
{
	if (nr_fisiere != 0) {
		char fisier[MSIZE];
		scanf("%s", fisier);
		// verificam daca avem sau nu parametru ascii dupa SAVE
		char parametri[MSIZE];
		fgets(parametri, MSIZE, stdin);
		int lungime = strlen(parametri);
		parametri[lungime - 1] = '\0';
		char *ascii = strtok(parametri, " ");
		if (!ascii)
			salveaza_binar(fisier, img, culori);
		else
			if (strcmp(ascii, "ascii") == 0)
				salveaza_ascii(fisier, img, culori);
	} else {
		ignora_erori_suplimentare();
		printf("No image loaded\n");
	}
	return 0;
}

void efect_toata_imaginea(img_t *img, double **mat_tmp, float filtru[3][3])
{
	for (int i = 1; i < img->lin - 1; i++)
		for (int j = 3; j < RGB * img->col - 3; j += RGB) {
			// aplica efectul parcurgand fiecare linie a mini-matricei
			mat_tmp[i][j] = img->mat[i - 1][j - 3] * filtru[0][0] +
							img->mat[i - 1][j] * filtru[0][1] +
							img->mat[i - 1][j + 3] * filtru[0][2] +
							img->mat[i][j - 3] * filtru[1][0] +
							img->mat[i][j] * filtru[1][1] +
							img->mat[i][j + 3] * filtru[1][2] +
							img->mat[i + 1][j - 3] * filtru[2][0] +
							img->mat[i + 1][j] * filtru[2][1] +
							img->mat[i + 1][j + 3] * filtru[2][2];
			mat_tmp[i][j + 1] = img->mat[i - 1][j - 2] * filtru[0][0] +
								img->mat[i - 1][j + 1] * filtru[0][1] +
								img->mat[i - 1][j + 4] * filtru[0][2] +
								img->mat[i][j - 2] * filtru[1][0] +
								img->mat[i][j + 1] * filtru[1][1] +
								img->mat[i][j + 4] * filtru[1][2] +
								img->mat[i + 1][j - 2] * filtru[2][0] +
								img->mat[i + 1][j + 1] * filtru[2][1] +
								img->mat[i + 1][j + 4] * filtru[2][2];
			mat_tmp[i][j + 2] = img->mat[i - 1][j - 1] * filtru[0][0] +
								img->mat[i - 1][j + 2] * filtru[0][1] +
								img->mat[i - 1][j + 5] * filtru[0][2] +
								img->mat[i][j - 1] * filtru[1][0] +
								img->mat[i][j + 2] * filtru[1][1] +
								img->mat[i][j + 5] * filtru[1][2] +
								img->mat[i + 1][j - 1] * filtru[2][0] +
								img->mat[i + 1][j + 2] * filtru[2][1] +
								img->mat[i + 1][j + 5] * filtru[2][2];
		}

	for (int i = 0; i < img->lin; i++)
		for (int j = 0; j < RGB * img->col; j++)
			img->mat[i][j] = (int)mat_tmp[i][j];

	for (int i = 0; i < img->lin; i++)
		free(mat_tmp[i]);
	free(mat_tmp);
}

void efect_selectie(img_t *img, double **mat_tmp, float filtru[3][3])
{
	int copie_x1 = img->x1, copie_x2 = img->x2, copie_y1 = img->y1,
	copie_y2 = img->y2;
	if (img->x1 == 0)
		copie_x1++;
	if (img->x2 == img->lin)
		copie_x2--;
	if (img->y1 == 0)
		copie_y1++;
	if (img->y2 == img->col)
		copie_y2--;
	for (int i = copie_x1; i < copie_x2; i++)
		for (int j = RGB * copie_y1; j < RGB * copie_y2; j += RGB) {
			// aplica efectul parcurgand fiecare linie a mini-matricei
			mat_tmp[i][j] = img->mat[i - 1][j - 3] * filtru[0][0] +
							img->mat[i - 1][j] * filtru[0][1] +
							img->mat[i - 1][j + 3] * filtru[0][2] +
							img->mat[i][j - 3] * filtru[1][0] +
							img->mat[i][j] * filtru[1][1] +
							img->mat[i][j + 3] * filtru[1][2] +
							img->mat[i + 1][j - 3] * filtru[2][0] +
							img->mat[i + 1][j] * filtru[2][1] +
							img->mat[i + 1][j + 3] * filtru[2][2];
			mat_tmp[i][j + 1] = img->mat[i - 1][j - 2] * filtru[0][0] +
								img->mat[i - 1][j + 1] * filtru[0][1] +
								img->mat[i - 1][j + 4] * filtru[0][2] +
								img->mat[i][j - 2] * filtru[1][0] +
								img->mat[i][j + 1] * filtru[1][1] +
								img->mat[i][j + 4] * filtru[1][2] +
								img->mat[i + 1][j - 2] * filtru[2][0] +
								img->mat[i + 1][j + 1] * filtru[2][1] +
								img->mat[i + 1][j + 4] * filtru[2][2];
			mat_tmp[i][j + 2] = (img->mat[i - 1][j - 1] * filtru[0][0] +
								img->mat[i - 1][j + 2] * filtru[0][1] +
								img->mat[i - 1][j + 5] * filtru[0][2] +
								img->mat[i][j - 1] * filtru[1][0] +
								img->mat[i][j + 2] * filtru[1][1] +
								img->mat[i][j + 5] * filtru[1][2] +
								img->mat[i + 1][j - 1] * filtru[2][0] +
								img->mat[i + 1][j + 2] * filtru[2][1] +
								img->mat[i + 1][j + 5] * filtru[2][2]);
		}
	for (int i = 0; i < img->lin; i++)
		for (int j = 0; j < RGB * img->col; j++)
			img->mat[i][j] = (int)mat_tmp[i][j];

	for (int i = 0; i < img->lin; i++)
		free(mat_tmp[i]);
	free(mat_tmp);
}

int efect(img_t *img, float filtru[3][3])
{
	double **mat_tmp = (double **)calloc(img->lin, sizeof(double *));
	if (!mat_tmp)
		return -1;
	for (int i = 0; i < img->lin; i++) {
		mat_tmp[i] = (double *)calloc(RGB * img->col, sizeof(double));
		if (!mat_tmp[i])
			return -1;
	}
	for (int i = 0; i < img->lin; i++)
		for (int j = 0; j < RGB * img->col; j++)
			mat_tmp[i][j] = (double)img->mat[i][j];
	if (img->x1 == 0 && img->x2 == img->lin && img->y1 == 0 &&
		img->y2 == img->col) {
		efect_toata_imaginea(img, mat_tmp, filtru);
	} else {
		efect_selectie(img, mat_tmp, filtru);
	}
	return 0;
}

int f_edge(img_t *img)
{
	float edge[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};

	if (efect(img, edge) == -1)
		return -1;

	for (int i = img->x1; i < img->x2; i++)
		for (int j = RGB * img->y1; j < RGB * img->y2; j++)
			img->mat[i][j] = clamp(img->mat[i][j], 0, 255);

	return 0;
}

int f_sharpen(img_t *img)
{
	float sharpen[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};

	if (efect(img, sharpen) == -1)
		return -1;

	for (int i = img->x1; i < img->x2; i++)
		for (int j = RGB * img->y1; j < RGB * img->y2; j++)
			img->mat[i][j] = clamp(img->mat[i][j], 0, 255);

	return 0;
}

int f_blur(img_t *img)
{
	float blur[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			blur[i][j] = (blur[i][j] * 1.0) * 1 / 9;

	if (efect(img, blur) == -1)
		return -1;

	return 0;
}

int f_gaussian_blur(img_t *img)
{
	float gaussian_blur[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			gaussian_blur[i][j] = (gaussian_blur[i][j] * 1.0) * 1 / 16;

	if (efect(img, gaussian_blur) == -1)
		return -1;

	return 0;
}

int aplica(img_t *img, int nr_fisiere)
{
	if (nr_fisiere != 0) {
		int nr_param = 0;
		// verificam daca avem sau nu parametru dupa APPLY
		char parametri[MSIZE];
		fgets(parametri, MSIZE, stdin);
		int lungime = strlen(parametri);
		parametri[lungime - 1] = '\0';
		if (!parametri[0]) {
			printf("Invalid command\n");
			return 0;
		}
		char *spatii = strtok(parametri, " ");
		char parametru[50];
		strcpy(parametru, spatii);
		while (spatii) {
			if (verifica_string(spatii))
				nr_param++;
			spatii = strtok(NULL, " ");
		}
		if (nr_param != 0) {
			printf("Invalid command\n");
			return 0;
		}
		if (strcmp(img->p_nr, "P3") == 0 || strcmp(img->p_nr, "P6") == 0) {
			if (strcmp(parametru, "EDGE") == 0) {
				if (f_edge(img) == -1)
					return -1;
				printf("APPLY %s done\n", parametru);
			} else if (strcmp(parametru, "SHARPEN") == 0) {
				if (f_sharpen(img) == -1)
					return -1;
				printf("APPLY %s done\n", parametru);
			} else if (strcmp(parametru, "BLUR") == 0) {
				if (f_blur(img) == -1)
					return -1;
				printf("APPLY %s done\n", parametru);
			} else if (strcmp(parametru, "GAUSSIAN_BLUR") == 0) {
				if (f_gaussian_blur(img) == -1)
					return -1;
				printf("APPLY %s done\n", parametru);
			} else {
				printf("APPLY parameter invalid\n");
			}
		} else {
			printf("Easy, Charlie Chaplin\n");
		}
	} else {
		ignora_erori_suplimentare();
		printf("No image loaded\n");
	}

	return 0;
}

int main(void)
{
	char comanda[MSIZE], buf[MSIZE];
	int nr_fisiere = 0;
	img_t img;
	scanf("%s", comanda);
	while (strcmp(comanda, "EXIT") != 0) {
		if (strcmp(comanda, "LOAD") == 0) {
			if (incarcare_cu_succes(&img, &nr_fisiere) == -1)
				return -1;
		} else if (strcmp(comanda, "SELECT") == 0) {
			selecteaza(&img, nr_fisiere);
		} else if (strcmp(comanda, "HISTOGRAM") == 0) {
			histograma(&img, nr_fisiere);
		} else if (strcmp(comanda, "EQUALIZE") == 0) {
			egalizare(&img, nr_fisiere);
		} else if (strcmp(comanda, "ROTATE") == 0) {
			if (strcmp(img.p_nr, "P2") == 0 || strcmp(img.p_nr, "P5") == 0) {
				if (roteste_imagine(&img, NON_RGB, nr_fisiere) == -1) {
					eliminare_mat(img.mat, img.lin);
					return -1;
				}
			} else {
				if (roteste_imagine(&img, RGB, nr_fisiere) == -1) {
					eliminare_mat(img.mat, img.lin);
					return -1;
				}
			}
		} else if (strcmp(comanda, "CROP") == 0) {
			if (strcmp(img.p_nr, "P2") == 0 || strcmp(img.p_nr, "P5") == 0) {
				if (taie_imagine(&img, NON_RGB, nr_fisiere) == -1) {
					eliminare_mat(img.mat, img.lin);
					return -1;
				}
			} else {
				if (taie_imagine(&img, RGB, nr_fisiere) == -1) {
					eliminare_mat(img.mat, img.lin);
					return -1;
				}
			}
		} else if (strcmp(comanda, "APPLY") == 0) {
			if (aplica(&img, nr_fisiere) == -1) {
				eliminare_mat(img.mat, img.lin);
				return -1;
			}
		} else if (strcmp(comanda, "SAVE") == 0) {
			if (nr_fisiere != 0 && (strcmp(img.p_nr, "P2") == 0 ||
									strcmp(img.p_nr, "P5") == 0))
				salveaza(&img, NON_RGB, nr_fisiere);
			else
				salveaza(&img, RGB, nr_fisiere);
		} else {
			fgets(buf, MSIZE, stdin);
			printf("Invalid command\n");
		}
		scanf("%s", comanda);
	}

	if (strcmp(comanda, "EXIT") == 0) {
		if (nr_fisiere > 0)
			eliminare_mat(img.mat, img.lin);
		else
			printf("No image loaded\n");
	}

	return 0;
}
