
#include "3DAnimApp.h"
#include <fstream>
#include <strstream>
#include <algorithm>
using namespace std;
using namespace cv;


struct vec3d // WEKTOR WE WSP. JEDNORODNYCH / KWATERNION
{
	float x = 0;
	float y = 0;
	float z = 0;
	float w = 1;
};

struct mat4x4 // MACIERZ ZEROWA 4X4
{
	float m[4][4] = { 0 };
};

struct color // KOLOR HSV
{
	uchar hue = 0; // odcieÒ
	uchar sat = 0; // nasycenie
	uchar val = 0; // jasnoúÊ
};

struct pTriangle // TR”JK•T PUNKTOWY
{
	vec3d p[3]; // wspÛ≥rzÍdne wierzcho≥kÛw 
	color col; // kolor trÛjkπta
};

struct iTriangle // TR”JK•T INDEKSOWY
{
	int points[3];
};

struct mesh // SIATKA TR”JK•T”W
{
	vector<vec3d> points; // punkty
	vector<iTriangle> tris; // trÛjkπty indeksowe
	string filename; // nazwa pliku siatki trÛjkπtÛw
};

struct orientedPoint // ZORIENTOWANY PUNKT W PRZESTRZENI
{
	vec3d vPosition; // wektor pozycji
	vec3d qRotation; // kwaternion rotacji
	float fScale = 1.0f; // wspÛ≥czynnik skali 
};

struct keyframe : orientedPoint // KLATKA KLUCZOWA
{
	float time = 0.0f; // czas
	int nextCam = 0; // indeks kamery do wykonania ciÍcia
};

struct element // ELEMENT SCENY
{
	vector<vec3d> pointMesh; // chmura punktÛw
	vector<keyframe> keyframes; // klatki kluczowe
	orientedPoint actPos; // aktualna transformacja
	orientedPoint prevPos; // poprzednia transformacja
	color col; // kolor
	int obj_name = 0; //nr identyfikacyjny siatki obiektu
};

struct scene // SCENA
{
	vector<element> cameras; // kamery
	vector<element> objects; // obiekty
	vector<element> lights; // ürÛd≥a úwiat≥a
	orientedPoint freeCam; // wolna kamera
	color col; // kolor etykiety
	int startCam = 1; // indeks kamery poczπtkowej
	float startTime = 0.0f; // czas poczπtkowy
	float endTime = 1.0f; // czas koÒcowy
};

struct letter // ZNAK FONTU
{
	char value = '`'; // znak
	bool pixels[45] = { 0 }; // siatka pikseli
};

struct button // PRZYCISK
{
	float posx;
	float posy; // pozycja
	float sizex;
	float sizey; // rozmiar
	float border; // gruboúÊ ramki
	string txt1; // tytu≥
	string txtKey; // opis
	string txt2;
	string sym; // symbol
};

struct window // OKNO
{
	float posx;
	float posy; //pozycja
	float sizex;
	float sizey; // rozmiar
	color col; // kolor t≥a
	vector<button> buttons; // przyciski
	vector<uchar> bStates; // aktualne stany przyciskÛw
	vector<uchar> bPrevStates; // poprzednie stany przyciskÛw
	string title = ""; // tytu≥
	bool line = 0;

	void NewButton(float bPosX, float bPosY, float bSizeX, float bSizeY, float bBorder,
		string bTxt1, string bKey = "", string bTxt2 = "", string bSym = "")
	{
		button bTmp = { bPosX,bPosY,bSizeX,bSizeY,bBorder,bTxt1,bKey,bTxt2,bSym };
		buttons.push_back(bTmp);
		bStates.push_back(2);
		bPrevStates.push_back(0);
	} // dodanie przycisku do okna
};


class olcEngine3D : public Animation3DEngine
{
public:
	olcEngine3D()
	{
		m_sAppName = L"3D Animation Engine";
	}


private:
	vector<scene> scenes;
	mat4x4 matProj;	// macierz projekcji
	vector<mesh> meshes; // siatki obiektÛw
	window windows[15];
	uchar winToDraw[6] = { 0,2,3,4,6,9 };
	int chosen_scn = 0;
	int chosen_obj = 0; // indeks aktualnie wybranego obiektu
	int chosen_cam = 0; // indeks aktualnie wybranej kamery
	int chosen_lgt = 0; // indeks aktualnie wybranego ürÛd≥a úwiat≥a
	float basePlane_dist = 1.0f;
	float basePlane_size = 40.0f; // wymiary pod≥oøa
	int showMode = 2; // tryb wyúwietlania
	int ElementType = 2; // typ elementu sceny w edycji
	int EditMode = 0; // tryb edycji
	float fSceneTime = 0.0f; // aktualny czas sceny
	bool playback = 0; // stan odtwarzania
	bool ObjNamesLoaded = 0;
	bool freeCamMode = 0;
	int anim = 0;
	bool checkTri = 1;
	bool switchKeyframe = 0;
	int top_cam = 0;
	int top_obj = 0;
	int top_lgt = 0;
	int top_scn = 0;
	int Ttop_cam = 0;
	int Ttop_obj = 0;
	int Ttop_lgt = 0;
	int Ttop_scn = 0;
	bool init = 1;
	int iOutFreeCam = 0;
	bool bDiffItem = 0;
	bool bDiffKeyframe = 0;
	bool bDiffColor = 0;
	uchar indWinKeyframe = 9;
	vector<button> buttons;
	vector<letter> font;

	// zdefiniowane kolory HSV
	color colButt = { 91,50,244 };
	color colBrdr = { 0,0,200 };
	color colWin = { 0,0,100 };
	color colWin2 = { 0,0,80 };
	color colWin3 = { 0,0,50 };
	color colWin4 = { 0,0,30 };
	color colBlack = { 0,0,0 };
	color colWhite = { 0,0,255 };

	// wymiary interfejsu graficznego
	float wNavigateWidth = screenWidth;
	float wNavigateHeight = screenHeight / 12;
	float wKeyframeWidth = screenWidth / 3;
	float wKeyframeHeight = screenHeight / 2;
	float wContentWidth = wKeyframeWidth;
	float wContentHeight = screenHeight - wNavigateHeight - wKeyframeHeight;
	float wElementWidth = screenWidth - wKeyframeWidth;
	float wElementHeight = screenHeight / 4;
	float wTimelineWidth = wElementWidth;
	float wTimelineHeight = screenHeight / 4;
	float wPreviewWidth = wElementWidth;
	float wPreviewHeight = screenHeight - wNavigateHeight - wElementHeight - wTimelineHeight;
	float wPreviewAR = wPreviewWidth / wPreviewHeight;
	float kFrame;
	float oyFrame;
	float oxFrame;
	float AR16x9 = 1.77777778;

	//----------------------------------------------------------------------------------------------//
	//--------------------------------------// WEKTORY //-------------------------------------------//
	//----------------------------------------------------------------------------------------------//

	float Vector_Norm(vec3d& v) // D£UGOå∆ WEKTORA
	{
		return sqrtf(Vector_DotProduct(v, v));
	}

	vec3d Vector_Normalise(vec3d& v) // NORMALIZACJA WEKTORA
	{
		float l = Vector_Norm(v);
		return { v.x / l, v.y / l, v.z / l };
	}

	vec3d Vector_Add(vec3d& v1, vec3d& v2) // SUMA WEKTOR”W
	{
		return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
	}

	vec3d Vector_Sub(vec3d& v1, vec3d& v2) // R”ZNICA WEKTOR”W
	{
		return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
	}

	vec3d Vector_Mul(vec3d& v1, float k) // MNOØENIE WEKTORA PRZEZ SKALAR
	{
		return { v1.x * k, v1.y * k, v1.z * k };
	}

	float Vector_DotProduct(vec3d& v1, vec3d& v2) // ILOCZYN SKALARNY DW”CH WEKTOR”W
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	vec3d Vector_CrossProduct(vec3d& v1, vec3d& v2) // ILOCZYN WEKTOROWY DW”CH WEKTOR”W
	{
		vec3d v;
		v.x = v1.y * v2.z - v1.z * v2.y;
		v.y = v1.z * v2.x - v1.x * v2.z;
		v.z = v1.x * v2.y - v1.y * v2.x;
		return v;
	}

	vec3d Vector_Interpolate(vec3d& startVec, vec3d& targetVec, float Ft) // INTERPOLACJA MIEDZY DWOMA WEKTORAMI
	{
		vec3d difVecPos = Vector_Sub(targetVec, startVec);
		float difVecPosLenght = Vector_Norm(difVecPos);
		difVecPos = Vector_Normalise(difVecPos);
		float MulFactorPos = Ft * difVecPosLenght;
		vec3d addVecMovePos = Vector_Mul(difVecPos, MulFactorPos);
		return Vector_Add(startVec, addVecMovePos);
	}

	bool Vectors_Equal(vec3d& Vec1, vec3d& Vec2) // SPRAWDZENIE CZY DWA WEKTORY S• TAKIE SAME
	{
		if ((Vec1.x == Vec2.x) && (Vec1.y == Vec2.y) && (Vec1.z == Vec2.z)) {
			return 1;
		}
		else {
			return 0;
		}
	}

	bool Vector_IntersectPlane(vec3d& intersect_p, vec3d& plane_p, vec3d& plane_n, vec3d& lineStart, vec3d& lineEnd) // ZNALEZIENIE PUNKTU PRZECI CIA LINII I P£ASZCZYZNY
	{
		vec3d v = Vector_Normalise(plane_n);//wektor normalny plaszczyzny
		vec3d q = Vector_Sub(plane_p, lineStart);//wektor od poczatku prostej do punktu na plaszczyznie
		vec3d u = Vector_Sub(lineEnd, lineStart);//wektor 'u' miÍdzy punktami na prostej (start -> koniec badanego odcinka)
		float vqDotProd = Vector_DotProduct(v, q);
		float vuDotProd = Vector_DotProduct(v, u);
		float t = vqDotProd / vuDotProd;
		vec3d tu = Vector_Mul(u, t);//skalowanie wektora od p
		intersect_p = Vector_Add(lineStart, tu);//wskazanie punktu przeciÍcia linii z p≥aszczyznπ
		if (t > 0 && t < 1) {
			return true;
		}
		else {
			return false;
		}
	}

	//----------------------------------------------------------------------------------------------//
	//-------------------------------------------// MACIERZE //-------------------------------------//
	//----------------------------------------------------------------------------------------------//

	mat4x4 Matrix_MakeIdentity() // MACIERZ JEDNOSTKOWA
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	vec3d Matrix_MultiplyVector(mat4x4& m, vec3d& i) // MNOØENIE MACIERZY PRZEZ WEKTOR
	{
		vec3d v;
		v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
		v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
		v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
		v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
		return v;
	}

	mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2) // MNOØENIE DW”CH MACIERZY
	{
		mat4x4 matrix;
		for (int c = 0; c < 4; c++) {
			for (int r = 0; r < 4; r++) {
				matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
			}
		}
		return matrix;
	}

	mat4x4 Matrix_MakeTranslation(float x, float y, float z) // MACIERZ TRANSLACJI
	{
		mat4x4 matrix;
		matrix.m[0][0] = 1.0f;
		matrix.m[1][1] = 1.0f;
		matrix.m[2][2] = 1.0f;
		matrix.m[3][3] = 1.0f;
		matrix.m[3][0] = x;
		matrix.m[3][1] = y;
		matrix.m[3][2] = z;
		return matrix;
	}

	mat4x4 Matrix_MakeScale(float fScale) // MACIERZ SKALUJACA
	{
		mat4x4 matrix;
		matrix.m[0][0] = fScale;
		matrix.m[1][1] = fScale;
		matrix.m[2][2] = fScale;
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeRotation(float fAngleRad, float a, float b, float c) // MACIERZ OBROTU W DOWOLNEJ OSI
	{
		mat4x4 matrix;
		matrix.m[0][0] = a * a * (1 - cosf(fAngleRad)) + cosf(fAngleRad);
		matrix.m[0][1] = a * b * (1 - cosf(fAngleRad)) - c * sinf(fAngleRad);
		matrix.m[0][2] = a * c * (1 - cosf(fAngleRad)) + b * sinf(fAngleRad);
		matrix.m[1][0] = a * b * (1 - cosf(fAngleRad)) + c * sinf(fAngleRad);
		matrix.m[1][1] = b * b * (1 - cosf(fAngleRad)) + cosf(fAngleRad);
		matrix.m[1][2] = b * c * (1 - cosf(fAngleRad)) - a * sinf(fAngleRad);
		matrix.m[2][0] = a * c * (1 - cosf(fAngleRad)) - b * sinf(fAngleRad);
		matrix.m[2][1] = b * c * (1 - cosf(fAngleRad)) + a * sinf(fAngleRad);
		matrix.m[2][2] = c * c * (1 - cosf(fAngleRad)) + cosf(fAngleRad);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_ChangeOfBasis(vec3d& vPos, vec3d& vForward, vec3d& vUp) // MACIERZ ZMIANY BAZY
	{
		vec3d ffnewRight = Vector_CrossProduct(vForward, vUp);
		mat4x4 matrix;
		matrix.m[0][0] = ffnewRight.x;	matrix.m[0][1] = ffnewRight.y;	matrix.m[0][2] = ffnewRight.z;	matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = vUp.x;			matrix.m[1][1] = vUp.y;			matrix.m[1][2] = vUp.z;			matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = vForward.x;	matrix.m[2][1] = vForward.y;	matrix.m[2][2] = vForward.z;	matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = vPos.x;		matrix.m[3][1] = vPos.y;		matrix.m[3][2] = vPos.z;		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_InverseView(mat4x4& m) // MACIERZ ODWROTNA, DO WYZNACZENIA MACIERZY KAMERY
	{
		mat4x4 matrix;
		matrix.m[0][0] = m.m[0][0]; matrix.m[0][1] = m.m[1][0]; matrix.m[0][2] = m.m[2][0]; matrix.m[0][3] = 0.0f;
		matrix.m[1][0] = m.m[0][1]; matrix.m[1][1] = m.m[1][1]; matrix.m[1][2] = m.m[2][1]; matrix.m[1][3] = 0.0f;
		matrix.m[2][0] = m.m[0][2]; matrix.m[2][1] = m.m[1][2]; matrix.m[2][2] = m.m[2][2]; matrix.m[2][3] = 0.0f;
		matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
		matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
		matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
		matrix.m[3][3] = 1.0f;
		return matrix;
	}

	mat4x4 Matrix_MakeProjection(float fFovDegrees, float fFrameRatio, float fNear, float fFar) // MACIERZ PROJEKCJI
	{
		float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
		mat4x4 matrix;
		matrix.m[0][0] = fFovRad;
		matrix.m[1][1] = fFrameRatio * fFovRad;
		matrix.m[2][2] = -fFar / (fFar - fNear);
		matrix.m[3][2] = -(-fFar * fNear) / (fFar - fNear);
		matrix.m[2][3] = -1.0f;
		matrix.m[3][3] = 0.0f;
		return matrix;
	}

	//----------------------------------------------------------------------------------------------//
	//------------------------------------// KWATERNIONY //-----------------------------------------//
	//----------------------------------------------------------------------------------------------//

	vec3d Quaternion_MakePure(vec3d& v) // UTWORZENIE KWATERNIONU CZYSTO UROJONEGO Z WEKTORA
	{
		vec3d q;
		q.w = 0;
		q.x = v.x;
		q.y = v.y;
		q.z = v.z;
		return q;
	}

	vec3d Quaternion_MakeAngleAxis(float fAngle, vec3d& vAxis) // UTWORZENIE KWATERNIONU Z WARTOåCI K•TA I WEKTORA OSI OBROTU
	{
		vec3d q;
		q.w = cos(fAngle / 2);
		q.x = sin(fAngle / 2) * vAxis.x;
		q.y = sin(fAngle / 2) * vAxis.y;
		q.z = sin(fAngle / 2) * vAxis.z;
		return q;
	}

	float Quaternion_Norm(vec3d& q) // MODU£ KWATERNIONU
	{
		float normQuat = sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
		return normQuat;
	}

	vec3d Quaternion_Normalise(vec3d& q) // NORMALIZACJA KWATERNIONU
	{
		float fQnorm = Quaternion_Norm(q);
		vec3d qUnitNorm;
		if (fQnorm == 0) {
			fQnorm = 1;
		}
		qUnitNorm.w = q.w / fQnorm;
		qUnitNorm.x = q.x / fQnorm;
		qUnitNorm.y = q.y / fQnorm;
		qUnitNorm.z = q.z / fQnorm;
		return qUnitNorm;
	}

	vec3d Quaternion_Conjugate(vec3d& q) // NEGACJA KWATERNIONU
	{
		vec3d qCon;
		qCon.w = q.w;
		qCon.x = -q.x;
		qCon.y = -q.y;
		qCon.z = -q.z;
		return qCon;
	}

	vec3d Quaternion_Invert(vec3d& q) // ODWROTNOå∆ KWATERNIONU
	{
		vec3d qInv = Quaternion_Conjugate(q);
		float fQnorm = Quaternion_Norm(q);
		qInv.w = qInv.w / (fQnorm * fQnorm);
		qInv.x = qInv.x / (fQnorm * fQnorm);
		qInv.y = qInv.y / (fQnorm * fQnorm);
		qInv.z = qInv.z / (fQnorm * fQnorm);
		return qInv;
	}

	vec3d Quaternion_CrossProduct(vec3d& q1, vec3d& q2) // MNOØENIE GEOMETRYCZNE (WEKTOROWE) DW”CH KWATERNION”W
	{
		vec3d qProd;
		float qDotProd = Vector_DotProduct(q1, q2);
		qProd.w = q1.w * q2.w - qDotProd;
		vec3d qPt1 = Vector_Mul(q2, q1.w);
		vec3d qPt2 = Vector_Mul(q1, q2.w);
		vec3d qPt3 = Vector_CrossProduct(q1, q2);
		vec3d qSum = Vector_Add(qPt1, qPt2);
		qSum = Vector_Add(qSum, qPt3);
		qProd.x = qSum.x;
		qProd.y = qSum.y;
		qProd.z = qSum.z;
		return qProd;
	}

	float Quaternion_DotProduct(vec3d& q1, vec3d& q2) // MNOØENIE SKALARNE DW”CH KWATERNION”W
	{
		float normq1 = Quaternion_Norm(q1);
		float normq2 = Quaternion_Norm(q2);
		float dotProd = (q1.w * q2.w + q1.x * q2.x + q1.y * q2.y + q1.z * q2.z) / (normq1 * normq2);
		return dotProd;
	}

	bool Quaternion_Equal(vec3d& q1, vec3d& q2) // SPRAWDZENIE CZY DWA KWATERNIONY S• TAKIE SAME
	{
		if ((q1.x == q2.x) && (q1.y == q2.y) && (q1.z == q2.z) && (q1.w == q2.w)) {
			return 1;
		}
		else {
			return 0;
		}
	}

	vec3d Quaternion_Slerp(vec3d& q1, vec3d& q2, float t) // INTERPOLACJA MI DZY DWOMA KWATERNIONAMI
	{
		float qDotProduct = Quaternion_DotProduct(q1, q2);
		if (qDotProduct < 0) {
			q2.x = -q2.x;
			q2.y = -q2.y;
			q2.z = -q2.z;
			q2.w = -q2.w;
		}
		float qAngleDif = acos(qDotProduct);
		float mod1 = sin((1 - t) * qAngleDif) / sin(qAngleDif);
		vec3d q1mod;
		q1mod.w = q1.w * mod1;
		q1mod.x = q1.x * mod1;
		q1mod.y = q1.y * mod1;
		q1mod.z = q1.z * mod1;
		float mod2 = sin(t * qAngleDif) / sin(qAngleDif);
		vec3d q2mod;
		q2mod.w = q2.w * mod2;
		q2mod.x = q2.x * mod2;
		q2mod.y = q2.y * mod2;
		q2mod.z = q2.z * mod2;
		vec3d qSlerp;
		qSlerp = Vector_Add(q1mod, q2mod);
		qSlerp.w = q1mod.w + q2mod.w;
		return qSlerp;
	}

	vec3d Quaternion_RotateVec(vec3d& pVec, vec3d& qAxis) // ROTACJA KWATERNIONOWA WEKTORA
	{
		vec3d normVec = pVec;
		vec3d qUnitNorm = Quaternion_Normalise(qAxis);
		vec3d pVecPure = Quaternion_MakePure(normVec);
		vec3d qAxisInv = Quaternion_Invert(qUnitNorm);
		vec3d pVecRotated = Quaternion_CrossProduct(qUnitNorm, pVecPure);
		pVecRotated = Quaternion_CrossProduct(pVecRotated, qAxisInv);
		pVecRotated.w = 1.0f;
		return pVecRotated;
	}

	void Quaterion_ChangeRotation(vec3d& quatRot, vec3d vAxis, float fTheta) // ZMIANA KWATERNIONU ROTACJI
	{
		vec3d vAxisRot = Quaternion_RotateVec(vAxis, quatRot); // rotacja wektora o aktualny kwaternion rotacji
		vec3d qNewRot = Quaternion_MakeAngleAxis(fTheta, vAxisRot); // kwaternion przyrostu rotacji
		quatRot = Quaternion_CrossProduct(qNewRot, quatRot);
	}

	//----------------------------------------------------------------------------------------------//
	//------------------------------// SIATKI TR”JK•T”W I SIATEK //---------------------------------//
	//----------------------------------------------------------------------------------------------//

	void PointMesh_Center(vector<vec3d>& Fmesh) // PRZESUNIECIE SRODKA GEOMETRYCZNEGO OBIEKTU NA (0,0,0)
	{
		float maxX = Fmesh[0].x;
		float minX = Fmesh[0].x;
		float maxY = Fmesh[0].y;
		float minY = Fmesh[0].y;
		float maxZ = Fmesh[0].z;
		float minZ = Fmesh[0].z;

		for (auto pointTmp : Fmesh) {
			maxX = max(maxX, pointTmp.x);
			minX = min(minX, pointTmp.x);
			maxY = max(maxY, pointTmp.y);
			minY = min(minY, pointTmp.y);
			maxZ = max(maxZ, pointTmp.z);
			minZ = min(minZ, pointTmp.z);
		}

		vec3d p_cntr; //úrodek ciÍøkoúci obiektu
		p_cntr.x = minX + (maxX - minX) / 2;
		p_cntr.y = minY + (maxY - minY) / 2;
		p_cntr.z = minZ + (maxZ - minZ) / 2;

		mat4x4 FmatCenter;
		FmatCenter = Matrix_MakeTranslation(-p_cntr.x, -p_cntr.y, -p_cntr.z);
		for (auto& pointCenter : Fmesh) { //translacja obiektu
			pointCenter = Matrix_MultiplyVector(FmatCenter, pointCenter);
		}
	}

	void PointMesh_Transform(element& obj) // TRANSFORMACJA CHMURY PUNKT”W
	{
		vec3d vUp0 = { 0,1,0 };
		vec3d vFwd0 = { 0,0,1 };
		vec3d vUpRot = Quaternion_RotateVec(vUp0, obj.actPos.qRotation);
		vec3d vFwdRot = Quaternion_RotateVec(vFwd0, obj.actPos.qRotation);

		mat4x4 matChangeBasis; // macierz zmiany bazy
		matChangeBasis = Matrix_ChangeOfBasis(obj.actPos.vPosition, vFwdRot, vUpRot);

		mat4x4 matWorld; // macierz ca≥kowitej transformacji obiektu
		matWorld = Matrix_MakeScale(obj.actPos.fScale);
		matWorld = Matrix_MultiplyMatrix(matWorld, matChangeBasis);

		int ObjIndex;
		if (ObjNamesLoaded == 0) { ObjIndex = 0; }
		else { ObjIndex = obj.obj_name; }

		obj.pointMesh.clear();
		for (auto point : meshes[ObjIndex].points) // transformacja punktÛw
		{
			vec3d pointTmp;
			pointTmp = Matrix_MultiplyVector(matWorld, point);
			obj.pointMesh.push_back(pointTmp);
		}
	}

	void Mesh_MakeCube(mesh& cubeMesh, bool is_cam = 0) // UTWORZENIE SZESCIANU
	{
		vec3d pB;
		for (int k = -1; k < 2; k = k + 2) {
			for (int m = -1; m < 2; m = m + 2) {
				for (int n = -1; n < 2; n = n + 2) {
					pB = { float(k),float(m),float(n) };
					cubeMesh.points.push_back(pB);
				}
			}
		}

		if (is_cam == 1) {
			for (int i = 0; i < 4; i++) {
				int i2Plus1 = 2 * i + 1;
				cubeMesh.points[i2Plus1].x = 2 * cubeMesh.points[i2Plus1].x;
				cubeMesh.points[i2Plus1].y = 2 * cubeMesh.points[i2Plus1].y;
			}
		}

		int cords[36] = { 0,2,4,4,6,5,5,7,1,1,3,0,2,3,6,1,0,5,2,6,4,6,7,5,7,3,1,3,2,0,3,7,6,0,4,5 };
		for (int t = 0; t < 12; t++) {
			iTriangle tB;
			tB.points[0] = cords[3 * t];
			tB.points[1] = cords[3 * t + 1];
			tB.points[2] = cords[3 * t + 2];
			cubeMesh.tris.push_back(tB);
		}
	}

	void Mesh_MakePlane(mesh& plane, float dist, float size) // UTWORZENIE SIATKI W P£ASZCZYèNIE XZ
	{
		for (int i = 0; i <= (size); i = i + dist) {
			for (int j = 0; j <= (size); j = j + dist) {
				vec3d base_p;
				base_p.x = j - size / 2;
				base_p.z = i - size / 2;
				plane.points.push_back(base_p);
			}
		}
		for (int n = 0; n < ((size / dist) * (size / dist)); n++) {
			iTriangle base_tri;
			base_tri.points[0] = n + n * dist / size;
			base_tri.points[2] = 1 + n + n * dist / size;
			base_tri.points[1] = size / dist + 1 + n + n * dist / size;
			plane.tris.push_back(base_tri);
		}
		for (int n = 0; n < ((size / dist) * (size / dist)); n++) {
			iTriangle base_tri;
			base_tri.points[0] = 1 + n + n * dist / size;
			base_tri.points[1] = size / dist + 1 + n + n * dist / size;
			base_tri.points[2] = size / dist + 2 + n + n * dist / size;
			plane.tris.push_back(base_tri);
		}
	}

	bool Mesh_Import(string sFilename, vector<mesh>& meshes) // FUNKCJA £ADUJ•CA SIATK  OBIEKTU Z PLIKU
	{
		mesh meshCube;
		Mesh_MakeCube(meshCube);
		meshes.push_back(meshCube);
		mesh meshCubeCam;
		Mesh_MakeCube(meshCubeCam, true);
		meshes.push_back(meshCubeCam);
		mesh meshBasePlane;
		Mesh_MakePlane(meshBasePlane, basePlane_dist, basePlane_size);
		meshes.push_back(meshBasePlane);

		vector<string> objFilenames;

		ifstream meshFile(sFilename); // ZA£ADOWANIE PLIKU Z NAZWAMI OBIEKT”W
		if (meshFile.is_open()) {
			while (!meshFile.eof()) {
				char dataLine[128];
				meshFile.getline(dataLine, 128);
				strstream dataNum;
				dataNum << dataLine;

				string nameObj;
				dataNum >> nameObj;
				objFilenames.push_back(nameObj);
			}
			for (int i = 0; i < objFilenames.size(); i++) {
				vector<pTriangle> meshTmp;

				ifstream f(objFilenames[i]);
				if (!f.is_open()) {
					meshes.push_back(meshCube);
				}
				else {
					mesh meshTmp; //lokalny wektor z wierzcho≥kami
					while (!f.eof()) {
						char line[128];
						f.getline(line, 128);
						strstream s;
						s << line;//za≥adowanie linijki tektu z pliku
						char junk;
						//≥adowanie wierzcho≥kÛw lokalnego wektora
						if ((line[0] == 'v') && (line[1] != 't') && (line[1] != 'n')) {
							vec3d v;
							s >> junk >> v.x >> v.y >> v.z;
							meshTmp.points.push_back(v);
						}
						else if (line[0] == 'f') {
							iTriangle triTmp;
							s >> junk >> triTmp.points[0] >> triTmp.points[1] >> triTmp.points[2];
							triTmp.points[0] = triTmp.points[0] - 1;
							triTmp.points[1] = triTmp.points[1] - 1;
							triTmp.points[2] = triTmp.points[2] - 1;
							meshTmp.tris.push_back(triTmp);
						}
					}
					meshTmp.filename = objFilenames[i];
					PointMesh_Center(meshTmp.points);
					meshes.push_back(meshTmp);
					meshTmp.points.clear();
					meshTmp.tris.clear();
				}
			}
			return true;
		}
		else {
			return false;
		}
	}

	int Triangle_Clip(vec3d plane_p, vec3d plane_n, pTriangle& in_tri, pTriangle& out_tri1, pTriangle& out_tri2) // PRZYCINANIE TR”JK•T”W WZGL DEM P£ASZCZYZNY
	{
		plane_n = Vector_Normalise(plane_n);

		auto dist = [&](vec3d& p) // odleg≥oúÊ punktu od p≥aszczyzny
		{
			vec3d n = Vector_Normalise(p);
			return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector_DotProduct(plane_n, plane_p));
		};

		float d0 = dist(in_tri.p[0]);
		float d1 = dist(in_tri.p[1]);
		float d2 = dist(in_tri.p[2]);

		vec3d* inside_points[3];  int nInsidePointCount = 0;
		vec3d* outside_points[3]; int nOutsidePointCount = 0; // wskaüniki do punktÛw zew./wewn.

		// klasyfikacja punktÛw na wew./zew. (na podstawie ujemnej/dodatniej odleg≥oúci od p≥aszczyzny)
		if (d0 >= 0) {
			inside_points[nInsidePointCount++] = &in_tri.p[0];
		}
		else {
			outside_points[nOutsidePointCount++] = &in_tri.p[0];
		}
		if (d1 >= 0) {
			inside_points[nInsidePointCount++] = &in_tri.p[1];
		}
		else {
			outside_points[nOutsidePointCount++] = &in_tri.p[1];
		}
		if (d2 >= 0) {
			inside_points[nInsidePointCount++] = &in_tri.p[2];
		}
		else {
			outside_points[nOutsidePointCount++] = &in_tri.p[2];
		}

		if (nInsidePointCount == 0) // brak punktÛw wewn.-zwrÛcenie 0 trÛjkπtÛw
		{
			return 0;
		}

		if (nInsidePointCount == 3) // brak punktÛw zewn.-zwrÛcenie 1 wejúciowy trÛjkπt
		{
			out_tri1 = in_tri;

			return 1;
		}

		if (nInsidePointCount == 1 && nOutsidePointCount == 2) // 1 punkt wewn.-zwrÛcenie 1 nowego trÛjkπta
		{
			out_tri1.col = in_tri.col;
			//zachowanie wewnÍtrznego wierzcho≥ka
			out_tri1.p[0] = *inside_points[0];

			//dwa nowe wierzcho≥ki (pkt. przeciÍcia linii i p≥aszczyzny)
			Vector_IntersectPlane(out_tri1.p[1], plane_p, plane_n, *inside_points[0], *outside_points[0]);
			Vector_IntersectPlane(out_tri1.p[2], plane_p, plane_n, *inside_points[0], *outside_points[1]);

			return 1;
		}

		if (nInsidePointCount == 2 && nOutsidePointCount == 1) // 1 punkt zewn.-zwrÛcenie 2 nowych trÛjkπtÛw
		{
			out_tri1.col = in_tri.col;
			out_tri2.col = in_tri.col;

			//pierwszy nowy trÛjkπt (dwa stare wierzcho≥ki i nowy z pkt. przeciÍcia linii i p≥aszczyzny)
			out_tri1.p[0] = *inside_points[0];
			out_tri1.p[1] = *inside_points[1];
			Vector_IntersectPlane(out_tri1.p[2], plane_p, plane_n, *inside_points[0], *outside_points[0]);

			//drugi nowy trÛjkπt (jeden stary wierzcho≥ek i dwa nowe z pkt. przeciÍcia linii i p≥aszczyzny)
			out_tri2.p[0] = *inside_points[1];
			out_tri2.p[1] = out_tri1.p[2];
			Vector_IntersectPlane(out_tri2.p[2], plane_p, plane_n, *inside_points[1], *outside_points[0]);

			return 2;
		}
	}

	vec3d Triangle_Center(pTriangle& tri)
	{
		vec3d lineCenter = { (tri.p[0].x + tri.p[1].x) / 2,(tri.p[0].y + tri.p[1].y) / 2,(tri.p[0].z + tri.p[1].z) / 2 };
		return { (lineCenter.x + tri.p[2].x) / 2,(lineCenter.y + tri.p[2].y) / 2,(lineCenter.z + tri.p[2].z) / 2 };
	};

	void Triangle_Shader(vector<pTriangle>& pTris, element& elem, vector<element>& elems, mat4x4& matView, vector<element>& lights, orientedPoint& vCam, bool mixColor = 0) // SHADER TR”JK•T”W
	{
		int ObjIndex;
		if (ObjNamesLoaded == 0) {
			ObjIndex = 0;
		}
		else
		{
			ObjIndex = elem.obj_name;
		}

		for (auto meshTri : meshes[ObjIndex].tris) {

			pTriangle triTransformed; // utworzenie trÛjkπta punktowego z indeksowego
			triTransformed.p[0] = elem.pointMesh[meshTri.points[0]];
			triTransformed.p[1] = elem.pointMesh[meshTri.points[1]];
			triTransformed.p[2] = elem.pointMesh[meshTri.points[2]];

			vec3d normal, vec0to1, vec0to2;
			vec0to1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
			vec0to2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);
			normal = Vector_CrossProduct(vec0to2, vec0to1);// wektor normalny p≥aszczyzny trÛjkata

			vec3d triCenter = Triangle_Center(triTransformed); // úrodek trÛjkπta
			vec3d vCameraRay = Vector_Sub(vCam.vPosition, triCenter);// wektor od trÛjkπta do kamery

			if (Vector_DotProduct(normal, vCameraRay) > 0.0f) // sprawdzenie zgodnoúci kierunkÛw wektorÛw
			{
				triTransformed.col = elem.col;

				float valColTri = 0;
				float hueColTri = float(triTransformed.col.hue);
				float satColTri = float(triTransformed.col.sat);
				for (int lit = 0; lit < lights.size(); lit++) // oúwietlenie trÛjkπta przez kolejne ürÛd≥a úwiat≥a
				{
					float valLit = 0.0f;
					vec3d light_direction = Vector_Sub(lights[lit].actPos.vPosition, triCenter); // wektor od trÛjkπta do ür. úw.
					float light_dist = max(1.0f, Vector_Norm(light_direction));// odleg≥oúÊ od trÛjkπta do ür. úw.
					light_direction = Vector_Normalise(light_direction);
					normal = Vector_Normalise(normal);

					if (light_dist < 100.0f) // maksymalna odleg≥oúÊ do ktÛrej trÛjkπt jest oúwietlany
					{
						valLit = max(0.1f, Vector_DotProduct(light_direction, normal)); // zgodnoúÊ normalnej z kierunkiem úwiat≥a
						valLit = lights[lit].actPos.fScale * 10 * valLit / light_dist; // jasnoúÊ trÛjkπta od danego ür. úw.
					}

					valColTri = valColTri + valLit; // suma jasnoúci trÛjkπta od kolejnych ürÛde≥ úwiat≥a

					if (mixColor == 1) // mieszanie barwy trÛjkπta i úwiat≥a
					{
						vec3d vecColTri = { satColTri / 255,0,0 }; // wektory o d≥ugoúci nasycenia bawrwy
						vec3d vecColLgt = { float(lights[lit].col.sat) * valLit / 255,0,0 };
						mat4x4 matRotColTri = Matrix_MakeRotation(2 * hueColTri * 3.14159265 / 180, 0, -1, 0);
						mat4x4 matRotColLgt = Matrix_MakeRotation(2 * float(lights[lit].col.hue) * 3.14159265 / 180, 0, -1, 0);
						vecColTri = Matrix_MultiplyVector(matRotColTri, vecColTri);
						vecColLgt = Matrix_MultiplyVector(matRotColLgt, vecColLgt); // rotacja wektorÛw o kπt odpow. odcieniowi barwy
						vec3d vecColSum = Vector_Add(vecColTri, vecColLgt); // suma wektorÛw barw

						satColTri = Vector_Norm(vecColSum);
						if (satColTri > 1) {
							satColTri = 1;
						}
						satColTri = satColTri * 255; // nasycenie barwy oúwietlonego trÛjkπta

						vecColSum = Vector_Normalise(vecColSum);
						vec3d vecBaseCol = { 1,0,0 };
						vec3d vecAxisCol = { 0,-1,0 };
						float dotProd = Vector_DotProduct(vecBaseCol, vecColSum);
						vec3d crossProd = Vector_CrossProduct(vecBaseCol, vecColSum);
						float hueAnglSum = acos(dotProd);
						hueAnglSum = hueAnglSum * 180.0 / 3.14159265;
						if (Vector_DotProduct(crossProd, vecAxisCol) > 0) {
							hueAnglSum = 360 - hueAnglSum;
						}
						hueColTri = hueAnglSum / 2; // odcieÒ barwy oúwietlonego trÛjkπta

					}
				}

				if (valColTri >= 1.0f) {
					triTransformed.col.val = 255;
				}
				else {
					triTransformed.col.val = uchar(255 * valColTri);
				}
				triTransformed.col.hue = uchar(hueColTri);
				triTransformed.col.sat = uchar(satColTri); // przypisanie barwy trÛjkπtowi

				pTriangle triViewed;
				for (int i = 0; i < 3; i++) {
					triViewed.p[i] = Matrix_MultiplyVector(matView, triTransformed.p[i]); // zmiana bazy do przestrzeni kamery
				}
				triViewed.col = triTransformed.col;

				int nClippedTriangles = 0;
				pTriangle clipped[2]; // trÛjkπty wynikowe przycinania wzgl. p≥aszczyzny bliskiej
				nClippedTriangles = Triangle_Clip({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);
				for (int n = 0; n < nClippedTriangles; n++)
				{
					pTriangle triProjected;
					for (int i = 0; i < 3; i++) {
						triProjected.p[i] = Matrix_MultiplyVector(matProj, clipped[n].p[i]); // projekcja trÛjkπta

						triProjected.col = clipped[n].col;
						triProjected.p[i] = Vector_Mul(triProjected.p[i], (1 / triProjected.p[i].w)); // skrÛcenie perspektywiczne

						vec3d vScreenMove = { 1,1,0 };
						triProjected.p[i] = Vector_Add(triProjected.p[i], vScreenMove);
						triProjected.p[i].x *= 0.5f * frameWidth;
						triProjected.p[i].y *= 0.5f * frameHeight; // mapowanie ekranu
					}
					pTris.push_back(triProjected); // wpisanie trÛjkπta w kolejkÍ do rasteryzacji
				}
			}
		}
	}

public:

	//----------------------------------------------------------------------------------------------//
	//----------------------------------// EDYCJA SCEN I ELEMENT”W //-------------------------------//
	//----------------------------------------------------------------------------------------------//

	uchar Color_SetChannel(uchar uColor, float fTime, int iStep, int iMax, bool bLoop = 0) // ZMIANA SK£ADOWEJ KOLORU OBIEKTU
	{
		if ((iStep > 0 && uColor + iStep * fTime <= iMax) || (iStep < 0 && uColor + iStep * fTime >= 0)) {
			uColor = uColor + iStep * fTime;
		}
		else if (bLoop) {
			if (iStep > 0) {
				uColor = 0;
			}
			else {
				uColor = iMax;
			}
		}
		else {
			if (iStep > 0) {
				uColor = iMax;
			}
			else {
				uColor = 0;
			}
		}
		return uColor;
	}

	void Element_Color(color& col, float fElapsedTime) // ZMIANA KOLORU ELEMENTU / SCENY
	{
		windows[8].bStates[0] = 1;
		windows[8].bStates[1] = 2;
		windows[8].bStates[2] = 1;
		windows[8].bStates[3] = 2;
		if (GetKey(VK_RIGHT).bHeld) {
			bDiffColor = 1;
			windows[8].bStates[3] = 3;
			//zwiÍkszenie kπta barwy
			col.hue = Color_SetChannel(col.hue, fElapsedTime, 20, 180, 1);
		}
		else if (GetKey(VK_LEFT).bHeld) {
			bDiffColor = 1;
			windows[8].bStates[1] = 3;
			//zmniejszenie kπta barwy
			col.hue = Color_SetChannel(col.hue, fElapsedTime, -20, 180, 1);
		}

		if (col.sat < 255) {
			windows[8].bStates[0] = 2;
			//moøliwoúÊ zwiÍkszenia wartoúci saturacji
			if (GetKey(VK_UP).bHeld) {
				bDiffColor = 1;
				windows[8].bStates[0] = 3;
				//zwiÍkszenie wartoúci saturacji
				col.sat = Color_SetChannel(col.sat, fElapsedTime, 50, 255, 0);
			}
		}

		if (col.sat > 0) {
			windows[8].bStates[2] = 2;
			//moøliwoúÊ zmniejszenia wartoúci saturacji
			if (GetKey(VK_DOWN).bHeld) {
				bDiffColor = 1;
				windows[8].bStates[2] = 3;
				//zmniejszenie wartoúci saturacji
				col.sat = Color_SetChannel(col.sat, fElapsedTime, -50, 255, 0);
			}
		}
	}

	int Element_Add(vector<element>& elements, int elem_name) // DODANIE NOWEGO ELEMENTU SCENY
	{
		element elmTmp;
		elmTmp.obj_name = elem_name;

		keyframe keyframeTmp;
		elmTmp.keyframes.push_back(keyframeTmp);
		elements.push_back(elmTmp);

		int chosen_elem = elements.size() - 1;
		elements[chosen_elem].pointMesh = meshes[elements[chosen_elem].obj_name].points;

		return chosen_elem;
	}

	int Element_Del(vector<element>& elements, int chosen_elem) // USUNI CIE WYBRANEGO ELEMENTU SCENY
	{
		vector<element> objectsTmp;
		for (int s = elements.size() - 1; s > chosen_elem; s--) {
			objectsTmp.push_back(elements[s]);
			elements.pop_back();
		}

		elements.pop_back();

		for (int s = objectsTmp.size() - 1; s >= 0; s--) {
			elements.push_back(objectsTmp[s]);
		}
		objectsTmp.clear();

		if (chosen_elem > elements.size() - 1) {
			chosen_elem = elements.size() - 1;
		}
		if (chosen_elem < 0) {
			chosen_elem = 0;
		}

		return chosen_elem;
	}

	int Element_Next(vector<element>& elements, int chosen_elem) // PRZE£•CZENIE ELEMENTU SCENY NA NAST PNY
	{
		chosen_elem++;
		if (chosen_elem >= elements.size()) {
			chosen_elem = 0;
		}
		return chosen_elem;
	}

	int Element_Prev(vector<element>& elements, int chosen_elem) // PRZE£•CZENIE ELMENTU SCENY NA POPRZEDNI
	{
		chosen_elem--;
		if (chosen_elem < 0) {
			chosen_elem = elements.size() - 1;
		}
		return chosen_elem;
	}

	float Element_FindLastKeyframe(vector<element>& elements, float testTime) // SPRAWDZENIE CZY CZAS PRZEKROCZY£ OSTATNI• KLATK  KLUCZOW• ELEMENTU
	{
		for (int i = 0; i < elements.size(); i++) {
			if (testTime < elements[i].keyframes[elements[i].keyframes.size() - 1].time) {
				testTime = elements[i].keyframes[elements[i].keyframes.size() - 1].time;
			}
		}
		return testTime;
	}

	float Scene_FindLastKeyframe(scene& sceneTest, float fTime) // SPRAWDZENIE CZY CZAS PRZEKROCZY£ OSTATNI• KLATK  KLUCZOW• W SCENIE
	{
		float lastFrTime = 0.0f;
		lastFrTime = Element_FindLastKeyframe(sceneTest.cameras, lastFrTime);
		lastFrTime = Element_FindLastKeyframe(sceneTest.objects, lastFrTime);
		lastFrTime = Element_FindLastKeyframe(sceneTest.lights, lastFrTime);
		if (fTime > lastFrTime) {
			fTime = lastFrTime;
		}
		return fTime;
	}

	int Scene_Move(vector<scene>& scns, int chsn_scn, int sceneSub = 0) // PRZESUNI CIE SCENY W KOLEJNOåCI SEKWENCJI SCEN
	{
		vector<scene> scenesTmp;
		for (int s = scns.size() - 1; s >= chsn_scn - sceneSub; s--) {
			scenesTmp.push_back(scns[s]);
			scns.pop_back();
		}
		scns.push_back(scenesTmp[scenesTmp.size() - 2]);
		scns.push_back(scenesTmp[scenesTmp.size() - 1]);
		for (int s = scenesTmp.size() - 3; s >= 0; s--) {
			scns.push_back(scenesTmp[s]);
		}
		scenesTmp.clear();
		if (sceneSub == 0) {
			chsn_scn++;
		}
		else {
			chsn_scn--;
		}
		return chsn_scn;
	}

	//----------------------------------------------------------------------------------------------//
	//----------------------------------// EDYCJA KLATEK KLUCZOWYCH //------------------------------//
	//----------------------------------------------------------------------------------------------//

	int Keyframe_Add(vector<keyframe>& keyfrms, orientedPoint& actpos, int frame_num, float fTime) // DODANIE KLATKI KLUCZOWEJ W RUCHU WYBRANEGO ELEMENTU SCENY
	{
		keyframe keyframeTmp;
		keyframeTmp.vPosition = actpos.vPosition;
		keyframeTmp.qRotation = actpos.qRotation;
		keyframeTmp.fScale = actpos.fScale;
		keyframeTmp.nextCam = 0;

		if (fSceneTime == keyfrms[frame_num].time) {
			fSceneTime = fSceneTime + 2 * fTime;
		}
		keyframeTmp.time = fSceneTime;

		vector<keyframe> vecKeyframeTmp;
		for (int s = keyfrms.size() - 1; s > frame_num; s--) {
			vecKeyframeTmp.push_back(keyfrms[s]);
			keyfrms.pop_back();
		}
		keyfrms.push_back(keyframeTmp);
		for (int s = vecKeyframeTmp.size() - 1; s >= 0; s--) {
			keyfrms.push_back(vecKeyframeTmp[s]);
		}
		vecKeyframeTmp.clear();
		frame_num++;
		return frame_num;
	}

	int Keyframe_Del(vector<keyframe>& keyfrms, int frame_num) // USUNI CIE WYBRANEJ KLATKI KLUCZOWEJ W RUCHU WYBRANEGO ELEMENTU SCENY
	{
		vector<keyframe> vecKeyframeTmp;
		for (int k = keyfrms.size() - 1; k > frame_num; k--) {
			vecKeyframeTmp.push_back(keyfrms[k]);
			keyfrms.pop_back();
		}
		keyfrms.pop_back();
		for (int k = vecKeyframeTmp.size() - 1; k >= 0; k--) {
			keyfrms.push_back(vecKeyframeTmp[k]);
		}
		vecKeyframeTmp.clear();
		frame_num--;

		return frame_num;
	}

	float Keyframe_Next(vector<keyframe>& keyfrs, float fTime, int frame_num) // PRZE£•CZENIE WYBRANEJ KLATKI KLUCZOWEJ NA NAST PN•
	{
		int nextFrame_num = frame_num + 1;
		if (fTime < keyfrs[keyfrs.size() - 1].time)
		{
			fTime = keyfrs[nextFrame_num].time;
		}
		return fTime;
	}

	float Keyframe_Prev(vector<keyframe>& keyfrs, float fTime, int frame_num) // PRZE£•CZENIE WYBRANEJ KLATKI KLUCZOWEJ NA POPRZEDNI•
	{
		int prevFrame_num = frame_num - 1;
		if (fTime == keyfrs[frame_num].time && frame_num > 0)
		{
			fTime = keyfrs[prevFrame_num].time;
		}
		else {
			fTime = keyfrs[frame_num].time;
		}
		return fTime;
	}

	float Keyframe_IncreaseTime(vector<keyframe>& keyfrs, float fTime, float fElTime, int frame_num) // PRZESUNI CIE KLATKI KLUCZOWEJ W CZASIE W PRZ”D
	{
		int nextFrame_num = frame_num + 1;
		if (fTime == keyfrs[frame_num].time && fTime > 0)
		{
			if (frame_num >= keyfrs.size() - 1) {
				fTime = fTime + fElTime;
				keyfrs[frame_num].time = fTime;
			}
			else if (fSceneTime + fElTime < keyfrs[nextFrame_num].time) {
				fTime = fTime + fElTime;
				keyfrs[frame_num].time = fTime;
			}
		}
		return fTime;
	}

	float Keyframe_DecreaseTime(vector<keyframe>& keyfrs, float fTime, float fElTime, int frame_num) // PRZESUNI CIE KLATKI KLUCZOWEJ W CZASIE W TY£ 
	{
		int prevFrame_num = frame_num - 1;
		if (fTime == keyfrs[frame_num].time && fTime > 0 && fTime > keyfrs[prevFrame_num].time + fElTime)
		{
			fTime = fTime - fElTime;
			keyfrs[frame_num].time = fTime;
		}
		return fTime;
	}

	int Keyframe_Find(vector<keyframe>& keyfrs, float Ftime) // WYZNACZENIE KLATKI KLUCZOWEJ PO KT”REJ ZNAJDUJE SI  KAMERA W AKTUALNYM CZASIE
	{
		int frameNum = keyfrs.size() - 1;
		for (int fm = 1; fm < keyfrs.size(); fm++) {
			if (Ftime < keyfrs[fm].time) {
				frameNum = fm - 1;
				break;
			}
		}
		return frameNum;
	}

	orientedPoint Keyframe_Interpolation(vector<keyframe>& keyfrs, float fTime) // INTERPOLACJA KLATKI KLUCZOWEJ
	{
		orientedPoint camTmp;
		if (fTime >= keyfrs[keyfrs.size() - 1].time) {
			camTmp.vPosition = keyfrs[keyfrs.size() - 1].vPosition;
			camTmp.qRotation = keyfrs[keyfrs.size() - 1].qRotation;
			camTmp.fScale = keyfrs[keyfrs.size() - 1].fScale;
		}
		else if (fTime <= 0) {
			camTmp.vPosition = keyfrs[0].vPosition;
			camTmp.qRotation = keyfrs[0].qRotation;
			camTmp.fScale = keyfrs[0].fScale;
		}
		else {
			int Sframe = Keyframe_Find(keyfrs, fTime);
			int Sframe1 = Sframe + 1;

			float t = (fTime - keyfrs[Sframe].time) / (keyfrs[Sframe1].time - keyfrs[Sframe].time);
			if (Vectors_Equal(keyfrs[Sframe].vPosition, keyfrs[Sframe1].vPosition)) {
				camTmp.vPosition = keyfrs[Sframe].vPosition;
			}
			else {
				camTmp.vPosition = Vector_Interpolate(keyfrs[Sframe].vPosition, keyfrs[Sframe1].vPosition, t);
			}

			if (Quaternion_Equal(keyfrs[Sframe].qRotation, keyfrs[Sframe1].qRotation)) {
				camTmp.qRotation = keyfrs[Sframe].qRotation;
			}
			else {
				camTmp.qRotation = Quaternion_Slerp(keyfrs[Sframe].qRotation, keyfrs[Sframe1].qRotation, t);
			}

			if (keyfrs[Sframe].fScale == keyfrs[Sframe1].fScale) {
				camTmp.fScale = keyfrs[Sframe].fScale;
			}
			else {
				camTmp.fScale = keyfrs[Sframe].fScale + (keyfrs[Sframe1].fScale - keyfrs[Sframe].fScale) * t;
			}
		}
		return camTmp;
	}

	int Keyframe_Load(vector<element>& elements, strstream& data, int cnt_elem) // ZA£ADOWANIE KLATKI KLUCZOWEJ
	{
		float fr_time;
		data >> fr_time;
		int elem_number;
		data >> elem_number;
		int elem_hue, elem_sat;
		data >> elem_hue >> elem_sat;

		keyframe keyframeTmp;
		keyframeTmp.time = fr_time;
		data >> keyframeTmp.vPosition.x >> keyframeTmp.vPosition.y >> keyframeTmp.vPosition.z
			>> keyframeTmp.qRotation.x >> keyframeTmp.qRotation.y >> keyframeTmp.qRotation.z >> keyframeTmp.qRotation.w
			>> keyframeTmp.fScale >> keyframeTmp.nextCam;

		if (fr_time == 0.0f) {
			element elementTmp;
			elementTmp.keyframes.push_back(keyframeTmp);
			elementTmp.obj_name = elem_number;
			elementTmp.col.sat = uchar(elem_sat);
			elementTmp.col.hue = uchar(elem_hue);
			elementTmp.actPos.vPosition = keyframeTmp.vPosition;
			elementTmp.actPos.qRotation = keyframeTmp.qRotation;
			elementTmp.actPos.fScale = keyframeTmp.fScale;
			vec3d tmpVec3d;
			elementTmp.prevPos.vPosition = tmpVec3d;
			elementTmp.prevPos.qRotation = tmpVec3d;
			elementTmp.prevPos.fScale = 0;
			elements.push_back(elementTmp);

			cnt_elem++;
		}
		else {
			elements[cnt_elem].keyframes.push_back(keyframeTmp);
		}

		return cnt_elem;
	}

	void Keyframe_Save(vector<element>& elements, ofstream& data, char char_ind)  // ZAPISANIE PARAMETR”W KLATKI KLUCZOWEJ
	{
		for (int i = 0; i < elements.size(); i++) {
			for (int k = 0; k < elements[i].keyframes.size(); k++) {
				data << char_ind << ' ' << elements[i].keyframes[k].time << ' ' << elements[i].obj_name << ' ' << int(elements[i].col.hue) << ' ' << int(elements[i].col.sat)
					<< ' ' << elements[i].keyframes[k].vPosition.x << ' ' << elements[i].keyframes[k].vPosition.y << ' ' << elements[i].keyframes[k].vPosition.z
					<< ' ' << elements[i].keyframes[k].qRotation.x << ' ' << elements[i].keyframes[k].qRotation.y << ' ' << elements[i].keyframes[k].qRotation.z << ' ' << elements[i].keyframes[k].qRotation.w
					<< ' ' << elements[i].keyframes[k].fScale << ' ' << elements[i].keyframes[k].nextCam << endl;
			}
		}
	}

	void Keyframe_Translate(vec3d& point, vec3d& quatRot, float fElTime) // TRANSLACJA ELEMENTU PRZEZ UØYTKOWNIKA
	{
		windows[12].bStates[2] = 2;
		windows[12].bStates[3] = 2;
		windows[12].bStates[4] = 2;
		windows[12].bStates[5] = 2;
		windows[12].bStates[6] = 2;
		windows[12].bStates[7] = 2;
		float fValue = 4.0f;
		vec3d vMoveVec = { 0,0,0 };
		if (GetKey(L'W').bHeld) {
			windows[12].bStates[3] = 3;
			vMoveVec.z = vMoveVec.z + fValue;
		}
		if (GetKey(L'S').bHeld) {
			windows[12].bStates[6] = 3;
			vMoveVec.z = vMoveVec.z - fValue;
		}
		if (GetKey(L'A').bHeld) {
			windows[12].bStates[5] = 3;
			vMoveVec.x = vMoveVec.x - fValue;
		}
		if (GetKey(L'D').bHeld) {
			windows[12].bStates[7] = 3;
			vMoveVec.x = vMoveVec.x + fValue;
		}
		if (GetKey(L'E').bHeld) {
			windows[12].bStates[4] = 3;
			vMoveVec.y = vMoveVec.y + fValue;
		}
		if (GetKey(L'Q').bHeld) {
			windows[12].bStates[2] = 3;
			vMoveVec.y = vMoveVec.y - fValue;
		}
		vec3d vecRot = Quaternion_RotateVec(vMoveVec, quatRot);
		vec3d vecRotScl = Vector_Mul(vecRot, 4.0f * fElTime);
		point = Vector_Add(point, vecRotScl);
	}

	void Keyframe_Rotate(vec3d& quat, float fElapsedTime) // ROTACJA ELEMENTU PRZEZ UØYTKOWNIKA
	{
		windows[12].bStates[8] = 2;
		windows[12].bStates[9] = 2;
		windows[12].bStates[10] = 2;
		windows[12].bStates[11] = 2;
		windows[12].bStates[12] = 2;
		windows[12].bStates[13] = 2;
		if (GetKey(L'J').bHeld) {
			windows[12].bStates[12] = 3;
			Quaterion_ChangeRotation(quat, { -1,0,0 }, 2.0f * fElapsedTime);
		}
		else if (GetKey(L'U').bHeld) {
			windows[12].bStates[9] = 3;
			Quaterion_ChangeRotation(quat, { -1,0,0 }, -2.0f * fElapsedTime);
		}

		if (GetKey(L'H').bHeld) {
			windows[12].bStates[11] = 3;
			Quaterion_ChangeRotation(quat, { 0,0,1 }, 2.0f * fElapsedTime);
		}
		else if (GetKey(L'K').bHeld) {
			windows[12].bStates[13] = 3;
			Quaterion_ChangeRotation(quat, { 0,0,1 }, -2.0f * fElapsedTime);
		}

		if (GetKey(L'I').bHeld) {
			windows[12].bStates[10] = 3;
			Quaterion_ChangeRotation(quat, { 0,1,0 }, 2.0f * fElapsedTime);
		}
		else if (GetKey(L'Y').bHeld) {
			windows[12].bStates[8] = 3;
			Quaterion_ChangeRotation(quat, { 0,1,0 }, -2.0f * fElapsedTime);
		}
	}

	//----------------------------------------------------------------------------------------------//
	//-----------------------------// GRAFICZNY INTERFEJS UØYTKOWNIKA //----------------------------//
	//----------------------------------------------------------------------------------------------//

	vector<letter> LoadFont(string filename) // ZA£ADOWANIE FONTU
	{
		vector<letter> fontTmp;
		ifstream fontfile(filename);
		if (!fontfile.is_open()) {
			letter letterTmp;
			fontTmp.push_back(letterTmp);
		}
		else {
			while (!fontfile.eof()) {
				letter letterTmp;
				char dataLine[128];
				fontfile.getline(dataLine, 128);
				strstream dataNum;
				dataNum << dataLine;
				dataNum >> letterTmp.value;
				int pixel;
				while (dataNum >> pixel) {
					letterTmp.pixels[pixel] = 1;
				}
				fontTmp.push_back(letterTmp);
			}
		}
		return fontTmp;
	}

	void Time_Format(float fTime, string& sFormatTime) // FORMATOWANIE CZASU
	{
		sFormatTime = "   :  :   ";
		if (fTime < 0) {
			sFormatTime[0] = '-';
		}
		float fTimeSigned = abs(fTime);
		int minutes = fTimeSigned / 60;
		int seconds = fTimeSigned - minutes * 60;
		float fmiliseconds = 100 * (fTimeSigned - seconds - minutes * 60);
		int miliseconds = fmiliseconds;

		int min10 = minutes / 10;
		int min1 = minutes - 10 * min10;
		int sec10 = seconds / 10;
		int sec1 = seconds - 10 * sec10;
		int msc10 = miliseconds / 10;
		int msc1 = miliseconds - 10 * msc10;

		sFormatTime[1] = min10 + 48;
		sFormatTime[2] = min1 + 48;
		sFormatTime[4] = sec10 + 48;
		sFormatTime[5] = sec1 + 48;
		sFormatTime[7] = msc10 + 48;
		sFormatTime[8] = msc1 + 48;
	}

	float Text_FindLength(string text, float scale = 0, bool isAutoWidth = true) // ZNALEZIENIE D£UGOåCI TEKSTU
	{
		if (scale == 0) {
			scale = screenHeight / 400;
		}
		float lengthx = 0;
		for (int s = 0; s < text.size(); s++) {
			char LetChar = text[s];
			if (LetChar == ' ') {
				lengthx = lengthx + scale * 3;
			}
			else {
				for (int i = 0; i < font.size(); i++) {
					if (font[i].value == LetChar) {
						int lastPix;
						for (int c = 0; c < 45; c++) {
							if (font[i].pixels[c] == true) {
								lastPix = c;
							}
						}
						int charWidth = 5;
						if (isAutoWidth == 1) {
							charWidth = 5 - (44 - lastPix) / 9;
						}
						lengthx = lengthx + scale * (1 + charWidth);
						break;
					}
					else if (i == font.size() - 1) {
						lengthx = lengthx + scale * 6;
					}
				}
			}
		}
		return lengthx;
	}

	float Text_Write(string text, float posx, float posy, float scale = 0, bool isAutoWidth = true, uchar val = 255, uchar hue = 0, uchar sat = 0) // PISANIE TEKSTU
	{
		if (scale == 0) {
			scale = screenHeight / 400;
		}
		for (int s = 0; s < text.size(); s++) {
			char LetChar = text[s];
			if (LetChar == ' ') {
				posx = posx + scale * 3;
			}
			else {
				for (int i = 0; i < font.size(); i++) {
					if (font[i].value == LetChar) {
						int lastPix;
						for (int c = 0; c < 45; c++) {
							if (font[i].pixels[c] == true) {
								lastPix = c;
								for (int m = 0; m < scale; m++) {
									for (int n = 0; n < scale; n++) {
										Draw(posx + scale * int(c / 9) + m, posy + scale * (c - 9 * int(c / 9)) + 1 + n, val, hue, sat);
									}
								}

							}
						}
						int charWidth = 5;
						if (isAutoWidth == 1) {
							charWidth = 5 - (44 - lastPix) / 9;
						}
						posx = posx + scale * (1 + charWidth);
						break;
					}
					else if (i == font.size() - 1) {
						for (int c = 0; c < 45; c++) {
							if (font[font.size() - 1].pixels[c] == true) {
								for (int m = 0; m < scale; m++) {
									for (int n = 0; n < scale; n++) {
										Draw(posx + scale * int(c / 9) + m, posy + scale * (c - 9 * int(c / 9)) + 1 + n, val, hue, sat);
									}
								}
							}
						}
						posx = posx + scale * 6;
					}
				}
			}
		}
		return posx;
	}

	void DrawButton(button& bTmp, float winx, float winy, bool open, color colFill, color colBrdr, color colText, bool bBrdr) // RYSOWANIE PRZYCISKU
	{
		Fill(winx + bTmp.posx, winy + bTmp.posy, winx + bTmp.posx + bTmp.sizex, winy + bTmp.posy + bTmp.sizey, colFill.val, colFill.hue, colFill.sat);//wypelnienie przycisku
		if (bBrdr == 1) {
			Fill(winx + bTmp.posx, winy + bTmp.posy, winx + bTmp.posx + bTmp.border, winy + bTmp.posy + bTmp.sizey, colBrdr.val, colBrdr.hue, colBrdr.sat);
			Fill(winx + bTmp.posx, winy + bTmp.posy, winx + bTmp.posx + bTmp.sizex, winy + bTmp.posy + bTmp.border, colBrdr.val, colBrdr.hue, colBrdr.sat);
			Fill(winx + bTmp.posx + bTmp.sizex - bTmp.border, winy + bTmp.posy, winx + bTmp.posx + bTmp.sizex, winy + bTmp.posy + bTmp.sizey, colBrdr.val, colBrdr.hue, colBrdr.sat);//ramka przycisku
			if (open == 0) {
				Fill(winx + bTmp.posx, winy + bTmp.posy + bTmp.sizey - bTmp.border, winx + bTmp.posx + bTmp.sizex, winy + bTmp.posy + bTmp.sizey, colBrdr.val, colBrdr.hue, colBrdr.sat);
			}
		}
		float symLenght = 0;
		symLenght = Text_FindLength(bTmp.sym, 4, 1);
		Text_Write(bTmp.sym, winx + bTmp.posx + 2 * bTmp.border, winy + bTmp.posy + 2 * bTmp.border, 4, 1, colText.val, colText.hue, colText.sat);
		float txtLength = 0;
		txtLength = Text_FindLength(bTmp.txt1, 2, 1);
		Text_Write(bTmp.txt1, symLenght + winx + bTmp.posx + (bTmp.sizex - symLenght) / 2 - txtLength / 2, winy + bTmp.posy + 2 * bTmp.border, 2, 1, colText.val, colText.hue, colText.sat);
		txtLength = Text_FindLength(bTmp.txt2, 2, 1);
		Text_Write(bTmp.txt2, symLenght + winx + bTmp.posx + (bTmp.sizex - symLenght) / 2 - txtLength / 2, winy + bTmp.posy + 2 * bTmp.border + 18, 2, 1, colText.val, colText.hue, colText.sat);
		txtLength = Text_FindLength(bTmp.txtKey, 1.6, 1);
		Text_Write(bTmp.txtKey, symLenght + winx + bTmp.posx + (bTmp.sizex - symLenght) / 2 - txtLength / 2, winy + bTmp.posy + bTmp.sizey - 24, 1.6, 1, colText.val + 140, colText.hue, colText.sat);
	}

	void DrawWindow(window& wTmp) // RYSOWANIE OKNA
	{
		float margin = 0.07 * screenHeight / 2;
		float tab = 0.056 * screenHeight - 1;
		Fill(wTmp.posx, wTmp.posy, wTmp.posx + wTmp.sizex, wTmp.posy + wTmp.sizey, wTmp.col.val, wTmp.col.hue, wTmp.col.sat);//wypelnienie okna
		if (wTmp.line == 1) {
			Fill(wTmp.posx, wTmp.posy, wTmp.posx + wTmp.sizex, wTmp.posy + margin + tab, wTmp.col.val - 20, wTmp.col.hue, wTmp.col.sat);
			DrawLine(wTmp.posx, wTmp.posy + margin + tab, wTmp.posx + wTmp.sizex, wTmp.posy + margin + tab, 255, 0, 0);
		}
		DrawLine(wTmp.posx, wTmp.posy, wTmp.posx + wTmp.sizex, wTmp.posy, 255, 0, 0);
		for (int i = 0; i < 15; i++) {
			for (int s = 0; s < windows[i].buttons.size(); s++) {
				windows[i].bPrevStates[s] = 0;
			}
		}
	}

	void UpdateWindow(window& wTmp, string title = "", int number = -2) // AKTUALIZACJA ZAWARTOåCI OKNA
	{
		if (title.size() > 0) {
			int titleLenghtMin = Text_FindLength("ZRODLO SWIATLA 99", 2, 1);
			int titleLenght = Text_FindLength(title, 2, 1);
			int length = max(titleLenght, titleLenghtMin);
			Fill(wTmp.posx + 5, wTmp.posy + 5, wTmp.posx + length + 40, wTmp.posy + 30, 80);
			float ofst = Text_Write(title, wTmp.posx + 10, wTmp.posy + 8, 2);//tytul okna
			if (number > -2) {
				string sNumber = "  ";
				int nmbr = number + 1;
				if (nmbr < 10) {
					sNumber[0] = nmbr + 48;
				}
				else {
					int tens = nmbr / 10;
					int sngl = nmbr - 10 * tens;
					sNumber[0] = tens + 48;
					sNumber[1] = sngl + 48;
				}
				Text_Write(sNumber, ofst + 5, wTmp.posy + 8, 2);//numer pozycji
			}
		}
		int i = 0;
		for (auto butt : wTmp.buttons) {
			if (wTmp.bStates[i] != wTmp.bPrevStates[i]) {
				button bTmp = butt;
				color colFill = colButt;
				bool open = 0;
				color colText = colBlack;
				bool bBorder = 1;
				switch (wTmp.bStates[i]) {
				case 0:
					colFill = colWin;
					bTmp.sym = "";
					bTmp.txt1 = "";
					bTmp.txt2 = "";
					bTmp.txtKey = "";
					bBorder = 0;
					break;
				case 1:
					colFill = colButt;
					colFill.val = colFill.val - 100;
					colFill.sat = colFill.sat - 20;
					break;
				case 2:
					colFill = colButt;
					break;
				case 3:
					colFill = colButt;
					colFill.sat = colFill.sat + 100;
					colFill.val = colFill.val - 40;
					break;
				case 4:
					colFill = colWin;
					colText = colWhite;
					bTmp.txtKey = "";
					break;
				case 5:
					colFill = colWin;
					colText = colWhite;
					bTmp.txtKey = "";
					open = 1;
					break;
				}
				DrawButton(bTmp, wTmp.posx, wTmp.posy, open, colFill, colWhite, colText, bBorder);
			}
			i++;
		}
	}

	void DrawColorWheel(float cntrx, float cntry, float radius) // RYSOWANIE KO£A BARW
	{
		Fill(cntrx - radius - 10, cntry - radius - 10, cntrx + radius + 10, cntry + radius + 10, colWin.val, colWin.hue, colWin.sat);
		vec3d vHueStart = { 0,-1,0 };
		for (int hue = 720; hue > 0; hue--) {
			float hueRad = float(hue) * 3.14159265 / 360;
			mat4x4 matHueRot;
			matHueRot = Matrix_MakeRotation(hueRad, 0.0f, 0.0f, -1.0f);
			vec3d vHueRot = Matrix_MultiplyVector(matHueRot, vHueStart);
			for (int sat = 255; sat > 0; sat--) {
				float lVecHue = float(sat) / 255.0f;
				float kVecHue = lVecHue * radius;
				vec3d vHueRotSat = Vector_Mul(vHueRot, kVecHue);
				Draw(cntrx + vHueRotSat.x, cntry + vHueRotSat.y, 255, uchar(hue / 4), uchar(sat));
			}
			for (int n = 3; n > 0; n--) {
				float kVecHue = radius + n;
				vec3d vHueRotSat = Vector_Mul(vHueRot, kVecHue);
				Draw(cntrx + vHueRotSat.x, cntry + vHueRotSat.y);
			}
		}
	}

	void DrawColorIndicator(float cntrx, float cntry, float radius, uchar hue, uchar sat) // RYSOWANIE WSKAèNIKA NA KOLE BARW
	{
		float fHue = 6.28 * float(hue) / 180;
		vec3d vHueStart = { 0,-1,0 };
		mat4x4 matHueRot;
		matHueRot = Matrix_MakeRotation(fHue, 0.0f, 0.0f, -1.0f);
		vec3d vHueRot = Matrix_MultiplyVector(matHueRot, vHueStart);
		vec3d vHueRotRad = Vector_Mul(vHueRot, radius);
		DrawLine(cntrx, cntry, cntrx + vHueRotRad.x, cntry + vHueRotRad.y, 0);
		float fSat = float(sat) / 255;
		vec3d vHueRotSat = Vector_Mul(vHueRotRad, fSat);
		Fill(cntrx + vHueRotSat.x - 8, cntry + vHueRotSat.y - 8, cntrx + vHueRotSat.x + 8, cntry + vHueRotSat.y + 8, 0);
		Fill(cntrx + vHueRotSat.x - 4, cntry + vHueRotSat.y - 4, cntrx + vHueRotSat.x + 4, cntry + vHueRotSat.y + 4, 255, hue, sat);
	}

	void DrawListItem(uchar hue, uchar sat, string txt_elem, float posElem, int chosen_elem, int i, int top_elem = 0) // RYSOWANIE POZYCJI LISTY
	{
		string num_elem = "  ";
		int tens = i / 10;
		int sngl = i - tens * 10;
		if (tens > 0) {
			num_elem[0] = tens + 48;
			num_elem[1] = sngl + 48;
		}
		else {
			num_elem[0] = sngl + 48;
		}
		Fill(wPreviewWidth + 30, posElem, wPreviewWidth + 30 + 20, posElem + 20, 240, hue, sat);
		float pos2 = Text_Write(txt_elem, wPreviewWidth + 80, posElem, 2, 1);
		float pos3 = Text_Write(num_elem, pos2 + 5, posElem, 2, 1);
		if (i == chosen_elem + 1) {
			Text_Write("  %", pos3, posElem, 2, 1);
		}
	}

	void DrawElemList(vector<element>& elements, string txt_elem, int chosen_elem, int& top_elem) // RYSOWANIE LISTY ELEMENT”W SCENY
	{
		if (top_elem < chosen_elem - 6) {
			top_elem = chosen_elem - 6;
		}
		else if (chosen_elem < top_elem) {
			top_elem = chosen_elem;
		}
		float posElem = wNavigateHeight + 120;
		Fill(wPreviewWidth + 1, posElem, wPreviewWidth + wContentWidth, wNavigateHeight + wContentHeight - 90, colWin.val);
		int size = elements.size();
		for (int i = 1; i <= min(7, size); i++) {
			int iact = i + top_elem;
			int imin1 = iact - 1;
			uchar hue = elements[imin1].col.hue;
			uchar sat = elements[imin1].col.sat;
			DrawListItem(hue, sat, txt_elem, posElem, chosen_elem, iact);
			posElem = posElem + 30;
		}
	}

	void DrawSceneList(vector<scene>& scenes, string txt_elem, int chosen_elem, int& top_elem) // RYSOWANIE LISTY SCEN
	{
		if (top_elem < chosen_elem - 6) {
			top_elem = chosen_elem - 6;
		}
		else if (chosen_elem < top_elem) {
			top_elem = chosen_elem;
		}
		float posElem = wNavigateHeight + 120;
		Fill(wPreviewWidth + 1, posElem, wPreviewWidth + wContentWidth, wNavigateHeight + wContentHeight - 90, colWin.val);
		int size = scenes.size();
		for (int i = 1; i <= min(7, size); i++) {
			int iact = i + top_elem;
			int imin1 = iact - 1;
			uchar hue = scenes[imin1].col.hue;
			uchar sat = scenes[imin1].col.sat;
			DrawListItem(hue, sat, txt_elem, posElem, chosen_elem, iact);
			posElem = posElem + 30;
		}
	}

	void DrawTimeIndicator(float fTime, float fTimeFin, bool bHandle = false, uchar hue = 0, uchar sat = 0, uchar val = 255) // RYSOWANIE WSKAZNIKA NA OSI CZASU
	{
		float posY = wNavigateHeight + wPreviewHeight + 40;
		float posX = 40;
		if (fTimeFin > 0) {
			posX = 40 + (fTime / fTimeFin) * (wPreviewWidth - 80);
		}
		DrawLine(posX, posY - 15, posX, posY + 209, val, hue, sat);
		if (bHandle == true) {
			Fill(posX - 4, posY - 15, posX + 4, posY - 5, val, hue, sat);
		}
	}

	void DrawTimelineHighlight(float chsn_elem, float Ttop_elem) // RYSOWANIE RAMKI WSKAZUJ•CEJ WYBRAN• POZTCJ  NA OSI CZASU
	{
		color colHilit = { 148,168,232 };
		float brdrSize = 2;
		Fill(40 - brdrSize, wNavigateHeight + wPreviewHeight + 50 + (chsn_elem - Ttop_elem) * 20 - brdrSize, wPreviewWidth - 40 + brdrSize, wNavigateHeight + wPreviewHeight + 50 + (chsn_elem - Ttop_elem) * 20, colHilit.val, colHilit.hue, colHilit.sat);//GORNA
		Fill(40 - brdrSize, wNavigateHeight + wPreviewHeight + 69 + (chsn_elem - Ttop_elem) * 20, wPreviewWidth - 40 + brdrSize, wNavigateHeight + wPreviewHeight + 69 + (chsn_elem - Ttop_elem) * 20 + brdrSize, colHilit.val, colHilit.hue, colHilit.sat);//DOLNA
		Fill(40 - brdrSize, wNavigateHeight + wPreviewHeight + 50 + (chsn_elem - Ttop_elem) * 20 - brdrSize, 40, wNavigateHeight + wPreviewHeight + 69 + (chsn_elem - Ttop_elem) * 20 + brdrSize, colHilit.val, colHilit.hue, colHilit.sat);//LEWA
		Fill(wPreviewWidth - 40, wNavigateHeight + wPreviewHeight + 50 + (chsn_elem - Ttop_elem) * 20 - brdrSize, wPreviewWidth - 40 + brdrSize, wNavigateHeight + wPreviewHeight + 69 + (chsn_elem - Ttop_elem) * 20 + brdrSize, colHilit.val, colHilit.hue, colHilit.sat);//PRAWA
		DrawLine(40, wNavigateHeight + wPreviewHeight + 69 + (chsn_elem - Ttop_elem) * 20, wPreviewWidth - 40, wNavigateHeight + wPreviewHeight + 69 + (chsn_elem - Ttop_elem) * 20);
		DrawLine(40, wNavigateHeight + wPreviewHeight + 50 + (chsn_elem - Ttop_elem) * 20, wPreviewWidth - 40, wNavigateHeight + wPreviewHeight + 50 + (chsn_elem - Ttop_elem) * 20);
		DrawLine(40, wNavigateHeight + wPreviewHeight + 50 + (chsn_elem - Ttop_elem) * 20, 40, wNavigateHeight + wPreviewHeight + 69 + (chsn_elem - Ttop_elem) * 20);
		DrawLine(wPreviewWidth - 40, wNavigateHeight + wPreviewHeight + 50 + (chsn_elem - Ttop_elem) * 20, wPreviewWidth - 40, wNavigateHeight + wPreviewHeight + 69 + (chsn_elem - Ttop_elem) * 20);
	}

	void DrawSequenceTimeline(vector<scene>& scenes, int chsn_scn, int& Ttop_scn) // RYSOWANIE OSI CZASU SEKWENCJI SCEN
	{
		for (int i = 0; i < 10; i++) {
			DrawLine(1, wNavigateHeight + wPreviewHeight + 50 + i * 20, wPreviewWidth - 1, wNavigateHeight + wPreviewHeight + 50 + i * 20, 20);
		}
		color colHilit = { 148,168,232 };
		color colKeyfr = { 100,255,255 };
		if (Ttop_scn < chsn_scn - 9) {
			Ttop_scn = chsn_scn - 9;
		}
		else if (chsn_scn < Ttop_scn) {
			Ttop_scn = chsn_scn;
		}
		float height = 9;
		float finTime = 0;
		for (auto sceneTmp : scenes) {
			finTime = finTime + (sceneTmp.endTime - sceneTmp.startTime);
		}
		float posY = wNavigateHeight + wPreviewHeight + 60 - 20 * Ttop_scn;
		float sceneLengthSoFar = 0;
		for (auto sceneTmp : scenes) {
			float sceneLength = sceneTmp.endTime - sceneTmp.startTime;
			if ((posY > wNavigateHeight + wPreviewHeight + 40) && (posY < wNavigateHeight + wPreviewHeight + wTimelineHeight)) {
				float poxXstart = 40 + (sceneLengthSoFar / finTime) * (wPreviewWidth - 80);
				float poxXend = 40 + ((sceneLengthSoFar + sceneLength) / finTime) * (wPreviewWidth - 80);
				Fill(poxXstart, posY - height, poxXend, posY + height, 240, sceneTmp.col.hue, sceneTmp.col.sat);

				float lastKeyfTime = 0;
				lastKeyfTime = Element_FindLastKeyframe(sceneTmp.cameras, lastKeyfTime);
				lastKeyfTime = Element_FindLastKeyframe(sceneTmp.objects, lastKeyfTime);
				lastKeyfTime = Element_FindLastKeyframe(sceneTmp.lights, lastKeyfTime);

				if (sceneTmp.endTime < lastKeyfTime) {
					float overTime = lastKeyfTime - sceneTmp.endTime;
					float poxXoverend = 40 + ((sceneLengthSoFar + sceneLength + overTime) / finTime) * (wPreviewWidth - 80);
					Fill(poxXend, posY - height, min(wTimelineWidth - 1, poxXoverend), posY + height, 160, sceneTmp.col.hue, sceneTmp.col.sat);
				}
				else {
					float posXlastKeyframe = 40 + ((sceneLengthSoFar + lastKeyfTime - sceneTmp.startTime) / finTime) * (wPreviewWidth - 80);
					DrawLine(posXlastKeyframe, posY - height, posXlastKeyframe, posY + height, colKeyfr.val, colKeyfr.hue, colKeyfr.sat);
				}

				if (sceneTmp.startTime > 0) {
					float preTime = sceneTmp.startTime;
					float poxXprestart = 40 + ((sceneLengthSoFar - preTime) / finTime) * (wPreviewWidth - 80);
					Fill(poxXprestart, posY - height, poxXstart, posY + height, 160, sceneTmp.col.hue, sceneTmp.col.sat);
				}
				else {
					float posXzeroKeyframe = 40 + ((sceneLengthSoFar - sceneTmp.startTime) / finTime) * (wPreviewWidth - 80);
					DrawLine(posXzeroKeyframe, posY - height, posXzeroKeyframe, posY + height, colKeyfr.val, colKeyfr.hue, colKeyfr.sat);
				}
			}
			posY = posY + 20;
			sceneLengthSoFar = sceneLengthSoFar + sceneLength;
		}
		//podswietlenie wybranej sceny
		DrawTimelineHighlight(chsn_scn, Ttop_scn);
	}

	void DrawSceneTimeline(vector<element>& elements, float finTime, int chsn_elem, int& Ttop_elem, float strtTime, float endTime, color scnCol) // RYSOWANIE OSI CZASU WYBRANEJ SCENY
	{
		for (int i = 0; i < 10; i++) {
			DrawLine(1, wNavigateHeight + wPreviewHeight + 50 + i * 20, wPreviewWidth - 1, wNavigateHeight + wPreviewHeight + 50 + i * 20, 20);
		}
		color colKeyfr = { 100,255,255 };
		if (Ttop_elem < chsn_elem - 9) {
			Ttop_elem = chsn_elem - 9;
		}
		else if (chsn_elem < Ttop_elem) {
			Ttop_elem = chsn_elem;
		}

		float height = 9;
		float posY = wNavigateHeight + wPreviewHeight + 60 - 20 * Ttop_elem;
		for (auto elemTmp : elements) {
			float poxXend = 40 + (elemTmp.keyframes[elemTmp.keyframes.size() - 1].time / finTime) * (wPreviewWidth - 80);
			if ((posY > wNavigateHeight + wPreviewHeight + 40) && (posY < wNavigateHeight + wPreviewHeight + wTimelineHeight)) {
				Fill(40, posY - height, poxXend, posY + height, 240, elemTmp.col.hue, elemTmp.col.sat);
				if (poxXend < wPreviewWidth - 40) {
					Fill(poxXend, posY - height, wPreviewWidth - 40, posY + height, 160, elemTmp.col.hue, elemTmp.col.sat);
				}
			}
			posY = posY + 20;
		}
		int count = 0;
		posY = wNavigateHeight + wPreviewHeight + 60 - 20 * Ttop_elem;
		for (auto elemTmp : elements) {
			float poxXend = 40 + (elemTmp.keyframes[elemTmp.keyframes.size() - 1].time / finTime) * (wPreviewWidth - 80);
			if ((posY > wNavigateHeight + wPreviewHeight + 40) && (posY < wNavigateHeight + wPreviewHeight + wTimelineHeight)) {
				if (count == chosen_scn) {
					//podswietlenie wybranej sceny
					DrawTimelineHighlight(chsn_elem, Ttop_elem);
				}
				for (int i = 0; i < elemTmp.keyframes.size(); i++) {
					float posXkey = 40 + (elemTmp.keyframes[i].time / finTime) * (wPreviewWidth - 80);
					DrawLine(posXkey, posY - height, posXkey, posY + height, colKeyfr.val, colKeyfr.hue, colKeyfr.sat);
					Fill(posXkey - 5, posY - 5, posXkey + 5, posY + 5);
					Fill(posXkey - 4, posY - 4, posXkey + 4, posY + 4, colKeyfr.val, colKeyfr.hue, colKeyfr.sat);
					if (elemTmp.keyframes[i].nextCam != 0 && i != 0) {
						string txtCut = "# ";
						txtCut[1] = elemTmp.keyframes[i].nextCam + 48;
						Fill(posXkey + 5, posY - 5, posXkey + 15, posY + 5);
						Text_Write(txtCut, posXkey + 5, posY - 6, 1, true, colKeyfr.val, colKeyfr.hue, colKeyfr.sat);
					}
				}
			}
			posY = posY + 20;
			count++;
		}

		float scnStart = max(40 + (strtTime / finTime) * (wPreviewWidth - 80), windows[3].posx);
		float scnEnd = min(40 + (endTime / finTime) * (wPreviewWidth - 80), windows[3].posx + windows[3].sizex);
		Fill(scnStart, windows[3].posy + 40 - 4, scnEnd, windows[3].posy + 40 + 2, 240, scnCol.hue, scnCol.sat);
	}

	void CreateGUI() // INICJALIZACJA GRAFICZNEGO INTERFEJSU UØYTKOWNIKA
	{
		float bBorder = 4;

		float bWidthS = 84;
		float bWidthM = 100;
		float bWidth = 140;
		float bHeightS = 0.056 * screenHeight;
		float bHeight = 70;

		float margin = 10;
		float bMargin = wPreviewWidth / 18;
		float bDist = (wElementWidth - 4 * bWidth - 4 * bMargin) / 5;
		float distHead = 0.07 * screenHeight / 2;

		float bFuncDistY = wKeyframeHeight / 3;
		float bFuncDistY2 = bFuncDistY + bHeight + 2 * bMargin / 3;
		float bFuncDistY3 = bFuncDistY + 2 * bHeight + 4 * bMargin / 3;

		float bCol1x = bMargin + bDist;
		float bCol2x = bMargin + 2 * bDist + bWidth;
		float bCol3x = 3 * bMargin + 3 * bDist + 2 * bWidth;
		float bCol4x = 3 * bMargin + 4 * bDist + 3 * bWidth;

		float bRow1y = bMargin;
		float bRow2y = bRow1y + bHeight + 2 * bMargin / 3;

		float arCol1x = wKeyframeWidth / 2 - bWidthS / 2 - bDist - bWidthS;
		float arCol2x = wKeyframeWidth / 2 - bWidthS / 2;
		float arCol3x = wKeyframeWidth / 2 + bWidthS / 2 + bDist;

		float arRow1y = 120;
		float arRow2y = arRow1y + bHeightS + 10;
		float arRow3y = arRow2y + bHeightS + 30;
		float arRow4y = arRow3y + bHeightS + 10;
		float arRow5y = arRow4y + bHeightS + 30;

		float arRow11y = arRow3y + 50;
		float arRow12y = arRow11y + bHeightS + 10;

		windows[0] = { 0,0,wNavigateWidth,wNavigateHeight,colWin };
		windows[0].NewButton(margin, margin, bWidthM, bHeightS, bBorder, "OPCJE", "[L CTRL]");
		windows[0].NewButton(bWidthM + 2 * margin, margin, bWidth, bHeightS, bBorder, "WOLNA KAMERA", "(TAB)");
		windows[0].NewButton(margin + bWidthM + bWidth + 2 * margin, margin, 2 * bWidthM + bWidthM / 5, bHeightS, bBorder, "EDYCJA WOLNEJ KAMERY", "[V]");
		windows[0].NewButton(wNavigateWidth - bWidthM - 2 * bWidth - 3 * margin, margin, bWidth, bHeightS, bBorder, "PLAYBACK", "(SPACJA)", "", "|");
		windows[0].NewButton(wNavigateWidth - bWidthM - bWidth - 2 * margin, margin, bWidth, bHeightS, bBorder, "RESTART", "(BACKSPACE)", "", "<");
		windows[0].NewButton(wNavigateWidth - bWidthM - margin, margin, bWidthM, bHeightS, bBorder, "WYJSCIE", "(ESC)");
		//windows[0].NewButton(bWidthM + 2 * margin, 2 * margin, bWidth, bHeightS, 0, "(wcisnij)", "", "[trzymaj]");

		windows[1] = { 0,wNavigateHeight,bWidthM + 2 * margin,2 * bHeightS + 3 * margin,colWin };
		windows[1].NewButton(margin, margin, bWidthM, bHeightS, bBorder, "ZAPISZ", "(Z)");
		windows[1].NewButton(margin, margin + bHeightS + margin, bWidthM, bHeightS, bBorder, "EKSPORT", "(E)");
		//windows[1].NewButton(margin, margin + 2 * bHeightS + 2 * margin, bWidthM, bHeightS, bBorder, "TRYB", "[M]");

		windows[2] = { 0,wNavigateHeight,wPreviewWidth,wPreviewHeight,colWin4 };

		windows[3] = { 0,wNavigateHeight + wPreviewHeight,wPreviewWidth,wTimelineHeight,colWin3 };

		windows[4] = { wPreviewWidth,wNavigateHeight,wContentWidth,wContentHeight,colWin };
		windows[4].line = 1;
		windows[4].NewButton(wContentWidth / 2, distHead, bWidth, bHeightS, 1, "SCENY");
		windows[4].NewButton(wContentWidth / 2 - bWidth / 2, wContentHeight - bHeight - bHeight / 4, bWidth, bHeight, bBorder, "ZMIEN KOLOR", "[C]", "SCENY");

		windows[5] = { wPreviewWidth,wNavigateHeight,wContentWidth,wContentHeight,colWin };
		windows[5].line = 1;
		windows[5].NewButton(wContentWidth / 2 - 2 * bWidthS, distHead, bWidthS, bHeightS, 1, "KAMERY", "(P)");
		windows[5].NewButton(wContentWidth / 2 - bWidthS, distHead, bWidthS, bHeightS, 1, "OBIEKTY", "(O)");
		windows[5].NewButton(wContentWidth / 2, distHead, bWidth, bHeightS, 1, "ZRODLA SWIATLA", "(L)");
		windows[5].NewButton(wContentWidth / 2 - bWidth / 2, wContentHeight - bHeight - bHeight / 4, bWidth, bHeight, bBorder, "ZMIEN KOLOR", "[C]", "ELEMENTU");

		windows[6] = { 0,wNavigateHeight + wPreviewHeight + wTimelineHeight,wPreviewWidth,wElementHeight,colWin2 };
		windows[6].NewButton(bCol1x, bRow1y, bWidth, bHeight, bBorder, "DODAJ", "(L SHIFT)", "SCENE");
		windows[6].NewButton(bCol2x, bRow1y, bWidth, bHeight, bBorder, "USUN", "(DEL)", "SCENE");
		windows[6].NewButton(bCol3x, bRow1y, bWidth, bHeight, bBorder, "POPRZEDNIA", "(@)", "SCENA");
		windows[6].NewButton(bCol4x, bRow1y, bWidth, bHeight, bBorder, "NASTEPNA", "($)", "SCENA");
		windows[6].NewButton(bCol1x, bRow2y, bWidth, bHeight, bBorder, "PRZESUN", "[ALT]", "SCENE");
		windows[6].NewButton(bCol2x, bRow2y, bWidth, bHeight, bBorder, "WCZESNIEJ", "(@)");
		windows[6].NewButton(bCol2x + bDist + bWidth, bRow2y, bWidth, bHeight, bBorder, "DALEJ", "($)");
		windows[6].NewButton(bCol4x, bRow2y, bWidth, bHeight, bBorder, "EDYTUJ", "(ENTER)", "SCENE");

		windows[7] = { 0,wNavigateHeight + wPreviewHeight + wTimelineHeight,wPreviewWidth,wElementHeight,colWin2 };
		windows[7].NewButton(bCol1x, bRow1y, bWidth, bHeight, bBorder, "DODAJ", "[L SHIFT]", "ELEMENT");
		windows[7].NewButton(bCol2x, bRow1y, bWidth, bHeight, bBorder, "USUN", "(DEL)", "ELEMENT");
		windows[7].NewButton(bCol3x, bRow1y, bWidth, bHeight, bBorder, "POPRZEDNI", "(@)", "ELEMENT");
		windows[7].NewButton(bCol4x, bRow1y, bWidth, bHeight, bBorder, "NASTEPNY", "($)", "ELEMENT");
		windows[7].NewButton(bCol1x, bRow2y, bWidth, bHeight, bBorder, "DODAJ KLATKE", "(R SHIFT)", "KLUCZOWA");
		windows[7].NewButton(bCol2x, bRow2y, bWidth, bHeight, bBorder, "USUN KLATKE", "(R CTRL)", "KLUCZOWA");
		windows[7].NewButton(bCol3x, bRow2y, bWidth, bHeight, bBorder, "POPRZEDNIA KL.", "(%)", "KLUCZOWA");
		windows[7].NewButton(bCol4x, bRow2y, bWidth, bHeight, bBorder, "NASTEPNA KL.", "(#)", "KLUCZOWA");

		windows[8] = { wPreviewWidth,wNavigateHeight + wContentHeight,wKeyframeWidth,wKeyframeHeight,colWin };
		windows[8].NewButton(arCol2x, arRow11y, bWidthS, bHeightS, bBorder, "SAT +", "[@]");
		windows[8].NewButton(arCol1x, arRow12y, bWidthS, bHeightS, bBorder, "HUE -", "[%]");
		windows[8].NewButton(arCol2x, arRow12y, bWidthS, bHeightS, bBorder, "SAT -", "[$]");
		windows[8].NewButton(arCol3x, arRow12y, bWidthS, bHeightS, bBorder, "HUE +", "[#]");

		windows[9] = { wPreviewWidth,wNavigateHeight + wContentHeight,wKeyframeWidth,wKeyframeHeight,colWin };
		windows[9].line = 1;
		windows[9].NewButton(wKeyframeWidth / 2 - bWidthS, distHead, bWidthS, bHeightS, 1, "PRZYTNIJ", "[ALT]");
		windows[9].NewButton(wKeyframeWidth / 2, distHead, bWidth, bHeightS, 1, "NAWIGACJA");
		windows[9].NewButton(wKeyframeWidth / 2 - bWidth / 2, bFuncDistY, bWidth, bHeight, bBorder, "PRZEJDZ DO", "(%)", "POCZATKU SCENY");
		windows[9].NewButton(wKeyframeWidth / 2 - bWidth / 2, bFuncDistY2, bWidth, bHeight, bBorder, "PRZEJDZ DO", "(#)", "KONCA SCENY");

		windows[10] = { wPreviewWidth,wNavigateHeight + wContentHeight,wKeyframeWidth,wKeyframeHeight,colWin };
		windows[10].line = 1;
		windows[10].NewButton(wKeyframeWidth / 2 - bWidthS, distHead, bWidthS, bHeightS, 1, "PRZYTNIJ");
		windows[10].NewButton(wKeyframeWidth / 2, distHead, bWidth, bHeightS, 1, "NAWIGACJA");
		windows[10].NewButton(wKeyframeWidth / 2 - bWidth / 2, bFuncDistY, bWidth, bHeight, bBorder, "PRZYTNIJ", "[%]", "W TYL");
		windows[10].NewButton(wKeyframeWidth / 2 - bWidth / 2, bFuncDistY2, bWidth, bHeight, bBorder, "PRZYTNIJ", "[#]", "WPRZOD");

		windows[11] = { wPreviewWidth,wNavigateHeight + wContentHeight,wKeyframeWidth,wKeyframeHeight,colWin };

		windows[12] = { wPreviewWidth,wNavigateHeight + wContentHeight,wKeyframeWidth,wKeyframeHeight,colWin };
		windows[12].line = 1;
		windows[12].NewButton(wKeyframeWidth / 2 - bWidthS, distHead, bWidthS, bHeightS, 1, "CZAS", "[ALT]");
		windows[12].NewButton(wKeyframeWidth / 2, distHead, bWidth, bHeightS, 1, "TRANSFORMACJA");
		windows[12].NewButton(arCol1x, arRow1y, bWidthS, bHeightS, bBorder, "$", "[Q]");
		windows[12].NewButton(arCol2x, arRow1y, bWidthS, bHeightS, bBorder, "@", "[W]");
		windows[12].NewButton(arCol3x, arRow1y, bWidthS, bHeightS, bBorder, "@", "[E]");
		windows[12].NewButton(arCol1x, arRow2y, bWidthS, bHeightS, bBorder, "%", "[A]");
		windows[12].NewButton(arCol2x, arRow2y, bWidthS, bHeightS, bBorder, "$", "[S]");
		windows[12].NewButton(arCol3x, arRow2y, bWidthS, bHeightS, bBorder, "#", "[D]");
		windows[12].NewButton(arCol1x, arRow3y, bWidthS, bHeightS, bBorder, "%", "[Y]");
		windows[12].NewButton(arCol2x, arRow3y, bWidthS, bHeightS, bBorder, "@", "[U]");
		windows[12].NewButton(arCol3x, arRow3y, bWidthS, bHeightS, bBorder, "#", "[I]");
		windows[12].NewButton(arCol1x, arRow4y, bWidthS, bHeightS, bBorder, "^", "[H]");
		windows[12].NewButton(arCol2x, arRow4y, bWidthS, bHeightS, bBorder, "$", "[J]");
		windows[12].NewButton(arCol3x, arRow4y, bWidthS, bHeightS, bBorder, "&", "[K]");
		windows[12].NewButton(arCol1x, arRow5y, bWidthS, bHeightS, bBorder, "MINUS", "[-]");
		windows[12].NewButton(arCol2x, arRow5y, bWidthS, bHeightS, bBorder, "PLUS", "[+]");

		windows[13] = { wPreviewWidth,wNavigateHeight + wContentHeight,wKeyframeWidth,wKeyframeHeight,colWin };
		windows[13].line = 1;
		windows[13].NewButton(wKeyframeWidth / 2 - bWidthS, distHead, bWidthS, bHeightS, 1, "CZAS");
		windows[13].NewButton(wKeyframeWidth / 2, distHead, bWidth, bHeightS, 1, "TRANSFORMACJA");
		windows[13].NewButton(wKeyframeWidth / 2 - bWidth / 2, bFuncDistY, bWidth, bHeight, bBorder, "PRZESUN", "[%]", "W TYL");
		windows[13].NewButton(wKeyframeWidth / 2 - bWidth / 2, bFuncDistY2, bWidth, bHeight, bBorder, "PRZESUN", "[#]", "WPRZOD");
		windows[13].NewButton(wKeyframeWidth / 2 - bWidth / 2, bFuncDistY3, bWidth, bHeight, bBorder, "CIECIE DO", "[PODAJ NUMER]", "INNEJ KAMERY");

		windows[14] = { wPreviewWidth,wNavigateHeight + wContentHeight,wKeyframeWidth,wKeyframeHeight,colWin };
		//windows[14].NewButton(wKeyframeWidth / 2, 380, bWidth, bHeight, bBorder, "PODAJ NAZWE", "[ENTER]", "SIATKI OBIEKTU");
	}

	//----------------------------------------------------------------------------------------------//
	//--------------------------// INICJALIZACJA I AKTUALIZACJA APLIKACJI //------------------------//
	//----------------------------------------------------------------------------------------------//

	bool LoadApp(float frameAspectRatio) override // ZA£ADOWANIE WEJåCIOWYCH DANYCH APLIKACJI
	{
		font = LoadFont("font.txt");

		ObjNamesLoaded = Mesh_Import("objects.txt", meshes);

		ifstream data("data.txt"); // za≥adowanie pliku z wejúciowymi danymi sekwencji
		if (!data.is_open()) {
			scene sceneTmp;
			sceneTmp.freeCam.vPosition = { 0,20,-38 };
			sceneTmp.freeCam.qRotation = { 0.17,0,0,0.98 };
			scenes.push_back(sceneTmp);
		}
		else {
			int cnt_scn = -1;
			int cnt_cam = -1;
			int cnt_obj = -1;
			int cnt_lgt = -1;

			while (!data.eof()) {
				char dataLine[128];
				data.getline(dataLine, 128);
				strstream dataNum;
				dataNum << dataLine;
				char obj_type;
				dataNum >> obj_type;

				if (obj_type == 'f') {
					break;
				}
				else {
					switch (obj_type) {
					case 's':
					{
						int iSceneColHue, iSceneColSat;
						scene sceneTmp;
						dataNum >> sceneTmp.startTime >> sceneTmp.endTime >> sceneTmp.startCam >> iSceneColHue >> iSceneColSat
							>> sceneTmp.freeCam.vPosition.x >> sceneTmp.freeCam.vPosition.y >> sceneTmp.freeCam.vPosition.z
							>> sceneTmp.freeCam.qRotation.x >> sceneTmp.freeCam.qRotation.y >> sceneTmp.freeCam.qRotation.z >> sceneTmp.freeCam.qRotation.w;
						sceneTmp.col.hue = uchar(iSceneColHue);
						sceneTmp.col.sat = uchar(iSceneColSat);
						scenes.push_back(sceneTmp);
						cnt_obj = -1;
						cnt_lgt = -1;
						cnt_cam = -1;
						cnt_scn++;
						if (cnt_scn == 0) {
							fSceneTime = sceneTmp.startTime;
						}
						break;
					}
					case 'v':
						cnt_cam = Keyframe_Load(scenes[cnt_scn].cameras, dataNum, cnt_cam);
						break;
					case 'o':
						cnt_obj = Keyframe_Load(scenes[cnt_scn].objects, dataNum, cnt_obj);
						break;
					case 'l':
						cnt_lgt = Keyframe_Load(scenes[cnt_scn].lights, dataNum, cnt_lgt);
						break;
					}
				}
			}
		}

		CreateGUI();
		DrawWindow(windows[0]);
		DrawWindow(windows[2]);
		DrawWindow(windows[3]);
		DrawWindow(windows[4]);
		DrawWindow(windows[7]);
		DrawWindow(windows[9]);
		DrawSceneList(scenes, "SCENA", chosen_scn, top_scn); // narysowanie interfejsu graficznego

		if (frameHeight <= wPreviewHeight) {
			kFrame = 1;
		}
		else {
			if (wPreviewAR >= frameAspectRatio) {
				kFrame = wPreviewHeight / frameHeight;
			}
			else {
				kFrame = wPreviewWidth / frameWidth;
			}
		}
		oyFrame = wNavigateHeight + wPreviewHeight / 2 - kFrame * frameHeight / 2;
		oxFrame = wPreviewWidth / 2 - kFrame * frameWidth / 2; // wspÛ≥cznniki skali podglπdu obrazu

		matProj = Matrix_MakeProjection(90.0f, frameAspectRatio, 0.1f, 1000.0f); // utworzenie macierzy projekcji
		return true;
	}

	void EditElement(vector<element>& elements, int& chosen_elm, float fElapsedTime, int& top_elm) // edycja elementÛw sceny wybranego typu przez uøytkownika
	{
		windows[12].bStates[0] = 2;
		windows[12].bStates[1] = 5;

		bool bFreeCamTrans = 0;
		windows[0].bStates[1] = 2;
		if (freeCamMode != 0) {
			windows[0].bStates[1] = 3;
			windows[0].bStates[2] = 2;
			//tryb wolnej kamery, moøliwoúÊ edycji
			if (GetKey(L'V').bHeld) {
				bFreeCamTrans = 1;
				windows[0].bStates[2] = 3;
				windows[12].bStates[0] = 1;
				windows[12].bStates[14] = 0;
				windows[12].bStates[15] = 0;
				indWinKeyframe = 12;
				Keyframe_Translate(scenes[chosen_scn].freeCam.vPosition, scenes[chosen_scn].freeCam.qRotation, fElapsedTime);
				Keyframe_Rotate(scenes[chosen_scn].freeCam.qRotation, fElapsedTime);
			}
		}

		if (scenes[chosen_scn].cameras.size() > 0) {
			//moøliwoúÊ wyjúcia z freeCamMode
			if (GetKey(VK_TAB).bPressed)
			{
				if (freeCamMode == 0) {
					freeCamMode = 1;
				}
				else {
					iOutFreeCam = 1;
					freeCamMode = 0;
				}
			}
		}

		if (elements.size() > 0 && bFreeCamTrans == 0) {
			windows[7].bStates[1] = 2;
			windows[7].bStates[2] = 1;
			windows[7].bStates[3] = 1;
			windows[7].bStates[4] = 2;
			windows[7].bStates[6] = 1;
			windows[7].bStates[7] = 1;
			windows[5].bStates[3] = 2;
			//jest element, moøliwoúÊ edycji
			if (GetKey(L'C').bHeld) {
				windows[5].bStates[3] = 3;
				indWinKeyframe = 8;
				//zmiana koloru elementu
				Element_Color(elements[chosen_elm].col, fElapsedTime);
			}
			else {
				int chosen_kfr = Keyframe_Find(elements[chosen_elm].keyframes, fSceneTime);
				if (GetKey(VK_RSHIFT).bPressed)
				{
					bDiffKeyframe = 1;
					windows[7].bStates[4] = 3;
					//dodanie klatki kluczowej do elementu
					if (elements.size() > 0) {
						chosen_kfr = Keyframe_Add(elements[chosen_elm].keyframes, elements[chosen_elm].actPos, chosen_kfr, fElapsedTime);
					}
				}

				bool arrowsDefault = 1;
				//znalazienie klatki kluczowej w ktÛrej jest aktualny czas (jeúli jest)
				if (fSceneTime == elements[chosen_elm].keyframes[chosen_kfr].time) {
					indWinKeyframe = 12;
					windows[12].bStates[8] = 0;
					windows[12].bStates[9] = 0;
					windows[12].bStates[10] = 0;
					windows[12].bStates[11] = 0;
					windows[12].bStates[12] = 0;
					windows[12].bStates[13] = 0;

					windows[12].bStates[14] = 0;
					windows[12].bStates[15] = 0;

					//moøliwoúÊ edycji klatki kluczowej elementu

					if (freeCamMode == 0 && ElementType == 1) {
						//przesuniÍcie we wspÛ≥rzÍdnych lokalnych
						Keyframe_Translate(elements[chosen_elm].keyframes[chosen_kfr].vPosition, elements[chosen_elm].keyframes[chosen_kfr].qRotation, fElapsedTime);
					}
					else {
						//przesuniÍcie we wspÛ≥rzÍdnych globalnych
						vec3d qQuatZero = { 0,0,0 };
						Keyframe_Translate(elements[chosen_elm].keyframes[chosen_kfr].vPosition, qQuatZero, fElapsedTime);
					}

					if (ElementType != 3) {
						//rotacja elementu w klatce kluczowej
						Keyframe_Rotate(elements[chosen_elm].keyframes[chosen_kfr].qRotation, fElapsedTime);
					}

					if (ElementType != 1) {
						windows[12].bStates[14] = 1;
						windows[12].bStates[15] = 2;
						//skalowanie elementu
						if (GetKey(VK_OEM_PLUS).bHeld) {
							windows[12].bStates[15] = 3;
							//zwiÍkszenie skali elementu
							elements[chosen_elm].keyframes[chosen_kfr].fScale += 0.3f * fElapsedTime;
						}

						if (elements[chosen_elm].keyframes[chosen_kfr].fScale > 0) {
							windows[12].bStates[14] = 2;
							//moøliwoúÊ zmniejszenia skali elementu
							if (GetKey(VK_OEM_MINUS).bHeld) {
								windows[12].bStates[14] = 3;
								//zmniejszenie skali elementu
								elements[chosen_elm].keyframes[chosen_kfr].fScale -= 0.3f * fElapsedTime;
								if (elements[chosen_elm].keyframes[chosen_kfr].fScale < 0) {
									elements[chosen_elm].keyframes[chosen_kfr].fScale = 0;
								}
							}
						}
					}

					if (GetKey(VK_MENU).bHeld) {
						//zmiana czasu klatki kluczowej, ciÍcie w kamerze
						indWinKeyframe = 13;
						windows[13].bStates[0] = 5;
						windows[13].bStates[1] = 4;
						windows[13].bStates[2] = 1;
						windows[13].bStates[3] = 1;
						windows[13].bStates[4] = 0;
						arrowsDefault = 0;
						if (fSceneTime != 0.0f) {
							//klatka kl. niezerowa, moøliwoúÊ edycji czasu
							int kfrNext = chosen_kfr + 1;
							int kfrPrev = chosen_kfr - 1;
							//przesuniÍcie klatki kluczowej elementu
							if (chosen_kfr == elements[chosen_elm].keyframes.size() - 1) {
								windows[13].bStates[3] = 2;
								//moøliwoúÊ przesuniÍcia kl. kl. wprzÛd
								if (GetKey(VK_RIGHT).bHeld) {
									windows[13].bStates[3] = 3;
									//przesuniÍcie kl.kl. wprzÛd
									fSceneTime = Keyframe_IncreaseTime(elements[chosen_elm].keyframes, fSceneTime, fElapsedTime, chosen_kfr);
								}
							}
							else if (fSceneTime < elements[chosen_elm].keyframes[kfrNext].time) {
								windows[13].bStates[3] = 2;
								//moøliwoúÊ przesuniÍcia kl. kl. wprzÛd
								if (GetKey(VK_RIGHT).bHeld) {
									windows[13].bStates[3] = 3;
									//przesuniÍcie kl.kl. wprzÛd
									fSceneTime = Keyframe_IncreaseTime(elements[chosen_elm].keyframes, fSceneTime, fElapsedTime, chosen_kfr);
								}
							}

							if (fSceneTime > elements[chosen_elm].keyframes[kfrPrev].time) {
								windows[13].bStates[2] = 2;
								//moøliwoúÊ przesuniÍcia kl. kl. w ty≥
								if (GetKey(VK_LEFT).bHeld) {
									windows[13].bStates[2] = 3;
									//przesuniÍcie kl.kl. w tyl
									fSceneTime = Keyframe_DecreaseTime(elements[chosen_elm].keyframes, fSceneTime, fElapsedTime, chosen_kfr);
								}
							}
						}

						if (ElementType == 1)
						{
							windows[13].bStates[4] = 1;
							if (elements.size() > 1) {
								windows[13].bStates[4] = 2;
								//moøliwoúÊ ustawienia ciÍcia do innej kamery w klatce kluczowej
								for (int i = 0; i < 10; i++) {
									if (GetKey(i + 48).bPressed && i <= elements.size()) {
										windows[13].bStates[4] = 2;
										if (fSceneTime == 0) {
											scenes[chosen_scn].startCam = i;
											bDiffItem = 1;
										}

										elements[chosen_elm].keyframes[chosen_kfr].nextCam = i;
										if (i != 0) {
											chosen_cam = i - 1;
											bDiffItem = 1;
										}

										chosen_kfr = Keyframe_Find(elements[chosen_elm].keyframes, fSceneTime);

										if (elements[chosen_elm].keyframes[chosen_kfr].time == fSceneTime) {
											elements[chosen_elm].keyframes[chosen_kfr].nextCam = 0;
										}
										else {
											orientedPoint camPointTmp;
											camPointTmp.vPosition = elements[chosen_elm].actPos.vPosition;
											camPointTmp.qRotation = elements[chosen_elm].actPos.qRotation;

											keyframe keyframeTmp;
											keyframeTmp.vPosition = camPointTmp.vPosition;
											keyframeTmp.qRotation = camPointTmp.qRotation;
											keyframeTmp.nextCam = 0;
											keyframeTmp.time = fSceneTime;

											vector<keyframe> vecKeyframeTmp;
											for (int par = chosen_kfr + 1; par < elements[chosen_elm].keyframes.size(); par++) {
												vecKeyframeTmp.push_back(elements[chosen_elm].keyframes[par]);
											}
											elements[chosen_elm].keyframes.erase(elements[chosen_elm].keyframes.begin() + chosen_kfr + 1, elements[chosen_elm].keyframes.end());
											elements[chosen_elm].keyframes.push_back(keyframeTmp);
											for (int par = 0; par < vecKeyframeTmp.size(); par++) {
												elements[chosen_elm].keyframes.push_back(vecKeyframeTmp[par]);
											}
											chosen_kfr++;
										}
										break;
									}
								}
							}
						}
					}

					if (fSceneTime != 0.0f) {
						windows[7].bStates[5] = 2;
						//moøliwoúÊ usuniÍcia niezerowej klatki kluczowej
						if (GetKey(VK_RCONTROL).bPressed) {
							bDiffKeyframe = 1;
							windows[7].bStates[5] = 3;
							//usuniÍcie klatki kluczowej elementu
							chosen_kfr = Keyframe_Del(elements[chosen_elm].keyframes, chosen_kfr);
						}
					}
				}

				if (elements.size() > 1 && arrowsDefault == 1) {
					windows[7].bStates[2] = 2;
					windows[7].bStates[3] = 2;
					//jest wiÍcej niø jeden element, moøliwoúÊ prze≥πczania
					if (GetKey(VK_DOWN).bPressed)
					{
						windows[7].bStates[3] = 3;
						bDiffItem = 1;
						//nastÍpny element
						chosen_elm = Element_Next(elements, chosen_elm);
					}
					else if (GetKey(VK_UP).bPressed)
					{
						windows[7].bStates[2] = 3;
						bDiffItem = 1;
						//poprzedni element
						chosen_elm = Element_Prev(elements, chosen_elm);
					}
				}

				if (fSceneTime > 0.0F && arrowsDefault == 1) {
					windows[7].bStates[6] = 2;
					//moøliwoúÊ wybrania poprzedniej klatki kluczowej
					if (GetKey(VK_LEFT).bPressed) {
						windows[7].bStates[6] = 3;
						//poprzednia klatka kluczowa
						fSceneTime = Keyframe_Prev(elements[chosen_elm].keyframes, fSceneTime, chosen_kfr);
					}
				}

				if (fSceneTime < elements[chosen_elm].keyframes[elements[chosen_elm].keyframes.size() - 1].time && arrowsDefault == 1) {
					windows[7].bStates[7] = 2;
					//moøliwoúÊ wybrania nastepnej klatki kluczowej
					if (GetKey(VK_RIGHT).bPressed) {
						windows[7].bStates[7] = 3;
						//nastÍpna klatka kluczowa
						fSceneTime = Keyframe_Next(elements[chosen_elm].keyframes, fSceneTime, chosen_kfr);
					}
				}

				if (GetKey(VK_DELETE).bPressed)
				{
					windows[7].bStates[1] = 3;
					bDiffItem = 1;
					if (top_elm > 0) {
						top_elm--;
					}
					//usuniÍcie wybranego elementu
					chosen_elm = Element_Del(elements, chosen_elm);
					if (ElementType == 1 && elements.size() == 0) {
						freeCamMode = 1;
					}
				}
			}
		}

		if (GetKey(VK_LSHIFT).bHeld)
		{
			windows[7].bStates[0] = 3;
			//windows[14].bStates[0] = 1;
			//dodanie nowego elementu
			if (ElementType == 2) {
				indWinKeyframe = 14;
				for (int i = 1; i < 10; i++) {
					if (GetKey(i + 48).bPressed && i < meshes.size() - 2) {
						chosen_elm = Element_Add(elements, i + 2);
						bDiffItem = 1;
						break;
					}
				}
			}
			else if (GetKey(VK_LSHIFT).bPressed) {
				bDiffItem = 1;
				if (ElementType == 1) {
					chosen_elm = Element_Add(elements, 1);
				}
				else {
					chosen_elm = Element_Add(elements, 0);
				}
			}
		}

	}

	bool UpdateApp(float fElapsedTime, int& toExport, int fps, float frameAspectRatio) override // AKTUALIZACJA STANU APLIKACJI
	{
		float prevTime = fSceneTime;
		bool bDiffEditMode = 0;
		bool bDiffTransformMode = 0;
		bool bOptionsBar = 0;
		uchar indPrevWinKeyframe = indWinKeyframe;

		bDiffItem = 0;
		bDiffKeyframe = 0;
		bDiffColor = 0;

		if (scenes[chosen_scn].cameras.size() == 0) {
			freeCamMode = 1;
		}

		if (toExport == 1)
		{
			fSceneTime = fSceneTime + 1 / float(fps); // zwiÍkszenie czasu o sta≥π wartoúÊ w trybie eksportu
			if (fSceneTime >= scenes[chosen_scn].endTime)
			{
				if (chosen_scn < scenes.size() - 1)
				{
					chosen_scn++;
					fSceneTime = scenes[chosen_scn].startTime;
					chosen_cam = scenes[chosen_scn].startCam - 1;
					if (scenes[chosen_scn].cameras.size() == 0) {
						freeCamMode = 1;
					}
					else {
						freeCamMode = 0;
					}
				}
				else {
					toExport = 2;
					playback = 0;
					fSceneTime = scenes[0].startTime;
					chosen_scn = 0;
					chosen_cam = scenes[0].startCam - 1;
				}
			}
		}
		else {

			//--------------// STEROWANIE APLIKACJ• PRZEZ UØYTKOWNIKA //-------------//

			windows[0].bStates[1] = 0;
			windows[0].bStates[2] = 0;
			windows[9].bStates[1] = 5;
			windows[4].bStates[0] = 5;
			windows[6].bStates[5] = 0;
			windows[6].bStates[6] = 0;
			if (playback == 1) {
				windows[0].bStates[3] = 3;
				if (GetKey(VK_SPACE).bPressed) {
					//zatrzymanie odtwarzania sceny
					windows[0].bStates[3] = 2;
					playback = 0;
				}
				else if (toExport == 0) {
					fSceneTime = fSceneTime + fElapsedTime; // zwiÍkszenie czasu o zmiennπ wartoúÊ
					if (EditMode == 0) {
						if (fSceneTime >= scenes[chosen_scn].endTime)
						{
							if (chosen_scn < scenes.size() - 1)
							{
								chosen_scn++;
								fSceneTime = scenes[chosen_scn].startTime;
								chosen_cam = scenes[chosen_scn].startCam - 1;
								if (scenes[chosen_scn].cameras.size() == 0) {
									freeCamMode = 1;
								}
								else {
									freeCamMode = 0;
								}
							}
							else {
								fSceneTime = scenes[chosen_scn].endTime;
								playback = 0;
							}
						}
					}
					else {
						float sumTimeTest = Scene_FindLastKeyframe(scenes[chosen_scn], fSceneTime);
						if (fSceneTime > sumTimeTest) {
							fSceneTime = sumTimeTest;
							playback = 0;
						}
					}
				}
			}
			else if (playback == 0) {
				if (GetKey(VK_SPACE).bPressed) {
					//uruchomienie odtwarzania sceny
					for (int w = 0; w < 15; w++) {
						for (int b = 0; b < windows[w].bStates.size(); b++) {
							windows[w].bStates[b] = 1;
						}
					}
					windows[0].bStates[3] = 3;
					playback = 1;

					windows[0].bStates[1] = 0;
					windows[0].bStates[2] = 0;
					windows[9].bStates[1] = 5;
					windows[4].bStates[0] = 5;

				}
				else {
					windows[0].bStates[0] = 2;
					windows[0].bStates[1] = 0;
					windows[0].bStates[2] = 0;
					windows[0].bStates[3] = 2;
					windows[0].bStates[4] = 2;
					windows[0].bStates[5] = 2;

					windows[6].bStates[0] = 2;
					windows[6].bStates[1] = 1;
					windows[6].bStates[2] = 1;
					windows[6].bStates[3] = 1;
					windows[6].bStates[4] = 1;
					windows[6].bStates[5] = 0;
					windows[6].bStates[6] = 0;
					windows[6].bStates[7] = 2;

					windows[7].bStates[0] = 2;
					windows[7].bStates[1] = 1;
					windows[7].bStates[2] = 1;
					windows[7].bStates[3] = 1;
					windows[7].bStates[4] = 1;
					windows[7].bStates[5] = 1;
					windows[7].bStates[6] = 1;
					windows[7].bStates[7] = 1;
					if (GetKey(VK_BACK).bPressed)
					{
						bDiffItem = 1;
						windows[0].bStates[4] = 3;
						//powrÛt do czasu poczatkowego
						if (EditMode == 0) {
							chosen_scn = 0;
							fSceneTime = scenes[chosen_scn].startTime;
							if (scenes[chosen_scn].cameras.size() == 0) {
								freeCamMode = 1;
							}
							else {
								freeCamMode = 0;
							}
						}
						else {
							fSceneTime = 0;
						}
						chosen_cam = scenes[chosen_scn].startCam - 1;
					}
					else if (GetKey(VK_LCONTROL).bHeld)
					{
						bOptionsBar = 1;
						windows[0].bStates[0] = 3;
						windows[1].bStates[1] = 1;
						windows[1].bStates[0] = 2;
						//windows[1].bStates[2] = 2;
						if (EditMode == 0) {
							windows[1].bStates[1] = 2;
							if (GetKey(L'E').bPressed)
							{
								windows[1].bStates[1] = 3;
								//render sekwencji
								playback = 1;
								toExport = 1;
								fSceneTime = scenes[0].startTime;
								chosen_scn = 0;
								chosen_cam = scenes[0].startCam - 1;
							}
						}
						if (GetKey(L'Z').bPressed)
						{
							windows[1].bStates[0] = 3;
							//zapis sekwencji
							ofstream data("data.txt");
							for (int s = 0; s < scenes.size(); s++) {//kolejne sceny
								data << 's' << ' ' << scenes[s].startTime << ' ' << scenes[s].endTime << ' ' << scenes[s].startCam << ' ' << int(scenes[s].col.hue) << ' ' << int(scenes[s].col.sat)
									<< ' ' << scenes[s].freeCam.vPosition.x << ' ' << scenes[s].freeCam.vPosition.y << ' ' << scenes[s].freeCam.vPosition.z
									<< ' ' << scenes[s].freeCam.qRotation.x << ' ' << scenes[s].freeCam.qRotation.y << ' ' << scenes[s].freeCam.qRotation.z << ' ' << scenes[s].freeCam.qRotation.w << endl;

								Keyframe_Save(scenes[s].cameras, data, 'v');
								Keyframe_Save(scenes[s].objects, data, 'o');
								Keyframe_Save(scenes[s].lights, data, 'l');
							}
							data << 'f';
							data.close();
						}
						/*
						else if (GetKey(L'M').bPressed)
						{
							windows[1].bStates[2] = 3;
							//zmiana trybu wyswietlania
							showMode++;
							if (showMode == 3) {
								showMode = 0;
							}
						}
						*/
					}
					else if (GetKey(VK_ESCAPE).bPressed)
					{
						windows[0].bStates[5] = 3;
						switch (EditMode) {
						case 0:
							return false;//wyjscie z programu
							break;
						case 1: {
							bDiffEditMode = 1;
							EditMode = 0;//powrÛt z edycji sceny do sekwencji scen
							if (fSceneTime > scenes[chosen_scn].endTime) {
								fSceneTime = scenes[chosen_scn].endTime;
							}
							if (fSceneTime < scenes[chosen_scn].startTime) {
								fSceneTime = scenes[chosen_scn].startTime;
							}
							break;
						}
						}
					}

					if (EditMode == 0)
					{

						//--------------// EDYCJA SEKWENCJI SCEN //-------------//

						windows[4].bStates[1] = 2;
						//windows[0].bStates[6] = 4;
						indWinKeyframe = 9;
						if (GetKey(VK_RETURN).bPressed)
						{
							windows[6].bStates[7] = 3;
							bDiffEditMode = 1;
							//wejscie w edycjÍ wybranej sceny
							EditMode = 1;
							if (fSceneTime < 0.0f) {
								fSceneTime = 0.0f;
							}
							fSceneTime = Scene_FindLastKeyframe(scenes[chosen_scn], fSceneTime);
						}
						else {
							if (GetKey(L'C').bHeld) {
								windows[4].bStates[1] = 3;
								indWinKeyframe = 8;
								//zmiana koloru sceny
								Element_Color(scenes[chosen_scn].col, fElapsedTime);
							}
							else {
								windows[9].bStates[2] = 1;
								windows[9].bStates[3] = 1;

								if (GetKey(VK_LSHIFT).bPressed)
								{
									windows[6].bStates[0] = 3;
									bDiffItem = 1;
									//dodanie nowej sceny
									vector<scene> scenesTmp;
									for (int s = scenes.size() - 1; s > chosen_scn; s--) {
										scenesTmp.push_back(scenes[s]);
										scenes.pop_back();
									}

									scene sceneTmp;
									sceneTmp.freeCam.vPosition = { 0,20,-38 };
									sceneTmp.freeCam.qRotation = { 0.17,0,0,0.98 };
									scenes.push_back(sceneTmp);

									for (int s = scenesTmp.size() - 1; s >= 0; s--) {
										scenes.push_back(scenesTmp[s]);
									}
									scenesTmp.clear();

									chosen_scn++;
									chosen_cam = 0;
									chosen_obj = 0;
									chosen_lgt = 0;
									fSceneTime = 0;
									freeCamMode = 1;
								}

								if (scenes.size() > 1) {
									windows[6].bStates[1] = 2;
									windows[6].bStates[2] = 2;
									windows[6].bStates[3] = 2;
									windows[6].bStates[4] = 2;
									if (GetKey(VK_DELETE).bPressed)
									{
										windows[6].bStates[1] = 3;
										bDiffItem = 1;
										//usuniÍcie wybranej sceny
										vector<scene> scenesTmp;
										for (int s = scenes.size() - 1; s > chosen_scn; s--) {
											scenesTmp.push_back(scenes[s]);
											scenes.pop_back();
										}
										scenes.pop_back();
										for (int s = scenesTmp.size() - 1; s >= 0; s--) {
											scenes.push_back(scenesTmp[s]);
										}
										scenesTmp.clear();
										if (chosen_scn > scenes.size() - 1) {
											chosen_scn = scenes.size() - 1;
										}
										if (top_scn > 0) {
											top_scn--;
										}
										if (scenes[chosen_scn].cameras.size() == 0) {
											freeCamMode = 1;
										}
										else {
											freeCamMode = 0;
										}
									}
									else if (GetKey(VK_MENU).bHeld)
									{
										windows[6].bStates[4] = 3;
										windows[6].bStates[5] = 1;
										windows[6].bStates[6] = 1;
										//przesuwanie scen w sekwencji
										if (chosen_scn != 0)
										{
											windows[6].bStates[5] = 2;
											//moøliwoúÊ przesuniÍcie sceny wczeúniej
											if (GetKey(VK_UP).bPressed)
											{
												windows[6].bStates[5] = 3;
												bDiffItem = 1;
												//przesuniecie sceny wczeúniej
												chosen_scn = Scene_Move(scenes, chosen_scn, 1);
											}
										}

										if (chosen_scn != scenes.size() - 1)
										{
											windows[6].bStates[6] = 2;
											//moøliwoúÊ przesuniÍcie sceny dalej
											if (GetKey(VK_DOWN).bPressed)
											{
												windows[6].bStates[6] = 3;
												bDiffItem = 1;
												//przesuniecie sceny dalej
												chosen_scn = Scene_Move(scenes, chosen_scn);
											}
										}
									}
									else {
										Fill(windows[6].posx + windows[6].sizex / 4, windows[6].posy + windows[6].sizey / 2, windows[6].posx + 2 * windows[6].sizex / 3, windows[6].posy + windows[6].sizey, colWin2.val, colWin2.hue, colWin2.sat);
										if (GetKey(VK_DOWN).bPressed)
										{
											windows[6].bStates[3] = 3;
											bDiffItem = 1;
											//nastÍpna scena
											chosen_scn++;
											if (chosen_scn >= scenes.size()) {
												chosen_scn = 0;
											}
											fSceneTime = scenes[chosen_scn].startTime;
											chosen_cam = scenes[chosen_scn].startCam - 1;
											chosen_obj = 0;
											chosen_lgt = 0;
											if (scenes[chosen_scn].cameras.size() == 0) {
												freeCamMode = 1;
											}
											else {
												freeCamMode = 0;
											}
										}
										else if (GetKey(VK_UP).bPressed)
										{
											windows[6].bStates[2] = 3;
											bDiffItem = 1;
											//poprzednia scena
											chosen_scn--;
											if (chosen_scn < 0) {
												chosen_scn = scenes.size() - 1;
											}
											fSceneTime = scenes[chosen_scn].startTime;
											chosen_cam = scenes[chosen_scn].startCam - 1;
											chosen_obj = 0;
											chosen_lgt = 0;
											if (scenes[chosen_scn].cameras.size() == 0) {
												freeCamMode = 1;
											}
											else {
												freeCamMode = 0;
											}
										}
									}
								}

								bool arrowsDefault = 1;
								if (fSceneTime == scenes[chosen_scn].startTime)
								{
									windows[10].bStates[2] = 1;
									windows[10].bStates[3] = 1;
									windows[9].bStates[0] = 2;
									//moøliwoúÊ przesuniÍcia poczπtkowej klatki kluczowej sceny
									if (GetKey(VK_MENU).bHeld)
									{
										windows[9].bStates[0] = 5;
										indWinKeyframe = 10;
										windows[10].bStates[0] = 5;
										arrowsDefault = 0;
										//przesuniÍcie poczπtkowej klatki kluczowej sceny
										if (fSceneTime > 0.0f)
										{
											windows[10].bStates[2] = 2;
											//moøliwoúÊ przesuniÍcia kl. kl. wczesniej
											if (GetKey(VK_LEFT).bHeld)
											{
												windows[10].bStates[2] = 3;
												//przesuniecie kl. kl. wczeúniej w czasie
												fSceneTime = fSceneTime - fElapsedTime;
												if (fSceneTime < 0.0f) {
													fSceneTime = 0.0f;
												}
												scenes[chosen_scn].startTime = fSceneTime;
											}
										}

										if (fSceneTime < scenes[chosen_scn].endTime)
										{
											windows[10].bStates[3] = 2;
											//moøliwoúÊ przesuniÍcia kl. kl. dalej
											if (GetKey(VK_RIGHT).bHeld) {
												windows[10].bStates[3] = 3;
												//przesuniecie kl. kl. dalej w czasie
												fSceneTime = fSceneTime + fElapsedTime;
												if (fSceneTime > scenes[chosen_scn].endTime) {
													fSceneTime = scenes[chosen_scn].endTime - 0.1;
												}
												scenes[chosen_scn].startTime = fSceneTime;
											}
										}
									}
								}

								if (fSceneTime == scenes[chosen_scn].endTime)
								{
									windows[9].bStates[0] = 2;
									//moøliwoúÊ przesuniÍcia koÒcowej klatki kluczowej sceny
									if (GetKey(VK_MENU).bHeld)
									{
										windows[9].bStates[0] = 5;
										indWinKeyframe = 10;
										windows[10].bStates[0] = 5;
										arrowsDefault = 0;
										//przesuniÍcie koÒcowej klatki kluczowej sceny
										float lastFrTime = 0.0f;
										lastFrTime = Element_FindLastKeyframe(scenes[chosen_scn].cameras, lastFrTime);
										lastFrTime = Element_FindLastKeyframe(scenes[chosen_scn].objects, lastFrTime);
										lastFrTime = Element_FindLastKeyframe(scenes[chosen_scn].lights, lastFrTime);
										if (fSceneTime < lastFrTime)
										{
											windows[10].bStates[3] = 2;
											//moøliwoúÊ przesuniÍcia kl kl. dalej
											if (GetKey(VK_RIGHT).bHeld)
											{
												windows[10].bStates[3] = 3;
												//przesuniecie kl. kl. dalej w czasie
												fSceneTime = fSceneTime + fElapsedTime;
												if (fSceneTime > lastFrTime) {
													fSceneTime = lastFrTime;
												}
												scenes[chosen_scn].endTime = fSceneTime;
											}
										}

										if (fSceneTime > scenes[chosen_scn].startTime) {
											windows[10].bStates[2] = 2;
											//moøliwoúÊ przesuniÍcia kl kl. wczeúniej
											if (GetKey(VK_LEFT).bHeld)
											{
												windows[10].bStates[2] = 3;
												//przesuniecie kl. kl. wczeúniej w czasie
												fSceneTime = fSceneTime - fElapsedTime;
												if (fSceneTime < scenes[chosen_scn].startTime) {
													fSceneTime = scenes[chosen_scn].startTime + 0.1;
												}
												scenes[chosen_scn].endTime = fSceneTime;
											}
										}
									}
								}

								if ((chosen_scn != scenes.size() - 1 || fSceneTime != scenes[scenes.size() - 1].endTime) && arrowsDefault == 1)
								{
									windows[9].bStates[3] = 2;
									//moøliwoúÊ wybrania nastÍpnej klatki kluczowej w sekwencji scen
									if (GetKey(VK_RIGHT).bPressed)
									{
										windows[9].bStates[3] = 3;
										//wybranie nastÍpnej klatki kluczowej w sekwencji scen
										if (fSceneTime == scenes[chosen_scn].endTime) {
											if (chosen_scn < scenes.size() - 1) {
												chosen_scn++;
												fSceneTime = scenes[chosen_scn].startTime;
												chosen_cam = scenes[chosen_scn].startCam - 1;
												chosen_obj = 0;
												chosen_lgt = 0;
												if (scenes[chosen_scn].cameras.size() == 0) {
													freeCamMode = 1;
												}
												else {
													freeCamMode = 0;
												}
											}
										}
										else {
											fSceneTime = scenes[chosen_scn].endTime;
										}
									}
								}

								if ((chosen_scn != 0 || fSceneTime != scenes[0].startTime) && arrowsDefault == 1)
								{
									windows[9].bStates[2] = 2;
									//moøliwoúÊ wybrania poprzedniej klatki kluczowej w sekwencji scen
									if (GetKey(VK_LEFT).bPressed) {
										windows[9].bStates[2] = 3;
										//wybranie poprzedniej klatki kluczowej w sekwencji scen
										if (fSceneTime == scenes[chosen_scn].startTime) {
											if (chosen_scn > 0) {
												chosen_scn--;
												fSceneTime = scenes[chosen_scn].endTime;
												chosen_cam = scenes[chosen_scn].startCam - 1;
												chosen_obj = 0;
												chosen_lgt = 0;
												if (scenes[chosen_scn].cameras.size() == 0) {
													freeCamMode = 1;
												}
												else {
													freeCamMode = 0;
												}
											}
										}
										else {
											fSceneTime = scenes[chosen_scn].startTime;
										}
									}
								}
							}
						}
					}

					if (EditMode != 0) {
						windows[5].bStates[0] = 2;
						windows[5].bStates[1] = 2;
						windows[5].bStates[2] = 2;
						indWinKeyframe = 11;

						if (GetKey(L'P').bPressed) {
							ElementType = 1;
							bDiffTransformMode = 1;
						}
						else if (GetKey(L'O').bPressed) {
							ElementType = 2;
							bDiffTransformMode = 1;
						}
						else if (GetKey(L'L').bPressed) {
							ElementType = 3;
							bDiffTransformMode = 1;
						}
						switch (ElementType) {
						case 1:
							windows[5].bStates[0] = 5;
							EditElement(scenes[chosen_scn].cameras, chosen_cam, fElapsedTime, top_cam);
							break;
						case 2:
							windows[5].bStates[1] = 5;
							EditElement(scenes[chosen_scn].objects, chosen_obj, fElapsedTime, top_obj);
							break;
						case 3:
							windows[5].bStates[2] = 5;
							EditElement(scenes[chosen_scn].lights, chosen_lgt, fElapsedTime, top_lgt);
							break;
						}
					}
				}
			}
		}

		//--------------// WYZNACZENIE AKTUALNEGO PO£OØENIA ELEMENT”W SCENY //-------------//

		if (scenes[chosen_scn].cameras.size() > 1) {
			int Sframe_num = Keyframe_Find(scenes[chosen_scn].cameras[chosen_cam].keyframes, fSceneTime);
			if (playback == 1 && scenes[chosen_scn].cameras[chosen_cam].keyframes[Sframe_num].nextCam > 0) // CI CIE DO WSKAZANEJ KAMERY PODCZAS PLAYBACKU
			{
				chosen_cam = scenes[chosen_scn].cameras[chosen_cam].keyframes[Sframe_num].nextCam - 1;
			}
		}

		if (ElementType == 1) {
			for (int i = 0; i < scenes[chosen_scn].cameras.size(); i++) // AKTUALNE PO£OØENIE KAMER
			{
				scenes[chosen_scn].cameras[i].actPos = Keyframe_Interpolation(scenes[chosen_scn].cameras[i].keyframes, fSceneTime);
				if (!Vectors_Equal(scenes[chosen_scn].cameras[i].actPos.vPosition, scenes[chosen_scn].cameras[i].prevPos.vPosition)
					|| !Vectors_Equal(scenes[chosen_scn].cameras[i].actPos.qRotation, scenes[chosen_scn].cameras[i].prevPos.qRotation)
					|| scenes[chosen_scn].cameras[i].actPos.qRotation.w != scenes[chosen_scn].cameras[i].prevPos.qRotation.w
					|| scenes[chosen_scn].cameras[i].actPos.fScale != scenes[chosen_scn].cameras[i].prevPos.fScale
					) {
					PointMesh_Transform(scenes[chosen_scn].cameras[i]);// WPISANIE TRANSFORMOWANYCH TR”JK•T”W DO WEKTORA
				}
			}
		}
		else if (freeCamMode == 0) {
			scenes[chosen_scn].cameras[chosen_cam].actPos = Keyframe_Interpolation(scenes[chosen_scn].cameras[chosen_cam].keyframes, fSceneTime);
		}

		for (int i = 0; i < scenes[chosen_scn].objects.size(); i++) { // AKTUALNE PO£OØENIE OBIEKT”W
			scenes[chosen_scn].objects[i].actPos = Keyframe_Interpolation(scenes[chosen_scn].objects[i].keyframes, fSceneTime);
			if (!Vectors_Equal(scenes[chosen_scn].objects[i].actPos.vPosition, scenes[chosen_scn].objects[i].prevPos.vPosition)
				|| !Vectors_Equal(scenes[chosen_scn].objects[i].actPos.qRotation, scenes[chosen_scn].objects[i].prevPos.qRotation)
				|| scenes[chosen_scn].objects[i].actPos.qRotation.w != scenes[chosen_scn].objects[i].prevPos.qRotation.w
				|| scenes[chosen_scn].objects[i].actPos.fScale != scenes[chosen_scn].objects[i].prevPos.fScale
				) {
				PointMesh_Transform(scenes[chosen_scn].objects[i]);// WPISANIE TRANSFORMOWANYCH TR”JK•T”W DO WEKTORA
			}
		}

		for (int i = 0; i < scenes[chosen_scn].lights.size(); i++) // AKTUALNE PO£OØENIE èR”DE£ åWIAT£A
		{
			scenes[chosen_scn].lights[i].actPos = Keyframe_Interpolation(scenes[chosen_scn].lights[i].keyframes, fSceneTime);
			if (ElementType == 3) // TRANSFORMACJA I PROJEKCJA SIATEK èR”DE£ åWIAT£A
			{
				if (!Vectors_Equal(scenes[chosen_scn].lights[i].actPos.vPosition, scenes[chosen_scn].lights[i].prevPos.vPosition)
					|| scenes[chosen_scn].lights[i].actPos.fScale != scenes[chosen_scn].lights[i].prevPos.fScale
					) {
					PointMesh_Transform(scenes[chosen_scn].lights[i]);// WPISANIE TRANSFORMOWANYCH TR”JK•T”W DO WEKTORA
				}

			}
		}

		for (int i = 0; i < scenes[chosen_scn].objects.size(); i++) {
			scenes[chosen_scn].objects[i].prevPos = scenes[chosen_scn].objects[i].actPos;
		}


		//--------------// WYZNACZENIE MACIERZY KAMERY //-------------//

		orientedPoint realCam;
		if (freeCamMode == 0) {
			realCam = scenes[chosen_scn].cameras[chosen_cam].actPos;
		}
		else {
			realCam = scenes[chosen_scn].freeCam;
		}

		vec3d vUp0 = { 0,1,0 };
		vec3d vFwd0 = { 0,0,1 };

		vec3d vUp1 = Quaternion_RotateVec(vUp0, realCam.qRotation);
		vec3d vFwd1 = Quaternion_RotateVec(vFwd0, realCam.qRotation);

		mat4x4 matCamera = Matrix_ChangeOfBasis(realCam.vPosition, vFwd1, vUp1); // MACIERZ ZMIANY BAZY KAMERY

		mat4x4 matView = Matrix_InverseView(matCamera); // MACIERZ KAMERY



		//--------------// CIENIOWANIE I PROJEKCJA TR”JK•T”W //-------------//

		vector<pTriangle> vecTrianglesToRaster; // WEKTOR Z TR”JK•TAMI DO RASTROWANIA

		if (ElementType == 3) // TRANSFORMACJA I PROJEKCJA SIATEK èR”DE£ åWIAT£A
		{
			for (int i = 0; i < scenes[chosen_scn].lights.size(); i++) // TRANSFORMACJA I PROJEKCJA SIATEK OBIEKT”W
			{
				Triangle_Shader(vecTrianglesToRaster, scenes[chosen_scn].lights[i], scenes[chosen_scn].objects, matView, scenes[chosen_scn].lights, realCam);// WPISANIE TRANSFORMOWANYCH TR”JK•T”W DO WEKTORA
			}
		}
		else if (ElementType == 1) // TRANSFORMACJA I PROJEKCJA SIATEK KAMER
		{
			for (int i = 0; i < scenes[chosen_scn].cameras.size(); i++) // TRANSFORMACJA I PROJEKCJA SIATEK OBIEKT”W
			{
				Triangle_Shader(vecTrianglesToRaster, scenes[chosen_scn].cameras[i], scenes[chosen_scn].objects, matView, scenes[chosen_scn].lights, realCam);// WPISANIE TRANSFORMOWANYCH TR”JK•T”W DO WEKTORA
			}
		}

		for (int i = 0; i < scenes[chosen_scn].objects.size(); i++) // TRANSFORMACJA I PROJEKCJA SIATEK OBIEKT”W
		{
			Triangle_Shader(vecTrianglesToRaster, scenes[chosen_scn].objects[i], scenes[chosen_scn].objects, matView, scenes[chosen_scn].lights, realCam, true);// WPISANIE TRANSFORMOWANYCH TR”JK•T”W DO WEKTORA
		}

		element basePlane;
		basePlane.pointMesh = meshes[2].points;
		basePlane.col = { 0,0,0 };
		basePlane.obj_name = 2;
		// PROJEKCJA SIATKI BAZOWEJ
		PointMesh_Transform(basePlane);// WPISANIE TRANSFORMOWANYCH TR”JK•T”W DO WEKTORA
		Triangle_Shader(vecTrianglesToRaster, basePlane, scenes[chosen_scn].objects, matView, scenes[chosen_scn].lights, realCam, true);// WPISANIE TRANSFORMOWANYCH TR”JK•T”W DO WEKTORA



		//--------------// PRZYCIANANIE I RASTERYZACJA TR”JK•T”W //-------------//

		// SORTOWANIE TR”JK•T”W NA PODSTAWIE åREDNIEJ ODLEG£OåCI WIERZCHOLKOW OD KAMERY W OSI Z
		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](pTriangle& t1, pTriangle& t2)
			{	float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
		float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
		return z1 > z2; });

		if (bOptionsBar == 1 && toExport == 0) {
			DrawWindow(windows[1]);
			UpdateWindow(windows[1]);
		}
		else {
			if (GetKey(VK_LCONTROL).bReleased) {
				DrawWindow(windows[2]);
			}
			float addX = 0;
			float addY = 0;
			float scl = 1;
			if (toExport == 0) {
				Fill(oxFrame, oyFrame, oxFrame + kFrame * frameWidth, oyFrame + kFrame * frameHeight, 0);
				addX = oxFrame;
				addY = oyFrame;
				scl = kFrame;
			}
			else {
				Fill(0, 0, frameWidth, frameHeight, 0);
			}

			for (auto& triToRaster : vecTrianglesToRaster) // PRZEJåCIE PRZEZ WSZYSTKIE TR”JK•TY DO RASTROWANIA
			{
				pTriangle clipped[2]; // EWENTUALNE NOWE TR”JK•TY POWSTA£E PRZY KLIPOWANIU WZGL. JEDNEJ Z P£ASZCZYZN
				list<pTriangle> listTriangles; // TROJK•TY DO RYSOWANIA PO CLIPOWANIU (MOØE BY∆ ICH WI KSZA LICZBA!)

				listTriangles.push_back(triToRaster);
				int nNewTriangles = 1; // WPISANIE TR”JK•TA DO KOLEJKI

				for (int p = 0; p < 4; p++) // CLIPOWANIE TR”JK•T”W WZGLEDEM CZTERECH KRAW DZI EKRANU
				{
					int nTrisToAdd = 0; // ILOå∆ TR”JK•T”W DO DOPISANIA (0/1/2) PO CLIPOWANIU
					while (nNewTriangles > 0) // SPRAWDZANIE TR”JK•T”W - TAKØE NOWO POWSTA£YCH - WZGL DEM KOLEJNYCH KRAW DZI
					{
						pTriangle test = listTriangles.front();
						listTriangles.pop_front();
						nNewTriangles--; // ZABRANIE PIERWSZEGO TR”JK•TA Z KOLEJKI

						switch (p) // CLIPOWANIE TR”JK•T”W O DAN• KOLEJN• KRAW Dè EKRANU, ZWR”CENIE NOWYCH TR”JK•T”W I ICH LICZBY
						{
						case 0:	nTrisToAdd = Triangle_Clip({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
						case 1:	nTrisToAdd = Triangle_Clip({ 0.0f, float(frameHeight) - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
						case 2:	nTrisToAdd = Triangle_Clip({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
						case 3:	nTrisToAdd = Triangle_Clip({ float(frameWidth) - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
						}

						for (int w = 0; w < nTrisToAdd; w++) { // WPISANIE NOWYCH TR”JK•T”W DO LISTY TR”JK•T”W
							listTriangles.push_back(clipped[w]);
						}
					}
					nNewTriangles = listTriangles.size(); // ILOå∆ TR”JK•T”W PO CLIPOWANIU WZGL. DANEJ KRAW DZI
				}

				for (auto& t : listTriangles) // NARYSOWANIE WSZYSTKICH TR”JK•T”W, PO TRANSFORMACJI, PROJEKCJI I CLIPOWANIU
				{
					FillTriangle(scl * t.p[0].x + addX, scl * t.p[0].y + addY, scl * t.p[1].x + addX, scl * t.p[1].y + addY, scl * t.p[2].x + addX, scl * t.p[2].y + addY, t.col.val, t.col.hue, t.col.sat);
				}
			}
			vecTrianglesToRaster.clear();
		}


		//--------------// AKTUALIZACJA INTERFEJSU GRAFICZNEGO //-------------//

		if (toExport == 2) {
			DrawWindow(windows[0]);
			DrawWindow(windows[2]);
			DrawWindow(windows[3]);
			DrawWindow(windows[4]);
			DrawWindow(windows[7]);
			DrawWindow(windows[9]);

			DrawSceneList(scenes, "SCENA", chosen_scn, top_scn);

			if (true) {
				//rysowanie osi czasu
				DrawWindow(windows[3]);
				DrawLine(40, wNavigateHeight + wPreviewHeight + 35, wPreviewWidth - 40, wNavigateHeight + wPreviewHeight + 35);
				DrawLine(40, wNavigateHeight + wPreviewHeight + 25, 40, wNavigateHeight + wPreviewHeight + 45);
				DrawLine(wPreviewWidth - 40, wNavigateHeight + wPreviewHeight + 25, wPreviewWidth - 40, wNavigateHeight + wPreviewHeight + 45);

				if (EditMode == 0) {
					DrawSequenceTimeline(scenes, chosen_scn, Ttop_scn);
				}
				else {
					float finTime = 0;
					finTime = Element_FindLastKeyframe(scenes[chosen_scn].cameras, finTime);
					finTime = Element_FindLastKeyframe(scenes[chosen_scn].objects, finTime);
					finTime = Element_FindLastKeyframe(scenes[chosen_scn].lights, finTime);
					if (finTime == 0) {
						finTime = 1;
					}
					switch (ElementType) {
					case 1:
						DrawSceneTimeline(scenes[chosen_scn].cameras, finTime, chosen_cam, Ttop_cam, scenes[chosen_scn].startTime, scenes[chosen_scn].endTime, scenes[chosen_scn].col);
						break;
					case 2:
						DrawSceneTimeline(scenes[chosen_scn].objects, finTime, chosen_obj, Ttop_obj, scenes[chosen_scn].startTime, scenes[chosen_scn].endTime, scenes[chosen_scn].col);
						break;
					case 3:
						DrawSceneTimeline(scenes[chosen_scn].lights, finTime, chosen_lgt, Ttop_lgt, scenes[chosen_scn].startTime, scenes[chosen_scn].endTime, scenes[chosen_scn].col);
						break;
					}
				}

				// WYSWIETLANIE CZASU
				float globalTime = fSceneTime;
				float sceneTime = fSceneTime;
				float globalTimeFin = 0.0f;
				if (EditMode == 0) {
					//aktualny i koÒcowy czas globalny w sekwencji scen
					globalTime = fSceneTime - scenes[chosen_scn].startTime;
					for (int s = 0; s < scenes.size(); s++) {
						globalTimeFin = globalTimeFin + (scenes[s].endTime - scenes[s].startTime);
						if (s < chosen_scn) {
							float prevScnTime = scenes[s].endTime - scenes[s].startTime;
							globalTime = globalTime + prevScnTime;
						}
					}
				}
				else {
					//lokalny czas koÒcowy w scenie
					globalTimeFin = Element_FindLastKeyframe(scenes[chosen_scn].cameras, globalTimeFin);
					globalTimeFin = Element_FindLastKeyframe(scenes[chosen_scn].objects, globalTimeFin);
					globalTimeFin = Element_FindLastKeyframe(scenes[chosen_scn].lights, globalTimeFin);
				}

				//wyswietlanie czasu lokalnego sceny oraz indeksu wybranej sceny/kamery
				if (EditMode == 0 || freeCamMode == 0) {
					string sScene1;
					string sScene2;
					if (EditMode == 0) {
						sScene1 = " SCENA   ";
						int scnTens = (chosen_scn + 1) / 10;
						int scnSingl = (chosen_scn + 1) - scnTens * 10;
						if (scnTens > 0) {
							sScene1[7] = scnTens + 48;
							sScene1[8] = scnSingl + 48;
						}
						else {
							sScene1[7] = scnSingl + 48;
						}
						Time_Format(sceneTime, sScene2);
					}
					else {
						if (freeCamMode == 0) {
							sScene1 = "KAMERA   ";
							int camTens = (chosen_cam + 1) / 10;
							int camSingl = (chosen_cam + 1) - camTens * 10;
							if (camTens > 0) {
								sScene1[7] = camTens + 48;
								sScene1[8] = camSingl + 48;
							}
							else {
								sScene1[7] = camSingl + 48;
							}
						}
						else {
							sScene1 = "   WOLNA";
							sScene2 = "   KAMERA";
						}
					}
					Fill(360, 15, 490, 65, 50);
					Text_Write(sScene1, 385, 20, 2);
					Text_Write(sScene2, 375, 40, 2, false);
				}

				//rysowanie wskaznika na osi czasu
				DrawTimeIndicator(globalTime, globalTimeFin, true, 100, 255);

				//wyswietlanie czasu koncowego i poczatkowego
				string sGlobalTimeFin;
				Time_Format(globalTimeFin, sGlobalTimeFin);
				Text_Write(sGlobalTimeFin, wPreviewWidth - 80, wNavigateHeight + wPreviewHeight + 8, 1.5, false);
				Text_Write("00:00:00", 8, wNavigateHeight + wPreviewHeight + 8, 1.5, false);

				//wyswietlanie aktualnego czasu globalnego
				string sGlobalTime;
				Time_Format(globalTime, sGlobalTime);
				Fill(545 - 25, 15, 780 - 25, 65, 50);
				Text_Write(sGlobalTime, 560 - 25, 20, 4, false);
			}
		}

		if (toExport == 0) {
			if (playback == 0) {
				windows[0].buttons[3].sym = "|";
			}
			else {
				windows[0].buttons[3].sym = ">";
			}
			UpdateWindow(windows[0]);
			DrawLine(0, wNavigateHeight, wPreviewWidth, wNavigateHeight);
			DrawLine(0, wNavigateHeight + wPreviewHeight, wPreviewWidth, wNavigateHeight + wPreviewHeight);

			if (indPrevWinKeyframe != indWinKeyframe) {
				DrawWindow(windows[indWinKeyframe]);
				if (indWinKeyframe == 12) {
					Text_Write("PRZESUNIECIE", windows[12].posx + 6, windows[12].posy + 98, 2);
					Text_Write("OBROT", windows[12].posx + 6, windows[12].posy + 250, 2);
					Text_Write("SKALOWANIE", windows[12].posx + 6, windows[12].posy + 400, 2);
				}
				if (indWinKeyframe == 14) {
					string meshName = "X - ";
					int size = meshes.size();
					for (int i = 3; i < min(12, size); i++) {
						meshName[0] = i + 48 - 2;
						float ofst = Text_Write(meshName, windows[14].posx + 30, windows[14].posy + i * 30 - 40, 2);
						Text_Write(meshes[i].filename, ofst, windows[14].posy + i * 30 - 40, 2);
					}
					//Text_Write("LUB", windows[14].posx + 10, windows[14].posy + 360, 2);
				}
			}

			string sKeyframe = "ERROR";
			int iKeyframe = -2;
			switch (indWinKeyframe) {
			case 8:
				if (EditMode == 0) {
					sKeyframe = "USTAW KOLOR SCENY";
					iKeyframe = chosen_scn;
				}
				else {
					switch (ElementType) {
					case 1:
						sKeyframe = "USTAW KOLOR KAMERY";
						iKeyframe = chosen_cam;
						break;
					case 2:
						sKeyframe = "USTAW KOLOR OBIEKTU";
						iKeyframe = chosen_obj;
						break;
					case 3:
						sKeyframe = "USTAW KOLOR ZRODLA SWIATLA";
						iKeyframe = chosen_lgt;
						break;
					}
				}
				break;
			case 9:
				sKeyframe = "SCENA";
				iKeyframe = chosen_scn;
				break;
			case 10:
				sKeyframe = "SCENA";
				iKeyframe = chosen_scn;
				break;
			case 11:
				sKeyframe = "PRZEJDZ DO KLATKI KLUCZOWEJ W CELU EDYCJI";
				break;
			case 14:
				sKeyframe = "WCISNIJ NUMER SIATKI OBIEKTU:";
				break;
			default:
				int iKey = 0;
				switch (ElementType) {
				case 1:
					if (scenes[chosen_scn].cameras.size() > 0) {
						iKey = Keyframe_Find(scenes[chosen_scn].cameras[chosen_cam].keyframes, fSceneTime);
					}
					break;
				case 2:
					if (scenes[chosen_scn].objects.size() > 0) {
						iKey = Keyframe_Find(scenes[chosen_scn].objects[chosen_obj].keyframes, fSceneTime);
					}
					break;
				case 3:
					if (scenes[chosen_scn].lights.size() > 0) {
						iKey = Keyframe_Find(scenes[chosen_scn].lights[chosen_lgt].keyframes, fSceneTime);
					}
					break;
				}
				sKeyframe = "KLATKA KLUCZOWA";
				iKeyframe = iKey - 1;
				break;
			}
			UpdateWindow(windows[indWinKeyframe], sKeyframe, iKeyframe);


			if (bDiffColor == 1 || (indPrevWinKeyframe != indWinKeyframe && indWinKeyframe == 8)) {
				DrawColorWheel(windows[8].posx + windows[8].sizex / 2, windows[8].posy + windows[8].sizey / 3, 100);
			}

			if (EditMode == 0) {
				if (bDiffEditMode == 1) {
					DrawWindow(windows[4]);
					DrawWindow(windows[6]);
				}
				UpdateWindow(windows[4], "SEKWENCJA SCEN");
				UpdateWindow(windows[6], "SCENA", chosen_scn);
				if (bDiffColor == 1 || bDiffItem == 1 || bDiffEditMode == 1) {
					DrawSceneList(scenes, "SCENA", chosen_scn, top_scn);
				}
				if (bDiffColor == 1 || (indPrevWinKeyframe != indWinKeyframe && indWinKeyframe == 8)) {
					DrawColorIndicator(windows[8].posx + windows[8].sizex / 2, windows[8].posy + windows[8].sizey / 3, 100.0f, scenes[chosen_scn].col.hue, scenes[chosen_scn].col.sat);
				}
			}
			else {
				if (bDiffEditMode == 1) {
					DrawWindow(windows[5]);
					DrawWindow(windows[7]);
				}
				UpdateWindow(windows[5], "ELEMENTY SCENY", chosen_scn);
				string sElement = "ELEMENT";
				int iElement = 0;
				switch (ElementType) {
				case 1:
					sElement = "KAMERA";
					iElement = chosen_cam;
					if (bDiffColor == 1 || bDiffItem == 1 || bDiffEditMode == 1 || bDiffTransformMode == 1) {
						DrawElemList(scenes[chosen_scn].cameras, "KAMERA", chosen_cam, top_cam);
					}
					if (bDiffColor == 1 || (indPrevWinKeyframe != indWinKeyframe && indWinKeyframe == 8)) {
						DrawColorIndicator(windows[8].posx + windows[8].sizex / 2, windows[8].posy + windows[8].sizey / 3, 100.0f, scenes[chosen_scn].cameras[chosen_cam].col.hue, scenes[chosen_scn].cameras[chosen_cam].col.sat);
					}
					break;
				case 2:
					sElement = "OBIEKT";
					iElement = chosen_obj;
					if (bDiffColor == 1 || bDiffItem == 1 || bDiffEditMode == 1 || bDiffTransformMode == 1) {
						DrawElemList(scenes[chosen_scn].objects, "OBIEKT", chosen_obj, top_obj);
					}
					if (bDiffColor == 1 || (indPrevWinKeyframe != indWinKeyframe && indWinKeyframe == 8)) {
						DrawColorIndicator(windows[8].posx + windows[8].sizex / 2, windows[8].posy + windows[8].sizey / 3, 100.0f, scenes[chosen_scn].objects[chosen_obj].col.hue, scenes[chosen_scn].objects[chosen_obj].col.sat);
					}
					break;
				case 3:
					sElement = "ZRODLO SWIATLA";
					iElement = chosen_lgt;
					if (bDiffColor == 1 || bDiffItem == 1 || bDiffEditMode == 1 || bDiffTransformMode == 1) {
						DrawElemList(scenes[chosen_scn].lights, "ZRODLO SWIATLA", chosen_lgt, top_lgt);
					}
					if (bDiffColor == 1 || (indPrevWinKeyframe != indWinKeyframe && indWinKeyframe == 8)) {
						DrawColorIndicator(windows[8].posx + windows[8].sizex / 2, windows[8].posy + windows[8].sizey / 3, 100.0f, scenes[chosen_scn].lights[chosen_lgt].col.hue, scenes[chosen_scn].lights[chosen_lgt].col.sat);
					}
					break;
				}
				UpdateWindow(windows[7], sElement, iElement);
			}

			if (fSceneTime != prevTime || bDiffItem == 1 || bDiffEditMode == 1 || bDiffTransformMode == 1 || bDiffColor == 1 || init == 1 || bDiffKeyframe == 1 || iOutFreeCam == 2) {
				//rysowanie osi czasu
				DrawWindow(windows[3]);
				DrawLine(40, wNavigateHeight + wPreviewHeight + 35, wPreviewWidth - 40, wNavigateHeight + wPreviewHeight + 35);
				DrawLine(40, wNavigateHeight + wPreviewHeight + 25, 40, wNavigateHeight + wPreviewHeight + 45);
				DrawLine(wPreviewWidth - 40, wNavigateHeight + wPreviewHeight + 25, wPreviewWidth - 40, wNavigateHeight + wPreviewHeight + 45);

				if (EditMode == 0) {
					DrawSequenceTimeline(scenes, chosen_scn, Ttop_scn);
				}
				else {
					float finTime = 0;
					finTime = Element_FindLastKeyframe(scenes[chosen_scn].cameras, finTime);
					finTime = Element_FindLastKeyframe(scenes[chosen_scn].objects, finTime);
					finTime = Element_FindLastKeyframe(scenes[chosen_scn].lights, finTime);
					if (finTime == 0) {
						finTime = 1;
					}
					switch (ElementType) {
					case 1:
						DrawSceneTimeline(scenes[chosen_scn].cameras, finTime, chosen_cam, Ttop_cam, scenes[chosen_scn].startTime, scenes[chosen_scn].endTime, scenes[chosen_scn].col);
						break;
					case 2:
						DrawSceneTimeline(scenes[chosen_scn].objects, finTime, chosen_obj, Ttop_obj, scenes[chosen_scn].startTime, scenes[chosen_scn].endTime, scenes[chosen_scn].col);
						break;
					case 3:
						DrawSceneTimeline(scenes[chosen_scn].lights, finTime, chosen_lgt, Ttop_lgt, scenes[chosen_scn].startTime, scenes[chosen_scn].endTime, scenes[chosen_scn].col);
						break;
					}
				}

				// WYSWIETLANIE CZASU
				float fSequenceTime = fSceneTime;
				float fSequenceEndTime = 0.0f;
				if (EditMode == 0) {
					//aktualny i koÒcowy czas globalny w sekwencji scen
					fSequenceTime = fSceneTime - scenes[chosen_scn].startTime;
					for (int s = 0; s < scenes.size(); s++) {
						fSequenceEndTime = fSequenceEndTime + (scenes[s].endTime - scenes[s].startTime);
						if (s < chosen_scn) {
							fSequenceTime = fSequenceTime + (scenes[s].endTime - scenes[s].startTime);
						}
					}
				}
				else {
					//lokalny czas koÒcowy w scenie
					fSequenceEndTime = Element_FindLastKeyframe(scenes[chosen_scn].cameras, fSequenceEndTime);
					fSequenceEndTime = Element_FindLastKeyframe(scenes[chosen_scn].objects, fSequenceEndTime);
					fSequenceEndTime = Element_FindLastKeyframe(scenes[chosen_scn].lights, fSequenceEndTime);
				}

				//wyswietlanie czasu lokalnego sceny oraz indeksu wybranej sceny/kamery
				if (EditMode == 0 || freeCamMode == 0) {
					string sScene1;
					string sScene2;
					if (EditMode == 0) {
						sScene1 = " SCENA   ";
						int scnTens = (chosen_scn + 1) / 10;
						int scnSingl = (chosen_scn + 1) - scnTens * 10;
						if (scnTens > 0) {
							sScene1[7] = scnTens + 48;
							sScene1[8] = scnSingl + 48;
						}
						else {
							sScene1[7] = scnSingl + 48;
						}
						Time_Format(fSceneTime, sScene2);
					}
					else {
						if (freeCamMode == 0) {
							sScene1 = "KAMERA   ";
							int camTens = (chosen_cam + 1) / 10;
							int camSingl = (chosen_cam + 1) - camTens * 10;
							if (camTens > 0) {
								sScene1[7] = camTens + 48;
								sScene1[8] = camSingl + 48;
							}
							else {
								sScene1[7] = camSingl + 48;
							}
						}
						else {
							sScene1 = "   WOLNA";
							sScene2 = "   KAMERA";
						}
					}
					Fill(360, 15, 490, 65, 50);
					Text_Write(sScene1, 385, 20, 2);
					Text_Write(sScene2, 375, 40, 2, false);
				}

				//rysowanie wskaznika na osi czasu
				DrawTimeIndicator(fSequenceTime, fSequenceEndTime, true, 100, 255);

				//wyswietlanie czasu koncowego i poczatkowego
				string sSequenceEndTime;
				Time_Format(fSequenceEndTime, sSequenceEndTime);
				Text_Write(sSequenceEndTime, wPreviewWidth - 80, wNavigateHeight + wPreviewHeight + 8, 1.5, false);
				Text_Write("00:00:00", 8, wNavigateHeight + wPreviewHeight + 8, 1.5, false);

				//wyswietlanie aktualnego czasu globalnego
				string sSequenceTime;
				Time_Format(fSequenceTime, sSequenceTime);
				Fill(545 - 25, 15, 780 - 25, 65, 50);
				Text_Write(sSequenceTime, 560 - 25, 20, 4, false);
			}

			for (int i = 0; i < 15; i++) {
				windows[i].bPrevStates.clear();
				for (int s = 0; s < windows[i].buttons.size(); s++) {
					uchar sTmp = windows[i].bStates[s];
					windows[i].bPrevStates.push_back(sTmp);
				}
			}

			init = 0;
			if (iOutFreeCam == 1) {
				iOutFreeCam++;
			}
		}

		return true;
	}
};

int main()
{
	olcEngine3D demo;
	if (demo.CreateBuffer())
		demo.Start();
	return 0;
}
