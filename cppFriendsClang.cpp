/* gccとclangを比較する */
#include <cstdint>
#include <cstring>
#include "cppFriendsClang.hpp"

namespace SwitchCase {
    // 呼び出し先をインライン展開させない
    double GetFixedTestValue(SwitchCase::Shape shape) {
        double actual = 0.0;
        switch(shape) {
        case SwitchCase::Shape::CIRCLE:
            actual = SwitchCase::GetAreaOfCircle(2.0);
            break;
        case SwitchCase::Shape::RECTANGULAR:
            actual = SwitchCase::GetAreaOfRectangular(2.0, 3.0);
            break;
        case SwitchCase::Shape::TRIANGLE:
            actual = SwitchCase::GetAreaOfTriangle(6.0, 8.0, 10.0);
            break;
            // caseが4個だと、clangはルックアップテーブルを作る
        case SwitchCase::Shape::SQUARE:
            actual = SwitchCase::GetAreaOfRectangular(7.0, 7.0);
            break;
        default:
            break;
        }

        return actual;
    }
}

namespace MemoryOperation {
    uint32_t g_shortBuffer[2];
    uint32_t g_largeBuffer[1024];

    void FillByZeroShort(void) {
        // mov qword ptr [rax], 0
        ::memset(g_shortBuffer, 0, sizeof(g_shortBuffer));
    }

    void FillByZeroLarge(void) {
        // rex64 jmp memset # TAILCALL
        ::memset(g_largeBuffer, 0, sizeof(g_largeBuffer));
    }

    struct BitFields {
        unsigned int member1 : 2;
        unsigned int member2 : 4;
        unsigned int member3 : 4;
        unsigned int member4 : 5;
        unsigned int : 0;
    };

    void SetBitFields(BitFields& fields) {
        fields.member1 = 1;
        fields.member2 = 3;
        fields.member3 = 7;
        fields.member4 = 15;
        // 11' 1101'1100'1101b = 15,821
        return;
   }

    void SetBitFieldsAtOnce(BitFields& fields) {
        const BitFields origin = {1, 3, 7, 15};
        fields = origin;
        return;
   }

#if 0
    // g++ -std=gnu++14 -Wall -IC:\MinGW\include
    // -O2 -S -masm=intel -mavx2 -o cppFriendsClang.s cppFriendsClang.cpp
    // 両関数ともこのコードになる
    movzx   eax, BYTE PTR [rcx]
    and eax, -64
    or  eax, 13
    mov BYTE PTR [rcx], al
    movzx   eax, WORD PTR [rcx]
    and ax, -961
    or  ax, 448
    mov WORD PTR [rcx], ax
    shr ax, 8
    and eax, -125
    or  eax, 60
    mov BYTE PTR 1[rcx], al
    ret

    // clang++ -std=gnu++14 -Wall --target=x86_64-pc-windows-gnu
    // -isystem C:\MinGW\include -isystem C:\MinGW\lib\gcc\x86_64-w64-mingw32\6.3.0\include
    // -O2 -S -masm=intel -mavx2 -o cppFriendsClang.s cppFriendsClang.cpp
    // SetBitFields
    movzx   eax, word ptr [rcx]
    and eax, 32768
    or  eax, 15821
    mov word ptr [rcx], ax
    ret
    // SetBitFieldsAtOnce
    mov dword ptr [rcx], 15821
    ret
#endif
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/