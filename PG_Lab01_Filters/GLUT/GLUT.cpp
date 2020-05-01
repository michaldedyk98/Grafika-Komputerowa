#include "stdafx.h"
/*
* Michal Dedyk grupa lab 1
* Zadanie - filtry na obrazie z pliku
*/

#include <windows.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <vector>

#define BYTES_PER_COLOR 3
#define WINDOW_HEIGHT 200
#define WINDOW_WIDTH 200

using namespace std;

const int bCoeffP = 5; // Wspolczynnik zmianny jasnosci
const int bCoeffN = -5; // Wspolczynnik zmiany jasnosci

GLubyte BrightnessLUT[255]; // LUT jasnosci
GLubyte GammaLUT[255]; // LUT wspolczynnika gamma
const char excImg[] = "Problem z odczytem obrazu z strumienia.";

const int lowPassFilter3[] = { 1, 1, 1,
1, 1, 1,
1, 1, 1 };

const int lowPassFilter5[] = { 1, 1, 2, 1, 1,
1, 2, 4, 2, 1,
2, 4, 8, 4, 2,
1, 2, 4, 2, 1,
1, 1, 2, 1, 1 };

const int highPassFilter3[] = { 0, -1,  0,
-1,  20, -1,
0, -1,  0 };

const int edgeSkewFilter3[] = { 0, 0, -1,
0, 1,  0,
0, 0,  0 };

const int edgeVerticalFilter3[] = { 0, -1, 0,
0,  1,  0,
0,  0,  0 };

const int edgeGradientWFilter3[] = { -1,  1, 1,
-1, -2, 1,
-1,  1, 1 };

const int edgeLaplaceFilter3[] = { 0, -1,  0,
-1,  4, -1,
0, -1,  0 };

class Image {
private:
	size_t imageWidth;
	size_t imageHeight;
	size_t imageSize;
	GLubyte * imageData;
	GLubyte * filterData;
	GLubyte * filteredImage;

	int imgBrightness;

	inline GLubyte getComponent(int val, GLubyte component) const {
		int temp = component + val;
		if (temp >= 255) return 255;
		else if (temp <= 0) return 0;

		return temp;
	}

	inline unsigned _2DIndexTo1D(unsigned row, unsigned col) const { return row * imageWidth + col; }

	inline GLubyte normalizeComponent(int component) const {
		if (component > 255) component = 255;
		else if (component < 0) component = 0;
		return component;
	}

	inline int qsPartition(GLubyte values[], int qsLow, int qsHigh) {
		GLubyte qsPivot = values[qsHigh];
		GLubyte swapT;
		int qsLP = qsLow - 1;

		for (int i = qsLow; i <= qsHigh - 1; i++) {
			if (values[i] < qsPivot) {
				qsLP++;
				swapT = values[i];

				values[i] = values[qsLP];
				values[qsLP] = swapT;
			}
		}

		swapT = values[qsLP + 1];

		values[qsLP + 1] = values[qsHigh];
		values[qsHigh] = swapT;

		return qsLP + 1;
	}

	inline void quickSortMedian(GLubyte values[], int qsLow, int qsHigh) {
		/*
		* Moze nie najszybciej, ale jako tako ~ Boguslaw L.
		*/

		if (qsLow > qsHigh) return;

		int partitionIndex = qsPartition(values, qsLow, qsHigh);

		quickSortMedian(values, qsLow, partitionIndex - 1);
		quickSortMedian(values, partitionIndex + 1, qsHigh);
	}

	inline GLubyte getMedian(GLubyte values[], size_t len) {
		quickSortMedian(values, 0, len - 1);

		return values[len / 2];
	}
public:
	Image() : imageWidth(0), imageHeight(0), imageData(0), filteredImage(0), imgBrightness(0) {}

	Image(ifstream & strImage);

	Image(const char * imgFile);

	Image(const Image & obj) : imageWidth(obj.imageWidth), imageHeight(obj.imageHeight), imageSize(obj.imageSize), imageData(obj.imageSize > 0 ? new GLubyte[obj.imageSize] : 0), filterData(obj.imageSize > 0 ? new GLubyte[obj.imageSize] : 0), filteredImage(obj.imageSize > 0 ? new GLubyte[obj.imageSize] : 0), imgBrightness(obj.imgBrightness) {
		if (imageData && filteredImage)
			for (unsigned imgData = 0; imgData < imageSize; imgData++) {
				imageData[imgData] = obj.imageData[imgData];
				filteredImage[imgData] = obj.filteredImage[imgData];
			}
	}

	Image & operator=(const Image & obj);

	~Image() { delete[] imageData; delete[] filteredImage; delete[] filterData; }

	const GLubyte * getImageData() const { return imageData; }

	const GLubyte * getFilteredImage() const { return filteredImage; }

	const size_t & getImageWidth() const { return imageWidth; }

	const size_t & getImageHeight() const { return imageHeight; }

	void changeBrightness(bool increaseBrightness);

	void changeRGB(int r, int g, int b);

	void changeGamma(float gamma);

	void applyLinearFilter(const int filter[], const size_t filterSize);

	void applyMinFilter();

	void applyMaxFilter();

	void applyMedianFilter();

	void restoreImage() const {
		for (unsigned i = 0; i < imageSize; i++)
			filteredImage[i] = imageData[i];
	}
};

void Image::applyMedianFilter() {
	GLubyte R[9], G[9], B[9], medianIndex;

	size_t filterSize = 3;
	int filterMargin = ((filterSize - 1) / 2), pIndex;

	for (size_t x = filterMargin; x < imageWidth - filterMargin; x++) {
		for (size_t y = filterMargin; y < imageHeight - filterMargin; y++) {
			medianIndex = 0;

			for (size_t m = 0; m < filterSize; m++) {
				for (size_t n = 0; n < filterSize; n++) {
					pIndex = _2DIndexTo1D(x + m - filterMargin, y + n - filterMargin) * BYTES_PER_COLOR;

					// R[medianIndex] = filteredImage[pIndex++]; - Undefined behaviour
					R[medianIndex] = filteredImage[pIndex]; pIndex++;
					G[medianIndex] = filteredImage[pIndex]; pIndex++;
					B[medianIndex++] = filteredImage[pIndex];
				}
			}
			pIndex = _2DIndexTo1D(x, y) * BYTES_PER_COLOR;

			filterData[pIndex] = getMedian(R, 9); pIndex++;
			filterData[pIndex] = getMedian(G, 9); pIndex++;
			filterData[pIndex] = getMedian(B, 9);
		}
	}

	/*
	* Kopiuj tylko zmodyfikowana czesc bufora
	*/
	for (size_t x = filterMargin; x < imageWidth - filterMargin; x++)
		for (size_t y = filterMargin; y < imageHeight - filterMargin; y++) {
			pIndex = _2DIndexTo1D(x, y) * BYTES_PER_COLOR;

			filteredImage[pIndex] = filterData[pIndex]; pIndex++;
			filteredImage[pIndex] = filterData[pIndex]; pIndex++;
			filteredImage[pIndex] = filterData[pIndex];
		}
}

void Image::applyMinFilter() {
	size_t filterSize = 3;
	int filterMargin = ((filterSize - 1) / 2);
	int minR, minG, minB, pIndex;
	GLubyte r, g, b;

	for (size_t x = filterMargin; x < imageWidth - filterMargin; x++) {
		for (size_t y = filterMargin; y < imageHeight - filterMargin; y++) {
			minR = 255, minG = 255, minB = 255;

			for (size_t m = 0; m < filterSize; m++) {
				for (size_t n = 0; n < filterSize; n++) {
					pIndex = _2DIndexTo1D(x + m - filterMargin, y + n - filterMargin) * BYTES_PER_COLOR;

					r = filteredImage[pIndex]; pIndex++;
					g = filteredImage[pIndex]; pIndex++;
					b = filteredImage[pIndex];

					if (minR > r) minR = r;
					pIndex++;
					if (minG > g) minG = g;
					pIndex++;
					if (minB > b) minB = b;
				}
			}
			pIndex = _2DIndexTo1D(x, y) * BYTES_PER_COLOR;

			filterData[pIndex] = minR; pIndex++;
			filterData[pIndex] = minG; pIndex++;
			filterData[pIndex] = minB;
		}
	}

	/*
	* Kopiuj tylko zmodyfikowana czesc bufora
	*/
	for (size_t x = filterMargin; x < imageWidth - filterMargin; x++)
		for (size_t y = filterMargin; y < imageHeight - filterMargin; y++) {
			pIndex = _2DIndexTo1D(x, y) * BYTES_PER_COLOR;

			filteredImage[pIndex] = filterData[pIndex]; pIndex++;
			filteredImage[pIndex] = filterData[pIndex]; pIndex++;
			filteredImage[pIndex] = filterData[pIndex];
		}
}

void Image::applyMaxFilter() {
	size_t filterSize = 3;
	int filterMargin = ((filterSize - 1) / 2);
	int maxR, maxG, maxB, pIndex;

	for (size_t x = filterMargin; x < imageWidth - filterMargin; x++) {
		for (size_t y = filterMargin; y < imageHeight - filterMargin; y++) {
			maxR = 0, maxG = 0, maxB = 0;

			for (size_t m = 0; m < filterSize; m++) {
				for (size_t n = 0; n < filterSize; n++) {
					pIndex = _2DIndexTo1D(x + m - filterMargin, y + n - filterMargin) * BYTES_PER_COLOR;

					if (maxR < filteredImage[pIndex]) maxR = filteredImage[pIndex];
					pIndex++;
					if (maxG < filteredImage[pIndex]) maxG = filteredImage[pIndex];
					pIndex++;
					if (maxB < filteredImage[pIndex]) maxB = filteredImage[pIndex];
				}
			}
			pIndex = _2DIndexTo1D(x, y) * BYTES_PER_COLOR;

			filterData[pIndex] = maxR; pIndex++;
			filterData[pIndex] = maxG; pIndex++;
			filterData[pIndex] = maxB;
		}
	}

	/*
	* Kopiuj tylko zmodyfikowana czesc bufora
	*/
	for (size_t x = filterMargin; x < imageWidth - filterMargin; x++)
		for (size_t y = filterMargin; y < imageHeight - filterMargin; y++) {
			pIndex = _2DIndexTo1D(x, y) * BYTES_PER_COLOR;

			filteredImage[pIndex] = filterData[pIndex]; pIndex++;
			filteredImage[pIndex] = filterData[pIndex]; pIndex++;
			filteredImage[pIndex] = filterData[pIndex];
		}
}

void Image::applyLinearFilter(const int filter[], const size_t filterSize) {
	int filterMargin = ((filterSize - 1) / 2);
	int sumR, sumG, sumB, pIndex;
	int filterNormal = 0;

	for (size_t i = 0; i < filterSize * filterSize; i++)
		filterNormal += filter[i];

	filterNormal = !filterNormal ? 1 : filterNormal;

	for (size_t x = filterMargin; x < imageWidth - filterMargin; x++) {
		for (size_t y = filterMargin; y < imageHeight - filterMargin; y++) {
			sumR = 0, sumG = 0, sumB = 0;

			for (size_t m = 0; m < filterSize; m++) {
				for (size_t n = 0; n < filterSize; n++) {
					pIndex = _2DIndexTo1D(x + m - filterMargin, y + n - filterMargin) * BYTES_PER_COLOR;

					sumR += filter[m * filterSize + n] * filteredImage[pIndex]; pIndex++;
					sumG += filter[m * filterSize + n] * filteredImage[pIndex]; pIndex++;
					sumB += filter[m * filterSize + n] * filteredImage[pIndex];
				}
			}
			/*
			* Normalizuj od 0 do 255
			*/

			sumR = normalizeComponent(sumR /= filterNormal);
			sumG = normalizeComponent(sumG /= filterNormal);
			sumB = normalizeComponent(sumB /= filterNormal);

			pIndex = _2DIndexTo1D(x, y) * BYTES_PER_COLOR;

			filterData[pIndex] = sumR; pIndex++;
			filterData[pIndex] = sumG; pIndex++;
			filterData[pIndex] = sumB;
		}
	}

	/*
	* Kopiuj tylko zmodyfikowana czesc bufora
	*/
	for (size_t x = filterMargin; x < imageWidth - filterMargin; x++)
		for (size_t y = filterMargin; y < imageHeight - filterMargin; y++) {
			pIndex = _2DIndexTo1D(x, y) * BYTES_PER_COLOR;

			filteredImage[pIndex] = filterData[pIndex]; pIndex++;
			filteredImage[pIndex] = filterData[pIndex]; pIndex++;
			filteredImage[pIndex] = filterData[pIndex];
		}
}

void Image::changeGamma(float gamma) {
	for (int i = 0; i < 256; i++) {
		GLubyte gammaValue = 255 * pow((i / 255.0), 1.0 / gamma);
		GammaLUT[i] = gammaValue > 255 ? 255 : gammaValue;
	}

	for (unsigned i = 0; i < imageSize; i++) {
		filteredImage[i] = GammaLUT[filteredImage[i]];
	}
}

void Image::changeRGB(int r, int g, int b) {
	for (unsigned i = 0; i < imageSize; i += 3) {
		filteredImage[i] = getComponent(r, filteredImage[i]);
		filteredImage[i + 1] = getComponent(g, filteredImage[i + 1]);
		filteredImage[i + 2] = getComponent(b, filteredImage[i + 2]);
	}
}

void Image::changeBrightness(bool increaseBrightness) {
	if (increaseBrightness) {
		/*
		* Bez nakladnia sie filtrow
		if (imgBrightness + bCoeffP <= 255)
		imgBrightness += bCoeffP;
		*/
		imgBrightness = bCoeffP;

		for (int i = 0; i < 256; i++) {
			if (i + imgBrightness > 255)
				BrightnessLUT[i] = 255;
			else if (0 > imgBrightness + i)
				BrightnessLUT[i] = 0;
			else
				BrightnessLUT[i] = imgBrightness + i;
		}

		for (unsigned i = 0; i < imageSize; i++)
			filteredImage[i] = BrightnessLUT[filteredImage[i]];
		/* Bez nakladania sie filtrow
		filteredImage[i] = BrightnessLUT[imageData[i]];
		*/
	}
	else {
		/*
		* Bez nakladnia sie filtrow
		if (imgBrightness + bCoeffN >= -255)
		imgBrightness += bCoeffN;
		*/
		imgBrightness = bCoeffN;

		for (int i = 0; i < 256; i++) {
			if (i + imgBrightness > 255)
				BrightnessLUT[i] = 255;
			else if (0 > imgBrightness + i)
				BrightnessLUT[i] = 0;
			else
				BrightnessLUT[i] = imgBrightness + i;
		}

		for (unsigned i = 0; i < imageSize; i++)
			filteredImage[i] = BrightnessLUT[filteredImage[i]];
		/* Bez nakladania sie filtrow
		filteredImage[i] = BrightnessLUT[imageData[i]];
		*/
	}
}

Image & Image::operator=(const Image & obj) {
	if (&obj != this) {
		if (obj.imageSize != imageSize) {
			delete[] imageData;
			delete[] filteredImage;
			delete[] filterData;

			filteredImage = new GLubyte[obj.imageSize];
			imageData = new GLubyte[obj.imageSize];
			filterData = new GLubyte[obj.imageSize];
		}

		imageWidth = obj.imageWidth;
		imageHeight = obj.imageHeight;
		imageSize = obj.imageSize;
		imgBrightness = obj.imgBrightness;

		for (unsigned imgPtr = 0; imgPtr < imageSize; imgPtr++) {
			imageData[imgPtr] = obj.imageData[imgPtr];
			filteredImage[imgPtr] = obj.filteredImage[imgPtr];
		}
	}

	return *this;
}

Image::Image(ifstream & strImage) {
	if (strImage.good()) {
		strImage >> imageWidth;
		if (!strImage) throw string(excImg);
		strImage >> imageHeight;
		if (!strImage) throw string(excImg);

		imageSize = imageHeight * imageWidth * BYTES_PER_COLOR;
		imgBrightness = 0;

		if (imageSize == 0) throw string(excImg);

		try {
			imageData = new GLubyte[imageSize];
			filteredImage = new GLubyte[imageSize];
			filterData = new GLubyte[imageSize];
		}
		catch (std::bad_alloc excAlloc) { throw string("Nie udalo sie zarezerwowac pamieci."); }

		size_t imgLen = imageHeight * imageWidth;
		unsigned indexData = 0, tempVal = 0;

		for (size_t p = 0; p < imgLen; p++) {
			for (size_t f = 0; f < BYTES_PER_COLOR; f++) {
				strImage >> tempVal;
				imageData[indexData++] = tempVal;
				if (!strImage) {
					throw string(excImg);

					delete[] imageData;
					delete[] filteredImage;
					delete[] filterData;
				}
			}
		}

		restoreImage();

		cout << "Obraz zostal wczytany prawidlowo." << endl;
	}
	else throw string(excImg);
}

Image::Image(const char * imgFile) {
	ifstream strImage(imgFile, ios::binary);

	if (strImage.good()) {
		strImage >> imageWidth;
		if (!strImage) throw string(excImg);
		strImage >> imageHeight;
		if (!strImage) throw string(excImg);

		imageSize = imageHeight * imageWidth * BYTES_PER_COLOR;
		imgBrightness = 0;

		if (imageSize == 0) throw string(excImg);

		try {
			imageData = new GLubyte[imageSize];
			filteredImage = new GLubyte[imageSize];
			filterData = new GLubyte[imageSize];
		}
		catch (std::bad_alloc excAlloc) { throw string("Nie udalo sie zarezerwowac pamieci."); }

		size_t imgLen = imageHeight * imageWidth;
		unsigned indexData = 0, tempVal = 0;

		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

		for (size_t p = 0; p < imgLen; p++) {
			for (size_t f = 0; f < BYTES_PER_COLOR; f++) {
				strImage >> tempVal;
				imageData[indexData++] = tempVal;

				if (!strImage) {
					throw string(excImg);

					delete[] imageData;
					delete[] filteredImage;
					delete[] filterData;
				}
			}
		}
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

		restoreImage();
		strImage.close();

		cout << "Obraz " << imgFile << " zostal wczytany prawidlowo." << endl;
	}
	else throw string(excImg);
}

Image * fImage = 0;

/* GLUT callback Handlers */

static void resize(int width, int height)
{
	const float ar = (float)width / (float)height;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width / ar, height / ar, 0, 0, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (fImage) // Zablokuj mozliwosc rozszerzania okna
		glutReshapeWindow(fImage->getImageWidth(), fImage->getImageHeight());
}

static void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glPointSize(10);
	glPushMatrix();
	glBegin(GL_POINTS);
	if (fImage != 0 && fImage->getFilteredImage()) {
		GLubyte r, g, b;
		unsigned pixelCnt = 0;
		for (size_t y = fImage->getImageHeight(); y > 0; y--) { // Rysujemy od gornego lewego
			for (size_t x = 0; x < fImage->getImageWidth(); x++) {
				r = fImage->getFilteredImage()[pixelCnt++];
				g = fImage->getFilteredImage()[pixelCnt++];
				b = fImage->getFilteredImage()[pixelCnt++];

				glColor3ub(r, g, b);
				glVertex2i(x, y);
			}
		}
	}
	glEnd();
	glPopMatrix();

	glutSwapBuffers();
}

static void key(unsigned char key, int x, int y)
{
	key = tolower(key);
	switch (key)
	{
	case 27:
	case 'q':
		delete fImage;
		fImage = 0;

		exit(0);

		break;
	case 'r':
		if (fImage != 0)
			fImage->restoreImage();

		break;
	case '+': {
		if (fImage != 0)
			fImage->changeBrightness(true);
	} break;
	case '-': {
		if (fImage != 0)
			fImage->changeBrightness(false);
	} break;
	case 'f': {
		if (fImage != 0)
			fImage->changeRGB(-5, 0, 0);
	} break;
	case 'g': {
		if (fImage != 0)
			fImage->changeRGB(0, -5, 0);
	} break;
	case 'h': {
		if (fImage != 0)
			fImage->changeRGB(0, 0, -5);
	} break;
	case 'a': {
		if (fImage != 0)
			fImage->changeRGB(5, 0, 0);
	} break;
	case 's': {
		if (fImage != 0)
			fImage->changeRGB(0, 5, 0);
	} break;
	case 'd': {
		if (fImage != 0)
			fImage->changeRGB(0, 0, 5);
	} break;
	case 'z': {
		if (fImage != 0) {
			fImage->changeGamma(0.25);
		}
	} break;
	case 'x': {
		if (fImage != 0) {
			fImage->changeGamma(0.5);
		}
	} break;
	case 'c': {
		if (fImage != 0) {
			fImage->changeGamma(0.75);
		}
	} break;
	case '1': {
		if (fImage != 0) {
			fImage->applyLinearFilter(lowPassFilter3, 3);
		}
	} break;
	case '2': {
		if (fImage != 0) {
			fImage->applyLinearFilter(lowPassFilter5, 5);
		}
	} break;
	case '3': {
		if (fImage != 0) {
			fImage->applyLinearFilter(highPassFilter3, 3);
		}
	} break;
	case '4': {
		if (fImage != 0) {
			fImage->applyMaxFilter();
		}
	} break;
	case '5': {
		if (fImage != 0) {
			fImage->applyMinFilter();
		}
	} break;
	case '6': {
		if (fImage != 0) {
			fImage->applyLinearFilter(edgeSkewFilter3, 3);
		}
	} break;
	case '7': {
		if (fImage != 0) {
			fImage->applyLinearFilter(edgeVerticalFilter3, 3);
		}
	} break;
	case '8': {
		if (fImage != 0) {
			fImage->applyLinearFilter(edgeGradientWFilter3, 3);
		}
	} break;
	case '9': {
		if (fImage != 0) {
			fImage->applyLinearFilter(edgeLaplaceFilter3, 3);
		}
	} break;
	case '0': {
		if (fImage != 0) {
			fImage->applyMedianFilter();
		}
	} break;
	}

	glutPostRedisplay();
}

static void idle(void)
{
	glutPostRedisplay();
}

/* Program entry point */

int main(int argc, char *argv[])
{
	try {
		//ifstream stream("zd3.txt");
		//Image *fDwa = new Image("zd3.txt");
		fImage = new Image("zd3.txt");
		//Image *fTrzy = new Image(*fImage);
		//*fImage = *fDwa;
		//delete fDwa;
		//stream.close();

		cout << "[q] Wyjdz z programu" << endl;
		cout << "[r] Przywroc obraz" << endl;
		cout << "---------------- Filtracja jednopunktowa ----------------" << endl;
		cout << "[+] Zwieksz jasnosc obrazu [FP]" << endl;
		cout << "[-] Zmniejsz jasnosc obrazu [FP]" << endl;
		cout << "[z] Ustaw wspolczynnik gamma 0.25 [FP]" << endl;
		cout << "[x] Ustaw wspolczynnik gamma 0.5 [FP]" << endl;
		cout << "[c] Ustaw wspolczynnik gamma 0.75 [FP]" << endl;
		cout << "[a] Zwieksz skladowa R (Czerwony) o 5 [FP]" << endl;
		cout << "[s] Zwieksz skladowa G (Zielony) o 5 [FP]" << endl;
		cout << "[d] Zwieksz skladowa B (Niebieski) o 5 [FP]" << endl;
		cout << "[f] Zmniejsz skladowa R (Czerwony) o 5 [FP]" << endl;
		cout << "[g] Zmniejsz skladowa G (Zielony) o 5 [FP]" << endl;
		cout << "[h] Zmniejsz skladowa B (Niebieski) o 5 [FP]" << endl;
		cout << "---------------- Filtracja splot ----------------" << endl;
		cout << "[0] Ustaw filtr medianowy [FS - statystyczny]" << endl;
		cout << "[1] Filtr dolnoprzepustowy 3x3 [FS]" << endl;
		cout << "[2] Filtr dolnoprzepustowy 5x5 [FS]" << endl;
		cout << "[3] Filtr gornoprzepustowy 3x3 [FS]" << endl;
		cout << "[4] Filtr maksymalny [FS - statystyczny]" << endl;
		cout << "[5] Filtr minimalny [FS - statystyczny]" << endl;
		cout << "[6] Filtr krawedziowy (Ukosny w lewo) [FS]" << endl;
		cout << "[7] Filtr krawedziowy (pionowy) [FS]" << endl;
		cout << "[8] Filtr kierunkowy gradientowy (wschod) [FS]" << endl;
		cout << "[9] Filtr Laplace'a [FS]" << endl;
	}
	catch (string err) {
		cerr << err;
	}

	glutInit(&argc, argv);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

	glutCreateWindow("Filtry");

	glutReshapeFunc(resize);
	glutDisplayFunc(display);
	glutKeyboardFunc(key);
	glutIdleFunc(idle);

	if (fImage)
		glutReshapeWindow(fImage->getImageWidth(), fImage->getImageHeight());

	glClearColor(1, 1, 1, 1);

	glutMainLoop();

	delete fImage;
	fImage = 0;

	return EXIT_SUCCESS;
}
