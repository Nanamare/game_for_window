#include <windows.h>
#include <mmsystem.h>
#include <time.h>
#include "resource.h"

#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "winmm.lib")

#define BEGIN_TIMER		1
#define KEYSTATE_TIMER	2
#define SHOT_TIMER		3

#define SHOT_MAX		7

#define CREATE_ENEMY	1
#define EXIST_ENEMY		1
#define HIT_ENEMY		2
#define DESTROY_ENEMY	0

#define CREATE_ENSHOT	1
#define EXIST_ENSHOT	1
#define DESTROY_ENSHOT	0

int i = 0;
int toggle = 0;

struct D_ENEMY {
	INT_PTR Pos_x;
	INT_PTR Pos_y;
	INT_PTR Shot_x;
	INT_PTR Shot_y;
	INT_PTR nCntShot;
	INT_PTR nCnt_Hit;
	INT_PTR nCntLocMove;
	INT_PTR nCntMove;
	BOOL bFlag_life;
	BOOL bFlag_Shot;
} dEnemy[20];

BOOL bFlag_Death;						// 파이터 사망 여부

INT_PTR nCntEnemy    = 5;
INT_PTR nEnPosSpeed	 = 1;
INT_PTR nEnShotSpeed = 2;

INT_PTR nPos_X=20, nPos_Y=20;

HDC hdc, BkMemDC, BitMemDC;

PAINTSTRUCT ps;

INT_PTR nBGND_x;

UINT wDeviceID = 0;

HFONT hFont, hOldFont;
INT_PTR nHitPoint;
CHAR arHitPoint[7];

BOOL bFlag_Shot[SHOT_MAX];
INT_PTR nCntShot;
INT_PTR nShot_x[SHOT_MAX], nShot_y[SHOT_MAX];

BOOL bBkGround;

HBITMAP hBgBitmap_1, hOldBgBitmap_1;	// 배경1
HBITMAP hBgBitmap_2, hOldBgBitmap_2;	// 배경2
HBITMAP hFighterBit, hOldFighterBit;	// 아군
HBITMAP hEnemyBit, hOldEnemyBit;		// 적군
HBITMAP hShotBit, hOldShotBit;			// 아군 총알
HBITMAP hEnShotBit, hOldEnShotBit;		// 적군 총알
HBITMAP hEnDieBit, hOldEnDieBit;		// 적군 폭발
HBITMAP hTempBit, hOldTempBit;
BITMAP ClientWork;

DWORD PlayWaveFile(HWND, LPSTR);

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT OnCreate(HWND, WPARAM, LPARAM);
LRESULT OnKeyDown(HWND, WPARAM, LPARAM);
VOID CALLBACK TimerProc(HWND, UINT, UINT, DWORD);
LRESULT OnPaint(HWND, WPARAM, LPARAM);
LRESULT OnDestroy(HWND, WPARAM, LPARAM);
LRESULT OnMCInotify(HWND, WPARAM, LPARAM);     

VOID DisplayBackGround(VOID);
VOID DisplayFighter(VOID);
VOID ControlShot(HWND hWnd);
VOID DisplayEnemy(HWND hWnd);
VOID DisplayEnShot(HWND hWnd);
VOID DisplayEnHit(VOID);
VOID DisplayHitPoint(VOID);
VOID CheckStage(VOID);

HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("201001596_신현성");

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 250, 100, 740, 459, NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	while(GetMessage(&Message, 0, 0, 0)){
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch(iMessage){
	case WM_CREATE:
		OnCreate(hWnd, wParam, lParam);
		return 0;
	case WM_KEYDOWN:
		OnKeyDown(hWnd, wParam, lParam);
		return 0;
	case WM_PAINT:
		OnPaint(hWnd, wParam, lParam);
		return 0;
	case MM_MCINOTIFY:
		OnMCInotify(hWnd, wParam, lParam);
		return 0;
	case WM_ERASEBKGND:
		return 0;
	case WM_DESTROY:
		OnDestroy(hWnd, wParam, lParam);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	hBgBitmap_1 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BGROUND1));
	hBgBitmap_2 = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BGROUND2));
	hFighterBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_FIGHTER));
	hShotBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_SHOT));
	hEnemyBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_ENEMY));
	hEnShotBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_ENSHOT));
	hEnDieBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_ENDIE));

	GetObject(hBgBitmap_1, sizeof(BITMAP), &ClientWork);

	hFont = CreateFont(15, 0, 0, 0, FW_BOLD, 0, 0, 0, HANGEUL_CHARSET, 0, 0, 0, 0, TEXT("돋움"));

	SetTimer(hWnd, BEGIN_TIMER, 10, TimerProc);
	SetTimer(hWnd, KEYSTATE_TIMER, 1, TimerProc);

	srand((unsigned int)time(NULL));

	return 0;
}

LRESULT OnKeyDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if(LOWORD(wParam) == VK_SPACE){
		PlaySound(MAKEINTRESOURCE(IDR_WFIRE), g_hInst, SND_RESOURCE | SND_ASYNC);
		if (nCntShot < SHOT_MAX){
			if (nCntShot == 0){
				SetTimer(hWnd, SHOT_TIMER, 5, TimerProc);
			}
			nShot_x[nCntShot] = nPos_X + 90;
			nShot_y[nCntShot] = nPos_Y;
			bFlag_Shot[nCntShot] = TRUE;
			nCntShot++;
		}

	}
	if (wParam == VK_ESCAPE){
		if (!wDeviceID&&!toggle){
			PlayWaveFile(hWnd, "Music\\Thema.wav");
			toggle++;
		}
		else if (toggle){
			mciSendCommand(wDeviceID, MCI_CLOSE, 0, (DWORD)NULL);
			toggle--;
			wDeviceID = false;
		}
	}
	return 0;
}

VOID CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	switch(idEvent){

		case BEGIN_TIMER:
		nBGND_x++;

		hdc = GetDC(hWnd);

		if(hTempBit){
			DeleteObject(hTempBit);
		}
		hTempBit = CreateCompatibleBitmap(hdc, ClientWork.bmWidth, ClientWork.bmHeight);

		BkMemDC = CreateCompatibleDC(hdc);
		BitMemDC = CreateCompatibleDC(hdc);

		// Z Order

		hOldTempBit  = (HBITMAP)SelectObject(BkMemDC, hTempBit);

		DisplayBackGround();

		DisplayEnHit();

		DisplayFighter();

		DisplayEnemy(hWnd);

		DisplayEnShot(hWnd);

		ControlShot(hWnd);

		DisplayHitPoint();

		ReleaseDC(hWnd, hdc);

		break;

	case KEYSTATE_TIMER:
		if(GetKeyState(VK_UP) & 0x8000){
			if(nPos_Y > 2){
				nPos_Y -= 6;
			}
		}
		if(GetKeyState(VK_LEFT) & 0x8000){
			if(nPos_X > 2){
				nPos_X -= 6;
			}
		}
		if(GetKeyState(VK_RIGHT) & 0x8000){
			if(nPos_X < 573){
				nPos_X += 6;
			}
		}
		if(GetKeyState(VK_DOWN) & 0x8000){
			if(nPos_Y < 373){
				nPos_Y += 6;
			}
		}
		break;
	case SHOT_TIMER:
		for(int i=0; i < nCntShot; i++){
			nShot_x[i] += 9;
		}
		break;
	}
	InvalidateRect(hWnd, NULL, FALSE);
}

VOID DisplayBackGround(VOID)
{
	if(bBkGround == FALSE){
		hOldBgBitmap_2 = (HBITMAP)SelectObject(BitMemDC, hBgBitmap_2);
		StretchBlt(BkMemDC, ClientWork.bmWidth-nBGND_x, 0, nBGND_x, ClientWork.bmHeight, BitMemDC, 0, 0, nBGND_x, ClientWork.bmHeight, SRCCOPY);
		SelectObject(BitMemDC, hOldBgBitmap_2);
		if(nBGND_x < ClientWork.bmWidth){
			hOldBgBitmap_1 = (HBITMAP)SelectObject(BitMemDC, hBgBitmap_1);
			StretchBlt(BkMemDC, 0, 0, ClientWork.bmWidth-nBGND_x, ClientWork.bmHeight, BitMemDC, nBGND_x, 0, ClientWork.bmWidth-nBGND_x, ClientWork.bmHeight, SRCCOPY);
			SelectObject(BitMemDC, hOldBgBitmap_1);
		}
		else{
			nBGND_x = 0;
			bBkGround = TRUE;
		}
	}
	else{
		hOldBgBitmap_1 = (HBITMAP)SelectObject(BitMemDC, hBgBitmap_1);
		StretchBlt(BkMemDC, ClientWork.bmWidth-nBGND_x, 0, nBGND_x, ClientWork.bmHeight, BitMemDC, 0, 0, nBGND_x, ClientWork.bmHeight, SRCCOPY);
		SelectObject(BitMemDC, hOldBgBitmap_1);
		if(nBGND_x < ClientWork.bmWidth){
			hOldBgBitmap_2 = (HBITMAP)SelectObject(BitMemDC, hBgBitmap_2);
			StretchBlt(BkMemDC, 0, 0, ClientWork.bmWidth-nBGND_x, ClientWork.bmHeight, BitMemDC, nBGND_x, 0, ClientWork.bmWidth-nBGND_x, ClientWork.bmHeight, SRCCOPY);
			SelectObject(BitMemDC, hOldBgBitmap_2);
		}
		else{
			nBGND_x = 0;
			bBkGround = FALSE;
		}
	}
}

VOID DisplayFighter(VOID)
{
	hOldFighterBit = (HBITMAP)SelectObject(BitMemDC, hFighterBit);

	TransparentBlt(BkMemDC, nPos_X, nPos_Y, 160, 62, BitMemDC, 0, 0, 160, 62, RGB(255, 255, 255));

	SelectObject(BitMemDC, hOldFighterBit);
}

VOID ControlShot(HWND hWnd)
{
	for(int i=0; i < nCntShot; i++){	// 적군과 총알 충돌 판정 검사
		for(int j=0; j < nCntEnemy; j++){
			if(((dEnemy[j].Pos_x - nShot_x[i]) < 10) && ((dEnemy[j].Pos_x - nShot_x[i]) > -30)){
				if(((dEnemy[j].Pos_y - nShot_y[i]) < 25) && ((dEnemy[j].Pos_y - nShot_y[i]) > -25)){
					if(dEnemy[j].bFlag_life != HIT_ENEMY){
						dEnemy[j].bFlag_life = HIT_ENEMY;
						dEnemy[j].nCnt_Hit = 100;
						nShot_x[i] = 801;
						nHitPoint += 10;
						CheckStage();
					}
				}
			}
		}
	}

	if(nCntShot != 0){
		hOldShotBit = (HBITMAP)SelectObject(BitMemDC, hShotBit);
	}

	for(i=0; i < nCntShot; i++){
		if(bFlag_Shot[i] == TRUE){
			if(nShot_x[i] < 800){
				TransparentBlt(BkMemDC, nShot_x[i], nShot_y[i], 30, 55, BitMemDC, 0, 0, 30, 55, RGB(255, 255, 255));
			}
			else{
				bFlag_Shot[i] = FALSE;
				for(int j=i; j < nCntShot-1; j++){
					bFlag_Shot[j] = bFlag_Shot[j+1];
					nShot_x[j] = nShot_x[j+1];
					nShot_y[j] = nShot_y[j+1];
				}
				nCntShot--;
			}
		}
	}
	SelectObject(BitMemDC, hOldShotBit);

	if(nCntShot == 0){
		KillTimer(hWnd, SHOT_TIMER);
	}
}

VOID DisplayEnemy(HWND hWnd)
{	
	for(int i=0; i < nCntEnemy; i++){
		if(dEnemy[i].Pos_x < -50){
			dEnemy[i].bFlag_life = DESTROY_ENEMY;
			dEnemy[i].Pos_x = 0;
			dEnemy[i].Pos_y = 0;
			dEnemy[i].bFlag_Shot = DESTROY_ENSHOT;
		}
	}

	for(i=0; i < nCntEnemy; i++){
		if(dEnemy[i].bFlag_life == 0){
			dEnemy[i].bFlag_life = CREATE_ENEMY;
			dEnemy[i].Pos_x = (rand() % 350) + 755;
			dEnemy[i].Pos_y = (rand() % 450);
		}
	}

	for(i=0; i < nCntEnemy; i++){
		if(((nPos_X - dEnemy[i].Pos_x) < 20) && ((nPos_X - dEnemy[i].Pos_x) > -130)){
			if(((nPos_Y - dEnemy[i].Pos_y) < 15) && ((nPos_Y - dEnemy[i].Pos_y) > -15)){
 				if(dEnemy[i].bFlag_life != HIT_ENEMY){
					bFlag_Death = TRUE;
					SendMessage(hWnd, WM_DESTROY, 0, 0);
					MessageBox(hWnd, TEXT("게임 종료"), TEXT("Game Over"), MB_OK);
				}
			}
		}
	}

	for(i=0; i < nCntEnemy; i++){
		switch(dEnemy[i].bFlag_life){
		case EXIST_ENEMY:
			dEnemy[i].Pos_x -= nEnPosSpeed;
			hOldEnemyBit = (HBITMAP)SelectObject(BitMemDC, hEnemyBit);
			TransparentBlt(BkMemDC, dEnemy[i].Pos_x, dEnemy[i].Pos_y, 57, 50, BitMemDC, 0, 0, 57, 50, RGB(255, 255, 255));
			SelectObject(BitMemDC, hOldEnemyBit);

			break;
			
		case HIT_ENEMY:
			if(dEnemy[i].nCnt_Hit > 0){
				dEnemy[i].Pos_x -= 1;
				dEnemy[i].nCnt_Hit--;
			}
			else{
				dEnemy[i].bFlag_life = DESTROY_ENEMY;
				dEnemy[i].Pos_x = 0;
				dEnemy[i].Pos_y = 0;
			}
			break;
		}
	}

	for(i=0; i < nCntEnemy; i++){
		if(dEnemy[i].bFlag_life != HIT_ENEMY){
			if(dEnemy[i].Pos_y < 0){
				dEnemy[i].nCntLocMove = 1;
				dEnemy[i].nCntMove = 60;
			}
			if(dEnemy[i].Pos_y > 380){
				dEnemy[i].nCntLocMove = -1;
				dEnemy[i].nCntMove = 60;
			}

			if(dEnemy[i].nCntMove == 0){
				switch(rand() % 2){
				case 0:
					dEnemy[i].nCntLocMove = -1;
					break;
				case 1:
					dEnemy[i].nCntLocMove = 1;
					break;
				}
				dEnemy[i].nCntMove = 60;
			}
			else{
				dEnemy[i].Pos_y += dEnemy[i].nCntLocMove;
				dEnemy[i].nCntMove--;
			}
		}
	}
}

VOID DisplayEnHit(VOID)
{
	for(int i=0; i < nCntEnemy; i++){
		if(dEnemy[i].bFlag_life == HIT_ENEMY){
			hOldEnDieBit = (HBITMAP)SelectObject(BitMemDC, hEnDieBit);

			TransparentBlt(BkMemDC, dEnemy[i].Pos_x, dEnemy[i].Pos_y, 57, 49, BitMemDC, 0, 0, 57, 49, RGB(255, 255, 255));

			SelectObject(BitMemDC, hOldEnDieBit);
		}
	}
}

VOID DisplayEnShot(HWND hWnd)	// 적군 총알 처리 루틴
{
	for(int i=0; i < nCntEnemy; i++){
		if(dEnemy[i].nCntShot < 0){						// 총알 파괴
			dEnemy[i].bFlag_Shot = DESTROY_ENSHOT;
		}

		if (((nPos_X - dEnemy[i].Shot_x) < -10) && ((nPos_X - dEnemy[i].Shot_x) > -115)){		// 아군과 총알의 충돌 판정 검사
			if(((nPos_Y - dEnemy[i].Shot_y) < -5) && ((nPos_Y - dEnemy[i].Shot_y) > -30)){
				if((dEnemy[i].bFlag_life != HIT_ENEMY) && (dEnemy[i].bFlag_Shot != DESTROY_ENSHOT)){
   					bFlag_Death = TRUE;
					SendMessage(hWnd, WM_DESTROY, 0, 0);
					MessageBox(hWnd, TEXT("게임 종료"), TEXT("Game Over"), MB_OK);
				}
			}
		}
	}

	for(i=0; i < nCntEnemy; i++){
		if(dEnemy[i].bFlag_Shot == EXIST_ENSHOT){		// 총알 이동
			dEnemy[i].Shot_x -= nEnShotSpeed;
			dEnemy[i].nCntShot -= 1;
			hOldEnShotBit = (HBITMAP)SelectObject(BitMemDC, hEnShotBit);
			TransparentBlt(BkMemDC, dEnemy[i].Shot_x, dEnemy[i].Shot_y, 22, 18, BitMemDC, 0, 0, 22, 18, RGB(255, 255, 255));
			SelectObject(BitMemDC, hOldEnShotBit);
		}
	}

	for(i=0; i < nCntEnemy; i++){
		if(dEnemy[i].bFlag_life == EXIST_ENEMY){		// 총알 생성
			if(dEnemy[i].bFlag_Shot != EXIST_ENSHOT){
				if(dEnemy[i].Pos_x < 700){
					dEnemy[i].Shot_x = dEnemy[i].Pos_x - 25;
					dEnemy[i].Shot_y = dEnemy[i].Pos_y + 15;
					dEnemy[i].nCntShot = 300;
					dEnemy[i].bFlag_Shot = CREATE_ENSHOT;
				}
			}
		}
	}
}

VOID DisplayHitPoint(VOID)
{
	hOldFont = (HFONT)SelectObject(BkMemDC, hFont);
	SetTextColor(BkMemDC, RGB(255, 255, 0));
	SetBkMode(BkMemDC, TRANSPARENT);
	wsprintf(arHitPoint, "현재 점수 %d", nHitPoint);
	SetTextAlign(BkMemDC, TA_RIGHT);
	TextOut(BkMemDC, 110, 5, TEXT(arHitPoint), lstrlen(arHitPoint));
	TextOut(BkMemDC, 210, 30, "배경음악 시작/제거 ESC", 24);
	SelectObject(BkMemDC, hOldFont);
}

LRESULT OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	hdc = BeginPaint(hWnd, &ps);

	StretchBlt(hdc, 0, 0, ClientWork.bmWidth, ClientWork.bmHeight, BkMemDC, 0, 0, ClientWork.bmWidth, ClientWork.bmHeight, SRCCOPY);

	DeleteDC(BitMemDC);
	DeleteDC(BkMemDC);

	EndPaint(hWnd, &ps);

	return 0;
}

VOID CheckStage(VOID)
{
	if(nHitPoint != 0){
 		if((nHitPoint % 100) == 0){
			nCntEnemy++;
		}
		if((nHitPoint % 100) == 0){
			nEnPosSpeed++;
			nEnShotSpeed++;
		}
	}
}

LRESULT OnMCInotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(wParam){
	case MCI_NOTIFY_SUCCESSFUL:
		mciSendCommand(LOWORD(lParam), MCI_CLOSE, 0, (DWORD)NULL);
		PlayWaveFile(hWnd, "Music\\Thema.wav");
		break;
	}
	return 0;
}

DWORD PlayWaveFile(HWND hWndNotify, LPSTR lpszWave)
{
	DWORD Result;
	MCI_OPEN_PARMS mciOpenParms;
	MCI_PLAY_PARMS mciPlayParms;

	// 장치를 Open하고 ID를 발급받는다.
	mciOpenParms.lpstrDeviceType = "WaveAudio";
	mciOpenParms.lpstrElementName = lpszWave;

	Result = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)(LPVOID)&mciOpenParms);

	if(Result){
		return Result;
	}

	wDeviceID = mciOpenParms.wDeviceID;

	mciPlayParms.dwCallback = (DWORD)hWndNotify;
	Result = mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID)&mciPlayParms);

	if(Result){
		mciSendCommand(wDeviceID, MCI_CLOSE, 0, (DWORD)NULL);
		return Result;
	}
	return 0;
}

LRESULT OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	KillTimer(hWnd, BEGIN_TIMER);
	KillTimer(hWnd, KEYSTATE_TIMER);
	KillTimer(hWnd, SHOT_TIMER);

	DeleteObject(hBgBitmap_1);
	DeleteObject(hBgBitmap_2);
	DeleteObject(hFighterBit);
	DeleteObject(hShotBit);
	DeleteObject(hEnemyBit);
	DeleteObject(hEnShotBit);
	DeleteObject(hEnDieBit);

	if(wDeviceID){
		mciSendCommand(wDeviceID, MCI_CLOSE, 0, (DWORD)NULL);
	}

	if(bFlag_Death != TRUE){
		PostQuitMessage(0);
	}
	bFlag_Death = FALSE;

	return 0;
}