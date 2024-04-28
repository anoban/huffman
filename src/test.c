#include <huffman.h> //#if defined(DEBUG) || defined(_DEBUG), huffman.h will define __RUN_TESTS__

#ifdef __RUN_TESTS__

int wmain(void) {
    size_t               avatar_size = 0, mobydick_size = 0;
    const uint8_t* const avatar   = openFile(L"./media/neytiri.jpg", &avatar_size);
    const uint8_t* const mobydick = openFile(L"./media/mobydick.txt", &mobydick_size);

    assert(avatar_size == 4820673LLU);
    assert(mobydick_size == 1276266LLU);

    free(avatar);
    free(mobydick);

    return EXIT_SUCCESS;
}

#endif // __RUN_TESTS__
