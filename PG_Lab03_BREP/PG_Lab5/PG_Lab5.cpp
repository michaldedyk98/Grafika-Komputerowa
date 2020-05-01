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
	Vector3() : xyz{ 0, 0, 0 }, clr{ (GLubyte)(0 + (rand() % 255)), (GLubyte)(0 + (rand() % 255)), (GLubyte)(0 + (rand() % 255)) } { }
	Vector3(const double & _x, const double & _y, const double & _z) : xyz{ _x, _y, _z }, clr{ (GLubyte)(0 + (rand() % 255)), (GLubyte)(0 + (rand() % 255)), (GLubyte)(0 + (rand() % 255)) } {}

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
	map<int, Vector3> verticesMap;
	vector<Face> faces;
	Vector3 modelMid;

	void loadModel(istream &);
public:
	Mesh() {}
	Mesh(const char *);
	Mesh(istream &);

	vector<Face> & getFaces() { return faces; }
	const vector<Face> & getFaces() const { return faces; }
	Vector3 & getMid() { return modelMid; }
	const Vector3 & getMid() const { return modelMid; }
};

const char meshID[] = { 'W', 'p' };
const char cBegin = '[';
const char cEnd = ']';
const char cDelim = ',';

bool bMouse[4];

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
		string mLine;
		string mToken;
		size_t mPos;
		char currentType = meshID[0];
		char lastType = meshID[0];

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

				verticesMap.insert(make_pair(tokenID, Vector3(pos[0], pos[1], pos[2])));
			}
			else if (meshID[1] == mLine[0]) {
				Vector3 * verticesFace[4];
				size_t it = 0;
				size_t tokenStart = mLine.find_first_of(cBegin);
				size_t tokenIterBegin = 0;
				mLine = mLine.substr(tokenStart + 1, mLine.length() - tokenStart - 2);

				while ((mPos = mLine.find(cDelim, tokenIterBegin)) != string::npos) {
					mToken = mLine.substr(tokenIterBegin, mPos - tokenIterBegin);
					verticesFace[it++] = &verticesMap.at(stoi(mToken.substr(1)));
					tokenIterBegin = mPos + 1; // Razem z delimiterem
				}
				verticesFace[it] = &verticesMap.at(stoi(mLine.substr(tokenIterBegin + 1)));

				if (it == 3)
					faces.push_back(Face(verticesFace[0], verticesFace[1], verticesFace[2], verticesFace[3]));
				else throw new out_of_range("[Face] out of index");
			}

			lastType = currentType;
		}

		modelMid = Vector3(0, 0, 0);

		for (map<int, Vector3>::iterator it = verticesMap.begin(); it != verticesMap.end(); ++it) {
			modelMid.x() += it->second.x();
			modelMid.y() += it->second.y();
			modelMid.z() += it->second.z();
		}

		modelMid.x() /= verticesMap.size();
		modelMid.y() /= verticesMap.size();
		modelMid.z() /= verticesMap.size();

		cout << "Wczytano model." << endl;
	} else cerr << "Nie udalo sie wczytac modelu." << endl;
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

GLdouble angX = 0;

void Display()
{
	const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
	const double a = t*90.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vector<Face> objFaces = objTect.getFaces();


	if (bMouse[2])
		angX += 1.5;
	if (bMouse[0])
		angX -= 1.5;

	glPushMatrix();
	glTranslated(objTect.getMid().x(), objTect.getMid().y(), objTect.getMid().z()); // Przy dynamicznej zmianie pozycji wierzcholkow, nalezy zaktualizowac srodek
	glRotated(angX, 1.0, 0.0, 0.0);
	glRotated(a, 0.0, 1.0, 0.0);
	glScaled(0.75, 0.75, 0.75);
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
	glutCreateWindow("Reprezentacja brzegowa");

	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glutReshapeFunc(Reshape);
	glutDisplayFunc(Display);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutIdleFunc(idle);

	glClearColor(0.15, 0.15, 0.15, 1);

	try {
		objTect = Mesh("Model.txt");

		cout << "[qQ] - Zakoncz dzialanie programu" << endl;
		cout << "[Lewy przycisk myszy | Prawy przycisk myszy] - Obrot wokol osi X" << endl;
	}
	catch (...) {
		cerr << "Nie udalo sie wczytac modelu." << endl;
	}

	glutMainLoop();

	exitGLUT();

    return 0;
}

// Model.txt

/*
W1[-1.5,-1.5,-5] W2[1.5,-1.5,-5] W3[1.5,1.5,-5] W4[-1.5,1.5,-5]
W5[-1.5,-1.5,-6.5] W6[1.5,-1.5,-6.5] W7[1.5,1.5,-6.5] W8[-1.5,1.5,-6.5]
W9[0,3,-5.75] W10[0,-3,-5.75]
p1[W1,W2,W3,W4]
p2[W6,W5,W8,W7]
p3[W5,W1,W4,W8]
p4[W2,W6,W7,W3]
p5[W5,W6,W2,W1]
p6[W4,W3,W7,W8]
p7[W4,W3,W9,W9]
p8[W3,W7,W9,W9]
p9[W7,W8,W9,W9]
p10[W8,W4,W9,W9]
*/
