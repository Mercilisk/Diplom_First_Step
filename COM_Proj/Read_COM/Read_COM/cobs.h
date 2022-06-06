
#include <stdint.h>
#include <stddef.h>
#include "stdafx.h"
#include "windows.h"
#include "math.h"

size_t cobs_encode(const UINT8 * input, size_t length,
	UINT8 * output);
	
size_t cobs_decode(const UINT8 * input, size_t length,
	UINT8 * output);

