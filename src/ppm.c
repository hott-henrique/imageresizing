#include "ppm.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

void ppm_GetProperties(const char * filePath, int * outWidth, int * outHeight, int * outMaxComponentValue) {
	FILE * f = fopen(filePath, "r");
	if (NULL == f) {
		fprintf(stderr, "Could not open file: %s\n", filePath);
		exit(EXIT_FAILURE);
	}

	fseek(f, sizeof(char) * 3, SEEK_SET); // Ignorar a assinatura do arquivo.

	char line[100];

	fgets(line, sizeof(line), f);

	// Se a linha é um comentario, ignore.
	// E repita enquanto houver mais comentarios.
	while (line[0] == '#') fgets(line, sizeof(line), f);

	// A próxima linha que tem um real significado vai ser a que indica altura e largura da imagem.
	sscanf(line, "%d %d", outWidth, outHeight);

	fgets(line, sizeof(line), f);
	while (line[0] == '#') fgets(line, sizeof(line), f);

	sscanf(line, "%d", outMaxComponentValue);

	fclose(f);
}

void ppm_ForEachPixel(const char * filePath, pixelCallback, void * structData) {
	FILE * file = fopen(filePath, "r");
	if (NULL == file) {
		fprintf(stderr, "Could not open file: %s\n", "example.ppm");
		exit(EXIT_FAILURE);
	}

	char line[100];

	fgets(line, sizeof(line), file); // Signature, first line.

	// Ignore comments between signature and properties of file.
	fgets(line, sizeof(line), file);
	while (line[0] == '#') fgets(line, sizeof(line), file);

	int width, height;
	sscanf(line, "%d %d", &width, &height);

	// Ignore comments between properties and max component value.
	fgets(line, sizeof(line), file);
	while (line[0] == '#') fgets(line, sizeof(line), file);

	int maxComponentValue = 0;
	sscanf(line, "%d", &maxComponentValue);

	int maxValueLen = maxComponentValue > 0 ? floor(log10(abs(maxComponentValue))) + 1 : 1;

	// Ignore comments between max value and first line with pixels
	fgets(line, sizeof(line), file);
	while (line[0] == '#') fgets(line, sizeof(line), file);

	short component = 0;

	int x = 0;
	int y = 0;

	char * lineRef = &line[0];

	char componentStr[maxValueLen + 1];
	componentStr[maxValueLen] = '\0';

	pixel p = { .r = -1, .g = -1, .b = -1 };

	while (1) {
		int lineLength = strlen(lineRef);

		short bHasFoundComponent = 0;
		for (int i = 0; i < lineLength && !bHasFoundComponent; i++) {
			char c = lineRef[i];
			int cCode = (int)(c);

			if (c == '#') break;

			if (cCode >= 48 && cCode <= 57) {
				bHasFoundComponent = 1;

				int j = 0;
				for (; j < maxValueLen; j++) {
					char d = lineRef[i + j];
					int dCode = (int)(d);

					if (!(dCode >= 48 && dCode <= 57)) {
						componentStr[j] = '\0';
						break;
					}

					componentStr[j] = d;
				}
				lineRef = &lineRef[i+j];
			}
		}

		if (!bHasFoundComponent) {
			void * OK = fgets(line, sizeof(line), file);

			if (OK == NULL) return;

			while (line[0] == '#') fgets(line, sizeof(line), file);

			if (OK == NULL) return; // Case last line is a comment.

			lineRef = &line[0];	

			continue;
		}

		int componentValue = strtol(componentStr, NULL, 10);

		switch (component) {
			case 0:
				component++;
				p.r = componentValue;
			continue;

			case 1:
				component++;
				p.g = componentValue;
			continue;

			case 2:
				component = 0;
				p.b = componentValue;

				//printf("%d, %d, %d\n", p.r, p.g, p.b);
				callback(x, y, p, structData);

				y++;
			break;
		}

		if (y == width) {
			y = 0;
			x++;
		}

		if (x == height) break;
	}

	fclose(file);
}

