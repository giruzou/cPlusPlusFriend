#include <assert.h>
#include <stdint.h>
#include <stdio.h>

// gcc, clang�ł́u�����s�̃R�����g�v�x�����o��

int main(int argc, char* argv[]) {
    // Shift_JIS�ł��̃R�����g�͂܂����ŃQ�\
    printf("First line\n");

    // -Wall�Ȃ��A�܂�Ōx���Ȃ��ł��̖����������͕̂s�\
    printf("Second line\n");

    // �ł͂Ȃ��Ǝv���܂����A-Wall -Werror ������ׂ����Ǝv���̂ł�
    printf("Third line\n");
    return 0;
}

/*
Local Variables:
mode: c
coding: shift_jis-dos
tab-width: nil
c-file-style: "stroustrup"
End:
*/
