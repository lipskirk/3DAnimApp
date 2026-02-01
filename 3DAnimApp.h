#pragma once
#pragma comment(lib, "winmm.lib")

#ifndef UNICODE
#error Enable UNICODE
#endif

#include <windows.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <condition_variable>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>  
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

struct pixHSV {
	uchar hue = 0;
	uchar sat = 0;
	uchar val = 255;
};

class Animation3DEngine
{
public:
	Animation3DEngine()
	{
		std::memset(m_keyNewState, 0, 256 * sizeof(short));
		std::memset(m_keyOldState, 0, 256 * sizeof(short));
		std::memset(m_keys, 0, 256 * sizeof(sKeyState)); // WYPE£NIENIE TABLIC ZERAMI

		m_sAppName = L"3D Animation Engine";
	}


	int CreateBuffer()
	{
		ScreenBuffer = new pixHSV[screenWidth * screenHeight]; // PRZYPISANIE PAMIÊCI NA BUFOR EKRANU
		memset(ScreenBuffer, 0, sizeof(pixHSV) * screenWidth * screenHeight);

		RenderBUffer = new pixHSV[frameWidth * frameHeight]; // PRZYPISANIE PAMIÊCI NA BUFOR RENDEROWANEJ KLATKI
		memset(RenderBUffer, 0, sizeof(pixHSV) * frameWidth * frameHeight);

		return 1;
	}

	virtual void Draw(int x, int y, uchar val = 255, uchar hue = 0, uchar sat = 0)
	{
		if (toExport == 1) {
			if (x >= 0 && x < frameWidth && y >= 0 && y < frameHeight)
			{
				RenderBUffer[y * frameWidth + x].val = val;
				RenderBUffer[y * frameWidth + x].hue = hue;
				RenderBUffer[y * frameWidth + x].sat = sat;
			}
		}
		else {
			if (x >= 0 && x < screenWidth && y >= 0 && y < screenHeight)
			{
				ScreenBuffer[y * screenWidth + x].val = val;
				ScreenBuffer[y * screenWidth + x].hue = hue;
				ScreenBuffer[y * screenWidth + x].sat = sat;
			}
		}

	}

	void Fill(float x1, float y1, float x2, float y2, uchar val = 255, uchar hue = 0, uchar sat = 0)
	{
		Clip(x1, y1);
		Clip(x2, y2);
		for (int x = x1; x < x2; x++) {
			for (int y = y1; y < y2; y++) {
				Draw(x, y, val, hue, sat);
			}
		}
	}

	void Clip(float& x, float& y)
	{
		if (toExport == 0) {
			if (x < 0) {
				x = 0;
			}
			if (x >= screenWidth) {
				x = screenWidth;
			}
			if (y < 0) {
				y = 0;
			}
			if (y >= screenHeight) {
				y = screenHeight;
			}
		}
		else {
			if (x < 0) {
				x = 0;
			}
			if (x >= frameWidth) {
				x = frameWidth;
			}
			if (y < 0) {
				y = 0;
			}
			if (y >= frameHeight) {
				y = frameHeight;
			}
		}
	}

	void DrawLine(float x1, float y1, float x2, float y2, uchar val = 255, uchar hue = 0, uchar sat = 0) // RYSOWANIE LINII MIÊDZY DWOMA PUNKTAMI
	{
		int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
		dx = x2 - x1; dy = y2 - y1;
		dx1 = abs(dx); dy1 = abs(dy);
		px = 2 * dy1 - dx1;	py = 2 * dx1 - dy1;
		if (dy1 <= dx1)
		{
			if (dx >= 0)
			{
				x = x1; y = y1; xe = x2;
			}
			else
			{
				x = x2; y = y2; xe = x1;
			}

			Draw(x, y, val, hue, sat);

			for (i = 0; x < xe; i++)
			{
				x = x + 1;
				if (px < 0) {
					px = px + 2 * dy1;
				}
				else
				{
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
						y = y + 1;
					}
					else {
						y = y - 1;
					}
					px = px + 2 * (dy1 - dx1);
				}
				Draw(x, y, val, hue, sat);
			}
		}
		else
		{
			if (dy >= 0)
			{
				x = x1; y = y1; ye = y2;
			}
			else
			{
				x = x2; y = y2; ye = y1;
			}

			Draw(x, y, val, hue, sat);

			for (i = 0; y < ye; i++)
			{
				y = y + 1;
				if (py <= 0) {
					py = py + 2 * dx1;
				}
				else
				{
					if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0)) {
						x = x + 1;
					}
					else {
						x = x - 1;
					}
					py = py + 2 * (dx1 - dy1);
				}
				Draw(x, y, val, hue, sat);
			}
		}
	}

	void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, uchar val = 255, uchar hue = 0, uchar sat = 0) // RYSOWANIE KRAWÊDZI TRÓJK¥TA MIÊDZY TRZEMA WIERZCHO£KAMI
	{
		DrawLine(x1, y1, x2, y2, val, hue, sat);
		DrawLine(x2, y2, x3, y3, val, hue, sat);
		DrawLine(x3, y3, x1, y1, val, hue, sat);
	}

	void FillTriangle(float fx1, float fy1, float fx2, float fy2, float fx3, float fy3, uchar val = 255, uchar hue = 0, uchar sat = 0) // WYPE£NIENIE TRÓJK¥TA MIÊDZY TRZEMA WIERZCHO£KAMI
	{
		int x1 = fx1;
		int x2 = fx2;
		int x3 = fx3;
		int y1 = fy1;
		int y2 = fy2;
		int y3 = fy3;

		auto SWAP = [](int& x, int& y) { int t = x; x = y; y = t; };
		auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) Draw(i, ny, val, hue, sat); };

		int t1x, t2x, y, minx, maxx, t1xp, t2xp;
		bool changed1 = false;
		bool changed2 = false;
		int signx1, signx2, dx1, dy1, dx2, dy2;
		int e1, e2;
		// Sortowanie trzech wierzcho³ków wg wartoœæi y
		if (y1 > y2) {
			SWAP(y1, y2);
			SWAP(x1, x2);
		}
		if (y1 > y3) {
			SWAP(y1, y3);
			SWAP(x1, x3);
		}
		if (y2 > y3) {
			SWAP(y2, y3);
			SWAP(x2, x3);
		}

		t1x = t2x = x1; y = y1;   // Punkty startowe
		dx1 = (int)(x2 - x1);

		if (dx1 < 0) {
			dx1 = -dx1;
			signx1 = -1;
		}
		else {
			signx1 = 1;
		}

		dy1 = (int)(y2 - y1);
		dx2 = (int)(x3 - x1);

		if (dx2 < 0) {
			dx2 = -dx2;
			signx2 = -1;
		}
		else {
			signx2 = 1;
		}

		dy2 = (int)(y3 - y1);

		if (dy1 > dx1) {
			SWAP(dx1, dy1);
			changed1 = true;
		}
		if (dy2 > dx2) {
			SWAP(dy2, dx2);
			changed2 = true;
		}

		e2 = (int)(dx2 >> 1);

		// Górna krawêdŸ p³aska, rysuj tylko drug¹ czêœæ
		if (y1 == y2) {
			goto next;
		}

		e1 = (int)(dx1 >> 1);

		for (int i = 0; i < dx1;) {
			t1xp = 0;
			t2xp = 0;
			if (t1x < t2x) {
				minx = t1x;
				maxx = t2x;
			}
			else {
				minx = t2x;
				maxx = t1x;
			}
			// pierwsza linia a¿ do zmiany y
			while (i < dx1) {
				i++;
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) {
						t1xp = signx1;
					}
					else {
						goto next1;
					}
				}
				if (changed1) {
					break;
				}
				else {
					t1x += signx1;
				}
			}
		next1:
			// druga linia a¿ do zmiany y
			while (1) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) {
						t2xp = signx2;
					}
					else {
						goto next2;
					}
				}
				if (changed2) {
					break;
				}
				else {
					t2x += signx2;
				}
			}
		next2:
			if (minx > t1x) {
				minx = t1x;
			}
			if (minx > t2x) {
				minx = t2x;
			}
			if (maxx < t1x) {
				maxx = t1x;
			}
			if (maxx < t2x) {
				maxx = t2x;
			}

			drawline(minx, maxx, y);    // Linia miêdzy wartoœciami min max w y
										// Zwiêkszenie y
			if (!changed1) {
				t1x += signx1;
			}

			t1x += t1xp;

			if (!changed2) {
				t2x += signx2;
			}

			t2x += t2xp;
			y += 1;

			if (y == y2) {
				break;
			}
		}
	next:
		// Druga czêœæ
		dx1 = (int)(x3 - x2);
		if (dx1 < 0) {
			dx1 = -dx1;
			signx1 = -1;;
		}
		else {
			signx1 = 1;
		}

		dy1 = (int)(y3 - y2);
		t1x = x2;

		if (dy1 > dx1) {
			SWAP(dy1, dx1);
			changed1 = true;
		}
		else {
			changed1 = false;
		}

		e1 = (int)(dx1 >> 1);

		for (int i = 0; i <= dx1; i++) {
			t1xp = 0;
			t2xp = 0;
			if (t1x < t2x) {
				minx = t1x;
				maxx = t2x;
			}
			else {
				minx = t2x;
				maxx = t1x;
			}
			// pierwsza linia a¿ do zmiany y
			while (i < dx1) {
				e1 += dy1;
				while (e1 >= dx1) {
					e1 -= dx1;
					if (changed1) {
						t1xp = signx1;
						break;
					}
					else {
						goto next3;
					}
				}
				if (changed1) {
					break;
				}
				else {
					t1x += signx1;
				}
				if (i < dx1) {
					i++;
				}
			}
		next3:
			// druga linia a¿ do zmiany y
			while (t2x != x3) {
				e2 += dy2;
				while (e2 >= dx2) {
					e2 -= dx2;
					if (changed2) {
						t2xp = signx2;
					}
					else {
						goto next4;
					}
				}
				if (changed2) {
					break;
				}
				else {
					t2x += signx2;
				}
			}
		next4:

			if (minx > t1x) {
				minx = t1x;
			}
			if (minx > t2x) {
				minx = t2x;
			}
			if (maxx < t1x) {
				maxx = t1x;
			}
			if (maxx < t2x) {
				maxx = t2x;
			}

			drawline(minx, maxx, y);

			if (!changed1) {
				t1x += signx1;
			}
			t1x += t1xp;
			if (!changed2) {
				t2x += signx2;
			}
			t2x += t2xp;
			y += 1;
			if (y > y3) {
				return;
			}
		}
	}

public:
	void Start()
	{
		bAtomActive = true; // URUCHOMIENIE W¥TKU
		std::thread t = std::thread(&Animation3DEngine::RunApp, this);
		t.join(); // WSTRZYMANIE FUNKCJI DO CZASU ZAKOÑCZENIA W¥TKU
	}

private:
	void RunApp()
	{
		if (!LoadApp(frameAspectRatio)) { // ZA£ADOWANIE DANYCH DO PROGRAMU
			bAtomActive = false;
		}
		VideoWriter video("OutputAnimation.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, Size(frameWidth, frameHeight), true);

		auto tp1 = std::chrono::system_clock::now();
		auto tp2 = std::chrono::system_clock::now();

		Mat window(screenHeight, screenWidth, CV_8UC3, Scalar(0, 0, 0));
		Mat windowRGB(screenHeight, screenWidth, CV_8UC3, Scalar(0, 0, 0));

		Mat frame(frameHeight, frameWidth, CV_8UC3, Scalar(0, 0, 0));
		Mat frameRGB(frameHeight, frameWidth, CV_8UC3, Scalar(0, 0, 0));

		namedWindow("Scene sequence", 0);
		resizeWindow("Scene sequence", screenWidth, screenHeight);

		while (bAtomActive) {
			while (bAtomActive) {

				tp2 = std::chrono::system_clock::now(); // LICZENIE CZASU
				std::chrono::duration<float> loopTime = tp2 - tp1;
				tp1 = tp2;
				float fLoopTime = loopTime.count();

				for (int i = 0; i < 256; i++) { // USTALENIE STANÓW LOGICZNYCH KLAWISZY KLAWIATURY 
					m_keyNewState[i] = GetAsyncKeyState(i);
					m_keys[i].bPressed = false;
					m_keys[i].bReleased = false;

					if (m_keyNewState[i] != m_keyOldState[i])
					{
						if (m_keyNewState[i] & 0x8000)
						{
							m_keys[i].bPressed = !m_keys[i].bHeld;
							m_keys[i].bHeld = true;
						}
						else
						{
							m_keys[i].bReleased = true;
							m_keys[i].bHeld = false;
						}
					}
					m_keyOldState[i] = m_keyNewState[i];
				}

				if (!UpdateApp(fLoopTime, toExport, fps, frameAspectRatio)) { // ZMIANY OBIEKTÓW W PROGRAMIE, AKTUALIZACJA STANU
					bAtomActive = false;
					destroyAllWindows();
				}

				for (int i = 0; i < screenWidth; i++) {
					for (int j = 0; j < screenHeight; j++) {
						window.at<Vec3b>(j, i)[0] = ScreenBuffer[j * screenWidth + i].hue;
						window.at<Vec3b>(j, i)[1] = ScreenBuffer[j * screenWidth + i].sat;
						window.at<Vec3b>(j, i)[2] = ScreenBuffer[j * screenWidth + i].val;
					}
				}

				cvtColor(window, windowRGB, COLOR_HSV2BGR, 0);

				imshow("Scene sequence", windowRGB);
				if (pollKey() > -1) {
					waitKey(1);
				}


				if (toExport == 1) {
					for (int i = 0; i < frameWidth; i++) {
						for (int j = 0; j < frameHeight; j++) {
							frame.at<Vec3b>(j, i)[0] = RenderBUffer[j * frameWidth + i].hue;
							frame.at<Vec3b>(j, i)[1] = RenderBUffer[j * frameWidth + i].sat;
							frame.at<Vec3b>(j, i)[2] = RenderBUffer[j * frameWidth + i].val;
						}
					}

					cvtColor(frame, frameRGB, COLOR_HSV2BGR, 0);
					video.write(frameRGB);

					namedWindow("Display window", 0);
					resizeWindow("Display window", frameWidth, frameHeight);
					imshow("Display window", frameRGB);
					waitKey(1);
				}

				if (toExport == 2) {
					destroyWindow("Display window");
					video.release();
					toExport = 0;
				}

				wchar_t s[256];
				swprintf_s(s, 256, L"3D Animation Engine - %s - FPS: %3.2f", m_sAppName.c_str(), 1.0f / fLoopTime);
				SetConsoleTitle(s); // WPISANIE TYTU£U KONSOLI Z LICZNIKIEM FPS

				if (toExport != 0) {
					Fill(0, 0, frameWidth, frameHeight, 0); // CZYSZCZENIE EKRANU KONSOLI
				}
			}
		}
	}

public:
	virtual bool LoadApp(float frameAspectRatio) = 0; // FUNKCJE DO PRZECI¥¯ENIA W .CPP
	virtual bool UpdateApp(float fElapsedTime, int& toExport, int fps, float frameAspectRatio) = 0;

protected:
	struct sKeyState
	{
		bool bPressed;
		bool bReleased;
		bool bHeld;
	} m_keys[256]; // TABLICA ZE STANAMI KLAWISZY KLAWIATURY

public:
	sKeyState GetKey(int nKeyID) { // FUNKCJA ZWRACAJ¥CA STAN DANEGO KLAWISZA KLAWIATURY
		return m_keys[nKeyID];
	}

protected:
	int fps = 30;
	float frameAspectRatio = 16.0f / 9.0f;
	int frameWidth = 3840;
	int frameHeight = frameWidth / frameAspectRatio;
	int screenWidth = 1200;
	int screenHeight = 1000;
	pixHSV* ScreenBuffer;
	pixHSV* RenderBUffer;
	int toExport = 0;
	std::wstring m_sAppName;
	short m_keyOldState[256] = { 0 };
	short m_keyNewState[256] = { 0 };
	static std::atomic<bool> bAtomActive;
};

std::atomic<bool> Animation3DEngine::bAtomActive(false);
