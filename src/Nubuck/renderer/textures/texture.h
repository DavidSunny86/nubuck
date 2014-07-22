#pragma once

#include <string>
#include <map>

#include <renderer\glew\glew.h>

#include <Nubuck\common\common.h>
#include <common\filesystem\filesystem.h>

namespace R {

	class Texture {
	private:
		GLuint _id;
		GLenum _internalFormat;
		GLenum _format;
		int _width;
		int _height;

		void Init(const COM::byte_t* pixelData);
	public:
		// passing NULL for pixelData makes sense when the texture is rendering target
		Texture(int width, int height, GLenum internalFormat, GLenum format,
			const COM::byte_t* pixelData = NULL);
		Texture(int width, int height, GLenum format, const COM::byte_t* pixelData = NULL);
        ~Texture();

		void Bind(unsigned level) const;

		GLuint GetID(void) const { return _id; }
		GLenum Format(void) const { return _format; }
		int Width(void) const { return _width; }
		int Height(void) const { return _height; }

        void WriteTGA(const std::string& filename);
	};

	typedef Texture* (*texAlloc_t)(GEN::Pointer<FS::File> file);

	class TextureManager : public GEN::Singleton<TextureManager> {
		friend class GEN::Singleton<TextureManager>;
	private:
		typedef std::map<std::string, texAlloc_t>::iterator allocIt_t;
		typedef std::map<std::string, GEN::Pointer<Texture> >::iterator texIt_t;

		std::map<std::string, texAlloc_t> _allocs;
		std::map<std::string, GEN::Pointer<Texture> > _textures;

		TextureManager(void) { }
	public:
		~TextureManager(void) { }

		void RegisterAlloc(const std::string& suffix, texAlloc_t alloc);
		GEN::Pointer<Texture> Get(const std::string& filename, FS::Loader& loader = FS::Disk::Instance());
	};

	struct RegisterTexAlloc {
		RegisterTexAlloc(const std::string& suffix, texAlloc_t alloc) {
			TextureManager::Instance().RegisterAlloc(suffix, alloc);
		}
	};

} // namespace R