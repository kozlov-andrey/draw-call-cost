#include "GLRenderer.h"

#include <vector>
#include <stdlib.h>

namespace
{

struct Vertex final
{
	Vertex() = delete;
	float x;
	float y;
	float z;
};

std::vector<Vertex> generate_vertices()
{
	std::vector<Vertex> vertices;

	const int x_count = 5;
	const int y_count = 4;
	
	for (int x_counter = 0; x_counter < x_count; ++x_counter)
	{
		for (int y_counter = 0; y_counter < y_count; ++y_counter)
		{
			vertices.push_back({-1.f + x_counter * (2.0f / (x_count - 1)), -1.f + y_counter * (2.0f / (y_count - 1)), 0.f});
		}
	}
	return vertices;
}

std::vector<unsigned short> generate_indices(const unsigned short vertex_count, unsigned triangle_count)
{
	std::vector<unsigned short> indices;

	for (unsigned i = 0; i < triangle_count; ++i)
	{
		indices.push_back(rand() % vertex_count);
		indices.push_back(indices.back());
		indices.push_back(rand() % vertex_count);
	}

	return indices;
}

GLuint LoadShader(GLenum type, const char *shaderSrc)
{
	GLuint shader;
	GLint compiled;

	shader = glCreateShader(type);

	if (shader == 0)
		return 0;

	glShaderSource(shader, 1, &shaderSrc, NULL);

	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		GLint infoLen = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

GLuint create_program()
{
	const char * vShaderStr =
		R"(
uniform mat4 g_matrix;
attribute vec4 vPosition;
void main()
{
	gl_Position = g_matrix * vPosition;
}
)";

	const char * fShaderStr =
		R"(
uniform vec4 g_color;
void main()
{
	gl_FragColor = g_color;
}
)";

	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint programObject;
	GLint linked;

	vertexShader = LoadShader(GL_VERTEX_SHADER, vShaderStr);
	fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fShaderStr);

	programObject = glCreateProgram();

	glAttachShader(programObject, vertexShader);
	glAttachShader(programObject, fragmentShader);

	glBindAttribLocation(programObject, 0, "vPosition");

	glLinkProgram(programObject);

	glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		GLint infoLen = 0;

		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);
		glDeleteProgram(programObject);
	}

	return programObject;
}

} // namespace

GLRenderer::GLRenderer(int width, int height)
	: width_(width)
	, height_(height)
{
	{
		program_object_[0] = create_program();
		Matrix matrix;
		State state;
		glUseProgram(program_object_[0]);
		color_location_[0] = glGetUniformLocation(program_object_[0], "g_color");
		matrix_location_[0] = glGetUniformLocation(program_object_[0], "g_matrix");
		glUniformMatrix4fv(matrix_location_[0], 1, false, matrix.values);
		glUniform4fv(color_location_[0], 1, (float*)&state.color);
	}
	{
		program_object_[1] = create_program();
		Matrix matrix;
		State state;
		glUseProgram(program_object_[1]);
		color_location_[1] = glGetUniformLocation(program_object_[1], "g_color");
		matrix_location_[1] = glGetUniformLocation(program_object_[1], "g_matrix");
		glUniformMatrix4fv(matrix_location_[1], 1, false, matrix.values);
		glUniform4fv(color_location_[1], 1, (float*)&state.color);
	}
}

void GLRenderer::Setup(const unsigned tri_per_batch, const unsigned tri_per_frame, ChangeStatePolicy policy)
{
	change_state_policy_ = policy;

	glDeleteBuffers(2, vbo_);
	glDeleteBuffers(2, ibo_);

	draw_calls_.clear();

	const auto vertices = generate_vertices();
	const auto indices = generate_indices((unsigned short)vertices.size(), tri_per_batch);
	index_count_ = indices.size();

	glGenBuffers(2, vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glGenBuffers(2, ibo_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), indices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * indices.size(), indices.data(), GL_STATIC_DRAW);

	for (int i = 0; i < (tri_per_frame / tri_per_batch); ++i)
	{
		draw_calls_.emplace_back();
		auto &call = draw_calls_.back();
		call.vbo = vbo_[0];
		call.ibo = ibo_[0];
		call.index_count = tri_per_batch * 3;
		call.offset_ = 0;
		if (i % 2)
		{
			call.state.martrix.SetTranslation(0.5f, 0.f, 0.f);
			call.state.alpha_blending_enabled = true;
			call.state.depth_test_enabled = true;
			call.state.depth_write_enabled = true;
			call.state.color = Color{ 1.0f,0.0f,1.0f,1.0f };
			call.state.program = program_object_[0];
			call.state.color_location = color_location_[0];
			call.state.matrix_location = matrix_location_[0];
			call.vbo = vbo_[0];
			call.ibo = ibo_[0];
		}
		else
		{
			call.state.martrix.SetTranslation(-0.5f, 0.f, 0.f);
			call.state.alpha_blending_enabled = false;
			call.state.depth_test_enabled = false;
			call.state.depth_write_enabled = false;
			call.state.color = Color{ 0.0f, 1.0f, 0.0f,1.0f };
			call.state.program = program_object_[1];
			call.state.color_location = color_location_[1];
			call.state.matrix_location = matrix_location_[1];
			call.vbo = vbo_[1];
			call.ibo = ibo_[1];
		}
	}
}

void GLRenderer::Draw()
{
	glViewport(0, 0, width_, height_);

	glClearColor((rand() % 255) / 255.f, (rand() % 255) / 255.f, (rand() % 255) / 255.f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program_object_[0]);

	for (const auto &call : draw_calls_)
	{
		if (change_state_policy_ != ChangeStatePolicy::DontChange)
		{
			if (change_state_policy_ != ChangeStatePolicy::DontChangeProgram)
			{
				glUseProgram(call.state.program);
			}
			
			if (change_state_policy_ != ChangeStatePolicy::ChangeOnlyProgram)
			{
				glBindBuffer(GL_ARRAY_BUFFER, call.vbo);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, call.ibo);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
				glUniform4fv(call.state.color_location, 1, (float*)&call.state.color);
				glUniformMatrix4fv(call.state.matrix_location, 1, false, call.state.martrix.values);
				glDepthMask(call.state.depth_write_enabled);
				call.state.depth_test_enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
				call.state.alpha_blending_enabled ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
			}
		}
		glDrawElements(GL_TRIANGLES, call.index_count, GL_UNSIGNED_SHORT, 0);
	}
}
