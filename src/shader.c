#include "shader.h"

#include "logger.h"
#include "vkinit_utils.h"

#include <errno.h>
#include <string.h>

bool compile_shader_file(const char *filename, const ShaderKind kind, shaderc_compilation_result_t *result) {

	errno         = 0;
	FILE *sd_file = fopen(filename, "r");
	if (sd_file == NULL || errno != 0) {
		llog(LOG_ERROR, "[SHADER] Could not read the shader file '%s': %s\n", filename, strerror(errno));
		if (sd_file != NULL) {
			fclose(sd_file);
		}
		return false;
	}

	llog(LOG_DEBUG, "[SHADER] Compiling shader file: %s\n", filename);

	fseek(sd_file, 0, SEEK_END);
	auto sz = (unsigned long)(ftell(sd_file));

	char *code = malloc(sz);
	if (code == NULL) {
		llog(LOG_FATAL, "[MEM] 'malloc' failed: %s\n", strerror(errno));
		fclose(sd_file);
		return false;
	}

	fseek(sd_file, 0, SEEK_SET);
	fread(code, 1, sz, sd_file);

	if (ferror(sd_file)) {
		llog(LOG_ERROR, "[SHADER] Could read from file '%s'\n", filename);
		fclose(sd_file);
		free(code);
		return false;
	}

	fclose(sd_file);

	auto res = compile_shader(code, sz, kind, result);

	free(code);

	return res;
}

bool compile_shader(const char *code, const size_t size, const ShaderKind kind, shaderc_compilation_result_t *result) {

	shaderc_shader_kind _kind = shaderc_vertex_shader;

	switch (kind) {
	case VERTEX_SHADER:
		_kind = shaderc_vertex_shader;
		break;

	case TESSELATION_SHADER:
		_kind = shaderc_tess_evaluation_shader;
		break;

	case GEOMETRY_SHADER:
		_kind = shaderc_geometry_shader;
		break;

	case FRAGMENT_SHADER:
		_kind = shaderc_fragment_shader;
		break;

	case COMPUTE_SHADER:
		_kind = shaderc_compute_shader;
		break;
	}

	auto compiler = shaderc_compiler_initialize();
	*result       = shaderc_compile_into_spv(compiler, code, size, _kind, "internal_compilation", "main", nullptr);

	auto c_status = shaderc_result_get_compilation_status(*result);

	if (c_status != shaderc_compilation_status_success) {
		llog(LOG_ERROR, "[SHADER] Could not compile shader: %s\n", shaderc_result_get_error_message(*result));
		return false;
	}

	shaderc_compiler_release(compiler);

	return true;
}

bool release_shader(shaderc_compilation_result_t res) {
	shaderc_result_release(res);

	return true;
}

bool create_shader_module(const char *filename, const ShaderKind shader_kind, const VkDevice logical_dev, VkShaderModule *module, shaderc_compilation_result_t *shaderc_result) {

	if (compile_shader_file(filename, shader_kind, shaderc_result)) {
		VkShaderModuleCreateInfo sh_create = {};
		sh_create.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		sh_create.codeSize                 = shaderc_result_get_length(*shaderc_result);
		sh_create.pCode                    = (const uint32_t *)(shaderc_result_get_bytes(*shaderc_result));

		auto res = vkCreateShaderModule(logical_dev, &sh_create, nullptr, module);
		if (res != VK_SUCCESS) {
			llog(LOG_FATAL, "[SHADER] Could not create vulkan shader module (%s): %s\n", ShaderKind_str(shader_kind), VkResult_str(res));
		}
		return true;
	}
	return false;
}

bool destroy_shader_module(const VkDevice logical_dev, shaderc_compilation_result_t shaderc_result, VkShaderModule shader_module) {
	vkDestroyShaderModule(logical_dev, shader_module, nullptr);
	release_shader(shaderc_result);

	return true;
}

