#include "ppm.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

void ppm_GetProperties(const char * filePath, int * outWidth, int * outHeight) {
	FILE * f = fopen(filePath, "r");
	if (NULL == f) {
		fprintf(stderr, "Could not open file: %s\n", filePath);
		exit(EXIT_FAILURE);
	}

	fseek(f, sizeof(char) * 3, SEEK_SET); // Ignorar a assinatura do arquivo.

	// A especificação diz que todas as linhas tem no maximo 70 caracteres.
	char line[70];

	fgets(line, sizeof(line), f);

	// Se a linha é um comentario, ignore.
	// E repita enquanto houver mais comentarios.
	while (line[0] == '#') fgets(line, sizeof(line), f);

	// A próxima linha que tem um real significado vai ser a que indica altura e largura da imagem.
	sscanf(line, "%d %d", outWidth, outHeight);

	fclose(f);
}

void ppm_ForEachPixel(const char * filePath, pixelCallback, void * structData) {
	FILE * file = fopen(filePath, "r");
	if (NULL == file) {
		fprintf(stderr, "Could not open file: %s\n", filePath);
		exit(EXIT_FAILURE);
	}

	char line[70];

	fgets(line, sizeof(line), file); // Signature, first line.

	// Ignore comments between signature and properties of file.
	fgets(line, sizeof(line), file);
	while (line[0] == '#') fgets(line, sizeof(line), file);

	int width, height;
	sscanf(line, "%d %d", &width, &height);

	// Ignore comments between properties and max component value.
	fgets(line, sizeof(line), file);
	while (line[0] == '#') fgets(line, sizeof(line), file);

	// Ignore comments between max value and first line with pixels
	fgets(line, sizeof(line), file);
	while (line[0] == '#') fgets(line, sizeof(line), file);

	pixel p = { .r = 255, .g = 255, .b = 255 };
	short component = 0;

	int x = 0;
	int y = 0;

	char * lineSection = line;

	while (1) {
		//char c = '0'; while(((c=getchar()) != '\n') && c != EOF);
		int componentValue = -1;

		int readOK = 0;
		if (lineSection[0] == '#') {
			readOK = EOF;
		} else {
			readOK = sscanf(lineSection, "%d", &componentValue);
		}

		if (readOK == EOF) {
			lineSection = fgets(line, sizeof(line), file);

			if (lineSection == NULL) {
				printf("File is over\n");
				break;
			}

			continue; // Try to read again
		}

		int nDigits = componentValue > 0 ? floor(log10(abs(componentValue))) + 1 : 1;
		lineSection = &lineSection[nDigits + 1]; // Ignore those numbers in the next sscanf.

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

				f(x, y, p, structData);

				x++;
			break;
		}

		if ((x == width) && (y == height)) break;

		if (x == width) {
			x = 0;
			y++;
		}
	}

	fclose(file);
}


