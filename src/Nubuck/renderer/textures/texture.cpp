#include <common\common.h>
#include <renderer\metrics\metrics.h>
#include "..\glcall.h"
#include "texture.h"

namespace R {

    inline int NumComponents(GLenum format) {
        switch(format) {
        case GL_RGB: return 3;
        case GL_RGBA: return 4;
        }
        Crash();
        return 0;
    }

	/* Texture Impl */

	void Texture::Init(const COM::byte_t* pixelData) {
		COM_assert(0 < _width);
		COM_assert(0 < _height);

		GL_CALL(glGenTextures(1, &_id));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, _id));
		
		GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, _internalFormat, _width, _height, 0, _format,
			GL_UNSIGNED_BYTE, pixelData));

        metrics.resources.totalTextureBufferSize += sizeof(char) * NumComponents(_internalFormat) * _width * _height;

		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR));

		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	}

	Texture::Texture(int width, int height, GLenum internalFormat, GLenum format,
		const COM::byte_t* pixelData)
		: _internalFormat(internalFormat), _format(format), _width(width), _height(height)
	{
		Init(pixelData);
	}

	Texture::Texture(int width, int height, GLenum format, const COM::byte_t* pixelData) 
		: _internalFormat(format), _format(format), _width(width), _height(height)
	{
		Init(pixelData);
	}

	void Texture::Bind(unsigned level) const {
		GL_CALL(glActiveTexture(GL_TEXTURE0 + level));
		GL_CALL(glBindTexture(GL_TEXTURE_2D, _id));
	}

	/* TextureManager Impl */

	std::string ToLower(const std::string& str) {
		std::string ret;
		for(std::size_t i = 0; i < str.length(); ++i) {
			char c = str[i];
			if(c >= 'A' && c <= 'Z') ret.push_back('a' + (c - 'A'));
			else ret.push_back(c);
		}
		return ret;
	}

	std::string Suffix(const std::string& str) {
		std::string suffix;
		std::size_t pos = str.find_last_of('.');
		if(std::string::npos != pos) {
			suffix = str.substr(pos + 1);
		}
		return suffix;
	}

	void TextureManager::RegisterAlloc(const std::string& suffix, texAlloc_t alloc) {
		if(std::string::npos != suffix.find('.')) {
            common.printf("TextureManager::RegisterAlloc: suffix %s contains '.'\n", suffix.c_str());
		}
		_allocs[ToLower(suffix)] = alloc;
	}

	GEN::Pointer<Texture> TextureManager::Get(const std::string& filename, FS::Loader& loader) {
		texIt_t texIt = _textures.find(filename);
		if(_textures.end() == texIt) {
			std::string suffix = ToLower(Suffix(filename));
			allocIt_t allocIt = _allocs.find(suffix);
			if(_allocs.end() == allocIt) {
                common.printf("TextureManager::Get: for suffix '%s' of filename '%s' is no allocator registered.\n",
                    suffix.c_str(), filename.c_str());
				throw std::exception();
			}

			GEN::Pointer<FS::File> file = loader.GetFile(filename);
			GEN::Pointer<Texture> texture(allocIt->second(file));
			_textures[filename] = texture;

			return texture;
		} else {
			return texIt->second;
		}
	}

} // namespace R