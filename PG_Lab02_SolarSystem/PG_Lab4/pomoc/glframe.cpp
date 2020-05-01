#include "glpomoc.h"
#include <string.h>

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

    memcpy(mMatrix+4, pFrame->vUp, sizeof(GLPVector3));
    mMatrix[7] = 0.0f;

    memcpy(mMatrix+8, pFrame->vForward, sizeof(GLPVector3));
    mMatrix[11] = 0.0f;

    memcpy(mMatrix+12, pFrame->vLocation, sizeof(GLPVector3));
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
    { pFrame->vLocation[0] += x; pFrame->vLocation[1] += y; pFrame->vLocation[2] += z; }

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
    vNewVect[0] = mRotation[0] * pFrame->vUp[0] + mRotation[4] * pFrame->vUp[1] + mRotation[8] *  pFrame->vUp[2];
    vNewVect[1] = mRotation[1] * pFrame->vUp[0] + mRotation[5] * pFrame->vUp[1] + mRotation[9] *  pFrame->vUp[2];
    vNewVect[2] = mRotation[2] * pFrame->vUp[0] + mRotation[6] * pFrame->vUp[1] + mRotation[10] * pFrame->vUp[2];
    memcpy(pFrame->vUp, vNewVect, sizeof(GLfloat) * 3);
    }
