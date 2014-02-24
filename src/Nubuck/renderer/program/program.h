#pragma once

#include <string>
#include <unordered_map>
#include <Nubuck\generic\uncopyable.h>
#include <renderer\glew\glew.h>
#include <renderer\shader\shader.h>

namespace M {
    struct Vector3;
    struct Matrix3;
    struct Matrix4;
};

namespace R {

    class Shader;
    struct Color;

    class Program : GEN::Uncopyable {
    private:
        GLuint  _id;
        bool    _linked;
        std::unordered_map<std::string, GLint> _uniformLocations;

        void BindAttributeLocations(const std::vector<AttributeLocation>& locs);
    public:
        Program(void);
        ~Program(void);

        void Init(void); // required gl context

        GLuint  GetID(void) const { return _id; }
        void    Use(void) const;

        void Attach(const Shader& shader);
        void Attach(int type /* in Shader::Type */, const GLchar* source);

        void Link(void);
        void Use(void);

        GLint GetUniformLocation(const GLchar* name);
        GLint GetAttributeLocation(const GLchar* name);

        void SetUniform(const char* name, int value);
        void SetUniform(const char* name, float value);
        void SetUniform(const char* name, const M::Vector3& vector);
        void SetUniform(const char* name, const M::Matrix3& matrix);
        void SetUniform(const char* name, const M::Matrix4& matrix);
        void SetUniform(const char* name, const R::Color& color);
        
        static void UnbindAll(void);
    };

} // namespace R
