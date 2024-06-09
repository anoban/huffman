#ifdef __TEST__

    #include <huffman.h>

int wmain(void) {
    size_t               avatar_size = 0, mobydick_size = 0;
    const uint8_t* const avatar   = openFile(L"./media/neytiri.jpg", &avatar_size);
    const uint8_t* const mobydick = openFile(L"./media/mobydick.txt", &mobydick_size);

    assert(avatar_size == 4820673LLU);
    assert(mobydick_size == 1276266LLU);

    free(avatar);
    free(mobydick);

    const uint8_t const bitstream[] = { 0x00, 0x01, 0x0E, 0xA7, 0xAA, 0xEF, 0xFF, 0x56, 0x91, 0xA7 };

    return EXIT_SUCCESS;
}

#endif // __TEST__