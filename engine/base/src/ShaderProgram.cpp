#include "engine/ShaderProgram.h"
#include "engine_p/logging_categories.h"

#include <print>
#include <ostream>



std::vector<ShaderProgram::UBOHandle>& ShaderProgram::get_ubos() {
    return m_ubos;
}

