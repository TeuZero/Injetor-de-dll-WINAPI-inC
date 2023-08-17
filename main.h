/*
    main.h
    C/C++
*/
#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <tlhelp32.h> 
#include <shlwapi.h> 

#define WIN32_LEAN_AND_MEAN 
#define CREATE_THREAD_ACCESS (PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ) 
#pragma comment(lib,"Advapi32.lib") 

using namespace std;

// Prototipos

void AddControls(HWND);
BOOL Inject(DWORD pID, const char * DLL_NAME); 
DWORD GetTargetThreadIDFromProcName(char ProcName[32]); 
int Unhook(DWORD pid);
void suspend(DWORD processId);
void resumeProc(DWORD processId);

typedef LONG (NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG (NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);

// arquivos do Injector-T0
#include "cdrs.h"

//var the program
char szFileName[MAX_PATH] = "";
char nameProc[32];

// WndProc() -> Processa as mensagens enviadas para o programa
LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {
    // VariÃ¡veis para manipulaÃ§Ã£ da parte grÃ¡fica do programa
    HDC hDC = NULL;
    PAINTSTRUCT psPaint;
    
    // Verifica qual foi a mensagem enviada 
    switch(Message) {
        case WM_CREATE: // Janela foi criada
        {
            AddControls(hWnd);

            // Retorna 0, significando que a mensagem foi processada corretamente
            return(0);
            break;
        }

        case WM_COMMAND:
        {
            if(LOWORD(wParam) == IDC_LOADLL)
            {
                //Open
                
                OPENFILENAME ofn;
                
                ZeroMemory(&ofn, sizeof(ofn));

                ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
                ofn.hwndOwner = hWnd;
                ofn.lpstrFilter = "Resource files (*.dll)\0*.dll\0All Files (*.*)\0*.*\0";
                ofn.lpstrFile = szFileName;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                ofn.lpstrDefExt = "dll";

                if(GetOpenFileName(&ofn))
                {
                    // Do something usefull with the filename stored in szFileName 
                }
                
                if(szFileName == ""){
                }else{
                    SetWindowText(hLblOutput,"Dll loaded! Ready to inject.");
                    EnableWindow(hInject,TRUE); 
                }
            }
            if(LOWORD(wParam) == IDC_INJECT)
            {
                
                //baixo
                GetWindowText(hTextInput,nameProc,sizeof(nameProc));
                
                // Retrieve process ID 
                DWORD pID = GetTargetThreadIDFromProcName(nameProc); 
                Unhook(pID);
                //suspend(pID);
                
                // Get the dll's full path name 
               char buf[MAX_PATH] = {0}; 
               GetFullPathName(szFileName, MAX_PATH,buf, NULL); 
        
        
                STARTUPINFO initInfo = { 0 };
                initInfo.cb = sizeof(initInfo);
                PROCESS_INFORMATION procInfo = { 0 };
                    
               if(!Inject(pID, buf)) 
               { 
            
                    MessageBox(0,"Dll not injected! =(","ERROR",0);
                }else{ 
                    MessageBox(0,"Dll successfully injected","OK",0);
                } 
                
                //suspend(pID)
                //resumeProc(pID);
                return 0;
            }
             
        }

        case WM_CTLCOLORLISTBOX:
        {
            HDC hdcListBox = (HDC) wParam;
            SetTextColor(hdcListBox, RGB(0,0,0));
            SetBkColor(hdcListBox, RGB(192,192,192));
            return (INT_PTR)CreateSolidBrush(RGB(192,192,192));
        }
        case WM_CTLCOLORSCROLLBAR:
        {
            HDC hdcScrollBar = (HDC) wParam;
            SetTextColor(hdcScrollBar, RGB(0,0,0));
            SetBkColor(hdcScrollBar, RGB(192,192,192));
            return (INT_PTR)CreateSolidBrush(RGB(192,192,192));
        }

        //Color caixa de texto static
        
        case WM_CTLCOLORSTATIC:
        {
            HDC hdcStatic = (HDC) wParam;
            SetTextColor(hdcStatic, RGB(0,0,0));
            SetBkColor(hdcStatic, RGB(192,192,192));
            SetBkMode(hdcStatic, RGB(192,192,192));
            return (INT_PTR)CreateSolidBrush(RGB(192,192,192));
        }        

        // Tamanho maximo e minimo da janela principal
       case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO hMainWindow = (LPMINMAXINFO)lParam;

            hMainWindow->ptMinTrackSize.x = 350;
            hMainWindow->ptMinTrackSize.y = 150;

            hMainWindow->ptMaxTrackSize.x = 350;
            hMainWindow->ptMaxTrackSize.y = 150;
        }

        case WM_PAINT: // Janela (ou parte dela) precisa ser atualizada
        {
            /* O cÃ³digo avisar manualmente ao Windows que a janela
            jÃ¡ foi atualizada, pois nÃ£o Ã© processo automÃ¡tico. Se isso nÃ£o for feito, o windows
            nÃ£o irÃ¡ parar de enviar a mensagem WM_PAINT ao programa 
            */

           // O cÃ³digo abaixo avisa o Windows que a janela jÃ¡ foi atualizada.
           hDC = BeginPaint(hWnd, &psPaint);
           COLORREF crBk;
           
           // Cor do texto
           //crNew = SetTextColor(hDC, RGB(192,192,192));
           
           // Cor de fundo do texto
           crBk = SetBkColor(hDC, RGB(192,192,192));
           char texto[4];
           sprintf(texto, "Process name:");
           TextOut(hDC,3,4,texto,lstrlen(texto));
           
           EndPaint(hWnd, &psPaint);
           
           return(0);
           break;
        }

        case WM_CLOSE: // Fecha janela
        {
            // DestrÃ³i a janela
            DestroyWindow(hWnd);

            return(0);
            break;
        }
    
        case WM_DESTROY: // Janela foi destruÃ­da 
        {
           // Envia mensagem WM_QUIT para o loop de mensagens 
            PostQuitMessage(0);
            return(0);
            break;
        }
        
        // All other messages (a lot of them) are processed using default procedures 
        default: // Outra mensagem
        // Deixa o Windows processar as mensagens que nÃ£o foram verificadas
        // na funÃ§Ã£o
            return DefWindowProc(hWnd, Message, wParam, lParam);
    }
    return 0;
}


int CJanelaPri (int width, int height, LPCSTR wCaption){
    // 'HANDLE', hence the H, or a pointer to our window 
    WNDCLASSEX wc; // A properties struct of our window 
    
    MSG msg; // Armazena dados da mensagem que seÃ¡ obitda (temporario)

    // zero out the struct and set the stuff we want to modify
    memset(&wc,0,sizeof(wc));
    
    // Cria a classe da janela e especifica seus atributos
    HINSTANCE hInstance;
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    
    // White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it 
    wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wc.lpszClassName = "WindowClass";
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MEUICONE)); // Load a standard icon
    wc.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MEUICONE)); // use the name "A" to use the project icon 

 
    // Registra a classe da janela
    if(!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
        return 0;
    }

    // Cria janela principal do programa
    hMainWindow = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass",wCaption,WS_VISIBLE | WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, // x 
    CW_USEDEFAULT, // y 
    width, // width 
    height, // height
    NULL,NULL,hInstance,NULL);

    hMainInstance = wc.hInstance;
    
    // Verifica se a janela foi criada
    if(hMainWindow == NULL) 
    {
        MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
        return 0;
    }

    //  Esse Ã© o coraÃ§Ã£o do programa onde todas as entradas sÃ£o processadas e
    //  enviada para WndProc. 
    
    // Loop de mensagens, enquanto mensagem nÃ£o for WM_QUIT.
    // obtÃ©m mensagem da fila de mensagens
    while(GetMessage(&msg, NULL, 0, 0) > 0) { // Se nÃ£o de erro Ã© recebido...
        TranslateMessage(&msg); // Traduz teclas virtuais ou aceleradoras (de atalho)
        DispatchMessage(&msg); // Envia mensagem para a funÃ§Ã£o que processa mensagens (WindowProc)
    }
    // Retorna ao windows com valor de msg.wParam
    return msg.wParam;
               
}


void AddControls(HWND hWnd){
            int height = 20;
            hMainWindow = hWnd;
            hTextInput = CreateWindowEx(WS_EX_TRANSPARENT, "EDIT", "",
                                        WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER, 
                                        105,3,150,height,
                                        hWnd,
                                        (HMENU)IDC_TEXTBOX, hMainInstance, NULL);
                                        
                                        hLoadDll = CreateWindowEx(WS_EX_TRANSPARENT, "BUTTON", "Load DLL",
                                        WS_VISIBLE | WS_CHILD | ES_LEFT,
                                        260,3,65,height,
                                        hWnd,
                                        (HMENU)IDC_LOADLL, hMainInstance, NULL);
                                        
                                
                                        hLblOutput = CreateWindowEx(WS_EX_STATICEDGE , "STATIC", "Load a dll",
                                        WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE | SS_CENTER ,
                                        3,50,324,height,
                                        hWnd,
                                        (HMENU)IDC_STATIC, hMainInstance, NULL);

                                        hInject = CreateWindowEx(WS_EX_TRANSPARENT, "BUTTON", "INJECT",
                                        WS_VISIBLE | WS_CHILD | ES_LEFT | WS_DISABLED | WS_BORDER,
                                        3,26,324,height,
                                        hWnd,
                                        (HMENU)IDC_INJECT, hMainInstance, NULL);           
            
                                        
                                        hTeuzero = CreateWindowEx(WS_EX_STATICEDGE , "STATIC", "By: Teuzero",
                                        WS_VISIBLE | WS_CHILD | WS_BORDER | SS_CENTERIMAGE | SS_CENTER ,
                                        100,80,130,height,
                                        hWnd,
                                        (HMENU)IDC_TEUZERO, hMainInstance, NULL);
                                      
}

BOOL Inject(DWORD pID, const char * DLL_NAME) 
{ 
    HANDLE Proc; 
    HMODULE hLib; 
    char buf[50] = {0}; 
    LPVOID RemoteString, LoadLibAddy; 
    
    Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID); 

    LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"); 
    
    // Allocate space in the process for our DLL 
    RemoteString = (LPVOID)VirtualAllocEx(Proc, NULL, strlen(DLL_NAME), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); 
    
    // Write the string name of our DLL in the memory allocated 
    WriteProcessMemory(Proc, (LPVOID)RemoteString, DLL_NAME, strlen(DLL_NAME), NULL); 
    
    // Load our DLL 
    CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL); 
    
    CloseHandle(Proc); 
    return true; 
} 

DWORD GetTargetThreadIDFromProcName(char ProcName[32]) 
{ 
   PROCESSENTRY32 pe; 
   HANDLE thSnapShot; 
   BOOL retval, ProcFound = false; 

   thSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
   pe.dwSize = sizeof(PROCESSENTRY32); 
    
   retval = Process32First(thSnapShot, &pe); 
   while(retval) 
   { 
      if(StrStrI(pe.szExeFile, ProcName)) 
      { 
         return pe.th32ProcessID; 
      } 
      retval = Process32Next(thSnapShot, &pe); 
   } 
   return 0; 
}

int Unhook(DWORD pid)
{
    HANDLE hProc;
    LPVOID addrLA,addrLW, addrLDR;
    LPVOID bytesToWriteLA[6], loadLW[6],loadLdr[6];
    
    addrLA = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"); 
    addrLW = (LPVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryW");
    addrLDR = (LPVOID)GetProcAddress(GetModuleHandle("ntdll.dll"), "LdrLoadDll");
    
    memmove(bytesToWriteLA,addrLA,6);
    memmove(loadLW,addrLW,6);
    memmove(loadLdr,addrLDR,6);
    
    hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    
    WriteProcessMemory(hProc,(LPVOID)addrLW,loadLW,6,NULL);
    WriteProcessMemory(hProc,(LPVOID)addrLA,bytesToWriteLA,6,NULL);
    WriteProcessMemory(hProc,(LPVOID)addrLDR,loadLdr,6,NULL);
    
    CloseHandle(hProc);
}

void suspend(DWORD processId)
{
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

    NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(GetModuleHandle("ntdll"), "NtSuspendProcess");

    pfnNtSuspendProcess(processHandle);
    CloseHandle(processHandle);
    
}

void resumeProc(DWORD processId)
{
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

    NtResumeProcess pfNtResumeProcess = (NtResumeProcess)GetProcAddress(GetModuleHandle("ntdll"), "NtResumeProcess");

    pfNtResumeProcess(processHandle);
    CloseHandle(processHandle);
}
