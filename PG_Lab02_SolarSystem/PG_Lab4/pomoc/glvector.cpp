
#include "glpomoc.h"
#include <math.h>
#include <string.h>

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
    return (vVector[0]*vVector[0]) + (vVector[1]*vVector[1]) + (vVector[2]*vVector[2]); 
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
    return vU[0]*vV[0] + vU[1]*vV[1] + vU[2]*vV[2]; 
    }

// Iloczyn wektorowy dwoch wektorow
void glpVectorCrossProduct(const GLPVector3 vU, const GLPVector3 vV, GLPVector3 vResult)
	{
	vResult[0] = vU[1]*vV[2] - vV[1]*vU[2];
	vResult[1] = -vU[0]*vV[2] + vV[0]*vU[2];
	vResult[2] = vU[0]*vV[1] - vV[0]*vU[1];
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
    vOut[0] = mMatrix[0] * vSrcVector[0] + mMatrix[4] * vSrcVector[1] + mMatrix[8] *  vSrcVector[2] + mMatrix[12];
    vOut[1] = mMatrix[1] * vSrcVector[0] + mMatrix[5] * vSrcVector[1] + mMatrix[9] *  vSrcVector[2] + mMatrix[13];
    vOut[2] = mMatrix[2] * vSrcVector[0] + mMatrix[6] * vSrcVector[1] + mMatrix[10] * vSrcVector[2] + mMatrix[14];    
    }

// Obrot wektora poprzez macierz obrotu
void glpRotateVector(const GLPVector3 vSrcVector, const GLPMatrix mMatrix, GLPVector3 vOut)
    {
    vOut[0] = mMatrix[0] * vSrcVector[0] + mMatrix[4] * vSrcVector[1] + mMatrix[8] *  vSrcVector[2];
    vOut[1] = mMatrix[1] * vSrcVector[0] + mMatrix[5] * vSrcVector[1] + mMatrix[9] *  vSrcVector[2];
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
    return vPoint[0]*vPlane[0] + vPoint[1]*vPlane[1] + vPoint[2]*vPlane[2] + vPlane[3];
    }
    
