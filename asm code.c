#include <windows.h>

int main()
{
    MessageBoxW(NULL,L"MyApplication",L"hello, world 64!",MB_OK);
    return 0;
}
// xor    rax,rax
// xor    rbx,rbx
// call   b
// b:
// pop    rax
// and    rax,0xfffffffffff00000
// mov    rbx,0x14d0
// or     rbx,rax 
// mov         esi,0  
// jmp         middle
// LL1: 
// mov         eax,esi  
// inc         eax  
// mov         esi,eax  

// middle:
// cmp         esi,0xA  
// jge         oout  
// movsxd      rax,esi  
// mov         rcx,qword ptr [rbx]  
// mov         rax,qword ptr [rcx+rax*8]  
// mov rcx,0xDEAFCAFEDEAFCAFE
// xor         rax,rcx     
// mov         rdx, rax
// movsxd      rax,esi  
// mov         rcx,qword ptr [rbx]
// mov         qword ptr [rcx+rax*8],rdx  
// jmp         LL1
// oout: