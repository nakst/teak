#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#ifdef __linux__
#define PRINT_I64 "%ld"
#else
#define PRINT_I64 "%lld"
#endif

typedef struct Bitmap {
	int width, height;
	uint32_t *bits;
} Bitmap;

bool ScriptParameterInt64(void *engine, int64_t *output);
bool ScriptParameterHandle(void *engine, void **output);
bool ScriptParameterString(void *engine, const void **output, size_t *outputBytes);
bool ScriptParameterCString(void *engine, char **output);
bool ScriptReturnInt(void *engine, int64_t input);
bool ScriptReturnHandle(void *engine, void *handleData, void (*close)(void *));
bool ScriptReturnString(void *engine, const void *data, size_t dataBytes);
bool ScriptReturnBoxInError(void *engine);
bool ScriptReturnError(void *engine, const char *message);

void CloseBitmapHandle(void *handleData) {
	Bitmap *bitmap = (Bitmap *) handleData;
	free(bitmap->bits);
	free(bitmap);
}

bool ScriptExtCreate(void *engine) {
	int64_t width, height;
	if (!ScriptParameterInt64(engine, &width)) return false;
	if (!ScriptParameterInt64(engine, &height)) return false;
	if (width < 0 || width > 1000000) { fprintf(stderr, "(imaging) Create: bad width parameter "PRINT_I64"\n", width); return false; }
	if (height < 0 || height > 1000000) { fprintf(stderr, "(imaging) Create: bad height parameter "PRINT_I64"\n", height); return false; }
	if (4 * width * height > 2000000000) { fprintf(stderr, "(imaging) Create: image too big "PRINT_I64"x"PRINT_I64"\n", width, height); return false; }
	Bitmap *bitmap = (Bitmap *) calloc(1, sizeof(Bitmap));
	bitmap->width = width;
	bitmap->height = height;
	bitmap->bits = (uint32_t *) calloc(1, 4 * width * height);
	if (!ScriptReturnHandle(engine, bitmap, CloseBitmapHandle)) return false;
	return true;
}

bool ScriptExtWidth(void *engine) {
	Bitmap *bitmap;
	if (!ScriptParameterHandle(engine, (void **) &bitmap)) return false;
	if (!bitmap) { fprintf(stderr, "(imaging) Width: Bitmap was null.\n"); return false; }
	if (!ScriptReturnInt(engine, bitmap->width)) return false;
	return true;
}

bool ScriptExtHeight(void *engine) {
	Bitmap *bitmap;
	if (!ScriptParameterHandle(engine, (void **) &bitmap)) return false;
	if (!bitmap) { fprintf(stderr, "(imaging) Height: Bitmap was null.\n"); return false; }
	if (!ScriptReturnInt(engine, bitmap->height)) return false;
	return true;
}

bool ScriptExtCrop(void *engine) {
	Bitmap *bitmap;
	int64_t left, right, top, bottom;

	if (!ScriptParameterHandle(engine, (void **) &bitmap)) return false;
	if (!bitmap) { fprintf(stderr, "(imaging) Crop: Bitmap was null.\n"); return false; }
	if (!ScriptParameterInt64(engine, &left)) return false;
	if (!ScriptParameterInt64(engine, &right)) return false;
	if (!ScriptParameterInt64(engine, &top)) return false;
	if (!ScriptParameterInt64(engine, &bottom)) return false;
	if (left < 0 || left > bitmap->width || left > right) { fprintf(stderr, "(imaging) Crop: bad left parameter "PRINT_I64"\n", left); return false; }
	if (right < 0 || right > bitmap->width) { fprintf(stderr, "(imaging) Crop: bad right parameter "PRINT_I64"\n", right); return false; }
	if (top < 0 || top > bitmap->height || top > bottom) { fprintf(stderr, "(imaging) Crop: bad top parameter "PRINT_I64"\n", top); return false; }
	if (bottom < 0 || bottom > bitmap->height) { fprintf(stderr, "(imaging) Crop: bad bottom parameter "PRINT_I64"\n", bottom); return false; }

	Bitmap copy = *bitmap;
	copy.width = right - left;
	copy.height = bottom - top;
	copy.bits = (uint32_t *) malloc(4 * copy.width * copy.height);

	for (int y = 0; y < copy.height; y++) {
		for (int x = 0; x < copy.width; x++) {
			copy.bits[y * copy.width + x] = bitmap->bits[(y + top) * bitmap->width + (x + left)];
		}
	}

	free(bitmap->bits);
	*bitmap = copy;

	return true;
}

bool ScriptExtBlit(void *engine) {
	Bitmap *destination, *source;
	int64_t x, y;

	if (!ScriptParameterHandle(engine, (void **) &destination)) return false;
	if (!ScriptParameterHandle(engine, (void **) &source)) return false;
	if (!destination) { fprintf(stderr, "(imaging) Blit: Destination bitmap was null.\n"); return false; }
	if (!source) { fprintf(stderr, "(imaging) Blit: Source bitmap was null.\n"); return false; }
	if (!ScriptParameterInt64(engine, &x)) return false;
	if (!ScriptParameterInt64(engine, &y)) return false;
	if (x >= destination->width || x <= -source->width) return true;
	if (y >= destination->height || y <= -source->height) return true;

	int left = x > 0 ? x : 0, right = x + source->width > destination->width ? destination->width : x + source->width;
	int top = y > 0 ? y : 0, bottom = y + source->height > destination->height ? destination->height : y + source->height;

	for (int j = top; j < bottom; j++) {
		for (int i = left; i < right; i++) {
			destination->bits[j * destination->width + i] = source->bits[(j - y) * source->width + (i - x)];
		}
	}

	return true;
}

bool ScriptExtLoad(void *engine) {
	const void *input;
	size_t bytes;
	if (!ScriptParameterString(engine, &input, &bytes)) return false;
	int width, height, components;
	uint8_t *data = stbi_load_from_memory((const uint8_t *) input, bytes, &width, &height, &components, 4);

	if (data) {
		Bitmap *bitmap = (Bitmap *) calloc(1, sizeof(Bitmap));
		bitmap->width = width;
		bitmap->height = height;
		bitmap->bits = (uint32_t *) data;
		if (!ScriptReturnHandle(engine, bitmap, CloseBitmapHandle)) return false;
		if (!ScriptReturnBoxInError(engine)) return false;
	} else {
		if (!ScriptReturnError(engine, "UNSUPPORTED_IMAGE_FORMAT")) return false;
	}

	return true;
}

bool ScriptExtSave(void *engine) {
	Bitmap *bitmap;
	char *format;
	if (!ScriptParameterHandle(engine, (void **) &bitmap)) return false;
	if (!bitmap) { fprintf(stderr, "(imaging) Save: Bitmap was null.\n"); return false; }
	if (!ScriptParameterCString(engine, &format)) return false;
	bool png = 0 == strcmp(format, "png");
	free(format);
	if (!png) return ScriptReturnError(engine, "UNSUPPORTED_IMAGE_FORMAT");
	int bytes;
	uint8_t *data = stbi_write_png_to_mem((uint8_t *) bitmap->bits, 4 * bitmap->width, bitmap->width, bitmap->height, 4, &bytes);
	if (!data) return ScriptReturnError(engine, "INSUFFICIENT_RESOURCES");
	if (!ScriptReturnString(engine, data, bytes)) { free(data); return false; }
	free(data);
	if (!ScriptReturnBoxInError(engine)) return false;
	return true;
}
