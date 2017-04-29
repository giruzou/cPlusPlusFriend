#include "cppFriends.hpp"

// インライン展開されると結果が変わる関数をここに書く

void Shift35ForInt32Asm(int32_t& result, int32_t src) {
    int32_t count = CPPFRIENDS_SHIFT_COUNT;
    asm volatile (
        "movl %1,   %0 \n\t"
        "movl %2,   %%ecx \n\t"
        "shl  %%cl, %0 \n\t"
        :"=r"(result):"r"(src),"r"(count):"%ecx","memory");
    // 第一引数はRCXレジスタに入っているので、破壊レジスタに指定しないとクラッシュする
//      :"=r"(result):"r"(src),"r"(count):);
}

int32_t Shift35ForInt32(int32_t src) {
    decltype(src) result = src;
// シフト回数が多すぎる警告を出さないようにする
#if CPPFRIENDS_SHIFT_COUNT >= 32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshift-count-overflow"
#endif
    result <<= CPPFRIENDS_SHIFT_COUNT;
// 以後は、シフト回数が多すぎる警告を出す
#if CPPFRIENDS_SHIFT_COUNT >= 32
#pragma GCC diagnostic pop
#endif
    return result;
}

int32_t ShiftForInt32(int32_t src, int32_t count) {
    decltype(src) result = src;
    result <<= count;
    return result;
}

/*
Local Variables:
mode: c++
coding: utf-8-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/