; an assembly listing with routines for bit operations.

option casemap:none

.data

.code

	; get the nth bit of an unsigned char
	; extern bool getbit(_In_ const uint8_t* const restrict bitstream, _In_ const size_t pos)
	; bitstream is really an array of bytes (well, bits too)
	; pos specifies the position of the BIT! 

	getbit proc


	getbit endp

end
