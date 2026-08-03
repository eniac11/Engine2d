#ifndef PREFIX
#define PREFIX
#endif

#ifndef OVERRIDE

#ifndef UNIFORM_FUNC
#define UNIFORM_FUNC(n,suffix, ARGS, PREFIX, VARS...) virtual void set_uniform(GLint location, ARGS) = 0; \
    virtual void set_uniform(Resource const& resource, ARGS) = 0
#endif

#ifndef UNIFORM_PVALUE
#define UNIFORM_PVALUE(n, suffix, ARGS, PREFIX, VARS...) virtual void set_uniform(GLint location, ARGS) = 0; \
    virtual void set_uniform(Resource const& resource, ARGS) = 0
#endif

#else

#ifndef UNIFORM_FUNC
#define UNIFORM_FUNC(n,suffix, ARGS, PREFIX, VARS...) void set_uniform(GLint location, ARGS) override; \
    void set_uniform(Resource const& resource, ARGS) override
#endif

#ifndef UNIFORM_PVALUE
#define UNIFORM_PVALUE(n, suffix, ARGS, PREFIX, VARS...) void set_uniform(GLint location, ARGS) override; \
    void set_uniform(Resource const& resource, ARGS) override
#endif

#endif

#define X(type, v) const type& v
#define X1(type) X(type, v1)
#define XV1 v1
#define UNIFORM_FUNC1(type, suffix) UNIFORM_FUNC(1, suffix, X1(type), PREFIX, XV1)
#define X3(type) X(type, v1), X(type, v2), X(type, v3)
#define XV3 v1, v2, v3
#define UNIFORM_FUNC3(type, suffix) UNIFORM_FUNC(3, suffix, X3(type), PREFIX, XV3)
#define PXV1 glm::value_ptr(v1)
#define UNIFORM_PVALUE1(type, suffix) UNIFORM_PVALUE(3, suffix, X1(type), PREFIX, PXV1)




// UNIFORM_FUNC(1, ui, const GLuint v1,v1)
UNIFORM_FUNC1(GLuint, ui);
UNIFORM_FUNC1(GLint, i);
UNIFORM_FUNC1(GLboolean, i);
UNIFORM_FUNC1(GLfloat, f);
UNIFORM_PVALUE1(glm::vec3, fv);
UNIFORM_PVALUE1(glm::mat4, d);

UNIFORM_FUNC3(GLuint, ui);
UNIFORM_FUNC3(GLfloat, f);


