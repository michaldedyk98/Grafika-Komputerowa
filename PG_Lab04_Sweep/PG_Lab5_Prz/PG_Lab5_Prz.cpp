// PG_Lab5.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <GL\glut.h>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <string>
#include <map>

using namespace std;

class Mesh;
class Vector3;

class Vector3 {
	GLdouble xyz[3];
	GLubyte clr[3];
public:
	Vector3() {
		xyz[0] = 0.0, xyz[1] = 0.0, xyz[2] = 0.0;
		clr[0] = (GLubyte)(0 + (rand() % 255)), clr[1] = (GLubyte)(0 + (rand() % 255)), clr[2] = (GLubyte)(0 + (rand() % 255));
	}
	Vector3(const double & _x, const double & _y, const double & _z) /*: xyz{ _x, _y, _z }, clr{ (GLubyte)(0 + (rand() % 255)), (GLubyte)(0 + (rand() % 255)), (GLubyte)(0 + (rand() % 255)) } */
	{
		xyz[0] = _x, xyz[1] = _y, xyz[2] = _z;
		clr[0] = (GLubyte)(0 + (rand() % 255)), clr[1] = (GLubyte)(0 + (rand() % 255)), clr[2] = (GLubyte)(0 + (rand() % 255));
	}

	const double & operator[](const int & i) {
		if (i > 3) throw new out_of_range("[Vector3] out of index");
		return xyz[i];
	}

	const GLubyte & r() const { return clr[0]; }

	const GLubyte & g() const { return clr[1]; }

	const GLubyte & b() const { return clr[2]; }

	const GLdouble & x() const { return xyz[0]; }

	const GLdouble & y() const { return xyz[1]; }

	const GLdouble & z() const { return xyz[2]; }

	GLdouble & x() { return xyz[0]; }

	GLdouble & y() { return xyz[1]; }

	GLdouble & z() { return xyz[2]; }

	GLubyte & r() { return clr[0]; }

	GLubyte & g() { return clr[1]; }

	GLubyte & b() { return clr[2]; }
};

//class Edge {
//	Vector3 * v1, *v2;
//public:
//	Edge() {}
//	Edge(Vector3 * _v1, Vector3 * _v2) : v1(_v1), v2(_v2) {}
//
//	const Vector3 * e1() const { return v1; }
//
//	const Vector3 * e2() const { return v2; }
//
//	Vector3 * e1() { return v1; }
//
//	Vector3 * e2() { return v2; }
//};

class Face {
	vector<Vector3 *> vert;
public:
	Face(Vector3 * e1, Vector3 * e2, Vector3 * e3) {
		vert.push_back(e1);
		vert.push_back(e2);
		vert.push_back(e3);
	}

	Face(Vector3 * e1, Vector3 * e2, Vector3 * e3, Vector3 * e4) {
		vert.push_back(e1);
		vert.push_back(e2);
		vert.push_back(e3);
		vert.push_back(e4);
	}

	Vector3 * operator[](const int & i) {
		if (i > vert.size()) throw new out_of_range("[Face] out of index");
		return vert[i];
	}

	size_t len() const { return vert.size(); }
};

class Mesh {
	Vector3 modelMid;
	map<int, Vector3> verticesMap;
	vector<Face> faces;
	vector< vector<Vector3> > segmentVertices;
	size_t verticesCnt;
	size_t segmentCnt;

	void loadModel(istream &);
public:
	Mesh(): verticesCnt(0), segmentCnt(0) {}
	Mesh(const char *);
	Mesh(istream &);

	vector<Face> & getFaces() { return faces; }
	const vector<Face> & getFaces() const { return faces; }
	Vector3 & getMid() { return modelMid; }
	const Vector3 & getMid() const { return modelMid; }
};

const char meshID[] = { 'W', 'p', 'T' };
const char cBegin = '[';
const char cEnd = ']';
const char cDelim = ',';

bool bMouse[4];

const size_t segC = 7;
size_t segU = 0;
Vector3 segmentColors[] = { Vector3(125, 72, 0), Vector3(5, 72, 32), Vector3(7, 120, 255), Vector3(125, 255, 0), Vector3(76, 6, 186), Vector3(221, 65, 54),  Vector3(55, 88, 124) };

Mesh::Mesh(const char * path) {
	ifstream meshStream(path);

	if (meshStream)
		loadModel(meshStream);
	else cerr << "Nie udalo sie wczytac modelu." << endl;
}

Mesh::Mesh(istream & meshStream) {
	loadModel(meshStream);
}

void Mesh::loadModel(istream & meshStream) {
	if (meshStream) {
		segmentCnt = 0;
		verticesCnt = 0;
		string mLine;
		string mToken;
		size_t mPos;
		char currentType = meshID[0];
		char lastType = meshID[0];

		size_t sectionFaceCnt = 0;
		vector< vector<int> > lastFaceID;

		segmentVertices.push_back(vector<Vector3>());

		while (meshStream >> mLine && mLine.size() > 0) {
			currentType = mLine[0];
			mPos = 0;

			if (meshID[0] == mLine[0]) {
				GLdouble pos[3];
				int tokenID = stoi(mLine.substr(1, mLine.find_first_of(cBegin) - 1));
				size_t it = 0;
				size_t tokenStart = mLine.find_first_of(cBegin);
				size_t tokenIterBegin = 0;
				mLine = mLine.substr(tokenStart + 1, mLine.length() - tokenStart - 2);

				while ((mPos = mLine.find(cDelim, tokenIterBegin)) != string::npos) {
					mToken = mLine.substr(tokenIterBegin, mPos - tokenIterBegin);
					pos[it++] = stod(mToken);
					tokenIterBegin = mPos + 1; // Razem z delimiterem
				}
				pos[it] = stod(mLine.substr(tokenIterBegin));

				verticesCnt++;
				verticesMap.insert(make_pair(tokenID, Vector3(pos[0], pos[1], pos[2])));

				if (segU < segC) {
					verticesMap[tokenID].r() = segmentColors[segU].x();
					verticesMap[tokenID].g() = segmentColors[segU].y();
					verticesMap[tokenID].b() = segmentColors[segU].z();
				}

				segmentVertices[segmentCnt].push_back(verticesMap[tokenID]);
			}
			else if (meshID[1] == mLine[0]) {
				size_t it = 0;
				size_t tokenStart = mLine.find_first_of(cBegin);
				size_t tokenIterBegin = 0;
				mLine = mLine.substr(tokenStart + 1, mLine.length() - tokenStart - 2);
				lastFaceID.push_back(vector<int>());

				while ((mPos = mLine.find(cDelim, tokenIterBegin)) != string::npos) {
					mToken = mLine.substr(tokenIterBegin, mPos - tokenIterBegin);
					int ID = stoi(mToken.substr(1));
					lastFaceID[sectionFaceCnt].push_back(ID - 1);
					tokenIterBegin = mPos + 1; // Razem z delimiterem
				}
				int atID = stoi(mLine.substr(tokenIterBegin + 1));
				lastFaceID[sectionFaceCnt].push_back(atID - 1);

				sectionFaceCnt++;
			} else if (meshID[2] == mLine[0]) {
				GLdouble trajXYZ[3];
				size_t it = 0;
				size_t tokenStart = mLine.find_first_of(cBegin);
				size_t tokenIterBegin = 0;
				mLine = mLine.substr(tokenStart + 1, mLine.length() - tokenStart - 2);

				while ((mPos = mLine.find(cDelim, tokenIterBegin)) != string::npos) {
					mToken = mLine.substr(tokenIterBegin, mPos - tokenIterBegin);
					trajXYZ[it++] = stod(mToken);
					tokenIterBegin = mPos + 1; // Razem z delimiterem
				}
				trajXYZ[it] = stod(mLine.substr(tokenIterBegin));

				segU++;
				segmentCnt++;
				segmentVertices.push_back(vector<Vector3>());

				for (size_t i = 0; i < verticesCnt; i++) {
					segmentVertices[segmentCnt].push_back(Vector3(segmentVertices[segmentCnt - 1][i].x()+trajXYZ[0], segmentVertices[segmentCnt - 1][i].y()+trajXYZ[1], segmentVertices[segmentCnt - 1][i].z()+trajXYZ[2]));

					if (segU < segC) {
						segmentVertices[segmentCnt].back().r() = segmentColors[segU].x();
						segmentVertices[segmentCnt].back().g() = segmentColors[segU].y();
						segmentVertices[segmentCnt].back().b() = segmentColors[segU].z();
					}
				}
			}

			lastType = currentType;
		}

		for (int i = 0; i < segmentVertices.size() - 1; i++) { // Dla kazdego segmentu
			vector<Vector3> *vert1 = &segmentVertices[i];
			vector<Vector3> *vert2 = &segmentVertices[i+1];

			for (int j = 0; j < vert1->size() - 1; j++)
				faces.push_back(Face(&vert1->at(j), &vert1->at(j + 1), &vert2->at(j + 1), &vert2->at(j)));

			faces.push_back(Face(&vert1->at(0), &vert1->at(vert1->size() - 1), &vert2->at(vert1->size() - 1), &vert2->at(0)));
		}

		modelMid = Vector3(0, 0, 0);
		size_t vertCount = 0;

		for (size_t i = 0; i < segmentVertices.size(); i++)
			for (size_t j = 0; j < segmentVertices[i].size(); j++) {
				modelMid.x() += segmentVertices[i][j].x();
				modelMid.y() += segmentVertices[i][j].y();
				modelMid.z() += segmentVertices[i][j].z();

				vertCount++;
			}

		vector<Vector3> * vertES = &segmentVertices.front();
		for (size_t i = 0; i < lastFaceID.size(); i++)
			faces.push_back(Face(&vertES->at(lastFaceID[i][0]), &vertES->at(lastFaceID[i][1]), &vertES->at(lastFaceID[i][2]), &vertES->at(lastFaceID[i][3])));

		vector<Vector3> * vertLS = &segmentVertices.back();
		for (size_t i = 0; i < lastFaceID.size(); i++)
			faces.push_back(Face(&vertLS->at(lastFaceID[i][0]), &vertLS->at(lastFaceID[i][1]), &vertLS->at(lastFaceID[i][2]), &vertLS->at(lastFaceID[i][3])));

		modelMid.x() /= vertCount;
		modelMid.y() /= vertCount;
		modelMid.z() /= vertCount;

		cout << "Wczytano model." << endl;
	}
	else cerr << "Nie udalo sie wczytac modelu." << endl;
}

Mesh objTect;

void idle(void) {
	glutPostRedisplay();
}

void Mouse(int key, int state, int x, int y) {
	if (key < 4)
		bMouse[key] = !state; // Stan przycisku myszy
}

void Keyboard(unsigned char key, int x, int y)
{
	key = tolower(key);
	switch (key)
	{
	case 'q':
		exit(0);
		break;
	}
}

void Reshape(int width, int height)
{
	const float ar = (float)width / (float)height;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

GLdouble angleY = 0;
GLdouble angleZ = 0;

void Display()
{
	const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	const double a = t*90.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (bMouse[0])
		angleY += 1.5;
	if (bMouse[2])
		angleY -= 1.5;
	if (bMouse[1])
		angleZ += 1.5;

	vector<Face> objFaces = objTect.getFaces();

	glPushMatrix();
	glTranslated(objTect.getMid().x(), objTect.getMid().y(), objTect.getMid().z());
	glScaled(0.7f, 0.7f, 0.7f);
	glRotated(30, 1.0, 0.0, 0.0);
	glRotated(60, 0.0, 1.0, 0.0);
	glRotated(angleY, 0.0, 1.0, 0.0);
	glRotated(angleZ, 0.0, 0.0, 1.0);
	glTranslated(-objTect.getMid().x(), -objTect.getMid().y(), -objTect.getMid().z());

	glBegin(GL_QUADS);
	for (size_t i = 0; i < objFaces.size(); i++) {
		Face objFace = objFaces[i];

		for (size_t j = 0; j < objFace.len(); j++) {
			glColor3ub(objFace[j]->r(), objFace[j]->g(), objFace[j]->b());
			glVertex3d(objFace[j]->x(), objFace[j]->y(), objFace[j]->z());
		}
	}

	glEnd();
	glPopMatrix();

	glutSwapBuffers();
}

void exitGLUT(void)
{
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("Reprezentacja z przesunieciem");

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	glutReshapeFunc(Reshape);
	glutDisplayFunc(Display);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutIdleFunc(idle);

	glClearColor(0.15, 0.15, 0.15, 1);

	try {
		objTect = Mesh("Model2.txt");

		cout << "[qQ] - Wyjdz" << endl;
		cout << "[Lewy przycisk myszy] - Obroc obiekt w lewo" << endl;
		cout << "[Prawy przycisk myszy] - Obroc obiekt w prawo" << endl;
		cout << "[Srodkowy przycisk myszy] - Obroc obiekt wokol osi Z" << endl;
	}
	catch (...) {
		cerr << "Nie udalo sie wczytac modelu." << endl;
	}

	glutMainLoop();

	exitGLUT();

	return 0;
}

//Model.txt

/*
W1[-2,0,-5] W2[-2,2,-5] W3[-1,1,-5] W4[1,1,-5] W5[2,2,-5] W6[2,0,-5]
T1[0,-1.2,-1.8] T1[0,0,-3.5] T1[0,1.2,-2.5] T1[0,0,-3.5] T1[4,3.5,-3.5] T1[0,0,-3.5]
p1[W1,W2,W3,W3] p2[W1,W3,W4,W6] p3[W6,W4,W5,W5]
*/