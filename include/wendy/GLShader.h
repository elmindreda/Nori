///////////////////////////////////////////////////////////////////////
// Wendy OpenGL library
// Copyright (c) 2006 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any
// damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any
// purpose, including commercial applications, and to alter it and
// redistribute it freely, subject to the following restrictions:
//
//  1. The origin of this software must not be misrepresented; you
//     must not claim that you wrote the original software. If you use
//     this software in a product, an acknowledgment in the product
//     documentation would be appreciated but is not required.
//
//  2. Altered source versions must be plainly marked as such, and
//     must not be misrepresented as being the original software.
//
//  3. This notice may not be removed or altered from any source
//     distribution.
//
///////////////////////////////////////////////////////////////////////
#ifndef WENDY_GLSHADER_H
#define WENDY_GLSHADER_H
///////////////////////////////////////////////////////////////////////

namespace wendy
{
  namespace GL
  {
  
///////////////////////////////////////////////////////////////////////

using namespace moira;

///////////////////////////////////////////////////////////////////////

class ShaderProgram;

///////////////////////////////////////////////////////////////////////

/*! @brief Base class for OpenGL Shading Language shader objects.
 *  @ingroup opengl
 */
class Shader : public RefObject<Shader>
{
  friend class ShaderProgram;
public:
  /*! Shader type enumeration.
   */
  enum Type
  {
    /*! Vertex shader.
     */
    VERTEX,
    /*! Fragment shader.
     */
    FRAGMENT,
  };
  /*! Destructor.
   */
  virtual ~Shader(void);
  /*! @return The type of this GLSL shader.
   */
  Type getType(void) const;
protected:
  Shader(Type type);
  bool init(const String& text);
  GLhandleARB shaderID;
  Type type;
};

///////////////////////////////////////////////////////////////////////

typedef Ref<Shader> ShaderRef;

///////////////////////////////////////////////////////////////////////

/*! @brief OpenGL Shading Language vertex shader object.
 *  @ingroup opengl
 */
class VertexShader : public Shader, public Resource<VertexShader>
{
public:
  static VertexShader* createInstance(const Path& path,
                                      const String& name = "");
  static VertexShader* createInstance(const String& text,
                                      const String& name = "");
private:
  VertexShader(const String& name);
};

///////////////////////////////////////////////////////////////////////

/*! @brief OpenGL Shading Language fragment shader object.
 *  @ingroup opengl
 */
class FragmentShader : public Shader, public Resource<FragmentShader>
{
public:
  static FragmentShader* createInstance(const Path& path,
                                        const String& name = "");
  static FragmentShader* createInstance(const String& text,
                                        const String& name = "");
private:
  FragmentShader(const String& name);
};

///////////////////////////////////////////////////////////////////////

/*! @brief GLSL program vertex attribute.
 *  @ingroup opengl
 *
 *  This class describes a single GLSL program vertex attribute.
 */
class ShaderAttribute
{
  friend class ShaderProgram;
public:
  /*! Shader attribute type enumeration.
   */
  enum Type
  {
    FLOAT = GL_FLOAT,
    FLOAT_VEC2 = GL_FLOAT_VEC2_ARB,
    FLOAT_VEC3 = GL_FLOAT_VEC3_ARB,
    FLOAT_VEC4 = GL_FLOAT_VEC4_ARB,
    FLOAT_MAT2 = GL_FLOAT_MAT2_ARB,
    FLOAT_MAT3 = GL_FLOAT_MAT3_ARB,
    FLOAT_MAT4 = GL_FLOAT_MAT4_ARB,
  };
  bool isArray(void) const;
  /*! @return @c true if this attribute is a vector, otherwise @c false.
   */
  bool isVector(void) const;
  /*! @return @c true if this attribute is a matrix, otherwise @c false.
   */
  bool isMatrix(void) const;
  /*! @return The type of this attribute.
   */
  Type getType(void) const;
  /*! @return The name of this attribute.
   */
  const String& getName(void) const;
  unsigned int getIndex(void) const;
  unsigned int getElementCount(void) const;
  /*! @return The GLSL program that contains this attribute.
   */
  ShaderProgram& getProgram(void) const;
private:
  ShaderAttribute(ShaderProgram& program);
  ShaderAttribute(const ShaderAttribute& source);
  ShaderAttribute& operator = (const ShaderAttribute& source);
  ShaderProgram& program;
  String name;
  Type type;
  unsigned int count;
  unsigned int index;
};

///////////////////////////////////////////////////////////////////////

/*! @brief GLSL program uniform.
 *  @ingroup opengl
 *
 *  This class describes a single writable GLSL program uniform, or an array of
 *  writable uniforms.
 */
class ShaderUniform
{
  friend class ShaderProgram;
public:
  /*! Shader uniform type enumeration.
   */
  enum Type
  {
    INT = GL_INT,
    BOOL = GL_BOOL,
    FLOAT = GL_FLOAT,
    INT_VEC2 = GL_INT_VEC2_ARB,
    INT_VEC3 = GL_INT_VEC3_ARB,
    INT_VEC4 = GL_INT_VEC4_ARB,
    BOOL_VEC2 = GL_BOOL_VEC2_ARB,
    BOOL_VEC3 = GL_BOOL_VEC3_ARB,
    BOOL_VEC4 = GL_BOOL_VEC4_ARB,
    FLOAT_VEC2 = GL_FLOAT_VEC2_ARB,
    FLOAT_VEC3 = GL_FLOAT_VEC3_ARB,
    FLOAT_VEC4 = GL_FLOAT_VEC4_ARB,
    FLOAT_MAT2 = GL_FLOAT_MAT2_ARB,
    FLOAT_MAT3 = GL_FLOAT_MAT3_ARB,
    FLOAT_MAT4 = GL_FLOAT_MAT4_ARB,
    SAMPLER_1D = GL_SAMPLER_1D_ARB,
    SAMPLER_2D = GL_SAMPLER_2D_ARB,
    SAMPLER_3D = GL_SAMPLER_3D_ARB,
    SAMPLER_CUBE = GL_SAMPLER_CUBE_ARB,
    SAMPLER_1D_SHADOW = GL_SAMPLER_1D_SHADOW_ARB,
    SAMPLER_2D_SHADOW = GL_SAMPLER_2D_SHADOW_ARB,
  };
  bool isArray(void) const;
  /*! @return @c true if this uniform is a vector, otherwise @c false.
   */
  bool isVector(void) const;
  /*! @return @c true if this uniform is a matrix, otherwise @c false.
   */
  bool isMatrix(void) const;
  /*! @return @c true if this uniform is a sampler, otherwise @c false.
   */
  bool isSampler(void) const;
  /*! @return The type of this uniform.
   */
  Type getType(void) const;
  /*! @return The name of this uniform.
   */
  const String& getName(void) const;
  unsigned int getElementCount(void) const;
  void setValue(int newValue, unsigned int index = 0);
  void setValue(bool newValue, unsigned int index = 0);
  void setValue(float newValue, unsigned int index = 0);
  void setValue(const Vector2& newValue, unsigned int index = 0);
  void setValue(const Vector3& newValue, unsigned int index = 0);
  void setValue(const Vector4& newValue, unsigned int index = 0);
  void setValue(const Matrix2& newValue, unsigned int index = 0);
  void setValue(const Matrix3& newValue, unsigned int index = 0);
  void setValue(const Matrix4& newValue, unsigned int index = 0);
  /*! @return The GLSL program that contains this uniform.
   */
  ShaderProgram& getProgram(void) const;
private:
  ShaderUniform(ShaderProgram& program);
  ShaderUniform(const ShaderUniform& source);
  ShaderUniform& operator = (const ShaderUniform& source);
  ShaderProgram& program;
  typedef std::vector<GLint> LocationList;
  String name;
  Type type;
  unsigned int count;
  LocationList locations;
};

///////////////////////////////////////////////////////////////////////

/*! @brief GLSL program.
 *  @ingroup opengl
 *
 *  This class represents a single GLSL program, i.e. a collection of shaders
 *  and associated uniforms.
 */
class ShaderProgram : public Resource<ShaderProgram>
{
public:
  ~ShaderProgram(void);
  void addShader(Shader& shader);
  void removeShader(Shader& shader);
  /*! Links all shaders added to this program.
   *  @return @c true if the link was successful, otherwise @c false.
   */
  bool link(void);
  /*! Makes this the current GLSL program.
   *  @return @c true if successful, otherwise @c false.
   */
  bool apply(void) const;
  /*! @return @c true if this program is successfully validated, otherwise @c
   *  false.
   */
  bool isValid(void) const;
  /*! @return @c true if this program needs to be relinked, otherwise @c false.
   */
  bool isModified(void) const;
  /*! @return The number of shaders added to this program.
   */
  unsigned int getShaderCount(void) const;
  /*! @param index The index of the desired shader.
   *  @return The shader at the specified index.
   */
  Shader& getShader(unsigned int index);
  /*! @param index The index of the desired shader.
   *  @return The shader at the specified index.
   */
  const Shader& getShader(unsigned int index) const;
  /*! @return The number of active uniforms in this program.
   */
  unsigned int getUniformCount(void) const;
  /*! @param index The index of the desired uniform.
   *  @return The uniform at the specified index.
   */
  ShaderUniform& getUniform(unsigned int index);
  /*! @param index The index of the desired uniform.
   *  @return The uniform at the specified index.
   */
  const ShaderUniform& getUniform(unsigned int index) const;
  /*! @param name The name of the desired uniform.
   *  @return The desired uniform, or @c NULL if no such uniform exists.
   */
  ShaderUniform* getUniform(const String& name);
  /*! @param name The name of the desired uniform.
   *  @return The desired uniform, or @c NULL if no such uniform exists.
   */
  const ShaderUniform* getUniform(const String& name) const;
  /*! Creates a GLSL program with the specified name.
   *  @param name The desired name of the program, or the empty string to
   *  automatically generate a name.
   */
  static ShaderProgram* createInstance(const String& name = "");
  /*! Disables any current GLSL program.
   */
  static void applyFixedFunction(void);
  /*! @return The current GLSL program, or @c NULL
   *  if no program is currently in use.
   */
  static ShaderProgram* getCurrent(void);
private:
  ShaderProgram(const String& name);
  bool init(void);
  bool createUniforms(void);
  bool createAttributes(void);
  void destroyUniforms(void);
  void destroyAttributes(void);
  typedef std::vector<ShaderRef> ShaderList;
  typedef std::vector<ShaderUniform*> UniformList;
  typedef std::vector<ShaderAttribute*> AttributeList;
  bool modified;
  ShaderList shaders;
  UniformList uniforms;
  AttributeList attributes;
  GLhandleARB programID;
  static ShaderProgram* current;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLSHADER_H*/
///////////////////////////////////////////////////////////////////////
