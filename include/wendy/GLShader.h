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

class ShaderPermutation;
class ShaderProgram;

///////////////////////////////////////////////////////////////////////

/*! @brief Base class for OpenGL Shading Language shader objects.
 *  @ingroup opengl
 */
class Shader
{
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
  Shader(Type type, const String& text);
  bool isUsingLighting(void) const;
  /*! @return The type of this GLSL shader.
   */
  Type getType(void) const;
  const String& getText(void) const;
protected:
  Type type;
  String text;
  bool lighting;
};

///////////////////////////////////////////////////////////////////////

/*! @brief OpenGL Shading Language vertex shader object.
 *  @ingroup opengl
 */
class VertexShader : public Shader,
                     public RefObject<VertexShader>,
		     public Resource<VertexShader>
{
public:
  static VertexShader* createInstance(const String& text, const String& name = "");
private:
  VertexShader(const String& text, const String& name);
};

///////////////////////////////////////////////////////////////////////

/*! @brief OpenGL Shading Language fragment shader object.
 *  @ingroup opengl
 */
class FragmentShader : public Shader,
		       public RefObject<FragmentShader>,
                       public Resource<FragmentShader>
{
public:
  static FragmentShader* createInstance(const String& text, const String& name = "");
private:
  FragmentShader(const String& text, const String& name);
};

///////////////////////////////////////////////////////////////////////

/*! @brief GLSL program vertex attribute.
 *  @ingroup opengl
 *
 *  This class describes a single GLSL program vertex attribute.
 */
class ShaderAttribute
{
  friend class ShaderPermutation;
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
  /*! @return The GLSL program permutation that contains this attribute.
   */
  ShaderPermutation& getPermutation(void) const;
private:
  ShaderAttribute(ShaderPermutation& permutation);
  ShaderAttribute(const ShaderAttribute& source);
  ShaderAttribute& operator = (const ShaderAttribute& source);
  ShaderPermutation& permutation;
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
  friend class ShaderPermutation;
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
  /*! @return The GLSL program permutation that contains this uniform.
   */
  ShaderPermutation& getPermutation(void) const;
private:
  ShaderUniform(ShaderPermutation& permutation);
  ShaderUniform(const ShaderUniform& source);
  ShaderUniform& operator = (const ShaderUniform& source);
  ShaderPermutation& permutation;
  typedef std::vector<GLint> LocationList;
  String name;
  Type type;
  unsigned int count;
  LocationList locations;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
class ShaderPermutation
{
  friend class ShaderProgram;
public:
  bool isValid(void) const;
  const String& getName(void) const;
  unsigned int getUniformCount(void) const;
  ShaderUniform& getUniform(unsigned int index);
  const ShaderUniform& getUniform(unsigned int index) const;
  ShaderUniform* getUniform(const String& name);
  const ShaderUniform* getUniform(const String& name) const;
  unsigned int getAttributeCount(void) const;
  ShaderAttribute& getAttribute(unsigned int index);
  const ShaderAttribute& getAttribute(unsigned int index) const;
  ShaderAttribute* getAttribute(const String& name);
  const ShaderAttribute* getAttribute(const String& name) const;
  ShaderProgram& getProgram(void) const;
  static ShaderPermutation* getCurrent(void);
private:
  ShaderPermutation(ShaderProgram& program);
  ~ShaderPermutation(void);    
  bool init(const LightState& state);
  bool apply(void) const;
  static void applyFixedFunction(void);
  GLhandleARB createShader(const Shader& shader, const LightState& state);
  bool createUniforms(void);
  bool createAttributes(void);
  typedef std::vector<ShaderUniform*> UniformList;
  typedef std::vector<ShaderAttribute*> AttributeList;
  ShaderProgram& program;
  GLhandleARB programID;
  GLhandleARB vertexID;
  GLhandleARB fragmentID;
  UniformList uniforms;
  AttributeList attributes;
  String name;
  static ShaderPermutation* current;
};

///////////////////////////////////////////////////////////////////////

/*! @ingroup opengl
 */
class ShaderProgram : public Resource<ShaderProgram>,
                      public RefObject<ShaderProgram>
{
public:
  ~ShaderProgram(void);
  bool apply(void);
  bool isUsingLighting(void) const;
  const VertexShader& getVertexShader(void) const;
  const FragmentShader& getFragmentShader(void) const;
  SignalProxy1<void, ShaderPermutation&> getPermutationCreatedSignal(void);
  SignalProxy1<void, ShaderPermutation&> getPermutationAppliedSignal(void);
  static ShaderProgram* createInstance(VertexShader& vertexShader,
				       FragmentShader& fragmentShader,
				       const String& name = "");
  static void applyFixedFunction(void);
private:
  ShaderProgram(const String& name);
  bool init(VertexShader& vertexShader, FragmentShader& fragmentShader);
  ShaderPermutation* createPermutation(const LightState& state);
  ShaderPermutation* findPermutation(const String& name);
  const ShaderPermutation* findPermutation(const String& name) const;
  typedef std::vector<ShaderPermutation*> PermutationList;
  Ref<VertexShader> vertexShader;
  Ref<FragmentShader> fragmentShader;
  PermutationList permutations;
  Signal1<void, ShaderPermutation&> createdSignal;
  Signal1<void, ShaderPermutation&> appliedSignal;
};

///////////////////////////////////////////////////////////////////////

  } /*namespace GL*/
} /*namespace wendy*/

///////////////////////////////////////////////////////////////////////
#endif /*WENDY_GLSHADER_H*/
///////////////////////////////////////////////////////////////////////
