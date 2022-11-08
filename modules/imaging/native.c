#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define MSF_GIF_IMPL
#include "msf_gif.h"

#ifdef __linux__
#define PRINT_I64 "%ld"
#else
#define PRINT_I64 "%lld"
#endif

#include "../native_interface.h"

typedef struct Bitmap {
	// This structure can be used as a UIPainter from the luigi module.
	int _unused[4];
	uint32_t *bits;
	int width, height;
} Bitmap;

typedef struct Animation {
	MsfGifState gif;
	int width, height;
} Animation;

void CloseBitmapHandle(struct ExecutionContext *context, void *handleData) {
	(void) context;
	Bitmap *bitmap = (Bitmap *) handleData;
	free(bitmap->bits);
	free(bitmap);
}

void CloseAnimationHandle(struct ExecutionContext *context, void *handleData) {
	(void) context;
	Animation *animation = (Animation *) handleData;
	msf_gif_free(msf_gif_end(&animation->gif));
	free(animation);
}

LIBRARY_EXPORT bool ScriptExtCreate(struct ExecutionContext *context) {
	int64_t width, height;
	if (!ScriptParameterInt64(context, &width)) return false;
	if (!ScriptParameterInt64(context, &height)) return false;
	if (width < 0 || width > 1000000) { fprintf(stderr, "(imaging) Create: bad width parameter "PRINT_I64"\n", width); return false; }
	if (height < 0 || height > 1000000) { fprintf(stderr, "(imaging) Create: bad height parameter "PRINT_I64"\n", height); return false; }
	if (4 * width * height > 2000000000) { fprintf(stderr, "(imaging) Create: image too big "PRINT_I64"x"PRINT_I64"\n", width, height); return false; }
	Bitmap *bitmap = (Bitmap *) calloc(1, sizeof(Bitmap));
	bitmap->width = width;
	bitmap->height = height;
	bitmap->bits = (uint32_t *) calloc(1, 4 * width * height);
	if (!ScriptReturnHandle(context, bitmap, CloseBitmapHandle)) return false;
	return true;
}

LIBRARY_EXPORT bool ScriptExtWidth(struct ExecutionContext *context) {
	Bitmap *bitmap;
	if (!ScriptParameterHandle(context, (void **) &bitmap)) return false;
	if (!bitmap) { fprintf(stderr, "(imaging) Width: Bitmap was null.\n"); return false; }
	if (!ScriptReturnInt(context, bitmap->width)) return false;
	return true;
}

LIBRARY_EXPORT bool ScriptExtHeight(struct ExecutionContext *context) {
	Bitmap *bitmap;
	if (!ScriptParameterHandle(context, (void **) &bitmap)) return false;
	if (!bitmap) { fprintf(stderr, "(imaging) Height: Bitmap was null.\n"); return false; }
	if (!ScriptReturnInt(context, bitmap->height)) return false;
	return true;
}

LIBRARY_EXPORT bool ScriptExtReadPixel(struct ExecutionContext *context) {
	Bitmap *bitmap;
	int64_t x, y;
	if (!ScriptParameterHandle(context, (void **) &bitmap)) return false;
	if (!bitmap) { fprintf(stderr, "(imaging) ReadPixel: Bitmap was null.\n"); return false; }
	if (!ScriptParameterInt64(context, &x)) return false;
	if (!ScriptParameterInt64(context, &y)) return false;
	if (x < 0 || x > bitmap->width) { fprintf(stderr, "(imaging) ReadPixel: bad x parameter "PRINT_I64"\n", x); return false; }
	if (y < 0 || y > bitmap->height) { fprintf(stderr, "(imaging) ReadPixel: bad y parameter "PRINT_I64"\n", y); return false; }
	return ScriptReturnInt(context, bitmap->bits[y * bitmap->width + x]);
}

LIBRARY_EXPORT bool ScriptExtCrop(struct ExecutionContext *context) {
	Bitmap *bitmap;
	int64_t left, right, top, bottom;

	if (!ScriptParameterHandle(context, (void **) &bitmap)) return false;
	if (!bitmap) { fprintf(stderr, "(imaging) Crop: Bitmap was null.\n"); return false; }
	if (!ScriptParameterInt64(context, &left)) return false;
	if (!ScriptParameterInt64(context, &right)) return false;
	if (!ScriptParameterInt64(context, &top)) return false;
	if (!ScriptParameterInt64(context, &bottom)) return false;
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

LIBRARY_EXPORT bool ScriptExtBlit(struct ExecutionContext *context) {
	Bitmap *destination, *source;
	int64_t x, y;

	if (!ScriptParameterHandle(context, (void **) &destination)) return false;
	if (!ScriptParameterHandle(context, (void **) &source)) return false;
	if (!destination) { fprintf(stderr, "(imaging) Blit: Destination bitmap was null.\n"); return false; }
	if (!source) { fprintf(stderr, "(imaging) Blit: Source bitmap was null.\n"); return false; }
	if (!ScriptParameterInt64(context, &x)) return false;
	if (!ScriptParameterInt64(context, &y)) return false;
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

LIBRARY_EXPORT bool ScriptExtLoad(struct ExecutionContext *context) {
	const void *input;
	size_t bytes;
	if (!ScriptParameterString(context, &input, &bytes)) return false;
	int width, height, components;
	uint8_t *data = stbi_load_from_memory((const uint8_t *) input, bytes, &width, &height, &components, 4);

	if (data) {
		for (int i = 0; i < width * height; i++) {
			uint32_t x = ((uint32_t *) data)[i];
			((uint32_t *) data)[i] = (x & 0xFF00FF00) | ((x & 0xFF0000) >> 16) | ((x & 0xFF) << 16);
		}

		Bitmap *bitmap = (Bitmap *) calloc(1, sizeof(Bitmap));
		bitmap->width = width;
		bitmap->height = height;
		bitmap->bits = (uint32_t *) data;
		if (!ScriptReturnHandle(context, bitmap, CloseBitmapHandle)) return false;
		if (!ScriptReturnBoxInError(context)) return false;
	} else {
		if (!ScriptReturnError(context, "UNSUPPORTED_IMAGE_FORMAT")) return false;
	}

	return true;
}

LIBRARY_EXPORT bool ScriptExtSave(struct ExecutionContext *context) {
	Bitmap *bitmap;
	char *format;
	if (!ScriptParameterHandle(context, (void **) &bitmap)) return false;
	if (!bitmap) { fprintf(stderr, "(imaging) Save: Bitmap was null.\n"); return false; }
	if (!ScriptParameterCString(context, &format)) return false;
	bool png = 0 == strcmp(format, "png");
	free(format);
	if (!png) return ScriptReturnError(context, "UNSUPPORTED_IMAGE_FORMAT");
	int bytes;

	for (int i = 0; i < bitmap->width * bitmap->height; i++) {
		uint32_t x = bitmap->bits[i];
		bitmap->bits[i] = (x & 0xFF00FF00) | ((x & 0xFF0000) >> 16) | ((x & 0xFF) << 16);
	}

	uint8_t *data = stbi_write_png_to_mem((uint8_t *) bitmap->bits, 4 * bitmap->width, bitmap->width, bitmap->height, 4, &bytes);

	for (int i = 0; i < bitmap->width * bitmap->height; i++) {
		uint32_t x = bitmap->bits[i];
		bitmap->bits[i] = (x & 0xFF00FF00) | ((x & 0xFF0000) >> 16) | ((x & 0xFF) << 16);
	}

	if (!data) return ScriptReturnError(context, "INSUFFICIENT_RESOURCES");
	if (!ScriptReturnString(context, data, bytes)) { free(data); return false; }
	free(data);
	if (!ScriptReturnBoxInError(context)) return false;
	return true;
}

LIBRARY_EXPORT bool ScriptExtDrawBlock(struct ExecutionContext *context) {
	Bitmap *bitmap;
	int64_t left, right, top, bottom;
	uint32_t color;
	if (!ScriptParameterScan(context, "hIIIIu", &bitmap, &left, &right, &top, &bottom, &color)) return false;
	if (!bitmap) { fprintf(stderr, "(imaging) DrawBlock: Bitmap was null.\n"); return false; }

	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right > bitmap->width) right = bitmap->width;
	if (bottom > bitmap->height) bottom = bitmap->height;

	if (left < right) {
		for (int64_t y = top; y < bottom; y++) {
			for (int64_t x = left; x < right; x++) {
				bitmap->bits[x + y * bitmap->width] = color;
			}
		}
	}

	return true;
}

LIBRARY_EXPORT bool ScriptExtAnimationCreate(struct ExecutionContext *context) {
	int64_t width, height;
	if (!ScriptParameterScan(context, "II", &width, &height)) return false;
	if (width < 0 || width > 1000000) { fprintf(stderr, "(imaging) AnimationCreate: bad width parameter "PRINT_I64"\n", width); return false; }
	if (height < 0 || height > 1000000) { fprintf(stderr, "(imaging) AnimationCreate: bad height parameter "PRINT_I64"\n", height); return false; }
	if (4 * width * height > 2000000000) { fprintf(stderr, "(imaging) AnimationCreate: image too big "PRINT_I64"x"PRINT_I64"\n", width, height); return false; }
	Animation *animation = (Animation *) calloc(1, sizeof(Animation));
	animation->width = width;
	animation->height = height;
	msf_gif_begin(&animation->gif, width, height);
	return ScriptReturnHandle(context, animation, CloseAnimationHandle);
}

LIBRARY_EXPORT bool ScriptExtAnimationAddFrame(struct ExecutionContext *context) {
	Bitmap *bitmap;
	Animation *animation;
	double time;
	if (!ScriptParameterScan(context, "hhF", &animation, &bitmap, &time)) return false;
	if (!bitmap) { fprintf(stderr, "(imaging) AnimationAddFrame: Bitmap was null.\n"); return false; }
	if (!animation) { fprintf(stderr, "(imaging) AnimationAddFrame: Animation was null.\n"); return false; }
	if (bitmap->width != animation->width) { fprintf(stderr, "(imaging) AnimationAddFrame: Bitmap width does not match the animation's.\n"); return false; }
	if (bitmap->height != animation->height) { fprintf(stderr, "(imaging) AnimationAddFrame: Bitmap height does not match the animation's.\n"); return false; }
	msf_gif_frame(&animation->gif, (uint8_t *) bitmap->bits, ceil(time * 100), 16, animation->width * 4);
	return true;
}

LIBRARY_EXPORT bool ScriptExtAnimationSave(struct ExecutionContext *context) {
	Animation *animation;
	const char *format; size_t formatBytes;
	if (!ScriptParameterScan(context, "hS", &animation, &format, &formatBytes)) return false;
	bool gif = formatBytes == 3 && format[0] == 'g' && format[1] == 'i' && format[2] == 'f';
	if (!gif) return ScriptReturnError(context, "UNSUPPORTED_IMAGE_FORMAT");
	if (!animation) { fprintf(stderr, "(imaging) AnimationSave: Animation was null.\n"); return false; }
	MsfGifResult result = msf_gif_end(&animation->gif);
	bool success = ScriptReturnString(context, result.data, result.dataSize);
	msf_gif_free(result);
	msf_gif_begin(&animation->gif, animation->width, animation->height); // TODO Keep the existing animation contents.
	return success && ScriptReturnBoxInError(context);
}
