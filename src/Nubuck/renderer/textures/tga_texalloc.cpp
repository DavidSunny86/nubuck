#include <Nubuck\common\common.h>
#include <common\filehandle.h>
#include "texture.h"

#define TYPE_UNCOMPRESSED_RGB24 2

typedef unsigned short tgaWord_t;

#pragma pack(push, 1)

typedef struct {
	COM::byte_t imageId;
	COM::byte_t colorMapType;
	COM::byte_t imageType;

	tgaWord_t	colorMapStart;
	tgaWord_t	colorMapLength;
	COM::byte_t colorMapEntrySize;
} FileSpec_t;

typedef struct {
	tgaWord_t originx;
	tgaWord_t originy;

	tgaWord_t	width;
	tgaWord_t	height;
	COM::byte_t	bitsPerPixel;

	COM::byte_t attributes;
} ImageSpec_t;

typedef struct {
	FileSpec_t fileSpec;
	ImageSpec_t imageSpec;
} TGAHeader_t;

#pragma pack(pop)

const FileSpec_t supportedFileSpec = {
	0, 0, TYPE_UNCOMPRESSED_RGB24, 0, 0, 0
};


R::Texture* AllocTGA(GEN::Pointer<FS::File> file) {
	TGAHeader_t header;
	int bytesPerPixel;
	int size;

	file->Read((char*)&header, sizeof(TGAHeader_t));

	if(0 != memcmp(&header.fileSpec, &supportedFileSpec, sizeof(FileSpec_t))) {
        common.printf("'%s': ", file->Name().c_str());
        common.printf("invalid tga format. currently only uncompressed 24bit rga, 32bit rgba files are supported.\n");
        common.printf("... header.fileSpec.imageId = %d\n", header.fileSpec.imageId);
        common.printf("... header.fileSpec.colorMapType = %d\n", header.fileSpec.colorMapType);
        common.printf("... header.fileSpec.imageType = %d\n", header.fileSpec.imageType);
        common.printf("... header.fileSpec.colorMapStart = %d\n", header.fileSpec.colorMapStart);
        common.printf("... header.fileSpec.colorMapLength = %d\n", header.fileSpec.colorMapLength);
        common.printf("... header.fileSpec.colorMapEntrySize = %d\n", header.fileSpec.colorMapEntrySize);
		throw COM::InvalidFormatException();
	}

	if(0 != header.imageSpec.originx || 0 != header.imageSpec.originy) {
        common.printf("'%s': tga image origin must be 0,0\n", file->Name().c_str());
		throw COM::InvalidFormatException();
	}

	// TODO: test for non-power-of-2 textures
	if(0 == header.imageSpec.width || 0 == header.imageSpec.height) {
        common.printf("'%s' has invalid size (%d, %d)\n",
            file->Name().c_str(), header.imageSpec.width, header.imageSpec.height);
		throw COM::InvalidFormatException();
	}

	GLenum format;
	switch(header.imageSpec.bitsPerPixel) {
	case 24:
		format = GL_RGB;
		break;
	case 32:
		format = GL_RGBA;
		break;
	default:
        common.printf("'%s' must have either 24 or 32bit color depth\n", file->Name().c_str());
		throw COM::InvalidFormatException();
	}

	bytesPerPixel = header.imageSpec.bitsPerPixel / 8;
	size = bytesPerPixel *
		header.imageSpec.width *
		header.imageSpec.height;

	COM::byte_t* pixelData = (COM::byte_t*)malloc(size);
	if(NULL == pixelData) {
        common.printf("out of memory loading %s\n", file->Name().c_str());
		throw std::bad_alloc();
	}

	file->Read((char*)pixelData, size);

	for(int i = 0; i < size; i += bytesPerPixel) {
		std::swap(pixelData[i], pixelData[i + 2]);
	}

    if(0 != (32 & header.imageSpec.attributes)) {
        // the origin is in the upper left hand corner instead of the lower
        // left hand corner, so we have to flip the image
        const int stride = bytesPerPixel * header.imageSpec.width;
        int h = 0, l = header.imageSpec.height - 1;
        while(h < l) {
            // swap rows h and l
            for(int i = 0; i < stride; ++i) {
                std::swap(pixelData[stride * h + i], pixelData[stride * l + i]);
            }
            h++;
            l--;
        }
    }

	int width = header.imageSpec.width;
	int height = header.imageSpec.height;

	R::Texture* texture = new R::Texture(width, height, format, pixelData);

	free(pixelData);
	return texture;
}

void WriteTGA_BGR(
    const std::string& filename,
    tgaWord_t width,
    tgaWord_t height,
    COM::byte_t* pixelData)
{
    COM::FileHandle file(fopen(filename.c_str(), "w"));
    if(NULL == file.Handle()) {
        common.printf("ERROR - WriteTGA_BGR: unable to open file '%s'\n", filename.c_str());
        return;
    }

    TGAHeader_t header;

    memcpy(&header.fileSpec, &supportedFileSpec, sizeof(FileSpec_t));
    memset(&header.imageSpec, 0, sizeof(ImageSpec_t));

    header.imageSpec.width          = width;
    header.imageSpec.height         = height;
    header.imageSpec.bitsPerPixel   = 24;

    fwrite(&header, sizeof(TGAHeader_t), 1, file.Handle());

    unsigned sz = width * height * 3;
    fwrite(pixelData, sz, 1, file.Handle());
}

R::RegisterTexAlloc registerTGA("tga", AllocTGA);