/*
	Michal Dedyk - grupa lab. 1 
*/
#include "stdafx.h"
#include <windows.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

class Stage;

/*
	Deklaracje z biblioteki glpomoc
*/

#define M_PI 3.14159265358979323846
#define GLP_PI	3.14159265358979323846
#define GLP_PI_DIV_180 0.017453292519943296
#define GLP_INV_PI_DIV_180 57.2957795130823229

#define glpDegToRad(x)	((x)*GLP_PI_DIV_180)
#define glpRadToDeg(x)	((x)*GLP_INV_PI_DIV_180)

typedef GLfloat GLPVector2[2];
typedef GLfloat GLPVector3[3];
typedef GLfloat GLPVector4[4];
typedef GLfloat GLPMatrix[16];

typedef struct {
	GLPVector3 vLocation;
	GLPVector3 vUp;
	GLPVector3 vForward;
} GLPFrame;

/*
	Deklaracje z biblioteki glpomoc
*/
void glpInitFrame(GLPFrame *pFrame);
void glpGetMatrixFromFrame(GLPFrame *pFrame, GLPMatrix mMatrix);
void glpApplyActorTransform(GLPFrame *pFrame);
void glpApplyCameraTransform(GLPFrame *pCamera);
void glpMoveFrameForward(GLPFrame *pFrame, GLfloat fStep);
void glpMoveFrameUp(GLPFrame *pFrame, GLfloat fStep);
void glpMoveFrameRight(GLPFrame *pFrame, GLfloat fStep);
void glpTranslateFrameWorld(GLPFrame *pFrame, GLfloat x, GLfloat y, GLfloat z);
void glpTranslateFrameLocal(GLPFrame *pFrame, GLfloat x, GLfloat y, GLfloat z);
void glpRotateFrameLocalX(GLPFrame *pFrame, GLfloat fAngle);
void glpRotateFrameLocalY(GLPFrame *pFrame, GLfloat fAngle);
void glpRotateFrameLocalZ(GLPFrame *pFrame, GLfloat fAngle);

/*
	Koniec deklaracji z biblioteki glpomoc
*/

bool bMouse[4] = { false, false, false, false };
GLfloat fButtonScale[2] = { 1, 1 };

enum SCALE_MODE {
	SCALE_PLANET = 0,
	SCALE_ORBIT = 1,
};

SCALE_MODE scalingMode = SCALE_PLANET;

void exitGLUT(void);
void Display();
void Reshape(int width, int height);
void Keyboard(unsigned char key, int x, int y);
void Mouse(int key, int state, int x, int y);
void SpecialKeys(int key, int x, int y);
void idle(void);

class Actor {
protected:
	GLfloat fScale; // Dla X, Y, Z taka sama skala
	GLfloat fAngle;
	GLfloat fPosition[3];
public:
	Actor() : fScale(0), fAngle(0), fPosition{ 0, 0, 0 } {}

	Actor(GLfloat aScale) : fScale(aScale), fAngle(0), fPosition{ 0, 0, 0 } {}

	virtual ~Actor() {}

	virtual void pAct(const double dDelta) {}

	virtual void pDraw(Stage & oStage, const double dDelta) = 0;

	virtual Actor * pCopy() const = 0;
};

class Sun : public Actor {
public:
	Sun() : Actor(25.0f) {}

	Sun(GLfloat aScale) : Actor(aScale) {}

	void pAct(const double dDelta) {
		fAngle += dDelta * 120.0f;
	}

	Actor * pCopy() const {
		return new Sun(fScale);
	}

	void pDraw(Stage & oStage, const double dDelta) {
		pAct(dDelta);

		glPushMatrix();
			glRotatef(fAngle, 1.0f, 0.3f, 0.0f); // Obracamy tylko w osi X

			glColor3f(1.0f, 0.843f, 0.0f);

			glutWireSphere(fScale, 48, 48);
		glPopMatrix();
	}
};

class Mercury : public Actor {
private:
	float fOrbitRadius;
public:
	Mercury() : Actor(12.5f), fOrbitRadius(60.0f) {}

	Mercury(GLfloat _fScale, float _fOrbitRadius) : Actor(_fScale), fOrbitRadius(_fOrbitRadius) {}

	Actor * pCopy() const {
		return new Mercury(fScale, fOrbitRadius);
	}

	void pAct(const double dDelta) {
		if (fAngle >= 2.0f * M_PI)
			fAngle = 0.0f;

		fAngle += dDelta * 4.5f;
	}

	void pDraw(Stage & oStage, const double dDelta) {
		pAct(dDelta);

		glPushMatrix();
			glRotatef(15.0f, 0.0f, 0.0f, 1.0f); // Obrot jest wspolny dla orbity i dla planety
			/*
			Merkury
			*/
			GLfloat _fOrbitRadius = fButtonScale[SCALE_ORBIT] * fOrbitRadius;
			glPushMatrix();
				glColor3f(0.3f, 0.3f, 0.3f);

				GLfloat pX = _fOrbitRadius * sin(fAngle);
				GLfloat pZ = _fOrbitRadius * cos(fAngle);

				glTranslatef(pX, 0.0f, pZ); // Orbituj w okol slonca

				GLfloat fPlanetScale = fButtonScale[SCALE_PLANET];
				glScalef(fPlanetScale, fPlanetScale, fPlanetScale);

				glutSolidSphere(fScale, 32, 32);

			glPopMatrix();

			/*
			Orbita merkurego
			*/
			glPushMatrix();
				glColor3f(0.5f, 0.4f, 0.3f);
				glPointSize(3.0f);
				glBegin(GL_POINTS);
				for (GLfloat fOrbit = 0.0f; fOrbit <= 2.0f * M_PI; fOrbit += 0.0005f)
					glVertex3f(_fOrbitRadius * sin(fOrbit), 0, _fOrbitRadius * cos(fOrbit));
				glEnd();
			glPopMatrix();

		glPopMatrix();
	}
};

class Venus : public Actor {
private:
	float fOrbitRadius;
public:
	Venus() : Actor(10.5f), fOrbitRadius(80.0f) {}

	Venus(GLfloat _fScale, float _fOrbitRadius) : Actor(_fScale), fOrbitRadius(_fOrbitRadius) {}

	Actor * pCopy() const {
		return new Venus(fScale, fOrbitRadius);
	}

	void pAct(const double dDelta) {
		if (fAngle >= 2.0f * M_PI)
			fAngle = 0.0f;

		fAngle += dDelta * 2.5f;
	}

	void pDraw(Stage & oStage, const double dDelta) {
		pAct(dDelta);

		glPushMatrix();
			glRotatef(15.0f, 0.0f, 0.0f, 1.0f); // Obrot jest wspolny dla orbity i dla planety
			/*
			Wenus
			*/
			GLfloat _fOrbitRadius = fButtonScale[SCALE_ORBIT] * fOrbitRadius;

			glPushMatrix();
				glColor3f(0.69f, 0.341f, 0.074f);

				GLfloat pX = _fOrbitRadius * sin(fAngle);
				GLfloat pZ = _fOrbitRadius * cos(fAngle);

				glTranslatef(pX, 0.0f, pZ); // Orbituj w okol slonca
				glRotatef(fAngle * 57.2958f, 1.0f, 0.0f, 1.0f);

				GLfloat fPlanetScale = fButtonScale[SCALE_PLANET];
				glScalef(fPlanetScale, fPlanetScale, fPlanetScale);

				glutSolidTorus(fScale, fScale * 2.0f, 32, 32);

			glPopMatrix();

				/*
				Orbita Wenus
				*/
				glPushMatrix();
				glColor3f(0.8f, 0.8f, 0.8f);
				glPointSize(4.0f);
				glBegin(GL_POINTS);
				for (GLfloat fOrbit = 0.0f; fOrbit <= 2.0f * M_PI; fOrbit += 0.0005f)
					glVertex3f(_fOrbitRadius * sin(fOrbit), 0, _fOrbitRadius * cos(fOrbit));
				glEnd();
			glPopMatrix();
		glPopMatrix();
	}
};


class Earth : public Actor {
private:
	float fMoonRadius;
	float fOrbitRadius;
	float fMoonAngle;
public:
	Earth() : Actor(10.0f), fMoonRadius(8.0f), fOrbitRadius(120.0f), fMoonAngle(0.0f) {}

	Earth(GLfloat _fScale, float _fOrbitRadius, float _fMoonRadius) : Actor(_fScale), fMoonRadius(8.0f), fOrbitRadius(_fOrbitRadius), fMoonAngle(0.0f) {}

	Actor * pCopy() const {
		return new Earth(fScale, fOrbitRadius, fMoonRadius);
	}

	void pAct(const double dDelta) {
		if (fAngle >= 2.0f * M_PI)
			fAngle = 0.0f;

		if (fMoonAngle >= 2.0f * M_PI)
			fMoonAngle = 0.0f;

		fAngle += dDelta * 2.1f;
		fMoonAngle += dDelta * 3.0f;
	}

	void pDraw(Stage & oStage, const double dDelta) {
		pAct(dDelta);

		glPushMatrix();
			/*
			Ziemia z „satelita” – szara kula
			*/
			GLfloat _fOrbitRadius = fButtonScale[SCALE_ORBIT] * fOrbitRadius;

			glRotatef(15.0f, 0.0f, 0.0f, 1.0f); // Obrot jest wspolny dla orbity i dla planety
			glPushMatrix();
				glColor3f(0.172f, 0.58f, 0.33f);

				GLfloat pX = _fOrbitRadius * sin(fAngle);
				GLfloat pZ = _fOrbitRadius * cos(fAngle);

				glTranslatef(pX, 0.0f, pZ); // Orbituj w okol slonca

				GLfloat fPlanetScale = fButtonScale[SCALE_PLANET];
				glScalef(fPlanetScale, fPlanetScale, fPlanetScale);

				glutSolidSphere(fScale, 32, 32);
			glPopMatrix();

			/*
				Orbita ziemi
			*/
			glPushMatrix();
				glColor3f(0.164f, 0.46f, 0.921f);
				glPointSize(5.0f);
				glBegin(GL_POINTS);
				for (GLfloat fOrbit = 0.0f; fOrbit <= 2.0f * M_PI; fOrbit += 0.0005f)
					glVertex3f(_fOrbitRadius * sin(fOrbit), 0, _fOrbitRadius * cos(fOrbit));
				glEnd();
			glPopMatrix();

			/*
				Satelita ziemii
			*/
			glTranslatef(_fOrbitRadius * sin(fAngle), 0.0f, _fOrbitRadius * cos(fAngle)); // Ksizeyc oraz orbita
			glRotatef(45.0f, 1.0f, 0.0f, 1.0f); // Ksiezyc oraz orbita maja taki sam obrot
			glScalef(fPlanetScale, fPlanetScale, fPlanetScale);

			glPushMatrix();
				glColor3f(0.25f, 0.25f, 0.25f);
				pX = fScale * 1.6f * sin(fMoonAngle);
				pZ = fScale * 1.6f * cos(fMoonAngle);
				glTranslatef(pX, 0.0f, pZ);
				glutSolidSphere(fMoonRadius, 32, 32);
			glPopMatrix();

			/*
				Orbita ksiezyca
			*/
			glPushMatrix();
				glColor3f(0.9f, 0.9f, 0.9f);
				glPointSize(3.0f);
				glBegin(GL_POINTS);
				for (GLfloat fOrbit = 0.0f; fOrbit <= 2.0f * M_PI; fOrbit += 0.0005f)
					glVertex3f(fScale * 1.6f * sin(fOrbit), 0, fScale * 1.6f * cos(fOrbit));
				glEnd();
				glPointSize(3.0f);
			glPopMatrix();
		glPopMatrix();
	}
};

class Mars : public Actor {
private:
	float fOrbitRadius;
public:
	Mars() : Actor(5.5f), fOrbitRadius(150.0) {}

	Mars(GLfloat _fScale, float _fOrbitRadius) : Actor(_fScale), fOrbitRadius(_fOrbitRadius) {}

	Actor * pCopy() const {
		return new Mars(fScale, fOrbitRadius);
	}

	void pAct(const double dDelta) {
		if (fAngle >= 2.0f * M_PI)
			fAngle = 0.0f;

		fAngle += dDelta;
	}

	void pDraw(Stage & oStage, const double dDelta) {
		pAct(dDelta);

		glPushMatrix();
			glRotatef(15.0f, 0.0f, 0.0f, 1.0f); // Obrot jest wspolny dla orbity i dla planety
			/*
			Mars
			*/
			GLfloat _fOrbitRadius = fButtonScale[SCALE_ORBIT] * fOrbitRadius;

			glPushMatrix();
				glColor3f(0.32, 0.19f, 0.09f);

				GLfloat pX = _fOrbitRadius * sin(fAngle);
				GLfloat pZ = _fOrbitRadius * cos(fAngle);

				glTranslatef(pX, 0.0f, pZ); // Orbituj w okol slonca

				GLfloat fPlanetScale = fButtonScale[SCALE_PLANET];
				glScalef(fPlanetScale, fPlanetScale, fPlanetScale);

				glutSolidCube(fScale);
			glPopMatrix();

			/*
			Orbita marsa
			*/
			glPushMatrix();
				glColor3f(0.81f, 0.68f, 0.59f);
				glPointSize(4.0f);
				glBegin(GL_POINTS);
				for (GLfloat fOrbit = 0.0f; fOrbit <= 2.0f * M_PI; fOrbit += 0.0005f)
					glVertex3f(_fOrbitRadius * sin(fOrbit), 0, _fOrbitRadius * cos(fOrbit));
				glEnd();
			glPopMatrix();
		glPopMatrix();
	}
};

class Jupiter : public Actor {
private:
	float fOrbitRadius;
public:
	Jupiter() : Actor(30.5f), fOrbitRadius(180.0f) {}

	Jupiter(GLfloat _fScale, float _fOrbitRadius) : Actor(_fScale), fOrbitRadius(_fOrbitRadius) {}

	Actor * pCopy() const {
		return new Jupiter(fScale, fOrbitRadius);
	}

	void pAct(const double dDelta) {
		if (fAngle >= 2.0f * M_PI)
			fAngle = 0.0f;

		fAngle += dDelta * 0.5f;
	}

	void pDraw(Stage & oStage, const double dDelta) {
		pAct(dDelta);

		glPushMatrix();
			glRotatef(15.0f, 0.0f, 0.0f, 1.0f); // Obrot jest wspolny dla orbity i dla planety
			/*
				Jowisz
			*/
			GLfloat _fOrbitRadius = fButtonScale[SCALE_ORBIT] * fOrbitRadius;

			glPushMatrix();
				glColor3f(0.69, 0.66f, 0.52);

				GLfloat pX = _fOrbitRadius * sin(fAngle);
				GLfloat pZ = _fOrbitRadius * cos(fAngle);

				glTranslatef(pX, 0.0f, pZ); // Orbituj w okol slonca

				GLfloat fPlanetScale = fButtonScale[SCALE_PLANET];
				glScalef(fPlanetScale, fPlanetScale, fPlanetScale);

				glutSolidSphere(fScale, 32, 32);
			glPopMatrix();

			/*
				Orbita jowisza
			*/
			glPushMatrix();
				glColor3f(0.52, 0.66f, 0.69f);
				glPointSize(4.0f);
				glBegin(GL_POINTS);
				for (GLfloat fOrbit = 0.0f; fOrbit <= 2.0f * M_PI; fOrbit += 0.0005f)
					glVertex3f(_fOrbitRadius * sin(fOrbit), 0, _fOrbitRadius * cos(fOrbit));
				glEnd();
			glPopMatrix();
		glPopMatrix();
	}
};

class Saturn : public Actor {
private:
	float fOrbitRadius;
public:
	Saturn() : Actor(10.0f), fOrbitRadius(270.0f) {}

	Saturn(GLfloat _fScale, float _fOrbitRadius) : Actor(_fScale), fOrbitRadius(_fOrbitRadius) {}

	Actor * pCopy() const {
		return new Saturn(fScale, fOrbitRadius);
	}

	void pAct(const double dDelta) {
		if (fAngle >= 2.0f * M_PI)
			fAngle = 0.0f;

		fAngle += dDelta * 0.3f;
	}

	void pDraw(Stage & oStage, const double dDelta) {
		pAct(dDelta);

		glPushMatrix();
			/*
				Saturn
			*/
			GLfloat _fOrbitRadius = fButtonScale[SCALE_ORBIT] * fOrbitRadius;

			glRotatef(15.0f, 0.0f, 0.0f, 1.0f); // Obrot jest wspolny dla orbity i dla planety
			glPushMatrix();
				glColor3f(0.77f, 0.666f, 0.35f);

				GLfloat pX = _fOrbitRadius * sin(fAngle);
				GLfloat pZ = _fOrbitRadius * cos(fAngle);

				glTranslatef(pX, 0.0f, pZ); // Orbituj w okol slonca

				GLfloat fPlanetScale = fButtonScale[SCALE_PLANET];
				glScalef(fPlanetScale, fPlanetScale, fPlanetScale);

				glutSolidSphere(fScale, 32, 32);
			glPopMatrix();

			/*
				Orbita saturna
			*/
			glPushMatrix();
				glColor3f(0.38f, 0.36f, 0.2f);
				glPointSize(5.0f);
				glBegin(GL_POINTS);
				for (GLfloat fOrbit = 0.0f; fOrbit <= 2.0f * M_PI; fOrbit += 0.0005f)
					glVertex3f(_fOrbitRadius * sin(fOrbit), 0, _fOrbitRadius * cos(fOrbit));
				glEnd();
			glPopMatrix();

			/*
				Pierscienie saturna
			*/
			glPushMatrix();
				glColor3f(0.63f, 0.75f, 0.85f);
				glTranslatef(pX, 0.0f, pZ + 4.0f);
				glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
				glScalef(fPlanetScale, fPlanetScale, fPlanetScale);
				glScalef(1.0f, 1.0f, 0.2f);
					glutSolidTorus(fScale * 0.5f, fScale * 1.7f, 32, 32);

			glPopMatrix();
		glPopMatrix();
	}
};

class Uranus : public Actor {
private:
	float fOrbitRadius;
public:
	Uranus() : Actor(15.5f), fOrbitRadius(350.f) {}

	Uranus(GLfloat _fScale, float _fOrbitRadius) : Actor(_fScale), fOrbitRadius(_fOrbitRadius) {}

	Actor * pCopy() const {
		return new Uranus(fScale, fOrbitRadius);
	}

	void pAct(const double dDelta) {
		if (fAngle >= 2.0f * M_PI)
			fAngle = 0.0f;

		fAngle += dDelta * 0.1f;
	}

	void pDraw(Stage & oStage, const double dDelta) {
		pAct(dDelta);

		glPushMatrix();
			GLfloat _fOrbitRadius = fButtonScale[SCALE_ORBIT] * fOrbitRadius;

			glRotatef(15.0f, 0.0f, 0.0f, 1.0f); // Obrot jest wspolny dla orbity i dla planety
			/*
			Uran
			*/
			glPushMatrix();
				glColor3f(0.28f, 0.43f, 0.56f);

				GLfloat pX = _fOrbitRadius * sin(fAngle);
				GLfloat pZ = _fOrbitRadius * cos(fAngle);

				glTranslatef(pX, 0.0f, pZ); // Orbituj w okol slonca

				GLfloat fPlanetScale = fButtonScale[SCALE_PLANET];
				glScalef(fPlanetScale, fPlanetScale, fPlanetScale);

				glutSolidSphere(fScale, 32, 32);
			glPopMatrix();

			/*
			Orbita uranu
			*/
			glPushMatrix();
				glColor3f(0.28f, 0.56f, 0.42f);
				glPointSize(3.0f);
				glBegin(GL_POINTS);
				for (GLfloat fOrbit = 0.0f; fOrbit <= 2.0f * M_PI; fOrbit += 0.0005f)
					glVertex3f(_fOrbitRadius * sin(fOrbit), 0, _fOrbitRadius * cos(fOrbit));
				glEnd();
			glPopMatrix();
		glPopMatrix();
	}
};

class Neptun : public Actor {
private:
	float fOrbitRadius;
public:
	Neptun() : Actor(12.5f), fOrbitRadius(400.0f) {}

	Neptun(GLfloat _fScale, float _fOrbitRadius) : Actor(_fScale), fOrbitRadius(_fOrbitRadius) {}

	Actor * pCopy() const {
		return new Neptun(fScale, fOrbitRadius);
	}

	void pAct(const double dDelta) {
		if (fAngle >= 2.0f * M_PI)
			fAngle = 0.0f;

		fAngle += dDelta * 0.05f;
	}

	void pDraw(Stage & oStage, const double dDelta) {
		pAct(dDelta);

		glPushMatrix();
			GLfloat _fOrbitRadius = fButtonScale[SCALE_ORBIT] * fOrbitRadius;

			glRotatef(15.0f, 0.0f, 0.0f, 1.0f); // Obrot jest wspolny dla orbity i dla planety
			/*
			Neptun
			*/
			glPushMatrix();
				glColor3f(0.28f, 0.26f, 0.56f);

				GLfloat pX = _fOrbitRadius * sin(fAngle);
				GLfloat pZ = _fOrbitRadius * cos(fAngle);

				glTranslatef(pX, 0.0f, pZ); // Orbituj w okol slonca

				GLfloat fPlanetScale = fButtonScale[SCALE_PLANET];
				glScalef(fPlanetScale, fPlanetScale, fPlanetScale);

				glutSolidTeapot(fScale);
			glPopMatrix();

			/*
			Orbita neptuna
			*/
			glPushMatrix();
				glColor3f(0.43f, 0.18f, 0.41f);
				glPointSize(3.0f);
				glBegin(GL_POINTS);
				for (GLfloat fOrbit = 0.0f; fOrbit <= 2.0f * M_PI; fOrbit += 0.0005f)
					glVertex3f(_fOrbitRadius * sin(fOrbit), 0, _fOrbitRadius * cos(fOrbit));
				glEnd();
			glPopMatrix();
		glPopMatrix();
	}
};

class Stage {
private:
	GLfloat cBackground[3];
	GLfloat fFOV;
	GLdouble dLastTime;
	GLPFrame fCamera;
	vector<Actor *> vActors;
public:
	Stage() : cBackground{ 0, 0, 0 }, fFOV(90), dLastTime(0), fCamera(), vActors(0) {
		glpInitFrame(&fCamera);

		glShadeModel(GL_SMOOTH);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT, GL_FILL);

		glpMoveFrameUp(&fCamera, 140.0f);
		glpMoveFrameForward(&fCamera, -600.0f);
		glpRotateFrameLocalX(&fCamera, -0.2f);
	}

	Stage(GLfloat fR, GLfloat fG, GLfloat fB, GLfloat _fFOV) : cBackground{ fR, fG, fB }, fFOV(_fFOV), dLastTime(0) {
		glpInitFrame(&fCamera);

		glShadeModel(GL_SMOOTH);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT, GL_FILL);

		glpMoveFrameUp(&fCamera, 140.0f);
		glpMoveFrameForward(&fCamera, -600.0f);
		glpRotateFrameLocalX(&fCamera, -0.2f);
	}

	Stage(const Stage & obj) : cBackground{ obj.cBackground[0], obj.cBackground[1], obj.cBackground[2] }, fFOV(obj.fFOV),  dLastTime(0), fCamera(obj.fCamera) {
		for (size_t i = 0; i < obj.vActors.size(); i++)
			vActors.push_back(obj.vActors[i]->pCopy());
	}

	Stage & operator=(const Stage & obj) {
		if (&obj != this) {
			for (std::vector<Actor *>::iterator it = vActors.begin(); it != vActors.end(); ++it) {
				delete *it;

				vActors.erase(it);
			}

			cBackground[0] = obj.cBackground[0];
			cBackground[1] = obj.cBackground[1];
			cBackground[2] = obj.cBackground[2];

			fFOV = obj.fFOV;
			fCamera = obj.fCamera;

			for (size_t i = 0; i < obj.vActors.size(); i++)
				vActors.push_back(obj.vActors[i]->pCopy());
		}
		return *this;
	}

	~Stage() {
		for (size_t i = 0; i < vActors.size(); i++)
			delete vActors[i];
	}

	void addActor(Actor * actor) {
		if (actor)
			vActors.push_back(actor);
	}

	void removeActor(Actor * actor) {
		if (actor)
			for (vector<Actor *>::iterator it = vActors.begin(); it != vActors.end(); ++it)
				if (actor == *it) {
					delete *it;
					vActors.erase(it);
				}
	}

	void removeActor(const unsigned & n) {
		if (vActors.size() > n) {
			delete vActors.at(n);
			vActors.erase(vActors.begin() + n);
		}
	}

	const GLfloat & getFOV() const { return fFOV; }

	GLPFrame & getCamera() { return fCamera;  }

	void sDraw(const double dTime) {
		if (bMouse[0])
			fButtonScale[scalingMode] += (dTime - dLastTime);
		if (bMouse[2])
			if ((fButtonScale[scalingMode] - (dTime - dLastTime)) > 0)
				fButtonScale[scalingMode] -= (dTime - dLastTime);

		glClearColor(cBackground[0], cBackground[1], cBackground[2], 1.0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glpApplyCameraTransform(&fCamera);

		glPushMatrix();
			for (size_t i = 0; i < vActors.size(); i++)
				vActors[i]->pDraw((*this), dTime - dLastTime);
		glPopMatrix();

		dLastTime = dTime;
		//glPopMatrix();
	}
};

Stage gStage;

void idle(void) {
	glutPostRedisplay();
}

void SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_LEFT:
		glpRotateFrameLocalY(&gStage.getCamera(), 0.03f); // Poruszanie kamery w lewo
		break;
	case GLUT_KEY_RIGHT:
		glpRotateFrameLocalY(&gStage.getCamera(), -0.03f); // Poruszanie kamery w prawo
		break;
	case GLUT_KEY_UP:
		glpRotateFrameLocalX(&gStage.getCamera(), 0.03f); // Poruszanie kamery w gore
		break;
	case GLUT_KEY_DOWN:
		glpRotateFrameLocalX(&gStage.getCamera(), -0.03f); // Poruszanie kamery w dol
		break;
	}
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
	case '1':
		scalingMode = SCALE_PLANET;
		cout << "Tryb skalowania planet." << endl;
		break;
	case '2':
		scalingMode = SCALE_ORBIT;
		cout << "Tryb skalowania orbit." << endl;
		break;
	case 'w':
		glpMoveFrameForward(&gStage.getCamera(), 4.0f);
		break;
	case 'a':
		glpMoveFrameRight(&gStage.getCamera(), 4.0f);
		break;
	case 's':
		glpMoveFrameForward(&gStage.getCamera(), -4.0f);
		break;
	case 'd':
		glpMoveFrameRight(&gStage.getCamera(), -4.0f);
		break;
	case 'q':
		exit(0);
		break;
	}
}

void Reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	GLdouble dAspectRatio = 1;

	if (height > 0)
		dAspectRatio = width / (GLdouble)height;
	gluPerspective(gStage.getFOV(), dAspectRatio, 1.0f, 5000.0f);
}

void Display()
{
	gStage.sDraw(glutGet(GLUT_ELAPSED_TIME) / 1000.0);

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
	glutCreateWindow("Przeksztalcenia geometryczne");

	glutReshapeFunc(Reshape);
	glutDisplayFunc(Display);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeys);
	glutMouseFunc(Mouse);
	glutIdleFunc(idle);

	gStage = Stage(0.1f, 0.1f, 0.1f, 80);
	gStage.addActor(new Sun(40.0f));
	gStage.addActor(new Mercury(10.0f, 60.0f));
	gStage.addActor(new Venus(7.0f, 90.0f));
	gStage.addActor(new Earth(15.0f, 125.0f, 8.0f));
	gStage.addActor(new Mars(10.0f, 162.0f));
	gStage.addActor(new Jupiter(30.0f, 210.0f));
	gStage.addActor(new Saturn(25.0f, 300.0f));
	gStage.addActor(new Uranus(15.0f, 380.0f));
	gStage.addActor(new Neptun(14.5f, 425.0f));

	cout << "[qQ]  - Wyjdz z programu" << endl;
	cout << "[1]   - Skalowanie planet" << endl;
	cout << "[2]   - Skalowanie orbit" << endl;
	cout << "[LPM] - Zwieksz skale" << endl;
	cout << "[PPM] - Zmniejsz skale" << endl;
	cout << "[wW] - Przesun kamere do przodu" << endl;
	cout << "[aA] - Przesun kamere w lewo" << endl;
	cout << "[sS] - Przesun kamere do tylu" << endl;
	cout << "[dD] - Przesun kamere w prawo" << endl;
	cout << "[Strzalka w gore]  - Obroc kamere do gory" << endl;
	cout << "[Strzalka w dol]   - Obroc kamere w dol" << endl;
	cout << "[Strzalka w prawo] - Obroc kamere w prawo" << endl;
	cout << "[Strzalka w lewo]  - Obroc kamere w lewo" << endl << endl;

	glutMainLoop();

	exitGLUT();
	return 0;
}

/*
	Definicje funkcji z biblioteki glpomoc
*/

// Ladowanie macierzy jednostkowej
void glpLoadIdentityMatrix(GLPMatrix m)
{
	static GLPMatrix identity = { 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };

	memcpy(m, identity, sizeof(GLPMatrix));
}

// Macierz obrotu
void glpRotationMatrix(float angle, float x, float y, float z, GLPMatrix mMatrix)
{
	float vecLength, sinSave, cosSave, oneMinusCos;
	float xx, yy, zz, xy, yz, zx, xs, ys, zs;

	if (x == 0.0f && y == 0.0f && z == 0.0f)
	{
		glpLoadIdentityMatrix(mMatrix);
		return;
	}

	vecLength = (float)sqrt(x*x + y*y + z*z);

	x /= vecLength;
	y /= vecLength;
	z /= vecLength;

	sinSave = (float)sin(angle);
	cosSave = (float)cos(angle);
	oneMinusCos = 1.0f - cosSave;

	xx = x * x;
	yy = y * y;
	zz = z * z;
	xy = x * y;
	yz = y * z;
	zx = z * x;
	xs = x * sinSave;
	ys = y * sinSave;
	zs = z * sinSave;

	mMatrix[0] = (oneMinusCos * xx) + cosSave;
	mMatrix[4] = (oneMinusCos * xy) - zs;
	mMatrix[8] = (oneMinusCos * zx) + ys;
	mMatrix[12] = 0.0f;

	mMatrix[1] = (oneMinusCos * xy) + zs;
	mMatrix[5] = (oneMinusCos * yy) + cosSave;
	mMatrix[9] = (oneMinusCos * yz) - xs;
	mMatrix[13] = 0.0f;

	mMatrix[2] = (oneMinusCos * zx) - ys;
	mMatrix[6] = (oneMinusCos * yz) + xs;
	mMatrix[10] = (oneMinusCos * zz) + cosSave;
	mMatrix[14] = 0.0f;

	mMatrix[3] = 0.0f;
	mMatrix[7] = 0.0f;
	mMatrix[11] = 0.0f;
	mMatrix[15] = 1.0f;
}

// Dodawanie dwoch wektorow
void glpAddVectors(const GLPVector3 vFirst, const GLPVector3 vSecond, GLPVector3 vResult) {
	vResult[0] = vFirst[0] + vSecond[0];
	vResult[1] = vFirst[1] + vSecond[1];
	vResult[2] = vFirst[2] + vSecond[2];
}

// Odejmowanie dwoch wektorow
void glpSubtractVectors(const GLPVector3 vFirst, const GLPVector3 vSecond, GLPVector3 vResult)
{
	vResult[0] = vFirst[0] - vSecond[0];
	vResult[1] = vFirst[1] - vSecond[1];
	vResult[2] = vFirst[2] - vSecond[2];
}

// Vektor skalarny
void glpScaleVector(GLPVector3 vVector, const GLfloat fScale)
{
	vVector[0] *= fScale; vVector[1] *= fScale; vVector[2] *= fScale;
}

// Dlugosc wektora kw
GLfloat glpGetVectorLengthSqrd(const GLPVector3 vVector)
{
	return (vVector[0] * vVector[0]) + (vVector[1] * vVector[1]) + (vVector[2] * vVector[2]);
}

// Dlugosc wektora
GLfloat glpGetVectorLength(const GLPVector3 vVector)
{
	return (GLfloat)sqrt(glpGetVectorLengthSqrd(vVector));
}

// Skalowanie wektora do jedynki
void glpNormalizeVector(GLPVector3 vNormal)
{
	GLfloat fLength = 1.0f / glpGetVectorLength(vNormal);
	glpScaleVector(vNormal, fLength);
}

// Kopiowanie wektora
void glpCopyVector(const GLPVector3 vSource, GLPVector3 vDest)
{
	memcpy(vDest, vSource, sizeof(GLPVector3));
}

// Iloczyn dwoch skalarny wektorow
GLfloat glpVectorDotProduct(const GLPVector3 vU, const GLPVector3 vV)
{
	return vU[0] * vV[0] + vU[1] * vV[1] + vU[2] * vV[2];
}

// Iloczyn wektorowy dwoch wektorow
void glpVectorCrossProduct(const GLPVector3 vU, const GLPVector3 vV, GLPVector3 vResult)
{
	vResult[0] = vU[1] * vV[2] - vV[1] * vU[2];
	vResult[1] = -vU[0] * vV[2] + vV[0] * vU[2];
	vResult[2] = vU[0] * vV[1] - vV[0] * vU[1];
}

// Wyznaczenie plaszczyzny normalnej
void glpGetNormalVector(const GLPVector3 vP1, const GLPVector3 vP2, const GLPVector3 vP3, GLPVector3 vNormal)
{
	GLPVector3 vV1, vV2;

	glpSubtractVectors(vP2, vP1, vV1);
	glpSubtractVectors(vP3, vP1, vV2);

	glpVectorCrossProduct(vV1, vV2, vNormal);
	glpNormalizeVector(vNormal);
}

// Przeksztalcenie punktu w macierz
void glpTransformPoint(const GLPVector3 vSrcVector, const GLPMatrix mMatrix, GLPVector3 vOut)
{
	vOut[0] = mMatrix[0] * vSrcVector[0] + mMatrix[4] * vSrcVector[1] + mMatrix[8] * vSrcVector[2] + mMatrix[12];
	vOut[1] = mMatrix[1] * vSrcVector[0] + mMatrix[5] * vSrcVector[1] + mMatrix[9] * vSrcVector[2] + mMatrix[13];
	vOut[2] = mMatrix[2] * vSrcVector[0] + mMatrix[6] * vSrcVector[1] + mMatrix[10] * vSrcVector[2] + mMatrix[14];
}

// Obrot wektora poprzez macierz obrotu
void glpRotateVector(const GLPVector3 vSrcVector, const GLPMatrix mMatrix, GLPVector3 vOut)
{
	vOut[0] = mMatrix[0] * vSrcVector[0] + mMatrix[4] * vSrcVector[1] + mMatrix[8] * vSrcVector[2];
	vOut[1] = mMatrix[1] * vSrcVector[0] + mMatrix[5] * vSrcVector[1] + mMatrix[9] * vSrcVector[2];
	vOut[2] = mMatrix[2] * vSrcVector[0] + mMatrix[6] * vSrcVector[1] + mMatrix[10] * vSrcVector[2];
}

// Wyznaczenie wspolczynnikow plaszczyzny
void glpGetPlaneEquation(GLPVector3 vPoint1, GLPVector3 vPoint2, GLPVector3 vPoint3, GLPVector3 vPlane)
{
	glpGetNormalVector(vPoint1, vPoint2, vPoint3, vPlane);

	vPlane[3] = -(vPlane[0] * vPoint3[0] + vPlane[1] * vPoint3[1] + vPlane[2] * vPoint3[2]);
}

// Odleglosz punktu od plaszczyzny
GLfloat glpDistanceToPlane(GLPVector3 vPoint, GLPVector4 vPlane)
{
	return vPoint[0] * vPlane[0] + vPoint[1] * vPlane[1] + vPoint[2] * vPlane[2] + vPlane[3];
}



// Inicjacja kamery
void glpInitFrame(GLPFrame *pFrame)
{
	pFrame->vLocation[0] = 0.0f;
	pFrame->vLocation[1] = 0.0f;
	pFrame->vLocation[2] = 0.0f;

	pFrame->vUp[0] = 0.0f;
	pFrame->vUp[1] = 1.0f;
	pFrame->vUp[2] = 0.0f;

	pFrame->vForward[0] = 0.0f;
	pFrame->vForward[1] = 0.0f;
	pFrame->vForward[2] = -1.0f;
}

// Wyznaczenie macierzy z ramki
void glpGetMatrixFromFrame(GLPFrame *pFrame, GLPMatrix mMatrix)
{
	GLPVector3 vXAxis;

	glpVectorCrossProduct(pFrame->vUp, pFrame->vForward, vXAxis);

	memcpy(mMatrix, vXAxis, sizeof(GLPVector3));
	mMatrix[3] = 0.0f;

	memcpy(mMatrix + 4, pFrame->vUp, sizeof(GLPVector3));
	mMatrix[7] = 0.0f;

	memcpy(mMatrix + 8, pFrame->vForward, sizeof(GLPVector3));
	mMatrix[11] = 0.0f;

	memcpy(mMatrix + 12, pFrame->vLocation, sizeof(GLPVector3));
	mMatrix[15] = 1.0f;
}

// Dodanie aktora
void glpApplyActorTransform(GLPFrame *pFrame)
{
	GLPMatrix mTransform;
	glpGetMatrixFromFrame(pFrame, mTransform);
	glMultMatrixf(mTransform);
}

// Dodanie kamery
void glpApplyCameraTransform(GLPFrame *pCamera)
{
	GLPMatrix mMatrix;
	GLPVector3 vAxisX;
	GLPVector3 zFlipped;

	zFlipped[0] = -pCamera->vForward[0];
	zFlipped[1] = -pCamera->vForward[1];
	zFlipped[2] = -pCamera->vForward[2];

	glpVectorCrossProduct(pCamera->vUp, zFlipped, vAxisX);

	mMatrix[0] = vAxisX[0];
	mMatrix[4] = vAxisX[1];
	mMatrix[8] = vAxisX[2];
	mMatrix[12] = 0.0f;

	mMatrix[1] = pCamera->vUp[0];
	mMatrix[5] = pCamera->vUp[1];
	mMatrix[9] = pCamera->vUp[2];
	mMatrix[13] = 0.0f;

	mMatrix[2] = zFlipped[0];
	mMatrix[6] = zFlipped[1];
	mMatrix[10] = zFlipped[2];
	mMatrix[14] = 0.0f;

	mMatrix[3] = 0.0f;
	mMatrix[7] = 0.0f;
	mMatrix[11] = 0.0f;
	mMatrix[15] = 1.0f;

	glMultMatrixf(mMatrix);

	glTranslatef(-pCamera->vLocation[0], -pCamera->vLocation[1], -pCamera->vLocation[2]);
}

// Ruch kamery do przodu
void glpMoveFrameForward(GLPFrame *pFrame, GLfloat fStep)
{
	pFrame->vLocation[0] += pFrame->vForward[0] * fStep;
	pFrame->vLocation[1] += pFrame->vForward[1] * fStep;
	pFrame->vLocation[2] += pFrame->vForward[2] * fStep;
}

// Ruch kamery do gory
void glpMoveFrameUp(GLPFrame *pFrame, GLfloat fStep)
{
	pFrame->vLocation[0] += pFrame->vUp[0] * fStep;
	pFrame->vLocation[1] += pFrame->vUp[1] * fStep;
	pFrame->vLocation[2] += pFrame->vUp[2] * fStep;
}

// Ruch kamery w prawo
void glpMoveFrameRight(GLPFrame *pFrame, GLfloat fStep)
{
	GLPVector3 vCross;

	glpVectorCrossProduct(pFrame->vUp, pFrame->vForward, vCross);
	pFrame->vLocation[0] += vCross[0] * fStep;
	pFrame->vLocation[1] += vCross[1] * fStep;
	pFrame->vLocation[2] += vCross[2] * fStep;
}

// Przesuniecie swiata
void glpTranslateFrameWorld(GLPFrame *pFrame, GLfloat x, GLfloat y, GLfloat z)
{
	pFrame->vLocation[0] += x; pFrame->vLocation[1] += y; pFrame->vLocation[2] += z;
}

// Przesuniecie kamery
void glpTranslateFrameLocal(GLPFrame *pFrame, GLfloat x, GLfloat y, GLfloat z)
{
	glpMoveFrameRight(pFrame, x);
	glpMoveFrameUp(pFrame, y);
	glpMoveFrameForward(pFrame, z);
}


// Obrot kamery
void glpRotateFrameLocalY(GLPFrame *pFrame, GLfloat fAngle)
{
	GLPMatrix mRotation;
	GLPVector3 vNewForward;

	glpRotationMatrix((float)glpDegToRad(fAngle), 0.0f, 1.0f, 0.0f, mRotation);
	glpRotationMatrix(fAngle, pFrame->vUp[0], pFrame->vUp[1], pFrame->vUp[2], mRotation);

	glpRotateVector(pFrame->vForward, mRotation, vNewForward);
	memcpy(pFrame->vForward, vNewForward, sizeof(GLPVector3));
}



//  Obrot kamery
void glpRotateFrameLocalX(GLPFrame *pFrame, GLfloat fAngle)
{
	GLPMatrix mRotation;
	GLPVector3 vCross{ 0, 0, 0 };

	glpVectorCrossProduct(pFrame->vForward, pFrame->vUp, vCross);
	glpRotationMatrix(fAngle, vCross[0], vCross[1], vCross[2], mRotation);

	GLPVector3 vNewVect;
	vNewVect[0] = mRotation[0] * pFrame->vForward[0] + mRotation[4] * pFrame->vForward[1] + mRotation[8] * pFrame->vForward[2];
	vNewVect[1] = mRotation[1] * pFrame->vForward[0] + mRotation[5] * pFrame->vForward[1] + mRotation[9] * pFrame->vForward[2];
	vNewVect[2] = mRotation[2] * pFrame->vForward[0] + mRotation[6] * pFrame->vForward[1] + mRotation[10] * pFrame->vForward[2];
	memcpy(pFrame->vForward, vNewVect, sizeof(GLfloat) * 3);

	vNewVect[0] = mRotation[0] * pFrame->vUp[0] + mRotation[4] * pFrame->vUp[1] + mRotation[8] * pFrame->vUp[2];
	vNewVect[1] = mRotation[1] * pFrame->vUp[0] + mRotation[5] * pFrame->vUp[1] + mRotation[9] * pFrame->vUp[2];
	vNewVect[2] = mRotation[2] * pFrame->vUp[0] + mRotation[6] * pFrame->vUp[1] + mRotation[10] * pFrame->vUp[2];
	memcpy(pFrame->vUp, vNewVect, sizeof(GLfloat) * 3);
}
// Obrot kamery
void glpRotateFrameLocalZ(GLPFrame *pFrame, GLfloat fAngle)
{
	GLPMatrix mRotation;

	glpRotationMatrix(fAngle, pFrame->vForward[0], pFrame->vForward[1], pFrame->vForward[2], mRotation);

	GLPVector3 vNewVect;
	vNewVect[0] = mRotation[0] * pFrame->vUp[0] + mRotation[4] * pFrame->vUp[1] + mRotation[8] * pFrame->vUp[2];
	vNewVect[1] = mRotation[1] * pFrame->vUp[0] + mRotation[5] * pFrame->vUp[1] + mRotation[9] * pFrame->vUp[2];
	vNewVect[2] = mRotation[2] * pFrame->vUp[0] + mRotation[6] * pFrame->vUp[1] + mRotation[10] * pFrame->vUp[2];
	memcpy(pFrame->vUp, vNewVect, sizeof(GLfloat) * 3);
}