

#include <glm/gtc/type_ptr.hpp>

#undef UNIFORM_FUNC
#define UNIFORM_FUNC(n,suffix, ARGS, PREFIX, VARS...) void OpenGLShaderProgram::set_uniform(GLint location, ARGS) body(n, suffix, VARS)
#undef UNIFORM_PVALUE
#define UNIFORM_PVALUE(n, suffix, ARGS, PREFIX, VARS...) void OpenGLShaderProgram::set_uniform(GLint location, ARGS) bodyv(n, suffix, VARS)

#define body(n, suffix,VARS...) {\
elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Setting Program {3} Uniform '{1}{2}': {0}", location, n, #suffix, m_id);\
glProgramUniform##n##suffix(m_id, location, ##VARS);\
}
#define bodyv(n, suffix, VARS...) {\
elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Setting Program {3} Uniform '{2}vec{1}': {0}", location, n, #suffix, m_id);\
    glProgramUniform##n##suffix##v(m_id, location, 1, ##VARS); \
    }
#define mat_body(n, suffix) {\
    glProgramUniformMatrix##n##suffix(m_id, location, 1, GL_FALSE, glm::value_ptr(v1)); \
    }


UNIFORM_FUNC1(GLuint, ui)
UNIFORM_FUNC1(GLint, i)
UNIFORM_FUNC1(GLboolean, i)
UNIFORM_FUNC1(GLfloat, f)
UNIFORM_PVALUE1(glm::vec3, f)

void OpenGLShaderProgram::set_uniform(GLint location, const glm::mat4 &v1) {
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Setting Program {1} Uniform 'mat4': {0}", location, m_id);

    glProgramUniformMatrix4fv(m_id, location, 1, GL_FALSE, glm::value_ptr(v1));
}


UNIFORM_FUNC3(GLuint, ui)
UNIFORM_FUNC3(GLfloat, f)
