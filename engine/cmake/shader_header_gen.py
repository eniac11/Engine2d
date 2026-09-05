#!/usr/bin/python3
import json
import os.path
import textwrap
import sys
from typing import TextIO

GL_VERBATIM_HEADER = """
#pragma once
#include <array>
#include <exception>
#include <string>
#include <span>
#include <GL/glew.h>
#include <engine/ShaderResources.h>

"""

shader_name = "Linked_Shader"

hello = {
    "entryPoints" : [
        {
            "name" : "main",
            "mode" : "vert"
        },
        {
            "name" : "main",
            "mode" : "frag"
        }
    ],
    "inputs" : [
        {
            "type" : "vec3",
            "name" : "aPos",
            "location" : 0
        }
    ],
    "uniforms" : [
        {
            "type" : "vec3",
            "name" : "ourColour",
            "location" : 0,
            "set" : 0,
            "binding" : 0
        }
    ]
}

gl_entrypoints = {
    "vert": "GL_VERTEX_SHADER",
    "frag": "GL_FRAGMENT_SHADER",
    "comp": "GL_COMPUTE_SHADER"
}

gl_uniform_sizes: dict[str, int] = {
    "float": 1,
    "int": 1,
    "uint": 1,
    "bool": 1,
    "vec2": 1,
    "ivec2": 1,
    "vec3": 1,
    "vec4": 1,
    "mat2": 2,
    "mat3": 3,
    "mat4": 4,
    "sampler2D": 1,
    "sampler2DArray": 1
}
gl_ncomponents: dict[str, int] = {
    "float": 1,
    "int": 1,
    "uint": 1,
    "bool": 1,
    "vec2": 2,
    "ivec2": 2,
    "vec3": 3,
    "vec4": 4,
    "mat2": 4,
    "mat3": 4,
    "mat4": 4,
    "sampler2D": 1,
    "sampler2DArray": 1
}

mode_names_map = {
    "vert": "vertex",
    "frag": "fragment",
    "comp": "compute"
}

def convert_snake_case(s: str) -> str:
    current_word = ""
    l = []
    for c in s:
        if c.isupper():
            l.append(current_word)
            current_word = ""
        current_word += c.lower()
    if len(current_word) > 0:
        l.append(current_word)
    return "_".join(l)


def compute_relative_location_offsets_for_structs(types: dict):
    """Calculate the number of locations used by a struct and relative locations for its members
    """
    for key, value in types.items():
        struct_name: str = value["name"]
        if struct_name.startswith("gl_"):
            continue
        location_offset = 0
        for member in value["members"]:
            typ: str = member["type"]
            member["location"] = location_offset
            # FIXME: Account for nested types.
            # FIXME: Figure out what do do with UBO's and SSBO's and glsl includes with structures as
            #        structure could be generated.
            if typ not in types:
                location_offset += gl_uniform_sizes[typ]
        value["locations"] = location_offset

# def flatten_resouce(resource, types: dict)

def flatten_resources(resources: list[dict], types: dict):
    new_resources = []
    for resource in resources:
        resource_name: str = resource["name"]
        cpp_name = resource_name
        location = resource["location"]
        type_: str = resource['type']
        if type_ in types:
            new_resources.extend(list(generate_struct(resource_name, location, types, types[type_])))
        else:
            new_resources.append(resource)
    return new_resources


def generate_struct(var_name: str, location: int, types: dict, struct: dict):
    resources = []
    for member in struct["members"]:
        member_name: str = member["name"]
        member_location: int = member["location"]
        type_ = member["type"]
        new_name = var_name+"."+member["name"]
        new_location = location+member_location

        if type_ in types:
            yield from generate_struct(new_name, new_location, types, types[type_])
        else:
            yield {"type": member["type"], "location": new_location, "name": new_name}



def generate_resource(resource: dict) -> str:
    resource_name: str = resource["name"]
    cpp_name = resource_name
    location = resource["location"]
    type_: str = resource['type']
    ncomponents = gl_ncomponents[type_]
    if "." in resource_name:
        cpp_name = resource_name.replace(".", "__")
    return f'static constexpr Resource {cpp_name}{{"{type_}", "{resource_name}", {location}, {ncomponents}}};'

def generate_entrypoint(mode: str, name: str) -> str:
    shader_name = mode_names_map[mode]
    entrypoint_type = gl_entrypoints[mode]
    return f'static constexpr EntryPoint {shader_name}{{{entrypoint_type}, "{name}"}};'

def generate_span_function(span_name, array_name, struct_class, exists=True):
    extra = ""
    extra2 = array_name
    if not exists:
        extra =", 0"
        extra2 = '{}'

    return f"""static constexpr std::span<const {struct_class}> {span_name}() {{ return std::span<const {struct_class}{extra}>({extra2}); }}\n"""

def generate_resource_struct(name, resources: list[dict], types: dict):
    new_resources = flatten_resources(resources, types)
    instance_name = name.title()
    struct_name = instance_name+"Type"
    array_name = instance_name+"Res"
    resource_count = len(new_resources)
    resource_lines = [generate_resource(resource) for resource in new_resources]
    resource_def = "\n".join(resource_lines)

    resource_struct = f"""static constexpr struct {struct_name} {{
    {resource_def}
}} {instance_name}{{}};
"""
    resource_links = ", ".join([struct_name +"::" + resource["name"].replace(".", "__") for resource in new_resources])
    resource_array = f"static constexpr std::array<Resource, {resource_count}> {array_name}{{{resource_links}}};\n"
    resource_span = generate_span_function(convert_snake_case(name), array_name, "Resource")
    return resource_struct+resource_array+resource_span

def generate_entrypoint_struct(resources: list[dict]):

    resource_count = len(resources)
    resource_lines = [generate_entrypoint(resource["mode"], resource["name"]) for resource in resources]
    resource_def = "\n".join(resource_lines)

    resource_struct = f"""static constexpr struct EntryPointsType {{
    {resource_def}
}} EntryPoints{{}};
"""
    resource_links = ", ".join(["EntryPointsType::" + mode_names_map[resource["mode"]] for resource in resources])
    resource_array = f"static constexpr std::array<EntryPoint, {resource_count}> EntryPointsRes{{{resource_links}}};\n"
    resource_entry_point = generate_span_function("entry_points", "EntryPointsRes", "EntryPoint")
    return resource_struct+resource_array+resource_entry_point


def generate_get_function(name: str) -> str:
    s = name[:-1]
    return f""" static consteval Resource const& get_{s}_location(const std::string& name) {{
    for (const auto& resource : {name.title()}Res) {{
        if (resource.name == name) {{
            return resource;
        }}
    }}
    throw unknown_location("unknown location name");
}}"""

def generate_class(class_name, extern_name: str, data: dict, file: TextIO):
    class_template = GL_VERBATIM_HEADER+"""
  
extern unsigned char {extern_name}[];
extern const unsigned int {extern_name}_len;
struct {class_name} {{
    static constexpr const unsigned char* const program = {extern_name};
    static constexpr const unsigned int& size = {extern_name}_len;
    
    {structs}
}};
"""
    ssbos = data.pop("ssbos", None)
    ubos = data.pop("ubos", None)
    types = data.pop("types", None)
    compute_relative_location_offsets_for_structs(types)
    entrypoints = data.pop("entryPoints")
    # generate_entrypoint_struct(entrypoints))
    structs = generate_entrypoint_struct(entrypoints)+"\n"

    for key, value in data.items():
#         instance_name = key.title()
#         struct_name = instance_name+"Type"
#         array_name = instance_name+"Res"
#         resource_count = len(value)
#         resource_lines = [generate_resource(resource) for resource in value]
#         resource_def = "\n".join(resource_lines)
#
#         print(f"""static constexpr struct {struct_name} {{
#     {resource_def}
# }} {instance_name}{{}};""")
#         resource_links = ", ".join([struct_name+"::"+resource["name"] for resource in value])
#         print(f"static constexpr std::array<Resource, {resource_count}> {array_name}{{{resource_links}}};")
#         print()
        structs += generate_resource_struct(key, value, types) + "\n\n"
        structs += generate_get_function(key) + "\n\n"
        # print()
    s = class_template.format(extern_name=extern_name, class_name=class_name, structs=textwrap.indent(structs, " "))
    file.write(s)

# commandline: main.py json_file class_name extern_name [FILE|stdout]
# generate_class("Linked_Shader", "linked_spv", hello)
if len(sys.argv[1:]) < 3:
    print("expected 3 or 4 arguments", file=sys.stderr)
    exit(1)

json_file = sys.argv[1]
if not os.path.exists(json_file):
    print("JSON reflection data does not exist.", json_file, file=sys.stderr)
    exit(1)
with open(json_file) as f:
    data = json.load(f)
clazz_name = sys.argv[2]
extern_name_ = sys.argv[3]
file = sys.stdout
if len(sys.argv[1:]) > 3:
    # os.unlink(sys.argv[4])
    file = open(sys.argv[4], "w")
    file.seek(0, os.SEEK_SET)

generate_class(clazz_name, extern_name_, data, file)
