#include <Windows.h>
#include <strsafe.h>
#include <windowsx.h>
#include <time.h>

#define HEIGHT 15
#define WIDTH 15
#define MAX_STRING 50

#define LAND L'○'
#define MINE L'●'
#define SAFE L'　'
#define FLAG L'⊙'
#define UNKNOWN L'？'

TCHAR szMatrix[MAX_STRING][MAX_STRING];                //扫雷游戏区域
size_t nSide;                        //区域边长
BOOL bMine[MAX_STRING][MAX_STRING];        //记录雷的位置
UINT uMine[MAX_STRING][MAX_STRING];        //记录周围雷的个数
int nMine;        //记录雷的个数

LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR szCmdLine,INT nCmdShow)
{
        static TCHAR szClassName[] = TEXT("MyClass");
        WNDCLASS wndclass;
        HWND hwnd;
        MSG msg;

        wndclass.cbClsExtra = 0;
        wndclass.cbWndExtra = 0;
        wndclass.hbrBackground = (HBRUSH)GetStockObject(DC_BRUSH);
        wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
        wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
        wndclass.hInstance = hInstance;
        wndclass.lpfnWndProc = WindowProc;
        wndclass.lpszClassName = szClassName;
        wndclass.lpszMenuName = NULL;
        wndclass.style = CS_HREDRAW | CS_VREDRAW;
        
        if (!RegisterClass(&wndclass))
        {
                MessageBox(NULL,TEXT("本程序必须在WINNT下运行！"),TEXT("警告"),MB_OK);
                return 0;
        }

        nSide = 20;
        nMine = 40;
        for (int i=0;i<nSide;i++)
        {
                for (int j=0;j<nSide;j++)
                {
                        szMatrix[i][j] = LAND;
                }
        }

        srand((unsigned)time(NULL));
        //下面随机出现nMine颗雷
        for (int i=0;i<nMine;i++)
        {
                int x = rand()%nSide;
                int y = rand()%nSide;
                if (!bMine[y][x])        //当前位置还没雷
                {
                        bMine[y][x] = TRUE;
                }
                else
                {
                        i--;
                }
        }
        //下面计算每一块周围有多少颗雷
        for (int i=0;i<nSide;i++)
        {
                for (int j=0;j<nSide;j++)
                {
                        if (bMine[i][j])        continue;                //当前块是雷
                        int nCounts = 0;
                        for (int m=i-1;m<=i+1;m++)
                        {
                                if (m<0 || m>=nSide)        continue;
                                for (int n=j-1;n<=j+1;n++)
                                {
                                        if (n<0 || n>=nSide)        continue;
                                        if (bMine[m][n])        nCounts++;
                                }
                        }
                        //存储雷数nCounts
                        uMine[i][j] = nCounts;
                }
        }

        hwnd = CreateWindow(szClassName,
                                                TEXT("BY:落叶"),
                                                WS_OVERLAPPEDWINDOW,
                                                CW_USEDEFAULT,
                                                CW_USEDEFAULT,
                                                CW_USEDEFAULT,
                                                CW_USEDEFAULT,
                                                NULL,
                                                NULL,
                                                hInstance,
                                                NULL);

        ShowWindow(hwnd,nCmdShow);
        UpdateWindow(hwnd);

        while (GetMessage(&msg,NULL,0,0))
        {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }

        return msg.wParam;
}

void SwipeLand(HDC hdc,int x,int y,BOOL bRecord[MAX_STRING][MAX_STRING]);        //计算并现出没雷的连续区域

LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
        HDC hdc;
        PAINTSTRUCT ps;
        RECT rect;

        int i,j;        //循环变量
        int xPos,yPos;        //记录鼠标位置
        int x,y;        //记录地雷下标

        switch (uMsg)
        {
                case WM_PAINT:
                        hdc = BeginPaint(hwnd,&ps);
                        //绘制扫雷区域
                        for (i=0;i<nSide;i++)
                        {
                                for (j=0;j<nSide;j++)
                                {
                                        TextOut(hdc,j*WIDTH,i*HEIGHT,&szMatrix[i][j],1);
                                }
                        }
                        EndPaint(hwnd,&ps);
                        break;
                case WM_LBUTTONUP:
                        xPos = GET_X_LPARAM(lParam);
                        yPos = GET_Y_LPARAM(lParam);
                        x = xPos/WIDTH;
                        y = yPos/HEIGHT;
                        if (x < nSide && y < nSide)
                        {
                                hdc = GetDC(hwnd);
                                //判断是否有雷
                                if (bMine[y][x])
                                {
                                        //先显示雷的位置然后退出
                                        for (i=0;i<nSide;i++)
                                                for (j=0;j<nSide;j++)
                                                        if (bMine[i][j])
                                                        {
                                                                TextOut(hdc,j*WIDTH,i*HEIGHT,TEXT("●"),1);
                                                                szMatrix[i][j] = MINE;
                                                        }
                                        MessageBox(hwnd,TEXT("游戏结束！"),TEXT("提示"),MB_OK);
                                        ReleaseDC(hwnd,hdc);
                                        DestroyWindow(hwnd);
                                        break;
                                }
                                BOOL bRecord[MAX_STRING][MAX_STRING] = {FALSE};        //记录是否已遍历
                                SwipeLand(hdc,x,y,bRecord);
                                ReleaseDC(hwnd,hdc);
                        }
                        break;
                case WM_RBUTTONUP:
                        xPos = GET_X_LPARAM(lParam);
                        yPos = GET_Y_LPARAM(lParam);
                        x = xPos/WIDTH;
                        y = yPos/HEIGHT;
                        if (x<nSide && y<nSide)
                        {
                                switch (szMatrix[y][x])
                                {
                                        case LAND:
                                                szMatrix[y][x] = FLAG;
                                                break;
                                        case FLAG:
                                                szMatrix[y][x] = UNKNOWN;
                                                break;
                                        case UNKNOWN:
                                                szMatrix[y][x] = LAND;
                                                break;
                                }
                                //画出图形
                                hdc = GetDC(hwnd);
                                TextOut(hdc,x*WIDTH,y*HEIGHT,&szMatrix[y][x],1);
                                ReleaseDC(hwnd,hdc);
                        }
                        break;
                case WM_CLOSE:
                        if (IDYES == MessageBox(hwnd,TEXT("确认退出扫雷？"),TEXT("提示"),MB_YESNO | MB_ICONINFORMATION))
                                DestroyWindow(hwnd);
                        break;
                case WM_DESTROY:
                        PostQuitMessage(0);
                        break;
                default:
                        return DefWindowProc(hwnd,uMsg,wParam,lParam);
        }

        return 0;
}

void SwipeLand(HDC hdc,int x,int y,BOOL bRecord[MAX_STRING][MAX_STRING])        //计算并现出没雷的连续区域
{
        bRecord[x][y] = TRUE;
        if (x<0 || x>=nSide || y<0 || y>=nSide)        return;
        static TCHAR szMine[5];        //为显示雷做准备
        //现出当前位置
        StringCchPrintf(szMine,5,TEXT("　"));
        TextOut(hdc,x*WIDTH,y*HEIGHT,szMine,1);
        szMatrix[y][x] = SAFE;
        if (uMine[y][x]>0)
        {
                StringCchPrintf(szMine,5,TEXT("%d"),uMine[y][x]);
                TextOut(hdc,x*WIDTH,y*HEIGHT,szMine,1);
                szMatrix[y][x] = szMine[0];
                return;
        }
        //现出上下左右4个方向
        if (!bRecord[x][y-1])        SwipeLand(hdc,x,y-1,bRecord);
        if (!bRecord[x][y+1])        SwipeLand(hdc,x,y+1,bRecord);
        if (!bRecord[x-1][y])        SwipeLand(hdc,x-1,y,bRecord);
        if (!bRecord[x+1][y])        SwipeLand(hdc,x+1,y,bRecord);
}
