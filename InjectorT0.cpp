/**************
* By: Teuzero *
***************/

#include <windows.h>
#include <iostream>
#include "main.h"

using namespace std;

/**
 * Get debug privileges fo current process token
 */
 
DWORD ChangePrivileges() {
    HANDLE currentProcess = GetCurrentProcess();
    PHANDLE htoken = 0;
    TOKEN_PRIVILEGES priv;
    if (!OpenProcessToken(currentProcess, TOKEN_ADJUST_PRIVILEGES, htoken)) {
        return FALSE;
    }

    if (!LookupPrivilegeValue(0, "SeDebugPrivilege", &priv.Privileges[0].Luid)) {
        return FALSE;
    }

    priv.PrivilegeCount = 1;
    priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(htoken, 0, &priv, sizeof(TOKEN_PRIVILEGES), 0, 0)) {
        std::cout << "\n Erro no previlegio";
        return FALSE;
    }

    CloseHandle(htoken);
    return 0;
}

int main(int argc,char*argv[]){
    BOOL EnableDebugPrivileges(void);
    system("cls");
    char teu[8] = "teuzero";
    char result[8];
    int j = 0;
    for(int i = 0; teu[i];i++){
        teu[i] = (teu[i] + 6);
    }
    
    sprintf(result,"%s",teu);
    
    cout << "Digite a senha: ";
    
    char senha[8];
    fgets(senha,8,stdin);
    
    for(int count= 0; senha[count];count++)
        if(senha[count] == '\n')
            senha[count] = 0;
    
    
    if(strcmp(senha, result) == 0){
        for(int i = 0; i <= 100; i++){
            
            system("cls");
            cout << "*******************************************************"<<endl;
            cout << "*********** Aguarde o injector abrir... ***************"<<endl;
            cout << "**********          "<< i << "%                 **************"<<endl;
            cout << "*******************************************************"<<endl;
        }
        system("start www.c0d3r3d.blogspot.com/");
        CJanelaPri(350,120,"Injector-T0");
        
    }else{
        cout << "\nSenha incorreta! tente novamente." << endl;
        system("Pause");
        main(0,0);
    }
}
